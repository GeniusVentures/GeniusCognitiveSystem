---
phase: 04-membership-invites
plan: 21
subsystem: membership-security
tags: [creator-authority, approval-validation, pending-state, tombstones, dormancy, crdt]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: signed member authority, verified read/write paths, and the creator/outrank/approval transition matrix (04-19), plus the signed test-seed/arrival contract (04-20)
provides:
  - Identity-based creator exception in CanChangeRole (catalog identity, never the record role)
  - Re-validated Approve pending roles and a creator-target rejection
  - A creator-signed approved tombstone that clears pending state and verifies
  - A pending-REMOVE write barrier in ChangeRole
  - Coherent dormant/immediate removal pending-state clearing
  - A RemoveMember dormancy predicate aligned with verification
  - Five pending-state, tombstone-verification, and creator-identity regression tests
affects: [04-22]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Creator authority is resolved from the catalog by identity, never from signed record data"
    - "Approve re-validates the pending operation before making it irreversible"
    - "A tombstone is written only with no pending state and no approval attribution unless the creator approved it"

key-files:
  created: []
  modified:
    - src/lib/gcs_membership.cpp
    - src/lib/gcs_membership.hpp
    - test/test_gcs_membership.cpp
    - .planning/workstreams/app/STATE.md

key-decisions:
  - "CanChangeRole's creator exception is IsSuperAdmin(scopeId, m_sender) — identity from the catalog"
  - "Approve rejects a creator target and any pending role outside Admin/Moderator/Member/Guest"
  - "Approve's REMOVE branch clears pending_action and pending_role before the creator-signed write"
  - "ChangeRole rejects while a removal is pending; Approve is the only exit"
  - "RemoveMember's dormancy predicate matches the verifier: 2+ admins AND (target or author admin-tier)"
  - "Immediate tombstones clear pending_action, pending_role, and approved_by"

requirements-completed: [MEMB-02, MEMB-03, MEMB-04]

# Metrics
duration: ~12min
completed: 2026-10-20
---

# Phase 4 Plan 21: Coherent Pending State and Creator Identity Summary

**The membership command path now fails closed independently of the trust chain: creator authority is catalog identity, Approve re-validates and clears pending state, a pending removal is terminal, and every tombstone it writes is verifiable and convergent.**

## Performance

- **Duration:** ~12 min
- **Completed:** 2026-10-20T00:41:00Z
- **Tasks:** 3
- **Files modified:** 4

## Accomplishments

- Replaced `CanChangeRole`'s creator exception (`author == ROLE_SUPER_ADMIN`, resolved from the record) with `IsSuperAdmin( scopeId, m_sender )`, so a record that merely claims SUPER_ADMIN never receives creator authority (WR-02/T-04-21-01).
- Re-validated `Approve`'s `PENDING_ACTION_ROLE_CHANGE` branch: it resolves the scope, rejects a creator target, and applies only a concrete pending role (Admin, Moderator, Member, Guest) before writing (WR-03/T-04-21-02).
- Fixed `Approve`'s `PENDING_ACTION_REMOVE` branch to clear `pending_action` and `pending_role` before the creator-signed write, producing a tombstone the 04-19 verifier accepts and replicates (WR-05/T-04-21-06).
- Added a `ChangeRole` write barrier for `PENDING_ACTION_REMOVE`, so a role change cannot silently cancel a pending removal (T-04-21-03).
- Aligned `RemoveMember`'s dormancy predicate with verification: dormancy requires 2+ admins AND (target or author admin-tier), so a Moderator's Member removal is immediate and verifiable (T-04-21-07).
- Canonicalized removal state: dormant removals clear `pending_role`; immediate tombstones clear `pending_action`, `pending_role`, and `approved_by` (T-04-21-04/05).
- Added five tests, including an `OnMemberArrived` roster-push proof that the approved and immediate tombstones pass verification.
- Recorded eleven accumulated Phase 04 decision bullets in `STATE.md`.

## Task Commits

Each task was committed atomically:

1. **Task 1: Identity-based creator exception, validated approval, and a verifiable approved tombstone** - `b17f00d` (feat)
2. **Task 2: Pending-removal write barrier, coherent pending-state clearing, and regression tests** - `cd91978` (feat)
3. **Task 3: Record the landed contract in STATE.md and the validation map** - `d87f0dc` (docs)

**Plan metadata:** pending final docs commit (SUMMARY.md, STATE.md, ROADMAP.md)

## Files Created/Modified

- `src/lib/gcs_membership.cpp` - Identity-based creator exception, validated/cleared `Approve` branches, pending-REMOVE barrier, verifier-aligned dormancy, canonical pending-field clearing.
- `src/lib/gcs_membership.hpp` - Doxygen for the identity exception, approval validation/tombstone clearing, the pending-removal barrier, and removal state clearing.
- `test/test_gcs_membership.cpp` - Five regression tests: `ChangeRoleRejectedWhileRemovalIsPending`, `DormantRemovalClearsAStalePendingRole`, `ImmediateRemovalClearsAllPendingFields`, `CreatorIdentityIsTheOnlyCreatorException`, `ModeratorMemberRemovalIsImmediateAfterTheTierAlignment`.
- `.planning/workstreams/app/STATE.md` - Eleven accumulated decisions covering the signed-record, approved-tombstone, monotonic-stamp, and pending-tier contracts.
- `.planning/workstreams/app/phases/04-membership-invites/04-VALIDATION.md` - No edit required: all twelve re-verification rows and the concrete expected-red note were already present and accurate.

## Decisions Made

- Creator authority is identity, not record data. `CanChangeRole` uses `IsSuperAdmin(scopeId, m_sender)`, so no signed record's claimed role can ever select the creator branch.
- `Approve` is a validation point, not a copy. The pending role must be one of the four concrete grantable roles and the target may never be the creator, because approval is irreversible.
- An approved removal is a creator-signed tombstone with both pending fields cleared. `WriteMemberAsAuthor` supplies `prior_role` from the stored dormant record and a strictly newer `updated_at_ms`.
- A pending removal is terminal. `ChangeRole` rejects it and only `Approve` clears it, so the destructive op cannot be silently overwritten.
- Command and verifier share one dormancy predicate (2+ admins AND target-or-author admin-tier), removing the wire-only dormant Member state.
- Immediate tombstones carry neither dormant state nor approval attribution; the removal push is filtered by the verified roster.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- **Acceptance-grep literal (not a code deviation):** the plan's check `grep -n "author == chat::ROLE_SUPER_ADMIN" src/lib/gcs_membership.cpp` returns nothing is overly broad. The phrase legitimately remains in `CanRemoveMember` and `CanDelete` (the scope boundary explicitly forbids changing them). The intended check — that `CanChangeRole` no longer uses the stored role as the creator-exception criterion — is satisfied and pinned by `grep -n "if ( IsSuperAdmin( scopeId, m_sender ) )"` at the `CanChangeRole` branch.
- **clang-format gate:** `clang-format --dry-run --Werror` reports pre-existing violations across these files (2211/394/635 error lines). The files are not clean at the plan base, and applying it would reformat the whole file against the minimal-change scope. This mirrors the documented 04-19/04-20 precedent; no new formatting divergence was introduced. No `.clang-tidy` exists at the GCS root (the only one belongs to the `GNUS-NEO-SWARM` submodule), so a project-configured clang-tidy run was not possible.
- **Build/lint gate actually run:** `ninja -C build/OSX/Debug gcs_core test_gcs_membership` completed with no errors or warnings; `ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` passed (51/51).

## Threat Flags

None — this plan changes command logic and tests only; no new network, auth, file-access, or schema surface was introduced.

## Known Stubs

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- 04-22 inherits a command path whose pending transitions are coherent and whose tombstones verify; the multinode suite remains owned by 04-22 (expected red until its cold-node catalog mirror lands).
- The unit suite is green at 51/51, including the approval-replication proof through `OnMemberArrived`.
- `test_gcs_membership_multinode` was not run or modified by this plan.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-20*

## Self-Check: PASSED

- Task commits exist: `b17f00d`, `cd91978`, `d87f0dc`.
- All modified source/test/state files and this SUMMARY exist on disk.
- `test_gcs_membership` ran 51/51 green.
