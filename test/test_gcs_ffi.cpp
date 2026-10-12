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

// Vendored Dart API_DL headers (reached through gcs_ffi's PUBLIC src/ include
// dir): the fake-table seam lets the pushed GcsEvent bytes land in a process
// capture log instead of a (nonexistent) Dart VM. The entry names/types match
// the cold-boot/sdk FFI fixtures (test_gcs_ffi_coldboot.cpp).
#include "ffi/dart_api_dl.h"
#include "ffi/dart_version.h"
#include "ffi/internal/dart_api_dl_impl.h"

// Own main() (std::_Exit after RUN_ALL_TESTS): gcs_init boots the embedded
// node INSIDE gcs_ffi's image, so this binary has the same exit-time
// straggler exposure as the SDK-booting tests — the node's detached retry
// threads must never observe static destruction (run 35924087303 x86 Debug:
// 8/8 passed, then "pthread lock: Invalid argument" abort at exit; see
// gcs_exit_main.hpp).
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
    /// Arbitrary non-zero fake Dart NativePort id (pushed-event capture seam).
    constexpr int64_t kFakeDartPort = 7777;

    /**
     * @brief Thread-safe log of payloads posted to the fake Dart port.
     *
     * gcs_ffi posts synchronously under its own mutex; the separate mutex here
     * keeps the capture self-contained (mirrors test_gcs_ffi_coldboot.cpp).
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
     * @brief Extracts the newest MemberList payload for a scope.
     *
     * @param[in] events  Pushed payload bytes (one serialized GcsEvent each).
     * @param[in] scopeId The scope whose roster is wanted.
     * @return The last matching MemberList payload, or nullopt when none pushed.
     */
    std::optional<gcs::chat::MemberList> FindLatestMemberList( const std::vector<std::string> &events,
                                                               const std::string &scopeId )
    {
        std::optional<gcs::chat::MemberList> latest;
        for ( const std::string &eventBytes : events )
        {
            gcs::chat::GcsEvent event;
            if ( event.ParseFromString( eventBytes ) && event.has_member_list()
                 && event.member_list().scope_id() == scopeId )
            {
                latest = event.member_list();
            }
        }
        return latest;
    }

    /**
     * @brief Extracts the newest InviteLink payload among captured events.
     *
     * @param[in] events Pushed payload bytes (one serialized GcsEvent each).
     * @return The last InviteLink payload found, or nullopt when none pushed.
     */
    std::optional<gcs::chat::InviteLink> FindLatestInviteLink( const std::vector<std::string> &events )
    {
        std::optional<gcs::chat::InviteLink> latest;
        for ( const std::string &eventBytes : events )
        {
            gcs::chat::GcsEvent event;
            if ( event.ParseFromString( eventBytes ) && event.has_invite_link() )
            {
                latest = event.invite_link();
            }
        }
        return latest;
    }

    /**
     * @brief Whether any captured payload is an ErrorNotice (D-29 raw string).
     *
     * @param[in] events Pushed payload bytes (one serialized GcsEvent each).
     * @return true when at least one payload carries an ErrorNotice.
     */
    bool HasErrorNotice( const std::vector<std::string> &events )
    {
        for ( const std::string &eventBytes : events )
        {
            gcs::chat::GcsEvent event;
            if ( event.ParseFromString( eventBytes ) && event.has_error() )
            {
                return true;
            }
        }
        return false;
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
     * @brief Fixture owning a per-test temp directory (for GcsConfig db paths)
     *        and the session handle under test.
     */
    class GcsFFI : public ::testing::Test
    {
    protected:
        void SetUp() override
        {
            // The embedded node boots INSIDE gcs_ffi's image (gcs_init boots
            // it), so the secure-storage factory must be set on the DLL's
            // copy — hence the exported seam, not the C++ helper (IMAGE RULE,
            // gcs_storage/common/test_env.hpp). Before any gcs_init; without
            // it the OS keychain makes account creation flaky on CI runners
            // (runs 35904291986 / 35918456744: "Failed to generate Genius
            // address from private key").
            gcs_use_test_secure_storage();

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

        /**
         * @brief gcs_init against this fixture's temp db path (PROTOBUF config).
         *
         * @return The session handle, or nullptr when the embedded boot failed.
         */
        GcsSession *InitSession()
        {
            const std::string bytes = MakeConfigBytes( m_tempPath + "/db", gcs::chat::CODEC_PROTOBUF );
            return gcs_init( reinterpret_cast<const uint8_t *>( bytes.data() ), bytes.size() );
        }

        /**
         * @brief Installs the fake API_DL table, inits, subscribes, and drains
         *        the subscribe-time replay batch.
         *
         * The fake table must be installed BEFORE any push so PostToDart's
         * Dart_PostCObject_DL resolves into g_pushedEvents instead of no-op'ing.
         * gcs_publish/gcs_subscribe are synchronous under g_mutex and post their
         * events before returning, so the capture log is complete with no
         * thread-wait: this file performs no sleeping of any kind.
         *
         * @return The session handle, or nullptr when the embedded boot failed.
         */
        GcsSession *InitSubscribedSession()
        {
            EXPECT_TRUE( InstallFakeApiDlTable() );
            GcsSession *handle = InitSession();
            if ( handle != nullptr )
            {
                EXPECT_EQ( gcs_subscribe( handle, kEventTopic, kFakeDartPort ), GCS_OK );
                (void)g_pushedEvents.Take(); // drop the subscribe-time replay batch
            }
            return handle;
        }

        /**
         * @brief Publishes a serialized GcsCommand to the command topic.
         *
         * @param[in] handle  Session handle from gcs_init.
         * @param[in] command The command envelope to publish.
         * @return The gcs_publish status code.
         */
        int PublishCommand( GcsSession *handle, const gcs::chat::GcsCommand &command )
        {
            const std::string payload = command.SerializeAsString();
            return gcs_publish( handle,
                                kCommandTopic,
                                reinterpret_cast<const uint8_t *>( payload.data() ),
                                payload.size() );
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

    // === Phase 4 (04-05 Task 1): Membership wiring at the FFI plane ===
    // The Membership component and its heal callback are file-static in
    // gcs_ffi, so they are observed through their effects: a create_space
    // minting the creator's Super Admin record pushes a MemberList through
    // Membership's own sink (unreachable unless g_membership exists and the
    // /gcs/members/.* callback was registered), and that MemberList carries
    // self_address stamped from the injected local wallet (never re-stamped by
    // the FFI).

    /**
     * @brief Creating a space constructs g_membership, mints the creator's
     *        Super Admin record (D-03), and pushes a MemberList whose
     *        self_address is the injected local wallet.
     */
    TEST_F( GcsFFI, CreateSpacePushesCreatorMemberListWithSelfAddress )
    {
        m_handle = InitSubscribedSession();
        ASSERT_NE( m_handle, nullptr );

        gcs::chat::GcsCommand createSpace;
        createSpace.mutable_create_space()->set_name( "ops" );
        createSpace.mutable_create_space()->set_is_public( true );
        ASSERT_EQ( PublishCommand( m_handle, createSpace ), GCS_OK );

        const std::vector<std::string> events = g_pushedEvents.Take();
        const auto                     tree   = FindLatestSpaceTree( events );
        ASSERT_TRUE( tree.has_value() );
        ASSERT_EQ( tree->space_size(), 1 );
        const std::string spaceId = tree->space( 0 ).id();

        const auto roster = FindLatestMemberList( events, spaceId );
        ASSERT_TRUE( roster.has_value() );
        ASSERT_EQ( roster->member_size(), 1 );
        EXPECT_FALSE( roster->self_address().empty() );
        EXPECT_EQ( roster->self_address(), roster->member( 0 ).address() );
        EXPECT_EQ( roster->member( 0 ).role(), gcs::chat::ROLE_SUPER_ADMIN );
    }

    /**
     * @brief Re-subscribing replays a MemberList for every member scope (D-04)
     *        — the space created below surfaces its roster on the second
     *        subscribe without any per-scope pull from Dart.
     */
    TEST_F( GcsFFI, SubscribeReplaysMemberListForEveryMemberScope )
    {
        m_handle = InitSubscribedSession();
        ASSERT_NE( m_handle, nullptr );

        gcs::chat::GcsCommand createSpace;
        createSpace.mutable_create_space()->set_name( "ops" );
        ASSERT_EQ( PublishCommand( m_handle, createSpace ), GCS_OK );

        const auto createdTree = FindLatestSpaceTree( g_pushedEvents.Take() );
        ASSERT_TRUE( createdTree.has_value() );
        ASSERT_EQ( createdTree->space_size(), 1 );
        const std::string spaceId = createdTree->space( 0 ).id();

        // Re-subscribe: the session-init replay pushes a MemberList per space.
        ASSERT_EQ( gcs_subscribe( m_handle, kEventTopic, kFakeDartPort ), GCS_OK );
        const auto replay = g_pushedEvents.Take();
        const auto roster = FindLatestMemberList( replay, spaceId );
        ASSERT_TRUE( roster.has_value() );
        EXPECT_EQ( roster->scope_id(), spaceId );
        ASSERT_GE( roster->member_size(), 1 );
    }

    // === Phase 4 (04-05 Task 2): membership command arms + P5 guards ===
    // Each arm re-validates the hostile command shape C++-side (V5) and
    // delegates the D-08 permission decision to the Membership write guard; a
    // rejection pushes a raw ErrorNotice and returns GCS_ERROR_INVALID_ARGUMENT.

    /**
     * @brief kInvite with an empty scope_id is rejected at the ABI boundary:
     *        INVALID_ARGUMENT plus an ErrorNotice, never a Membership call.
     */
    TEST_F( GcsFFI, InviteRejectsEmptyScopeId )
    {
        m_handle = InitSubscribedSession();
        ASSERT_NE( m_handle, nullptr );

        gcs::chat::GcsCommand invite;
        invite.mutable_invite()->set_scope_id( "" );
        EXPECT_EQ( PublishCommand( m_handle, invite ), GCS_ERROR_INVALID_ARGUMENT );

        EXPECT_TRUE( HasErrorNotice( g_pushedEvents.Take() ) );
    }

    /**
     * @brief A space creator (Super Admin) mints a D-05 capability invite link:
     *        the pushed InviteLink carries the exact
     *        `gcs://invite/space/<inviteId>?key=<IK>` shape.
     */
    TEST_F( GcsFFI, InviteMintsSpaceInviteLinkWithCapabilityShape )
    {
        m_handle = InitSubscribedSession();
        ASSERT_NE( m_handle, nullptr );

        gcs::chat::GcsCommand createSpace;
        createSpace.mutable_create_space()->set_name( "ops" );
        ASSERT_EQ( PublishCommand( m_handle, createSpace ), GCS_OK );
        const auto tree = FindLatestSpaceTree( g_pushedEvents.Take() );
        ASSERT_TRUE( tree.has_value() );
        ASSERT_EQ( tree->space_size(), 1 );
        const std::string spaceId = tree->space( 0 ).id();

        gcs::chat::GcsCommand invite;
        invite.mutable_invite()->set_scope_id( spaceId );
        ASSERT_EQ( PublishCommand( m_handle, invite ), GCS_OK );

        const auto link = FindLatestInviteLink( g_pushedEvents.Take() );
        ASSERT_TRUE( link.has_value() );
        EXPECT_EQ( link->url().rfind( "gcs://invite/space/", 0 ), 0u );
        EXPECT_NE( link->url().find( "?key=" ), std::string::npos );
    }

    /**
     * @brief kRedeem re-parses the URL C++-side (P8): a non-`gcs://invite/`
     *        string is rejected with INVALID_ARGUMENT plus an ErrorNotice.
     */
    TEST_F( GcsFFI, RedeemRejectsMalformedUrl )
    {
        m_handle = InitSubscribedSession();
        ASSERT_NE( m_handle, nullptr );

        gcs::chat::GcsCommand redeem;
        redeem.mutable_redeem()->set_invite_url( "not-an-invite-url" );
        EXPECT_EQ( PublishCommand( m_handle, redeem ), GCS_ERROR_INVALID_ARGUMENT );

        EXPECT_TRUE( HasErrorNotice( g_pushedEvents.Take() ) );
    }

    /**
     * @brief D-03/MEMB-03: demoting the scope creator (Super Admin) is rejected
     *        by the Membership guard — INVALID_ARGUMENT plus an ErrorNotice.
     */
    TEST_F( GcsFFI, ChangeRoleRejectsDemotingCreator )
    {
        m_handle = InitSubscribedSession();
        ASSERT_NE( m_handle, nullptr );

        gcs::chat::GcsCommand createSpace;
        createSpace.mutable_create_space()->set_name( "ops" );
        ASSERT_EQ( PublishCommand( m_handle, createSpace ), GCS_OK );
        const auto events = g_pushedEvents.Take();
        const auto tree   = FindLatestSpaceTree( events );
        ASSERT_TRUE( tree.has_value() );
        ASSERT_EQ( tree->space_size(), 1 );
        const std::string spaceId = tree->space( 0 ).id();
        const auto        roster  = FindLatestMemberList( events, spaceId );
        ASSERT_TRUE( roster.has_value() );
        ASSERT_EQ( roster->member_size(), 1 );
        const std::string creator = roster->member( 0 ).address();
        ASSERT_FALSE( creator.empty() );

        gcs::chat::GcsCommand changeRole;
        changeRole.mutable_change_role()->set_scope_id( spaceId );
        changeRole.mutable_change_role()->set_target( creator );
        changeRole.mutable_change_role()->set_role( gcs::chat::ROLE_MEMBER );
        EXPECT_EQ( PublishCommand( m_handle, changeRole ), GCS_ERROR_INVALID_ARGUMENT );

        EXPECT_TRUE( HasErrorNotice( g_pushedEvents.Take() ) );
    }

    /**
     * @brief kApprove with an empty target is rejected at the ABI boundary:
     *        INVALID_ARGUMENT plus an ErrorNotice, never a Membership call.
     */
    TEST_F( GcsFFI, ApproveRejectsEmptyTarget )
    {
        m_handle = InitSubscribedSession();
        ASSERT_NE( m_handle, nullptr );

        gcs::chat::GcsCommand approve;
        approve.mutable_approve()->set_scope_id( "space-missing" );
        approve.mutable_approve()->set_target( "" );
        EXPECT_EQ( PublishCommand( m_handle, approve ), GCS_ERROR_INVALID_ARGUMENT );

        EXPECT_TRUE( HasErrorNotice( g_pushedEvents.Take() ) );
    }

    // === Phase 4 (04-05 Task 3): D-02 explicit_leave term on derived joins ===
    // joined(room) = parentSpace.autoJoinRooms && !member.explicit_leave. The
    // leave arm cannot produce the explicit_leave==true case from the FFI: the
    // local wallet is always the space creator (Super Admin), and
    // Membership::Leave rejects the Super Admin, so a single-wallet FFI test can
    // never make HasExplicitLeave() true. That branch is covered at the owning
    // module's boundary in test_gcs_membership.cpp (MembershipTest explicit-
    // leave cases). What the FFI can prove is the non-filtered branch: a room
    // derived from an autoJoinRooms parent with no explicit leave still enters
    // the derived set, so the D-02 filter never over-evicts.

    /**
     * @brief A room derived from an autoJoinRooms parent (no explicit leave)
     *        still joins: its derived topic reaches the pushed RoomList, i.e.
     *        the D-02 explicit_leave filter keeps the room.
     */
    TEST_F( GcsFFI, DerivedAutoJoinedRoomSurvivesExplicitLeaveFilter )
    {
        m_handle = InitSubscribedSession();
        ASSERT_NE( m_handle, nullptr );

        gcs::chat::GcsCommand createSpace;
        createSpace.mutable_create_space()->set_name( "ops" );
        createSpace.mutable_create_space()->set_is_public( true );
        createSpace.mutable_create_space()->set_auto_join_rooms( true );
        ASSERT_EQ( PublishCommand( m_handle, createSpace ), GCS_OK );
        const auto tree = FindLatestSpaceTree( g_pushedEvents.Take() );
        ASSERT_TRUE( tree.has_value() );
        ASSERT_EQ( tree->space_size(), 1 );
        const std::string spaceId = tree->space( 0 ).id();

        gcs::chat::GcsCommand createRoom;
        createRoom.mutable_create_room()->set_name( "general" );
        createRoom.mutable_create_room()->set_parent_space_id( spaceId );
        ASSERT_EQ( PublishCommand( m_handle, createRoom ), GCS_OK );

        const auto        events = g_pushedEvents.Take();
        const auto        roomTree = FindLatestSpaceTree( events );
        ASSERT_TRUE( roomTree.has_value() );
        ASSERT_EQ( roomTree->room_size(), 1 );
        const std::string roomId = roomTree->room( 0 ).id();
        EXPECT_EQ( roomTree->room( 0 ).parent_space_id(), spaceId );

        // The derived topic reaches the RoomList — the explicit_leave term is
        // false for a local creator with no leave, so D-02 keeps it joined.
        const std::string roomTopic = std::string( "gcs/chat/" ) + roomId;
        EXPECT_TRUE( RoomListCarriesTopic( events, roomTopic ) );
    }

} // namespace gcs::test
