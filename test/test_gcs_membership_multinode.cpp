/**
 * @file       test_gcs_membership_multinode.cpp
 * @brief      Two-node in-process integration tests for remote membership
 *             convergence (P6) over the REAL GossipSub transport: node A creates
 *             a standalone room and mints a capability-token invite, node B
 *             redeems it, and BOTH nodes' rosters converge on the joined member
 *             — then a Super Admin role change on A converges on B live.
 * @details    Mirrors test_gcs_messaging_multinode.cpp's MultinodeNode /
 *             StartTransport / StartSession / ShutdownNode pattern (default
 *             gossip config so delivery is genuinely cross-node, graphsync
 *             scheduler sharing the pubsub host's running asio context,
 *             AddPeers connect). Each node registers the CRDT heal callback on
 *             gcs::Membership::kMembersKeyCallbackPattern and wires it to
 *             OnMemberArrived — the receive bridge is registered on CoreSession
 *             directly, NOT the FFI. The D-06 key-wrap seam is the real
 *             gcs::crypto adapter, enabled. All waits use the GCS
 *             wait-condition template — never a raw thread sleep.
 * @date       2026-10-02
 */

#include "account/GeniusSigner.hpp"
#include "lib/gcs_crypto.hpp"
#include "lib/gcs_entity_store.hpp"
#include "lib/gcs_membership.hpp"
#include "test_gcs_invite_helpers.hpp"
#include "test_graphsync_network.hpp"
#include "test_wait_condition.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <optional>
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
    /// GossipPubSub bind address used by every node.
    constexpr const char *kListenIp = "0.0.0.0";

    /// Room chat topic prefix — the gcs/chat/<id> convention (D-01).
    constexpr const char *kRoomTopicPrefix = "gcs/chat/";

    /// Syntactically valid base64url invite key (32 decoded bytes) used by the
    /// negative redeem cases — never the minted bearer key.
    constexpr const char *kWellFormedInviteKey = "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA";

    /// Minimal soralog YAML — console sink only, error level (mirrors
    /// test_gcs_messaging_multinode.cpp).
    constexpr const char *kLoggingYaml = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_membership_multinode_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";
} // namespace

namespace gcs::test
{
    /**
     * @brief One live node in the two-node mesh.
     *
     * Member destruction order is reverse declaration order; the fixture's
     * ShutdownNode() performs the explicit quiesce-before-release sequence
     * (session shutdown -> membership reset -> store reset -> session reset ->
     * pubsub stop) so no CRDT callback can touch a half-destroyed component.
     */
    struct MultinodeNode
    {
        std::string basePath; ///< Node tmp dir (key store + db live here)
        std::string dbPath;   ///< RocksDB path for the CRDT store
        std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> pubsub;
        TestGraphsyncContext                             graphsync;
        std::unique_ptr<gcs::CoreSession>                session;
        std::unique_ptr<gcs::EntityStore>                store;
        std::unique_ptr<gcs::Membership>                 membership;
        std::mutex                                       sinkMutex;
        std::vector<chat::GcsEvent>                      events;
    };

    /**
     * @brief Stand up the transport half of a node (key store + port-0
     *        GossipPubSub + graphsync bundle sharing the host's asio context).
     *
     * @param[in,out] node     The node being built (must outlive its callbacks).
     * @param[in]     basePath Directory for the key store.
     */
    void StartTransport( MultinodeNode &node, const std::string &basePath )
    {
        node.basePath = basePath;
        std::filesystem::create_directories( basePath );

        sgns::crdt::KeyPairFileStorage keyStore( basePath + "/key" );
        auto                           keyPairResult = keyStore.GetKeyPair();
        ASSERT_FALSE( keyPairResult.has_error() );

        node.pubsub = std::make_shared<sgns::ipfs_pubsub::GossipPubSub>( keyPairResult.value() );
        auto startFuture = node.pubsub->Start( 0, {}, kListenIp, {} ); // port 0 = ephemeral
        auto startError  = startFuture.get();
        ASSERT_FALSE( startError ) << "Could not start GossipPubSub: " << startError.message();

        node.graphsync.io = node.pubsub->GetAsioContext();
        node.graphsync.scheduler = std::make_shared<libp2p::basic::SchedulerImpl>(
            std::make_shared<libp2p::basic::AsioSchedulerBackend>( node.graphsync.io ),
            libp2p::basic::Scheduler::Config{ std::chrono::milliseconds{ kGraphsyncSchedulerTickMs } } );
        node.graphsync.network = std::make_shared<sgns::ipfs_lite::ipfs::graphsync::Network>(
            node.pubsub->GetHost(), node.graphsync.scheduler );
    }

    /**
     * @brief Construct + initialize the CoreSession over the injected pubsub +
     *        graphsync seam (no GeniusSDK).
     *
     * @param[in,out] node   The node whose session is being started.
     * @param[in]     dbPath RocksDB path for the CRDT store.
     */
    void StartSession( MultinodeNode &node, const std::string &dbPath )
    {
        node.dbPath = dbPath;

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = dbPath;
        node.session = std::make_unique<gcs::CoreSession>( cfg );
        ASSERT_TRUE( node.session->Initialize( node.pubsub, node.graphsync.network ).has_value() );
    }

    /**
     * @brief Wire the CRDT heal bridge + the creator-stamped EntityStore and the
     *        Membership component (real D-06 crypto seam, enabled).
     *
     * The receive-bridge lambda captures the node by pointer and fires on the
     * CRDT DagWorker thread, so it only reads the node's membership pointer
     * (never reassigned while the transport is live) — no data race.
     *
     * @param[in,out] node   The node being wired.
     * @param[in]     sender The local wallet address (D-03 authority stamp).
     * @param[in]     signer The fixture-owned real signer for @p sender; it
     *                       must outlive the membership (the issuer seam binds
     *                       it by reference).
     */
    void ArmMembership( MultinodeNode &node, const std::string &sender,
                        sgns::GeniusSigner &signer )
    {
        // CRDT heal path (D-01/D-04): member-record callback -> Membership.
        ASSERT_TRUE( node.session->RegisterNewElementCallback(
                          gcs::Membership::kMembersKeyCallbackPattern,
                          [ &node ]( const std::string &key, const std::string &value ) {
                              if ( node.membership )
                              {
                                  node.membership->OnMemberArrived( key, value );
                              }
                          } )
                          .has_value() );

        // WR-06: member and invite records replicate on the ONE well-known
        // membership topic — never a per-scope room topic, which a recipient
        // has by definition not joined. Listen first, then broadcast (D-07).
        ASSERT_TRUE( node.session
                         ->AddListenTopic( gcs::Membership::kMembershipTopic )
                         .has_value() );
        ASSERT_TRUE( node.session
                         ->AddBroadcastTopic( gcs::Membership::kMembershipTopic )
                         .has_value() );

        node.store = std::make_unique<gcs::EntityStore>( *node.session, sender );
        ASSERT_TRUE( node.store->LoadFromStore().has_value() );

        // Production D-06 posture: real vendored-OpenSSL seam, enabled. Built by
        // member assignment (KeyWrapSeam has a user-provided default ctor). The
        // issuer pair is the only difference from the pre-04-15 fixture: it
        // binds the REAL secp256k1 signer, so the seam's argument order and
        // binary handling are proven against the production implementation.
        gcs::Membership::KeyWrapSeam seam;
        seam.wrap       = &gcs::crypto::WrapRoomKey;
        seam.unwrap     = &gcs::crypto::UnwrapRoomKey;
        seam.mintAuth   = &gcs::crypto::MintInviteAuthenticator;
        seam.verifyAuth = &gcs::crypto::VerifyInviteAuthenticator;
        seam.enabled    = true;

        // The signer is a fixture member that outlives the membership
        // (ShutdownNode evicts it long before fixture destruction), so the
        // reference capture cannot dangle.
        seam.sign = [ &signer ]( const std::string &payload,
                                 const std::string &issuer )
            -> outcome::result<std::string>
        {
            if ( issuer != signer.GetAddress() )
            {
                return outcome::failure( sgns::gcs::Error::GcsDbError );
            }
            const std::vector<uint8_t> data( payload.begin(), payload.end() );
            const std::vector<uint8_t> raw = signer.Sign( data );
            if ( raw.empty() )
            {
                return outcome::failure( sgns::gcs::Error::GcsDbError );
            }
            // Binary-safe: never a C-string operation on raw signature bytes.
            return outcome::success( std::string( raw.begin(), raw.end() ) );
        };

        // Seam order is (issuer, payload, signature); GeniusSigner takes
        // (address, signature, data). The payload must never occupy the
        // signature slot, and the signer needs BARE hex (base::unhex).
        seam.verifyIssuer = []( const std::string &issuer,
                                const std::string &payload,
                                const std::string &signature )
            -> outcome::result<bool>
        {
            std::string bareAddress = issuer;
            if ( bareAddress.rfind( "0x", 0 ) == 0 )
            {
                bareAddress.erase( 0, 2 );
            }
            if ( signature.size() != gcs::Membership::kSignatureBytes )
            {
                return outcome::success( false );
            }
            const std::vector<uint8_t> signatureBytes( signature.begin(),
                                                       signature.end() );
            const std::vector<uint8_t> payloadBytes( payload.begin(),
                                                     payload.end() );
            return outcome::success( sgns::GeniusSigner::VerifySignature(
                bareAddress, signatureBytes, payloadBytes ) );
        };

        node.membership = std::make_unique<gcs::Membership>(
            *node.session, *node.store, sender,
            [ &node ]( const chat::GcsEvent &event ) {
                std::lock_guard<std::mutex> lock( node.sinkMutex );
                node.events.push_back( event );
            },
            seam );
    }

    /**
     * @brief Quiesce then release a node (session -> membership -> store ->
     *        session -> transport), so no callback touches a dying component.
     *
     * @param[in,out] node The node to tear down.
     */
    void ShutdownNode( MultinodeNode &node )
    {
        if ( node.session )
        {
            node.session->Shutdown();
        }
        if ( node.membership )
        {
            node.membership.reset();
        }
        if ( node.store )
        {
            node.store.reset();
        }
        if ( node.session )
        {
            node.session.reset();
        }
        if ( node.pubsub )
        {
            node.pubsub->Stop();
            node.pubsub.reset();
        }
    }

    /**
     * @brief The role of @p address in @p list (ROLE_UNSPECIFIED when absent).
     *
     * @param[in] list    The roster.
     * @param[in] address The wallet address to look up.
     * @return The member's role, or ROLE_UNSPECIFIED when not present.
     */
    chat::Role RoleOf( const chat::MemberList &list, const std::string &address )
    {
        for ( int i = 0; i < list.member_size(); ++i )
        {
            if ( list.member( i ).address() == address )
            {
                return list.member( i ).role();
            }
        }
        return chat::ROLE_UNSPECIFIED;
    }

    /**
     * @brief Snapshot the newest pushed MemberList for @p scopeId on @p node.
     *
     * Copies under the node's sink mutex and releases it before returning —
     * never holds the mutex across a wait.
     *
     * @param[in] node    The node whose captured events are inspected.
     * @param[in] scopeId The scope whose newest pushed roster is wanted.
     * @return The newest pushed roster for the scope, or std::nullopt.
     */
    std::optional<chat::MemberList> LatestMemberListFor( MultinodeNode &node,
                                                         const std::string &scopeId )
    {
        std::lock_guard<std::mutex> lock( node.sinkMutex );
        for ( auto it = node.events.rbegin(); it != node.events.rend(); ++it )
        {
            if ( it->has_member_list() && it->member_list().scope_id() == scopeId )
            {
                return it->member_list();
            }
        }
        return std::nullopt;
    }

    /**
     * @brief Two-node mesh fixture: two fully wired nodes (A + B),
     *        cross-connected via AddPeers and confirmed ready before each test.
     */
    class MembershipMultinodeTest : public ::testing::Test
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
                           / ( std::string{ "gcs_membership_multinode_" } + info->name() + "_" +
                               std::to_string( uniqueSalt ) ) )
                             .string();
            std::filesystem::create_directories( m_tempPath );

            StartTransport( m_nodeA, m_tempPath + "/a" );
            StartTransport( m_nodeB, m_tempPath + "/b" );
            StartSession( m_nodeA, m_nodeA.basePath + "/db" );
            StartSession( m_nodeB, m_nodeB.basePath + "/db" );
            // Real secp256k1 identities (D-03): Generate() returns by value and
            // the signer need not be copyable, so the fixture owns it.
            m_signerA = std::make_unique<sgns::GeniusSigner>(
                sgns::GeniusSigner::Generate() );
            m_signerB = std::make_unique<sgns::GeniusSigner>(
                sgns::GeniusSigner::Generate() );
            ArmMembership( m_nodeA, SenderA(), *m_signerA );
            ArmMembership( m_nodeB, SenderB(), *m_signerB );

            ASSERT_NE( m_nodeA.membership, nullptr );
            ASSERT_NE( m_nodeB.membership, nullptr );

            ConnectNodes();
        }

        void TearDown() override
        {
            ShutdownNode( m_nodeA );
            ShutdownNode( m_nodeB );
            std::error_code ec;
            std::filesystem::remove_all( m_tempPath, ec );
        }

        /**
         * @brief Explicitly cross-connect the two nodes and wait for the mesh to
         *        be ready (both connection managers see the peer).
         */
        void ConnectNodes()
        {
            m_nodeA.pubsub->AddPeers( { m_nodeB.pubsub->GetInterfaceAddress() } );

            constexpr size_t kExpectedPeerCount = 1;
            EXPECT_TRUE( WaitForCondition(
                [ & ]() {
                    const size_t connA = m_nodeA.pubsub->GetHost()
                                             ->getNetwork()
                                             .getConnectionManager()
                                             .getConnections()
                                             .size();
                    const size_t connB = m_nodeB.pubsub->GetHost()
                                             ->getNetwork()
                                             .getConnectionManager()
                                             .getConnections()
                                             .size();
                    return connA >= kExpectedPeerCount && connB >= kExpectedPeerCount;
                },
                kWaitTimeout ) );
        }

        /**
         * @brief Node A creates a standalone room, mints an invite, node B
         *        redeems it, and both rosters converge on two members.
         *
         * @param[out] roomIdOut The created room id.
         */
        void CreateRoomAndJoin( std::string &roomIdOut )
        {
            auto room = m_nodeA.store->CreateRoom( "lounge", "", true ); // standalone room
            ASSERT_TRUE( room.has_value() );
            roomIdOut = room.value().id();
            const std::string topic = std::string( kRoomTopicPrefix ) + roomIdOut;

            // WR-06: no per-room topic registration here — ArmMembership already
            // registered the well-known membership replication topic on both
            // nodes, exactly as production init wiring does (04-11). The room
            // topic is still passed to OnScopeCreated for room-key storage.
            ASSERT_TRUE( m_nodeA.membership->OnScopeCreated( roomIdOut, topic, false ).has_value() );
            auto minted = m_nodeA.membership->MintInvite( roomIdOut );
            ASSERT_TRUE( minted.has_value() );

            // The creator's member record + the invite record must replicate to
            // B before B can redeem (the CRDT heal path carries both).
            ASSERT_TRUE( WaitForCondition(
                [ & ]() {
                    auto invites = m_nodeB.session->QueryKeyValues( gcs::Membership::kInvitesKeyPrefix );
                    auto members = m_nodeB.session->QueryKeyValues(
                        std::string( gcs::Membership::kMembersKeyPrefix ) + roomIdOut + "/" );
                    return invites.has_value() && invites.value().size() == 1 &&
                           members.has_value() && members.value().size() == 1;
                },
                kWaitTimeout ) );

            ASSERT_TRUE( m_nodeB.membership->RedeemInvite( minted.value() ).has_value() );

            // Both rosters converge on B's freshly written member record.
            ASSERT_TRUE( WaitForCondition(
                [ & ]() {
                    auto listA = m_nodeA.membership->MembersFor( roomIdOut );
                    auto listB = m_nodeB.membership->MembersFor( roomIdOut );
                    return listA.has_value() && listA.value().member_size() == 2 &&
                           listB.has_value() && listB.value().member_size() == 2;
                },
                kWaitTimeout ) );
        }

        /**
         * @brief Node A's bare 128-hex wallet address (the Membership sender).
         *
         * @return The address derived from the fixture's real signer A.
         */
        std::string SenderA() const
        {
            return m_signerA->GetAddress();
        }

        /**
         * @brief Node B's bare 128-hex wallet address (the Membership sender).
         *
         * @return The address derived from the fixture's real signer B.
         */
        std::string SenderB() const
        {
            return m_signerB->GetAddress();
        }

        std::string   m_tempPath; ///< Per-test temp root (removed in TearDown)
        /// Real secp256k1 identities. Declared BEFORE the nodes so they outlive
        /// every membership that binds them by reference.
        std::unique_ptr<sgns::GeniusSigner> m_signerA;
        std::unique_ptr<sgns::GeniusSigner> m_signerB;
        MultinodeNode m_nodeA;    ///< Mesh node A (Super Admin creator)
        MultinodeNode m_nodeB;    ///< Mesh node B (invite redeemer)
    };

    /**
     * @brief Test 1 (P6/MEMB-01): an invite minted on A and redeemed on B
     *        converges into BOTH nodes' rosters — the joined member plus the
     *        creator are visible from either side.
     */
    TEST_F( MembershipMultinodeTest, RemoteMemberListConvergesAfterRedeem )
    {
        std::string roomId;
        CreateRoomAndJoin( roomId );

        auto listA = m_nodeA.membership->MembersFor( roomId );
        auto listB = m_nodeB.membership->MembersFor( roomId );
        ASSERT_TRUE( listA.has_value() );
        ASSERT_TRUE( listB.has_value() );
        ASSERT_EQ( listA.value().member_size(), 2 );
        ASSERT_EQ( listB.value().member_size(), 2 );

        // The creator is a Super Admin on both sides; B's grant is the Guest
        // default stamped by the invite record (D-05).
        EXPECT_EQ( RoleOf( listA.value(), SenderA() ), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( listB.value(), SenderA() ), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( listA.value(), SenderB() ), chat::ROLE_GUEST );
        EXPECT_EQ( RoleOf( listB.value(), SenderB() ), chat::ROLE_GUEST );

        // self_address is the pushed local wallet on each side (D-04).
        EXPECT_EQ( listA.value().self_address(), SenderA() );
        EXPECT_EQ( listB.value().self_address(), SenderB() );
    }

    /**
     * @brief Test 2 (P6/MEMB-02): a Super Admin role change applied on A
     *        converges on B live (no restart) through the CRDT heal callback.
     */
    TEST_F( MembershipMultinodeTest, RemoteRoleChangeConverges )
    {
        std::string roomId;
        CreateRoomAndJoin( roomId );

        // A (the room's Super Admin) promotes B to Moderator.
        ASSERT_TRUE( m_nodeA.membership->ChangeRole( roomId, SenderB(), chat::ROLE_MODERATOR ).has_value() );

        // B's local role and A's roster both converge on the new role.
        EXPECT_TRUE( WaitForCondition(
            [ & ]() { return m_nodeB.membership->LocalRoleFor( roomId ) == chat::ROLE_MODERATOR; },
            kWaitTimeout ) );

        EXPECT_TRUE( WaitForCondition(
            [ & ]() {
                auto listA = m_nodeA.membership->MembersFor( roomId );
                auto listB = m_nodeB.membership->MembersFor( roomId );
                return listA.has_value() && listB.has_value() &&
                       RoleOf( listA.value(), SenderB() ) == chat::ROLE_MODERATOR &&
                       RoleOf( listB.value(), SenderB() ) == chat::ROLE_MODERATOR;
            },
            kWaitTimeout ) );

        // The creator remains the singular Super Admin on both sides (D-03).
        auto listA = m_nodeA.membership->MembersFor( roomId );
        auto listB = m_nodeB.membership->MembersFor( roomId );
        ASSERT_TRUE( listA.has_value() );
        ASSERT_TRUE( listB.has_value() );
        EXPECT_EQ( RoleOf( listA.value(), SenderA() ), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( listB.value(), SenderA() ), chat::ROLE_SUPER_ADMIN );
    }

    /**
     * @brief Test 3 (WR-06): a SPACE invite reaches a recipient that never
     *        joined the space and redeems there on the membership topic alone.
     *
     * Node B registers no topic derived from the space — the creator's member
     * record and the invite record must arrive on the well-known membership
     * replication topic, exactly as production init wiring does (04-11).
     */
    TEST_F( MembershipMultinodeTest, SpaceInviteConvergesWithoutTheRecipientJoiningTheSpace )
    {
        auto space = m_nodeA.store->CreateSpace( "ops", true, false, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        // The FFI's space path (04-11): a space has no room topic.
        ASSERT_TRUE(
            m_nodeA.membership->OnScopeCreated( spaceId, "", /*isSpace=*/true ).has_value() );

        auto minted = m_nodeA.membership->MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );
        EXPECT_EQ( minted.value().rfind( "gcs://invite/space/", 0 ), 0 );

        // B registered nothing for this space: the creator's member record and
        // the invite record must reach it on the membership replication topic.
        ASSERT_TRUE( WaitForCondition(
            [ & ]() {
                auto invites = m_nodeB.session->QueryKeyValues(
                    gcs::Membership::kInvitesKeyPrefix );
                auto members = m_nodeB.session->QueryKeyValues(
                    std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/" );
                return invites.has_value() && invites.value().size() == 1 &&
                       members.has_value() && members.value().size() == 1;
            },
            kWaitTimeout ) );

        ASSERT_TRUE( m_nodeB.membership->RedeemInvite( minted.value() ).has_value() );

        // Both rosters converge on B's freshly written member record.
        ASSERT_TRUE( WaitForCondition(
            [ & ]() {
                auto listA = m_nodeA.membership->MembersFor( spaceId );
                auto listB = m_nodeB.membership->MembersFor( spaceId );
                return listA.has_value() && listA.value().member_size() == 2 &&
                       listB.has_value() && listB.value().member_size() == 2;
            },
            kWaitTimeout ) );

        auto listA = m_nodeA.membership->MembersFor( spaceId );
        auto listB = m_nodeB.membership->MembersFor( spaceId );
        ASSERT_TRUE( listA.has_value() );
        ASSERT_TRUE( listB.has_value() );

        // The creator is a Super Admin on both sides; B's grant is the Guest
        // default stamped by MintInvite (D-05).
        EXPECT_EQ( RoleOf( listA.value(), SenderA() ), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( listB.value(), SenderA() ), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( listA.value(), SenderB() ), chat::ROLE_GUEST );
        EXPECT_EQ( RoleOf( listB.value(), SenderB() ), chat::ROLE_GUEST );
    }

    /**
     * @brief Test 4 (WR-04): the bearer key gates EVERY redeem on the wire.
     *
     * A childless space invite carries no wrapped room-key material, so it is
     * the case that exposes an invite gate conditioned on wrapped keys. After
     * the authentic redeem establishes a two-member baseline, a wrong key and a
     * forged record written straight through the session (empty authenticator)
     * must each be rejected without writing or rewriting any member record.
     */
    TEST_F( MembershipMultinodeTest, ForgedOrKeylessSpaceInviteWritesNoMemberRecord )
    {
        auto space = m_nodeA.store->CreateSpace( "secure", true, false, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        ASSERT_TRUE(
            m_nodeA.membership->OnScopeCreated( spaceId, "", /*isSpace=*/true ).has_value() );

        auto minted = m_nodeA.membership->MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );
        const std::string mintedUrl = minted.value();

        // 1. Authentic baseline. The invite record reaches B and redeems.
        ASSERT_TRUE( WaitForCondition(
            [ & ]() {
                auto invites = m_nodeB.session->QueryKeyValues(
                    gcs::Membership::kInvitesKeyPrefix );
                return invites.has_value() && invites.value().size() == 1;
            },
            kWaitTimeout ) );

        // The zero-key-material space invite proves the gate is unconditional:
        // the replicated record carries an authenticator but NO wrapped keys.
        auto replicated = m_nodeB.session->QueryKeyValues(
            gcs::Membership::kInvitesKeyPrefix );
        ASSERT_TRUE( replicated.has_value() );
        ASSERT_EQ( replicated.value().size(), 1U );
        chat::InviteRecord authentic;
        ASSERT_TRUE( authentic.ParseFromString( replicated.value().front().second ) );
        EXPECT_FALSE( authentic.authenticator().empty() );
        EXPECT_EQ( authentic.wrapped_room_key_size(), 0 );

        ASSERT_TRUE( m_nodeB.membership->RedeemInvite( mintedUrl ).has_value() );
        ASSERT_TRUE( WaitForCondition(
            [ & ]() {
                auto listA = m_nodeA.membership->MembersFor( spaceId );
                auto listB = m_nodeB.membership->MembersFor( spaceId );
                return listA.has_value() && listA.value().member_size() == 2 &&
                       listB.has_value() && listB.value().member_size() == 2;
            },
            kWaitTimeout ) );

        // Post-sub-case-1 baseline on the redeeming node: the creator plus B.
        const auto baselineMembers = m_nodeB.membership->MembersFor( spaceId );
        ASSERT_TRUE( baselineMembers.has_value() );
        ASSERT_EQ( baselineMembers.value().member_size(), 2 );
        const std::string memberPrefix =
            std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/";
        auto baselineScan = m_nodeB.session->QueryKeyValues( memberPrefix );
        ASSERT_TRUE( baselineScan.has_value() );
        const std::size_t baselineRecordCount = baselineScan.value().size();
        ASSERT_EQ( baselineRecordCount, 2U );
        const auto baselineRecord = m_nodeB.session->Get( memberPrefix + SenderB() );
        ASSERT_TRUE( baselineRecord.has_value() );

        // 2. Wrong key of the same base64url shape: parsed as a key, rejected
        // by the authenticator — the member-record set must not change.
        const std::string keyMarker = "?key=";
        const std::size_t keyPos    = mintedUrl.find( keyMarker );
        ASSERT_NE( keyPos, std::string::npos );
        const std::string wrongKeyUrl =
            mintedUrl.substr( 0, keyPos + keyMarker.size() ) + kWellFormedInviteKey;
        EXPECT_FALSE( m_nodeB.membership->RedeemInvite( wrongKeyUrl ).has_value() );

        auto afterWrongKeyScan = m_nodeB.session->QueryKeyValues( memberPrefix );
        ASSERT_TRUE( afterWrongKeyScan.has_value() );
        EXPECT_EQ( afterWrongKeyScan.value().size(), baselineRecordCount );
        auto afterWrongKeyMembers = m_nodeB.membership->MembersFor( spaceId );
        ASSERT_TRUE( afterWrongKeyMembers.has_value() );
        EXPECT_EQ( afterWrongKeyMembers.value().member_size(),
                   baselineMembers.value().member_size() );
        EXPECT_EQ( RoleOf( afterWrongKeyMembers.value(), SenderA() ),
                   chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( afterWrongKeyMembers.value(), SenderB() ), chat::ROLE_GUEST );

        // 3. Forged record with no authenticator, written at the WIRE level
        // under a distinct invite id on the membership replication topic.
        chat::InviteRecord forged = authentic;
        forged.clear_authenticator();
        const std::string forgedInviteId = "invite-forged-noauth";
        ASSERT_TRUE( m_nodeA.session
                         ->Put( std::string( gcs::Membership::kInvitesKeyPrefix ) + spaceId
                                    + "/" + forgedInviteId,
                                forged.SerializeAsString(),
                                std::unordered_set<std::string>{
                                    gcs::Membership::kMembershipTopic } )
                         .has_value() );
        ASSERT_TRUE( WaitForCondition(
            [ & ]() {
                auto seen = m_nodeB.session->QueryKeyValues(
                    gcs::Membership::kInvitesKeyPrefix );
                return seen.has_value() && seen.value().size() == 2;
            },
            kWaitTimeout ) );

        const std::string forgedUrl = "gcs://invite/space/" + spaceId + "/"
                                      + forgedInviteId + keyMarker
                                      + kWellFormedInviteKey;
        EXPECT_FALSE( m_nodeB.membership->RedeemInvite( forgedUrl ).has_value() );

        auto afterForgedScan = m_nodeB.session->QueryKeyValues( memberPrefix );
        ASSERT_TRUE( afterForgedScan.has_value() );
        EXPECT_EQ( afterForgedScan.value().size(), baselineRecordCount );
        const auto afterForgedRecord = m_nodeB.session->Get( memberPrefix + SenderB() );
        ASSERT_TRUE( afterForgedRecord.has_value() );
        EXPECT_EQ( afterForgedRecord.value(), baselineRecord.value() );
        auto afterForgedMembers = m_nodeB.membership->MembersFor( spaceId );
        ASSERT_TRUE( afterForgedMembers.has_value() );
        EXPECT_EQ( afterForgedMembers.value().member_size(),
                   baselineMembers.value().member_size() );
        EXPECT_EQ( RoleOf( afterForgedMembers.value(), SenderA() ),
                   chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( afterForgedMembers.value(), SenderB() ), chat::ROLE_GUEST );
    }

    /**
     * @brief Test 5 (CR-08): a wire-level record claiming the creator as its
     *        issuer but signed by an attacker key grants nothing.
     *
     * The record carries a VALID bearer authenticator, so the authenticator
     * gate passes; only issuer binding can reject it. The attacker signs the
     * canonical payload with its own key while created_by names the creator,
     * so verification against the CLAIMED issuer fails before any member write.
     */
    TEST_F( MembershipMultinodeTest, ForgedIssuerSignatureCannotGrantARoleOnTheWire )
    {
        auto space = m_nodeA.store->CreateSpace( "forged", true, false, true );
        ASSERT_TRUE( space.has_value() );
        const std::string spaceId = space.value().id();

        ASSERT_TRUE(
            m_nodeA.membership->OnScopeCreated( spaceId, "", /*isSpace=*/true ).has_value() );

        auto minted = m_nodeA.membership->MintInvite( spaceId );
        ASSERT_TRUE( minted.has_value() );

        const std::string scopedInvitePrefix =
            std::string( gcs::Membership::kInvitesKeyPrefix ) + spaceId + "/";
        const std::string memberPrefix =
            std::string( gcs::Membership::kMembersKeyPrefix ) + spaceId + "/";

        // The genuine record and the creator's member record must replicate to
        // B so the forged record starts from a well-formed shape.
        ASSERT_TRUE( WaitForCondition(
            [ & ]() {
                auto invites = m_nodeB.session->QueryKeyValues( scopedInvitePrefix );
                auto members = m_nodeB.session->QueryKeyValues( memberPrefix );
                return invites.has_value() && invites.value().size() == 1 &&
                       members.has_value() && members.value().size() == 1;
            },
            kWaitTimeout ) );

        auto replicated = m_nodeB.session->QueryKeyValues( scopedInvitePrefix );
        ASSERT_TRUE( replicated.has_value() );
        ASSERT_EQ( replicated.value().size(), 1U );
        chat::InviteRecord authentic;
        ASSERT_TRUE( authentic.ParseFromString( replicated.value().front().second ) );

        // Build the forged record: attacker's signature, the creator's claimed
        // identity, an escalated role, and a bearer authenticator minted with
        // an attacker-chosen key so the authenticator gate cannot be the
        // reason for rejection.
        sgns::GeniusSigner attacker = sgns::GeniusSigner::Generate();
        const std::string  forgedInviteId = "invite-forged-issuer";
        chat::InviteRecord forged         = authentic;
        forged.set_created_by( SenderA() );
        forged.set_role( chat::ROLE_SUPER_ADMIN );
        forged.clear_wrapped_room_key();

        // kWellFormedInviteKey is the unpadded base64url encoding of
        // kRoomKeyLengthBytes zero bytes, so the URL's key decodes to exactly
        // these bytes and this authenticator verifies.
        const std::vector<unsigned char> attackerInviteKey(
            gcs::crypto::kRoomKeyLengthBytes, 0 );
        auto attackerAuth = gcs::crypto::MintInviteAuthenticator(
            attackerInviteKey, spaceId, forgedInviteId );
        ASSERT_TRUE( attackerAuth.has_value() );
        forged.set_authenticator( attackerAuth.value() );

        const std::string forgedPayload = BuildInviteIssuerPayload( forgedInviteId, forged );
        const std::vector<uint8_t> forgedData( forgedPayload.begin(),
                                               forgedPayload.end() );
        const std::vector<uint8_t> forgedSignature = attacker.Sign( forgedData );
        ASSERT_EQ( forgedSignature.size(), gcs::Membership::kSignatureBytes );
        forged.set_issuer_signature( gcs::crypto::HexEncode(
            std::string( forgedSignature.begin(), forgedSignature.end() ) ) );

        // Write it at the WIRE level under the PUBLIC prefix on the membership
        // replication topic — the same channel any peer may publish on.
        ASSERT_TRUE( m_nodeA.session
                         ->Put( scopedInvitePrefix + forgedInviteId,
                                forged.SerializeAsString(),
                                std::unordered_set<std::string>{
                                    gcs::Membership::kMembershipTopic } )
                         .has_value() );
        ASSERT_TRUE( WaitForCondition(
            [ & ]() {
                auto seen = m_nodeB.session->QueryKeyValues( scopedInvitePrefix );
                return seen.has_value() && seen.value().size() == 2;
            },
            kWaitTimeout ) );

        // Baseline on B: the creator's record only. The genuine invite is never
        // redeemed here, so B has no member record at all.
        auto baselineScan = m_nodeB.session->QueryKeyValues( memberPrefix );
        ASSERT_TRUE( baselineScan.has_value() );
        const std::size_t baselineRecordCount = baselineScan.value().size();
        ASSERT_EQ( baselineRecordCount, 1U );
        EXPECT_FALSE( m_nodeB.session->Get( memberPrefix + SenderB() ).has_value() );

        // The issuer-signature check fires here: the record is signed by a key
        // that does not control created_by, after the bearer authenticator has
        // already passed.
        const std::string forgedUrl = "gcs://invite/space/" + spaceId + "/"
                                      + forgedInviteId + "?key=" + kWellFormedInviteKey;
        EXPECT_FALSE( m_nodeB.membership->RedeemInvite( forgedUrl ).has_value() );

        auto afterScan = m_nodeB.session->QueryKeyValues( memberPrefix );
        ASSERT_TRUE( afterScan.has_value() );
        EXPECT_EQ( afterScan.value().size(), baselineRecordCount );
        // Absent before and after: never compare bytes of a record that must
        // not exist.
        EXPECT_FALSE( m_nodeB.session->Get( memberPrefix + SenderB() ).has_value() );

        auto listB = m_nodeB.membership->MembersFor( spaceId );
        ASSERT_TRUE( listB.has_value() );
        // RoleOf returns ROLE_UNSPECIFIED when the address is absent.
        EXPECT_EQ( RoleOf( listB.value(), SenderB() ), chat::ROLE_UNSPECIFIED );
    }

    /**
     * @brief Test 6 (WR-05): a remote removal converges on the removed node as
     *        a PUSHED MemberList, not merely in its local CRDT scan.
     *
     * Node A removes B after both rosters converge; B's captured events are
     * drained first, so the only way the wait can pass is the arrival push that
     * 04-09's OnMemberArrived fix restored.
     */
    TEST_F( MembershipMultinodeTest, RemovedMemberReceivesTheConvergedRoster )
    {
        std::string roomId;
        CreateRoomAndJoin( roomId );

        auto before = m_nodeB.membership->MembersFor( roomId );
        ASSERT_TRUE( before.has_value() );
        EXPECT_EQ( RoleOf( before.value(), SenderB() ), chat::ROLE_GUEST );

        // The assertion must pin the POST-removal push: discard every event B
        // has already captured, including the pre-removal rosters.
        {
            std::lock_guard<std::mutex> lock( m_nodeB.sinkMutex );
            std::vector<chat::GcsEvent>().swap( m_nodeB.events );
        }

        // A is the creator (Super Admin rank > Guest) and acts alone.
        ASSERT_TRUE( m_nodeA.membership->RemoveMember( roomId, SenderB() ).has_value() );

        ASSERT_TRUE( WaitForCondition(
            [ & ]() {
                auto latest = LatestMemberListFor( m_nodeB, roomId );
                return latest.has_value() &&
                       RoleOf( latest.value(), SenderB() ) == chat::ROLE_UNSPECIFIED &&
                       RoleOf( latest.value(), SenderA() ) == chat::ROLE_SUPER_ADMIN;
            },
            kWaitTimeout ) );
    }

} // namespace gcs::test
