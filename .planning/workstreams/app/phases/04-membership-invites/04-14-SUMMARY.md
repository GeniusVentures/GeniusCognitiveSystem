---
phase: 04-membership-invites
plan: 14
subsystem: membership
tags: [entity-catalog, tombstone, dormant-approval, fail-closed, crdt, cpp17, gtest]

# Dependency graph
requires:
  - phase: 04-08
    provides: EntityStore catalog mutex discipline (mutators never hold it across a session Put/Get)
  - phase: 04-09
    provides: D-07 dormant-until-approved model and Super Admin approval gate
  - phase: 04-13
    provides: RedeemInvite state-preservation semantics built on the member records
provides:
  - EntityStore::ApplySpaceState/ApplyRoomState full-state catalog-publishing mutators
  - DeleteScope and Approve's entity branch routed through those mutators
  - Approve re-reads the authoritative stored record before evaluating dormancy
  - AdminCount returns outcome::result and fails closed on a scan failure
  - Injectable ScanFn seam for the approval-gate admin scan
affects: [04-15 invite URL work, membership multinode convergence, FFI delete/approve arms]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Full-state entity mutator: blocking Put outside m_catalogMutex, catalog swap inside"
    - "Authoritative read-modify-write: catalog only reveals the kind, dormancy is evaluated on session bytes"
    - "Fail-closed counter: a scan failure propagates instead of degrading to zero"

key-files:
  created: []
  modified:
    - src/lib/gcs_entity_store.hpp
    - src/lib/gcs_entity_store.cpp
    - src/lib/gcs_membership.hpp
    - src/lib/gcs_membership.cpp
    - test/test_gcs_entities.cpp
    - test/test_gcs_membership.cpp

key-decisions:
  - "Apply*State are full-state writes: no immutable-field preservation and no timestamp restamp, because the session is authoritative"
  - "Tombstoned records stay in the catalog map so parent-validity and derived-join readers keep excluding them"
  - "Approve learns only the record kind from the catalog, then re-reads the authoritative bytes for the dormancy predicate"
  - "AdminCount fails closed (Error::GcsDbError) because it gates the D-07 immediate-destructive branch"

patterns-established:
  - "Entity catalog publication: every destructive entity write goes through ApplySpaceState/ApplyRoomState before PushSpaceTree"

requirements-completed: [MEMB-04]

# Metrics
duration: 15min
completed: 2026-10-18
---

# Phase 4 Plan 14: Catalog-Consistent Destructive Entity Writes Summary

**Destructive scope deletes now publish to the EntityStore catalog through new ApplySpaceState/ApplyRoomState mutators, Approve re-reads the authoritative record, and AdminCount fails closed on a scan error**

## Performance

- **Duration:** 15 min
- **Started:** 2026-10-18T23:20:00Z
- **Completed:** 2026-10-18T23:35:22Z
- **Tasks:** 3
- **Files modified:** 6

## Accomplishments

- `EntityStore` gained symmetric `ApplySpaceState`/`ApplyRoomState` mutators: the caller's full record is persisted with the blocking Put outside `m_catalogMutex`, then published to the in-memory catalog under it (T-04-14-01 lock order preserved)
- `Membership::DeleteScope` writes both the space and room branches through the mutators and pushes only after the catalog swap, so the pushed `SpaceTree` no longer re-emits a stale live scope (CR-06 / VERIFICATION truth 11)
- `Membership::Approve`'s entity branch is now reachable: the catalog identifies the record kind, then the authoritative stored bytes are re-read to evaluate `approved_by`/`pending_action` before the tombstone is applied and pushed (T-04-14-03)
- `AdminCount` returns `outcome::result<unsigned int>`; `ChangeRole`, `RemoveMember`, and `DeleteScope` propagate the error before any decision, so a failed member scan can no longer read as zero admins (CR-05)
- An injectable `ScanFn` constructor argument makes the fail-closed branch testable without a broken store; production defaults to `CoreSession::QueryKeyValues`
- Regression coverage: catalog-publish unit tests for spaces and rooms, dormant-delete-then-approve against the catalog and pushed tree, no-dormant-delete rejection, and byte-identical fail-closed assertions across all three destructive call sites

## Task Commits

Each task was committed atomically:

1. **Task 1: EntityStore ApplySpaceState/ApplyRoomState mutators** - `2b6bd0f` (feat)
2. **Task 2: Route DeleteScope/Approve through the mutators; AdminCount fails closed** - `84e4dce` (fix)
3. **Task 3: Catalog, dormant-approval, and fail-closed regression tests** - `aa7b2d0` (test)

**Plan metadata:** `_pending_` (docs: complete plan)

## Files Created/Modified

- `src/lib/gcs_entity_store.hpp` - `ApplySpaceState`/`ApplyRoomState` declarations with Doxygen covering full-state semantics and the lock-order rule
- `src/lib/gcs_entity_store.cpp` - Put-then-catalog-swap implementations; tombstoned records kept in the map
- `src/lib/gcs_membership.hpp` - `ScanFn` typedef, defaulted constructor seam, `m_memberScan`, outcome-returning `AdminCount`
- `src/lib/gcs_membership.cpp` - `DeleteScope`/`Approve` routed through the entity mutators; three `AdminCount` call sites propagate; Approve re-reads the session record
- `test/test_gcs_entities.cpp` - `ApplySpaceStatePublishesTombstoneToCatalogAndStore`, `ApplyRoomStatePublishesTombstoneToCatalogAndStore`
- `test/test_gcs_membership.cpp` - `DeleteScopeDormantThenApproved`, `ApproveWithoutDormantEntityDeleteIsRejected`, `AdminCountScanFailurePropagatesAndFailsClosed`; the stale-catalog reload tail was replaced with catalog assertions

## Decisions Made

- `Apply*State` deliberately skip `UpdateSpace`'s immutable preservation and timestamp restamping: the caller has already read and amended the authoritative record, and the catalog must mirror exactly what was just written.
- A tombstoned record remains in the catalog map (readers filter it) so `IsValidParentSpace` and `DerivedJoinedTopics` keep excluding children immediately.
- Approve treats an absent authoritative key and a failed read as the same rejection path, matching `CoreSession::Get`'s `outcome::result<std::string>` contract.
- The `ScanFn` seam is intentionally narrow: only `AdminCount` consults it; production leaves it empty.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- `clang-format --dry-run --Werror` (v22.1.7 with the repository `.clang-format`) reports whole-file violations across all six touched files, including pre-existing lines (class-member indentation and `( ... )` spacing); the same condition was documented in 04-13. No reformat was applied to avoid an unrelated whole-file diff — the added lines follow the surrounding file style.
- `clang-tidy -p build/OSX/Debug` could not run: the generated `compile_commands.json` entries contain two compiler jobs (x86_64 and arm64 universal build), and clang-tidy requires exactly one. The build itself completed with no new warnings for all changed targets.

## Verification

- `ninja -C build/OSX/Debug gcs_core test_gcs_entities test_gcs_membership` - success, no new warnings
- `ctest --test-dir build/OSX/Debug -R "^test_gcs_(entities|membership)$" --output-on-failure` - passed (5 new/amended cases confirmed green by `--gtest_filter`)
- `ctest --test-dir build/OSX/Debug --output-on-failure` - all 13 suites passed, including `test_gcs_ffi` delete/approve arms and both multinode convergence suites (no deadlock, no stale push)
- Source checks: no `m_session.Put` remains in `DeleteScope`/`Approve`; every `PushSpaceTree()` follows the mutator on the success path; `AdminCount` has the outcome signature at its definition plus exactly three propagating call sites; `m_memberScan` is used only in `AdminCount`; Approve uses `outcome::result` semantics, never `std::optional`; no `sleep_for`/`sleep_until` in either test file

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- VERIFICATION truth 11 can be re-verified as PASS: a dormant scope delete is visible in the catalog, the approval branch is reachable, and the pushed tree carries the tombstone.
- CR-05 is closed: a member-scan failure aborts every destructive membership path.
- Plan 04-15's invite URL-shape work is independent of these changes.

---

*Phase: 04-membership-invites*
*Completed: 2026-10-18*

## Self-Check: PASSED

- FOUND: `.planning/workstreams/app/phases/04-membership-invites/04-14-SUMMARY.md`
- FOUND: `2b6bd0f` (Task 1)
- FOUND: `84e4dce` (Task 2)
- FOUND: `aa7b2d0` (Task 3)
- FOUND: `src/lib/gcs_entity_store.hpp`, `src/lib/gcs_entity_store.cpp`, `src/lib/gcs_membership.hpp`, `src/lib/gcs_membership.cpp`, `test/test_gcs_entities.cpp`, `test/test_gcs_membership.cpp`
