/**
 * @file       test_gcs_messaging_multinode.cpp
 * @brief      Two-node in-process integration tests for the end-to-end messaging
 *             path (SC1-SC3 + D-08 live/at-rest/restart) over the REAL GossipSub
 *             transport: two live nodes (own key + db dir, DEFAULT gossip
 *             config so echo is OFF and delivery is genuinely cross-node),
 *             explicit AddPeers connect, and the FFI join wiring mirrored per
 *             node (listen + broadcast topic, live subscribe, CRDT receive
 *             bridge). The REAL gcs::crypto seam (EncryptPayload/DecryptPayload,
 *             enabled) is injected into every Messaging instance — the D-08
 *             production posture. Follows the ../SuperGenius
 *             globaldb_integration.cpp multinode pattern.
 * @details    Three tests:
 *               1. LiveDeliveryAndHistoryConvergeAcrossNodes — live cross-node
 *                  delivery + converged identical histories, both directions.
 *               2. AtRestOpacityBothSides — binary scan of BOTH nodes' db dirs
 *                  finds zero plaintext marker bytes (D-08 ciphertext at rest).
 *               3. HistoryReplaysAfterRestart — session teardown + re-init on
 *                  the same db dir replays the full history decrypted and
 *                  sorted (D-06 restart replay).
 *             All waits use the GCS wait-condition template — never a raw sleep.
 * @date       2026-09-23
 */

#include "lib/gcs_crypto.hpp"
#include "lib/gcs_messaging.hpp"
#include "test_graphsync_network.hpp"
#include "test_wait_condition.hpp"

#include <algorithm>
#include <chrono>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <mutex>
#include <set>
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

    /// The single room exercised by the mesh (room topic is a PUBLIC routing
    /// key per D-02 — never a secret).
    constexpr const char *kRoom = "room-multinode";

    /// Distinct injected senders (display-only per D-04).
    constexpr const char *kSenderA = "0xAAAA";
    constexpr const char *kSenderB = "0xBBBB";

    /// Unique plaintext markers for the live/convergence test.
    constexpr const char *kMarkerA = "multinode-live-marker-alpha";
    constexpr const char *kMarkerB = "multinode-live-marker-beta";

    /// Distinctive plaintext marker whose bytes must NEVER appear on disk (D-08).
    constexpr const char *kOpaqueMarker = "multinode-opacity-marker-7f3a9c";

    /// Distinct texts for the restart replay exchange (one per direction).
    constexpr const char *kRestartFirst  = "multinode-restart-first";
    constexpr const char *kRestartSecond = "multinode-restart-second";

    /// Minimal soralog YAML — console sink only, error level, sufficient to
    /// satisfy libp2p::log::setLoggingSystem() before any SuperGenius logger is
    /// constructed (mirrors test_gcs_messaging.cpp).
    constexpr const char *kLoggingYaml = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_messaging_multinode_test
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
     * Member destruction order is reverse declaration order, so a
     * half-configured node still tears down safely: messaging (borrows the
     * session reference) dies first, then the session (borrows pubsub +
     * graphsync Network), then graphsync, then pubsub. The fixture's
     * ShutdownNode() performs the explicit quiesce-before-release sequence
     * (session shutdown -> pubsub stop -> messaging reset) before the members
     * fall out of scope, matching the test_gcs_messaging.cpp bring-down order.
     */
    struct MultinodeNode
    {
        std::string basePath; ///< Node tmp dir (key store + db live here)
        std::string dbPath;   ///< RocksDB path for the CRDT store (basePath + "/db")
        std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> pubsub;
        TestGraphsyncContext graphsync;
        std::unique_ptr<gcs::CoreSession> session;
        std::mutex sinkMutex;               ///< Guards events
        std::vector<chat::GcsEvent> events; ///< Sink captures (mutex-guarded)
        std::unique_ptr<gcs::Messaging> messaging;
    };

    /**
     * @brief Stand up the transport half of a node: key store + a real
     *        GossipPubSub bound to port 0 + the graphsync lifetime bundle.
     *
     * The DEFAULT single-arg GossipPubSub constructor is used, which means the
     * gossip Config's echo_forward_mode is FALSE — delivery is genuinely
     * cross-node (no local loopback), unlike the single-node messaging suite.
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

        // Default gossip config: echo_forward_mode stays FALSE so the live
        // publish is delivered to the peer over the real transport.
        node.pubsub = std::make_shared<sgns::ipfs_pubsub::GossipPubSub>( keyPairResult.value() );
        auto startFuture = node.pubsub->Start( 0, {}, kListenIp, {} ); // port 0 = ephemeral
        auto startError  = startFuture.get();
        ASSERT_FALSE( startError ) << "Could not start GossipPubSub: " << startError.message();

        node.graphsync = MakeGraphsyncContext( node.pubsub );
    }

    /**
     * @brief Construct + initialize the CoreSession over the injected pubsub +
     *        graphsync seam (gcs_core.hpp injected-seam — NO GeniusSDK).
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
     * @brief Mirror the FFI join wiring (kJoinTopic arm + gcs_init receive
     *        bridge) for the room, then construct the Messaging component with
     *        the REAL production crypto seam injected and enabled (D-08).
     *
     * The subscribe and receive-bridge lambdas capture the node by pointer and
     * fire on the GossipSub strand / CRDT DagWorker threads respectively, so
     * they only read the node's messaging pointer (never reassigned while the
     * transport is live) — no data race (T-03-14).
     *
     * @param[in,out] node   The node being wired.
     * @param[in]     sender The local sender address (D-04 authority stamp).
     */
    void ArmMessaging( MultinodeNode &node, const std::string &sender )
    {
        // CRDT archive topics (D-02): listen first, then broadcast (D-07).
        ASSERT_TRUE( node.session->AddListenTopic( kRoom ).has_value() );
        ASSERT_TRUE( node.session->AddBroadcastTopic( kRoom ).has_value() );

        // Live fast path (D-03): raw GossipSub subscribe -> Messaging receive.
        ASSERT_TRUE( node.session->Subscribe(
                          kRoom,
                          [ &node ]( const std::string &topic, const std::string &data ) {
                              if ( node.messaging )
                              {
                                  node.messaging->OnLiveMessage( topic, data );
                              }
                          } )
                          .has_value() );

        // CRDT heal path (D-03): new-element callback -> Messaging receive.
        ASSERT_TRUE( node.session->RegisterNewElementCallback(
                          gcs::Messaging::kMessagesKeyCallbackPattern,
                          [ &node ]( const std::string &key, const std::string &value ) {
                              if ( node.messaging )
                              {
                                  node.messaging->OnMessageArrived( key, value );
                              }
                          } )
                          .has_value() );

        // Production D-08 posture: real vendored-OpenSSL seam, enabled. Built
        // by member assignment (CryptoSeam has a user-provided default ctor,
        // so brace-init would not compile).
        gcs::Messaging::CryptoSeam seam;
        seam.encrypt = &gcs::crypto::EncryptPayload;
        seam.decrypt = &gcs::crypto::DecryptPayload;
        seam.enabled = true;

        node.messaging = std::make_unique<gcs::Messaging>(
            *node.session, sender,
            [ &node ]( const chat::GcsEvent &event ) {
                std::lock_guard<std::mutex> lock( node.sinkMutex );
                node.events.push_back( event );
            },
            seam );
    }

    /**
     * @brief Quiesce then release a node: stop the session (CRDT threads), stop
     *        the pubsub (live strand), THEN release the messaging component so
     *        no GossipSub/DagWorker callback can touch it mid-destruction.
     *
     * @param[in,out] node The node to tear down.
     */
    void ShutdownNode( MultinodeNode &node )
    {
        if ( node.session )
        {
            node.session->Shutdown();
        }
        // Destroy the messaging component (drains + joins its archive worker)
        // BEFORE the session it borrows, then the session, then the transport.
        if ( node.messaging )
        {
            node.messaging.reset();
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
     * @brief Whether the node's event sink has captured a message with @p text.
     *
     * @param[in,out] node The node to inspect (sink mutex is taken).
     * @param[in]     text The plaintext text to search for.
     * @return true when any captured message event carries @p text.
     */
    bool NodeHasMessageText( MultinodeNode &node, const std::string &text )
    {
        std::lock_guard<std::mutex> lock( node.sinkMutex );
        for ( const chat::GcsEvent &event : node.events )
        {
            if ( event.has_message() && event.message().text() == text )
            {
                return true;
            }
        }
        return false;
    }

    /**
     * @brief Binary-scan every regular file under @p dirPath for the plaintext
     *        marker bytes (portable recursive directory walk + ifstream — no
     *        external grep). Level-DB WAL/log files are included via recursion.
     *
     * @param[in] dirPath Directory to scan (the node's db dir).
     * @param[in] marker  The plaintext marker bytes to search for.
     * @return The number of regular files containing the marker.
     */
    size_t CountPlaintextHits( const std::string &dirPath, const std::string &marker )
    {
        size_t         hits = 0;
        std::error_code ec;
        if ( !std::filesystem::is_directory( dirPath, ec ) || ec )
        {
            return hits;
        }

        std::filesystem::recursive_directory_iterator it( dirPath, ec );
        const std::filesystem::recursive_directory_iterator end;
        while ( !ec && it != end )
        {
            std::error_code fileEc;
            if ( it->is_regular_file( fileEc ) && !fileEc )
            {
                std::ifstream in( it->path().string(), std::ios::binary );
                if ( in )
                {
                    const std::string content( ( std::istreambuf_iterator<char>( in ) ),
                                               std::istreambuf_iterator<char>() );
                    if ( content.find( marker ) != std::string::npos )
                    {
                        ++hits;
                    }
                }
            }
            it.increment( ec );
        }
        return hits;
    }

    /**
     * @brief Two-node mesh fixture: two fully wired nodes (A + B), cross-
     *        connected via AddPeers and confirmed ready before each test.
     */
    class MessagingMultinodeTest : public ::testing::Test
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
                           / ( std::string{ "gcs_messaging_multinode_" } + info->name() + "_" +
                               std::to_string( uniqueSalt ) ) )
                             .string();
            std::filesystem::create_directories( m_tempPath );

            StartTransport( m_nodeA, m_tempPath + "/a" );
            StartTransport( m_nodeB, m_tempPath + "/b" );
            StartSession( m_nodeA, m_nodeA.basePath + "/db" );
            StartSession( m_nodeB, m_nodeB.basePath + "/db" );
            ArmMessaging( m_nodeA, kSenderA );
            ArmMessaging( m_nodeB, kSenderB );

            // If bring-up failed anywhere, the messaging component stays null —
            // abort SetUp cleanly (fatal in a fixture) rather than deref null in
            // ConnectNodes / the test body.
            ASSERT_NE( m_nodeA.messaging, nullptr );
            ASSERT_NE( m_nodeB.messaging, nullptr );

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
         * @brief Explicitly cross-connect the two nodes and wait for the mesh
         *        to be ready (both connection managers see the peer).
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

        std::string    m_tempPath; ///< Per-test temp root (created in SetUp, removed in TearDown)
        MultinodeNode m_nodeA;     ///< First mesh node (sender 0xAAAA)
        MultinodeNode m_nodeB;     ///< Second mesh node (sender 0xBBBB)
    };

    /**
     * @brief Test 1 (SC1-SC3): live cross-node delivery in BOTH directions plus
     *        converged, identical histories through the real GossipSub transport.
     */
    TEST_F( MessagingMultinodeTest, LiveDeliveryAndHistoryConvergeAcrossNodes )
    {
        // A -> B: B's sink captures the message via the live path (no refresh
        // analog) — proof of genuine cross-node delivery.
        ASSERT_TRUE( m_nodeA.messaging->SendMessage( kRoom, kMarkerA ).has_value() );
        EXPECT_TRUE( WaitForCondition(
            [ & ]() { return NodeHasMessageText( m_nodeB, kMarkerA ); }, kWaitTimeout ) );

        // Both histories converge on the single message.
        EXPECT_TRUE( WaitForCondition(
            [ & ]() {
                auto historyA = m_nodeA.messaging->QueryHistory( kRoom );
                auto historyB = m_nodeB.messaging->QueryHistory( kRoom );
                return historyA.has_value() && historyA.value().message_size() == 1 &&
                       historyB.has_value() && historyB.value().message_size() == 1;
            },
            kWaitTimeout ) );

        // B -> A: reverse direction.
        ASSERT_TRUE( m_nodeB.messaging->SendMessage( kRoom, kMarkerB ).has_value() );
        EXPECT_TRUE( WaitForCondition(
            [ & ]() { return NodeHasMessageText( m_nodeA, kMarkerB ); }, kWaitTimeout ) );

        EXPECT_TRUE( WaitForCondition(
            [ & ]() {
                auto historyA = m_nodeA.messaging->QueryHistory( kRoom );
                auto historyB = m_nodeB.messaging->QueryHistory( kRoom );
                return historyA.has_value() && historyA.value().message_size() == 2 &&
                       historyB.has_value() && historyB.value().message_size() == 2;
            },
            kWaitTimeout ) );

        const auto historyA = m_nodeA.messaging->QueryHistory( kRoom );
        const auto historyB = m_nodeB.messaging->QueryHistory( kRoom );
        ASSERT_TRUE( historyA.has_value() );
        ASSERT_TRUE( historyB.has_value() );
        ASSERT_EQ( historyA.value().message_size(), 2 );
        ASSERT_EQ( historyB.value().message_size(), 2 );

        // Both histories carry both markers.
        std::set<std::string> textsA;
        std::set<std::string> textsB;
        for ( int i = 0; i < historyA.value().message_size(); ++i )
        {
            textsA.insert( historyA.value().message( i ).text() );
            textsB.insert( historyB.value().message( i ).text() );
        }
        EXPECT_EQ( textsA, ( std::set<std::string>{ kMarkerA, kMarkerB } ) );
        EXPECT_EQ( textsB, ( std::set<std::string>{ kMarkerA, kMarkerB } ) );

        // Converged histories are identical per-index (id, timestamp, text).
        for ( int i = 0; i < historyA.value().message_size(); ++i )
        {
            const auto &msgA = historyA.value().message( i );
            const auto &msgB = historyB.value().message( i );
            EXPECT_EQ( msgA.id(), msgB.id() );
            EXPECT_EQ( msgA.timestamp(), msgB.timestamp() );
            EXPECT_EQ( msgA.text(), msgB.text() );
        }
    }

    /**
     * @brief Test 2 (D-08 at-rest): after both histories display the message,
     *        a binary scan of BOTH nodes' db dirs finds zero plaintext marker
     *        occurrences (disk carries nonce||ciphertext||tag only).
     */
    TEST_F( MessagingMultinodeTest, AtRestOpacityBothSides )
    {
        ASSERT_TRUE( m_nodeA.messaging->SendMessage( kRoom, kOpaqueMarker ).has_value() );

        // Both histories display the marker (live + archive converged).
        EXPECT_TRUE( WaitForCondition(
            [ & ]() {
                auto historyA = m_nodeA.messaging->QueryHistory( kRoom );
                auto historyB = m_nodeB.messaging->QueryHistory( kRoom );
                return historyA.has_value() && historyA.value().message_size() == 1 &&
                       historyA.value().message( 0 ).text() == kOpaqueMarker &&
                       historyB.has_value() && historyB.value().message_size() == 1 &&
                       historyB.value().message( 0 ).text() == kOpaqueMarker;
            },
            kWaitTimeout ) );

        // Zero plaintext marker bytes on either node's disk (D-08; the room
        // topic string is a public routing key by design — not scanned).
        EXPECT_EQ( CountPlaintextHits( m_nodeA.dbPath, kOpaqueMarker ), 0U );
        EXPECT_EQ( CountPlaintextHits( m_nodeB.dbPath, kOpaqueMarker ), 0U );
    }

    /**
     * @brief Test 3 (D-06 restart replay): after a two-message exchange (one
     *        per direction), tearing down and re-initializing node A on the SAME
     *        db dir replays the full history decrypted and sorted oldest-first.
     */
    TEST_F( MessagingMultinodeTest, HistoryReplaysAfterRestart )
    {
        // Exchange >= 2 messages (one per direction).
        ASSERT_TRUE( m_nodeA.messaging->SendMessage( kRoom, kRestartFirst ).has_value() );
        EXPECT_TRUE( WaitForCondition(
            [ & ]() { return NodeHasMessageText( m_nodeB, kRestartFirst ); }, kWaitTimeout ) );

        ASSERT_TRUE( m_nodeB.messaging->SendMessage( kRoom, kRestartSecond ).has_value() );
        EXPECT_TRUE( WaitForCondition(
            [ & ]() { return NodeHasMessageText( m_nodeA, kRestartSecond ); }, kWaitTimeout ) );

        // Both nodes converge on the two-message history before the restart.
        EXPECT_TRUE( WaitForCondition(
            [ & ]() {
                auto historyA = m_nodeA.messaging->QueryHistory( kRoom );
                auto historyB = m_nodeB.messaging->QueryHistory( kRoom );
                return historyA.has_value() && historyA.value().message_size() == 2 &&
                       historyB.has_value() && historyB.value().message_size() == 2;
            },
            kWaitTimeout ) );

        const std::string dbPath = m_nodeA.dbPath;

        // Tear down node A's session and rebuild on the SAME db dir (fresh
        // transport/key is fine — the archive lives in dbPath, and the room key
        // is derived from the public room topic so decryption survives the
        // process-lifetime boundary, D-06/D-08).
        ShutdownNode( m_nodeA );
        StartTransport( m_nodeA, m_tempPath + "/a-restart" );
        StartSession( m_nodeA, dbPath );
        ArmMessaging( m_nodeA, kSenderA );

        auto history = m_nodeA.messaging->QueryHistory( kRoom );
        ASSERT_TRUE( history.has_value() );
        ASSERT_EQ( history.value().message_size(), 2 );

        std::set<std::string> texts;
        for ( int i = 0; i < history.value().message_size(); ++i )
        {
            texts.insert( history.value().message( i ).text() );
        }
        EXPECT_EQ( texts, ( std::set<std::string>{ kRestartFirst, kRestartSecond } ) );

        // Replayed history is sorted oldest -> newest (timestamps non-decreasing).
        for ( int i = 1; i < history.value().message_size(); ++i )
        {
            EXPECT_LE( history.value().message( i - 1 ).timestamp(),
                       history.value().message( i ).timestamp() );
        }
    }

} // namespace gcs::test
