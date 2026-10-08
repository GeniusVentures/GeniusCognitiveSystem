/**
 * @file       test_gcs_core_smoke.cpp
 * @brief      CORE-05 substrate smoke tests for gcs::CoreSession.
 * @details    Phase 1 smoke tests for the gcs_core session: GlobalDB lifecycle via
 *             the injected-pubsub seam, AddBroadcastTopic+AddListenTopic on a
 *             gcs/chat/<roomname> topic, and a Put->Get round-trip over a real
 *             GossipPubSub on port 0 (ephemeral). Mirrors the structure of the
 *             moved test_gcs_global_db.cpp (soralog one-time setup, per-test
 *             temp dirs, KeyPairFileStorage key bring-up). Uses the GCS
 *             wait-condition template (condition_variable polling — never a
 *             raw thread sleep). The production Initialize() path
 *             (GeniusSDKGetNode) is exercised by the FFI test binary, not
 *             here — CI never brings a GeniusNode online.
 * @date       2026-08-15
 */

#include "lib/gcs_core.hpp"
#include "test_graphsync_network.hpp"
#include "test_wait_condition.hpp"

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
       - name: gcs_core_smoke_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";

    /// Smoke topic — follows the architecture convention gcs/chat/<roomname>.
    constexpr const char *kSmokeTopic = "gcs/chat/smoke-test";
} // namespace

namespace gcs::test
{
    /**
     * @brief Fixture that owns a per-test temp directory under
     *        std::filesystem::temp_directory_path() and removes it in TearDown.
     */
    class GcsCoreSmokeTest : public ::testing::Test
    {
    protected:
        /**
         * @brief One-time logging-system init — SuperGenius's base::createLogger asserts
         *        a configured soralog LoggingSystem; without it GossipPubSub construction
         *        crashes with "Logging system is not ready". Mirrors the moved
         *        test_gcs_global_db.cpp SetUpTestSuite.
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
                           / ( std::string{ "gcs_core_smoke_" } + info->name() + "_" +
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
         *        Mirrors the moved test_gcs_global_db.cpp bring-up recipe.
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

        std::string m_tempPath; ///< Per-test temp directory (created in SetUp, removed in TearDown)
    };

    /**
     * @brief Full CoreSession lifecycle with an injected pubsub: Initialize succeeds,
     *        IsRunning() becomes true, the db directory appears on disk, and
     *        Shutdown() returns the session to a stopped state before the pubsub
     *        is stopped (shutdown ordering: session first, then pubsub).
     */
    TEST_F( GcsCoreSmokeTest, LifecycleWithInjectedPubSub )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );

        auto res = session.Initialize( pubsub, graphsync.network );
        ASSERT_TRUE( res.has_value() );
        EXPECT_TRUE( session.IsRunning() );

        // The db directory may be created lazily — wait for it via the wait-condition template.
        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( gcs::test::WaitForCondition( [&dbPath]() { return std::filesystem::exists( dbPath ); },
                                                  gcs::test::kWaitTimeout ) );

        session.Shutdown();
        EXPECT_FALSE( session.IsRunning() );

        pubsub->Stop();
    }

    /**
     * @brief CORE-05 substrate proof: broadcast + listen topics join on a
     *        gcs/chat/<roomname> topic and a Put->Get round-trip through the
     *        CRDT store returns the stored value.
     */
    TEST_F( GcsCoreSmokeTest, CrdtPutGetRoundTripsOnGcsChatTopic )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );
        EXPECT_TRUE( session.IsRunning() );

        // The db directory may be created lazily — wait for it via the wait-condition
        // template before exercising the store.
        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( gcs::test::WaitForCondition( [&dbPath]() { return std::filesystem::exists( dbPath ); },
                                                  gcs::test::kWaitTimeout ) );

        // Topic name follows architecture convention gcs/chat/<roomname>; GossipSub
        // topics are implicit — "join" = broadcast + listen registration.
        ASSERT_TRUE( session.AddBroadcastTopic( kSmokeTopic ).has_value() );
        ASSERT_TRUE( session.AddListenTopic( kSmokeTopic ).has_value() );

        // Put->Get round-trip via the GcsGlobalDb pass-through accessors.
        const std::string key   = "smoke-key";
        const std::string value = "smoke-value";
        ASSERT_TRUE( session.Put( key, value ).has_value() );

        auto getResult = session.Get( key );
        ASSERT_TRUE( getResult.has_value() );
        EXPECT_EQ( getResult.value(), value );

        session.Shutdown();
        EXPECT_FALSE( session.IsRunning() );

        pubsub->Stop();
    }

} // namespace gcs::test
