---
phase: 04-membership-invites
plan: 08
subsystem: concurrency
tags: [mutex, std-map, data-race, entity-store, tombstone, gtest, c++17]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: EntityStore catalog + Membership ResolveScope reading it from the FFI's unlocked window (04-04, 04-05)
provides:
  - Mutex-guarded EntityStore catalog reads/writes that are safe from the FFI's unlocked membership window
  - DerivedJoinedTopics() direct-m_rooms loop preserving the room-level tombstone skip
  - Concurrent reader/writer stress test and tombstoned-child-room derivation regression
affects: [04-membership-invites verification truth 21, any future EntityStore consumer on a second thread]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Owner-owned catalog mutex: EntityStore hides synchronization of its private maps; callers keep taking by-value snapshots"
    - "Lock only across in-memory map access, never across CoreSession Put/Get (lock-order discipline)"

key-files:
  created: []
  modified:
    - src/lib/gcs_entity_store.hpp
    - src/lib/gcs_entity_store.cpp
    - test/test_gcs_entities.cpp

key-decisions:
  - "EntityStore owns the catalog mutex rather than the FFI snapshotting at every call site (single source of truth)"
  - "DerivedJoinedTopics() locks once and iterates m_rooms directly, re-applying the room-level deleted() skip"
  - "m_catalogMutex is never held across m_session Put/Get to avoid inverting the FFI g_mutex lock order"

patterns-established:
  - "Catalog accessors lock internally and return by value; no caller-visible synchronization primitive"
  - "Mutators copy immutable authority fields under the lock, Put unlocked, then publish the stored record under the lock"

requirements-completed: [MEMB-01, MEMB-02, MEMB-04]

# Metrics
duration: 11min
completed: 2026-10-18
---

# Phase 04 Plan 08: EntityStore Catalog Synchronization Summary

**EntityStore's space/room catalog is now internally mutex-guarded, removing the CR-01 data race between unlocked Membership readers and entity-mutating command arms while preserving every accessor's filtering and ordering.**

## Performance

- **Duration:** 11 min
- **Started:** 2026-10-18T20:42:47Z
- **Completed:** 2026-10-18T20:53:00Z
- **Tasks:** 3
- **Files modified:** 3

## Accomplishments

- Added `mutable std::mutex m_catalogMutex` to `EntityStore` with documented lock discipline (guards the two maps only; never held across a session Put/Get).
- All four catalog readers (`Spaces()`, `Rooms()`, `DerivedJoinedTopics()`, `IsValidParentSpace()`) take the lock; `DerivedJoinedTopics()` locks once, iterates `m_rooms` directly, and re-applies the room-level `deleted()` skip.
- All catalog writers (`CreateSpace`, `CreateRoom`, `UpdateSpace`, `LoadFromStore`) mutate the maps under the same mutex; `UpdateSpace` was restructured into copy-locals → unlocked Put → publish, so no iterator or lock spans the blocking store call.
- Added a deterministic one-writer/two-reader stress test and a tombstoned-child-room derivation regression (13 entity cases, up from 11).
- Full C++ suite green: 13/13 tests passed.

## Task Commits

Each task was committed atomically:

1. **Task 1: Add the catalog mutex and synchronize the read accessors** - `a01012f` (fix)
2. **Task 2: Synchronize the catalog mutators and LoadFromStore** - `2a20d40` (fix)
3. **Task 3: Concurrent stress test + tombstoned-child-room derivation case** - `6d22fe6` (test)

**Plan metadata:** (docs: complete 04-08 plan)

## Files Created/Modified

- `src/lib/gcs_entity_store.hpp` - `<mutex>` include, `m_catalogMutex` member with lock-order documentation.
- `src/lib/gcs_entity_store.cpp` - Mutex-guarded readers and writers; direct `m_rooms` loop with tombstone skip; `UpdateSpace` three-phase restructure; `<mutex>` include.
- `test/test_gcs_entities.cpp` - `ConcurrentCatalogReadsAndRoomCreatesDoNotRace` and `DerivedJoinedTopicsSkipsTombstonedChildRoom`; two writer/reader count constants; `<atomic>`/`<thread>` includes.

## Decisions Made

- The mutex lives in the owner (`EntityStore`), not in the FFI. Every FFI unlock site (`ResolveScope`, `PushSpaceTree`, `DeleteScope`, `Approve`, `MintInvite`, `RefreshDerivedJoins`) keeps calling the same public accessors unchanged.
- `DerivedJoinedTopics()` cannot call the now-locking `Rooms()` (non-recursive mutex), so it locks once and re-applies the room-level tombstone skip itself.
- `AppendToManifest` remains outside the mutex: entity creation is already serialized by the FFI's `g_mutex`, and locking it would add nesting without closing a race.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical] Guarded LoadFromStore's map clears**
- **Found during:** Task 2 (mutators)
- **Issue:** The plan's action text named only the `emplace` inserts, but `m_spaces.clear()` / `m_rooms.clear()` are also catalog writes and would race a reader just like an insert.
- **Fix:** Wrapped both clears in one `m_catalogMutex` scope; the manifest Get and per-record Gets remain unlocked.
- **Files modified:** `src/lib/gcs_entity_store.cpp`
- **Verification:** Entity suite passes; no Put/Get inside the locked scope.
- **Committed in:** `2a20d40`

**2. [Rule 3 - Blocking] Added `<mutex>` include to the implementation file**
- **Found during:** Task 1
- **Issue:** `gcs_entity_store.cpp` directly uses `std::lock_guard`; relying on the header's transitive include violates include-what-you-use.
- **Fix:** Added `#include <mutex>` to the cpp's standard-library block (the plan only required it in the header).
- **Files modified:** `src/lib/gcs_entity_store.cpp`
- **Verification:** Builds and tests pass.
- **Committed in:** `a01012f`

---

**Total deviations:** 2 auto-fixed (1 missing critical, 1 blocking)
**Impact on plan:** Both are direct consequences of the synchronization contract; no scope creep, no behavior change.

## Issues Encountered

- **clang-format version mismatch:** the installed clang-format 22.1.7 reports the entire pre-existing `gcs_entity_store.{hpp,cpp}` and `test_gcs_entities.cpp` as unformatted (the same failure exists on the `HEAD` baseline). To avoid a whole-file reformat, formatting was applied/verified only to the new line ranges; those ranges are clean under the repo `.clang-format`.
- **clang-tidy could not run to completion:** the repo root has no `.clang-tidy` (only `GNUS-NEO-SWARM/.clang-tidy`), and the generated `compile_commands.json` emits two compiler jobs per file (x86_64 + arm64 universal build), which clang-tidy rejects ("expected exactly one compiler job"). The build itself is warning-free.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- VERIFICATION truth 21 (CR-01) can be re-verified as PASS: both catalog readers and writers use one mutex owned by `EntityStore`, and `Membership` is untouched.
- The remaining Phase 4 gap plans (04-09..04-12) are unaffected by this refactor: no public signature changed.

## Self-Check: PASSED

---
*Phase: 04-membership-invites*
*Completed: 2026-10-18*
