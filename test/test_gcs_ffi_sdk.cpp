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
    /// Entity id prefixes (mirror gcs_entity_store.cpp kSpace/kRoomIdPrefix).
    constexpr const char kSpaceIdPrefix[] = "space-";
    constexpr const char kRoomIdPrefix[]  = "room-";
    /// Derived room-topic prefix (D-01: gcs/chat/<room-id>).
    constexpr const char kRoomTopicPrefix[] = "gcs/chat/";
    /// Space name created in the entity persistence test.
    constexpr const char kSpaceName[] = "ops";
    /// Room name created inside kSpaceName.
    constexpr const char kRoomName[] = "general";
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
         * @brief gcs_init against dbPath with the protobuf codec (D-29).
         *
         * @param[in] dbPath Persistent CRDT store path.
         * @return Session handle, or nullptr when gcs_init failed.
         */
        GcsSession *InitSession( const std::string &dbPath )
        {
            gcs::chat::GcsConfig config;
            config.set_db_path( dbPath );
            config.set_codec( gcs::chat::CODEC_PROTOBUF );
            const std::string configBytes = config.SerializeAsString();
            return gcs_init( reinterpret_cast<const uint8_t *>( configBytes.data() ),
                             configBytes.size() );
        }

        /**
         * @brief Drains the pushed-event log; reports the latest SpaceTree.
         *
         * gcs_ffi posts synchronously under its own mutex, so every push from
         * the preceding publishes/subscribe is already buffered when this
         * runs — no wait needed.
         *
         * \param[out] outEvents Every drained event parsed, in push order
         *                       (ordering assertions inspect this).
         * \param[out] outTree   The latest (last-pushed) SpaceTree seen.
         * @return true when at least one SpaceTree event was drained.
         */
        bool TakeLatestSpaceTree( std::vector<gcs::chat::GcsEvent> &outEvents,
                                  gcs::chat::SpaceTree &outTree )
        {
            bool found = false;
            for ( const std::string &eventBytes : g_pushedEvents.Take() )
            {
                gcs::chat::GcsEvent event;
                if ( !event.ParseFromString( eventBytes ) )
                {
                    continue;
                }
                outEvents.push_back( event );
                if ( event.has_space_tree() )
                {
                    outTree = event.space_tree();
                    found   = true;
                }
            }
            return found;
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

    /**
     * @brief Phase 2 entity flow through the real ABI: create_space +
     *        create_room mutate the catalog and push SpaceTree; the room in
     *        the autoJoin space derives into the pushed RoomList (D-04); the
     *        whole catalog survives a full init/shutdown cycle on the SAME
     *        db_path (restart persistence, criterion 4 — Pitfall 6: shutdown
     *        between cycles so the second gcs_init is a true restart).
     */
    TEST_F( GcsFfiSdk, CreateSpaceAndRoomPersistAcrossSessionCyclesOnSharedDb )
    {
        ASSERT_TRUE( InstallFakeApiDlTable() ) << "gcs_ffi rejected the fake Dart API_DL table";

        const char *initPath = GeniusSDKInit( m_tempPath.c_str(), kDevConfig );
        if ( initPath == nullptr )
        {
            GTEST_SKIP() << "GeniusSDKInit could not boot a node in this environment (option C)";
        }
        m_sdkStarted = true;

        const std::string dbPath = m_tempPath + "/db";
        std::string       spaceId;
        std::string       roomId;

        // Cycle A: create the space, then the room inside it.
        GcsSession *handleA = InitSession( dbPath );
        ASSERT_NE( handleA, nullptr ) << "SDK is up but gcs_init cycle A failed";
        ASSERT_EQ( gcs_subscribe( handleA, kEventTopic, kFakeDartPort ), GCS_OK );

        gcs::chat::GcsCommand createSpace;
        createSpace.mutable_create_space()->set_name( kSpaceName );
        createSpace.mutable_create_space()->set_is_public( true );
        createSpace.mutable_create_space()->set_auto_join_rooms( true );
        PublishCommand( handleA, createSpace );

        std::vector<gcs::chat::GcsEvent> eventsA;
        gcs::chat::SpaceTree             treeA;
        ASSERT_TRUE( TakeLatestSpaceTree( eventsA, treeA ) ) << "create_space pushed no SpaceTree";
        ASSERT_EQ( treeA.space_size(), 1 );
        spaceId = treeA.space( 0 ).id();
        EXPECT_EQ( spaceId.rfind( kSpaceIdPrefix, 0 ), 0 ) << "space id lost its prefix";
        EXPECT_EQ( treeA.space( 0 ).name(), kSpaceName );
        EXPECT_TRUE( treeA.space( 0 ).auto_join_rooms() );

        gcs::chat::GcsCommand createRoom;
        createRoom.mutable_create_room()->set_name( kRoomName );
        createRoom.mutable_create_room()->set_parent_space_id( spaceId );
        PublishCommand( handleA, createRoom );

        eventsA.clear();
        ASSERT_TRUE( TakeLatestSpaceTree( eventsA, treeA ) ) << "create_room pushed no SpaceTree";
        ASSERT_EQ( treeA.space_size(), 1 );
        ASSERT_EQ( treeA.room_size(), 1 );
        roomId = treeA.room( 0 ).id();
        EXPECT_EQ( roomId.rfind( kRoomIdPrefix, 0 ), 0 ) << "room id lost its prefix";
        EXPECT_EQ( treeA.room( 0 ).name(), kRoomName );
        EXPECT_EQ( treeA.room( 0 ).parent_space_id(), spaceId );

        // Derived join (D-04): a RoomList pushed after the SpaceTree carrying
        // the room must contain the room's derived topic gcs/chat/<room-id>.
        const std::string derivedTopic = std::string( kRoomTopicPrefix ) + roomId;
        bool sawTreeCarryingRoom       = false;
        bool roomListCarriesDerived    = false;
        for ( const gcs::chat::GcsEvent &event : eventsA )
        {
            if ( event.has_space_tree() && event.space_tree().room_size() == 1 )
            {
                sawTreeCarryingRoom = true;
            }
            if ( sawTreeCarryingRoom && event.has_room_list() )
            {
                for ( const std::string &topic : event.room_list().room_topic() )
                {
                    if ( topic == derivedTopic )
                    {
                        roomListCarriesDerived = true;
                    }
                }
            }
        }
        EXPECT_TRUE( sawTreeCarryingRoom ) << "create_room pushed no SpaceTree carrying the room";
        EXPECT_TRUE( roomListCarriesDerived ) << "derived join topic '" << derivedTopic
                                              << "' missing from the pushed RoomList";

        gcs_shutdown( handleA );

        // Cycle B: reopen the SAME db_path — the catalog must survive restart.
        GcsSession *handleB = InitSession( dbPath );
        ASSERT_NE( handleB, nullptr ) << "SDK is up but gcs_init cycle B failed";
        ASSERT_EQ( gcs_subscribe( handleB, kEventTopic, kFakeDartPort ), GCS_OK );

        std::vector<gcs::chat::GcsEvent> eventsB;
        gcs::chat::SpaceTree             treeB;
        ASSERT_TRUE( TakeLatestSpaceTree( eventsB, treeB ) )
            << "subscribe pushed no SpaceTree after restart";
        ASSERT_EQ( treeB.space_size(), 1 );
        ASSERT_EQ( treeB.room_size(), 1 );
        EXPECT_EQ( treeB.space( 0 ).id(), spaceId );
        EXPECT_EQ( treeB.space( 0 ).name(), kSpaceName );
        EXPECT_EQ( treeB.room( 0 ).id(), roomId );
        EXPECT_EQ( treeB.room( 0 ).name(), kRoomName );
        EXPECT_EQ( treeB.room( 0 ).parent_space_id(), spaceId );

        gcs_shutdown( handleB );
    }
} // namespace gcs::test
