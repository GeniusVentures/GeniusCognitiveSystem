/**
 * @file       test_gcs_global_db.cpp
 * @brief      Lifecycle unit tests for the GCS GlobalDB component (Phase 3, Task 3).
 *             Uses a real GossipPubSub on port 0 (Tier 2 fixture pattern from
 *             SuperGenius/test/src/crdt/globaldb_integration.cpp) and the NEO-SWARM
 *             wait-condition template (condition_variable polling — no
 *             std::this_thread sleeps).
 * @date       2026-08-10
 */

#include "gcs_storage/gcs_global_db.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "crdt/globaldb/keypair_file_storage.hpp"

#include "ipfs_pubsub/gossip_pubsub.hpp"

#include <libp2p/log/configurator.hpp>
#include <libp2p/log/logger.hpp>

#include <soralog/impl/configurator_from_yaml.hpp>
#include <soralog/logging_system.hpp>

#include "test_graphsync_network.hpp"
#include "test_wait_condition.hpp"

namespace
{
    /// GossipPubSub bind address used by every test.
    constexpr const char *kListenIp = "0.0.0.0";

    /**
     * @brief Minimal soralog YAML — console sink only, error level, sufficient to satisfy
     *        libp2p::log::setLoggingSystem() before any SuperGenius logger is constructed.
     */
    constexpr const char *kLoggingYaml = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_global_db_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";
} // namespace

namespace sgns::neoswarm::storage::test
{
    // Shared wait-condition template (test_wait_condition.hpp) — symbols live
    // in ::gcs::test (leading-global: a bare gcs:: here resolves to sgns::gcs);
    // alias them so the unqualified call sites keep working.
    using ::gcs::test::WaitForCondition;
    using ::gcs::test::kWaitTimeout;
    /**
     * @brief Fixture that owns a per-test temp directory under
     *        std::filesystem::temp_directory_path() and removes it in TearDown.
     */
    class GcsGlobalDbTest : public ::testing::Test
    {
    protected:
        /**
         * @brief One-time logging-system init — SuperGenius's base::createLogger asserts
         *        a configured soralog LoggingSystem; without it GossipPubSub construction
         *        crashes with "Logging system is not ready". Mirrors the SuperGenius
         *        globaldb_integration.cpp SetUpTestSuite.
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
                           / ( std::string{ "gcs_global_db_" } + info->name() + "_" +
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
         *        Mirrors globaldb_integration.cpp:93-99.
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
            auto pubsub = std::make_shared<sgns::ipfs_pubsub::GossipPubSub>( keyPairResult.value() );
            auto startFuture = pubsub->Start( 0, {}, kListenIp, {} );
            auto startError  = startFuture.get();
            EXPECT_FALSE( startError ) << "Could not start GossipPubSub: " << startError.message();
            if ( startError )
            {
                return nullptr;
            }
            return pubsub;
        }

        std::string m_tempPath; ///< Per-test temp directory (created in SetUp, removed in TearDown)
    };

    /**
     * @brief Default-constructed component: inert, IsRunning() false, Shutdown() a safe no-op.
     */
    TEST_F( GcsGlobalDbTest, DefaultConstructedIsNotRunning )
    {
        GcsGlobalDb::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        GcsGlobalDb db( cfg );

        EXPECT_FALSE( db.IsRunning() );
        db.Shutdown(); // must not crash, must not throw
        EXPECT_FALSE( db.IsRunning() );
    }

    /**
     * @brief Initialize() without GeniusSDK initialized must fail with the specific
     *        Error::SdkNotInitialized code. This binary never calls GeniusSDKInit, so
     *        GeniusSDKGetNode() returns nullptr throughout the test.
     */
    TEST_F( GcsGlobalDbTest, InitializeWithoutSdkFails )
    {
        GcsGlobalDb::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        GcsGlobalDb db( cfg );

        auto res = db.Initialize();
        ASSERT_FALSE( res.has_value() );
        EXPECT_EQ( res.error(), Error::SdkNotInitialized );
        EXPECT_FALSE( db.IsRunning() );
    }

    /**
     * @brief Full lifecycle with an injected pubsub: GlobalDB::New succeeds, Start is
     *        called, IsRunning() becomes true, the db directory appears on disk, and
     *        Shutdown() returns the component to a stopped state with the io thread
     *        joined.
     */
    TEST_F( GcsGlobalDbTest, LifecycleWithInjectedPubSub )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = ::gcs::test::MakeGraphsyncContext( pubsub );

        GcsGlobalDb::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        GcsGlobalDb db( cfg );

        auto res = db.Initialize( pubsub, graphsync.network );
        ASSERT_TRUE( res.has_value() );
        EXPECT_TRUE( db.IsRunning() );

        // The db directory may be created lazily — wait for it via the wait-condition template.
        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( WaitForCondition( [&dbPath]() { return std::filesystem::exists( dbPath ); },
                                       kWaitTimeout ) );

        db.Shutdown();
        EXPECT_FALSE( db.IsRunning() );

        pubsub->Stop();
    }

    /**
     * @brief Double Initialize() must fail the second call with Error::GcsDbError
     *        (programmer error — surfaced, not silently swallowed).
     */
    TEST_F( GcsGlobalDbTest, DoubleInitializeFails )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = ::gcs::test::MakeGraphsyncContext( pubsub );

        GcsGlobalDb::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        GcsGlobalDb db( cfg );

        auto first = db.Initialize( pubsub, graphsync.network );
        ASSERT_TRUE( first.has_value() );
        EXPECT_TRUE( db.IsRunning() );

        auto second = db.Initialize( pubsub, graphsync.network );
        ASSERT_FALSE( second.has_value() );
        EXPECT_EQ( second.error(), Error::GcsDbError );

        db.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Regression (2026-08-27 handler-clobber bug): the injected graphsync
     *        Network must be BORROWED, never re-constructed. A libp2p host keeps
     *        one protocol-handler slot per protocol — a locally constructed
     *        Network would silently replace the injector's registration. Pointer
     *        equality against the injected instance proves no re-registration.
     */
    TEST_F( GcsGlobalDbTest, InjectedGraphsyncNetworkIsBorrowedNotReplaced )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = ::gcs::test::MakeGraphsyncContext( pubsub );

        GcsGlobalDb::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        GcsGlobalDb db( cfg );

        ASSERT_TRUE( db.GraphsyncNetwork() == nullptr );

        auto res = db.Initialize( pubsub, graphsync.network );
        ASSERT_TRUE( res.has_value() );
        EXPECT_EQ( db.GraphsyncNetwork().get(), graphsync.network.get() );

        db.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Widened storage surface round-trips (03-02): topics-aware Put,
     *        2-arg Put, PutLocal, QueryKeyValues prefix scan, and raw GossipSub
     *        Publish/Subscribe all succeed on a started injected pubsub. Local
     *        loopback delivery is intentionally NOT asserted here — the injected
     *        pubsub uses echo_forward_mode=false, so full-value delivery is
     *        asserted at the component layer (03-04) and live cross-node (03-06).
     */
    TEST_F( GcsGlobalDbTest, WidenedStorageSurfaceRoundTrips )
    {
        constexpr const char *kRoomA               = "roomA";
        constexpr const char *kRoomB               = "roomB";
        constexpr const char *kPrefixA             = "gcs/messages/roomA/";
        constexpr const char *kPrefixB             = "gcs/messages/roomB/";
        constexpr const char *kPayloadOne          = "hello from one";
        constexpr const char *kPayloadTwo          = "hello from two";
        constexpr const char *kPayloadThree        = "hello from three";
        constexpr size_t      kRoomAInitialEntries = 2;

        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = ::gcs::test::MakeGraphsyncContext( pubsub );

        GcsGlobalDb::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        GcsGlobalDb db( cfg );

        auto initRes = db.Initialize( pubsub, graphsync.network );
        ASSERT_TRUE( initRes.has_value() );
        EXPECT_TRUE( db.IsRunning() );

        const std::string kKeyOne = std::string{ kPrefixA } + "msg-1";
        const std::string kKeyTwo = std::string{ kPrefixA } + "msg-2";

        // QueryKeyValues returns RAW datastore keys (/crdt/k/<key>/v), not the
        // logical key — match on the embedded message id instead of the logical
        // key (which 03-04 parses out of the raw key). Returns (found, value).
        const auto valueForKeyFragment =
            []( const std::vector<std::pair<std::string, std::string>> &entries,
                const std::string &fragment ) {
                for ( const auto &entry : entries )
                {
                    if ( entry.first.find( fragment ) != std::string::npos )
                    {
                        return std::make_pair( true, entry.second );
                    }
                }
                return std::make_pair( false, std::string{} );
            };

        // Topics-aware Put + 2-arg Put both succeed.
        EXPECT_TRUE( db.Put( kKeyOne, kPayloadOne, { kRoomA } ).has_value() );
        EXPECT_TRUE( db.Put( kKeyTwo, kPayloadTwo ).has_value() );

        // Prefix scan returns exactly the two roomA entries with their values.
        auto       scanA    = db.QueryKeyValues( kPrefixA );
        ASSERT_TRUE( scanA.has_value() );
        const auto &entriesA = scanA.value();
        ASSERT_EQ( entriesA.size(), kRoomAInitialEntries );

        const auto foundOne = valueForKeyFragment( entriesA, "msg-1" );
        EXPECT_TRUE( foundOne.first );
        EXPECT_EQ( foundOne.second, kPayloadOne );
        const auto foundTwo = valueForKeyFragment( entriesA, "msg-2" );
        EXPECT_TRUE( foundTwo.first );
        EXPECT_EQ( foundTwo.second, kPayloadTwo );

        // roomB prefix returns empty.
        auto scanB = db.QueryKeyValues( kPrefixB );
        ASSERT_TRUE( scanB.has_value() );
        EXPECT_TRUE( scanB.value().empty() );

        // PutLocal overwrites an EXISTING key (SuperGenius contract: local
        // side-effect writes bypass DAG broadcast and re-derive an existing
        // record — a fresh key has no priority record, so PutLocal on a fresh
        // key is rejected). Overwrite msg-1 and confirm the scan reflects it.
        EXPECT_TRUE( db.PutLocal( kKeyOne, kPayloadThree, "local-rewrite" ).has_value() );
        auto scanA2 = db.QueryKeyValues( kPrefixA );
        ASSERT_TRUE( scanA2.has_value() );
        const auto &entriesA2 = scanA2.value();
        ASSERT_EQ( entriesA2.size(), kRoomAInitialEntries );
        const auto rewrittenOne = valueForKeyFragment( entriesA2, "msg-1" );
        EXPECT_TRUE( rewrittenOne.first );
        EXPECT_EQ( rewrittenOne.second, kPayloadThree );
        const auto unchangedTwo = valueForKeyFragment( entriesA2, "msg-2" );
        EXPECT_TRUE( unchangedTwo.first );
        EXPECT_EQ( unchangedTwo.second, kPayloadTwo );

        // Raw Publish + Subscribe succeed on the injected (started) pubsub.
        EXPECT_TRUE( db.Publish( kRoomA, kPayloadOne ).has_value() );
        EXPECT_TRUE( db.Subscribe(
                         kRoomA,
                         []( const std::string & /*topic*/,
                             const std::string & /*data*/ ) {
                             // Loopback delivery intentionally not asserted here.
                         } )
                         .has_value() );

        db.Shutdown();
        EXPECT_FALSE( db.IsRunning() );

        pubsub->Stop();
    }

} // namespace sgns::neoswarm::storage::test
