/**
 * @file       test_gcs_messaging.cpp
 * @brief      gcs::Messaging component unit tests (D-01/D-03/D-04/D-06/D-07/D-08):
 *             send lifecycle, sender stamp, two-route dedupe, peer role flip,
 *             history sort, restart persistence, history role flip, encrypted
 *             round-trip + wire opacity, at-rest opacity, wrong-key skip-and-log,
 *             and the plaintext path with the injected seam never called.
 * @details    Uses an injected-pubsub CoreSession with echo_forward_mode=true so
 *             the single-node live publish is observable through a local loopback
 *             subscription. The original seven tests run on an UNENCRYPTED
 *             instance; the D-08 tests construct the injected CryptoSeam. Uses the
 *             GCS wait-condition template — never a raw thread sleep.
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
    /// GossipPubSub bind address used by every test.
    constexpr const char *kListenIp = "0.0.0.0";

    /// Room topics + archive prefix exercised by the tests.
    constexpr const char *kRoomA    = "roomA";
    constexpr const char *kRoomB    = "roomB";
    constexpr const char *kPrefixA  = "gcs/messages/roomA/";

    /// Fixed injected sender + a distinct peer sender for role-flip coverage.
    constexpr const char *kSender     = "0xAAA";
    constexpr const char *kPeerSender = "0xBBB";

    /// Minimal soralog YAML — console sink only, error level, sufficient to satisfy
    /// libp2p::log::setLoggingSystem() before any SuperGenius logger is constructed.
    constexpr const char *kLoggingYaml = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_messaging_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";
} // namespace

namespace gcs::test
{
    /// Raw GossipSub subscription callback type (Gossip::SubscriptionCallback).
    using SubscriptionData = libp2p::protocol::gossip::Gossip::SubscriptionData;

    /**
     * @brief Thread-safe recorder for the raw GossipSub loopback subscription.
     *
     * The subscription callback fires on the pubsub strand thread; the test
     * polls Count() via WaitForCondition and reads back with At() under the
     * same mutex so no data race occurs.
     */
    class RawRecorder
    {
    public:
        /**
         * @brief Record a delivered (topic, data) pair.
         */
        void Record( const std::string &topic, const std::string &data )
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            m_messages.emplace_back( topic, data );
        }

        /**
         * @brief Number of delivered messages so far.
         */
        size_t Count()
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            return m_messages.size();
        }

        /**
         * @brief The delivered pair at @p index (index 0 = first delivery).
         */
        std::pair<std::string, std::string> At( size_t index )
        {
            std::lock_guard<std::mutex> lock( m_mutex );
            return m_messages.at( index );
        }

    private:
        std::mutex                                    m_mutex;
        std::vector<std::pair<std::string, std::string>> m_messages;
    };

    /**
     * @brief Builds a fully-stamped ChatMessageState for direct-write and
     *        delivery-path fixtures.
     */
    chat::ChatMessageState MakeMessage( const std::string &roomTopic,
                                        const std::string &id,
                                        const std::string &text,
                                        const std::string &sender,
                                        int64_t timestamp,
                                        chat::MessageRole role = chat::MESSAGE_ROLE_USER_SELF )
    {
        chat::ChatMessageState msg;
        msg.set_id( id );
        msg.set_room_topic( roomTopic );
        msg.set_role( role );
        msg.set_state( chat::MESSAGE_STATE_COMPLETE );
        msg.set_text( text );
        msg.set_timestamp( timestamp );
        msg.set_sender( sender );
        return msg;
    }

    /**
     * @brief Finds the (found, value) for the first scan entry whose raw key
     *        contains @p fragment. QueryKeyValues returns datastore-internal
     *        keys (/crdt/k/<key>/v), so match on the embedded message id.
     */
    std::pair<bool, std::string> ValueForKeyFragment(
        const std::vector<std::pair<std::string, std::string>> &entries,
        const std::string &fragment )
    {
        for ( const auto &entry : entries )
        {
            if ( entry.first.find( fragment ) != std::string::npos )
            {
                return std::make_pair( true, entry.second );
            }
        }
        return std::make_pair( false, std::string{} );
    }

    /**
     * @brief Fixture that owns a per-test temp directory and the echo-enabled
     *        injected-pubsub bring-up recipe.
     */
    class GcsMessagingTest : public ::testing::Test
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
                           / ( std::string{ "gcs_messaging_" } + info->name() + "_" +
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
         * @brief Stand up a real GossipPubSub bound to port 0 with
         *        echo_forward_mode = true so the single-node live publish is
         *        observable through a local loopback subscription.
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
            libp2p::protocol::gossip::Config gossipConfig{};
            gossipConfig.echo_forward_mode = true; // local loopback for live-path observation
            auto pubsub = std::make_shared<sgns::ipfs_pubsub::GossipPubSub>( keyPairResult.value(),
                                                                             gossipConfig );
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
     * @brief Test 1 (D-03/D-07): send pushes pending then complete with the
     *        same id, archives under gcs/messages/<room>/<id>, and publishes
     *        the full serialized ChatMessageState on the room topic (not a CID).
     */
    TEST_F( GcsMessagingTest, SendLifecyclePushesPendingThenCompleteAndArchives )
    {
        RawRecorder recorder;
        auto        pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        const auto dbPath = cfg.m_dbPath;
        EXPECT_TRUE( WaitForCondition( [ &dbPath ]() { return std::filesystem::exists( dbPath ); },
                                       kWaitTimeout ) );

        // Pre-registered raw subscription (active before the send).
        auto subscription = pubsub->Subscribe(
                                    kRoomA,
                                    [ &recorder ]( const SubscriptionData &data ) {
                                        if ( !data )
                                        {
                                            return;
                                        }
                                        const auto &msg = data.get();
                                        recorder.Record( msg.topic,
                                                         std::string( msg.data.begin(), msg.data.end() ) );
                                    } )
                                .get();
        ASSERT_NE( subscription, nullptr );

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); } );

        auto sendResult = messaging.SendMessage( kRoomA, "hi" );
        ASSERT_TRUE( sendResult.has_value() );

        // Two events in order: pending then complete, same non-empty id.
        ASSERT_EQ( events.size(), 2U );
        ASSERT_TRUE( events[ 0 ].has_message() );
        ASSERT_TRUE( events[ 1 ].has_message() );
        EXPECT_EQ( events[ 0 ].message().state(), chat::MESSAGE_STATE_PENDING );
        EXPECT_EQ( events[ 1 ].message().state(), chat::MESSAGE_STATE_COMPLETE );
        EXPECT_FALSE( events[ 0 ].message().id().empty() );
        EXPECT_EQ( events[ 0 ].message().id(), events[ 1 ].message().id() );

        // Archive: exactly one entry whose raw key contains the message id.
        auto scan = session.QueryKeyValues( kPrefixA );
        ASSERT_TRUE( scan.has_value() );
        ASSERT_EQ( scan.value().size(), 1U );
        EXPECT_NE( scan.value().front().first.find( events[ 0 ].message().id() ),
                   std::string::npos );

        // Live path: the loopback subscription received the FULL serialized
        // ChatMessageState (parseable, text/id/sender intact) — never a CID.
        EXPECT_TRUE( WaitForCondition( [ &recorder ]() { return recorder.Count() >= 1; }, kWaitTimeout ) );
        const auto recorded = recorder.At( 0 );
        EXPECT_EQ( recorded.first, kRoomA );
        chat::ChatMessageState liveMsg;
        ASSERT_TRUE( liveMsg.ParseFromString( recorded.second ) );
        EXPECT_EQ( liveMsg.text(), "hi" );
        EXPECT_EQ( liveMsg.id(), events[ 0 ].message().id() );
        EXPECT_EQ( liveMsg.sender(), kSender );

        subscription->cancel();
        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Test 2 (D-04): the complete event carries the injected sender,
     *        the room topic, and MESSAGE_ROLE_USER_SELF.
     */
    TEST_F( GcsMessagingTest, CompleteEventCarriesStampedSenderAndSelfRole )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); } );

        ASSERT_TRUE( messaging.SendMessage( kRoomA, "stamp" ).has_value() );
        ASSERT_GE( events.size(), 2U );
        const auto &complete = events[ 1 ].message();
        EXPECT_EQ( complete.sender(), kSender );
        EXPECT_EQ( complete.room_topic(), kRoomA );
        EXPECT_EQ( complete.role(), chat::MESSAGE_ROLE_USER_SELF );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Test 3 (D-03): the same message bytes delivered via BOTH routes
     *        (live + CRDT heal) are applied exactly once (id-keyed dedupe) and
     *        archived exactly once.
     */
    TEST_F( GcsMessagingTest, TwoRouteDeliveryIsDedupedByMessageId )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); } );

        const chat::ChatMessageState peerMsg =
            MakeMessage( kRoomA, "m-1", "hello", kPeerSender, 1000 );
        const std::string serialized = peerMsg.SerializeAsString();

        messaging.OnLiveMessage( kRoomA, serialized );
        messaging.OnMessageArrived( "gcs/messages/roomA/m-1", serialized );

        ASSERT_EQ( events.size(), 1U );
        EXPECT_EQ( events[ 0 ].message().id(), "m-1" );

        // The receive-path archive write is async (dedicated worker) — wait for
        // the converged archive to contain exactly the one deduped entry.
        EXPECT_TRUE( WaitForCondition(
            [ &session ]() {
                auto scan = session.QueryKeyValues( kPrefixA );
                return scan.has_value() && scan.value().size() == 1U;
            },
            kWaitTimeout ) );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Test 4 (D-04): peer-stamped messages flip to MESSAGE_ROLE_USER_PEER
     *        on BOTH the live route and the CRDT heal route.
     */
    TEST_F( GcsMessagingTest, PeerMessagesFlipRoleOnBothRoutes )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); } );

        const chat::ChatMessageState livePeer =
            MakeMessage( kRoomA, "m-8", "live-peer", kPeerSender, 1000 );
        const chat::ChatMessageState healPeer =
            MakeMessage( kRoomA, "m-9", "heal-peer", kPeerSender, 1001 );

        messaging.OnLiveMessage( kRoomA, livePeer.SerializeAsString() );
        messaging.OnMessageArrived( "gcs/messages/roomA/m-9", healPeer.SerializeAsString() );

        ASSERT_EQ( events.size(), 2U );
        EXPECT_EQ( events[ 0 ].message().role(), chat::MESSAGE_ROLE_USER_PEER );
        EXPECT_EQ( events[ 1 ].message().role(), chat::MESSAGE_ROLE_USER_PEER );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Test 5 (D-01): QueryHistory returns the room's messages sorted by
     *        (timestamp asc, id asc) — the timestamp tie broken by id.
     */
    TEST_F( GcsMessagingTest, QueryHistorySortsByTimestampThenId )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        const chat::ChatMessageState m1 = MakeMessage( kRoomA, "id-1", "a", kSender, 100 );
        const chat::ChatMessageState m2 = MakeMessage( kRoomA, "id-2", "b", kSender, 300 );
        const chat::ChatMessageState m3 = MakeMessage( kRoomA, "id-3", "c", kSender, 200 );
        const chat::ChatMessageState m4 = MakeMessage( kRoomA, "id-4", "d", kSender, 200 ); // tie with id-3

        ASSERT_TRUE( session.Put( "gcs/messages/roomA/id-1", m1.SerializeAsString() ).has_value() );
        ASSERT_TRUE( session.Put( "gcs/messages/roomA/id-2", m2.SerializeAsString() ).has_value() );
        ASSERT_TRUE( session.Put( "gcs/messages/roomA/id-3", m3.SerializeAsString() ).has_value() );
        ASSERT_TRUE( session.Put( "gcs/messages/roomA/id-4", m4.SerializeAsString() ).has_value() );

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); } );

        auto history = messaging.QueryHistory( kRoomA );
        ASSERT_TRUE( history.has_value() );
        EXPECT_EQ( history.value().room_topic(), kRoomA );
        ASSERT_EQ( history.value().message_size(), 4 );

        const std::vector<std::string> expected = { "id-1", "id-3", "id-4", "id-2" };
        for ( int i = 0; i < history.value().message_size(); ++i )
        {
            EXPECT_EQ( history.value().message( i ).id(), expected[ static_cast<size_t>( i ) ] );
        }

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Test 6 (restart): a message archived in session A survives into
     *        session B on the same db_path and is returned by QueryHistory.
     */
    TEST_F( GcsMessagingTest, MessageSurvivesSessionRestartOnSharedDb )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        const std::string dbPath = m_tempPath + "/db";
        {
            gcs::CoreSession::Config cfg{};
            cfg.m_dbPath = dbPath;
            gcs::CoreSession sessionA( cfg );
            ASSERT_TRUE( sessionA.Initialize( pubsub, graphsync.network ).has_value() );
            EXPECT_TRUE( WaitForCondition( [ &dbPath ]() { return std::filesystem::exists( dbPath ); },
                                           kWaitTimeout ) );

            std::vector<chat::GcsEvent> events;
            gcs::Messaging messaging( sessionA, kSender,
                                      [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); } );
            ASSERT_TRUE( messaging.SendMessage( kRoomA, "persist-me" ).has_value() );
            sessionA.Shutdown();
        }
        {
            gcs::CoreSession::Config cfg{};
            cfg.m_dbPath = dbPath;
            gcs::CoreSession sessionB( cfg );
            ASSERT_TRUE( sessionB.Initialize( pubsub, graphsync.network ).has_value() );

            std::vector<chat::GcsEvent> events;
            gcs::Messaging messaging( sessionB, kSender,
                                      [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); } );

            auto history = messaging.QueryHistory( kRoomA );
            ASSERT_TRUE( history.has_value() );
            ASSERT_EQ( history.value().message_size(), 1 );
            EXPECT_EQ( history.value().message( 0 ).text(), "persist-me" );

            sessionB.Shutdown();
        }
        pubsub->Stop();
    }

    /**
     * @brief Test 7 (D-04/D-06): replayed history flips a peer-stamped message
     *        to USER_PEER while a self-stamped message stays USER_SELF.
     */
    TEST_F( GcsMessagingTest, HistoryFlipsPeerRoleButKeepsSelfRole )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        const chat::ChatMessageState peerMsg =
            MakeMessage( kRoomA, "peer-1", "from-peer", kPeerSender, 100, chat::MESSAGE_ROLE_USER_SELF );
        const chat::ChatMessageState selfMsg =
            MakeMessage( kRoomA, "self-1", "from-self", kSender, 200, chat::MESSAGE_ROLE_USER_SELF );

        ASSERT_TRUE( session.Put( "gcs/messages/roomA/peer-1", peerMsg.SerializeAsString() ).has_value() );
        ASSERT_TRUE( session.Put( "gcs/messages/roomA/self-1", selfMsg.SerializeAsString() ).has_value() );

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); } );

        auto history = messaging.QueryHistory( kRoomA );
        ASSERT_TRUE( history.has_value() );
        ASSERT_EQ( history.value().message_size(), 2 );

        EXPECT_EQ( history.value().message( 0 ).id(), "peer-1" );
        EXPECT_EQ( history.value().message( 0 ).role(), chat::MESSAGE_ROLE_USER_PEER );
        EXPECT_EQ( history.value().message( 1 ).id(), "self-1" );
        EXPECT_EQ( history.value().message( 1 ).role(), chat::MESSAGE_ROLE_USER_SELF );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Test 8 (D-08): on the encrypted instance the sink sees plaintext
     *        while the live publish carries the envelope (nonce||ciphertext||tag),
     *        which decrypts back to the sent message.
     */
    TEST_F( GcsMessagingTest, EncryptedSendPushesPlaintextButPublishesEnvelope )
    {
        RawRecorder recorder;
        auto        pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        auto subscription = pubsub->Subscribe(
                                    kRoomA,
                                    [ &recorder ]( const SubscriptionData &data ) {
                                        if ( !data )
                                        {
                                            return;
                                        }
                                        const auto &msg = data.get();
                                        recorder.Record( msg.topic,
                                                         std::string( msg.data.begin(), msg.data.end() ) );
                                    } )
                                .get();
        ASSERT_NE( subscription, nullptr );

        gcs::Messaging::CryptoSeam seam;
        seam.encrypt = &gcs::crypto::EncryptPayload;
        seam.decrypt = &gcs::crypto::DecryptPayload;
        seam.enabled = true;

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); },
                                  seam );

        ASSERT_TRUE( messaging.SendMessage( kRoomA, "secret-hi" ).has_value() );

        // Sink still carries plaintext pending + complete with the same id.
        ASSERT_EQ( events.size(), 2U );
        EXPECT_EQ( events[ 0 ].message().text(), "secret-hi" );
        EXPECT_EQ( events[ 1 ].message().text(), "secret-hi" );
        EXPECT_EQ( events[ 0 ].message().id(), events[ 1 ].message().id() );

        // Live path carries the envelope, not the serialized plaintext.
        EXPECT_TRUE( WaitForCondition( [ &recorder ]() { return recorder.Count() >= 1; }, kWaitTimeout ) );
        const std::string recorded = recorder.At( 0 ).second;
        const std::string serialized = events[ 1 ].message().SerializeAsString();
        EXPECT_NE( recorded, serialized );
        EXPECT_EQ( recorded.size(),
                   serialized.size() + gcs::crypto::kGcmNonceLength + gcs::crypto::kGcmTagLength );

        // The envelope decrypts back to the sent ChatMessageState.
        auto decrypted = gcs::crypto::DecryptPayload( kRoomA, recorded );
        ASSERT_TRUE( decrypted.has_value() );
        chat::ChatMessageState sentMsg;
        ASSERT_TRUE( sentMsg.ParseFromString( decrypted.value() ) );
        EXPECT_EQ( sentMsg.text(), "secret-hi" );
        EXPECT_EQ( sentMsg.id(), events[ 1 ].message().id() );

        subscription->cancel();
        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Test 9 (D-08): the archive stores the envelope — every value under
     *        the room prefix is ciphertext+nonce, never the plaintext text.
     */
    TEST_F( GcsMessagingTest, EncryptedArchiveIsOpaqueAtRest )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        gcs::Messaging::CryptoSeam seam;
        seam.encrypt = &gcs::crypto::EncryptPayload;
        seam.decrypt = &gcs::crypto::DecryptPayload;
        seam.enabled = true;

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); },
                                  seam );

        ASSERT_TRUE( messaging.SendMessage( kRoomA, "secret-hi" ).has_value() );
        ASSERT_EQ( events.size(), 2U );

        const std::string serialized = events[ 1 ].message().SerializeAsString();
        auto scan = session.QueryKeyValues( kPrefixA );
        ASSERT_TRUE( scan.has_value() );
        ASSERT_EQ( scan.value().size(), 1U );

        const auto &value = scan.value().front().second;
        EXPECT_EQ( value.find( "secret-hi" ), std::string::npos );
        EXPECT_EQ( value.size(),
                   serialized.size() + gcs::crypto::kGcmNonceLength + gcs::crypto::kGcmTagLength );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Test 10 (D-08): wrong-key/garbage records are skipped-and-logged in
     *        BOTH funnels — the live funnel ignores a foreign-key envelope and
     *        the history scan skips a foreign-key record without hiding the rest.
     */
    TEST_F( GcsMessagingTest, WrongKeyRecordsAreSkippedInBothFunnels )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        gcs::Messaging::CryptoSeam seam;
        seam.encrypt = &gcs::crypto::EncryptPayload;
        seam.decrypt = &gcs::crypto::DecryptPayload;
        seam.enabled = true;

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); },
                                  seam );

        const chat::ChatMessageState peerMsg =
            MakeMessage( kRoomA, "m-1", "hello", kPeerSender, 1000 );
        const std::string serialized = peerMsg.SerializeAsString();

        // (a) Foreign-key envelope via the live funnel: sink unchanged, no archive key.
        auto foreign = gcs::crypto::EncryptPayload( kRoomB, serialized );
        ASSERT_TRUE( foreign.has_value() );
        messaging.OnLiveMessage( kRoomA, foreign.value() );
        EXPECT_TRUE( events.empty() );

        auto scanAfterLive = session.QueryKeyValues( kPrefixA );
        ASSERT_TRUE( scanAfterLive.has_value() );
        EXPECT_TRUE( scanAfterLive.value().empty() );

        // (b) Foreign-key record + one good record under roomA: history returns only the good one.
        ASSERT_TRUE( session.Put( "gcs/messages/roomA/bad-1", foreign.value() ).has_value() );
        const chat::ChatMessageState goodMsg =
            MakeMessage( kRoomA, "good-1", "good", kPeerSender, 2000 );
        auto good = gcs::crypto::EncryptPayload( kRoomA, goodMsg.SerializeAsString() );
        ASSERT_TRUE( good.has_value() );
        ASSERT_TRUE( session.Put( "gcs/messages/roomA/good-1", good.value() ).has_value() );

        auto history = messaging.QueryHistory( kRoomA );
        ASSERT_TRUE( history.has_value() );
        ASSERT_EQ( history.value().message_size(), 1 );
        EXPECT_EQ( history.value().message( 0 ).id(), "good-1" );
        EXPECT_EQ( history.value().message( 0 ).role(), chat::MESSAGE_ROLE_USER_PEER );

        session.Shutdown();
        pubsub->Stop();
    }

    /**
     * @brief Test 11 (D-08): with encryption disabled the injected functions are
     *        never called — send, live delivery, and history all flow as
     *        plaintext and the at-rest value is the serialized plaintext.
     */
    TEST_F( GcsMessagingTest, PlaintextPathNeverCallsTheInjectedSeam )
    {
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );
        ASSERT_TRUE( session.Initialize( pubsub, graphsync.network ).has_value() );

        int encryptCalls = 0;
        int decryptCalls = 0;

        gcs::Messaging::CryptoSeam seam;
        seam.encrypt = [ &encryptCalls ]( const std::string &,
                                          const std::string & ) -> libp2p::outcome::result<std::string> {
            ++encryptCalls;
            return libp2p::outcome::failure( sgns::gcs::Error::GcsDbError );
        };
        seam.decrypt = [ &decryptCalls ]( const std::string &,
                                          const std::string & ) -> libp2p::outcome::result<std::string> {
            ++decryptCalls;
            return libp2p::outcome::failure( sgns::gcs::Error::GcsDbError );
        };
        seam.enabled = false;

        std::vector<chat::GcsEvent> events;
        gcs::Messaging messaging( session, kSender,
                                  [ &events ]( const chat::GcsEvent &event ) { events.push_back( event ); },
                                  seam );

        // Send.
        ASSERT_TRUE( messaging.SendMessage( kRoomA, "plain" ).has_value() );

        // Live delivery (peer message).
        const chat::ChatMessageState peerMsg =
            MakeMessage( kRoomA, "m-1", "live-plain", kPeerSender, 1000 );
        messaging.OnLiveMessage( kRoomA, peerMsg.SerializeAsString() );

        // History scan — the live peer message's archive write is async, so
        // wait for both records to converge before asserting.
        EXPECT_TRUE( WaitForCondition(
            [ &messaging ]() {
                auto history = messaging.QueryHistory( kRoomA );
                return history.has_value() && history.value().message_size() == 2;
            },
            kWaitTimeout ) );

        auto history = messaging.QueryHistory( kRoomA );
        ASSERT_TRUE( history.has_value() );
        ASSERT_EQ( history.value().message_size(), 2 );

        // The seam was never invoked.
        EXPECT_EQ( encryptCalls, 0 );
        EXPECT_EQ( decryptCalls, 0 );

        // Delivery parsed (2 send events + 1 live event).
        ASSERT_EQ( events.size(), 3U );
        EXPECT_EQ( events[ 2 ].message().id(), "m-1" );
        EXPECT_EQ( events[ 2 ].message().text(), "live-plain" );

        // At-rest value for the sent message is the serialized plaintext.
        const std::string serialized = events[ 1 ].message().SerializeAsString();
        auto scan = session.QueryKeyValues( kPrefixA );
        ASSERT_TRUE( scan.has_value() );
        const auto sentValue = ValueForKeyFragment( scan.value(), events[ 0 ].message().id() );
        ASSERT_TRUE( sentValue.first );
        EXPECT_EQ( sentValue.second, serialized );

        session.Shutdown();
        pubsub->Stop();
    }

} // namespace gcs::test
