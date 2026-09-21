---
phase: 02-spaces-rooms
plan: 02
subsystem: ffi
tags: [ffi, entity-store, protobuf, space-tree, derived-joins, dart-push, c++17]

# Dependency graph
requires:
  - phase: 02-spaces-rooms plan 01
    provides: gcs::EntityStore over CoreSession Put/Get, Phase 2 proto arms (create_space=3/create_room=4/update_space=5, space_tree=5)
provides:
  - gcs_publish dispatch of create_space/create_room/update_space (validate-first, T-02-04/05) mutating gcs::EntityStore under g_mutex
  - BuildSpaceTreeEvent + push ordering SpaceTree -> RoomList -> Readiness at subscribe and after mutations
  - RefreshDerivedJoins D-04 projection (listen-first registration, projection-only removal on toggle-false)
  - FFI-level restart persistence proof via fake Dart port capture (test_gcs_ffi_sdk)
affects: [02-05 (Dart rail tree + dialog depend on pushed SpaceTree), phase-03 messaging (send_text joined-set check now sees derived rooms)]

# Tech tracking
tech-stack:
  added: [] # zero new packages (02-RESEARCH audit holds)
  patterns:
    - "Entity catalog global beside session global (unique_ptr<EntityStore>, guarded by the same g_mutex, reset in shutdown)"
    - "Derived-join third source of the joined set: smoke ∪ explicit ∪ derived; derived tracked separately so toggle-false removes from projection only (Pitfall 4)"
    - "Subscribe-time catalog push before membership before readiness (D-02 ordering)"

key-files:
  created:
    - .planning/workstreams/app/phases/02-spaces-rooms/02-02-SUMMARY.md
  modified:
    - src/ffi/gcs_core_ffi.cpp
    - test/test_gcs_ffi_sdk.cpp

key-decisions:
  - "gcs_init never fails on catalog load — LoadFromStore error logs + continues (empty-catalog contract; missing manifest is normal first launch)"
  - "create_space pushes SpaceTree only (no RoomList) — a new space has no rooms, so the derived set cannot change"
  - "create_room/update_space call RefreshDerivedJoins BEFORE pushing so RoomList reflects the new derived set in the same publish"
  - "Derived topics register listen-first then broadcast (D-07); failed registration keeps the topic out of BOTH g_derivedTopics and g_roomTopics so projection always matches registrations"
  - "Shutdown ordering: g_derivedTopics.clear() + g_entities.reset() after g_session.reset() is safe (EntityStore dtor never dereferences the borrowed session)"

patterns-established:
  - "Fake-Dart-port push capture for entity events: TakeLatestSpaceTree drains the ordered log and returns the latest tree + parsed event sequence"

requirements-completed: [CORE-01, CORE-02, CORE-03]

# Metrics
duration: 5min
completed: 2026-09-19
---

# Phase 2 Plan 2: FFI Entity Dispatch Summary

**EntityStore wired into the C ABI: 3 validate-first command arms, SpaceTree pushed at subscribe + after mutations (tree → RoomList → Readiness), D-04 derived-join projection with sticky pubsub, proven end-to-end by a two-session fake-port FFI test (create → derived join → restart persistence)**

## Performance

- **Duration:** 5 min
- **Started:** 2026-09-19T19:14:21Z
- **Completed:** 2026-09-19T19:19:08Z
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments
- `gcs_publish` gained kCreateSpace/kCreateRoom/kUpdateSpace arms: empty-name and unknown-parent rejection via PostErrorNotice + GCS_ERROR_INVALID_ARGUMENT before any store write (T-02-05); store failures surface as GCS_ERROR_GENERIC with the entity name in the raw error string (D-29)
- `BuildSpaceTreeEvent()` mirrors `BuildRoomListEvent()` — flat SpaceRecord/RoomRecord lists off `g_entities->Spaces()/Rooms()` (tombstoned already skipped by the store)
- `RefreshDerivedJoins()` recomputes `DerivedJoinedTopics()` (D-04): new topics register listen-then-broadcast (D-07) and enter `g_derivedTopics` + `g_roomTopics`; topics that left the derived set (autoJoinRooms toggled false) leave both vectors while pubsub registrations stay sticky (no Remove*Topic API exists — Pitfall 4 documented, not "fixed")
- `gcs_init` constructs `g_entities` strictly AFTER the `g_session` move, loads the catalog (failure = empty catalog, never init failure), refreshes derived joins with no pushes (port unregistered — Pitfall 9); `gcs_shutdown` resets `g_entities` and clears `g_derivedTopics`
- `gcs_subscribe` pushes SpaceTree before RoomList before Readiness (tree before membership before readiness, D-02)
- `test_gcs_ffi_sdk` extended: `CreateSpaceAndRoomPersistAcrossSessionCyclesOnSharedDb` captures the minted space id from the pushed SpaceTree (prefix-asserted "space-"/"room-", CR-01 salt shape preserved by the store), creates a room in it, asserts the derived topic `gcs/chat/<room-id>` appears in a RoomList pushed after the tree, then reopens the same db_path after a full shutdown and asserts the catalog is re-pushed at subscribe (restart persistence, criterion 4)
- Full backend verification green: `ninja -C build/OSX/Debug && ctest -R "gcs"` → 6/6 passed (including the known-intermittent `test_gcs_global_db_sdk`, which passed on this run)

## Task Commits

Each task was committed atomically:

1. **Task 1: Wire EntityStore into FFI (globals, arms, SpaceTree push, derived joins)** - `04dc37e` (feat)
2. **Task 2: FFI two-session entity persistence test** - `3555991` (test)

**Plan metadata:** (see final commit below)

## Files Created/Modified
- `src/ffi/gcs_core_ffi.cpp` - g_entities/g_derivedTopics globals, BuildSpaceTreeEvent, RefreshDerivedJoins, 3 command arms, init load, subscribe push ordering, shutdown teardown
- `test/test_gcs_ffi_sdk.cpp` - InitSession + TakeLatestSpaceTree helpers, CreateSpaceAndRoomPersistAcrossSessionCyclesOnSharedDb test, entity-prefix constants

## Decisions Made
- gcs_init treats a LoadFromStore error as "continue with an empty catalog" — the store's own contract makes a missing manifest an empty catalog, so an error here degrades gracefully and never blocks readiness
- create_space pushes SpaceTree only (no RoomList): a fresh space has no rooms, so the derived set and RoomList provably cannot change — saves a redundant push
- create_room/update_space run RefreshDerivedJoins before pushing so a single publish carries the consistent SpaceTree + RoomList pair
- A derived topic whose AddListenTopic/AddBroadcastTopic fails is kept out of both g_derivedTopics and g_roomTopics — the RoomList projection keeps matching only topics that joined both ways (same invariant as the join_topic arm)
- RefreshDerivedJoins erases by rebuilding the retained vector (iterate g_derivedTopics, partition on membership in the fresh derived set, remove leavers from g_roomTopics) — no iterator invalidation, smoke/explicit topics untouched by construction

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered
None. (The pre-existing intermittent `test_gcs_global_db_sdk` teardown segfault from deferred-items.md did not fire on this run — no re-run needed.)

## User Setup Required
None - no external service configuration required.

## Known Stubs
None — every arm dispatches to the real EntityStore and every push reads live catalog state; no placeholder data paths exist.

## Threat Flags
None — no security-relevant surface beyond the plan's threat model. T-02-04 (parse-before-state-change, inherited from the existing gcs_publish parse guard) and T-02-05 (empty name / unknown parent / empty space_id all rejected with PostErrorNotice + GCS_ERROR_INVALID_ARGUMENT before any Put) implemented exactly as registered; T-02-06 narrowing guard unchanged and inherited.

## Next Phase Readiness
- Pushed SpaceTree ordering (SpaceTree → RoomList → Readiness) and the derived-join RoomList behavior are frozen for the Dart plans: dispatch arm `hasSpaceTree → RailCubit.setTree`, tree render, create/edit dialog publishing through GcsCommandTransport
- Dart pb regeneration (pinned protoc_plugin 22.5.0) is still pending and must precede all Dart work (02-PATTERNS Pitfall 1)
- Entity writes remain local-restart persistence only (`GcsGlobalDb::Put` hard-codes kNoTopics — Pitfall 3); no cross-node sync expectation this phase

## Self-Check: PASSED

Both task commits verified in git log (04dc37e, 3555991); both modified files verified on disk; this SUMMARY present in the plan directory.

---
*Phase: 02-spaces-rooms*
*Completed: 2026-09-19*
