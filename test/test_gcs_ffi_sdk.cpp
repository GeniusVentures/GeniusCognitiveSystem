/**
 * @file       test_gcs_ffi_sdk.cpp
 * @brief      CR-01 regression: message ids must never revisit a prior
 *             session's key space across init/shutdown cycles on one db_path.
 * @details    Own binary on purpose (mirrors test_gcs_global_db_sdk.cpp):
 *             GeniusSDKInit toggles a process-global node, which must not
 *             leak into the option-C binaries. The test installs a fake Dart
 *             API_DL table — Dart_InitializeApiDL copies a plain
 *             name/function table, so a fake Dart_PostCObject entry captures
 *             the pushed GcsEvent bytes and lets a plain C++ test observe the
 *             C++-stamped message ids — then runs TWO init/join/send/shutdown
 *             cycles against the SAME db_path and reopens the store (injected
 *             pubsub seam, no second node) to assert session A's persisted
 *             record survived session B. The node is booted through
 *             gcs_ffi's exported GeniusSDKInit so the ABI path sees the same
 *             node the test started. Skips when the SDK cannot boot (option
 *             C). Wait-condition templates only; no raw thread sleeps.
 * @date       2026-09-16
 */

#include "ffi/gcs_core.h"
#include "proto/gcs_chat.pb.h"
#include "test_graphsync_network.hpp"
#include "test_wait_condition.hpp"

#include "gcs_storage/gcs_global_db.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "dart_api_dl.h"
#include "dart_version.h"
#include "internal/dart_api_dl_impl.h"

#include "crdt/globaldb/keypair_file_storage.hpp"

#include "ipfs_pubsub/gossip_pubsub.hpp"

#include <libp2p/log/configurator.hpp>
#include <libp2p/log/logger.hpp>

#include <soralog/impl/configurator_from_yaml.hpp>
#include <soralog/logging_system.hpp>

#include "GeniusSDK.hpp"

namespace
{
    /// Minimal soralog YAML — console sink, error level (mirrors
    /// test_gcs_global_db_sdk.cpp; required before any SuperGenius logger).
    constexpr const char kLoggingYaml[] = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_ffi_sdk_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";

    /// Dev config accepted by GeniusSDKInit's parser (offline-safe placeholders;
    /// identical to test_gcs_global_db_sdk.cpp).
    constexpr const char kDevConfig[] = R"(
     {
       "Address": "0x0000000000000000000000000000000000000001",
       "Cut": "100",
       "TokenValue": "1000",
       "TokenID": "0x0000000000000000000000000000000000000000000000000000000000000001"
     }
    )";

    /// Arbitrary non-zero fake Dart NativePort id (pushed-event capture seam).
    constexpr int64_t kFakeDartPort = 7777;
    /// Room joined and sent to in both session cycles.
    constexpr const char kRoomTopic[] = "gcs/chat/cr01-regression";
    /// Text payload stamped by session cycle A (must survive cycle B).
    constexpr const char kSessionAText[] = "cr01-record-session-a";
    /// Text payload stamped by session cycle B.
    constexpr const char kSessionBText[] = "cr01-record-session-b";
    /// Dart -> C++ ingress topic (D-27: commands are topic publishes).
    constexpr const char kCommandTopic[] = "gcs/command";
    /// Event-stream topic used for the subscribe call.
    constexpr const char kEventTopic[] = "gcs/event";
    /// C++-stamped message id prefix (mirrors gcs_core_ffi.cpp kMessageIdPrefix).
    constexpr const char kMessageIdPrefix[] = "msg-";
    /// Minimum '-'-separated salt fields an id must carry after the prefix
    /// ("msg-<wallclock-ms>-<random-token>-<seq>" carries two: a revert to a
    /// bare per-process counter carries none and revisits prior key space).
    constexpr int kMinIdSaltSeparators = 2;
    /// GossipPubSub bind address for the verification store.
    constexpr const char kListenIp[] = "0.0.0.0";

    /**
     * @brief Thread-safe log of payloads posted to the fake Dart port.
     *
     * gcs_ffi posts synchronously under its own mutex; the separate mutex here
     * keeps the capture self-contained.
     */
    class PushedEventLog
    {
    public:
        /**
         * @brief Appends one posted typed-data (uint8) payload.
         *
         * @param[in] data   Payload bytes.
         * @param[in] length Payload length in bytes.
         */
        void Add( const char *data, size_t length )
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            m_events.emplace_back( data, length );
        }

        /**
         * @brief Returns the buffered payloads and clears the log.
         *
         * @return All payloads posted since the previous Take, in order.
         */
        std::vector<std::string> Take()
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            return std::exchange( m_events, std::vector<std::string>{} );
        }

    private:
        std::mutex m_mutex;                ///< guards m_events
        std::vector<std::string> m_events; ///< posted payloads in order
    };

    /// Process-global capture sink — the C callback carries no user-data slot.
    PushedEventLog g_pushedEvents;

    /**
     * @brief Fake Dart_PostCObject: captures typed-data (uint8) posts.
     *
     * @param[in] port    The registered fake port id (unused).
     * @param[in] message The posted CObject.
     * @return true (the post "succeeded").
     */
    bool FakePostCObject( Dart_Port_DL port, Dart_CObject *message )
    {
        (void)port;
        if ( message->type == Dart_CObject_kTypedData
             && message->value.as_typed_data.type == Dart_TypedData_kUint8 )
        {
            g_pushedEvents.Add( reinterpret_cast<const char *>( message->value.as_typed_data.values ),
                                static_cast<size_t>( message->value.as_typed_data.length ) );
        }
        return true;
    }

    /**
     * @brief Installs the fake API_DL table into gcs_ffi so pushed GcsEvent
     *        bytes land in g_pushedEvents instead of a (nonexistent) Dart VM.
     *
     * Only the Dart_PostCObject entry is populated — gcs_ffi uses no other DL
     * symbol, and the table's remaining entries resolve to null and are never
     * called.
     *
     * @return true when the library accepted the table (major version match).
     */
    bool InstallFakeApiDlTable()
    {
        static const DartApiEntry kEntries[] = {
            { "Dart_PostCObject", reinterpret_cast<void ( * )()>( &FakePostCObject ) },
            { nullptr, nullptr },
        };
        static const DartApi kTable = { DART_API_DL_MAJOR_VERSION, DART_API_DL_MINOR_VERSION, kEntries };
        return Dart_InitializeApiDL( const_cast<DartApi *>( &kTable ) ) == 0;
    }
} // namespace

namespace gcs::test
{
    /**
     * @brief Fixture: per-test temp directory, one-time soralog init, and the
     *        SDK-node teardown flag.
     */
    class GcsFfiSdk : public ::testing::Test
    {
    protected:
        /**
         * @brief One-time logging-system init — SuperGenius's base::createLogger
         *        asserts a configured soralog LoggingSystem (mirrors
         *        test_gcs_global_db_sdk.cpp SetUpTestSuite).
         */
        static void SetUpTestSuite()
        {
            auto loggerConfigurator = std::make_shared<libp2p::log::Configurator>();
            auto configFromYaml     = std::make_shared<soralog::ConfiguratorFromYAML>( loggerConfigurator,
                                                                                       std::string{ kLoggingYaml } );
            auto loggingSystem      = std::make_shared<soralog::LoggingSystem>( configFromYaml );
            const auto confResult   = loggingSystem->configure();
            ASSERT_FALSE( confResult.has_error ) << "Could not configure test logging system";
            libp2p::log::setLoggingSystem( loggingSystem );
        }

        void SetUp() override
        {
            const auto *info       = ::testing::UnitTest::GetInstance()->current_test_info();
            const auto  uniqueSalt = std::chrono::steady_clock::now().time_since_epoch().count();
            m_tempPath = ( std::filesystem::temp_directory_path()
                           / ( std::string{ "gcs_ffi_sdk_" } + info->name() + "_"
                               + std::to_string( uniqueSalt ) ) )
                             .string();
            std::filesystem::create_directories( m_tempPath );
        }

        void TearDown() override
        {
            if ( m_sdkStarted )
            {
                GeniusSDKShutdown();
                m_sdkStarted = false;
            }
            std::error_code ec;
            std::filesystem::remove_all( m_tempPath, ec );
        }

        /**
         * @brief Publishes a serialized GcsCommand to the command topic.
         *
         * @param[in] handle  Session handle from gcs_init.
         * @param[in] command The command envelope to publish.
         */
        void PublishCommand( GcsSession *handle, const gcs::chat::GcsCommand &command )
        {
            const std::string payload = command.SerializeAsString();
            EXPECT_EQ( gcs_publish( handle,
                                    kCommandTopic,
                                    reinterpret_cast<const uint8_t *>( payload.data() ),
                                    payload.size() ),
                       GCS_OK );
        }

        /**
         * @brief One full FFI session cycle: init against dbPath, subscribe the
         *        fake port, join kRoomTopic, publish sendText, shutdown.
         *
         * @param[in]  dbPath        Persistent CRDT store path (shared across cycles).
         * @param[in]  sendText      Distinct per-cycle text payload.
         * @param[out] outMessageIds C++-stamped ids observed on the pushed echo.
         * @return false when gcs_init could not create the session (caller
         *         decides skip vs failure); true after a completed cycle.
         */
        bool RunSessionCycle( const std::string &dbPath,
                              const std::string &sendText,
                              std::vector<std::string> &outMessageIds )
        {
            gcs::chat::GcsConfig config;
            config.set_db_path( dbPath );
            config.set_codec( gcs::chat::CODEC_PROTOBUF );
            const std::string configBytes = config.SerializeAsString();
            GcsSession *handle            = gcs_init( reinterpret_cast<const uint8_t *>( configBytes.data() ),
                                                      configBytes.size() );
            if ( handle == nullptr )
            {
                return false;
            }

            EXPECT_EQ( gcs_subscribe( handle, kEventTopic, kFakeDartPort ), GCS_OK );

            gcs::chat::GcsCommand joinCommand;
            joinCommand.mutable_join_topic()->set_room_topic( kRoomTopic );
            PublishCommand( handle, joinCommand );

            gcs::chat::GcsCommand sendCommand;
            sendCommand.mutable_send_text()->set_room_topic( kRoomTopic );
            sendCommand.mutable_send_text()->set_text( sendText );
            PublishCommand( handle, sendCommand );

            for ( const std::string &eventBytes : g_pushedEvents.Take() )
            {
                gcs::chat::GcsEvent event;
                if ( event.ParseFromString( eventBytes ) && event.has_message() )
                {
                    outMessageIds.push_back( event.message().id() );
                }
            }

            gcs_shutdown( handle );
            return true;
        }

        /**
         * @brief Stand up a real GossipPubSub on an ephemeral port for the
         *        verification store (mirrors test_gcs_core_smoke.cpp).
         *
         * @param[in] keyDir Directory for the KeyPairFileStorage key store.
         * @return A started GossipPubSub, or nullptr on bring-up failure.
         */
        std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> MakeStartedPubSub( const std::string &keyDir )
        {
            sgns::crdt::KeyPairFileStorage keyStore( keyDir );
            auto                           keyPairResult = keyStore.GetKeyPair();
            EXPECT_FALSE( keyPairResult.has_error() );
            if ( keyPairResult.has_error() )
            {
                return nullptr;
            }
            auto pubsub      = std::make_shared<sgns::ipfs_pubsub::GossipPubSub>( keyPairResult.value() );
            auto startFuture = pubsub->Start( 0, {}, kListenIp, {} ); // port 0 = ephemeral
            auto startError  = startFuture.get();
            EXPECT_FALSE( startError ) << "Could not start GossipPubSub: " << startError.message();
            if ( startError )
            {
                return nullptr;
            }
            return pubsub;
        }

        /**
         * @brief Waits (wait-condition template) for a non-empty record under key.
         *
         * @param[in] db  Verification store opened on the cycles' db_path.
         * @param[in] key HierarchicalKey of the record.
         * @return The stored bytes once visible; empty on timeout.
         */
        std::string WaitForRecord( sgns::neoswarm::storage::GcsGlobalDb &db, const std::string &key )
        {
            std::string value;
            EXPECT_TRUE( gcs::test::WaitForCondition(
                [&db, &key, &value]() {
                    auto result = db.Get( key );
                    if ( result.has_value() && !result.value().empty() )
                    {
                        value = result.value();
                        return true;
                    }
                    return false;
                },
                gcs::test::kWaitTimeout ) )
                << "record '" << key << "' not visible after both session cycles";
            return value;
        }

        std::string m_tempPath;      ///< Per-test temp directory (removed in TearDown)
        bool        m_sdkStarted = false; ///< Whether GeniusSDKInit booted (TearDown shuts it down)
    };

    /**
     * @brief CR-01 regression: two init/send/shutdown cycles against the SAME
     *        db_path — session A's persisted record must survive session B,
     *        and stamped ids must carry a per-process salt (wall-clock ms +
     *        random token), never a bare per-process counter that restarts at
     *        zero every launch and overwrites prior sessions' records.
     */
    TEST_F( GcsFfiSdk, SendTextRecordsSurviveAcrossSessionCyclesOnSharedDb )
    {
        ASSERT_TRUE( InstallFakeApiDlTable() ) << "gcs_ffi rejected the fake Dart API_DL table";

        const char *initPath = GeniusSDKInit( m_tempPath.c_str(), kDevConfig );
        if ( initPath == nullptr )
        {
            GTEST_SKIP() << "GeniusSDKInit could not boot a node in this environment (option C)";
        }
        m_sdkStarted = true;

        const std::string  dbPath = m_tempPath + "/db";
        std::vector<std::string> idsA;
        ASSERT_TRUE( RunSessionCycle( dbPath, kSessionAText, idsA ) )
            << "SDK is up but the first gcs_init cycle failed";
        std::vector<std::string> idsB;
        ASSERT_TRUE( RunSessionCycle( dbPath, kSessionBText, idsB ) )
            << "SDK is up but the second gcs_init cycle failed";

        ASSERT_EQ( idsA.size(), 1U );
        ASSERT_EQ( idsB.size(), 1U );
        EXPECT_NE( idsA.front(), idsB.front() ) << "the two cycles stamped colliding ids";

        for ( const std::string *id : { &idsA.front(), &idsB.front() } )
        {
            EXPECT_EQ( id->rfind( kMessageIdPrefix, 0 ), 0 ) << "id '" << *id << "' lost its prefix";
            const std::string body       = id->substr( std::strlen( kMessageIdPrefix ) );
            const int         separators = static_cast<int>( std::count( body.begin(), body.end(), '-' ) );
            EXPECT_GE( separators, kMinIdSaltSeparators )
                << "id '" << *id << "' carries no per-process salt (bare counter revisits prior "
                   "sessions' key space)";
        }

        // Persistence proof: reopen the SAME db_path through an injected-pubsub
        // GcsGlobalDb (test seam — no second node) and read both cycles' records.
        auto pubsub = MakeStartedPubSub( m_tempPath + "/verify-key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        sgns::neoswarm::storage::GcsGlobalDb::Config cfg{};
        cfg.m_dbPath                                = dbPath;
        sgns::neoswarm::storage::GcsGlobalDb verifyDb( cfg );
        ASSERT_TRUE( verifyDb.Initialize( pubsub, graphsync.network ).has_value() );

        const std::string recordA = WaitForRecord( verifyDb, std::string( kRoomTopic ) + "/" + idsA.front() );
        ASSERT_FALSE( recordA.empty() );
        gcs::chat::ChatMessageState messageA;
        ASSERT_TRUE( messageA.ParseFromString( recordA ) );
        EXPECT_EQ( messageA.text(), kSessionAText ) << "session A's record was overwritten (CR-01)";

        const std::string recordB = WaitForRecord( verifyDb, std::string( kRoomTopic ) + "/" + idsB.front() );
        ASSERT_FALSE( recordB.empty() );
        gcs::chat::ChatMessageState messageB;
        ASSERT_TRUE( messageB.ParseFromString( recordB ) );
        EXPECT_EQ( messageB.text(), kSessionBText );

        verifyDb.Shutdown();
        pubsub->Stop();
    }
} // namespace gcs::test
