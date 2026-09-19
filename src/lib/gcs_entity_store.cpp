/**
 * @file       gcs_entity_store.cpp
 * @brief      Implementation of gcs::EntityStore — the C++-owned catalog of
 * spaces and rooms persisted as CRDT records in GlobalDB (D-01/D-02/D-03/D-04).
 * @details    EntityStore writes one serialized record proto per entity key
 * (gcs/entities/spaces/<id>, gcs/entities/rooms/<id>) and maintains the
 * gcs/index/manifest id list via a read-union-write convention (append-only
 * membership — GlobalDB has no enumeration, so the manifest is the index).
 * All storage access goes through the CoreSession Put/Get pass-throughs; the
 * store owns no I/O of its own. Deletion is tombstone-based (D-03): readers
 * skip tombstoned records, no Phase 2 write ever sets the tombstone fields.
 * @copyright  (c) 2026 GNUS.AI
 */

#include "gcs_entity_store.hpp"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <random>

namespace gcs {
namespace
{
    // CRDT key prefix for space records — one serialized SpaceRecord per key.
    constexpr const char *kSpacesKeyPrefix = "gcs/entities/spaces/";
    // CRDT key prefix for room records — one serialized RoomRecord per key.
    constexpr const char *kRoomsKeyPrefix = "gcs/entities/rooms/";
    // CRDT key holding the serialized EntityManifest (the catalog index —
    // GlobalDB offers no enumeration/prefix query, D-02).
    constexpr const char *kManifestKey = "gcs/index/manifest";
    // Opaque-id prefixes (D-01) — self-describing in keys and logs.
    constexpr const char *kSpaceIdPrefix = "space-";
    constexpr const char *kRoomIdPrefix  = "room-";
    // Room chat topic prefix — the Phase 1 gcs/chat/<...> convention (D-01).
    constexpr const char *kRoomTopicPrefix = "gcs/chat/";

    /**
     * @brief Wall-clock milliseconds since the epoch.
     *
     * Same stamp as the FFI message stamp (gcs_core_ffi.cpp NextMessageId).
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
     * @brief Appends an entity id to the manifest (read-union-write, D-02).
     *
     * Reads the manifest key; a failed Get means the key is absent (empty
     * manifest — GcsGlobalDb::Get conflates missing with failed) and
     * unparseable bytes are treated the same: the union write heals the key on
     * the Put back. Idempotent — an id already listed is not appended twice.
     *
     * @param[in] session   Session to read/write through.
     * @param[in] isSpaceId true for the space_id list, false for room_id.
     * @param[in] id        Entity id to append.
     * @return outcome::success on a persisted manifest; propagated Error from
     *         the manifest Put otherwise.
     */
    outcome::result<void> AppendToManifest( CoreSession &session, bool isSpaceId,
                                            const std::string &id )
    {
        gcs::chat::EntityManifest manifest;
        auto                     getResult = session.Get( kManifestKey );
        if ( getResult.has_value() )
        {
            if ( !manifest.ParseFromString( getResult.value() ) )
            {
                spdlog::debug( "gcs_entity_store: manifest bytes unparseable — "
                               "treating as empty (appending '{}')",
                               id );
                manifest.Clear();
            }
        }
        else
        {
            spdlog::debug( "gcs_entity_store: manifest key absent — starting an "
                           "empty manifest (appending '{}')",
                           id );
        }
        const auto &ids = isSpaceId ? manifest.space_id() : manifest.room_id();
        if ( std::find( ids.begin(), ids.end(), id ) == ids.end() )
        {
            if ( isSpaceId )
            {
                manifest.add_space_id( id );
            }
            else
            {
                manifest.add_room_id( id );
            }
        }
        return session.Put( kManifestKey, manifest.SerializeAsString() );
    }
} // namespace

EntityStore::EntityStore( CoreSession &session )
    : m_session( session )
{
}

std::string EntityStore::NextEntityId( const char *prefix )
{
    static const std::string seed = [] {
        const int64_t      nowMs        = NowMs();
        std::random_device randomDevice;
        return std::to_string( nowMs ) + "-" + std::to_string( randomDevice() );
    }();
    static std::atomic<uint64_t> sequence{ 0 };
    return std::string( prefix ) + seed + "-" + std::to_string( sequence.fetch_add( 1 ) );
}

outcome::result<void> EntityStore::LoadFromStore()
{
    m_spaces.clear();
    m_rooms.clear();

    auto manifestBytes = m_session.Get( kManifestKey );
    if ( !manifestBytes.has_value() )
    {
        // Missing manifest key = empty catalog (GcsGlobalDb::Get conflates
        // missing with failed — never an error on first launch, Pitfall 2).
        spdlog::debug( "gcs_entity_store: manifest key absent — empty catalog" );
        return outcome::success();
    }
    gcs::chat::EntityManifest manifest;
    if ( !manifest.ParseFromString( manifestBytes.value() ) )
    {
        spdlog::warn( "gcs_entity_store: manifest bytes unparseable — treating "
                      "as an empty catalog" );
        return outcome::success();
    }

    for ( const std::string &spaceId : manifest.space_id() )
    {
        auto recordBytes = m_session.Get( std::string( kSpacesKeyPrefix ) + spaceId );
        if ( !recordBytes.has_value() )
        {
            spdlog::warn( "gcs_entity_store: skipping space '{}' — record read "
                          "failed",
                          spaceId );
            continue;
        }
        gcs::chat::SpaceRecord record;
        if ( !record.ParseFromString( recordBytes.value() ) )
        {
            spdlog::warn( "gcs_entity_store: skipping space '{}' — record bytes "
                          "unparseable",
                          spaceId );
            continue;
        }
        // Tombstoned records stay in the map (hidden by Spaces()) so
        // IsValidParentSpace() keeps returning false for them (D-03).
        m_spaces.emplace( record.id(), record );
    }

    for ( const std::string &roomId : manifest.room_id() )
    {
        auto recordBytes = m_session.Get( std::string( kRoomsKeyPrefix ) + roomId );
        if ( !recordBytes.has_value() )
        {
            spdlog::warn( "gcs_entity_store: skipping room '{}' — record read "
                          "failed",
                          roomId );
            continue;
        }
        gcs::chat::RoomRecord record;
        if ( !record.ParseFromString( recordBytes.value() ) )
        {
            spdlog::warn( "gcs_entity_store: skipping room '{}' — record bytes "
                          "unparseable",
                          roomId );
            continue;
        }
        m_rooms.emplace( record.id(), record );
    }
    return outcome::success();
}

outcome::result<SpaceRecord> EntityStore::CreateSpace( const std::string &name,
                                                       bool               isPublic,
                                                       bool               autoJoinRooms )
{
    SpaceRecord record;
    const std::string id = NextEntityId( kSpaceIdPrefix );
    const int64_t      nowMs = NowMs();
    record.set_id( id );
    record.set_name( name );
    record.set_is_public( isPublic );
    record.set_auto_join_rooms( autoJoinRooms );
    record.set_created_at_ms( nowMs );
    record.set_updated_at_ms( nowMs );
    record.set_deleted( false );          // D-03: tombstone fields present from
    record.set_deleted_at_ms( 0 );        // creation, set by no Phase 2 command

    // Manifest first, then the record (WR-02): a failed record Put leaves a
    // manifest id LoadFromStore skips gracefully ("record read failed"),
    // never unreachable orphan record bytes the manifest-less order left
    // behind (GlobalDB has no enumeration, so the manifest is the only index).
    auto manifestResult = AppendToManifest( m_session, true, id );
    if ( !manifestResult.has_value() )
    {
        return manifestResult.error();
    }
    auto putResult = m_session.Put( std::string( kSpacesKeyPrefix ) + id,
                                    record.SerializeAsString() );
    if ( !putResult.has_value() )
    {
        return putResult.error();
    }
    m_spaces.emplace( id, record );
    return record;
}

outcome::result<RoomRecord> EntityStore::CreateRoom( const std::string &name,
                                                     const std::string &parentSpaceId )
{
    if ( !parentSpaceId.empty() && !IsValidParentSpace( parentSpaceId ) )
    {
        spdlog::warn( "gcs_entity_store: create_room rejected — parent space "
                      "'{}' not found or deleted",
                      parentSpaceId );
        // The Error domain carries no NotFound code (Pitfall 2 — none added
        // this phase); GcsDbError doubles as the generic store rejection.
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    RoomRecord record;
    const std::string id = NextEntityId( kRoomIdPrefix );
    const int64_t      nowMs = NowMs();
    record.set_id( id );
    record.set_name( name );
    record.set_parent_space_id( parentSpaceId ); // empty = standalone (D-01)
    record.set_created_at_ms( nowMs );
    record.set_updated_at_ms( nowMs );
    record.set_deleted( false );          // D-03: tombstone fields present from
    record.set_deleted_at_ms( 0 );        // creation, set by no Phase 2 command

    // Manifest first, then the record (WR-02 — same rationale as CreateSpace:
    // a failed record Put degrades to a skipped manifest id on reload, never
    // an unreachable orphan record).
    auto manifestResult = AppendToManifest( m_session, false, id );
    if ( !manifestResult.has_value() )
    {
        return manifestResult.error();
    }
    auto putResult = m_session.Put( std::string( kRoomsKeyPrefix ) + id,
                                    record.SerializeAsString() );
    if ( !putResult.has_value() )
    {
        return putResult.error();
    }
    m_rooms.emplace( id, record );
    return record;
}

outcome::result<SpaceRecord> EntityStore::UpdateSpace( const SpaceRecord &desired )
{
    const auto existingIt = m_spaces.find( desired.id() );
    if ( existingIt == m_spaces.end() || existingIt->second.deleted() )
    {
        spdlog::warn( "gcs_entity_store: update_space rejected — space '{}' not "
                      "found or deleted",
                      desired.id() );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }

    // Full-state rewrite (per-key LWW replaces the whole value) — but the
    // immutable fields stay stamped by C++: the caller's desired record may
    // carry only id/name/is_public/auto_join_rooms, so created_at_ms and the
    // tombstone state are preserved from the stored record, never zeroed.
    SpaceRecord updated = desired;
    updated.set_created_at_ms( existingIt->second.created_at_ms() );
    updated.set_deleted( existingIt->second.deleted() );
    updated.set_deleted_at_ms( existingIt->second.deleted_at_ms() );
    updated.set_updated_at_ms( NowMs() );

    auto putResult = m_session.Put( std::string( kSpacesKeyPrefix ) + updated.id(),
                                    updated.SerializeAsString() );
    if ( !putResult.has_value() )
    {
        return putResult.error();
    }
    m_spaces[updated.id()] = updated;
    return updated;
}

std::vector<SpaceRecord> EntityStore::Spaces() const
{
    std::vector<SpaceRecord> spaces;
    for ( const auto &entry : m_spaces )
    {
        if ( !entry.second.deleted() )
        {
            spaces.push_back( entry.second );
        }
    }
    return spaces;
}

std::vector<RoomRecord> EntityStore::Rooms() const
{
    std::vector<RoomRecord> rooms;
    for ( const auto &entry : m_rooms )
    {
        if ( !entry.second.deleted() )
        {
            rooms.push_back( entry.second );
        }
    }
    return rooms;
}

std::vector<std::string> EntityStore::DerivedJoinedTopics() const
{
    std::vector<std::string> topics;
    for ( const RoomRecord &room : Rooms() )
    {
        if ( room.parent_space_id().empty() )
        {
            continue; // standalone rooms never join via a space (D-04)
        }
        const auto parentIt = m_spaces.find( room.parent_space_id() );
        if ( parentIt == m_spaces.end() )
        {
            continue; // orphaned record — no parent to derive from
        }
        if ( parentIt->second.deleted() || !parentIt->second.auto_join_rooms() )
        {
            continue; // tombstoned parent or auto-join disabled
        }
        topics.push_back( std::string( kRoomTopicPrefix ) + room.id() );
    }
    return topics;
}

bool EntityStore::IsValidParentSpace( const std::string &id ) const
{
    const auto it = m_spaces.find( id );
    return it != m_spaces.end() && !it->second.deleted();
}

} // namespace gcs
