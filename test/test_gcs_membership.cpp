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
#include "test_graphsync_network.hpp"
#include "test_wait_condition.hpp"

#include <chrono>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <mutex>
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

        /// @brief The production D-06 seam, enabled (built by member assignment).
        static gcs::Membership::KeyWrapSeam MakeSeam()
        {
            gcs::Membership::KeyWrapSeam seam;
            seam.wrap    = &gcs::crypto::WrapRoomKey;
            seam.unwrap  = &gcs::crypto::UnwrapRoomKey;
            seam.enabled = true;
            return seam;
        }

        /// @brief A sink that captures every pushed event (test-thread only).
        gcs::Membership::EventSink CapturingSink( std::vector<chat::GcsEvent> *out )
        {
            return [ out ]( const chat::GcsEvent &event ) { out->push_back( event ); };
        }

        /// @brief Persist a member record directly (the hostile/forged-write path).
        void SeedMember( const std::string &scopeId, const std::string &address,
                         chat::Role role, int64_t createdAtMs = kSeedCreatedAtMs,
                         bool deleted = false )
        {
            chat::MemberRecord record;
            record.set_scope_id( scopeId );
            record.set_address( address );
            record.set_role( role );
            record.set_deleted( deleted );
            record.set_created_at_ms( createdAtMs );
            record.set_updated_at_ms( createdAtMs );
            ASSERT_TRUE( m_rig.session
                             ->Put( std::string( gcs::Membership::kMembersKeyPrefix ) + scopeId
                                        + "/" + address,
                                    record.SerializeAsString() )
                             .has_value() );
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
                                    CapturingSink( &events ), MakeSeam() );

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

        EXPECT_EQ( membership.LocalRoleFor( spaceId ), chat::ROLE_SUPER_ADMIN );
        // Unknown scope: no local record -> safest default (D-08 read-only).
        EXPECT_EQ( membership.LocalRoleFor( "space-unknown" ), chat::ROLE_GUEST );
        EXPECT_FALSE( membership.HasExplicitLeave( spaceId ) );

        // The roster push landed on the sink.
        ASSERT_FALSE( events.empty() );
        EXPECT_TRUE( events.back().has_member_list() );
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
                                    CapturingSink( &events ), MakeSeam() );
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
                                 CapturingSink( &creatorEvents ), MakeSeam() );

        std::vector<chat::GcsEvent> memberEvents;
        gcs::Membership             member( *m_rig.session, *m_rig.store, kMemberAddress,
                                CapturingSink( &memberEvents ), MakeSeam() );

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
                                    CapturingSink( &events ), MakeSeam() );
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
                                 CapturingSink( &creatorEvents ), MakeSeam() );
        ASSERT_TRUE( creator.OnScopeCreated( roomId, roomTopic, false ).has_value() );

        auto minted = creator.MintInvite( roomId );
        ASSERT_TRUE( minted.has_value() );

        // Prove the redeem itself stores the unwrapped key: clear the local
        // per-room key store first, then redeem and assert it is repopulated.
        gcs::crypto::ClearRoomKey( roomTopic );
        EXPECT_FALSE( gcs::crypto::RoomKeyFor( roomTopic ).has_value() );

        std::vector<chat::GcsEvent> joinerEvents;
        gcs::Membership             joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                                CapturingSink( &joinerEvents ), MakeSeam() );
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
                                 CapturingSink( &joiner2Events ), MakeSeam() );
        ASSERT_TRUE( joiner2.RedeemInvite( minted.value() ).has_value() );

        auto list = creator.MembersFor( roomId );
        ASSERT_TRUE( list.has_value() );
        ASSERT_EQ( list.value().member_size(), 3 );
        EXPECT_NE( FindMember( list.value(), kTestCreatorAddress ), nullptr );
        EXPECT_NE( FindMember( list.value(), kJoinerAddress ), nullptr );
        EXPECT_NE( FindMember( list.value(), kJoiner2Address ), nullptr );
    }

    /**
     * @brief Test 6 (MEMB-04): a malformed invite URL is rejected and a forged
     *        invite record with an unwrap-as-wrong-key envelope is skip-and-logged
     *        without aborting the redeem.
     */
    TEST_F( GcsMembershipTest, RedeemRejectsMalformedUrlAndSkipsBadEnvelope )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam() );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        gcs::Membership joiner( *m_rig.session, *m_rig.store, kJoinerAddress,
                               CapturingSink( &events ), MakeSeam() );
        EXPECT_FALSE( joiner.RedeemInvite( "https://example.com/not-an-invite" ).has_value() );
        EXPECT_FALSE( joiner.RedeemInvite( "gcs://invite/space/" ).has_value() );
        EXPECT_FALSE( joiner.RedeemInvite( "gcs://invite/space/abc" ).has_value() );
        EXPECT_FALSE( joiner.RedeemInvite( "gcs://invite/other/abc?key=AAAA" ).has_value() );
        EXPECT_FALSE( joiner.RedeemInvite( "gcs://invite/space/nope?key=AAAA" ).has_value() );

        // A forged invite record whose bytes are not wrap output: redeem still
        // writes the member record (skip-and-log on the undecryptable key).
        chat::InviteRecord forged;
        forged.set_scope_id( spaceId );
        forged.set_role( chat::ROLE_GUEST );
        chat::WrappedRoomKey *bad = forged.add_wrapped_room_key();
        bad->set_room_id( "room-forged" );
        bad->set_envelope( "not-a-real-envelope" );
        ASSERT_TRUE( m_rig.session
                         ->Put( std::string( gcs::Membership::kInvitesKeyPrefix ) + spaceId
                                    + "/invite-forged",
                                forged.SerializeAsString() )
                         .has_value() );

        ASSERT_TRUE( joiner.RedeemInvite( "gcs://invite/space/invite-forged?key=AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA" )
                         .has_value() );
        chat::MemberRecord joinerRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kJoinerAddress, joinerRecord ) );
        EXPECT_EQ( joinerRecord.role(), chat::ROLE_GUEST );
        EXPECT_FALSE( joinerRecord.deleted() );
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
                                    CapturingSink( &events ), MakeSeam() );
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
                               CapturingSink( &events ), MakeSeam() );
        EXPECT_FALSE( guest.Approve( spaceId, kAdminAddress ).has_value() );
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
                                 CapturingSink( &events ), MakeSeam() );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Sole admin (the creator): the removal applies at once.
        ASSERT_TRUE( creator.RemoveMember( spaceId, kTargetAddress ).has_value() );
        chat::MemberRecord removed;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, removed ) );
        EXPECT_TRUE( removed.deleted() );
        EXPECT_EQ( removed.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );

        // Guest: read-only across every mutator (D-08).
        gcs::Membership guest( *m_rig.session, *m_rig.store, kGuestAddress,
                               CapturingSink( &events ), MakeSeam() );
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
                                    CapturingSink( &events ), MakeSeam() );
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
                                    CapturingSink( &events ), MakeSeam() );
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
                                    CapturingSink( &events ), MakeSeam() );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        // Heal delivery: the callback derives the scope and pushes the roster.
        chat::MemberRecord arrived;
        arrived.set_scope_id( spaceId );
        arrived.set_address( kMemberAddress );
        arrived.set_role( chat::ROLE_MEMBER );
        const std::size_t beforeCount = events.size();
        membership.OnMemberArrived(
            "/crdt/k" + std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/"
                + kMemberAddress + "/v",
            arrived.SerializeAsString() );
        ASSERT_GT( events.size(), beforeCount );
        EXPECT_TRUE( events.back().has_member_list() );
        EXPECT_EQ( events.back().member_list().scope_id(), spaceId );

        // A tombstoned heal delivery must NOT push.
        chat::MemberRecord deadArrived = arrived;
        deadArrived.set_deleted( true );
        const std::size_t afterLive = events.size();
        membership.OnMemberArrived(
            std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/" + kDeadAddress,
            deadArrived.SerializeAsString() );
        EXPECT_EQ( events.size(), afterLive );

        // Sole admin: DeleteScope tombstones the entity record immediately. The
        // tombstone is proven through a fresh catalog reload (the delete is a
        // direct store write, not an EntityStore mutator).
        ASSERT_TRUE( membership.DeleteScope( spaceId ).has_value() );
        gcs::EntityStore reloaded( *m_rig.session, kTestCreatorAddress );
        ASSERT_TRUE( reloaded.LoadFromStore().has_value() );
        EXPECT_TRUE( reloaded.Spaces().empty() );
        const auto tombBytes = m_rig.session->Get( "gcs/entities/spaces/" + spaceId );
        ASSERT_TRUE( tombBytes.has_value() );
        chat::SpaceRecord tomb;
        ASSERT_TRUE( tomb.ParseFromString( tombBytes.value() ) );
        EXPECT_TRUE( tomb.deleted() );
        EXPECT_GT( tomb.deleted_at_ms(), 0 );
    }

    /**
     * @brief Test (WR-02/D-08): a role governs which members its holder may act
     *        on — a Moderator cannot remove an Admin and an Admin cannot remove
     *        a non-creator Super Admin, while a Moderator may remove a Member.
     */
    TEST_F( GcsMembershipTest, ModeratorCannotRemoveAdminAndAdminCannotRemoveSuperAdmin )
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
                                 CapturingSink( &events ), MakeSeam() );
        ASSERT_TRUE( creator.OnScopeCreated( spaceId, "", true ).has_value() );

        // Rank 3 > 2: a Moderator may remove a plain Member.
        gcs::Membership moderator( *m_rig.session, *m_rig.store, kModeratorAddress,
                                   CapturingSink( &events ), MakeSeam() );
        ASSERT_TRUE( moderator.RemoveMember( spaceId, kMemberAddress ).has_value() );

        // Rank 3 is not > 4: a Moderator may NOT remove an Admin.
        EXPECT_FALSE( moderator.RemoveMember( spaceId, kAdminAddress ).has_value() );
        chat::MemberRecord adminRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kAdminAddress, adminRecord ) );
        EXPECT_EQ( adminRecord.role(), chat::ROLE_ADMIN );
        EXPECT_FALSE( adminRecord.deleted() );
        EXPECT_EQ( adminRecord.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );

        // A second, non-creator Super Admin target.
        SeedMember( spaceId, kTargetAddress, chat::ROLE_SUPER_ADMIN );
        gcs::Membership admin( *m_rig.session, *m_rig.store, kAdminAddress,
                               CapturingSink( &events ), MakeSeam() );

        // Rank 4 is not > 5: an Admin may NOT remove the Super Admin directly.
        EXPECT_FALSE( admin.RemoveMember( spaceId, kTargetAddress ).has_value() );
        chat::MemberRecord superAdminRecord;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress,
                                       superAdminRecord ) );
        EXPECT_FALSE( superAdminRecord.deleted() );
        EXPECT_EQ( superAdminRecord.role(), chat::ROLE_SUPER_ADMIN );

        // CanChangeRole is not a removal guard: the demotion is accepted, but
        // WR-03 dormancy keeps the Super Admin's role until approval.
        ASSERT_TRUE( admin.ChangeRole( spaceId, kTargetAddress, chat::ROLE_ADMIN ).has_value() );
        chat::MemberRecord dormant;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, dormant ) );
        EXPECT_EQ( dormant.role(), chat::ROLE_SUPER_ADMIN );
        EXPECT_TRUE( dormant.approved_by().empty() );
        EXPECT_EQ( dormant.pending_action(), chat::PENDING_ACTION_ROLE_CHANGE );
        EXPECT_EQ( dormant.pending_role(), chat::ROLE_ADMIN );
    }

    /**
     * @brief Test (WR-03): demoting a non-creator Super Admin with 2+ admins
     *        writes a dormant ROLE_CHANGE record instead of applying it.
     */
    TEST_F( GcsMembershipTest, SuperAdminDemotionIsDormantWhenTwoAdmins )
    {
        StartRig( kTestCreatorAddress );

        auto space = m_rig.store->CreateSpace( "ops", true, true, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();
        SeedMember( spaceId, kTargetAddress, chat::ROLE_SUPER_ADMIN );

        std::vector<chat::GcsEvent> events;
        gcs::Membership membership( *m_rig.session, *m_rig.store, kTestCreatorAddress,
                                    CapturingSink( &events ), MakeSeam() );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        // Two admins (creator + target) -> the demotion is dormant, not applied.
        ASSERT_TRUE( membership.ChangeRole( spaceId, kTargetAddress, chat::ROLE_ADMIN ).has_value() );

        chat::MemberRecord record;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, record ) );
        EXPECT_EQ( record.role(), chat::ROLE_SUPER_ADMIN );
        EXPECT_TRUE( record.approved_by().empty() );
        EXPECT_EQ( record.pending_action(), chat::PENDING_ACTION_ROLE_CHANGE );
        EXPECT_EQ( record.pending_role(), chat::ROLE_ADMIN );
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
                                    CapturingSink( &events ), MakeSeam() );
        ASSERT_TRUE( membership.OnScopeCreated( spaceId, "", true ).has_value() );

        ASSERT_TRUE( membership.ChangeRole( spaceId, kTargetAddress, chat::ROLE_GUEST ).has_value() );

        chat::MemberRecord record;
        ASSERT_TRUE( ReadMemberRecord( *m_rig.session, spaceId, kTargetAddress, record ) );
        EXPECT_EQ( record.role(), chat::ROLE_GUEST );
        EXPECT_EQ( record.pending_action(), chat::PENDING_ACTION_UNSPECIFIED );
        EXPECT_EQ( record.pending_role(), chat::ROLE_UNSPECIFIED );
    }

} // namespace gcs::test
