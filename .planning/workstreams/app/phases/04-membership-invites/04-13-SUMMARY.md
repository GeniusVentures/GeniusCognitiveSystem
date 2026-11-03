---
phase: 04-membership-invites
plan: 13
subsystem: membership
tags: [invite-redeem, tombstone, dormant-removal, crdt, cpp17, gtest]

# Dependency graph
requires:
  - phase: 04-09
    provides: Invite mint/redeem path with bearer authenticator and D-07 dormant approval model
provides:
  - RedeemInvite merges into an existing live member record (key material only)
  - Tombstoned wallets rejected after the authenticator gate and before the unwrap loop
  - Fresh members always written as ROLE_GUEST
  - Regression tests pinning CR-01 and CR-02
affects: [04-15 invite URL shape, membership multinode convergence, FFI redeem arms]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Read-before-write merge: TryReadMember after the authenticator gate, merge stored state, refresh only wrapped_room_key"
    - "Reject-before-side-effect ordering: tombstone check precedes crypto::StoreRoomKey"

key-files:
  created: []
  modified:
    - src/lib/gcs_membership.cpp
    - src/lib/gcs_membership.hpp
    - test/test_gcs_membership.cpp

key-decisions:
  - "Redeem delivers key material only; role and dormant state are never changed by a redeem"
  - "Tombstone rejection precedes the unwrap loop so no process-local room key is stored"
  - "A fresh member's role is the ROLE_GUEST constant, never the invite record's role"

patterns-established:
  - "Redeem merge pattern: existing live record is copied and only wrapped_room_key replaced"

requirements-completed: [MEMB-01, MEMB-04]

# Metrics
duration: 10min
completed: 2026-10-18
---

# Phase 4 Plan 13: Redeem State Preservation Summary

**RedeemInvite now merges key material into an existing live member record and rejects tombstoned wallets before any write, closing the CR-01 approval bypass and the CR-02 stale-link resurrection**

## Performance

- **Duration:** 10 min
- **Started:** 2026-10-18T23:16:00Z
- **Completed:** 2026-10-18T23:26:05Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments

- `RedeemInvite` reads the redeemer's stored record immediately after the bearer-authenticator gate and before the unwrap loop
- A tombstoned record is rejected with no member write and no `crypto::StoreRoomKey` call (CR-02)
- An existing live record is merged: `role`, `approved_by`, `pending_action`, `pending_role`, `deleted`, and `explicit_leave` survive; only `wrapped_room_key` is refreshed (CR-01)
- A fresh record is always written as `ROLE_GUEST`, never the invite record's role
- Two regression tests pin both behaviours, including a byte-identical baseline comparison for the rejected redeem

## Task Commits

Each task was committed atomically:

1. **Task 1: RedeemInvite preserves live state and rejects tombstones** - `21b2036` (fix)
2. **Task 2: Regression tests for dormant removal and tombstoned redeem** - `e52a719` (test)

**Plan metadata:** `_pending_` (docs: complete plan)

## Files Created/Modified

- `src/lib/gcs_membership.cpp` - tombstone rejection inserted before the unwrap loop; write step changed to read-merge
- `src/lib/gcs_membership.hpp` - `RedeemInvite` Doxygen documents preserve/reject semantics and the ROLE_GUEST fresh-record rule
- `test/test_gcs_membership.cpp` - `DormantRemovalSurvivesRedeem` and `TombstonedMemberCannotRedeemStaleInvite`

## Decisions Made

- Redeem is key delivery only: it never grants or revokes a role, never cancels a pending destructive op, and never revives a tombstone. A pending removal is activated only by `Approve`.
- The tombstone read/reject is placed after the authenticator gate so only an invite holder can probe, and before the unwrap loop so a rejected redeem writes nothing at all.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- `clang-format --dry-run --Werror` (v22.1.7) reports pre-existing violations across the file (include-comment alignment in `gcs_membership.cpp`, namespace/declaration indentation in `gcs_membership.hpp`) that are unrelated to this plan's changes. No reformat was applied to avoid an unrelated whole-file diff; the added lines follow the surrounding Allman/alignment style. `clang-tidy` was not run for the same reason (no new warnings from the build).

## Verification

- `ninja -C build/OSX/Debug test_gcs_membership` - success, no new warnings
- `ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` - passed (2 new cases; `--gtest_filter` confirms both run green)
- `ctest --test-dir build/OSX/Debug --output-on-failure` - all 13 suites passed (including FFI, FFI SDK, cold boot, and both multinode suites)
- Source checks: `existing.deleted()` rejection appears before `crypto::StoreRoomKey`; the live-record branch contains no `set_role`/`set_deleted`/`set_approved_by`/`set_pending_action`/`set_pending_role`; no `sleep_for`/`sleep_until` in the test file

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- CR-01 and CR-02 are closed; VERIFICATION truths 5, 29, and 30 can be re-verified as PASS for the redeem path
- Plan 04-15's URL-shape change can proceed; both new tests use the URL returned by `MintInvite` verbatim

---
*Phase: 04-membership-invites*
*Completed: 2026-10-18*

## Self-Check: PASSED

- FOUND: `.planning/workstreams/app/phases/04-membership-invites/04-13-SUMMARY.md`
- FOUND: `21b2036` (Task 1)
- FOUND: `e52a719` (Task 2)
