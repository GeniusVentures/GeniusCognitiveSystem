/**
 * @file       test_gcs_ffi_coldboot.cpp
 * @brief      Live-app regression: gcs_init alone must boot the embedded node.
 * @details    2026-09-19 UAT failure — the packaged Flutter app reached
 *             gcs_init with no GeniusSDK node in the process
 *             (GeniusSDKGetNode() == nullptr), so
 *             CoreSession::Initialize failed with SdkNotInitialized and every
 *             publish died at the ABI ("can't reach the chat core"). The fix:
 *             gcs_init boots the embedded GeniusSDK node itself when none
 *             exists. This binary deliberately boots NO node itself — the
 *             cold-boot path IS the test. An empty-mnemonic GcsConfig keeps
 *             GeniusSDKInit's child-wallet contract (reuse the wallet
 *             persisted under the base path, create a child wallet when none
 *             exists). Two full init/subscribe/publish/shutdown cycles run
 *             against one db_path: the entity catalog must survive the cycle
 *             boundary, and gcs_shutdown's boot pairing must take the node
 *             down between cycles (cycle B re-boots it). On a gcs_init
 *             failure the SDK is probed directly to tell a real regression
 *             (the SDK boots fine) from an environment that cannot boot the
 *             SDK at all (skip). Own binary for the same reason as
 *             test_gcs_ffi_sdk (process-global node isolation). No raw
 *             thread sleeps — the FFI calls are synchronous under g_mutex.
 * @date       2026-09-19
 */

#include "ffi/gcs_core.h"
#include "proto/gcs_chat.pb.h"
#include "gcs_storage/common/logging.hpp"

// Own main() (std::_Exit after RUN_ALL_TESTS): the node's detached retry
// threads must never observe static destruction — see gcs_exit_main.hpp for
// the exit-time aborts this prevents (CI 35904291986 aarch64 segfaults).
#include "gcs_exit_main.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "dart_api_dl.h"
#include "dart_version.h"
#include "internal/dart_api_dl_impl.h"

#include "GeniusSDK.hpp"

#include <libp2p/log/configurator.hpp>
#include <libp2p/log/logger.hpp>

#include <soralog/impl/configurator_from_yaml.hpp>
#include <soralog/logging_system.hpp>

namespace
{
    /// Minimal soralog YAML — console sink, error level (mirrors
    /// test_gcs_ffi_sdk.cpp; quiets the booted node's libp2p logging).
    constexpr const char kLoggingYaml[] = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_ffi_coldboot_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";

    /// Dev config accepted by GeniusSDKInit's parser (offline-safe
    /// placeholders; identical to test_gcs_ffi_sdk.cpp — used only by the
    /// failure-classification probe, never to pre-boot the node).
    constexpr const char kDevConfig[] = R"(
     {
       "Address": "0x0000000000000000000000000000000000000001",
       "Cut": "100",
       "TokenValue": "1000",
       "TokenID": "0x0000000000000000000000000000000000000000000000000000000000000001"
     }
    )";

    /// Pinned pubsub listen port for this binary's node, written to
    /// network_config.json under the node base path in SetUp (db paths are
    /// <tmp>/db, so the base path is the tmp dir; the failure-classification
    /// probe's direct GeniusSDKInit uses the tmp dir as base too). GeniusNode
    /// derives ports as 40001 + hash%301 with no availability probe, so
    /// parallel node processes can collide on a derived port (observed in
    /// SuperGenius CI — child_registration.cpp); the pin sits OUTSIDE the
    /// derived range and differs per FFI test binary so parallel ctest runs
    /// never collide.
    constexpr uint16_t kPinnedPubsubPort = 41502;
    /// Arbitrary non-zero fake Dart NativePort id (pushed-event capture seam).
    constexpr int64_t kFakeDartPort = 7777;
    /// Dart -> C++ ingress topic (D-27: commands are topic publishes).
    constexpr const char kCommandTopic[] = "gcs/command";
    /// Event-stream topic used for the subscribe call.
    constexpr const char kEventTopic[] = "gcs/event";
    /// Entity id prefixes (mirror gcs_entity_store.cpp kSpace/kRoomIdPrefix).
    constexpr const char kSpaceIdPrefix[] = "space-";
    constexpr const char kRoomIdPrefix[]  = "room-";
    /// Derived room-topic prefix (D-01: gcs/chat/<room-id>).
    constexpr const char kRoomTopicPrefix[] = "gcs/chat/";
    /// Space name created in cycle A (must replay in cycle B).
    constexpr const char kSpaceName[] = "ops";
    /// Room name created inside kSpaceName (must replay in cycle B).
    constexpr const char kRoomName[] = "general";

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

    /**
     * @brief Extracts the newest SpaceTree among captured event payloads.
     *
     * @param[in] events Pushed payload bytes (one serialized GcsEvent each).
     * @return The last SpaceTree payload found, or nullopt when none pushed.
     */
    std::optional<gcs::chat::SpaceTree> FindLatestSpaceTree( const std::vector<std::string> &events )
    {
        std::optional<gcs::chat::SpaceTree> latest;
        for ( const std::string &eventBytes : events )
        {
            gcs::chat::GcsEvent event;
            if ( event.ParseFromString( eventBytes ) && event.has_space_tree() )
            {
                latest = event.space_tree();
            }
        }
        return latest;
    }

    /**
     * @brief Reports whether any captured RoomList payload carries the topic.
     *
     * @param[in] events Pushed payload bytes (one serialized GcsEvent each).
     * @param[in] topic   The room topic to look for.
     * @return true when at least one RoomList lists the topic.
     */
    bool RoomListCarriesTopic( const std::vector<std::string> &events, const std::string &topic )
    {
        for ( const std::string &eventBytes : events )
        {
            gcs::chat::GcsEvent event;
            if ( !event.ParseFromString( eventBytes ) || !event.has_room_list() )
            {
                continue;
            }
            for ( const std::string &listed : event.room_list().room_topic() )
            {
                if ( listed == topic )
                {
                    return true;
                }
            }
        }
        return false;
    }
} // namespace

namespace gcs::test
{
    /**
     * @brief Fixture: per-test temp directory, one-time soralog init, and a
     *        teardown that guarantees the process-global node is down even
     *        from mid-test failure paths.
     */
    class GcsFfiColdBoot : public ::testing::Test
    {
    protected:
        /**
         * @brief One-time logging-system init — quiets the booted node's
         *        libp2p logging (mirrors test_gcs_ffi_sdk.cpp SetUpTestSuite).
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
            // The cold-boot path boots the node INSIDE gcs_ffi's image (the
            // test itself boots nothing — that IS the test), so the
            // secure-storage factory must be set on the DLL's copy via the
            // exported seam (IMAGE RULE, gcs_storage/common/test_env.hpp).
            // Avoids the flaky OS keychain on CI runners (35904291986 /
            // 35918456744: "Failed to generate Genius address from private
            // key" → skip instead of exercising the cold-boot path).
            gcs_use_test_secure_storage();

            const auto *info       = ::testing::UnitTest::GetInstance()->current_test_info();
            const auto  uniqueSalt = std::chrono::steady_clock::now().time_since_epoch().count();
            m_tempPath = ( std::filesystem::temp_directory_path()
                           / ( std::string{ "gcs_ffi_coldboot_" } + info->name() + "_"
                               + std::to_string( uniqueSalt ) ) )
                             .string();
            std::filesystem::create_directories( m_tempPath );

            // Pin the node's pubsub port (child_registration.cpp pattern):
            // the node reads this file at InitNetwork.
            std::ofstream networkConfig( m_tempPath + "/network_config.json" );
            networkConfig << "{ \"port_seed\": " << kPinnedPubsubPort
                          << ", \"auto_dht\": false, \"upnp_enabled\": false"
                          << ", \"pubsub_port\": \"" << kPinnedPubsubPort << "\" }";
        }

        void TearDown() override
        {
            // A failure path may leave the internally-booted node up (gcs_init
            // booted it, the test died before gcs_shutdown) — take it down so
            // nothing leaks past this binary.
            if ( GeniusSDKGetNode() != nullptr )
            {
                GeniusSDKShutdown();
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
         * @brief gcs_init against dbPath with an empty-mnemonic config — the
         *        child-wallet cold-boot path. A null return is classified by
         *        probing the SDK directly: a probe that also fails means this
         *        environment cannot boot a node (caller skips); a probe that
         *         succeeds means gcs_init dropped the boot — the exact
         *        regression this test guards.
         *
         * @param[in] dbPath Store path for the config bytes.
         * @return The session handle, or nullptr (caller decides skip/fail).
         */
        GcsSession *InitSessionOrSkip( const std::string &dbPath )
        {
            gcs::chat::GcsConfig config;
            config.set_db_path( dbPath );
            config.set_codec( gcs::chat::CODEC_PROTOBUF );
            const std::string configBytes = config.SerializeAsString();
            GcsSession *handle            = gcs_init( reinterpret_cast<const uint8_t *>( configBytes.data() ),
                                                      configBytes.size() );
            if ( handle != nullptr )
            {
                return handle;
            }
            if ( GeniusSDKInit( m_tempPath.c_str(), kDevConfig ) == nullptr )
            {
                return nullptr; // environment cannot boot the SDK at all
            }
            GeniusSDKShutdown(); // leave clean state for TearDown
            EXPECT_NE( handle, nullptr ) << "gcs_init returned null though the SDK boots — embedded-node boot regression";
            return nullptr;
        }

        std::string m_tempPath; ///< Per-test temp directory (created in SetUp, removed in TearDown)
    };

    /**
     * @brief A lone caller with no booted node gets a working session:
     *        gcs_init boots the embedded node (child wallet), serves
     *        create_space/create_room with SpaceTree + derived-join pushes,
     *        gcs_shutdown takes the node down, and a second cycle re-boots
     *        and replays the persisted catalog.
     */
    TEST_F( GcsFfiColdBoot, ColdInitBootsEmbeddedNodeAndPersistsEntitiesAcrossCycles )
    {
        ASSERT_TRUE( InstallFakeApiDlTable() );

        const std::string dbPath = m_tempPath + "/db";

        // === Cycle A — no node exists anywhere in the process ===
        GcsSession *handleA = InitSessionOrSkip( dbPath );
        if ( handleA == nullptr )
        {
            GTEST_SKIP() << "SDK cannot boot in this environment";
        }
        EXPECT_EQ( gcs_subscribe( handleA, kEventTopic, kFakeDartPort ), GCS_OK );

        gcs::chat::GcsCommand createSpace;
        createSpace.mutable_create_space()->set_name( kSpaceName );
        createSpace.mutable_create_space()->set_is_public( true );
        createSpace.mutable_create_space()->set_auto_join_rooms( true );
        PublishCommand( handleA, createSpace );

        const auto treeA1 = FindLatestSpaceTree( g_pushedEvents.Take() );
        ASSERT_TRUE( treeA1.has_value() );
        ASSERT_EQ( treeA1->space_size(), 1 );
        const std::string spaceId = treeA1->space( 0 ).id();
        EXPECT_EQ( spaceId.rfind( kSpaceIdPrefix, 0 ), 0 );

        gcs::chat::GcsCommand createRoom;
        createRoom.mutable_create_room()->set_name( kRoomName );
        createRoom.mutable_create_room()->set_parent_space_id( spaceId );
        PublishCommand( handleA, createRoom );

        const auto eventsA2      = g_pushedEvents.Take();
        const auto treeA2        = FindLatestSpaceTree( eventsA2 );
        ASSERT_TRUE( treeA2.has_value() );
        ASSERT_EQ( treeA2->space_size(), 1 );
        ASSERT_EQ( treeA2->room_size(), 1 );
        const std::string roomId = treeA2->room( 0 ).id();
        EXPECT_EQ( roomId.rfind( kRoomIdPrefix, 0 ), 0 );
        EXPECT_EQ( treeA2->room( 0 ).parent_space_id(), spaceId );
        // Derived join: the autoJoin space's room topic enters the RoomList.
        EXPECT_TRUE( RoomListCarriesTopic( eventsA2, std::string( kRoomTopicPrefix ) + roomId ) );

        // gcs_shutdown pairs the internal boot — the node goes down with it.
        gcs_shutdown( handleA );
        EXPECT_EQ( GeniusSDKGetNode(), nullptr );

        // File logging (GeniusWallet requirement): the session wrote its own
        // rotating log beside the store and it captured this cycle's entries.
        const auto logFile = std::filesystem::path( m_tempPath ) / sgns::gcs::kGcsLogFileName;
        ASSERT_TRUE( std::filesystem::exists( logFile ) );
        std::error_code logSizeEc;
        EXPECT_GT( std::filesystem::file_size( logFile, logSizeEc ), 0u );

        // === Cycle B — same db_path, still no external boot anywhere ===
        GcsSession *handleB = InitSessionOrSkip( dbPath );
        if ( handleB == nullptr )
        {
            GTEST_SKIP() << "SDK cannot reboot in this environment";
        }
        EXPECT_EQ( gcs_subscribe( handleB, kEventTopic, kFakeDartPort ), GCS_OK );

        // The subscribe-time push replays the persisted catalog (persistence
        // through the internal boot path; the child wallet persists under the
        // base path across the cycle boundary).
        const auto treeB = FindLatestSpaceTree( g_pushedEvents.Take() );
        ASSERT_TRUE( treeB.has_value() );
        ASSERT_EQ( treeB->space_size(), 1 );
        EXPECT_EQ( treeB->space( 0 ).name(), kSpaceName );
        ASSERT_EQ( treeB->room_size(), 1 );
        EXPECT_EQ( treeB->room( 0 ).name(), kRoomName );
        EXPECT_EQ( treeB->room( 0 ).parent_space_id(), spaceId );

        gcs_shutdown( handleB );
        EXPECT_EQ( GeniusSDKGetNode(), nullptr );
    }
} // namespace gcs::test
