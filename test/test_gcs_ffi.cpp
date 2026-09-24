/**
 * @file       test_gcs_ffi.cpp
 * @brief      Smoke tests for the gcs_ffi four-function C ABI (D-27/D-29).
 * @details    Exercises the ABI exactly as Dart will: gcs_init over serialized
 *             gcs.chat.GcsConfig bytes (codec-tagged per D-29), argument
 *             validation on gcs_publish/gcs_subscribe, and null-safe shutdown.
 *
 *             EMBEDDED-NODE CONTRACT (2026-09-19 live-app fix): gcs_init
 *             boots the embedded GeniusSDK node itself when none exists in
 *             the process (empty mnemonic = child-wallet contract: reuse the
 *             wallet persisted under the base path, create one when none
 *             exists), so a valid config yields a live handle even with no
 *             external boot; gcs_shutdown pairs that internal boot with node
 *             teardown. The null-return failure classes below are all
 *             rejected BEFORE the boot (null/empty/garbage bytes,
 *             unsupported codec).
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
#include <cstdint>
#include <filesystem>
#include <fstream>
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
    /// Pinned pubsub listen port for the embedded node, written to
    /// network_config.json under the node base path in SetUp (the config db
    /// path is <tmp>/db, so the base path is the tmp dir). GeniusNode derives
    /// ports as 40001 + hash%301 with no availability probe, so parallel node
    /// processes can collide on a derived port (observed in SuperGenius CI —
    /// child_registration.cpp); the pin sits OUTSIDE the derived range.
    constexpr uint16_t kPinnedPubsubPort = 41500;
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

            // Pin the embedded node's pubsub port (child_registration.cpp
            // pattern): the node reads this file at InitNetwork.
            std::ofstream networkConfig( m_tempPath + "/network_config.json" );
            networkConfig << "{ \"port_seed\": " << kPinnedPubsubPort
                          << ", \"auto_dht\": false, \"upnp_enabled\": false"
                          << ", \"pubsub_port\": \"" << kPinnedPubsubPort << "\" }";
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

        GcsSession *m_handle = nullptr; ///< Session handle under test (TearDown shuts it down)

        std::string m_tempPath; ///< Per-test temp directory (created in SetUp, removed in TearDown)
    };

    /**
     * @brief A parseable PROTOBUF config with no externally booted node boots
     *        the embedded GeniusSDK node inside gcs_init (empty mnemonic =
     *        child-wallet contract) and returns a live session handle.
     *        TearDown's gcs_shutdown pairs the internal boot with node
     *        teardown.
     */
    TEST_F( GcsFFI, InitWithValidConfigBootsEmbeddedNode )
    {
        const std::string bytes = MakeConfigBytes( m_tempPath + "/db", gcs::chat::CODEC_PROTOBUF );
        m_handle                = gcs_init( reinterpret_cast<const uint8_t *>( bytes.data() ), bytes.size() );
        EXPECT_NE( m_handle, nullptr );
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
     * @brief Repeated gcs_init calls with valid configs all return the SAME
     *        live handle (IN-05: the idempotent path ignores later configs) —
     *        one session, one internally booted node, no re-boot churn.
     */
    TEST_F( GcsFFI, RepeatedInitCallsReturnSameIdempotentHandle )
    {
        const std::string bytes = MakeConfigBytes( m_tempPath + "/db", gcs::chat::CODEC_PROTOBUF );
        GcsSession *first       = nullptr;
        for ( int callIndex = 0; callIndex < kRepeatedInitCallCount; ++callIndex )
        {
            GcsSession *handle = gcs_init( reinterpret_cast<const uint8_t *>( bytes.data() ), bytes.size() );
            ASSERT_NE( handle, nullptr );
            if ( callIndex == 0 )
            {
                first = handle;
            }
            else
            {
                EXPECT_EQ( handle, first );
            }
        }
        m_handle = first; // TearDown shuts the one session down
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
