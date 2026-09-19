/**
 * @file       test_gcs_entities.cpp
 * @brief      Phase 2 EntityStore unit tests (CORE-01/CORE-02/CORE-03,
 *             tombstone skip, restart persistence).
 * @details    Covers gcs::EntityStore over an injected-pubsub CoreSession (the
 *             test_gcs_core_smoke.cpp fixture seam — no GeniusNode boot):
 *             space/room create (salted opaque ids, D-01), standalone rooms
 *             (empty parent), derived joins (D-04 — including the retroactive
 *             autoJoinRooms toggle, CORE-03), unknown-parent rejection,
 *             tombstone reader-skip (D-03, exercised by directly writing
 *             tombstoned record bytes), and catalog survival across two
 *             sequential sessions on one shared db_path (restart criterion).
 *             Uses the GCS wait-condition template — never a raw thread sleep.
 * @date       2026-09-19
 */

#include "lib/gcs_entity_store.hpp"
#include "test_graphsync_network.hpp"
#include "test_wait_condition.hpp"

#include <algorithm>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "crdt/globaldb/keypair_file_storage.hpp"

#include "ipfs_pubsub/gossip_pubsub.hpp"

#include <libp2p/log/configurator.hpp>
#include <libp2p/log/logger.hpp>

#include <soralog/impl/configurator_from_yaml.hpp>
#include <soralog/logging_system.hpp>

namespace
{
    /// GossipPubSub bind address used by every test.
    constexpr const char *kListenIp = "0.0.0.0";

    /// Minimal soralog YAML — console sink only, error level, sufficient to satisfy
    /// libp2p::log::setLoggingSystem() before any SuperGenius logger is constructed.
    constexpr const char *kLoggingYaml = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_entities_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";

    /// C++-stamped entity id prefixes (mirror gcs_entity_store.cpp constants).
    constexpr const char kSpaceIdPrefix[] = "space-";
    constexpr const char kRoomIdPrefix[]  = "room-";

    /// Room chat topic prefix — the gcs/chat/<id> convention (D-01).
    constexpr const char kRoomTopicPrefix[] = "gcs/chat/";

    /// Minimum '-'-separated salt fields an id must carry after the prefix
    /// ("space-<wallclock-ms>-<random-token>-<seq>" carries two: a revert to a
    /// bare per-process counter carries none and revisits prior key space).
    constexpr int kMinIdSaltSeparators = 2;

    /// CRDT keys (mirror gcs_entity_store.cpp constants) for direct-write tests.
    constexpr const char kSpacesKeyPrefix[] = "gcs/entities/spaces/";
    constexpr const char kRoomsKeyPrefix[]  = "gcs/entities/rooms/";
    constexpr const char kManifestKey[]     = "gcs/index/manifest";

    /// Arbitrary tombstone timestamp for direct-write fixtures (D-03).
    constexpr int64_t kTombstoneAtMs = 1234;
} // namespace

namespace gcs::test
{
    /**
     * @brief Fixture that owns a per-test temp directory under
     *        std::filesystem::temp_directory_path() and removes it in TearDown.
     */
    class GcsEntitiesTest : public ::testing::Test
    {
    protected:
        /**
         * @brief One-time logging-system init — SuperGenius's base::createLogger asserts
         *        a configured soralog LoggingSystem; without it GossipPubSub construction
         *        crashes with "Logging system is not ready". Mirrors test_gcs_core_smoke.cpp.
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
                           / ( std::string{ "gcs_entities_" } + info->name() + "_" +
                               std::to_string( uniqueSalt ) ) )
                             .string();
            std::filesystem::create_directories( m_tempPath );
        }

        void TearDown() override
        {
            std::error_code ec;
            std::filesystem::remove_all( m_tempPath, ec );
        }

        /**
         * @brief Stand up a real GossipPubSub bound to port 0 (random free port).
         *        Mirrors test_gcs_core_smoke.cpp bring-up.
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
         * @brief Asserts the CR-01 salt-separator shape of a C++-minted entity id.
         *
         * @param[in] id     The minted entity id.
         * @param[in] prefix The expected id prefix ("space-" / "room-").
         */
        void ExpectSaltedEntityId( const std::string &id, const char *prefix )
        {
            EXPECT_EQ( id.rfind( prefix, 0 ), 0 ) << "id '" << id << "' lost its prefix";
            const std::string body       = id.substr( std::strlen( prefix ) );
            const int         separators = static_cast<int>( std::count( body.begin(), body.end(), '-' ) );
            EXPECT_GE( separators, kMinIdSaltSeparators )
                << "id '" << id << "' carries no per-process salt (bare counter revisits prior "
                   "sessions' key space)";
        }

        std::string m_tempPath; ///< Per-test temp directory (created in SetUp, removed in TearDown)
    };

    /**
     * @brief CORE-01: CreateSpace mints a salted opaque id, persists record +
     *        manifest, and the record is visible through Spaces() and a fresh reload.
     */
    TEST_F( GcsEntitiesTest, CreateSpaceWritesRecordAndManifest )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( gcs::test::WaitForCondition( [&dbPath]() { return std::filesystem::exists( dbPath ); },
                                                  gcs::test::kWaitTimeout ) );

        gcs::EntityStore store( session );
        ASSERT_TRUE( store.LoadFromStore().has_value() );

        auto created = store.CreateSpace( "ops", true, true );
        ASSERT_TRUE( created.has_value() );
        ExpectSaltedEntityId( created.value().id(), kSpaceIdPrefix );
        EXPECT_EQ( created.value().name(), "ops" );
        EXPECT_TRUE( created.value().is_public() );
        EXPECT_TRUE( created.value().auto_join_rooms() );
        EXPECT_FALSE( created.value().deleted() );

        const auto spaces = store.Spaces();
        ASSERT_EQ( spaces.size(), 1U );
        EXPECT_EQ( spaces.front().id(), created.value().id() );
        EXPECT_TRUE( store.IsValidParentSpace( created.value().id() ) );

        // Manifest proof: a fresh store replays the same record from the
        // manifest + record keys (N+1 reads).
        gcs::EntityStore reloaded( session );
        ASSERT_TRUE( reloaded.LoadFromStore().has_value() );
        ASSERT_EQ( reloaded.Spaces().size(), 1U );
        EXPECT_EQ( reloaded.Spaces().front().id(), created.value().id() );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief CORE-02: a standalone room (empty parent, D-01) is created and
     *        never derives a joined topic.
     */
    TEST_F( GcsEntitiesTest, CreateStandaloneRoomHasNoParent )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( gcs::test::WaitForCondition( [&dbPath]() { return std::filesystem::exists( dbPath ); },
                                                  gcs::test::kWaitTimeout ) );

        gcs::EntityStore store( session );
        ASSERT_TRUE( store.LoadFromStore().has_value() );

        auto created = store.CreateRoom( "lounge", "" );
        ASSERT_TRUE( created.has_value() );
        ExpectSaltedEntityId( created.value().id(), kRoomIdPrefix );

        const auto rooms = store.Rooms();
        ASSERT_EQ( rooms.size(), 1U );
        EXPECT_EQ( rooms.front().id(), created.value().id() );
        EXPECT_TRUE( rooms.front().parent_space_id().empty() );
        EXPECT_TRUE( store.DerivedJoinedTopics().empty() );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief CORE-02/D-04: a room inside an autoJoin space derives the
     *        gcs/chat/<room-id> joined topic at creation.
     */
    TEST_F( GcsEntitiesTest, CreateRoomInAutoJoinSpaceIsDerivedJoined )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( gcs::test::WaitForCondition( [&dbPath]() { return std::filesystem::exists( dbPath ); },
                                                  gcs::test::kWaitTimeout ) );

        gcs::EntityStore store( session );
        ASSERT_TRUE( store.LoadFromStore().has_value() );

        auto space = store.CreateSpace( "ops", true, true );
        ASSERT_TRUE( space.has_value() );
        auto room = store.CreateRoom( "general", space.value().id() );
        ASSERT_TRUE( room.has_value() );

        const auto topics = store.DerivedJoinedTopics();
        ASSERT_EQ( topics.size(), 1U );
        EXPECT_EQ( topics.front(), std::string( kRoomTopicPrefix ) + room.value().id() );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief CORE-02: a room with a non-empty unknown parent is rejected
     *        without writing anything.
     */
    TEST_F( GcsEntitiesTest, CreateRoomWithUnknownParentFails )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( gcs::test::WaitForCondition( [&dbPath]() { return std::filesystem::exists( dbPath ); },
                                                  gcs::test::kWaitTimeout ) );

        gcs::EntityStore store( session );
        ASSERT_TRUE( store.LoadFromStore().has_value() );

        auto created = store.CreateRoom( "x", "space-does-not-exist" );
        ASSERT_FALSE( created.has_value() );
        EXPECT_TRUE( store.Rooms().empty() );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief CORE-03: toggling autoJoinRooms retroactively evaporates and
     *        restores the derived joins of EXISTING rooms (D-04), while
     *        UpdateSpace preserves the immutable created_at_ms stamp.
     */
    TEST_F( GcsEntitiesTest, UpdateSpaceTogglesDerivedJoinsRetroactively )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( gcs::test::WaitForCondition( [&dbPath]() { return std::filesystem::exists( dbPath ); },
                                                  gcs::test::kWaitTimeout ) );

        gcs::EntityStore store( session );
        ASSERT_TRUE( store.LoadFromStore().has_value() );

        auto space = store.CreateSpace( "ops", true, false ); // auto-join OFF
        ASSERT_TRUE( space.has_value() );
        auto room = store.CreateRoom( "general", space.value().id() );
        ASSERT_TRUE( room.has_value() );
        EXPECT_TRUE( store.DerivedJoinedTopics().empty() );

        const std::string expectedTopic = std::string( kRoomTopicPrefix ) + room.value().id();

        // FFI-shaped desired record: id/name/flags only (D-27) — the store must
        // preserve the immutable created_at_ms from the stored record.
        gcs::SpaceRecord desired;
        desired.set_id( space.value().id() );
        desired.set_name( "ops" );
        desired.set_is_public( true );
        desired.set_auto_join_rooms( true );
        auto toggledOn = store.UpdateSpace( desired );
        ASSERT_TRUE( toggledOn.has_value() );
        EXPECT_EQ( toggledOn.value().created_at_ms(), space.value().created_at_ms() );
        ASSERT_EQ( store.DerivedJoinedTopics().size(), 1U );
        EXPECT_EQ( store.DerivedJoinedTopics().front(), expectedTopic );

        desired.set_auto_join_rooms( false );
        ASSERT_TRUE( store.UpdateSpace( desired ).has_value() );
        EXPECT_TRUE( store.DerivedJoinedTopics().empty() );

        desired.set_auto_join_rooms( true );
        ASSERT_TRUE( store.UpdateSpace( desired ).has_value() );
        ASSERT_EQ( store.DerivedJoinedTopics().size(), 1U );
        EXPECT_EQ( store.DerivedJoinedTopics().front(), expectedTopic );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief D-03: tombstoned records written directly to the CRDT store are
     *        kept by LoadFromStore but skipped by every reader — Spaces(),
     *        Rooms(), DerivedJoinedTopics(), IsValidParentSpace().
     */
    TEST_F( GcsEntitiesTest, TombstonedRecordsAreSkipped )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( gcs::test::WaitForCondition( [&dbPath]() { return std::filesystem::exists( dbPath ); },
                                                  gcs::test::kWaitTimeout ) );

        // Direct record writes (tombstone bytes never come from Phase 2
        // commands — D-03; tests write them to prove reader-skip semantics).
        gcs::chat::SpaceRecord tombSpace;
        tombSpace.set_id( "space-tomb" );
        tombSpace.set_name( "gone" );
        tombSpace.set_auto_join_rooms( true );
        tombSpace.set_created_at_ms( kTombstoneAtMs );
        tombSpace.set_updated_at_ms( kTombstoneAtMs );
        tombSpace.set_deleted( true );
        tombSpace.set_deleted_at_ms( kTombstoneAtMs );

        gcs::chat::RoomRecord tombRoom;
        tombRoom.set_id( "room-tomb" );
        tombRoom.set_name( "gone-room" );
        tombRoom.set_created_at_ms( kTombstoneAtMs );
        tombRoom.set_updated_at_ms( kTombstoneAtMs );
        tombRoom.set_deleted( true );
        tombRoom.set_deleted_at_ms( kTombstoneAtMs );

        // A LIVE room parented to the tombstoned space: it stays visible as a
        // catalog record but derives no join (parent tombstoned).
        gcs::chat::RoomRecord liveRoom;
        liveRoom.set_id( "room-live" );
        liveRoom.set_name( "orphaned-view" );
        liveRoom.set_parent_space_id( "space-tomb" );
        liveRoom.set_created_at_ms( kTombstoneAtMs );
        liveRoom.set_updated_at_ms( kTombstoneAtMs );

        gcs::chat::EntityManifest manifest;
        manifest.add_space_id( "space-tomb" );
        manifest.add_room_id( "room-tomb" );
        manifest.add_room_id( "room-live" );

        ASSERT_TRUE( session.Put( std::string( kSpacesKeyPrefix ) + "space-tomb",
                                  tombSpace.SerializeAsString() )
                         .has_value() );
        ASSERT_TRUE( session.Put( std::string( kRoomsKeyPrefix ) + "room-tomb",
                                  tombRoom.SerializeAsString() )
                         .has_value() );
        ASSERT_TRUE( session.Put( std::string( kRoomsKeyPrefix ) + "room-live",
                                  liveRoom.SerializeAsString() )
                         .has_value() );
        ASSERT_TRUE( session.Put( kManifestKey, manifest.SerializeAsString() ).has_value() );

        gcs::EntityStore reloaded( session );
        ASSERT_TRUE( reloaded.LoadFromStore().has_value() );
        EXPECT_TRUE( reloaded.Spaces().empty() ); // tombstoned space skipped
        const auto rooms = reloaded.Rooms();
        ASSERT_EQ( rooms.size(), 1U ); // tombstoned room skipped, live room kept
        EXPECT_EQ( rooms.front().id(), "room-live" );
        EXPECT_TRUE( reloaded.DerivedJoinedTopics().empty() ); // parent tombstoned
        EXPECT_FALSE( reloaded.IsValidParentSpace( "space-tomb" ) );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Restart criterion: entities created in session A survive into
     *        session B on the same db_path (manifest + N+1 record reads).
     *        Session A is shut down before B is constructed (no two live
     *        handles); one shared pubsub serves both.
     */
    TEST_F( GcsEntitiesTest, EntitiesSurviveSessionRestartOnSharedDb )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        const std::string dbPath = m_tempPath + "/db";
        std::string       spaceId;
        std::string       roomId;
        {
            gcs::CoreSession::Config cfg{};
            cfg.m_dbPath = dbPath;
            gcs::CoreSession sessionA( cfg );
            ASSERT_TRUE( sessionA.Initialize( pubsub, graphsync.network ).has_value() );
            EXPECT_TRUE( gcs::test::WaitForCondition(
                [&dbPath]() { return std::filesystem::exists( dbPath ); }, gcs::test::kWaitTimeout ) );

            gcs::EntityStore storeA( sessionA );
            ASSERT_TRUE( storeA.LoadFromStore().has_value() );
            auto space = storeA.CreateSpace( "ops", true, true );
            ASSERT_TRUE( space.has_value() );
            auto room = storeA.CreateRoom( "general", space.value().id() );
            ASSERT_TRUE( room.has_value() );
            spaceId = space.value().id();
            roomId  = room.value().id();
            sessionA.Shutdown();
        }
        {
            gcs::CoreSession::Config cfg{};
            cfg.m_dbPath = dbPath;
            gcs::CoreSession sessionB( cfg );
            ASSERT_TRUE( sessionB.Initialize( pubsub, graphsync.network ).has_value() );

            gcs::EntityStore storeB( sessionB );
            ASSERT_TRUE( storeB.LoadFromStore().has_value() ); // manifest -> N+1 Gets

            const auto spaces = storeB.Spaces();
            ASSERT_EQ( spaces.size(), 1U );
            EXPECT_EQ( spaces.front().id(), spaceId );
            EXPECT_EQ( spaces.front().name(), "ops" );

            const auto rooms = storeB.Rooms();
            ASSERT_EQ( rooms.size(), 1U );
            EXPECT_EQ( rooms.front().id(), roomId );
            EXPECT_EQ( rooms.front().parent_space_id(), spaceId );

            const auto topics = storeB.DerivedJoinedTopics();
            ASSERT_EQ( topics.size(), 1U );
            EXPECT_EQ( topics.front(), std::string( kRoomTopicPrefix ) + roomId );

            sessionB.Shutdown();
        }
        pubsub->Stop();
    }

} // namespace gcs::test
