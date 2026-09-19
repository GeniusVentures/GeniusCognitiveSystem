---
phase: 02-spaces-rooms
plan: 01
subsystem: database
tags: [protobuf, crdt, globaldb, entity-store, gcs-chat, c++17]

# Dependency graph
requires:
  - phase: 01-foundation
    provides: gcs::CoreSession Put/Get pass-throughs, gcs_chat.proto wire contract, gcs_test macro + wait-condition template
provides:
  - SpaceRecord/RoomRecord/EntityManifest/SpaceTree protos + GcsCommand arms create_space=3/create_room=4/update_space=5 + GcsEvent arm space_tree=5
  - gcs::EntityStore (manifest read-union-write, record CRUD, derived joins, tombstone skip) linked into gcs_core
  - test_gcs_entities binary proving CORE-01/02/03, tombstone skip, restart persistence
affects: [02-02 (FFI dispatch), 02-05 (Dart rail tree), phase-03 messaging]

# Tech tracking
tech-stack:
  added: [] # zero new packages (02-RESEARCH audit holds)
  patterns:
    - "Per-entity CRDT record keys + id manifest (no enumeration API exists)"
    - "Seed+counter opaque entity ids generalized from NextMessageId (CR-01)"
    - "Tombstone fields present from creation; readers skip, writers never set"
    - "Derived joins = pure recomputation over catalog (D-04), retroactive by construction"

key-files:
  created:
    - src/lib/gcs_entity_store.hpp
    - src/lib/gcs_entity_store.cpp
    - test/test_gcs_entities.cpp
  modified:
    - src/proto/gcs_chat.proto
    - src/CMakeLists.txt
    - test/CMakeLists.txt

key-decisions:
  - "Unparseable manifest bytes = empty catalog (union write heals the key); Get failure = absent key (Pitfall 2 conflation)"
  - "Entity-store rejections reuse Error::GcsDbError — no NotFound code added this phase (minimal change)"
  - "UpdateSpace preserves created_at_ms and tombstone state from the stored record; full-state rewrite otherwise"
  - "Tombstoned records stay in the in-memory maps so IsValidParentSpace keeps returning false (D-03)"

patterns-established:
  - "EntityStore over CoreSession&: unit-testable catalog without FFI/ABI"
  - "Read-union-write manifest membership (idempotent append, never prune)"

requirements-completed: [CORE-01, CORE-02, CORE-03]  # CORE-04 restart proven in test 7 but not a plan frontmatter requirement

# Metrics
duration: 9min
completed: 2026-09-19
---

# Phase 2 Plan 1: Entity Store Summary

**CRDT-backed spaces/rooms catalog: 7 append-only proto messages, gcs::EntityStore (seed+counter ids, manifest read-union-write, tombstone skip, D-04 derived joins) with 7 green unit tests incl. restart persistence**

## Performance

- **Duration:** 9 min
- **Started:** 2026-09-19T19:02:23Z
- **Completed:** 2026-09-19T19:10:50Z
- **Tasks:** 3
- **Files modified:** 6

## Accomplishments
- gcs_chat.proto extended append-only: SpaceRecord, RoomRecord, EntityManifest, CreateSpaceCommand, CreateRoomCommand, UpdateSpaceCommand, SpaceTree; command oneof arms 3-5 and event arm 5; all existing tags verified untouched by grep
- gcs::EntityStore implemented over CoreSession Put/Get: opaque C++-minted ids (wallclock ms + random_device + atomic seq), per-entity record keys, read-union-write manifest, absent-manifest = empty catalog, corrupt record bytes warn+skip (T-02-01), tombstone reader-skip (T-02-02 salt-shape test)
- test_gcs_entities: 7/7 green — salted-id shape, standalone room, autoJoin derived join, unknown-parent rejection, retroactive autoJoinRooms toggle (true→false→true) with created_at_ms preservation, tombstone skip via direct CRDT writes, two-session restart on shared db_path

## Task Commits

Each task was committed atomically:

1. **Task 1: Extend gcs_chat.proto (append-only entity contract)** - `8a5e892` (feat)
2. **Task 2: Implement gcs::EntityStore + wire into gcs_core target** - `059955f` (feat)
3. **Task 3: EntityStore unit tests (create/update/derived-join/tombstone/restart)** - `707eae8` (test)

**Plan metadata:** (see final commit below)

## Files Created/Modified
- `src/proto/gcs_chat.proto` - Phase 2 entity messages + oneof arms (append-only)
- `src/lib/gcs_entity_store.hpp` - gcs::EntityStore declaration (exact plan signatures)
- `src/lib/gcs_entity_store.cpp` - manifest/records/derived joins/tombstone skip implementation
- `src/CMakeLists.txt` - gcs_core gains gcs_entity_store.cpp + PUBLIC gcs_proto link
- `test/test_gcs_entities.cpp` - 7-case suite over the injected-pubsub seam
- `test/CMakeLists.txt` - gcs_test(test_gcs_entities ...) registration

## Decisions Made
- Manifest Get failure AND unparseable manifest bytes both mean "empty catalog" (GcsGlobalDb::Get conflates missing with failed — Pitfall 2); the union write heals an unparseable key on the next append
- EntityStore rejections (unknown parent, unknown/tombstoned space on update) return Error::GcsDbError — the domain has no NotFound code and research forbids adding one this phase; the FFI arms (Plan 02-02) do INVALID_ARGUMENT validation before the store call
- UpdateSpace copies the full desired state but re-stamps updated_at_ms and preserves created_at_ms/deleted/deleted_at_ms from the stored record (the FFI-built desired record carries only id/name/flags)
- Tombstoned records are loaded and kept in the in-memory maps (hidden by Spaces()/Rooms()) so IsValidParentSpace returns false for them

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] gcs_core required a PUBLIC gcs_proto link**
- **Found during:** Task 2 (EntityStore implementation)
- **Issue:** Plan asserted "include dirs and link libs already cover" the target — but only gcs_ffi linked gcs_proto; gcs_entity_store.cpp includes proto/gcs_chat.pb.h and the header exposes record types in public signatures, so gcs_core (and its test consumers linking gcs_core only) could not compile/link
- **Fix:** Added `target_link_libraries(gcs_core PUBLIC gcs_proto)` guarded by the existing `if(TARGET ...)` + FATAL_ERROR pattern in src/CMakeLists.txt — nothing else in the target changed
- **Files modified:** src/CMakeLists.txt
- **Verification:** `ninja -C build/OSX/Debug gcs_core` and `gcs_ffi` both exit 0; test_gcs_entities links with exactly the plan's lib list
- **Committed in:** 059955f (Task 2 commit)

**2. [Rule 1 - Bug] Proto record types needed using-declarations in namespace gcs**
- **Found during:** Task 2 (first gcs_core build)
- **Issue:** Plan said "SpaceRecord/RoomRecord are gcs::chat::* ... include it" — including gcs_chat.pb.h alone does not make them visible unqualified inside namespace gcs (20 unknown-type-name errors)
- **Fix:** Added `using chat::SpaceRecord; using chat::RoomRecord;` to gcs_entity_store.hpp so the plan's exact public signatures (`outcome::result<SpaceRecord>` etc.) compile
- **Files modified:** src/lib/gcs_entity_store.hpp
- **Verification:** Clean gcs_core compile
- **Committed in:** 059955f (Task 2 commit)

---

**Total deviations:** 2 auto-fixed (1 blocking, 1 bug)
**Impact on plan:** Both fixes required for the plan's own contract to compile. No scope creep — one CMake link line plus two using-declarations.

## Issues Encountered
- `test_gcs_global_db_sdk` (pre-existing Phase 1 binary) segfaults intermittently (~1-in-3 ctest runs) during global tear-down AFTER its gtest body passes. Unrelated to this plan: the binary links gcs_storage/neoswarm_common/sgns::crdt_globaldb only — no link path to gcs_core, gcs_entity_store, or gcs_proto. Logged to `deferred-items.md` (scope boundary rule; not fixed).

## User Setup Required
None - no external service configuration required.

## Known Stubs
None — every EntityStore method is fully implemented and reads/writes real CRDT state; no placeholder data paths exist.

## Threat Flags
None — no security-relevant surface beyond the plan's threat model. T-02-01 (corrupt bytes warn+skip, never abort startup) and T-02-02 (seed+counter ids with salt-shape test assertion) mitigations implemented exactly as registered.

## Next Phase Readiness
- EntityStore surface is frozen for Plan 02-02 (FFI dispatch): exact signatures per plan, g_entities construction + LoadFromStore in gcs_init, three command arms in gcs_publish under g_mutex, BuildSpaceTreeEvent mirroring BuildRoomListEvent
- C++ proto halves regenerate automatically; the Dart pb regen (pinned protoc_plugin 22.5.0) is still pending and must precede all Dart work (02-PATTERNS Pitfall 1 command)
- Restart criterion already proven at the store layer; FFI-level two-session coverage still owed by the later FFI test-extension plan (02-RESEARCH Wave 0 list)

## Self-Check: PASSED

All three task commits verified in git log (8a5e892, 059955f, 707eae8); all created files verified on disk (src/lib/gcs_entity_store.{hpp,cpp}, test/test_gcs_entities.cpp, this SUMMARY).

---
*Phase: 02-spaces-rooms*
*Completed: 2026-09-19*
