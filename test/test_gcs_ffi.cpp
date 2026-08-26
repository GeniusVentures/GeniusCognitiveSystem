/**
 * @file       test_gcs_ffi.cpp
 * @brief      Smoke tests for the gcs_ffi four-function C ABI (D-27/D-29).
 * @details    Exercises the ABI exactly as Dart will: gcs_init over serialized
 *             gcs.chat.GcsConfig bytes (codec-tagged per D-29), argument
 *             validation on gcs_publish/gcs_subscribe, and null-safe shutdown.
 *
 *             OPTION C CONTRACT (locked at planning time): this test binary
 *             never initializes GeniusSDK, so GeniusSDKGetNode() returns
 *             nullptr for every gcs_init call with a parseable PROTOBUF
 *             config — gcs_init must return nullptr GRACEFULLY (no crash, no
 *             exception). No test asserts a non-null handle. The
 *             init-success + live subscribe + command-publish round-trip is
 *             OWNED BY PLAN 05 (Dart spike), which runs against a real node.
 *
 *             Shutdown discipline: TearDown conditionally calls gcs_shutdown
 *             on any non-null handle — the GcsGlobalDb destructor touches
 *             asio::io_context internals and, if left to static destruction
 *             after __cxa_finalize, a "pthread lock: Invalid argument" abort
 *             fires at process exit (same reasoning as the GeniusElm FFI test).
 * @date       2026-08-26
 */

#include "ffi/gcs_core.h"
#include "proto/gcs_chat.pb.h"

#include <chrono>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

namespace
{
    /// Parse-failure config input — field-number/wire-type bytes protobuf rejects.
    constexpr const char kGarbageConfig[] = "\xff\xff\xff\xff";
    /// Dart -> C++ ingress topic (D-27: commands are topic publishes).
    constexpr const char *kCommandTopic = "gcs/command";
    /// Event-stream topic used by the subscribe argument-validation cases.
    constexpr const char *kEventTopic = "gcs/event";
    /// Arbitrary non-zero Dart NativePort id for subscribe validation.
    constexpr int64_t kTestDartPort = 1234;
    /// Number of gcs_init calls in the repeated-init case.
    constexpr int kRepeatedInitCallCount = 3;
} // namespace

namespace gcs::test
{
    /**
     * @brief Fixture owning a per-test temp directory (for GcsConfig db paths)
     *        and the session handle under test.
     */
    class GcsFFI : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            const auto *info       = ::testing::UnitTest::GetInstance()->current_test_info();
            const auto  uniqueSalt = std::chrono::steady_clock::now().time_since_epoch().count();
            m_tempPath = ( std::filesystem::temp_directory_path()
                           / ( std::string{ "gcs_ffi_" } + info->name() + "_" + std::to_string( uniqueSalt ) ) )
                             .string();
            std::filesystem::create_directories( m_tempPath );
        }

        void TearDown() override
        {
            // Destroy the C++ session before GTest global teardown — see file header.
            if ( m_handle != nullptr )
            {
                gcs_shutdown( m_handle );
                m_handle = nullptr;
            }
            std::error_code ec;
            std::filesystem::remove_all( m_tempPath, ec );
        }

        /**
         * @brief Serializes a GcsConfig (D-29 codec-tagged init bytes).
         *
         * @param[in] dbPath RocksDB path carried by the config.
         * @param[in] codec  Wire codec bound to the store at creation.
         * @return The serialized protobuf bytes; the caller passes .data()/.size().
         */
        std::string MakeConfigBytes( const std::string &dbPath, gcs::chat::Codec codec )
        {
            gcs::chat::GcsConfig config;
            config.set_db_path( dbPath );
            config.set_codec( codec );
            return config.SerializeAsString();
        }

        GcsSession *m_handle = nullptr; ///< Session handle under test (null in every option-C case)

        std::string m_tempPath; ///< Per-test temp directory (created in SetUp, removed in TearDown)
    };

    /**
     * @brief A parseable PROTOBUF config reaches CoreSession::Initialize(), whose
     *        GeniusSDKGetNode() returns nullptr in this binary — gcs_init must
     *        return nullptr gracefully (option C: no crash, no exception).
     */
    TEST_F( GcsFFI, InitWithValidConfigWithoutNodeReturnsNullptrGracefully )
    {
        const std::string bytes = MakeConfigBytes( m_tempPath + "/db", gcs::chat::CODEC_PROTOBUF );
        m_handle = gcs_init( reinterpret_cast<const uint8_t *>( bytes.data() ), bytes.size() );
        EXPECT_EQ( m_handle, nullptr );
    }

    /**
     * @brief Null config bytes and zero-length configs are invalid arguments —
     *        both must return nullptr without touching the bytes.
     */
    TEST_F( GcsFFI, InitRejectsNullConfigBytes )
    {
        EXPECT_EQ( gcs_init( nullptr, sizeof( kGarbageConfig ) ), nullptr );

        // data() on an empty string is a valid non-null pointer — isolates the
        // zero-length rejection from the null-pointer rejection above.
        const std::string emptyBytes;
        EXPECT_EQ( gcs_init( reinterpret_cast<const uint8_t *>( emptyBytes.data() ), emptyBytes.size() ), nullptr );
    }

    /**
     * @brief Garbage (unparseable) config bytes must never reach a partially
     *        parsed state — gcs_init returns nullptr.
     */
    TEST_F( GcsFFI, InitRejectsGarbageConfigBytes )
    {
        EXPECT_EQ( gcs_init( reinterpret_cast<const uint8_t *>( kGarbageConfig ), sizeof( kGarbageConfig ) - 1 ),
                   nullptr );
    }

    /**
     * @brief A parseable config with CODEC_JSON parses but violates the per-store
     *        codec binding (D-29: GCS Phase 1 is PROTOBUF only) — gcs_init returns
     *        nullptr.
     */
    TEST_F( GcsFFI, InitRejectsUnsupportedCodec )
    {
        const std::string bytes = MakeConfigBytes( m_tempPath + "/db", gcs::chat::CODEC_JSON );
        EXPECT_EQ( gcs_init( reinterpret_cast<const uint8_t *>( bytes.data() ), bytes.size() ), nullptr );
    }

    /**
     * @brief Repeated gcs_init calls with valid configs all fail identically and
     *        leave the ABI in a callable state (no session ever exists).
     */
    TEST_F( GcsFFI, RepeatedInitCallsWithoutNodeAllReturnNullptr )
    {
        const std::string bytes = MakeConfigBytes( m_tempPath + "/db", gcs::chat::CODEC_PROTOBUF );
        for ( int callIndex = 0; callIndex < kRepeatedInitCallCount; ++callIndex )
        {
            EXPECT_EQ( gcs_init( reinterpret_cast<const uint8_t *>( bytes.data() ), bytes.size() ), nullptr );
        }
    }

    /**
     * @brief gcs_shutdown(nullptr) is a safe no-op (null-tolerant teardown).
     */
    TEST_F( GcsFFI, ShutdownNullptrIsSafeNoOp )
    {
        gcs_shutdown( nullptr );
        SUCCEED();
    }

    /**
     * @brief gcs_publish validates its arguments before anything else — with no
     *        session alive, every null/malformed combination returns
     *        GCS_ERROR_INVALID_ARGUMENT.
     */
    TEST_F( GcsFFI, PublishValidatesArgumentsWithoutNode )
    {
        const std::string bytes = MakeConfigBytes( m_tempPath + "/db", gcs::chat::CODEC_PROTOBUF );
        EXPECT_EQ( gcs_publish( nullptr, kCommandTopic, reinterpret_cast<const uint8_t *>( bytes.data() ),
                                bytes.size() ),
                   GCS_ERROR_INVALID_ARGUMENT );
        EXPECT_EQ( gcs_publish( nullptr, nullptr, reinterpret_cast<const uint8_t *>( bytes.data() ), bytes.size() ),
                   GCS_ERROR_INVALID_ARGUMENT );
        EXPECT_EQ( gcs_publish( nullptr, kCommandTopic, nullptr, 0 ), GCS_ERROR_INVALID_ARGUMENT );
    }

    /**
     * @brief gcs_subscribe validates its arguments — null session/topic and a
     *        zero port all return GCS_ERROR_INVALID_ARGUMENT.
     */
    TEST_F( GcsFFI, SubscribeValidatesArgumentsWithoutNode )
    {
        EXPECT_EQ( gcs_subscribe( nullptr, kEventTopic, kTestDartPort ), GCS_ERROR_INVALID_ARGUMENT );
        EXPECT_EQ( gcs_subscribe( nullptr, nullptr, kTestDartPort ), GCS_ERROR_INVALID_ARGUMENT );
        EXPECT_EQ( gcs_subscribe( nullptr, kEventTopic, 0 ), GCS_ERROR_INVALID_ARGUMENT );
    }

} // namespace gcs::test
