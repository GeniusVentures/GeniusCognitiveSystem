/**
 * @file       gcs_messaging.cpp
 * @brief      Implementation of gcs::Messaging — send/receive/history over the
 *             CoreSession CRDT + raw GossipSub surface with the D-08 crypto seam.
 * @details    This file NEVER includes OpenSSL headers directly: encryption is
 *             reached only through the injected m_crypto.encrypt /
 *             m_crypto.decrypt callables (gcs::crypto in production), and only
 *             when m_encryptionEnabled.
 *             When the seam is disabled the callables are simply not called and
 *             payloads flow as plaintext. The receive funnel (ApplyMessage)
 *             decrypts before parse/dedupe; QueryHistory decrypts before
 *             parse/sort; both skip-and-log undecryptable/unparseable records.
 * @copyright  (c) 2026 GNUS.AI
 */

#include "gcs_messaging.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <random>
#include <utility>

namespace gcs {
namespace {
/// Message-id prefix (mirrors the FFI NextMessageId authority stamp, CR-01).
constexpr const char *kMessageIdPrefix = "msg-";
/// Upper bound on the apply-once dedupe set (T-03-09 — bounded, clear-on-full).
constexpr size_t kMaxSeenIds = 4096;
/// Raw datastore value/priority suffixes (QueryKeyValues key framing).
constexpr const char kValueSuffix[]    = "/v";
constexpr const char kPrioritySuffix[] = "/p";

/**
 * @brief Wall-clock milliseconds since the epoch (same stamp as the FFI path).
 *
 * @return Millisecond count.
 */
int64_t NowMs()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
               std::chrono::system_clock::now().time_since_epoch() )
        .count();
}

/**
 * @brief Whether @p value ends with the NUL-terminated @p suffix.
 */
bool EndsWith( const std::string &value, const char *suffix )
{
    const std::size_t suffixLen = std::char_traits<char>::length( suffix );
    return value.size() >= suffixLen
           && value.compare( value.size() - suffixLen, suffixLen, suffix ) == 0;
}
} // namespace

Messaging::Messaging( CoreSession &session, std::string senderAddress,
                      EventSink sink, CryptoSeam crypto )
    : m_session( session ),
      m_sender( std::move( senderAddress ) ),
      m_sink( std::move( sink ) ),
      m_crypto( std::move( crypto ) ),
      m_encryptionEnabled( m_crypto.enabled
                           && static_cast<bool>( m_crypto.encrypt )
                           && static_cast<bool>( m_crypto.decrypt ) )
{
    m_archiveThread = std::thread( &Messaging::ArchiveWorker, this );
}

Messaging::~Messaging()
{
    {
        std::lock_guard<std::mutex> lock( m_archiveMutex );
        m_archiveStopped = true;
    }
    m_archiveCond.notify_all();
    if ( m_archiveThread.joinable() )
    {
        m_archiveThread.join();
    }
}

void Messaging::EnqueueArchive( const std::string &key, const std::string &value )
{
    {
        std::lock_guard<std::mutex> lock( m_archiveMutex );
        m_archiveQueue.emplace( key, value );
    }
    m_archiveCond.notify_one();
}

void Messaging::ArchiveWorker()
{
    for ( ;; )
    {
        std::pair<std::string, std::string> item;
        {
            std::unique_lock<std::mutex> lock( m_archiveMutex );
            m_archiveCond.wait( lock,
                                [ this ]() { return m_archiveStopped || !m_archiveQueue.empty(); } );
            if ( m_archiveStopped && m_archiveQueue.empty() )
            {
                return;
            }
            item = std::move( m_archiveQueue.front() );
            m_archiveQueue.pop();
        }

        const auto putResult = m_session.Put( item.first, item.second );
        if ( !putResult.has_value() )
        {
            spdlog::warn( "gcs_messaging: failed to archive received message under key '{}'",
                          item.first );
        }
    }
}

bool Messaging::EncryptionEnabled() const
{
    return m_encryptionEnabled;
}

std::string Messaging::NextMessageId()
{
    static const std::string seed = [] {
        const int64_t      nowMs        = NowMs();
        std::random_device randomDevice;
        return std::to_string( nowMs ) + "-" + std::to_string( randomDevice() );
    }();
    static std::atomic<uint64_t> sequence{ 0 };
    return std::string( kMessageIdPrefix ) + seed + "-"
           + std::to_string( sequence.fetch_add( 1 ) );
}

std::string Messaging::BuildKey( const std::string &roomTopic, const std::string &id )
{
    return std::string( kMessagesKeyPrefix ) + roomTopic + "/" + id;
}

std::string Messaging::RoomTopicFromKey( const std::string &key )
{
    // Normalize raw datastore keys (/<ns>/k/gcs/messages/<topic>/<id>/v) to the
    // logical form: the logical key always begins with kMessagesKeyPrefix, so
    // locate it and drop any leading raw framing / leading '/'.
    const auto prefixPos = key.find( kMessagesKeyPrefix );
    if ( prefixPos == std::string::npos )
    {
        return std::string{}; // not a message archive key
    }
    std::string logical = key.substr( prefixPos );

    // Drop a trailing /v or /p raw suffix if present.
    if ( EndsWith( logical, kValueSuffix ) || EndsWith( logical, kPrioritySuffix ) )
    {
        logical.resize( logical.size() - std::char_traits<char>::length( kValueSuffix ) );
    }

    // Strip the gcs/messages/ prefix, then the trailing "/<id>" segment — room
    // topics may themselves contain '/', so only the last segment is the id.
    logical = logical.substr( std::char_traits<char>::length( kMessagesKeyPrefix ) );

    const auto lastSlash = logical.rfind( '/' );
    if ( lastSlash != std::string::npos )
    {
        logical = logical.substr( 0, lastSlash );
    }
    return logical;
}

outcome::result<void> Messaging::SendMessage( const std::string &roomTopic,
                                              const std::string &text )
{
    chat::ChatMessageState msg;
    const std::string      id = NextMessageId();
    msg.set_id( id );
    msg.set_room_topic( roomTopic );
    msg.set_role( chat::MESSAGE_ROLE_USER_SELF );
    msg.set_state( chat::MESSAGE_STATE_PENDING );
    msg.set_text( text );
    msg.set_timestamp( NowMs() );
    msg.set_sender( m_sender );

    {
        std::lock_guard<std::mutex> lock( m_seenMutex );
        m_seenIds.insert( id ); // absorb our own live/CRDT echo (apply-once)
    }

    PushMessage( msg ); // pending plaintext echo (D-07)

    msg.set_state( chat::MESSAGE_STATE_COMPLETE );
    const std::string serialized = msg.SerializeAsString();

    // D-07 terminal failure path: push an error-state event for this id and
    // propagate the underlying error.
    const auto fail = [ & ]( const std::error_code &ec ) -> outcome::result<void> {
        chat::ChatMessageState errorMsg = msg;
        errorMsg.set_state( chat::MESSAGE_STATE_ERROR );
        PushMessage( errorMsg );
        return outcome::failure( ec );
    };

    // Compute the payload ONCE: envelope when enabled, serialized record when not
    // (D-08 — the SAME bytes go to BOTH the live publish and the archive Put).
    std::string payload;
    if ( m_encryptionEnabled )
    {
        auto encrypted = m_crypto.encrypt( roomTopic, serialized );
        if ( !encrypted.has_value() )
        {
            return fail( encrypted.error() );
        }
        payload = std::move( encrypted.value() );
    }
    else
    {
        payload = serialized;
    }

    // (a) LIVE: raw full-value GossipSub publish (D-03 fast path — never a CID).
    const auto publishResult = m_session.Publish( roomTopic, payload );
    if ( !publishResult.has_value() )
    {
        return fail( publishResult.error() );
    }

    // (b) ARCHIVE: topics-aware Put — the CRDT archive/heal write receiving the
    // SAME envelope (D-02/D-08; Put-with-topics replicates the archive via the
    // CRDT CID broadcast, while the live delivery is the explicit Publish above).
    const auto putResult = m_session.Put( BuildKey( roomTopic, id ), payload, { roomTopic } );
    if ( !putResult.has_value() )
    {
        return fail( putResult.error() );
    }

    PushMessage( msg ); // complete plaintext echo
    return outcome::success();
}

void Messaging::OnLiveMessage( const std::string &topic, const std::string &valueBytes )
{
    ApplyMessage( topic, valueBytes );
}

void Messaging::OnMessageArrived( const std::string &key, const std::string &valueBytes )
{
    const std::string roomTopic = RoomTopicFromKey( key );
    if ( roomTopic.empty() )
    {
        spdlog::warn( "gcs_messaging: OnMessageArrived with non-message key '{}'", key );
        return;
    }
    ApplyMessage( roomTopic, valueBytes );
}

void Messaging::ApplyMessage( const std::string &roomTopic, const std::string &valueBytes )
{
    // DECRYPT FIRST when enabled (D-08): wrong key / tampered / garbage -> skip-and-log.
    std::string plain;
    if ( m_encryptionEnabled )
    {
        auto decrypted = m_crypto.decrypt( roomTopic, valueBytes );
        if ( !decrypted.has_value() )
        {
            spdlog::warn( "gcs_messaging: skipping undecryptable message in room '{}'",
                          roomTopic );
            return;
        }
        plain = std::move( decrypted.value() );
    }
    else
    {
        plain = valueBytes;
    }

    chat::ChatMessageState msg;
    if ( !msg.ParseFromString( plain ) )
    {
        spdlog::warn( "gcs_messaging: skipping unparseable message in room '{}'", roomTopic );
        return;
    }

    {
        std::lock_guard<std::mutex> lock( m_seenMutex );
        if ( m_seenIds.find( msg.id() ) != m_seenIds.end() )
        {
            return; // apply-once — every message arrives twice (live + CRDT heal)
        }

        if ( m_seenIds.size() >= kMaxSeenIds )
        {
            m_seenIds.clear(); // bounded policy (T-03-09)
        }
        m_seenIds.insert( msg.id() );
    }

    PushMessage( msg ); // role flipped to PEER when sender != m_sender

    // Archive the envelope AS RECEIVED (never the decrypted plaintext) with an
    // EMPTY topic set — a local-only write that does not re-broadcast (D-03
    // echo-loop guard, D-08 ciphertext at rest). The write is handed to the
    // dedicated archive worker so it never blocks the GossipSub/CRDT callback
    // thread (the strand deadlock fix).
    EnqueueArchive( BuildKey( msg.room_topic(), msg.id() ), valueBytes );
}

outcome::result<chat::MessageHistory> Messaging::QueryHistory( const std::string &roomTopic )
{
    const std::string prefix = std::string( kMessagesKeyPrefix ) + roomTopic + "/";
    auto              scanResult = m_session.QueryKeyValues( prefix );
    if ( !scanResult.has_value() )
    {
        return scanResult.error();
    }

    std::vector<chat::ChatMessageState> messages;
    for ( const auto &entry : scanResult.value() )
    {
        // entry.first is a raw datastore key (/<ns>/k/.../v); the value carries
        // the full record, so the key is not needed here.
        const std::string &value = entry.second;

        std::string plain;
        if ( m_encryptionEnabled )
        {
            auto decrypted = m_crypto.decrypt( roomTopic, value );
            if ( !decrypted.has_value() )
            {
                spdlog::warn( "gcs_messaging: skipping undecryptable history record in "
                              "room '{}'",
                              roomTopic );
                continue;
            }
            plain = std::move( decrypted.value() );
        }
        else
        {
            plain = value;
        }

        chat::ChatMessageState msg;
        if ( !msg.ParseFromString( plain ) )
        {
            spdlog::warn( "gcs_messaging: skipping unparseable history record in room '{}'",
                          roomTopic );
            continue;
        }
        msg.set_role( RoleFor( msg, m_sender ) ); // self/peer flip for the batch
        messages.push_back( std::move( msg ) );
    }

    std::sort( messages.begin(), messages.end(),
               []( const chat::ChatMessageState &a, const chat::ChatMessageState &b ) {
                   if ( a.timestamp() != b.timestamp() )
                   {
                       return a.timestamp() < b.timestamp();
                   }
                   return a.id() < b.id(); // tie-break makes the order total (D-01)
               } );

    chat::MessageHistory history;
    history.set_room_topic( roomTopic );
    for ( const auto &msg : messages )
    {
        *history.add_message() = msg;
    }

    // Absorb every scanned id so a live/CRDT message landing mid-join is deduped.
    {
        std::lock_guard<std::mutex> lock( m_seenMutex );
        for ( const auto &msg : messages )
        {
            if ( m_seenIds.size() >= kMaxSeenIds )
            {
                m_seenIds.clear();
            }
            m_seenIds.insert( msg.id() );
        }
    }

    return history;
}

void Messaging::PushMessage( const chat::ChatMessageState &msg )
{
    chat::GcsEvent           event;
    chat::ChatMessageState *message = event.mutable_message();
    *message = msg;
    message->set_role( RoleFor( msg, m_sender ) );
    m_sink( event );
}

chat::MessageRole Messaging::RoleFor( const chat::ChatMessageState &msg,
                                      const std::string &sender )
{
    return msg.sender() == sender ? chat::MESSAGE_ROLE_USER_SELF
                                  : chat::MESSAGE_ROLE_USER_PEER;
}

} // namespace gcs
