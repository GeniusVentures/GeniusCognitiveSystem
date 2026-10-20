/**
 * @file       test_gcs_membership.cpp
 * @brief      gcs::Membership unit tests (MEMB-01..MEMB-05): the C++-owned
 *             member roster, five-tier permission matrix, capability-token
 *             invite mint/redeem (write-at-redeem), the D-03 Super Admin
 *             demotion guard, and the D-07 dormant-until-approved model.
 * @details    Uses the injected-pubsub CoreSession fixture seam of
 *             test_gcs_entities.cpp (no GeniusNode boot) with a
 *             creator-stamped EntityStore and the REAL D-06 key-wrap seam
 *             (gcs::crypto::WrapRoomKey/UnwrapRoomKey, enabled) built by member
 *             assignment — never brace-init (KeyWrapSeam has a user-provided
 *             default ctor). Uses the GCS wait-condition template — never a raw
 *             thread sleep.
 * @date       2026-10-02
 */

#include "lib/gcs_crypto.hpp"
#include "lib/gcs_entity_store.hpp"
#include "lib/gcs_membership.hpp"
#include "test_gcs_invite_helpers.hpp"
#include "test_gcs_member_helpers.hpp"
#include "test_graphsync_network.hpp"
#include "test_wait_condition.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>
#include <utility>
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

    /// Injected local wallet address used as the Super Admin creator (D-03).
    constexpr const char *kTestCreatorAddress = "5GrwvaEF5zXb26Fz9rcQpDWS57CtERHpNehXCPcNoHGKutQY";

    /// Distinct member wallets exercised by the permission/invite tests.
    constexpr const char *kAdminAddress  = "5FHneW46xGXgs5mUiveU4sbTyGBzmstUspZC92UhjJM694ty";
    constexpr const char *kModeratorAddress = "5E9oDs9PjpsBbxXxRE9uMaZZhnBAV38n2oiVspkzJTshyzC2";
    constexpr const char *kMemberAddress = "5FLSigC9HGRKVhB9FiEo4Y3koPsNmBmLJbpXg2mp1hXcS59Y";
    constexpr const char *kGuestAddress  = "5DAAnrj7VHTznn2AWBemMuyBwZWs6FNFjdyVXUeYum3PTXFy";
    constexpr const char *kJoinerAddress = "5HGjWAeFDfFCWPsjFQdVV2Msvz2XtMktvgocEZcCj68kUMaw";
    constexpr const char *kJoiner2Address = "5CiPPseXPECbkjWCa6MnjNokrgYjMqmKndv2rSnekmSKydrd";
    constexpr const char *kTargetAddress = "5GNJqTPyNqANBkUVMN1LPPrxXnFouWXoe2wNSmmEoLctxiZY";
    constexpr const char *kDeadAddress   = "5HpG9w8EBLe5XCrbczpwq5TSXvedjrBGCwqxK1iQ7qUsSWFc";

    /// Room chat topic prefix — the gcs/chat/<id> convention (D-01).
    constexpr const char *kRoomTopicPrefix = "gcs/chat/";

    /**
     * @brief Extract the invite id from a minted
     *        gcs://invite/<kind>/<scope>/<invite>?key=... URL.
     *
     * @param[in] url The minted URL.
     * @return The final path segment; empty when the URL is malformed.
     */
    std::string InviteIdFromUrl( const std::string &url )
    {
        const std::size_t queryPos = url.find( '?' );
        if ( queryPos == std::string::npos )
        {
            return std::string();
        }
        const std::size_t inviteStart = url.rfind( '/', queryPos );
        if ( inviteStart == std::string::npos )
        {
            return std::string();
        }
        return url.substr( inviteStart + 1, queryPos - inviteStart - 1 );
    }

    /**
     * @brief The HEX-ENCODED deterministic signature the MakeSeam double
     *        produces.
     *
     * Kept beside the double so every re-signing test case builds the SAME
     * token (invariant 11). The token embeds the payload, which can contain
     * binary wrapped-key envelopes, so — exactly like MintInvite — it must be
     * hex-encoded before it is stored in the proto3 `string` field; a raw
     * write would fail UTF-8 validation on parse. The real crypto is exercised
     * in 04-18.
     *
     * @param[in] issuer  The claimed issuer address.
     * @param[in] payload The canonical payload bytes.
     * @return The double's signature token, hex-encoded for the proto field.
     */
    std::string TestIssuerSignatureHex( const std::string &issuer,
                                        const std::string &payload )
    {
        return gcs::crypto::HexEncode( std::string( "testsig:" ) + issuer + ":"
                                       + payload );
    }

    /**
     * @brief Read an invite record straight from the CRDT store.
     *
     * @param[in]  session  Live session.
     * @param[in]  scopeId  Scope id.
     * @param[in]  inviteId Invite id.
     * @param[out] out      Parsed record when present.
     * @return true when a parseable record exists.
     */
    bool ReadInviteRecord( gcs::CoreSession &session, const std::string &scopeId,
                           const std::string &inviteId, gcs::chat::InviteRecord &out )
    {
        const auto stored = session.Get(
            std::string( gcs::Membership::kInvitesKeyPrefix ) + scopeId + "/" + inviteId );
        return stored.has_value() && out.ParseFromString( stored.value() );
    }

    /**
     * @brief Overwrite an invite record in the CRDT store (the hostile path).
     *
     * @param[in] session  Live session.
     * @param[in] scopeId  Scope id.
     * @param[in] inviteId Invite id.
     * @param[in] record   The record bytes to write.
     * @return true when the Put succeeded.
     */
    bool WriteInviteRecord( gcs::CoreSession &session, const std::string &scopeId,
                            const std::string &inviteId,
                            const gcs::chat::InviteRecord &record )
    {
        return session
            .Put( std::string( gcs::Membership::kInvitesKeyPrefix ) + scopeId + "/"
                      + inviteId,
                  record.SerializeAsString() )
            .has_value();
    }

    /**
     * @brief Count the raw member keys in a scope (the no-write baseline).
     *
     * @param[in] session Live session.
     * @param[in] scopeId Scope id.
     * @return The key count; 0 on a scan failure.
     */
    std::size_t MemberScanCount( gcs::CoreSession &session,
                                 const std::string &scopeId )
    {
        auto scan = session.QueryKeyValues(
            std::string( gcs::Membership::kMembersKeyPrefix ) + scopeId + "/" );
        return scan.has_value() ? scan.value().size() : 0;
    }

    /// CRDT key prefix for space records (mirrors gcs_entity_store.cpp) —
    /// used to read the authoritative entity bytes in the delete/approve tests.
    constexpr const char kSpacesKeyPrefix[] = "gcs/entities/spaces/";

    /// Base64url characters encoding a 32-byte key without padding (D-05).
    constexpr std::size_t kEncodedKeyLength = 43;

    /// Arbitrary persisted timestamp for the immutability check (D-03).
    constexpr int64_t kSeedCreatedAtMs = 4242;

    /// Minimal soralog YAML — console sink only, error level, sufficient to
    /// satisfy libp2p::log::setLoggingSystem() before any SuperGenius logger is
    /// constructed (mirrors test_gcs_entities.cpp).
    constexpr const char *kLoggingYaml = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_membership_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";
} // namespace

namespace gcs::test
{
    /// Test rig: one live session + creator-stamped entity catalog.
    struct MembershipRig
    {
        std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> pubsub;
        TestGraphsyncContext                             graphsync;
        std::unique_ptr<gcs::CoreSession>                session;
        std::unique_ptr<gcs::EntityStore>                store;
    };

    /**
     * @brief Read a member record straight from the CRDT store.
     *
     * @param[in]  session  Live session.
     * @param[in]  scopeId  Scope id.
     * @param[in]  address  Member wallet address.
     * @param[out] out      Parsed record when present.
     * @return true when a parseable record exists.
     */
    bool ReadMemberRecord( gcs::CoreSession &session, const std::string &scopeId,
                           const std::string &address, chat::MemberRecord &out )
    {
        const auto stored = session.Get(
            std::string( gcs::Membership::kMembersKeyPrefix ) + scopeId + "/" + address );
        if ( !stored.has_value() )
        {
            return false;
        }
        return out.ParseFromString( stored.value() );
    }

    /**
     * @brief Put member bytes on the production replication topic.
     *
     * Production authority writes replicate on `kMembershipTopic`; seeding and
     * re-announcing through the same path gives the CRDT a tracked head, which
     * is what makes a re-announce of previously written bytes a new DAG node
     * rather than a deduplicated no-op.
     *
     * @param[in] session Live session.
     * @param[in] scopeId Scope id.
     * @param[in] address Member wallet address.
     * @param[in] bytes   Serialized member record.
     * @return true when the Put succeeded.
     */
    bool PutMemberBytes( gcs::CoreSession &session, const std::string &scopeId,
                         const std::string &address, const std::string &bytes )
    {
        const std::unordered_set<std::string> topics{
            gcs::Membership::kMembershipTopic };
        return session
            .Put( std::string( gcs::Membership::kMembersKeyPrefix ) + scopeId + "/"
                      + address,
                  bytes, topics )
            .has_value();
    }

    /**
     * @brief Find a member entry by address in a pushed MemberList.
     *
     * @param[in] list    The roster.
     * @param[in] address The wallet address to find.
     * @return Pointer into @p list, or nullptr when absent.
     */
    const chat::MemberRecord *FindMember( const chat::MemberList &list,
                                          const std::string       &address )
    {
        for ( int i = 0; i < list.member_size(); ++i )
        {
            if ( list.member( i ).address() == address )
            {
                return &list.member( i );
            }
        }
        return nullptr;
    }

    /**
     * @brief Fixture owning a per-test temp dir, the injected pubsub + graphsync
     *        seam, and the creator-stamped EntityStore (mirrors
     *        test_gcs_entities.cpp's fixture).
     */
    class GcsMembershipTest : public ::testing::Test
    {
    protected:
        static void SetUpTestSuite()
        {
            auto loggerConfigurator = std::make_shared<libp2p::log::Configurator>();
            auto configFromYaml     = std::make_shared<soralog::ConfiguratorFromYAML>(
                loggerConfigurator, std::string{ kLoggingYaml } );
            auto loggingSystem  = std::make_shared<soralog::LoggingSystem>( configFromYaml );
            const auto confResult = loggingSystem->configure();
            ASSERT_FALSE( confResult.has_error ) << "Could not configure test logging system";
            libp2p::log::setLoggingSystem( loggingSystem );
        }

        void SetUp() override
        {
            const auto *info       = ::testing::UnitTest::GetInstance()->current_test_info();
            const auto  uniqueSalt = std::chrono::steady_clock::now().time_since_epoch().count();
            m_tempPath = ( std::filesystem::temp_directory_path()
                           / ( std::string{ "gcs_membership_" } + info->name() + "_" +
                               std::to_string( uniqueSalt ) ) )
                             .string();
            std::filesystem::create_directories( m_tempPath );
        }

        void TearDown() override
        {
            if ( m_rig.session )
            {
                m_rig.session->Shutdown();
            }
            m_rig.store.reset();
            m_rig.session.reset();
            if ( m_rig.pubsub )
            {
                m_rig.pubsub->Stop();
                m_rig.pubsub.reset();
            }
            std::error_code ec;
            std::filesystem::remove_all( m_tempPath, ec );
        }

        /**
         * @brief Stand up a real GossipPubSub on port 0 (mirrors
         *        test_gcs_entities.cpp).
         *
         * @param[in] keyDir Directory for the KeyPairFileStorage key store.
         * @return A started GossipPubSub, or nullptr on failure.
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
         * @brief Start the rig's session + creator-stamped entity store.
         *
         * @param[in] creator Wallet address stamped as the creator (D-03).
         */
        void StartRig( const std::string &creator )
        {
            m_rig.pubsub = MakeStartedPubSub( m_tempPath + "/key" );
            ASSERT_NE( m_rig.pubsub, nullptr );
            m_rig.graphsync = gcs::test::MakeGraphsyncContext( m_rig.pubsub );

            gcs::CoreSession::Config cfg{};
            cfg.m_dbPath = m_tempPath + "/db";
            m_rig.session = std::make_unique<gcs::CoreSession>( cfg );
            ASSERT_TRUE( m_rig.session->Initialize( m_rig.pubsub, m_rig.graphsync.network ).has_value() );

            const std::string dbPath = cfg.m_dbPath;
            ASSERT_TRUE( gcs::test::WaitForCondition(
                [ &dbPath ]() { return std::filesystem::exists( dbPath ); }, gcs::test::kWaitTimeout ) );

            m_rig.store = std::make_unique<gcs::EntityStore>( *m_rig.session, creator );
            ASSERT_TRUE( m_rig.store->LoadFromStore().has_value() );
        }

        /**
         * @brief The D-06 seam, enabled (built by member assignment).
         *
         * The wrap/unwrap/authenticator callables are the REAL gcs::crypto
         * adapters. The issuer pair is a TEST DOUBLE that models signer
         * binding: `sign` emits a deterministic token over the signer address
         * and payload, and `verifyIssuer` accepts it only for the CLAIMED
         * issuer. The real secp256k1 path is exercised in 04-18's multinode
         * proof. When @p issuerEnabled is false both callables stay empty, so
         * IssuerAuthEnabled() is false and mint/redeem fail closed.
         *
         * @param[in] signerAddress The wallet the seam's signing key belongs to.
         * @param[in] issuerEnabled Whether to install the issuer callables.
         */
        static gcs::Membership::KeyWrapSeam MakeSeam( const std::string &signerAddress,
                                                      bool issuerEnabled = true )
        {
            gcs::Membership::KeyWrapSeam seam;
            seam.wrap      = &gcs::crypto::WrapRoomKey;
            seam.unwrap    = &gcs::crypto::UnwrapRoomKey;
            seam.mintAuth  = &gcs::crypto::MintInviteAuthenticator;
            seam.verifyAuth = &gcs::crypto::VerifyInviteAuthenticator;
            if ( issuerEnabled )
            {
                seam.sign = [ signerAddress ]( const std::string &payload,
                                               const std::string &issuer )
                    -> gcs::outcome::result<std::string> {
                    return std::string( "testsig:" ) + signerAddress + ":" + payload;
                };
                seam.verifyIssuer = []( const std::string &issuer,
                                        const std::string &payload,
                                        const std::string &signature )
                    -> gcs::outcome::result<bool> {
                    return signature
                           == std::string( "testsig:" ) + issuer + ":" + payload;
                };
            }
            seam.enabled = true;
            return seam;
        }

        /// @brief A sink that captures every pushed event (test-thread only).
        gcs::Membership::EventSink CapturingSink( std::vector<chat::GcsEvent> *out )
        {
            return [ out ]( const chat::GcsEvent &event ) { out->push_back( event ); };
        }

        /// @brief Persist a member record directly.
        ///
        /// The shared helper signs the authority payload exactly as production
        /// verifies it, so every seeded record models a record the trust chain
        /// can construct. A hostile case passes a different @p signer; the
        /// rejection then comes from the authority matrix, not from a broken
        /// signature.
        void SeedMember( const std::string &scopeId, const std::string &address,
                         chat::Role role, int64_t createdAtMs = kSeedCreatedAtMs,
                         bool deleted = false,
                         const std::string &signer = kTestCreatorAddress )
        {
            chat::MemberRecord record;
            record.set_scope_id( scopeId );
            record.set_address( address );
            record.set_role( role );
            record.set_deleted( deleted );
            // A tombstone changes no role; a fresh grant has no predecessor.
            record.set_prior_role( deleted ? role : chat::ROLE_UNSPECIFIED );
            record.set_created_at_ms( createdAtMs );
            record.set_updated_at_ms( createdAtMs );
            record.set_created_by( signer );
            record.set_record_signature(
                TestMemberSignatureHex( signer, scopeId, address, record ) );
            ASSERT_TRUE( PutMemberBytes( *m_rig.session, scopeId, address,
                                         record.SerializeAsString() ) );
        }

        MembershipRig m_rig;
        std::string   m_tempPath;
    };

    /**
     * @brief Test 1 (MEMB-02): OnScopeCreated writes the creator's Super Admin
     *        record and MembersFor stamps self_address from the injected sender;
     *        an unknown scope resolves the local role to the safe Guest default.
     */
    TEST_F( GcsMembershipTest, OnScopeCreatedWritesSuperAdminRecordAndSelfAddress )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );

        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        auto list = membership.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        ASSERT_EQ( list.value().member_size(), 1 );
        EXPECT_EQ( list.value().scope_id(), spaceId );
        EXPECT_EQ( list.value().self_address(), kTestCreatorAddress );
        EXPECT_EQ( list.value().member( 0 ).address(), kTestCreatorAddress );
        EXPECT_EQ( list.value().member( 0 ).role(), chat::ROLE_SUPER_ADMIN );
        EXPECT_TRUE( list.value().member( 0 ).approved_by().empty() );
        EXPECT_EQ( list.value().member( 0 ).pending_action(), chat::PENDING_ACTION_UNSPECIFIED );

        // The creator root is author-signed with no predecessor claim.
        chat::MemberRecord creatorRecord;
        ASSERT_TRUE(
            ReadMemberRecord( *m_rig.session, spaceId, kTestCreatorAddress, creatorRecord ) );
        EXPECT_EQ( creatorRecord.created_by(), kTestCreatorAddress );
        EXPECT_FALSE( creatorRecord.record_signature().empty() );
        EXPECT_EQ( creatorRecord.prior_role(), chat::ROLE_UNSPECIFIED );

        EXPECT_EQ( membership.LocalRoleFor( spaceId ), chat::ROLE_SUPER_ADMIN );
        // Unknown scope: no local record -> safest default (D-08 read-only).
        EXPECT_EQ( membership.LocalRoleFor( "space-unknown" ), chat::ROLE_GUEST );
        EXPECT_FALSE( membership.HasExplicitLeave( spaceId ) );

        // The roster push landed on the sink.
        ASSERT_FALSE( events.empty() );
        EXPECT_TRUE( events.back().has_member_list() );
    }

    /**
     * @brief Positive control: a creator-signed seed is admitted to the roster.
     *
     * The helper signs the SAME canonical payload production verifies, so this
     * case fails if they drift.
     */
    TEST_F( GcsMembershipTest, SignedSeedIsAdmitted )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        auto list = membership.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        ASSERT_EQ( list.value().member_size(), 2 );
        EXPECT_NE( FindMember( list.value(), kTestCreatorAddress ), nullptr );
        const chat::MemberRecord *seeded = FindMember( list.value(), kMemberAddress );
        ASSERT_NE( seeded, nullptr );
        EXPECT_EQ( seeded->role(), chat::ROLE_MEMBER );
        EXPECT_EQ( seeded->created_by(), kTestCreatorAddress );
    }

    /**
     * @brief Test 2 (MEMB-03): every demotion targeting the scope creator is
     *        rejected (D-03), while a non-creator member's role change succeeds.
     */
    TEST_F( GcsMembershipTest, CreatorDemotionIsRejected )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        // The creator (Super Admin) is never demotable — any target role.
        EXPECT_FALSE( membership.ChangeRole( spaceId, kTestCreatorAddress, chat::ROLE_ADMIN ).has_value() );
        EXPECT_FALSE( membership.ChangeRole( spaceId, kTestCreatorAddress, chat::ROLE_MODERATOR ).has_value() );
        EXPECT_FALSE( membership.ChangeRole( spaceId, kTestCreatorAddress, chat::ROLE_MEMBER ).has_value() );
        EXPECT_FALSE( membership.ChangeRole( spaceId, kTestCreatorAddress, chat::ROLE_GUEST ).has_value() );

        chat::MemberRecord creator;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTestCreatorAddress, creator ) );
        EXPECT_EQ( creator.role(), chat::ROLE_SUPER_ADMIN );

        // A plain member's promotion by the Super Admin applies immediately.
        ASSERT_TRUE( membership.ChangeRole( spaceId, kTargetAddress, chat::ROLE_MODERATOR ).has_value() );
        chat::MemberRecord target;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, target ) );
        EXPECT_EQ( target.role(), chat::ROLE_MODERATOR );
        EXPECT_EQ( target.created_at_ms(), kSeedCreatedAtMs ); // immutables preserved
    }

    /**
     * @brief Test 3 (MEMB-05): a Member may mint only when the scope's
     *        members_can_invite gate is on; the space-scope URL carries the
     *        space kind.
     */
    TEST_F( GcsMembershipTest, membersCanInviteGatesMemberMinting )
    {
        StartRig( kTestCreatorAddress );

        std::vector<chat::GcsEvent> creatorEvents;
        gcs::Membership             creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &creatorEvents ), MakeSeam( kTestCreatorAddress ) );

        std::vector<chat::GcsEvent> memberEvents;
        gcs::Membership             member( *m_rig.session, *m_rig.store, kMemberAddress,
                                CapturingSink( &memberEvents ), MakeSeam( kMemberAddress ) );

        auto closed   = m_rig.store->CreateSpace( "closed", true, true, false );
        auto open     = m_rig.store->CreateSpace( "open", true, true, true );
        ASSERT_TRUE( closed.has_value() );
        ASSERT_TRUE( open.has_value() );
        const std::string closedId = closed.value().id();
        const std::string openId   = open.value().id();
        SeedMember( closedId, kMemberAddress, chat::ROLE_MEMBER );
        SeedMember( openId, kMemberAddress, chat::ROLE_MEMBER );
        ASSERT_TRUE( creator.OnScopeCreated( closedId, "", true ).has_value() );
        ASSERT_TRUE( creator.OnScopeCreated( openId, "", true ).has_value() );

        // Gate OFF: the Member's mint is rejected (MEMB-05/D-08).
        EXPECT_FALSE( member.MintInvite( closedId ).has_value() );

        // Gate ON: the Member's mint succeeds and names a space scope.
        auto minted = member.MintInvite( openId );
        ASSERT_TRUE( minted.has_value() );
        EXPECT_EQ( minted.value().rfind( "gcs://invite/space/", 0 ), 0 );

        // No invite record was written for the rejected scope.
        auto closedInvites = m_rig.session->QueryKeyValues(
            std::string( gcs::Membership::kInvitesKeyPrefix ) + closedId + "/" );
        ASSERT_TRUE( closedInvites.has_value() );
        EXPECT_TRUE( closedInvites.value().empty() );
    }

    /**
     * @brief Test 4 (MEMB-01 mint): MintInvite persists ONE InviteRecord under
     *        gcs/invites/<scopeId>/<inviteId> carrying the Guest grant and the
     *        wrapped room key, returns the bearer URL, and writes NO member
     *        record for the (unknown) recipient.
     */
    TEST_F( GcsMembershipTest, MintPersistsInviteRecordAndWritesNoMember )
    {
        StartRig( kTestCreatorAddress );

        auto room = m_rig.store->CreateRoom( "lounge", "", true ); // standalone room
        ASSERT_TRUE( room.has_value() );
        const std::string roomId  = room.value().id();
        const std::string roomTopic = std::string( kRoomTopicPrefix ) + roomId;

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( roomId, roomTopic, false ).has_value() );

        auto minted = membership.MintInvite( roomId );
        ASSERT_TRUE( minted.has_value() );
        const std::string url = minted.value();
        EXPECT_EQ( url.rfind( "gcs://invite/room/", 0 ), 0 );
        EXPECT_NE( url.find( "?key=" ), std::string::npos );

        // ONE invite record, keyed <scopeId>/<inviteId>, carrying a wrapped key.
        auto invites = m_rig.session->QueryKeyValues( gcs::Membership::kInvitesKeyPrefix );
        ASSERT_TRUE( invites.has_value() );
        ASSERT_EQ( invites.value().size(), 1U );

        chat::InviteRecord invite;
        ASSERT_TRUE( invite.ParseFromString( invites.value().front().second ) );
        EXPECT_EQ( invite.scope_id(), roomId );
        EXPECT_EQ( invite.role(), chat::ROLE_GUEST ); // C++-stamped grant (D-05)
        ASSERT_EQ( invite.wrapped_room_key_size(), 1 );
        EXPECT_EQ( invite.wrapped_room_key( 0 ).room_id(), roomId );
        EXPECT_FALSE( invite.wrapped_room_key( 0 ).envelope().empty() );

        // Only the creator is a member — no recipient record exists at mint.
        auto list = membership.MembersFor( roomId );
        ASSERT_TRUE( list.has_value() );
        EXPECT_EQ( list.value().member_size(), 1 );
        EXPECT_EQ( list.value().member( 0 ).address(), kTestCreatorAddress );
    }

    /**
     * @brief Test 5 (MEMB-01 redeem): redeeming unwraps + stores the room key,
     *        writes the redeemer's wallet-keyed member record with the guest
     *        grant, and is multi-use (a second wallet redeems the same URL).
     */
    TEST_F( GcsMembershipTest, RedeemWritesMemberStoresKeyAndIsMultiUse )
    {
        StartRig( kTestCreatorAddress );

        auto room = m_rig.store->CreateRoom( "lounge", "", true );
        ASSERT_TRUE( room.has_value() );
        const std::string roomId    = room.value().id();
        const std::string roomTopic = std::string( kRoomTopicPrefix ) + roomId;

        std::vector<chat::GcsEvent> creatorEvents;
        gcs::Membership             creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &creatorEvents ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( roomId, roomTopic, false ).has_value() );

        auto minted = creator.MintInvite( roomId );
        ASSERT_TRUE( minted.has_value() );

        // Prove the redeem itself stores the unwrapped key: clear the local
        // per-room key store first, then redeem and assert it is repopulated.
        gcs::crypto::ClearRoomKey( roomTopic );
        EXPECT_FALSE( gcs::crypto::RoomKeyFor( roomTopic ).has_value() );

        std::vector<chat::GcsEvent> joinerEvents;
        gcs::Membership             joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                                CapturingSink( &joinerEvents ), MakeSeam( kJoinerAddress ) );
        ASSERT_TRUE( joiner.RedeemInvite( minted.value() ).has_value() );

        chat::MemberRecord joinerRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, roomId, kJoinerAddress, joinerRecord ) );
        EXPECT_EQ( joinerRecord.scope_id(), roomId );
        EXPECT_EQ( joinerRecord.role(), chat::ROLE_GUEST );
        EXPECT_FALSE( joinerRecord.deleted() );
        EXPECT_TRUE( joinerRecord.approved_by().empty() );

        // The unwrapped room key landed in the per-room key store (D-06).
        EXPECT_TRUE( gcs::crypto::RoomKeyFor( roomTopic ).has_value() );

        // The redeemer's MemberList push landed on its sink.
        ASSERT_FALSE( joinerEvents.empty() );
        EXPECT_TRUE( joinerEvents.back().has_member_list() );

        // Multi-use / no expiry: the invite record is never consumed.
        std::vector<chat::GcsEvent> joiner2Events;
        gcs::Membership             joiner2( *m_rig.session, *m_rig.store, kJoiner2Address,
                                 CapturingSink( &joiner2Events ), MakeSeam( kJoiner2Address ) );
        ASSERT_TRUE( joiner2.RedeemInvite( minted.value() ).has_value() );

        auto list = creator.MembersFor( roomId );
        ASSERT_TRUE( list.has_value() );
        ASSERT_EQ( list.value().member_size(), 3 );
        EXPECT_NE( FindMember( list.value(), kTestCreatorAddress ), nullptr );
        EXPECT_NE( FindMember( list.value(), kJoinerAddress ), nullptr );
        EXPECT_NE( FindMember( list.value(), kJoiner2Address ), nullptr );
    }

    /**
     * @brief Test 6 (MEMB-04/WR-04): malformed invite URLs are rejected and a
     *        post-mint record mutation no longer writes a member record — the
     *        issuer signature covers the whole record.
     */
    TEST_F( GcsMembershipTest, RedeemRejectsMalformedUrlAndSkipsBadEnvelope )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        // Give the space a child room so a minted invite carries at least one
        // wrapped room key (the sub-case mutates index 0).
        ASSERT_TRUE( m_rig.store->CreateRoom( "lounge", spaceId, true ).has_value() );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                               CapturingSink( &events ), MakeSeam( kJoinerAddress ) );
        // New shape: gcs://invite/<kind>/<scope>/<invite>?key=<key>.
        EXPECT_FALSE( joiner.RedeemInvite( "https://example.com/not-an-invite" ).has_value() );
        EXPECT_FALSE( joiner.RedeemInvite( "gcs://invite/space/" ).has_value() );
        EXPECT_FALSE( joiner.RedeemInvite( "gcs://invite/space/abc" ).has_value() );
        EXPECT_FALSE( joiner.RedeemInvite( "gcs://invite/space/abc/def" ).has_value() );
        EXPECT_FALSE( joiner.RedeemInvite( "gcs://invite/other/abc/def?key=AAAA" ).has_value() );
        EXPECT_FALSE( joiner.RedeemInvite( "gcs://invite/space//invite?key=AAAA" ).has_value() );
        EXPECT_FALSE(
            joiner.RedeemInvite( "gcs://invite/space/scope/?key=AAAA" ).has_value() );
        EXPECT_FALSE(
            joiner.RedeemInvite( "gcs://invite/space/scope/invite?key=" ).has_value() );
        EXPECT_FALSE(
            joiner.RedeemInvite( "gcs://invite/space/scope/invite?key=!!!" ).has_value() );

        // A genuine mint, then a post-mint mutation of the wrapped envelope:
        // the authenticator still covers only key/scope/id, but the issuer
        // signature covers the record, so the redeem is rejected (assert the
        // rejection, not the reason).
        auto minted = membership.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );
        const std::string inviteId = InviteIdFromUrl( minted.value() );
        ASSERT_FALSE( inviteId.empty() );

        chat::InviteRecord stored;
        ASSERT_TRUE( ReadInviteRecord( *m_rig.session, spaceId, inviteId, stored ) );
        ASSERT_GE( stored.wrapped_room_key_size(), 1 );
        stored.mutable_wrapped_room_key( 0 )->set_envelope( "tampered-envelope" );
        ASSERT_TRUE( WriteInviteRecord( *m_rig.session, spaceId, inviteId, stored ) );

        EXPECT_FALSE( joiner.RedeemInvite( minted.value() ).has_value() );

        // No rejected redeem wrote a member record.
        chat::MemberRecord joinerRecord;
        EXPECT_FALSE(
            ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, joinerRecord ) );
    }

    /**
     * @brief Test (T-04-15-05): the URL's scope segment selects the lookup key,
     *        so a link retargeted at another scope finds no record and writes
     *        nothing.
     */
    TEST_F( GcsMembershipTest, RedeemRejectsUrlScopeThatDoesNotOwnTheInvite )
    {
        StartRig( kTestCreatorAddress );

        auto first  = m_rig.store->CreateSpace( "first", true, true, true );
        auto second = m_rig.store->CreateSpace( "second", true, true, true );
        ASSERT_TRUE( first.has_value() );
        ASSERT_TRUE( second.has_value() );
        const std::string firstId  = first.value().id();
        const std::string secondId = second.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( firstId, "", true ).has_value() );
        ASSERT_TRUE( membership.OnScopeCreated( secondId, "", true ).has_value() );

        auto minted = membership.MintInvite( firstId );
        ASSERT_TRUE( minted.has_value() );
        const std::string inviteId = InviteIdFromUrl( minted.value() );
        ASSERT_FALSE( inviteId.empty() );
        const std::size_t keyPos = minted.value().find( "?key=" );
        ASSERT_NE( keyPos, std::string::npos );

        // Same invite key, but the URL names a scope that does not own the
        // record: the exact-key Get finds nothing.
        const std::string retargeted = std::string( "gcs://invite/space/" ) + secondId
                                       + "/" + inviteId
                                       + minted.value().substr( keyPos );

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                               CapturingSink( &events ), MakeSeam( kJoinerAddress ) );
        EXPECT_FALSE( joiner.RedeemInvite( retargeted ).has_value() );

        chat::MemberRecord record;
        EXPECT_FALSE( ReadMemberRecord( *m_rig.session, firstId, kJoinerAddress, record ) );
        EXPECT_FALSE( ReadMemberRecord( *m_rig.session, secondId, kJoinerAddress, record ) );
    }

    /**
     * @brief Test (WR-04 user decision): a space invite with NO child rooms
     *        carries zero wrapped room keys but is still bearer-gated — the
     *        correct key redeems, a wrong key writes nothing.
     */
    TEST_F( GcsMembershipTest, ChildlessSpaceInviteIsBearerGated )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "empty", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        auto minted = creator.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );

        // Zero wrapped room keys, yet a non-empty authenticator.
        auto invites = m_rig.session->QueryKeyValues( gcs::Membership::kInvitesKeyPrefix );
        ASSERT_TRUE( invites.has_value() );
        ASSERT_EQ( invites.value().size(), 1U );
        chat::InviteRecord invite;
        ASSERT_TRUE( invite.ParseFromString( invites.value().front().second ) );
        EXPECT_EQ( invite.wrapped_room_key_size(), 0 );
        EXPECT_FALSE( invite.authenticator().empty() );
        EXPECT_TRUE( invite.is_space() );

        // The minted bearer key redeems.
        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                               CapturingSink( &events ), MakeSeam( kJoinerAddress ) );
        ASSERT_TRUE( joiner.RedeemInvite( minted.value() ).has_value() );
        chat::MemberRecord joined;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, joined ) );
        EXPECT_EQ( joined.role(), chat::ROLE_GUEST );

        // A wrong bearer key (same URL, replaced key) is rejected and writes
        // nothing — the every-invite gate, not room-key unwrapping.
        const std::size_t queryPos = minted.value().find( "?key=" );
        ASSERT_NE( queryPos, std::string::npos );
        const std::string wrongKey( kEncodedKeyLength, 'B' );
        const std::string wrongUrl = minted.value().substr( 0, queryPos )
                                     + "?key=" + wrongKey;

        gcs::Membership joiner2( *m_rig.session, *m_rig.store, kJoiner2Address,
                                CapturingSink( &events ), MakeSeam( kJoiner2Address ) );
        EXPECT_FALSE( joiner2.RedeemInvite( wrongUrl ).has_value() );
        chat::MemberRecord rejected;
        EXPECT_FALSE(
            ReadMemberRecord( *m_rig.session, spaceId, kJoiner2Address, rejected ) );
    }

    /**
     * @brief Test (MEMB-01 fail-closed): without the issuer seam neither mint
     *        nor redeem may proceed — no unsigned record is ever written or
     *        accepted.
     */
    TEST_F( GcsMembershipTest, MintRejectedWhenIssuerSeamIsDisabled )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership noIssuer( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                  CapturingSink( &events ),
                                  MakeSeam( kTestCreatorAddress, /*issuerEnabled=*/false ) );
        // Fail closed: a scope cannot be created without a signer, so no
        // unsigned authority record is ever persisted.
        EXPECT_FALSE( noIssuer.OnScopeCreated( spaceId, "", true ).has_value() );

        const std::string creatorKey = std::string( gcs::Membership::kMembersKeyPrefix )
                                       + spaceId + "/" + kTestCreatorAddress;
        EXPECT_FALSE( m_rig.session->Get( creatorKey ).has_value() );

        EXPECT_FALSE( noIssuer.MintInvite( spaceId ).has_value() );

        auto invites = m_rig.session->QueryKeyValues(
            std::string( gcs::Membership::kInvitesKeyPrefix ) + spaceId + "/" );
        ASSERT_TRUE( invites.has_value() );
        EXPECT_TRUE( invites.value().empty() );

        // A genuinely minted invite is still rejected by the disabled seam.
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        auto minted = creator.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );

        auto baseline = m_rig.session->Get( creatorKey );
        ASSERT_TRUE( baseline.has_value() );

        EXPECT_FALSE( noIssuer.RedeemInvite( minted.value() ).has_value() );

        auto after = m_rig.session->Get( creatorKey );
        ASSERT_TRUE( after.has_value() );
        EXPECT_EQ( after.value(), baseline.value() );
    }

    /**
     * @brief Test (MEMB-01): a minted invite carries its issuer, its signature,
     *        and a scope-bearing URL.
     */
    TEST_F( GcsMembershipTest, MintStampsIssuerAndSignatureAndScopedUrl )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        auto minted = creator.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );
        const std::string expectedPrefix = "gcs://invite/space/" + spaceId + "/";
        EXPECT_EQ( minted.value().rfind( expectedPrefix, 0 ), 0 );

        const std::string inviteId = InviteIdFromUrl( minted.value() );
        ASSERT_FALSE( inviteId.empty() );

        chat::InviteRecord stored;
        ASSERT_TRUE( ReadInviteRecord( *m_rig.session, spaceId, inviteId, stored ) );
        EXPECT_EQ( stored.created_by(), kTestCreatorAddress );
        EXPECT_FALSE( stored.issuer_signature().empty() );
    }

    /**
     * @brief Test (T-04-15-01): a record whose role was raised after signing
     *        and then RE-SIGNED by the issuer is rejected by the D-05 GUEST
     *        role cap — the signature verifies, so the cap is what fires.
     */
    TEST_F( GcsMembershipTest, RedeemRejectsRecordWithTamperedRole )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        auto minted = creator.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );
        const std::string inviteId = InviteIdFromUrl( minted.value() );
        ASSERT_FALSE( inviteId.empty() );

        chat::InviteRecord stored;
        ASSERT_TRUE( ReadInviteRecord( *m_rig.session, spaceId, inviteId, stored ) );
        stored.set_role( chat::ROLE_SUPER_ADMIN );
        const std::string payload = gcs::test::BuildInviteIssuerPayload( inviteId, stored );
        stored.set_issuer_signature(
            TestIssuerSignatureHex( kTestCreatorAddress, payload ) );
        ASSERT_TRUE( WriteInviteRecord( *m_rig.session, spaceId, inviteId, stored ) );

        const std::size_t baselineCount = MemberScanCount( *m_rig.session, spaceId );

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                               CapturingSink( &events ), MakeSeam( kJoinerAddress ) );
        // The signature was recomputed by the creator, so it verifies; the
        // GUEST role cap is the rejection under test.
        EXPECT_FALSE( joiner.RedeemInvite( minted.value() ).has_value() );

        EXPECT_EQ( MemberScanCount( *m_rig.session, spaceId ), baselineCount );
        chat::MemberRecord joinerRecord;
        EXPECT_FALSE(
            ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, joinerRecord ) );
    }

    /**
     * @brief Test (T-04-15-02): a record signed by X but claiming
     *        created_by = Y fails — verification binds the CLAIMED issuer.
     */
    TEST_F( GcsMembershipTest, RedeemRejectsForgedIssuerIdentity )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        auto minted = creator.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );
        const std::string inviteId = InviteIdFromUrl( minted.value() );
        ASSERT_FALSE( inviteId.empty() );

        // Keep the creator's signature bytes but claim another identity.
        chat::InviteRecord stored;
        ASSERT_TRUE( ReadInviteRecord( *m_rig.session, spaceId, inviteId, stored ) );
        stored.set_created_by( kMemberAddress );
        ASSERT_TRUE( WriteInviteRecord( *m_rig.session, spaceId, inviteId, stored ) );

        const std::size_t baselineCount = MemberScanCount( *m_rig.session, spaceId );

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                               CapturingSink( &events ), MakeSeam( kJoinerAddress ) );
        EXPECT_FALSE( joiner.RedeemInvite( minted.value() ).has_value() );

        EXPECT_EQ( MemberScanCount( *m_rig.session, spaceId ), baselineCount );
        chat::MemberRecord joinerRecord;
        EXPECT_FALSE(
            ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, joinerRecord ) );
    }

    /**
     * @brief Test (MEMB-01 authority): a genuine invite minted by a
     *        membersCanInvite MEMBER whose membership is then revoked no longer
     *        redeems — the signature still verifies, so the AUTHORITY check is
     *        what rejects.
     */
    TEST_F( GcsMembershipTest, RedeemRejectsIssuerWhoseAuthorityWasRevoked )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true,
                                              /*membersCanInvite=*/true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> creatorEvents;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &creatorEvents ),
                                 MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        std::vector<chat::GcsEvent> issuerEvents;
        gcs::Membership issuer( *m_rig.session, *m_rig.store, kMemberAddress,
                                CapturingSink( &issuerEvents ),
                                MakeSeam( kMemberAddress ) );
        // Legitimate mint: the MEMBER holds invite authority via the gate.
        auto minted = issuer.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );

        // The creator is the ONLY admin (AdminCount 1) — immediate tombstone.
        ASSERT_TRUE( creator.RemoveMember( spaceId, kMemberAddress ).has_value() );
        chat::MemberRecord revoked;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kMemberAddress, revoked ) );
        ASSERT_TRUE( revoked.deleted() );

        const std::size_t baselineCount = MemberScanCount( *m_rig.session, spaceId );

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                               CapturingSink( &creatorEvents ),
                               MakeSeam( kJoinerAddress ) );
        // The signature still verifies against the claimed issuer; the issuer's
        // tombstoned record resolves to GUEST, so authority fails.
        EXPECT_FALSE( joiner.RedeemInvite( minted.value() ).has_value() );

        EXPECT_EQ( MemberScanCount( *m_rig.session, spaceId ), baselineCount );
        chat::MemberRecord joinerRecord;
        EXPECT_FALSE(
            ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, joinerRecord ) );
    }

    /**
     * @brief Test (T-04-15-08): an authorized membersCanInvite issuer re-signing
     *        a ROLE_SUPER_ADMIN record under its OWN identity is stopped by the
     *        GUEST role cap — signature and authority both pass.
     */
    TEST_F( GcsMembershipTest, RedeemRejectsSuperAdminGrantFromAuthorizedIssuer )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true,
                                              /*membersCanInvite=*/true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> creatorEvents;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &creatorEvents ),
                                 MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        std::vector<chat::GcsEvent> issuerEvents;
        gcs::Membership issuer( *m_rig.session, *m_rig.store, kMemberAddress,
                                CapturingSink( &issuerEvents ),
                                MakeSeam( kMemberAddress ) );
        auto minted = issuer.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );
        const std::string inviteId = InviteIdFromUrl( minted.value() );
        ASSERT_FALSE( inviteId.empty() );

        // The authorized Member raises the role and re-signs with its OWN
        // identity: created_by is unchanged and the authenticator is untouched.
        chat::InviteRecord stored;
        ASSERT_TRUE( ReadInviteRecord( *m_rig.session, spaceId, inviteId, stored ) );
        stored.set_role( chat::ROLE_SUPER_ADMIN );
        const std::string payload = gcs::test::BuildInviteIssuerPayload( inviteId, stored );
        stored.set_issuer_signature( TestIssuerSignatureHex( kMemberAddress, payload ) );
        ASSERT_TRUE( WriteInviteRecord( *m_rig.session, spaceId, inviteId, stored ) );

        const std::size_t baselineCount = MemberScanCount( *m_rig.session, spaceId );

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                               CapturingSink( &creatorEvents ),
                               MakeSeam( kJoinerAddress ) );
        // Signature verifies against created_by and CanInviteFor passes (the
        // gate is on), so ONLY the GUEST cap prevents a second Super Admin.
        EXPECT_FALSE( joiner.RedeemInvite( minted.value() ).has_value() );

        EXPECT_EQ( MemberScanCount( *m_rig.session, spaceId ), baselineCount );
        chat::MemberRecord joinerRecord;
        EXPECT_FALSE(
            ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, joinerRecord ) );
    }

    /**
     * @brief Test (T-04-15-07): wrapped room keys naming another scope are
     *        rejected even when the record is validly re-signed — the scope
     *        check, not the signature, is what fires.
     */
    TEST_F( GcsMembershipTest, RedeemRejectsWrappedRoomKeyOutsideScope )
    {
        StartRig( kTestCreatorAddress );

        auto roomA = m_rig.store->CreateRoom( "a", "", true );
        auto roomB = m_rig.store->CreateRoom( "b", "", true );
        ASSERT_TRUE( roomA.has_value() );
        ASSERT_TRUE( roomB.has_value() );
        const std::string roomAId = roomA.value().id();
        const std::string roomBId = roomB.value().id();
        const std::string roomATopic = std::string( kRoomTopicPrefix ) + roomAId;
        const std::string roomBTopic = std::string( kRoomTopicPrefix ) + roomBId;

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( roomAId, roomATopic, false ).has_value() );
        ASSERT_TRUE( creator.OnScopeCreated( roomBId, roomBTopic, false ).has_value() );

        auto minted = creator.MintInvite( roomAId );
        ASSERT_TRUE( minted.has_value() );
        const std::string inviteId = InviteIdFromUrl( minted.value() );
        ASSERT_FALSE( inviteId.empty() );

        // Append a wrapped key that belongs to room B and re-sign so the
        // signature verifies — the scope check must be what rejects.
        chat::InviteRecord stored;
        ASSERT_TRUE( ReadInviteRecord( *m_rig.session, roomAId, inviteId, stored ) );
        chat::WrappedRoomKey *foreign = stored.add_wrapped_room_key();
        foreign->set_room_id( roomBId );
        foreign->set_envelope( "not-a-real-envelope" );
        const std::string payload = gcs::test::BuildInviteIssuerPayload( inviteId, stored );
        stored.set_issuer_signature(
            TestIssuerSignatureHex( kTestCreatorAddress, payload ) );
        ASSERT_TRUE( WriteInviteRecord( *m_rig.session, roomAId, inviteId, stored ) );

        const auto roomBKeyBefore = gcs::crypto::RoomKeyFor( roomBTopic );

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                               CapturingSink( &events ), MakeSeam( kJoinerAddress ) );
        EXPECT_FALSE( joiner.RedeemInvite( minted.value() ).has_value() );

        chat::MemberRecord joinerRecord;
        EXPECT_FALSE(
            ReadMemberRecord( *m_rig.session, roomAId, kJoinerAddress, joinerRecord ) );
        // Room B's local key is untouched (it exists because OnScopeCreated
        // stored it, so compare BEFORE/AFTER rather than asserting absence).
        EXPECT_EQ( gcs::crypto::RoomKeyFor( roomBTopic ), roomBKeyBefore );

        // Space-invite twin: a space invite carrying its own scope as a room.
        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        auto spaceMinted = creator.MintInvite( spaceId );
        ASSERT_TRUE( spaceMinted.has_value() );
        const std::string spaceInviteId = InviteIdFromUrl( spaceMinted.value() );
        ASSERT_FALSE( spaceInviteId.empty() );

        chat::InviteRecord spaceRecord;
        ASSERT_TRUE(
            ReadInviteRecord( *m_rig.session, spaceId, spaceInviteId, spaceRecord ) );
        chat::WrappedRoomKey *selfRef = spaceRecord.add_wrapped_room_key();
        selfRef->set_room_id( spaceId );
        selfRef->set_envelope( "not-a-real-envelope" );
        const std::string spacePayload =
            gcs::test::BuildInviteIssuerPayload( spaceInviteId, spaceRecord );
        spaceRecord.set_issuer_signature(
            TestIssuerSignatureHex( kTestCreatorAddress, spacePayload ) );
        ASSERT_TRUE(
            WriteInviteRecord( *m_rig.session, spaceId, spaceInviteId, spaceRecord ) );

        gcs::Membership joiner2( *m_rig.session, *m_rig.store, kJoiner2Address,
                                CapturingSink( &events ), MakeSeam( kJoiner2Address ) );
        EXPECT_FALSE( joiner2.RedeemInvite( spaceMinted.value() ).has_value() );

        chat::MemberRecord spaceJoinerRecord;
        EXPECT_FALSE( ReadMemberRecord( *m_rig.session, spaceId, kJoiner2Address,
                                        spaceJoinerRecord ) );
    }

    /**
     * @brief Test 7 (MEMB-04): with 2+ admins a removal goes dormant (empty
     *        approved_by + pending_action, not tombstoned) and the Super Admin's
     *        Approve overwrites approved_by on the SAME key and tombstones it.
     */
    TEST_F( GcsMembershipTest, TwoAdminRemovalIsDormantUntilApproved )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        ASSERT_TRUE( membership.RemoveMember( spaceId, kTargetAddress ).has_value() );

        chat::MemberRecord dormant;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, dormant ) );
        EXPECT_FALSE( dormant.deleted() );                    // not applied yet
        EXPECT_TRUE( dormant.approved_by().empty() );         // dormant (D-07)
        EXPECT_EQ( dormant.pending_action(), chat::PENDING_ACTION_REMOVE );

        ASSERT_TRUE( membership.Approve( spaceId, kTargetAddress ).has_value() );
        chat::MemberRecord approved;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, approved ) );
        EXPECT_TRUE( approved.deleted() );
        EXPECT_EQ( approved.approved_by(), kTestCreatorAddress ); // same-key overwrite
        EXPECT_EQ( approved.created_at_ms(), kSeedCreatedAtMs );

        // A guest cannot activate the dormant op (Super Admin only).
        SeedMember( spaceId, kGuestAddress, chat::ROLE_GUEST );
        gcs::Membership guest( *m_rig.session, *m_rig.store, kGuestAddress,
                               CapturingSink( &events ), MakeSeam( kGuestAddress ) );
        EXPECT_FALSE( guest.Approve( spaceId, kAdminAddress ).has_value() );
    }

    /**
     * @brief Test (CR-01): redeeming an invite must not cancel a dormant
     *        removal or rewrite the stored role — only key material changes.
     *
     * A live member with a pending removal keeps role, approved_by,
     * pending_action, and pending_role across a redeem; the pending op remains
     * activatable by the Super Admin's Approve.
     */
    TEST_F( GcsMembershipTest, DormantRemovalSurvivesRedeem )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, false );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        auto minted = creator.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );

        // Two admins (creator + kAdminAddress): the removal goes dormant.
        ASSERT_TRUE( creator.RemoveMember( spaceId, kTargetAddress ).has_value() );
        chat::MemberRecord dormant;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, dormant ) );
        ASSERT_EQ( dormant.pending_action(), chat::PENDING_ACTION_REMOVE );
        ASSERT_FALSE( dormant.deleted() );
        ASSERT_TRUE( dormant.approved_by().empty() );

        // The target redeems the invite: the stored role and dormant state must
        // survive; only the carried key material may change.
        gcs::Membership target( *m_rig.session, *m_rig.store, kTargetAddress,
                                CapturingSink( &events ), MakeSeam( kTargetAddress ) );
        ASSERT_TRUE( target.RedeemInvite( minted.value() ).has_value() );

        chat::MemberRecord redeemed;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, redeemed ) );
        EXPECT_EQ( redeemed.role(), chat::ROLE_MEMBER ); // not the invite's Guest
        EXPECT_EQ( redeemed.pending_action(), chat::PENDING_ACTION_REMOVE );
        EXPECT_TRUE( redeemed.approved_by().empty() );
        EXPECT_FALSE( redeemed.deleted() );

        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        const chat::MemberRecord *targetEntry = FindMember( list.value(), kTargetAddress );
        ASSERT_NE( targetEntry, nullptr );
        EXPECT_EQ( targetEntry->role(), chat::ROLE_MEMBER );

        // The dormant removal is still activatable after the redeem.
        ASSERT_TRUE( creator.Approve( spaceId, kTargetAddress ).has_value() );
        chat::MemberRecord approved;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, approved ) );
        EXPECT_TRUE( approved.deleted() );
    }

    /**
     * @brief Test (CR-02): a tombstoned wallet cannot rejoin by redeeming a
     *        stale invite — the rejection precedes the unwrap loop, so neither
     *        the stored record nor the roster changes at all.
     */
    TEST_F( GcsMembershipTest, TombstonedMemberCannotRedeemStaleInvite )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, false );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kJoinerAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Mint BEFORE the removal so a stale link exists at redeem time.
        auto minted = creator.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );

        // Sole admin (the creator): the removal tombstones immediately.
        ASSERT_TRUE( creator.RemoveMember( spaceId, kJoinerAddress ).has_value() );
        chat::MemberRecord removed;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, removed ) );
        ASSERT_TRUE( removed.deleted() );
        ASSERT_EQ( removed.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );

        // Baseline: stored bytes and scan count captured BEFORE the redeem.
        const std::string memberKey = std::string( gcs::Membership::kMembersKeyPrefix )
                                      + spaceId + "/" + kJoinerAddress;
        const std::string memberPrefix = std::string( gcs::Membership::kMembersKeyPrefix )
                                         + spaceId + "/";
        auto baselineBytes = m_rig.session->Get( memberKey );
        ASSERT_TRUE( baselineBytes.has_value() );
        const std::string baselineRecord = baselineBytes.value();
        auto baselineScan = m_rig.session->QueryKeyValues( memberPrefix );
        ASSERT_TRUE( baselineScan.has_value() );
        const std::size_t baselineCount = baselineScan.value().size();

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                                CapturingSink( &events ), MakeSeam( kJoinerAddress ) );
        EXPECT_FALSE( joiner.RedeemInvite( minted.value() ).has_value() );

        // A rejected redeem writes nothing at all.
        auto afterBytes = m_rig.session->Get( memberKey );
        ASSERT_TRUE( afterBytes.has_value() );
        EXPECT_EQ( afterBytes.value(), baselineRecord );
        auto afterScan = m_rig.session->QueryKeyValues( memberPrefix );
        ASSERT_TRUE( afterScan.has_value() );
        EXPECT_EQ( afterScan.value().size(), baselineCount );

        chat::MemberRecord stillRemoved;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress,
                                       stillRemoved ) );
        EXPECT_TRUE( stillRemoved.deleted() );

        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        EXPECT_EQ( FindMember( list.value(), kJoinerAddress ), nullptr );
    }

    /**
     * @brief Test 8 (MEMB-04/D-08): with a single admin a removal applies
     *        immediately, while a Guest's every mutator is rejected read-only.
     */
    TEST_F( GcsMembershipTest, SoleAdminRemovesImmediatelyAndGuestIsReadOnly )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );
        SeedMember( spaceId, kGuestAddress, chat::ROLE_GUEST );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Sole admin (the creator): the removal applies at once.
        ASSERT_TRUE( creator.RemoveMember( spaceId, kTargetAddress ).has_value() );
        chat::MemberRecord removed;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, removed ) );
        EXPECT_TRUE( removed.deleted() );
        EXPECT_EQ( removed.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );

        // Guest: read-only across every mutator (D-08).
        gcs::Membership guest( *m_rig.session, *m_rig.store, kGuestAddress,
                               CapturingSink( &events ), MakeSeam( kGuestAddress ) );
        EXPECT_FALSE( guest.ChangeRole( spaceId, kTargetAddress, chat::ROLE_MEMBER ).has_value() );
        EXPECT_FALSE( guest.RemoveMember( spaceId, kTargetAddress ).has_value() );
        EXPECT_FALSE( guest.DeleteScope( spaceId ).has_value() );
        EXPECT_FALSE( guest.MintInvite( spaceId ).has_value() );
        // Leave is self-service (D-02), not a privileged mutator: only the
        // Super Admin is barred from leaving its own scope.
        EXPECT_TRUE( guest.Leave( spaceId ).has_value() );
        EXPECT_EQ( guest.LocalRoleFor( spaceId ), chat::ROLE_GUEST );

        // The space survived the rejected deletion.
        EXPECT_EQ( m_rig.store->Spaces().size(), 1U );
    }

    /**
     * @brief Test 9 (D-03/D-07): MembersFor skips tombstoned records, a
     *        full-state rewrite preserves created_at_ms, and Leave writes
     *        explicit_leave on the caller's own record without a tombstone.
     */
    TEST_F( GcsMembershipTest, TombstoneSkipAndLeaveWriteExplicitLeave )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );
        SeedMember( spaceId, kDeadAddress, chat::ROLE_MEMBER, kSeedCreatedAtMs, true );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        auto list = membership.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        ASSERT_EQ( list.value().member_size(), 3 ); // creator + target + member (dead skipped)
        EXPECT_EQ( FindMember( list.value(), kDeadAddress ), nullptr );

        // Full-state rewrite preserves the immutable created_at_ms stamp.
        ASSERT_TRUE( membership.ChangeRole( spaceId, kTargetAddress, chat::ROLE_MODERATOR ).has_value() );
        chat::MemberRecord rewritten;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, rewritten ) );
        EXPECT_EQ( rewritten.created_at_ms(), kSeedCreatedAtMs );
        EXPECT_GT( rewritten.updated_at_ms(), 0 );

        // D-02: explicit leave is a bool on the caller's OWN record (no tombstone).
        gcs::Membership memberSelf( *m_rig.session, *m_rig.store, kMemberAddress,
                                    CapturingSink( &events ), MakeSeam( kMemberAddress ) );
        ASSERT_TRUE( memberSelf.Leave( spaceId ).has_value() );
        EXPECT_TRUE( memberSelf.HasExplicitLeave( spaceId ) );
        chat::MemberRecord left;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kMemberAddress, left ) );
        EXPECT_TRUE( left.explicit_leave() );
        EXPECT_FALSE( left.deleted() );
        EXPECT_EQ( left.role(), chat::ROLE_MEMBER ); // role untouched by leave

        // The Super Admin cannot leave its own scope (guard rejects).
        EXPECT_FALSE( membership.Leave( spaceId ).has_value() );
    }

    /**
     * @brief Test 10 (D-03/D-07): DeleteScope tombstones the entity with a sole
     *        admin, while a remote member-list heal delivery is pushed through
     *        OnMemberArrived.
     */
    TEST_F( GcsMembershipTest, DeleteScopeTombstonesEntityAndOnMemberArrivedPushes )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        // Heal delivery: the callback derives the scope, verifies the record,
        // and pushes the roster. The delivery is a creator-signed record,
        // exactly as a remote node would replicate it.
        chat::MemberRecord arrived;
        arrived.set_scope_id( spaceId );
        arrived.set_address( kMemberAddress );
        arrived.set_role( chat::ROLE_MEMBER );
        arrived.set_created_by( kTestCreatorAddress );
        arrived.set_prior_role( chat::ROLE_UNSPECIFIED );
        arrived.set_created_at_ms( kSeedCreatedAtMs );
        arrived.set_updated_at_ms( kSeedCreatedAtMs );
        arrived.set_record_signature( TestMemberSignatureHex(
            kTestCreatorAddress, spaceId, kMemberAddress, arrived ) );
        const std::size_t beforeCount = events.size();
        membership.OnMemberArrived(
            "/crdt/k" + std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/"
                + kMemberAddress + "/v",
            arrived.SerializeAsString() );
        ASSERT_GT( events.size(), beforeCount );
        EXPECT_TRUE( events.back().has_member_list() );
        EXPECT_EQ( events.back().member_list().scope_id(), spaceId );

        // WR-05: a tombstoned heal delivery still pushes the roster so a remote
        // removal converges — the pushed list must exclude the dead member.
        // A tombstone preserves the role and changes no predecessor.
        chat::MemberRecord deadArrived = arrived;
        deadArrived.set_address( kDeadAddress );
        deadArrived.set_deleted( true );
        deadArrived.set_prior_role( chat::ROLE_MEMBER );
        deadArrived.set_record_signature( TestMemberSignatureHex(
            kTestCreatorAddress, spaceId, kDeadAddress, deadArrived ) );
        const std::size_t afterLive = events.size();
        membership.OnMemberArrived(
            std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/" + kDeadAddress,
            deadArrived.SerializeAsString() );
        ASSERT_GT( events.size(), afterLive );
        EXPECT_TRUE( events.back().has_member_list() );
        EXPECT_EQ( events.back().member_list().scope_id(), spaceId );
        EXPECT_EQ( FindMember( events.back().member_list(), kDeadAddress ), nullptr );

        // Sole admin: DeleteScope tombstones the entity record through the
        // catalog mutator, so the catalog is the source of the push and no
        // stale live entry survives the call (CR-06).
        ASSERT_TRUE( membership.DeleteScope( spaceId ).has_value() );
        EXPECT_TRUE( m_rig.store->Spaces().empty() ); // id no longer live
        EXPECT_TRUE( m_rig.store->Spaces().empty() ); // still excluded on re-read
        const auto tombBytes = m_rig.session->Get( std::string( kSpacesKeyPrefix ) + spaceId );
        ASSERT_TRUE( tombBytes.has_value() );
        chat::SpaceRecord tomb;
        ASSERT_TRUE( tomb.ParseFromString( tombBytes.value() ) );
        EXPECT_TRUE( tomb.deleted() );
        EXPECT_GT( tomb.deleted_at_ms(), 0 );
    }

    /**
     * @brief CR-06 (VERIFICATION truth 11): a dormant scope delete is visible
     *        in the catalog, so Approve's entity branch is reachable and the
     *        pushed tree carries the tombstone rather than a stale live scope.
     */
    TEST_F( GcsMembershipTest, DeleteScopeDormantThenApproved )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Two admins: the delete is dormant, not applied.
        ASSERT_TRUE( creator.DeleteScope( spaceId ).has_value() );

        auto dormantBytes = m_rig.session->Get( std::string( kSpacesKeyPrefix ) + spaceId );
        ASSERT_TRUE( dormantBytes.has_value() );
        chat::SpaceRecord dormant;
        ASSERT_TRUE( dormant.ParseFromString( dormantBytes.value() ) );
        EXPECT_FALSE( dormant.deleted() );
        EXPECT_TRUE( dormant.approved_by().empty() );
        EXPECT_EQ( dormant.pending_action(), chat::PENDING_ACTION_DELETE );

        // The catalog reflects the pending delete: the pushed tree carries the
        // marker (pre-fix the stale live record was re-emitted).
        ASSERT_FALSE( events.empty() );
        ASSERT_TRUE( events.back().has_space_tree() );
        ASSERT_EQ( events.back().space_tree().space_size(), 1 );
        EXPECT_EQ( events.back().space_tree().space( 0 ).id(), spaceId );
        EXPECT_EQ( events.back().space_tree().space( 0 ).pending_action(),
                   chat::PENDING_ACTION_DELETE );

        // The entity branch is reachable because the catalog holds the dormant
        // record rather than the pre-delete live one.
        ASSERT_TRUE( creator.Approve( spaceId, spaceId ).has_value() );
        EXPECT_TRUE( m_rig.store->Spaces().empty() );

        auto approvedBytes = m_rig.session->Get( std::string( kSpacesKeyPrefix ) + spaceId );
        ASSERT_TRUE( approvedBytes.has_value() );
        chat::SpaceRecord approved;
        ASSERT_TRUE( approved.ParseFromString( approvedBytes.value() ) );
        EXPECT_TRUE( approved.deleted() );
        EXPECT_EQ( approved.approved_by(), kTestCreatorAddress );

        // The push is built from the catalog the mutator just updated.
        ASSERT_TRUE( events.back().has_space_tree() );
        EXPECT_EQ( events.back().space_tree().space_size(), 0 );
    }

    /**
     * @brief D-07: Approve rejects an entity target with no dormant DELETE and
     *        writes nothing.
     */
    TEST_F( GcsMembershipTest, ApproveWithoutDormantEntityDeleteIsRejected )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        const auto beforeBytes = m_rig.session->Get( std::string( kSpacesKeyPrefix ) + spaceId );
        ASSERT_TRUE( beforeBytes.has_value() );

        EXPECT_FALSE( creator.Approve( spaceId, spaceId ).has_value() );

        const auto afterBytes = m_rig.session->Get( std::string( kSpacesKeyPrefix ) + spaceId );
        ASSERT_TRUE( afterBytes.has_value() );
        EXPECT_EQ( afterBytes.value(), beforeBytes.value() );
    }

    /**
     * @brief CR-05: a failed member scan fails closed — RemoveMember,
     *        ChangeRole, and DeleteScope abort instead of interpreting the
     *        failure as "fewer than two admins" and writing the immediate
     *        destructive branch.
     */
    TEST_F( GcsMembershipTest, AdminCountScanFailurePropagatesAndFailsClosed )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        gcs::Membership::ScanFn failingScan =
            []( const std::string & )
            -> outcome::result<std::vector<std::pair<std::string, std::string>>> {
            return outcome::failure( sgns::gcs::Error::GcsDbError );
        };
        gcs::Membership failing( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ), failingScan );

        const std::string targetKey = std::string( gcs::Membership::kMembersKeyPrefix )
                                      + spaceId + "/" + kTargetAddress;
        const std::string adminKey = std::string( gcs::Membership::kMembersKeyPrefix )
                                     + spaceId + "/" + kAdminAddress;
        const std::string spaceKey = std::string( kSpacesKeyPrefix ) + spaceId;

        const auto targetBefore = m_rig.session->Get( targetKey );
        ASSERT_TRUE( targetBefore.has_value() );
        const auto adminBefore = m_rig.session->Get( adminKey );
        ASSERT_TRUE( adminBefore.has_value() );
        const auto spaceBefore = m_rig.session->Get( spaceKey );
        ASSERT_TRUE( spaceBefore.has_value() );

        // Every destructive call must propagate the scan error, never write.
        EXPECT_FALSE( failing.RemoveMember( spaceId, kTargetAddress ).has_value() );
        const auto targetAfter = m_rig.session->Get( targetKey );
        ASSERT_TRUE( targetAfter.has_value() );
        EXPECT_EQ( targetAfter.value(), targetBefore.value() );

        EXPECT_FALSE( failing.ChangeRole( spaceId, kAdminAddress, chat::ROLE_MEMBER ).has_value() );
        const auto adminAfter = m_rig.session->Get( adminKey );
        ASSERT_TRUE( adminAfter.has_value() );
        EXPECT_EQ( adminAfter.value(), adminBefore.value() );

        EXPECT_FALSE( failing.DeleteScope( spaceId ).has_value() );
        const auto spaceAfter = m_rig.session->Get( spaceKey );
        ASSERT_TRUE( spaceAfter.has_value() );
        EXPECT_EQ( spaceAfter.value(), spaceBefore.value() );
        EXPECT_FALSE( m_rig.store->Spaces().empty() ); // immediate branch never ran
    }

    /**
     * @brief Test (WR-02/D-08): a role governs which members its holder may act
     *        on — a Moderator cannot remove an Admin, while a Moderator may
     *        remove a Member. A creator-signed SUPER_ADMIN grant to another
     *        wallet is itself invalid, so the record is unverifiable and grants
     *        nothing.
     */
    TEST_F( GcsMembershipTest, ModeratorCannotRemoveAdminAndAnUnverifiableSuperAdminGrantsNothing )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kModeratorAddress, chat::ROLE_MODERATOR );
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Rank 3 > 2: a Moderator may remove a plain Member.
        gcs::Membership moderator( *m_rig.session, *m_rig.store, kModeratorAddress,
                                   CapturingSink( &events ), MakeSeam( kModeratorAddress ) );
        ASSERT_TRUE( moderator.RemoveMember( spaceId, kMemberAddress ).has_value() );

        // Rank 3 is not > 4: a Moderator may NOT remove an Admin.
        EXPECT_FALSE( moderator.RemoveMember( spaceId, kAdminAddress ).has_value() );
        chat::MemberRecord adminRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kAdminAddress, adminRecord ) );
        EXPECT_EQ( adminRecord.role(), chat::ROLE_ADMIN );
        EXPECT_FALSE( adminRecord.deleted() );
        EXPECT_EQ( adminRecord.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );

        // A second, non-creator SUPER_ADMIN target. The creator-signed grant
        // is invalid by design (only the scope root may be Super Admin), so the
        // record is unverifiable and is not a member.
        SeedMember( spaceId, kTargetAddress, chat::ROLE_SUPER_ADMIN );
        gcs::Membership admin( *m_rig.session, *m_rig.store, kAdminAddress,
                               CapturingSink( &events ), MakeSeam( kAdminAddress ) );

        EXPECT_FALSE( admin.RemoveMember( spaceId, kTargetAddress ).has_value() );
        EXPECT_FALSE( admin.ChangeRole( spaceId, kTargetAddress, chat::ROLE_ADMIN ).has_value() );

        // The rejected calls wrote nothing: the raw record is unchanged.
        chat::MemberRecord superAdminRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress,
                                       superAdminRecord ) );
        EXPECT_FALSE( superAdminRecord.deleted() );
        EXPECT_EQ( superAdminRecord.role(), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( superAdminRecord.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
        EXPECT_EQ( superAdminRecord.pending_role(), chat::ROLE_UNSPECIFIED );
        EXPECT_TRUE( superAdminRecord.approved_by().empty() );

        // An unverifiable record is not a member: excluded from the roster.
        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        EXPECT_EQ( FindMember( list.value(), kTargetAddress ), nullptr );
    }

    /**
     * @brief Test (CR-04/T-04-17-02): a Moderator may not initiate a role
     *        change against an equal- or higher-ranked target, while a plain
     *        Member remains manageable.
     */
    TEST_F( GcsMembershipTest, ModeratorCannotInitiateHighRankRoleChange )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kModeratorAddress, chat::ROLE_MODERATOR );
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MODERATOR );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        gcs::Membership moderator( *m_rig.session, *m_rig.store, kModeratorAddress,
                                   CapturingSink( &events ), MakeSeam( kModeratorAddress ) );

        // Rank 3 is not > 4: a Moderator cannot demote an Admin.
        EXPECT_FALSE( moderator.ChangeRole( spaceId, kAdminAddress, chat::ROLE_MEMBER ).has_value() );
        chat::MemberRecord adminRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kAdminAddress, adminRecord ) );
        EXPECT_EQ( adminRecord.role(), chat::ROLE_ADMIN );
        EXPECT_EQ( adminRecord.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
        EXPECT_EQ( adminRecord.pending_role(), chat::ROLE_UNSPECIFIED );

        // Rank 3 is not > 3: equal rank is rejected too.
        EXPECT_FALSE( moderator.ChangeRole( spaceId, kTargetAddress, chat::ROLE_MEMBER ).has_value() );
        chat::MemberRecord peerRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, peerRecord ) );
        EXPECT_EQ( peerRecord.role(), chat::ROLE_MODERATOR );
        EXPECT_EQ( peerRecord.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );

        // Positive control: rank 3 > 2 — a plain Member is manageable.
        ASSERT_TRUE( moderator.ChangeRole( spaceId, kMemberAddress, chat::ROLE_GUEST ).has_value() );
        chat::MemberRecord memberRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kMemberAddress, memberRecord ) );
        EXPECT_EQ( memberRecord.role(), chat::ROLE_GUEST );
    }

    /**
     * @brief Test (CR-03/T-04-17-01/T-04-17-04): no author may grant
     *        ROLE_SUPER_ADMIN or write ROLE_UNSPECIFIED — including the creator
     *        on itself.
     */
    TEST_F( GcsMembershipTest, ChangeRoleRejectsSuperAdminGrantAndUnspecifiedRole )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // A Super Admin grant would create a second authority root.
        EXPECT_FALSE( creator.ChangeRole( spaceId, kMemberAddress, chat::ROLE_SUPER_ADMIN ).has_value() );
        // ROLE_UNSPECIFIED sorts below Guest; it is never written.
        EXPECT_FALSE( creator.ChangeRole( spaceId, kMemberAddress, chat::ROLE_UNSPECIFIED ).has_value() );

        chat::MemberRecord memberRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kMemberAddress, memberRecord ) );
        EXPECT_EQ( memberRecord.role(), chat::ROLE_MEMBER );
        EXPECT_EQ( memberRecord.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
        EXPECT_EQ( memberRecord.pending_role(), chat::ROLE_UNSPECIFIED );
        EXPECT_TRUE( memberRecord.approved_by().empty() );

        // The creator is not self-promotable; nothing changes.
        EXPECT_FALSE( creator.ChangeRole( spaceId, kTestCreatorAddress, chat::ROLE_SUPER_ADMIN ).has_value() );
        chat::MemberRecord creatorRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTestCreatorAddress,
                                       creatorRecord ) );
        EXPECT_EQ( creatorRecord.role(), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( creatorRecord.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
    }

    /**
     * @brief Test (CR-04/T-04-17-03): an Admin cannot act on a non-creator
     *        SUPER_ADMIN record that the trust chain cannot verify — the
     *        attempt is rejected before any write or roster push.
     */
    TEST_F( GcsMembershipTest, AdminCannotActOnAnUnverifiableSuperAdminRecord )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_SUPER_ADMIN );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        gcs::Membership admin( *m_rig.session, *m_rig.store, kAdminAddress,
                               CapturingSink( &events ), MakeSeam( kAdminAddress ) );
        const std::size_t pushBaseline = events.size();

        EXPECT_FALSE( admin.ChangeRole( spaceId, kTargetAddress, chat::ROLE_ADMIN ).has_value() );

        chat::MemberRecord record;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, record ) );
        EXPECT_EQ( record.role(), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( record.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
        EXPECT_EQ( record.pending_role(), chat::ROLE_UNSPECIFIED );
        EXPECT_TRUE( record.approved_by().empty() );
        EXPECT_FALSE( record.deleted() );

        // An unverifiable record is excluded from the roster.
        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        EXPECT_EQ( FindMember( list.value(), kTargetAddress ), nullptr );

        // A rejected change emits no roster push.
        EXPECT_EQ( events.size(), pushBaseline );
    }

    /**
     * @brief Test (WR-03): demoting an Admin with 2+ admins writes a dormant
     *        ROLE_CHANGE record instead of applying it.
     */
    TEST_F( GcsMembershipTest, AdminDemotionIsDormantWhenTwoAdmins )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kTargetAddress, chat::ROLE_ADMIN );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        // Two admins (creator + target) -> the demotion is dormant, not applied.
        ASSERT_TRUE( membership.ChangeRole( spaceId, kTargetAddress, chat::ROLE_MODERATOR ).has_value() );

        chat::MemberRecord record;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, record ) );
        EXPECT_EQ( record.role(), chat::ROLE_ADMIN );
        EXPECT_TRUE( record.approved_by().empty() );
        EXPECT_EQ( record.pending_action(), chat::PENDING_ACTION_ROLE_CHANGE );
        EXPECT_EQ( record.pending_role(), chat::ROLE_MODERATOR );
    }

    /**
     * @brief Test (WR-03 boundary): a Member -> Guest demotion is not in the
     *        D-07 destructive set, so it applies even with 2+ admins.
     */
    TEST_F( GcsMembershipTest, MemberDemotionIsNotDormant )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        ASSERT_TRUE( membership.ChangeRole( spaceId, kTargetAddress, chat::ROLE_GUEST ).has_value() );

        chat::MemberRecord record;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, record ) );
        EXPECT_EQ( record.role(), chat::ROLE_GUEST );
        EXPECT_EQ( record.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
        EXPECT_EQ( record.pending_role(), chat::ROLE_UNSPECIFIED );
    }

    /**
     * @brief Truth 29: a self-signed SUPER_ADMIN record is excluded from the
     *        roster, resolves the local role to GUEST, and grants no command.
     */
    TEST_F( GcsMembershipTest, SelfSignedSuperAdminRecordIsExcludedAndGrantsNothing )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_SUPER_ADMIN, kSeedCreatedAtMs, false,
                    kTargetAddress );

        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        ASSERT_EQ( list.value().member_size(), 2 );
        EXPECT_NE( FindMember( list.value(), kTestCreatorAddress ), nullptr );
        EXPECT_NE( FindMember( list.value(), kMemberAddress ), nullptr );
        EXPECT_EQ( FindMember( list.value(), kTargetAddress ), nullptr );

        gcs::Membership attacker( *m_rig.session, *m_rig.store, kTargetAddress,
                                  CapturingSink( &events ), MakeSeam( kTargetAddress ) );
        EXPECT_EQ( attacker.LocalRoleFor( spaceId ), chat::ROLE_GUEST );
        EXPECT_FALSE( attacker.DeleteScope( spaceId ).has_value() );
        EXPECT_FALSE( attacker.RemoveMember( spaceId, kMemberAddress ).has_value() );
        EXPECT_FALSE( attacker.ChangeRole( spaceId, kMemberAddress, chat::ROLE_GUEST ).has_value() );

        // Rejection, never deletion: the forged record stays in the CRDT.
        chat::MemberRecord forged;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, forged ) );
        EXPECT_EQ( forged.role(), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( forged.created_by(), kTargetAddress );
    }

    /**
     * @brief Truth 29/D-05: a self-signed record cannot assert MEMBER or above,
     *        a tombstone, a pending action, or a claimed prior role.
     */
    TEST_F( GcsMembershipTest, SelfSignedRecordCannotExceedGuestOrCarryTombstoneOrPending )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        // (a) A self-signed MEMBER grant.
        SeedMember( spaceId, kAdminAddress, chat::ROLE_MEMBER, kSeedCreatedAtMs, false,
                    kAdminAddress );
        // (b) A self-signed GUEST tombstone.
        SeedMember( spaceId, kModeratorAddress, chat::ROLE_GUEST, kSeedCreatedAtMs, true,
                    kModeratorAddress );
        // (c) A self-signed GUEST carrying a pending removal.
        SeedMember( spaceId, kGuestAddress, chat::ROLE_GUEST, kSeedCreatedAtMs, false,
                    kGuestAddress );
        chat::MemberRecord pendingRemove;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kGuestAddress, pendingRemove ) );
        pendingRemove.set_pending_action( chat::PENDING_ACTION_REMOVE );
        pendingRemove.set_prior_role( chat::ROLE_GUEST );
        pendingRemove.set_record_signature( TestMemberSignatureHex(
            kGuestAddress, spaceId, kGuestAddress, pendingRemove ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kGuestAddress,
                                     pendingRemove.SerializeAsString() ) );
        // (d) A self-signed GUEST claiming a prior role.
        SeedMember( spaceId, kTargetAddress, chat::ROLE_GUEST, kSeedCreatedAtMs, false,
                    kTargetAddress );
        chat::MemberRecord claimedPrior;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, claimedPrior ) );
        claimedPrior.set_prior_role( chat::ROLE_MODERATOR );
        claimedPrior.set_record_signature( TestMemberSignatureHex(
            kTargetAddress, spaceId, kTargetAddress, claimedPrior ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     claimedPrior.SerializeAsString() ) );

        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        ASSERT_EQ( list.value().member_size(), 2 );
        EXPECT_NE( FindMember( list.value(), kTestCreatorAddress ), nullptr );
        EXPECT_NE( FindMember( list.value(), kMemberAddress ), nullptr );
        EXPECT_EQ( FindMember( list.value(), kAdminAddress ), nullptr );
        EXPECT_EQ( FindMember( list.value(), kModeratorAddress ), nullptr );
        EXPECT_EQ( FindMember( list.value(), kGuestAddress ), nullptr );
        EXPECT_EQ( FindMember( list.value(), kTargetAddress ), nullptr );

        // All six records remain in the raw store.
        EXPECT_EQ( MemberScanCount( *m_rig.session, spaceId ), 6U );
    }

    /**
     * @brief Truth 29: a tampered role without a fresh signature is excluded.
     */
    TEST_F( GcsMembershipTest, TamperedMemberSignatureIsExcluded )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );

        const std::string targetKey = std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId
                                      + "/" + kTargetAddress;
        chat::MemberRecord tampered;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, tampered ) );
        tampered.set_role( chat::ROLE_ADMIN ); // no re-sign
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     tampered.SerializeAsString() ) );

        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        EXPECT_EQ( FindMember( list.value(), kTargetAddress ), nullptr );

        gcs::Membership targetMembership( *m_rig.session, *m_rig.store, kTargetAddress,
                                          CapturingSink( &events ), MakeSeam( kTargetAddress ) );
        EXPECT_EQ( targetMembership.LocalRoleFor( spaceId ), chat::ROLE_GUEST );

        // The rejected bytes are still the winning LWW value (never deleted).
        auto raw = m_rig.session->Get( targetKey );
        ASSERT_TRUE( raw.has_value() );
        EXPECT_EQ( raw.value(), tampered.SerializeAsString() );
    }

    /**
     * @brief Truth 29: the signed timestamp is the write-ordering input, so a
     *        re-dated record without a fresh signature is excluded.
     */
    TEST_F( GcsMembershipTest, TamperedUpdatedAtMsIsExcluded )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );

        const std::string targetKey = std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId
                                      + "/" + kTargetAddress;
        chat::MemberRecord tampered;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, tampered ) );
        tampered.set_updated_at_ms( tampered.updated_at_ms() + 1000 ); // no re-sign
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     tampered.SerializeAsString() ) );

        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        EXPECT_EQ( FindMember( list.value(), kTargetAddress ), nullptr );

        auto raw = m_rig.session->Get( targetKey );
        ASSERT_TRUE( raw.has_value() );
        EXPECT_EQ( raw.value(), tampered.SerializeAsString() );
    }

    /**
     * @brief Truth 5: a fresh third-party grant requires the creator, and the
     *        grant matrix requires a sufficient signer rank.
     */
    TEST_F( GcsMembershipTest, InsufficientSignerCannotGrantAnotherWalletARole )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );
        // A MEMBER signer cannot make the creator's fresh grant.
        SeedMember( spaceId, kTargetAddress, chat::ROLE_ADMIN, kSeedCreatedAtMs, false,
                    kMemberAddress );
        // Independently, a MEMBER signer cannot grant MEMBER rank.
        SeedMember( spaceId, kGuestAddress, chat::ROLE_MEMBER, kSeedCreatedAtMs, false,
                    kMemberAddress );

        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        ASSERT_EQ( list.value().member_size(), 2 );
        EXPECT_NE( FindMember( list.value(), kTestCreatorAddress ), nullptr );
        EXPECT_NE( FindMember( list.value(), kMemberAddress ), nullptr );
        EXPECT_EQ( FindMember( list.value(), kTargetAddress ), nullptr );
        EXPECT_EQ( FindMember( list.value(), kGuestAddress ), nullptr );

        EXPECT_EQ( MemberScanCount( *m_rig.session, spaceId ), 4U );
    }

    /**
     * @brief Truth 5/D-03: no third-party record may touch the creator. The
     *        hostile record wins the key (LWW) but never gains authority, and
     *        the creator's verified root returns when the saved bytes are
     *        restored.
     */
    TEST_F( GcsMembershipTest, CreatorTargetIsRejectedFromEveryThirdPartyRecord )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MODERATOR );

        const std::string creatorKey = std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId
                                       + "/" + kTestCreatorAddress;
        const auto saved = m_rig.session->Get( creatorKey );
        ASSERT_TRUE( saved.has_value() );
        chat::MemberRecord savedRecord;
        ASSERT_TRUE( savedRecord.ParseFromString( saved.value() ) );

        chat::MemberRecord forged = savedRecord;
        forged.set_role( chat::ROLE_GUEST );
        forged.set_created_by( kMemberAddress );
        forged.set_prior_role( chat::ROLE_SUPER_ADMIN );
        forged.set_updated_at_ms( savedRecord.updated_at_ms() + 1 );
        forged.set_record_signature( TestMemberSignatureHex(
            kMemberAddress, spaceId, kTestCreatorAddress, forged ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTestCreatorAddress,
                                     forged.SerializeAsString() ) );

        auto hostile = m_rig.session->Get( creatorKey );
        ASSERT_TRUE( hostile.has_value() );
        EXPECT_EQ( hostile.value(), forged.SerializeAsString() );

        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        EXPECT_EQ( FindMember( list.value(), kTestCreatorAddress ), nullptr );

        // The attacker holds MODERATOR but never gains creator authority.
        // The forged root is unverifiable, so EVERY creator-rooted chain fails
        // closed: the attacker loses its own Moderator claim too (the stronger
        // outcome — a forged root grants nobody authority, including its author).
        gcs::Membership attacker( *m_rig.session, *m_rig.store, kMemberAddress,
                                  CapturingSink( &events ), MakeSeam( kMemberAddress ) );
        EXPECT_EQ( attacker.LocalRoleFor( spaceId ), chat::ROLE_GUEST );
        EXPECT_FALSE( attacker.RemoveMember( spaceId, kTestCreatorAddress ).has_value() );
        EXPECT_FALSE(
            attacker.ChangeRole( spaceId, kTestCreatorAddress, chat::ROLE_GUEST ).has_value() );

        // Restoring the creator-signed bytes restores the verified root.
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTestCreatorAddress,
                                     saved.value() ) );
        auto restored = creator.MembersFor( spaceId );
        ASSERT_TRUE( restored.has_value() );
        const chat::MemberRecord *creatorEntry =
            FindMember( restored.value(), kTestCreatorAddress );
        ASSERT_NE( creatorEntry, nullptr );
        EXPECT_EQ( creatorEntry->role(), chat::ROLE_SUPER_ADMIN );
    }

    /**
     * @brief Truth 5/D-07: an Admin cannot tombstone or demote a peer Admin —
     *        every admin-tier destruction needs the creator.
     */
    TEST_F( GcsMembershipTest, AdminCannotTombstoneOrDemotePeerAdmin )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_ADMIN );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_ADMIN );

        const std::string targetKey = std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId
                                      + "/" + kTargetAddress;
        const auto saved = m_rig.session->Get( targetKey );
        ASSERT_TRUE( saved.has_value() );
        chat::MemberRecord savedRecord;
        ASSERT_TRUE( savedRecord.ParseFromString( saved.value() ) );

        gcs::Membership targetMembership( *m_rig.session, *m_rig.store, kTargetAddress,
                                          CapturingSink( &events ), MakeSeam( kTargetAddress ) );

        // A peer Admin's tombstone is not authority.
        chat::MemberRecord tombstone = savedRecord;
        tombstone.set_deleted( true );
        tombstone.set_prior_role( chat::ROLE_ADMIN );
        tombstone.set_approved_by( "" );
        tombstone.set_pending_action( chat::PENDING_ACTION_UNSPECIFIED );
        tombstone.set_updated_at_ms( savedRecord.updated_at_ms() + 1 );
        tombstone.set_created_by( kMemberAddress );
        tombstone.set_record_signature( TestMemberSignatureHex(
            kMemberAddress, spaceId, kTargetAddress, tombstone ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     tombstone.SerializeAsString() ) );

        auto hostileTomb = m_rig.session->Get( targetKey );
        ASSERT_TRUE( hostileTomb.has_value() );
        EXPECT_EQ( hostileTomb.value(), tombstone.SerializeAsString() );

        auto tombList = creator.MembersFor( spaceId );
        ASSERT_TRUE( tombList.has_value() );
        EXPECT_EQ( FindMember( tombList.value(), kTargetAddress ), nullptr );
        EXPECT_EQ( targetMembership.LocalRoleFor( spaceId ), chat::ROLE_GUEST );

        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     saved.value() ) );
        auto restoredRaw = m_rig.session->Get( targetKey );
        ASSERT_TRUE( restoredRaw.has_value() );
        EXPECT_EQ( restoredRaw.value(), saved.value() );
        auto restoredList = creator.MembersFor( spaceId );
        ASSERT_TRUE( restoredList.has_value() );
        const chat::MemberRecord *restored = FindMember( restoredList.value(), kTargetAddress );
        ASSERT_NE( restored, nullptr );
        EXPECT_EQ( restored->role(), chat::ROLE_ADMIN );

        // A peer Admin's applied demotion is not authority either.
        chat::MemberRecord demotion = savedRecord;
        demotion.set_role( chat::ROLE_MODERATOR );
        demotion.set_prior_role( chat::ROLE_ADMIN );
        demotion.set_approved_by( "" );
        demotion.set_pending_action( chat::PENDING_ACTION_UNSPECIFIED );
        demotion.set_pending_role( chat::ROLE_UNSPECIFIED );
        demotion.set_updated_at_ms( savedRecord.updated_at_ms() + 2 );
        demotion.set_created_by( kMemberAddress );
        demotion.set_record_signature( TestMemberSignatureHex(
            kMemberAddress, spaceId, kTargetAddress, demotion ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     demotion.SerializeAsString() ) );

        auto hostileDemotion = m_rig.session->Get( targetKey );
        ASSERT_TRUE( hostileDemotion.has_value() );
        EXPECT_EQ( hostileDemotion.value(), demotion.SerializeAsString() );

        auto demoteList = creator.MembersFor( spaceId );
        ASSERT_TRUE( demoteList.has_value() );
        EXPECT_EQ( FindMember( demoteList.value(), kTargetAddress ), nullptr );
        EXPECT_EQ( targetMembership.LocalRoleFor( spaceId ), chat::ROLE_GUEST );

        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     saved.value() ) );
        auto finalList = creator.MembersFor( spaceId );
        ASSERT_TRUE( finalList.has_value() );
        const chat::MemberRecord *finalEntry = FindMember( finalList.value(), kTargetAddress );
        ASSERT_NE( finalEntry, nullptr );
        EXPECT_EQ( finalEntry->role(), chat::ROLE_ADMIN );
    }

    /**
     * @brief Truth 5/D-03: strict outrank uses the signed predecessor, so a
     *        peer Moderator cannot demote another Moderator and a lying
     *        prior_role contradicts the node's verified state.
     */
    TEST_F( GcsMembershipTest, ModeratorCannotDemotePeerModerator )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MODERATOR );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MODERATOR );

        const std::string targetKey = std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId
                                      + "/" + kTargetAddress;
        const auto saved = m_rig.session->Get( targetKey );
        ASSERT_TRUE( saved.has_value() );
        chat::MemberRecord savedRecord;
        ASSERT_TRUE( savedRecord.ParseFromString( saved.value() ) );

        // Warm the node-local cache with the target's verified MODERATOR role.
        gcs::Membership moderator( *m_rig.session, *m_rig.store, kMemberAddress,
                                   CapturingSink( &events ), MakeSeam( kMemberAddress ) );
        auto warmed = moderator.MembersFor( spaceId );
        ASSERT_TRUE( warmed.has_value() );
        const chat::MemberRecord *warmedEntry = FindMember( warmed.value(), kTargetAddress );
        ASSERT_NE( warmedEntry, nullptr );
        EXPECT_EQ( warmedEntry->role(), chat::ROLE_MODERATOR );

        // Same-rank demotion: strict outrank rejects it.
        chat::MemberRecord demotion = savedRecord;
        demotion.set_role( chat::ROLE_MEMBER );
        demotion.set_prior_role( chat::ROLE_MODERATOR );
        demotion.set_updated_at_ms( savedRecord.updated_at_ms() + 1 );
        demotion.set_created_by( kMemberAddress );
        demotion.set_record_signature( TestMemberSignatureHex(
            kMemberAddress, spaceId, kTargetAddress, demotion ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     demotion.SerializeAsString() ) );

        auto hostile = m_rig.session->Get( targetKey );
        ASSERT_TRUE( hostile.has_value() );
        EXPECT_EQ( hostile.value(), demotion.SerializeAsString() );

        auto hostileList = moderator.MembersFor( spaceId );
        ASSERT_TRUE( hostileList.has_value() );
        EXPECT_EQ( FindMember( hostileList.value(), kTargetAddress ), nullptr );

        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     saved.value() ) );
        auto restored = moderator.MembersFor( spaceId );
        ASSERT_TRUE( restored.has_value() );
        const chat::MemberRecord *restoredEntry = FindMember( restored.value(), kTargetAddress );
        ASSERT_NE( restoredEntry, nullptr );
        EXPECT_EQ( restoredEntry->role(), chat::ROLE_MODERATOR );

        // A lying predecessor contradicts the cached MODERATOR and is rejected.
        chat::MemberRecord lie = savedRecord;
        lie.set_role( chat::ROLE_MEMBER );
        lie.set_prior_role( chat::ROLE_GUEST );
        lie.set_updated_at_ms( savedRecord.updated_at_ms() + 2 );
        lie.set_created_by( kMemberAddress );
        lie.set_record_signature( TestMemberSignatureHex(
            kMemberAddress, spaceId, kTargetAddress, lie ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     lie.SerializeAsString() ) );

        auto liedList = moderator.MembersFor( spaceId );
        ASSERT_TRUE( liedList.has_value() );
        EXPECT_EQ( FindMember( liedList.value(), kTargetAddress ), nullptr );

        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     saved.value() ) );
        auto finalList = moderator.MembersFor( spaceId );
        ASSERT_TRUE( finalList.has_value() );
        const chat::MemberRecord *finalEntry = FindMember( finalList.value(), kTargetAddress );
        ASSERT_NE( finalEntry, nullptr );
        EXPECT_EQ( finalEntry->role(), chat::ROLE_MODERATOR );
    }

    /**
     * @brief Truth 29/D-02: a replayed pre-demotion record is rejected once the
     *        node has verified the newer state. The cache is per-Membership, so
     *        one target-bound instance is constructed before any verification
     *        read and reused throughout.
     */
    TEST_F( GcsMembershipTest, ReplayedPreDemotionRecordIsRejected )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Construct BEFORE any target verification read (the cache is
        // per-instance; a fresh instance after the replay would start cold).
        gcs::Membership targetMembership( *m_rig.session, *m_rig.store, kTargetAddress,
                                          CapturingSink( &events ), MakeSeam( kTargetAddress ) );

        SeedMember( spaceId, kTargetAddress, chat::ROLE_ADMIN );
        const std::string targetKey = std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId
                                      + "/" + kTargetAddress;
        const auto saved = m_rig.session->Get( targetKey );
        ASSERT_TRUE( saved.has_value() );
        chat::MemberRecord adminRecord;
        ASSERT_TRUE( adminRecord.ParseFromString( saved.value() ) );

        auto warmed = targetMembership.MembersFor( spaceId );
        ASSERT_TRUE( warmed.has_value() );
        const chat::MemberRecord *warmedEntry = FindMember( warmed.value(), kTargetAddress );
        ASSERT_NE( warmedEntry, nullptr );
        EXPECT_EQ( warmedEntry->role(), chat::ROLE_ADMIN );

        // Publish the newer creator-signed demotion.
        chat::MemberRecord demotion = adminRecord;
        demotion.set_role( chat::ROLE_MODERATOR );
        demotion.set_prior_role( chat::ROLE_ADMIN );
        demotion.set_updated_at_ms( adminRecord.updated_at_ms() + 1 );
        demotion.set_record_signature( TestMemberSignatureHex(
            kTestCreatorAddress, spaceId, kTargetAddress, demotion ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     demotion.SerializeAsString() ) );

        auto demoted = targetMembership.MembersFor( spaceId );
        ASSERT_TRUE( demoted.has_value() );
        const chat::MemberRecord *demotedEntry = FindMember( demoted.value(), kTargetAddress );
        ASSERT_NE( demotedEntry, nullptr );
        EXPECT_EQ( demotedEntry->role(), chat::ROLE_MODERATOR );

        // Replay the OLD ADMIN bytes (same signature, older timestamp).
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     saved.value() ) );
        auto replayBytes = m_rig.session->Get( targetKey );
        ASSERT_TRUE( replayBytes.has_value() );
        EXPECT_EQ( replayBytes.value(), saved.value() );

        // Fail closed on rollback: the same instance stays at GUEST.
        EXPECT_EQ( targetMembership.LocalRoleFor( spaceId ), chat::ROLE_GUEST );
        EXPECT_FALSE(
            targetMembership.ChangeRole( spaceId, kTestCreatorAddress, chat::ROLE_GUEST )
                .has_value() );
        EXPECT_FALSE(
            targetMembership.RemoveMember( spaceId, kTestCreatorAddress ).has_value() );
    }

    /**
     * @brief Truth 29/D-02: a flipped explicit_leave without the subject's
     *        signature is excluded, in both the missing and the stale direction.
     */
    TEST_F( GcsMembershipTest, FlippedExplicitLeaveWithoutSubjectSignatureIsExcluded )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );

        const std::string targetKey = std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId
                                      + "/" + kTargetAddress;
        const auto saved = m_rig.session->Get( targetKey );
        ASSERT_TRUE( saved.has_value() );
        chat::MemberRecord savedRecord;
        ASSERT_TRUE( savedRecord.ParseFromString( saved.value() ) );

        // explicit_leave = true without a subject signature. The authority
        // signature still verifies because the canonical payload excludes the
        // subject-owned flag.
        chat::MemberRecord flipped = savedRecord;
        flipped.set_explicit_leave( true );
        flipped.clear_leave_signature();
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     flipped.SerializeAsString() ) );

        auto flippedList = creator.MembersFor( spaceId );
        ASSERT_TRUE( flippedList.has_value() );
        EXPECT_EQ( FindMember( flippedList.value(), kTargetAddress ), nullptr );

        gcs::Membership targetMembership( *m_rig.session, *m_rig.store, kTargetAddress,
                                          CapturingSink( &events ), MakeSeam( kTargetAddress ) );
        EXPECT_EQ( targetMembership.LocalRoleFor( spaceId ), chat::ROLE_GUEST );

        auto flippedBytes = m_rig.session->Get( targetKey );
        ASSERT_TRUE( flippedBytes.has_value() );
        EXPECT_EQ( flippedBytes.value(), flipped.SerializeAsString() );

        // The inverse: explicit_leave = false carrying a stale signature.
        chat::MemberRecord stale = savedRecord;
        stale.set_explicit_leave( false );
        stale.set_leave_signature(
            TestLeaveSignatureHex( kTargetAddress, spaceId, kTargetAddress ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     stale.SerializeAsString() ) );

        auto staleList = creator.MembersFor( spaceId );
        ASSERT_TRUE( staleList.has_value() );
        EXPECT_EQ( FindMember( staleList.value(), kTargetAddress ), nullptr );

        auto staleBytes = m_rig.session->Get( targetKey );
        ASSERT_TRUE( staleBytes.has_value() );
        EXPECT_EQ( staleBytes.value(), stale.SerializeAsString() );
    }

    /**
     * @brief Truth 29/D-02: Leave is an authority-preserving write — role,
     *        creator-issued signature, prior role, and signed timestamp survive.
     */
    TEST_F( GcsMembershipTest, LeavePreservesPromotedRoleAuthority )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        chat::MemberRecord before;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kMemberAddress, before ) );

        gcs::Membership memberSelf( *m_rig.session, *m_rig.store, kMemberAddress,
                                    CapturingSink( &events ), MakeSeam( kMemberAddress ) );
        ASSERT_TRUE( memberSelf.Leave( spaceId ).has_value() );

        chat::MemberRecord after;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kMemberAddress, after ) );
        EXPECT_EQ( after.role(), chat::ROLE_MEMBER );
        EXPECT_TRUE( after.explicit_leave() );
        EXPECT_FALSE( after.deleted() );
        EXPECT_EQ( after.created_by(), kTestCreatorAddress );
        EXPECT_FALSE( after.record_signature().empty() );
        EXPECT_FALSE( after.leave_signature().empty() );
        EXPECT_EQ( after.updated_at_ms(), before.updated_at_ms() );
        EXPECT_EQ( after.prior_role(), chat::ROLE_UNSPECIFIED );

        auto list = memberSelf.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        const chat::MemberRecord *member = FindMember( list.value(), kMemberAddress );
        ASSERT_NE( member, nullptr );
        EXPECT_EQ( member->role(), chat::ROLE_MEMBER );
    }

    /**
     * @brief Truth 29/D-02/D-05: redeeming an invite as an existing live member
     *        preserves the creator-issued authority and prior role verbatim.
     */
    TEST_F( GcsMembershipTest, RedeemOfExistingPromotedMemberPreservesAuthority )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        auto minted = creator.MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );

        SeedMember( spaceId, kJoinerAddress, chat::ROLE_MEMBER );
        chat::MemberRecord before;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, before ) );

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                                CapturingSink( &events ), MakeSeam( kJoinerAddress ) );
        ASSERT_TRUE( joiner.RedeemInvite( minted.value() ).has_value() );

        chat::MemberRecord after;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, after ) );
        EXPECT_EQ( after.role(), chat::ROLE_MEMBER );
        EXPECT_EQ( after.created_by(), kTestCreatorAddress );
        EXPECT_FALSE( after.record_signature().empty() );
        EXPECT_EQ( after.record_signature(), before.record_signature() );
        EXPECT_EQ( after.updated_at_ms(), before.updated_at_ms() );
        EXPECT_EQ( after.prior_role(), before.prior_role() );

        auto list = joiner.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        const chat::MemberRecord *member = FindMember( list.value(), kJoinerAddress );
        ASSERT_NE( member, nullptr );
        EXPECT_EQ( member->role(), chat::ROLE_MEMBER );
    }

    /**
     * @brief Truth 29/D-02: a cold node cannot resolve the scope, yet a leave
     *        still preserves the authority bytes verbatim; the record remains
     *        excluded from the roster because its creator chain is unresolvable.
     */
    TEST_F( GcsMembershipTest, ColdNodeLeavePreservesAuthorityWithoutLocalScope )
    {
        StartRig( kTestCreatorAddress );

        const std::string coldScopeId = "space-cold";
        SeedMember( coldScopeId, kMemberAddress, chat::ROLE_MEMBER );

        chat::MemberRecord before;
        ASSERT_TRUE(
            ReadMemberRecord( *m_rig.session, coldScopeId, kMemberAddress, before ) );

        std::vector<chat::GcsEvent> events;
        gcs::Membership memberSelf( *m_rig.session, *m_rig.store, kMemberAddress,
                                    CapturingSink( &events ), MakeSeam( kMemberAddress ) );
        ASSERT_TRUE( memberSelf.Leave( coldScopeId ).has_value() );

        chat::MemberRecord after;
        ASSERT_TRUE(
            ReadMemberRecord( *m_rig.session, coldScopeId, kMemberAddress, after ) );
        EXPECT_EQ( after.role(), chat::ROLE_MEMBER );
        EXPECT_TRUE( after.explicit_leave() );
        EXPECT_EQ( after.created_by(), kTestCreatorAddress );
        EXPECT_EQ( after.record_signature(), before.record_signature() );
        EXPECT_EQ( after.updated_at_ms(), before.updated_at_ms() );
        EXPECT_FALSE( after.leave_signature().empty() );

        // An unresolvable creator chain is not a member, but the bytes remain.
        auto list = memberSelf.MembersFor( coldScopeId );
        ASSERT_TRUE( list.has_value() );
        EXPECT_EQ( list.value().member_size(), 0 );
        const std::string memberKey = std::string( gcs::Membership::kMembersKeyPrefix )
                                      + coldScopeId + "/" + kMemberAddress;
        auto raw = m_rig.session->Get( memberKey );
        ASSERT_TRUE( raw.has_value() );
        EXPECT_EQ( raw.value(), after.SerializeAsString() );
    }

    /**
     * @brief Truth 29/D-03: a verified tombstone advances the predecessor cache
     *        before MembersFor skips it, so replaying the pre-tombstone bytes
     *        cannot readmit the member.
     */
    TEST_F( GcsMembershipTest, PreTombstoneReplayAfterTombstoneIsRejected )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Both instances exist before the first target read: the target-bound
        // cache must be warmed by the tombstone, never by the replay.
        gcs::Membership targetMembership( *m_rig.session, *m_rig.store, kTargetAddress,
                                          CapturingSink( &events ), MakeSeam( kTargetAddress ) );

        SeedMember( spaceId, kTargetAddress, chat::ROLE_MEMBER );
        const std::string targetKey = std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId
                                      + "/" + kTargetAddress;
        const auto saved = m_rig.session->Get( targetKey );
        ASSERT_TRUE( saved.has_value() );
        chat::MemberRecord liveRecord;
        ASSERT_TRUE( liveRecord.ParseFromString( saved.value() ) );

        auto liveList = creator.MembersFor( spaceId );
        ASSERT_TRUE( liveList.has_value() );
        const chat::MemberRecord *liveEntry = FindMember( liveList.value(), kTargetAddress );
        ASSERT_NE( liveEntry, nullptr );
        EXPECT_EQ( liveEntry->role(), chat::ROLE_MEMBER );

        // Publish the creator-signed tombstone.
        chat::MemberRecord tombstone = liveRecord;
        tombstone.set_deleted( true );
        tombstone.set_prior_role( chat::ROLE_MEMBER );
        tombstone.set_approved_by( "" );
        tombstone.set_pending_action( chat::PENDING_ACTION_UNSPECIFIED );
        tombstone.set_pending_role( chat::ROLE_UNSPECIFIED );
        tombstone.set_updated_at_ms( liveRecord.updated_at_ms() + 1 );
        tombstone.set_record_signature( TestMemberSignatureHex(
            kTestCreatorAddress, spaceId, kTargetAddress, tombstone ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     tombstone.SerializeAsString() ) );

        auto tombCreatorList = creator.MembersFor( spaceId );
        ASSERT_TRUE( tombCreatorList.has_value() );
        EXPECT_EQ( FindMember( tombCreatorList.value(), kTargetAddress ), nullptr );
        auto tombTargetList = targetMembership.MembersFor( spaceId );
        ASSERT_TRUE( tombTargetList.has_value() );
        EXPECT_EQ( FindMember( tombTargetList.value(), kTargetAddress ), nullptr );

        // Replay the exact pre-tombstone bytes.
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTargetAddress,
                                     saved.value() ) );
        auto replayBytes = m_rig.session->Get( targetKey );
        ASSERT_TRUE( replayBytes.has_value() );
        EXPECT_EQ( replayBytes.value(), saved.value() );

        auto replayCreatorList = creator.MembersFor( spaceId );
        ASSERT_TRUE( replayCreatorList.has_value() );
        EXPECT_EQ( FindMember( replayCreatorList.value(), kTargetAddress ), nullptr );
        auto replayTargetList = targetMembership.MembersFor( spaceId );
        ASSERT_TRUE( replayTargetList.has_value() );
        EXPECT_EQ( FindMember( replayTargetList.value(), kTargetAddress ), nullptr );
        EXPECT_EQ( targetMembership.LocalRoleFor( spaceId ), chat::ROLE_GUEST );
    }

    /**
     * @brief Truth 29/D-03: a second OnScopeCreated never rewrites a verified
     *        creator root and fails closed on an occupying unverifiable record.
     */
    TEST_F( GcsMembershipTest, SecondOnScopeCreatedDoesNotRewriteTheCreatorRoot )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        const std::string creatorKey = std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId
                                       + "/" + kTestCreatorAddress;
        const auto firstBytes = m_rig.session->Get( creatorKey );
        ASSERT_TRUE( firstBytes.has_value() );
        chat::MemberRecord firstRecord;
        ASSERT_TRUE( firstRecord.ParseFromString( firstBytes.value() ) );

        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        const auto secondBytes = m_rig.session->Get( creatorKey );
        ASSERT_TRUE( secondBytes.has_value() );
        EXPECT_EQ( secondBytes.value(), firstBytes.value() );
        chat::MemberRecord secondRecord;
        ASSERT_TRUE( secondRecord.ParseFromString( secondBytes.value() ) );
        EXPECT_EQ( secondRecord.record_signature(), firstRecord.record_signature() );
        EXPECT_EQ( secondRecord.updated_at_ms(), firstRecord.updated_at_ms() );
        EXPECT_EQ( secondRecord.created_at_ms(), firstRecord.created_at_ms() );
        EXPECT_EQ( secondRecord.prior_role(), chat::ROLE_UNSPECIFIED );

        auto list = creator.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        const chat::MemberRecord *root = FindMember( list.value(), kTestCreatorAddress );
        ASSERT_NE( root, nullptr );
        EXPECT_EQ( root->role(), chat::ROLE_SUPER_ADMIN );

        // An occupying record the node cannot verify is never overwritten.
        chat::MemberRecord hostile = firstRecord;
        hostile.set_role( chat::ROLE_GUEST );
        hostile.set_created_by( kMemberAddress );
        hostile.set_updated_at_ms( firstRecord.updated_at_ms() + 1 );
        hostile.set_record_signature( TestMemberSignatureHex(
            kMemberAddress, spaceId, kTestCreatorAddress, hostile ) );
        ASSERT_TRUE( PutMemberBytes( *m_rig.session, spaceId, kTestCreatorAddress,
                                     hostile.SerializeAsString() ) );

        const std::size_t pushBaseline = events.size();
        EXPECT_FALSE( creator.OnScopeCreated( spaceId, "", true ).has_value() );
        EXPECT_EQ( events.size(), pushBaseline );

        auto hostileBytes = m_rig.session->Get( creatorKey );
        ASSERT_TRUE( hostileBytes.has_value() );
        EXPECT_EQ( hostileBytes.value(), hostile.SerializeAsString() );
    }

    /**
     * @brief WR-05: a pending removal is terminal until Approve; a role change
     *        cannot cancel it, and the approved tombstone verifies.
     *
     * The approved tombstone must clear pending_action/pending_role (a deleted
     * record carrying a pending action fails 04-19 verification), keep the
     * signed role as its prior_role, and carry a strictly newer timestamp so
     * the predecessor gate accepts it. An OnMemberArrived roster push is the
     * proof that verification accepted the written bytes.
     */
    TEST_F( GcsMembershipTest, ChangeRoleRejectedWhileRemovalIsPending )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kTargetAddress, chat::ROLE_ADMIN );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        // Two admins (creator + target): the removal is dormant, not applied.
        ASSERT_TRUE( membership.RemoveMember( spaceId, kTargetAddress ).has_value() );

        chat::MemberRecord dormant;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, dormant ) );
        EXPECT_FALSE( dormant.deleted() );
        EXPECT_EQ( dormant.role(), chat::ROLE_ADMIN );
        EXPECT_TRUE( dormant.approved_by().empty() );
        EXPECT_EQ( dormant.pending_action(), chat::PENDING_ACTION_REMOVE );
        EXPECT_EQ( dormant.pending_role(), chat::ROLE_UNSPECIFIED );

        // A role change must not silently convert the pending removal.
        EXPECT_FALSE( membership.ChangeRole( spaceId, kTargetAddress, chat::ROLE_MODERATOR )
                          .has_value() );
        chat::MemberRecord stillPending;
        ASSERT_TRUE(
            ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, stillPending ) );
        EXPECT_EQ( stillPending.role(), chat::ROLE_ADMIN );
        EXPECT_EQ( stillPending.pending_action(), chat::PENDING_ACTION_REMOVE );
        EXPECT_EQ( stillPending.pending_role(), chat::ROLE_UNSPECIFIED );

        // Approve is the only way out: a creator-signed tombstone with no
        // pending state.
        ASSERT_TRUE( membership.Approve( spaceId, kTargetAddress ).has_value() );
        chat::MemberRecord approved;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, approved ) );
        EXPECT_TRUE( approved.deleted() );
        EXPECT_EQ( approved.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
        EXPECT_EQ( approved.pending_role(), chat::ROLE_UNSPECIFIED );
        EXPECT_EQ( approved.approved_by(), kTestCreatorAddress );
        EXPECT_EQ( approved.role(), chat::ROLE_ADMIN );
        EXPECT_FALSE( approved.record_signature().empty() );
        EXPECT_GT( approved.updated_at_ms(), dormant.updated_at_ms() );

        // Verification acceptance: OnMemberArrived returns before pushing when
        // the record does not verify (04-19), so a roster push that excludes
        // the target proves the tombstone was accepted.
        const std::size_t beforeArrival = events.size();
        membership.OnMemberArrived(
            "/crdt/k" + std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/"
                + kTargetAddress + "/v",
            approved.SerializeAsString() );
        ASSERT_GT( events.size(), beforeArrival );
        ASSERT_TRUE( events.back().has_member_list() );
        EXPECT_EQ( events.back().member_list().scope_id(), spaceId );
        EXPECT_EQ( FindMember( events.back().member_list(), kTargetAddress ), nullptr );

        auto list = membership.MembersFor( spaceId );
        ASSERT_TRUE( list.has_value() );
        EXPECT_EQ( FindMember( list.value(), kTargetAddress ), nullptr );
    }

    /**
     * @brief WR-05: a dormant REMOVE clears a stale pending_role left by a
     *        superseded dormant ROLE_CHANGE.
     */
    TEST_F( GcsMembershipTest, DormantRemovalClearsAStalePendingRole )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kTargetAddress, chat::ROLE_ADMIN );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        // Two admins: the admin demotion is dormant with a concrete role.
        ASSERT_TRUE(
            membership.ChangeRole( spaceId, kTargetAddress, chat::ROLE_MODERATOR )
                .has_value() );
        chat::MemberRecord demotion;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, demotion ) );
        ASSERT_EQ( demotion.pending_action(), chat::PENDING_ACTION_ROLE_CHANGE );
        ASSERT_EQ( demotion.pending_role(), chat::ROLE_MODERATOR );

        // The removal supersedes it and must not carry the stale role.
        ASSERT_TRUE( membership.RemoveMember( spaceId, kTargetAddress ).has_value() );
        chat::MemberRecord removal;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, removal ) );
        EXPECT_EQ( removal.pending_action(), chat::PENDING_ACTION_REMOVE );
        EXPECT_EQ( removal.pending_role(), chat::ROLE_UNSPECIFIED );
        EXPECT_TRUE( removal.approved_by().empty() );
        EXPECT_FALSE( removal.deleted() );
    }

    /**
     * @brief WR-05: an immediate removal is a coherent tombstone — no pending
     *        state and no approval attribution — and it verifies.
     */
    TEST_F( GcsMembershipTest, ImmediateRemovalClearsAllPendingFields )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Sole admin: the removal tombstones immediately.
        ASSERT_TRUE( creator.RemoveMember( spaceId, kMemberAddress ).has_value() );
        chat::MemberRecord removed;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kMemberAddress, removed ) );
        EXPECT_TRUE( removed.deleted() );
        EXPECT_EQ( removed.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
        EXPECT_EQ( removed.pending_role(), chat::ROLE_UNSPECIFIED );
        EXPECT_TRUE( removed.approved_by().empty() );

        const std::size_t beforeArrival = events.size();
        creator.OnMemberArrived(
            "/crdt/k" + std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/"
                + kMemberAddress + "/v",
            removed.SerializeAsString() );
        ASSERT_GT( events.size(), beforeArrival );
        ASSERT_TRUE( events.back().has_member_list() );
        EXPECT_EQ( events.back().member_list().scope_id(), spaceId );
        EXPECT_EQ( FindMember( events.back().member_list(), kMemberAddress ), nullptr );
    }

    /**
     * @brief WR-02: creator authority is catalog identity, never a claimed
     *        SUPER_ADMIN role.
     *
     * No verified non-creator Super Admin is constructible by design, so this
     * is also a defense against a future verification regression: even with a
     * self-signed SUPER_ADMIN record occupying the wallet, every command
     * fails, while the real creator succeeds on the same operations.
     */
    TEST_F( GcsMembershipTest, CreatorIdentityIsTheOnlyCreatorException )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );
        SeedMember( spaceId, kTargetAddress, chat::ROLE_SUPER_ADMIN, kSeedCreatedAtMs,
                    false, kTargetAddress );

        gcs::Membership attacker( *m_rig.session, *m_rig.store, kTargetAddress,
                                  CapturingSink( &events ), MakeSeam( kTargetAddress ) );
        EXPECT_EQ( attacker.LocalRoleFor( spaceId ), chat::ROLE_GUEST );
        EXPECT_FALSE( attacker.DeleteScope( spaceId ).has_value() );
        EXPECT_FALSE( attacker.RemoveMember( spaceId, kMemberAddress ).has_value() );
        EXPECT_FALSE( attacker.ChangeRole( spaceId, kMemberAddress, chat::ROLE_GUEST )
                          .has_value() );

        // The forged record stays: rejection is not rollback.
        chat::MemberRecord forged;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, forged ) );
        EXPECT_EQ( forged.role(), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( forged.created_by(), kTargetAddress );

        // The real creator succeeds on the same operations.
        ASSERT_TRUE(
            creator.ChangeRole( spaceId, kMemberAddress, chat::ROLE_MODERATOR )
                .has_value() );
        ASSERT_TRUE( creator.RemoveMember( spaceId, kMemberAddress ).has_value() );
        ASSERT_TRUE( creator.DeleteScope( spaceId ).has_value() );
    }

    /**
     * @brief T-04-21-07: the dormancy predicate matches verification, so a
     *        Moderator's Member removal is immediate and verifiable.
     */
    TEST_F( GcsMembershipTest, ModeratorMemberRemovalIsImmediateAfterTheTierAlignment )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kAdminAddress, chat::ROLE_ADMIN );
        SeedMember( spaceId, kModeratorAddress, chat::ROLE_MODERATOR );
        SeedMember( spaceId, kMemberAddress, chat::ROLE_MEMBER );

        std::vector<chat::GcsEvent> events;
        gcs::Membership creator( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                 CapturingSink( &events ), MakeSeam( kTestCreatorAddress ) );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Two admins, but neither the target nor the author is admin-tier: the
        // verifier rejects a below-admin dormant Member state, so the command
        // takes the immediate branch instead.
        gcs::Membership moderator( *m_rig.session, *m_rig.store, kModeratorAddress,
                                   CapturingSink( &events ), MakeSeam( kModeratorAddress ) );
        ASSERT_TRUE( moderator.RemoveMember( spaceId, kMemberAddress ).has_value() );

        chat::MemberRecord removed;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kMemberAddress, removed ) );
        EXPECT_TRUE( removed.deleted() );
        EXPECT_EQ( removed.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
        EXPECT_EQ( removed.pending_role(), chat::ROLE_UNSPECIFIED );
        EXPECT_TRUE( removed.approved_by().empty() );

        // The immediate tombstone verifies through the delivery path.
        const std::size_t beforeArrival = events.size();
        creator.OnMemberArrived(
            "/crdt/k" + std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/"
                + kMemberAddress + "/v",
            removed.SerializeAsString() );
        ASSERT_GT( events.size(), beforeArrival );
        ASSERT_TRUE( events.back().has_member_list() );
        EXPECT_EQ( events.back().member_list().scope_id(), spaceId );
        EXPECT_EQ( FindMember( events.back().member_list(), kMemberAddress ), nullptr );
    }

} // namespace gcs::test
