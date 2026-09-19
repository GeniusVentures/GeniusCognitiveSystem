/**
 * @file       gcs_entity_store.hpp
 * @brief      GCS entity store — the C++-owned catalog of spaces and rooms
 * persisted as CRDT records in GlobalDB (Phase 2, D-01/D-02/D-03/D-04).
 * @details    EntityStore composes over CoreSession's Put/Get pass-throughs:
 * per-entity record keys (gcs/entities/spaces/<id>, gcs/entities/rooms/<id>)
 * each hold one serialized entity record, and a manifest key
 * (gcs/index/manifest) lists the known entity ids (GlobalDB offers no
 * enumeration, so the manifest is the catalog index — D-02). Entity ids are
 * opaque and C++-minted (D-01); deletion is tombstone-based (the deleted flag,
 * never key removal — D-03) and readers skip tombstoned records; joined rooms
 * are derived state recomputed over the catalog (D-04) — never emitted ops.
 * @copyright  (c) 2026 GNUS.AI
 */

#ifndef GCS_ENTITY_STORE_HPP
#define GCS_ENTITY_STORE_HPP

#include <map>
#include <string>
#include <vector>

#include "gcs_core.hpp"                 // CoreSession Put/Get pass-throughs
#include "gcs_storage/common/error.hpp" // sgns::gcs::Error (no NotFound code — Pitfall 2)
#include "proto/gcs_chat.pb.h"          // SpaceRecord / RoomRecord / EntityManifest

namespace gcs {

// The wire record protos live in gcs::chat (proto package); the store's public
// signatures use them unqualified.
using chat::SpaceRecord;
using chat::RoomRecord;

/**
 * @brief Catalog of spaces and rooms persisted as CRDT entity records.
 *
 * The constructor stores the session reference only (deferred registration
 * idiom — no I/O). Mutators read-union-write the manifest first, then write
 * the record key — a failed record write degrades to a manifest id that
 * LoadFromStore skips gracefully, never unreachable orphan record bytes.
 * Call LoadFromStore() after session Initialize() to replay the
 * persisted catalog into memory (missing manifest = empty catalog, never an
 * error). Copy and move are deleted: the store borrows the session by
 * reference for its whole lifetime.
 */
class EntityStore {
public:
  /**
   * @brief Construct the store over a live session, storing the reference only.
   *
   * Performs no I/O and no fallible work (deferred registration idiom).
   *
   * @param[in] session The owning session; must outlive this store.
   */
  explicit EntityStore(CoreSession &session);

  EntityStore(const EntityStore &) = delete;
  EntityStore &operator=(const EntityStore &) = delete;

  /**
   * @brief Replay the persisted catalog into memory (startup, D-02).
   *
   * Reads the manifest, then Gets each listed record (N+1 reads). A missing
   * manifest key means an empty catalog (GcsGlobalDb::Get conflates missing
   * with failed — success is returned, never an error). A record whose Get
   * fails or whose bytes fail to parse is skipped with a warning — hostile or
   * corrupt bytes never abort startup (T-02-01). Tombstoned records are KEPT
   * in memory so IsValidParentSpace() returns false for them.
   *
   * @return outcome::success once the catalog is loaded (an absent or
   *         unparseable manifest is an empty catalog, never an error).
   */
  outcome::result<void> LoadFromStore();

  /**
   * @brief Create a space: mint an opaque id, persist the record + manifest.
   *
   * C++ stamps id, timestamps, and the tombstone fields (D-01/D-27 — the
   * authority fields never come from Dart).
   *
   * @param[in] name          Display name (mutable metadata, not identity).
   * @param[in] isPublic      Public/private display flag.
   * @param[in] autoJoinRooms D-04 derived-join source of truth.
   * @return outcome::success with the stored SpaceRecord; propagated Error on
   *         a failed record/manifest write.
   */
  outcome::result<SpaceRecord> CreateSpace(const std::string &name,
                                           bool isPublic, bool autoJoinRooms);

  /**
   * @brief Create a room: mint an opaque id, persist the record + manifest.
   *
   * An empty parentSpaceId creates a standalone room (D-01 unified room
   * model). A non-empty parentSpaceId must reference a known, non-tombstoned
   * space — otherwise the call is rejected without writing anything.
   *
   * @param[in] name          Display name (mutable metadata, not identity).
   * @param[in] parentSpaceId Parent space id; empty = standalone.
   * @return outcome::success with the stored RoomRecord; Error on an unknown
   *         parent or a failed record/manifest write.
   */
  outcome::result<RoomRecord> CreateRoom(const std::string &name,
                                         const std::string &parentSpaceId);

  /**
   * @brief Overwrite a space record with the full desired state (CORE-03).
   *
   * Per-key last-writer-wins replaces the whole value, so the desired record
   * is a full state, never a patch. The immutable fields (id, created_at_ms,
   * tombstone state) are preserved from the stored record; updated_at_ms is
   * re-stamped.
   *
   * @param[in] desired Full desired space state (id must exist and be alive).
   * @return outcome::success with the stored record; Error if the id is
   *         unknown or tombstoned, or the write failed.
   */
  outcome::result<SpaceRecord> UpdateSpace(const SpaceRecord &desired);

  /**
   * @brief All non-tombstoned spaces in id order.
   *
   * @return Id-sorted SpaceRecord list (std::map order).
   */
  std::vector<SpaceRecord> Spaces() const;

  /**
   * @brief All non-tombstoned rooms in id order.
   *
   * @return Id-sorted RoomRecord list (std::map order).
   */
  std::vector<RoomRecord> Rooms() const;

  /**
   * @brief Derived joined-room topics (D-04 — pure recomputation).
   *
   * Emits gcs/chat/<room-id> for every non-tombstoned room whose parent space
   * exists, is not tombstoned, and has auto_join_rooms() == true. Never emits
   * ops; retroactive by construction (re-evaluated on every call).
   *
   * @return Topic strings in id-sorted room order.
   */
  std::vector<std::string> DerivedJoinedTopics() const;

  /**
   * @brief Whether the id references a known, non-tombstoned space.
   *
   * @param[in] id Candidate parent space id.
   * @return true if the space exists and is not deleted.
   */
  bool IsValidParentSpace(const std::string &id) const;

private:
  /**
   * @brief Builds the next process-unique entity id (D-01 authority stamp).
   *
   * The CRDT store persists across process launches while a bare per-process
   * counter restarts at zero every launch — ids stamped as prefix + counter
   * alone revisit a prior session's key space and silently overwrite its
   * records under last-writer-wins (CR-01). The id therefore carries a
   * per-process seed — wall-clock milliseconds plus a std::random_device
   * token — followed by the in-process counter.
   *
   * @param[in] prefix Id prefix ("space-" / "room-").
   * @return prefix + "<wallclock-ms>-<random-token>-<seq>".
   */
  static std::string NextEntityId(const char *prefix);

  CoreSession &m_session; ///< Borrowed session (Put/Get pass-throughs)

  std::map<std::string, SpaceRecord>
      m_spaces; ///< Known spaces by id (tombstoned records kept)
  std::map<std::string, RoomRecord>
      m_rooms; ///< Known rooms by id (tombstoned records kept)
};

} // namespace gcs

#endif // GCS_ENTITY_STORE_HPP
