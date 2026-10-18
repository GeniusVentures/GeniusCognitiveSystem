/**
 * @file       gcs_membership.cpp
 * @brief      Implementation of gcs::Membership — the C++-owned member roster,
 *             five-tier role/permission enforcement, capability-token invite
 *             mint/redeem, and the D-07 dormant-until-approved model.
 * @details    Member records are per-key LWW values under
 *             gcs/members/<scopeId>/<wallet> (D-01), enumerated by the Phase 3
 *             QueryKeyValues prefix scan and skip-and-logging unparseable or
 *             tombstoned bytes. Invites are ONE InviteRecord under
 *             gcs/invites/<scopeId>/<inviteId> written at mint (D-05); the
 *             redeemer's wallet-keyed member record is written at redeem.
 *             Role/permission rules are write-time C++ guards (D-03/D-08); the
 *             destructive ops go dormant (empty approved_by) when 2+ admins
 *             exist (D-07). This file NEVER includes OpenSSL headers directly:
 *             key wrap/unwrap flows through the injected m_keyWrap callables
 *             (gcs::crypto in production).
 * @copyright  (c) 2026 GNUS.AI
 */

#include "gcs_membership.hpp"

#include <spdlog/spdlog.h>

#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <random>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "gcs_crypto.hpp"                  // D-06 wrap/unwrap + per-room key store
#include "gcs_entity_store.hpp"            // EntityStore Spaces()/Rooms() scope reads
#include "gcs_storage/common/error.hpp"    // sgns::gcs::Error (no NotFound code)

namespace gcs {
namespace {
/// Invite-id prefix (mirrors the CR-01-safe EntityStore id idiom).
constexpr const char *kInviteIdPrefix = "invite-";
/// CRDT key prefixes for the entity records the scope lookup scans.
constexpr const char *kSpacesKeyPrefix = "gcs/entities/spaces/";
constexpr const char *kRoomsKeyPrefix  = "gcs/entities/rooms/";
/// Room chat topic prefix — the Phase 1 gcs/chat/<id> convention (D-01).
constexpr const char *kRoomTopicPrefix = "gcs/chat/";
/// Invite URL fragments (D-05 literal shape).
constexpr const char *kInviteUrlScheme = "gcs://invite/";
constexpr const char *kInviteKeyParam  = "key=";
constexpr const char *kScopeKindSpace  = "space";
constexpr const char *kScopeKindRoom   = "room";

/**
 * @brief Broadcast topic set for a scope's records.
 *
 * Member and invite records replicate on the scope's room topic
 * (gcs/chat/<scopeId>) — the same D-02 per-room replication channel the
 * messaging archive uses. A space scope's id has no registered room topic
 * today, so its records land locally only.
 *
 * @param[in] scopeId The scope id.
 * @return The single-topic broadcast set for a Put.
 */
std::unordered_set<std::string> ScopeTopics( const std::string &scopeId )
{
    return std::unordered_set<std::string>{ std::string( kRoomTopicPrefix ) + scopeId };
}

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
 * @brief Privilege rank for a role — higher is more capable (D-08).
 *
 * @param[in] role The five-tier role.
 * @return ROLE_UNSPECIFIED/unknown -> 0, GUEST -> 1, MEMBER -> 2,
 *         MODERATOR -> 3, ADMIN -> 4, SUPER_ADMIN -> 5.
 */
unsigned int PrivilegeRank( chat::Role role )
{
    switch ( role )
    {
        case chat::ROLE_SUPER_ADMIN:
            return 5;
        case chat::ROLE_ADMIN:
            return 4;
        case chat::ROLE_MODERATOR:
            return 3;
        case chat::ROLE_MEMBER:
            return 2;
        case chat::ROLE_GUEST:
            return 1;
        default:
            return 0;
    }
}

/**
 * @brief Extract a '/'-delimited segment after a key marker.
 *
 * Handles both the logical key form (gcs/members/<scopeId>/<wallet>) and the
 * raw datastore form QueryKeyValues returns (/crdt/k/<key>/v), where the final
 * segment carries a trailing framing segment.
 *
 * @param[in] key     The key to parse.
 * @param[in] marker  The literal marker that precedes the first segment.
 * @param[in] segment Zero-based index of the wanted segment after the marker.
 * @return The segment; empty when the marker or segment is absent.
 */
std::string SegmentAfterMarker( const std::string &key, const char *marker,
                                std::size_t segment )
{
    const std::string markerStr( marker );
    const std::size_t markerPos = key.find( markerStr );
    if ( markerPos == std::string::npos )
    {
        return std::string();
    }
    std::size_t pos = markerPos + markerStr.size();
    for ( std::size_t i = 0; i < segment; ++i )
    {
        const std::size_t next = key.find( '/', pos );
        if ( next == std::string::npos )
        {
            return std::string();
        }
        pos = next + 1;
    }
    const std::size_t end = key.find( '/', pos );
    if ( end == std::string::npos )
    {
        return key.substr( pos );
    }
    return key.substr( pos, end - pos );
}

/// Base64url alphabet (RFC 4648 §5, no padding) — the D-05 URL key codec.
constexpr const char kBase64UrlAlphabet[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_";

/**
 * @brief Base64url-encode bytes without padding (RESEARCH Open Q5).
 *
 * @param[in] bytes The bytes to encode.
 * @return The URL-safe, pad-free encoding.
 */
std::string Base64UrlEncode( const std::vector<unsigned char> &bytes )
{
    std::string out;
    out.reserve( ( bytes.size() + 2 ) / 3 * 4 );
    std::size_t i = 0;
    while ( i + 3 <= bytes.size() )
    {
        const uint32_t triple = ( static_cast<uint32_t>( bytes[i] ) << 16 )
                                | ( static_cast<uint32_t>( bytes[i + 1] ) << 8 )
                                | static_cast<uint32_t>( bytes[i + 2] );
        out.push_back( kBase64UrlAlphabet[( triple >> 18 ) & 0x3F] );
        out.push_back( kBase64UrlAlphabet[( triple >> 12 ) & 0x3F] );
        out.push_back( kBase64UrlAlphabet[( triple >> 6 ) & 0x3F] );
        out.push_back( kBase64UrlAlphabet[triple & 0x3F] );
        i += 3;
    }
    const std::size_t remaining = bytes.size() - i;
    if ( remaining == 1 )
    {
        const uint32_t triple = static_cast<uint32_t>( bytes[i] ) << 16;
        out.push_back( kBase64UrlAlphabet[( triple >> 18 ) & 0x3F] );
        out.push_back( kBase64UrlAlphabet[( triple >> 12 ) & 0x3F] );
    }
    else if ( remaining == 2 )
    {
        const uint32_t triple = ( static_cast<uint32_t>( bytes[i] ) << 16 )
                                | ( static_cast<uint32_t>( bytes[i + 1] ) << 8 );
        out.push_back( kBase64UrlAlphabet[( triple >> 18 ) & 0x3F] );
        out.push_back( kBase64UrlAlphabet[( triple >> 12 ) & 0x3F] );
        out.push_back( kBase64UrlAlphabet[( triple >> 6 ) & 0x3F] );
    }
    return out;
}

/**
 * @brief Decode one base64url character.
 *
 * @param[in] c The character.
 * @return Its 6-bit value; -1 when the character is not in the alphabet.
 */
int Base64UrlValue( char c )
{
    if ( c >= 'A' && c <= 'Z' )
    {
        return c - 'A';
    }
    if ( c >= 'a' && c <= 'z' )
    {
        return c - 'a' + 26;
    }
    if ( c >= '0' && c <= '9' )
    {
        return c - '0' + 52;
    }
    if ( c == '-' )
    {
        return 62;
    }
    if ( c == '_' )
    {
        return 63;
    }
    return -1;
}

/**
 * @brief Decode a pad-free base64url string back to bytes.
 *
 * @param[in] text The encoded string.
 * @return outcome::success with the bytes; outcome::failure on an invalid
 *         character (the caller rejects the URL).
 */
outcome::result<std::vector<unsigned char>> Base64UrlDecode( const std::string &text )
{
    std::vector<unsigned char> out;
    out.reserve( text.size() * 3 / 4 );
    uint32_t buffer = 0;
    int      bits   = 0;
    for ( const char c : text )
    {
        const int value = Base64UrlValue( c );
        if ( value < 0 )
        {
            return outcome::failure( sgns::gcs::Error::GcsDbError );
        }
        buffer = ( buffer << 6 ) | static_cast<uint32_t>( value );
        bits += 6;
        if ( bits >= 8 )
        {
            bits -= 8;
            out.push_back( static_cast<unsigned char>( ( buffer >> bits ) & 0xFF ) );
        }
    }
    return out;
}
} // namespace

Membership::Membership( CoreSession &session, EntityStore &entities,
                        std::string senderAddress, EventSink sink,
                        KeyWrapSeam keyWrap )
    : m_session( session ),
      m_entities( entities ),
      m_sender( std::move( senderAddress ) ),
      m_sink( std::move( sink ) ),
      m_keyWrap( std::move( keyWrap ) )
{
}

std::string Membership::MemberKey( const std::string &scopeId,
                                   const std::string &address )
{
    return std::string( kMembersKeyPrefix ) + scopeId + "/" + address;
}

std::string Membership::InviteKey( const std::string &scopeId,
                                   const std::string &inviteId )
{
    return std::string( kInvitesKeyPrefix ) + scopeId + "/" + inviteId;
}

std::string Membership::RoomTopicFor( const std::string &roomId )
{
    return std::string( kRoomTopicPrefix ) + roomId;
}

std::string Membership::ScopeIdFromKey( const std::string &key )
{
    // The scope id is the FIRST segment after the member marker
    // (gcs/members/<scopeId>/<wallet>).
    return SegmentAfterMarker( key, kMembersKeyPrefix, 0 );
}

std::string Membership::NextInviteId()
{
    static const std::string seed = [] {
        const int64_t      nowMs = NowMs();
        std::random_device randomDevice;
        return std::to_string( nowMs ) + "-" + std::to_string( randomDevice() );
    }();
    static std::atomic<uint64_t> sequence{ 0 };
    return std::string( kInviteIdPrefix ) + seed + "-"
           + std::to_string( sequence.fetch_add( 1 ) );
}

bool Membership::KeyWrapEnabled() const
{
    return m_keyWrap.enabled && static_cast<bool>( m_keyWrap.wrap )
           && static_cast<bool>( m_keyWrap.unwrap );
}

Membership::ScopeInfo Membership::ResolveScope( const std::string &scopeId ) const
{
    ScopeInfo info;
    for ( const chat::SpaceRecord &space : m_entities.Spaces() )
    {
        if ( space.id() == scopeId )
        {
            info.found            = true;
            info.isSpace          = true;
            info.creator          = space.creator();
            info.membersCanInvite = space.members_can_invite();
            return info;
        }
    }
    for ( const chat::RoomRecord &room : m_entities.Rooms() )
    {
        if ( room.id() == scopeId )
        {
            info.found            = true;
            info.isSpace          = false;
            info.isStandaloneRoom = room.parent_space_id().empty();
            info.creator          = room.creator();
            info.membersCanInvite = room.members_can_invite();
            return info;
        }
    }
    return info;
}

bool Membership::TryReadMember( const std::string &scopeId,
                                const std::string &address,
                                chat::MemberRecord &out ) const
{
    const auto stored = m_session.Get( MemberKey( scopeId, address ) );
    if ( !stored.has_value() )
    {
        return false;
    }
    chat::MemberRecord record;
    if ( !record.ParseFromString( stored.value() ) )
    {
        spdlog::warn( "gcs_membership: member record for '{}' in scope '{}' is "
                      "unparseable",
                      address, scopeId );
        return false;
    }
    out = record;
    return true;
}

bool Membership::IsSuperAdmin( const std::string &scopeId,
                               const std::string &address ) const
{
    const ScopeInfo scope = ResolveScope( scopeId );
    return scope.found && !scope.creator.empty() && scope.creator == address;
}

unsigned int Membership::AdminCount( const std::string &scopeId ) const
{
    const std::string prefix = std::string( kMembersKeyPrefix ) + scopeId + "/";
    auto              scanResult = m_session.QueryKeyValues( prefix );
    if ( !scanResult.has_value() )
    {
        return 0;
    }
    unsigned int count = 0;
    for ( const auto &entry : scanResult.value() )
    {
        chat::MemberRecord record;
        if ( !record.ParseFromString( entry.second ) )
        {
            continue;
        }
        if ( record.deleted() )
        {
            continue;
        }
        if ( PrivilegeRank( record.role() ) >= PrivilegeRank( chat::ROLE_ADMIN ) )
        {
            ++count;
        }
    }
    return count;
}

outcome::result<void> Membership::WriteMember( const chat::MemberRecord &record )
{
    chat::MemberRecord updated = record;
    const auto         stored  = m_session.Get( MemberKey( record.scope_id(),
                                                           record.address() ) );
    if ( stored.has_value() )
    {
        chat::MemberRecord existing;
        if ( existing.ParseFromString( stored.value() ) )
        {
            updated.set_created_at_ms( existing.created_at_ms() );
        }
    }
    if ( updated.created_at_ms() == 0 )
    {
        updated.set_created_at_ms( NowMs() );
    }
    updated.set_updated_at_ms( NowMs() );
    // Replicate on the scope's room topic so remote nodes converge (D-01/D-04).
    return m_session.Put( MemberKey( updated.scope_id(), updated.address() ),
                          updated.SerializeAsString(),
                          ScopeTopics( updated.scope_id() ) );
}

outcome::result<void> Membership::PushMemberList( const std::string &scopeId )
{
    auto list = MembersFor( scopeId );
    if ( !list.has_value() )
    {
        return list.error();
    }
    if ( !m_sink )
    {
        return outcome::success();
    }
    chat::GcsEvent event;
    *event.mutable_member_list() = std::move( list.value() );
    m_sink( event );
    return outcome::success();
}

outcome::result<void> Membership::PushSpaceTree()
{
    if ( !m_sink )
    {
        return outcome::success();
    }
    chat::GcsEvent   event;
    chat::SpaceTree *tree = event.mutable_space_tree();
    for ( const chat::SpaceRecord &space : m_entities.Spaces() )
    {
        *tree->add_space() = space;
    }
    for ( const chat::RoomRecord &room : m_entities.Rooms() )
    {
        *tree->add_room() = room;
    }
    m_sink( event );
    return outcome::success();
}

outcome::result<chat::MemberList> Membership::MembersFor(
    const std::string &scopeId ) const
{
    const std::string prefix = std::string( kMembersKeyPrefix ) + scopeId + "/";
    auto              scanResult = m_session.QueryKeyValues( prefix );
    if ( !scanResult.has_value() )
    {
        return scanResult.error();
    }

    chat::MemberList list;
    list.set_scope_id( scopeId );
    // D-04: the pushed local wallet so Dart derives the "You" marker from
    // authority instead of guessing.
    list.set_self_address( m_sender );
    for ( const auto &entry : scanResult.value() )
    {
        chat::MemberRecord record;
        if ( !record.ParseFromString( entry.second ) )
        {
            spdlog::warn( "gcs_membership: skipping unparseable member record in "
                          "scope '{}'",
                          scopeId );
            continue;
        }
        if ( record.deleted() )
        {
            continue; // tombstone-skip, never key removal (D-03)
        }
        *list.add_member() = record;
    }
    return list;
}

chat::Role Membership::LocalRoleFor( const std::string &scopeId ) const
{
    chat::MemberRecord record;
    if ( TryReadMember( scopeId, m_sender, record ) && !record.deleted() )
    {
        return record.role();
    }
    return chat::ROLE_GUEST; // safest default (D-08 read-only)
}

bool Membership::HasExplicitLeave( const std::string &spaceId ) const
{
    chat::MemberRecord record;
    if ( TryReadMember( spaceId, m_sender, record ) && !record.deleted() )
    {
        return record.explicit_leave();
    }
    return false;
}

bool Membership::CanInvite( const std::string &scopeId ) const
{
    const chat::Role role = LocalRoleFor( scopeId );
    if ( role == chat::ROLE_SUPER_ADMIN || role == chat::ROLE_ADMIN )
    {
        return true;
    }
    if ( role == chat::ROLE_MEMBER )
    {
        const ScopeInfo scope = ResolveScope( scopeId );
        return scope.found && scope.membersCanInvite; // MEMB-05 gate
    }
    return false; // Moderator/Guest may not invite (D-08)
}

bool Membership::CanChangeRole( const std::string &scopeId,
                                const std::string &target,
                                chat::Role         role ) const
{
    const ScopeInfo scope = ResolveScope( scopeId );
    if ( !scope.found )
    {
        return false;
    }
    // D-03/MEMB-03: the scope creator (Super Admin) is never demotable. The
    // guard rejects any demotion targeting the creator — never CRDT semantics.
    if ( !scope.creator.empty() && target == scope.creator
         && role != chat::ROLE_SUPER_ADMIN )
    {
        return false;
    }
    const chat::Role author = LocalRoleFor( scopeId );
    if ( author == chat::ROLE_SUPER_ADMIN )
    {
        return true;
    }
    if ( author == chat::ROLE_ADMIN )
    {
        // An Admin may not grant the Super Admin tier nor touch the creator.
        return role != chat::ROLE_SUPER_ADMIN && target != scope.creator;
    }
    if ( author == chat::ROLE_MODERATOR )
    {
        // Moderator: member management only — no admin-tier grants.
        return role == chat::ROLE_MEMBER || role == chat::ROLE_GUEST;
    }
    return false; // Member/Guest may not change roles (D-08)
}

bool Membership::CanRemoveMember( const std::string &scopeId,
                                  const std::string &target ) const
{
    const ScopeInfo scope = ResolveScope( scopeId );
    if ( !scope.found )
    {
        return false;
    }
    if ( !scope.creator.empty() && target == scope.creator )
    {
        return false; // the Super Admin is never removable (D-03)
    }
    const chat::Role  author = LocalRoleFor( scopeId );
    chat::MemberRecord targetRecord;
    if ( !TryReadMember( scopeId, target, targetRecord )
         || targetRecord.deleted() )
    {
        return false;
    }
    // WR-02/D-08: a role governs which members its holder may act on — the
    // author must STRICTLY outrank the target (a Moderator cannot remove an
    // Admin, an Admin cannot remove a non-creator Super Admin).
    if ( PrivilegeRank( author ) <= PrivilegeRank( targetRecord.role() ) )
    {
        return false;
    }
    return author == chat::ROLE_SUPER_ADMIN || author == chat::ROLE_ADMIN
           || author == chat::ROLE_MODERATOR;
}

bool Membership::CanDelete( const std::string &scopeId ) const
{
    const chat::Role author = LocalRoleFor( scopeId );
    return author == chat::ROLE_SUPER_ADMIN || author == chat::ROLE_ADMIN;
}

bool Membership::CanApprove( const std::string &scopeId ) const
{
    return IsSuperAdmin( scopeId, m_sender );
}

outcome::result<std::string> Membership::MintInvite( const std::string &scopeId )
{
    const ScopeInfo scope = ResolveScope( scopeId );
    if ( !scope.found || ( !scope.isSpace && !scope.isStandaloneRoom ) )
    {
        spdlog::warn( "gcs_membership: mint_invite rejected — scope '{}' is not a "
                      "live space or standalone room",
                      scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    if ( !CanInvite( scopeId ) )
    {
        spdlog::warn( "gcs_membership: mint_invite rejected — sender lacks invite "
                      "permission in scope '{}'",
                      scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    const std::string          inviteId = NextInviteId();
    std::vector<unsigned char> inviteKey = crypto::MintRoomKey();
    if ( inviteKey.empty() )
    {
        spdlog::warn( "gcs_membership: mint_invite could not mint an invite key "
                      "for scope '{}'",
                      scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    // Role is C++-stamped (Guest default per D-05) — never client-supplied.
    chat::InviteRecord record;
    record.set_scope_id( scopeId );
    record.set_role( chat::ROLE_GUEST );

    if ( scope.isSpace )
    {
        // Wrap every non-tombstoned child room's key at mint (discretion
        // resolved: wrap all, not lazily).
        for ( const chat::RoomRecord &room : m_entities.Rooms() )
        {
            if ( room.parent_space_id() != scopeId )
            {
                continue;
            }
            const std::string topic = RoomTopicFor( room.id() );
            std::vector<unsigned char> roomKey;
            auto stored = crypto::RoomKeyFor( topic );
            if ( stored.has_value() )
            {
                roomKey = stored.value();
            }
            else
            {
                auto derived = crypto::DeriveRoomKey( topic );
                if ( !derived.has_value() )
                {
                    spdlog::warn( "gcs_membership: mint_invite could not resolve a "
                                  "key for room '{}'",
                                  room.id() );
                    continue;
                }
                roomKey = derived.value();
            }
            if ( !KeyWrapEnabled() )
            {
                continue;
            }
            auto wrapped = m_keyWrap.wrap( inviteKey, roomKey );
            if ( !wrapped.has_value() )
            {
                spdlog::warn( "gcs_membership: mint_invite could not wrap the key "
                              "for room '{}'",
                              room.id() );
                continue;
            }
            chat::WrappedRoomKey *entry = record.add_wrapped_room_key();
            entry->set_room_id( room.id() );
            entry->set_envelope( wrapped.value() );
        }
    }
    else
    {
        const std::string          topic = RoomTopicFor( scopeId );
        std::vector<unsigned char> roomKey;
        auto                       stored = crypto::RoomKeyFor( topic );
        if ( stored.has_value() )
        {
            roomKey = stored.value();
        }
        else
        {
            auto derived = crypto::DeriveRoomKey( topic ); // pre-membership room
            if ( derived.has_value() )
            {
                roomKey = derived.value();
            }
            else
            {
                spdlog::warn( "gcs_membership: mint_invite could not resolve a key "
                              "for standalone room '{}'",
                              scopeId );
            }
        }
        if ( !roomKey.empty() && KeyWrapEnabled() )
        {
            auto wrapped = m_keyWrap.wrap( inviteKey, roomKey );
            if ( wrapped.has_value() )
            {
                chat::WrappedRoomKey *entry = record.add_wrapped_room_key();
                entry->set_room_id( scopeId );
                entry->set_envelope( wrapped.value() );
            }
            else
            {
                spdlog::warn( "gcs_membership: mint_invite could not wrap the key "
                              "for standalone room '{}'",
                              scopeId );
            }
        }
    }

    // Replicate on the scope's room topic so a remote redeemer can find it.
    auto putResult = m_session.Put( InviteKey( scopeId, inviteId ),
                                    record.SerializeAsString(),
                                    ScopeTopics( scopeId ) );
    if ( !putResult.has_value() )
    {
        return putResult.error();
    }

    const std::string scopeKind = scope.isSpace ? kScopeKindSpace : kScopeKindRoom;
    return std::string( kInviteUrlScheme ) + scopeKind + "/" + inviteId + "?"
           + kInviteKeyParam + Base64UrlEncode( inviteKey );
}

outcome::result<void> Membership::RedeemInvite( const std::string &inviteUrl )
{
    const std::string scheme( kInviteUrlScheme );
    if ( inviteUrl.rfind( scheme, 0 ) != 0 )
    {
        spdlog::warn( "gcs_membership: redeem rejected — not a gcs://invite/ URL" );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    const std::size_t scopeKindEnd = inviteUrl.find( '/', scheme.size() );
    if ( scopeKindEnd == std::string::npos )
    {
        spdlog::warn( "gcs_membership: redeem rejected — missing scope kind" );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    const std::string scopeKind = inviteUrl.substr( scheme.size(),
                                                    scopeKindEnd - scheme.size() );
    if ( scopeKind != kScopeKindSpace && scopeKind != kScopeKindRoom )
    {
        spdlog::warn( "gcs_membership: redeem rejected — unknown scope kind '{}'",
                      scopeKind );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    const std::size_t queryPos = inviteUrl.find( '?', scopeKindEnd );
    if ( queryPos == std::string::npos )
    {
        spdlog::warn( "gcs_membership: redeem rejected — missing key query" );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    const std::string inviteId = inviteUrl.substr( scopeKindEnd + 1,
                                                   queryPos - scopeKindEnd - 1 );
    if ( inviteId.empty() )
    {
        spdlog::warn( "gcs_membership: redeem rejected — empty invite id" );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    const std::string query = inviteUrl.substr( queryPos + 1 );
    const std::string keyParam( kInviteKeyParam );
    if ( query.rfind( keyParam, 0 ) != 0 )
    {
        spdlog::warn( "gcs_membership: redeem rejected — missing key parameter" );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    auto decodedKey = Base64UrlDecode( query.substr( keyParam.size() ) );
    if ( !decodedKey.has_value() || decodedKey.value().empty() )
    {
        spdlog::warn( "gcs_membership: redeem rejected — invite key is not valid "
                      "base64url" );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    const std::vector<unsigned char> inviteKey = decodedKey.value();

    // Scan the invite records for the inviteId (the value carries the record).
    auto scanResult = m_session.QueryKeyValues( kInvitesKeyPrefix );
    if ( !scanResult.has_value() )
    {
        return scanResult.error();
    }
    chat::InviteRecord invite;
    bool               found = false;
    for ( const auto &entry : scanResult.value() )
    {
        if ( SegmentAfterMarker( entry.first, kInvitesKeyPrefix, 1 ) != inviteId )
        {
            continue;
        }
        chat::InviteRecord candidate;
        if ( !candidate.ParseFromString( entry.second ) )
        {
            spdlog::warn( "gcs_membership: redeem skipping unparseable invite "
                          "record for '{}'",
                          inviteId );
            continue;
        }
        invite = candidate;
        found  = true;
        break;
    }
    if ( !found )
    {
        spdlog::warn( "gcs_membership: redeem rejected — invite '{}' not found",
                      inviteId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    const std::string scopeId = invite.scope_id();
    if ( scopeId.empty() )
    {
        spdlog::warn( "gcs_membership: redeem rejected — invite '{}' carries no "
                      "scope",
                      inviteId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    // Unwrap each room key and store it locally (wrong key = skip-and-log,
    // never abort the whole redeem — T-04-03-T-2).
    for ( const chat::WrappedRoomKey &wrapped : invite.wrapped_room_key() )
    {
        if ( !KeyWrapEnabled() )
        {
            break;
        }
        auto unwrapped = m_keyWrap.unwrap( inviteKey, wrapped.envelope() );
        if ( !unwrapped.has_value() )
        {
            spdlog::warn( "gcs_membership: redeem skipping an undecryptable room "
                          "key for room '{}'",
                          wrapped.room_id() );
            continue;
        }
        crypto::StoreRoomKey( RoomTopicFor( wrapped.room_id() ),
                              unwrapped.value() );
    }

    // Write the redeemer's OWN wallet-keyed member record with the invite
    // record's C++-stamped role (never client-supplied, D-05). The invite
    // record is NOT consumed — N redeemers each write their own record.
    chat::Role role = invite.role();
    if ( role == chat::ROLE_UNSPECIFIED )
    {
        role = chat::ROLE_GUEST;
    }
    chat::MemberRecord mine;
    mine.set_scope_id( scopeId );
    mine.set_address( m_sender );
    mine.set_role( role );
    mine.set_deleted( false );
    mine.set_approved_by( "" );
    mine.set_pending_action( chat::PENDING_ACTION_UNSPECIFIED );
    mine.set_pending_role( chat::ROLE_UNSPECIFIED );
    for ( const chat::WrappedRoomKey &wrapped : invite.wrapped_room_key() )
    {
        *mine.add_wrapped_room_key() = wrapped;
    }
    auto writeResult = WriteMember( mine );
    if ( !writeResult.has_value() )
    {
        return writeResult;
    }
    return PushMemberList( scopeId );
}

outcome::result<void> Membership::ChangeRole( const std::string &scopeId,
                                              const std::string &target,
                                              chat::Role         role )
{
    if ( !CanChangeRole( scopeId, target, role ) )
    {
        spdlog::warn( "gcs_membership: change_role rejected — permission denied "
                      "for target '{}' in scope '{}'",
                      target, scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    chat::MemberRecord record;
    if ( !TryReadMember( scopeId, target, record ) || record.deleted() )
    {
        spdlog::warn( "gcs_membership: change_role rejected — member '{}' not "
                      "found in scope '{}'",
                      target, scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    // D-07/WR-03: demoting any admin-tier-or-above target (Admin or Super
    // Admin) to a lower rank is destructive; with 2+ admins it goes dormant
    // (empty approved_by). A Member/Guest demotion stays non-destructive.
    const bool isAdminDemotion =
        PrivilegeRank( record.role() ) >= PrivilegeRank( chat::ROLE_ADMIN )
        && PrivilegeRank( role ) < PrivilegeRank( record.role() );
    if ( isAdminDemotion && AdminCount( scopeId ) > 1 )
    {
        record.set_approved_by( "" );
        record.set_pending_action( chat::PENDING_ACTION_ROLE_CHANGE );
        record.set_pending_role( role );
    }
    else
    {
        record.set_role( role );
        record.set_approved_by( "" );
        record.set_pending_action( chat::PENDING_ACTION_UNSPECIFIED );
        record.set_pending_role( chat::ROLE_UNSPECIFIED );
    }
    auto writeResult = WriteMember( record );
    if ( !writeResult.has_value() )
    {
        return writeResult;
    }
    return PushMemberList( scopeId );
}

outcome::result<void> Membership::RemoveMember( const std::string &scopeId,
                                                const std::string &target )
{
    if ( !CanRemoveMember( scopeId, target ) )
    {
        spdlog::warn( "gcs_membership: remove_member rejected — permission denied "
                      "for target '{}' in scope '{}'",
                      target, scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    chat::MemberRecord record;
    if ( !TryReadMember( scopeId, target, record ) || record.deleted() )
    {
        spdlog::warn( "gcs_membership: remove_member rejected — member '{}' not "
                      "found in scope '{}'",
                      target, scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    // D-07: dormant when 2+ admins (not tombstoned yet), sole-admin acts alone.
    if ( AdminCount( scopeId ) > 1 )
    {
        record.set_approved_by( "" );
        record.set_pending_action( chat::PENDING_ACTION_REMOVE );
    }
    else
    {
        record.set_deleted( true );
    }
    auto writeResult = WriteMember( record );
    if ( !writeResult.has_value() )
    {
        return writeResult;
    }
    return PushMemberList( scopeId );
}

outcome::result<void> Membership::Leave( const std::string &scopeId )
{
    if ( IsSuperAdmin( scopeId, m_sender ) )
    {
        spdlog::warn( "gcs_membership: leave rejected — the Super Admin cannot "
                      "leave scope '{}'",
                      scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    chat::MemberRecord record;
    if ( !TryReadMember( scopeId, m_sender, record ) || record.deleted() )
    {
        spdlog::warn( "gcs_membership: leave rejected — no live member record in "
                      "scope '{}'",
                      scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    // D-02: explicit leave is a bool on the caller's own record — never a
    // tombstone, never a key removal.
    record.set_explicit_leave( true );
    auto writeResult = WriteMember( record );
    if ( !writeResult.has_value() )
    {
        return writeResult;
    }
    return PushMemberList( scopeId );
}

outcome::result<void> Membership::DeleteScope( const std::string &scopeId )
{
    if ( !CanDelete( scopeId ) )
    {
        spdlog::warn( "gcs_membership: delete_scope rejected — permission denied "
                      "in scope '{}'",
                      scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    const bool needsApproval = AdminCount( scopeId ) > 1;

    for ( const chat::SpaceRecord &space : m_entities.Spaces() )
    {
        if ( space.id() != scopeId )
        {
            continue;
        }
        chat::SpaceRecord desired = space;
        if ( needsApproval )
        {
            desired.set_approved_by( "" );
            desired.set_pending_action( chat::PENDING_ACTION_DELETE );
        }
        else
        {
            desired.set_deleted( true );
            desired.set_deleted_at_ms( NowMs() );
        }
        desired.set_updated_at_ms( NowMs() );
        auto putResult = m_session.Put( std::string( kSpacesKeyPrefix ) + scopeId,
                                        desired.SerializeAsString() );
        if ( !putResult.has_value() )
        {
            return putResult.error();
        }
        return PushSpaceTree();
    }
    for ( const chat::RoomRecord &room : m_entities.Rooms() )
    {
        if ( room.id() != scopeId )
        {
            continue;
        }
        chat::RoomRecord desired = room;
        if ( needsApproval )
        {
            desired.set_approved_by( "" );
            desired.set_pending_action( chat::PENDING_ACTION_DELETE );
        }
        else
        {
            desired.set_deleted( true );
            desired.set_deleted_at_ms( NowMs() );
        }
        desired.set_updated_at_ms( NowMs() );
        auto putResult = m_session.Put( std::string( kRoomsKeyPrefix ) + scopeId,
                                        desired.SerializeAsString() );
        if ( !putResult.has_value() )
        {
            return putResult.error();
        }
        return PushSpaceTree();
    }
    spdlog::warn( "gcs_membership: delete_scope rejected — scope '{}' not found",
                  scopeId );
    return outcome::failure( sgns::gcs::Error::GcsDbError );
}

outcome::result<void> Membership::Approve( const std::string &scopeId,
                                           const std::string &target )
{
    if ( !CanApprove( scopeId ) )
    {
        spdlog::warn( "gcs_membership: approve rejected — sender is not the Super "
                      "Admin of scope '{}'",
                      scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    // Member-level dormant op (REMOVE / ROLE_CHANGE) — the target is a member
    // address, and activation overwrites approved_by on the SAME key.
    if ( target != scopeId )
    {
        chat::MemberRecord record;
        if ( TryReadMember( scopeId, target, record ) && !record.deleted()
             && record.approved_by().empty() )
        {
            if ( record.pending_action() == chat::PENDING_ACTION_REMOVE )
            {
                record.set_approved_by( m_sender );
                record.set_deleted( true );
                auto writeResult = WriteMember( record );
                if ( !writeResult.has_value() )
                {
                    return writeResult;
                }
                return PushMemberList( scopeId );
            }
            if ( record.pending_action() == chat::PENDING_ACTION_ROLE_CHANGE )
            {
                record.set_approved_by( m_sender );
                record.set_role( record.pending_role() );
                record.set_pending_action( chat::PENDING_ACTION_UNSPECIFIED );
                record.set_pending_role( chat::ROLE_UNSPECIFIED );
                auto writeResult = WriteMember( record );
                if ( !writeResult.has_value() )
                {
                    return writeResult;
                }
                return PushMemberList( scopeId );
            }
        }
    }

    // Entity-level dormant DELETE — the target is the scope id itself.
    if ( target == scopeId )
    {
        for ( const chat::SpaceRecord &space : m_entities.Spaces() )
        {
            if ( space.id() != scopeId )
            {
                continue;
            }
            if ( !space.approved_by().empty()
                 || space.pending_action() != chat::PENDING_ACTION_DELETE )
            {
                break;
            }
            chat::SpaceRecord approved = space;
            approved.set_approved_by( m_sender );
            approved.set_deleted( true );
            approved.set_deleted_at_ms( NowMs() );
            approved.set_updated_at_ms( NowMs() );
            auto putResult = m_session.Put(
                std::string( kSpacesKeyPrefix ) + scopeId,
                approved.SerializeAsString() );
            if ( !putResult.has_value() )
            {
                return putResult.error();
            }
            return PushSpaceTree();
        }
        for ( const chat::RoomRecord &room : m_entities.Rooms() )
        {
            if ( room.id() != scopeId )
            {
                continue;
            }
            if ( !room.approved_by().empty()
                 || room.pending_action() != chat::PENDING_ACTION_DELETE )
            {
                break;
            }
            chat::RoomRecord approved = room;
            approved.set_approved_by( m_sender );
            approved.set_deleted( true );
            approved.set_deleted_at_ms( NowMs() );
            approved.set_updated_at_ms( NowMs() );
            auto putResult = m_session.Put(
                std::string( kRoomsKeyPrefix ) + scopeId,
                approved.SerializeAsString() );
            if ( !putResult.has_value() )
            {
                return putResult.error();
            }
            return PushSpaceTree();
        }
    }

    spdlog::warn( "gcs_membership: approve rejected — no dormant op for target "
                  "'{}' in scope '{}'",
                  target, scopeId );
    return outcome::failure( sgns::gcs::Error::GcsDbError );
}

void Membership::OnMemberArrived( const std::string &key,
                                  const std::string &value )
{
    const std::string scopeId = ScopeIdFromKey( key );
    if ( scopeId.empty() )
    {
        return;
    }
    chat::MemberRecord record;
    if ( !record.ParseFromString( value ) )
    {
        spdlog::warn( "gcs_membership: skipping unparseable member record for key "
                      "'{}'",
                      key );
        return;
    }
    if ( record.deleted() )
    {
        return;
    }
    const auto pushResult = PushMemberList( scopeId );
    if ( !pushResult.has_value() )
    {
        spdlog::warn( "gcs_membership: member-list push failed for scope '{}'",
                      scopeId );
    }
}

outcome::result<void> Membership::OnScopeCreated( const std::string &scopeId,
                                                  const std::string &roomTopic,
                                                  bool               isSpace )
{
    chat::MemberRecord creator;
    creator.set_scope_id( scopeId );
    creator.set_address( m_sender );
    creator.set_role( chat::ROLE_SUPER_ADMIN ); // D-03: creator is the Super Admin
    creator.set_deleted( false );
    creator.set_created_at_ms( NowMs() );
    creator.set_approved_by( "" );
    creator.set_pending_action( chat::PENDING_ACTION_UNSPECIFIED );
    creator.set_pending_role( chat::ROLE_UNSPECIFIED );

    if ( !isSpace )
    {
        // Standalone room: mint the room key, store it, and carry it wrapped
        // under a freshly minted creator member key for recovery (D-06).
        std::vector<unsigned char> roomKey = crypto::MintRoomKey();
        if ( roomKey.empty() )
        {
            spdlog::warn( "gcs_membership: on_scope_created could not mint a room "
                          "key for '{}'",
                          scopeId );
            return outcome::failure( sgns::gcs::Error::GcsDbError );
        }
        crypto::StoreRoomKey( roomTopic, roomKey );

        std::vector<unsigned char> memberKey = crypto::MintRoomKey();
        if ( memberKey.empty() )
        {
            spdlog::warn( "gcs_membership: on_scope_created could not mint a "
                          "creator member key for '{}'",
                          scopeId );
            return outcome::failure( sgns::gcs::Error::GcsDbError );
        }
        if ( KeyWrapEnabled() )
        {
            auto wrapped = m_keyWrap.wrap( memberKey, roomKey );
            if ( wrapped.has_value() )
            {
                chat::WrappedRoomKey *entry = creator.add_wrapped_room_key();
                entry->set_room_id( scopeId );
                entry->set_envelope( wrapped.value() );
            }
            else
            {
                spdlog::warn( "gcs_membership: on_scope_created could not wrap the "
                              "room key for '{}'",
                              scopeId );
            }
        }
    }

    auto writeResult = WriteMember( creator );
    if ( !writeResult.has_value() )
    {
        return writeResult;
    }
    return PushMemberList( scopeId );
}

} // namespace gcs
