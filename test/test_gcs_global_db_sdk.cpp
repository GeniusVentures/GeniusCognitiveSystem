/**
 * @file       test_gcs_global_db_sdk.cpp
 * @brief      SDK-wiring regression test for the 2026-08-27 graphsync
 *             handler-clobber bug: GcsGlobalDb::Initialize() must BORROW the
 *             node's graphsync Network (GeniusNode::GetGraphsyncNetwork()),
 *             never construct a second one on the node's host.
 * @details    Lives in its own binary on purpose: GeniusSDKInit/GeniusSDKShutdown
 *             toggle a process-global node, which would break the option-C
 *             tests in test_gcs_global_db (they assert GeniusSDKGetNode() is
 *             nullptr in that binary). Booting the full node is the
 *             documented-flaky path (RESEARCH Q-01) — the test SKIPS (option-C
 *             pattern) when the SDK cannot boot in this environment, and
 *             asserts the wiring when it can. No raw thread sleeps.
 * @date       2026-08-27
 */

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include <libp2p/log/configurator.hpp>
#include <libp2p/log/logger.hpp>

#include <soralog/impl/configurator_from_yaml.hpp>
#include <soralog/logging_system.hpp>

#include "gcs_storage/gcs_global_db.hpp"

#include "GeniusSDK.hpp"

namespace
{
    /// Minimal soralog YAML — console sink, error level (mirrors
    /// test_gcs_global_db.cpp; required before any SuperGenius logger exists).
    constexpr const char *kLoggingYaml = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_global_db_sdk_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";

    /// Dev config accepted by GeniusSDKInit's parser (Address/Cut/TokenValue/
    /// TokenID — presence+string-checked; offline-safe placeholder values).
    constexpr const char *kDevConfig = R"(
     {
       "Address": "0x0000000000000000000000000000000000000001",
       "Cut": "100",
       "TokenValue": "1000",
       "TokenID": "0x0000000000000000000000000000000000000000000000000000000000000001"
     }
    )";
} // namespace

namespace sgns::neoswarm::storage::test
{
    /**
     * @brief Fixture: per-test temp base_path for the SDK's wallet/db files,
     *        removed in TearDown; one-time soralog init for the node's loggers.
     */
    class GcsGlobalDbSdkTest : public ::testing::Test
    {
    protected:
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
                           / ( std::string{ "gcs_global_db_sdk_" } + info->name() + "_" +
                               std::to_string( uniqueSalt ) ) )
                             .string();
            std::filesystem::create_directories( m_tempPath );
        }

        void TearDown() override
        {
            std::error_code ec;
            std::filesystem::remove_all( m_tempPath, ec );
        }

        std::string m_tempPath; ///< Per-test temp directory for SDK files
    };

    /**
     * @brief Regression (2026-08-27): after the SDK-wired Initialize(), GCS
     *        holds the NODE's graphsync Network instance — pointer equality
     *        against GeniusNode::GetGraphsyncNetwork() proves we never
     *        re-register the /ipfs/graphsync/1.0.0 protocol handler (a libp2p
     *        host has one handler slot per protocol; a locally constructed
     *        Network would silently kill the node's inbound graphsync).
     */
    TEST_F( GcsGlobalDbSdkTest, InitializeBorrowsNodeGraphsyncNetwork )
    {
        const char *initPath = GeniusSDKInit( m_tempPath.c_str(), kDevConfig );
        if ( initPath == nullptr )
        {
            GTEST_SKIP() << "GeniusSDKInit could not boot a node in this environment "
                            "(option C — wiring not provable here)";
        }

        auto node = GeniusSDKGetNode();
        ASSERT_NE( node, nullptr );
        ASSERT_NE( node->GetGraphsyncNetwork(), nullptr );

        GcsGlobalDb::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        GcsGlobalDb db( cfg );

        auto res = db.Initialize();
        ASSERT_TRUE( res.has_value() ) << "SDK is up but GcsGlobalDb::Initialize failed";
        EXPECT_EQ( db.GraphsyncNetwork().get(), node->GetGraphsyncNetwork().get() )
            << "GCS constructed its own graphsync Network — the node's inbound "
               "graphsync handler was clobbered";

        db.Shutdown();
        GeniusSDKShutdown();
    }
} // namespace sgns::neoswarm::storage::test
