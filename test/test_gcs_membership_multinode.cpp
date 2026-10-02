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

#include "lib/gcs_crypto.hpp"
#include "lib/gcs_entity_store.hpp"
#include "lib/gcs_membership.hpp"
#include "test_graphsync_network.hpp"
#include "test_wait_condition.hpp"

#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <string>
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

    /// Distinct node wallet identities (D-03).
    constexpr const char *kSenderA = "5GrwvaEF5zXb26Fz9rcQpDWS57CtERHpNehXCPcNoHGKutQY";
    constexpr const char *kSenderB = "5FHneW46xGXgs5mUiveU4sbTyGBzmstUspZC92UhjJM694ty";

    /// Room chat topic prefix — the gcs/chat/<id> convention (D-01).
    constexpr const char *kRoomTopicPrefix = "gcs/chat/";

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
     */
    void ArmMembership( MultinodeNode &node, const std::string &sender )
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

        node.store = std::make_unique<gcs::EntityStore>( *node.session, sender );
        ASSERT_TRUE( node.store->LoadFromStore().has_value() );

        // Production D-06 posture: real vendored-OpenSSL seam, enabled. Built by
        // member assignment (KeyWrapSeam has a user-provided default ctor).
        gcs::Membership::KeyWrapSeam seam;
        seam.wrap    = &gcs::crypto::WrapRoomKey;
        seam.unwrap  = &gcs::crypto::UnwrapRoomKey;
        seam.enabled = true;

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
            ArmMembership( m_nodeA, kSenderA );
            ArmMembership( m_nodeB, kSenderB );

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

            // Both nodes register the room topic (listen first, then broadcast —
            // D-07), exactly as the FFI join wiring does in production, so the
            // CRDT records written on it replicate across the mesh.
            for ( MultinodeNode *node : { &m_nodeA, &m_nodeB } )
            {
                ASSERT_TRUE( node->session->AddListenTopic( topic ).has_value() );
                ASSERT_TRUE( node->session->AddBroadcastTopic( topic ).has_value() );
            }

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

        std::string   m_tempPath; ///< Per-test temp root (removed in TearDown)
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
        EXPECT_EQ( RoleOf( listA.value(), kSenderA ), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( listB.value(), kSenderA ), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( listA.value(), kSenderB ), chat::ROLE_GUEST );
        EXPECT_EQ( RoleOf( listB.value(), kSenderB ), chat::ROLE_GUEST );

        // self_address is the pushed local wallet on each side (D-04).
        EXPECT_EQ( listA.value().self_address(), kSenderA );
        EXPECT_EQ( listB.value().self_address(), kSenderB );
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
        ASSERT_TRUE( m_nodeA.membership->ChangeRole( roomId, kSenderB, chat::ROLE_MODERATOR ).has_value() );

        // B's local role and A's roster both converge on the new role.
        EXPECT_TRUE( WaitForCondition(
            [ & ]() { return m_nodeB.membership->LocalRoleFor( roomId ) == chat::ROLE_MODERATOR; },
            kWaitTimeout ) );

        EXPECT_TRUE( WaitForCondition(
            [ & ]() {
                auto listA = m_nodeA.membership->MembersFor( roomId );
                auto listB = m_nodeB.membership->MembersFor( roomId );
                return listA.has_value() && listB.has_value() &&
                       RoleOf( listA.value(), kSenderB ) == chat::ROLE_MODERATOR &&
                       RoleOf( listB.value(), kSenderB ) == chat::ROLE_MODERATOR;
            },
            kWaitTimeout ) );

        // The creator remains the singular Super Admin on both sides (D-03).
        auto listA = m_nodeA.membership->MembersFor( roomId );
        auto listB = m_nodeB.membership->MembersFor( roomId );
        ASSERT_TRUE( listA.has_value() );
        ASSERT_TRUE( listB.has_value() );
        EXPECT_EQ( RoleOf( listA.value(), kSenderA ), chat::ROLE_SUPER_ADMIN );
        EXPECT_EQ( RoleOf( listB.value(), kSenderA ), chat::ROLE_SUPER_ADMIN );
    }

} // namespace gcs::test
