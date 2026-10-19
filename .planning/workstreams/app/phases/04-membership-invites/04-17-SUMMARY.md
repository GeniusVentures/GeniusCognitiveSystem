---
phase: 04-membership-invites
plan: 17
subsystem: membership
tags: [authorization, role-change, privilege-rank, cr03, cr04, cpp17, gtest]
gap_closure: true

# Dependency graph
requires:
  - phase: 04-09
    provides: D-03 creator authority root and D-07/WR-03 dormant role-change model
  - phase: 04-15
    provides: Issuer-signed invite semantics and the D-05 GUEST role cap
  - phase: 04-16
    provides: Production issuer seam; the FFI suite is green again
provides:
  - CanChangeRole rejects ROLE_UNSPECIFIED and ROLE_SUPER_ADMIN grants for every author
  - CanChangeRole requires a live target and strict outrank of the target's CURRENT stored role
  - Documented creator exception preserving the WR-03 peer Super Admin demotion path
  - Authorization-matrix tests covering every rejected role/rank combination
affects: [04-18 real-signer multinode proof]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Role-change authorization mirrors removal authorization: concrete grant set + strict PrivilegeRank outrank against the target's current record"
    - "Reject-before-write ordering: guard failure returns before the dormancy branch, WriteMember, or PushMemberList"

key-files:
  created: []
  modified:
    - src/lib/gcs_membership.cpp
    - src/lib/gcs_membership.hpp
    - test/test_gcs_membership.cpp

key-decisions:
  - "ROLE_SUPER_ADMIN is never grantable through ChangeRole by any author, including the creator: it is the D-03 root created with its scope"
  - "The concrete-role guard rejects ROLE_UNSPECIFIED because it sorts below Guest and would corrupt every rank comparison"
  - "A role change requires a live target record, read BEFORE the creator exception and the rank comparison"
  - "The creator exception sits after the target read and before the rank comparison, preserving the documented WR-03 peer-demotion dormancy workflow"
  - "Both CanRemoveMember and CanChangeRole now use the identical PrivilegeRank strict-outrank rule (single ordering authority)"

patterns-established:
  - "Grant validation before author resolution: an unrepresentable desired role is rejected regardless of who asks"

requirements-completed: [MEMB-02, MEMB-03]

# Metrics
duration: 5min
completed: 2026-10-19
---

# Phase 4 Plan 17: Role-Change Authorization Hardening Summary

**CanChangeRole now grants only concrete roles and requires a non-creator author to strictly outrank the target's current stored role, closing CR-03 and CR-04**

## Performance

- **Duration:** 5 min
- **Started:** 2026-10-19T00:00:14Z
- **Completed:** 2026-10-19T00:05:09Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments

- Closed CR-03: `CanChangeRole` rejects `ROLE_UNSPECIFIED` and every `ROLE_SUPER_ADMIN` grant for all authors, including the creator, so no second authority root or unset role can be written
- Closed CR-04: `CanChangeRole` now reads the target's CURRENT stored record and applies the same `PrivilegeRank( author ) <= PrivilegeRank( targetRecord.role() )` strict-outrank rule as `CanRemoveMember`
- Preserved the documented creator exception: the creator may still amend any non-self member, and the WR-03 dormancy rule keeps a peer Super Admin demotion dormant until approval
- Pinned the full authorization matrix: flipped the prior Admin→Super Admin success expectation and added Moderator overreach, Super Admin grant / unset role, and explicit Admin-initiated peer demotion cases

## Task Commits

Each task was committed atomically:

1. **Task 1: CanChangeRole requires a concrete grant role and strict outrank** - `dfef1a0` (fix)
2. **Task 2: Role/rank tests, including the flipped Admin→Super Admin expectation** - `372d3e4` (test)

**Plan metadata:** `_pending_` (docs: complete plan)

## Files Created/Modified

- `src/lib/gcs_membership.cpp` - `CanChangeRole` rewritten: concrete-role guard, allowed-author set, live-target read, creator exception, strict-outrank guard, then author-specific grant sets
- `src/lib/gcs_membership.hpp` - `CanChangeRole` Doxygen states the concrete-role rule, the strict-outrank rule, and the creator exception
- `test/test_gcs_membership.cpp` - flipped Admin→Super Admin expectation; added `ModeratorCannotInitiateHighRankRoleChange`, `ChangeRoleRejectsSuperAdminGrantAndUnspecifiedRole`, and `AdminCannotInitiateSuperAdminDemotion`

## Decisions Made

- The concrete-role guard runs before author resolution so an invalid desired role is rejected even for the creator.
- The creator exception is deliberately placed after the target-liveness read but before the rank comparison; this keeps WR-03 peer demotion dormant rather than over-restricting it (T-04-17-05).
- `SuperAdminDemotionIsDormantWhenTwoAdmins` was left untouched as the pin for the creator exception.
- The Admin branch keeps `role != ROLE_SUPER_ADMIN` even though the concrete-role guard makes it redundant, for D-03 clarity as the plan specified.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- `clang-format --dry-run --Werror` (v22.1.7 with the repository `.clang-format`) reports whole-file pre-existing violations on all three touched files (namespace/declaration indentation and include alignment). The same condition was documented in 04-13, 04-14, and 04-15; no whole-file reformat was applied. The added lines follow the surrounding file style and the targets build without warnings.
- `clang-tidy -p build/OSX/Debug` reports "no checks enabled": no `.clang-tidy` config resolves at the repository root (the only one lives inside the `GNUS-NEO-SWARM` submodule). No Required findings could therefore be evaluated.

## Threat Surface

No security-relevant surface beyond the plan's `<threat_model>` was introduced. The change narrows an existing authorization boundary; it adds no endpoint, storage key, or schema field.

## Verification

- `ninja -C build/OSX/Debug gcs_core test_gcs_membership` - success, no new warnings
- `ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` - passed
- Targeted `--gtest_filter` run of the amended plus three new cases - 5/5 passed
- `ctest --test-dir build/OSX/Debug -E "^test_gcs_membership_multinode$" --output-on-failure` - 12/12 suites passed
- Source checks: two strict-outrank sites; `TryReadMember( scopeId, target` inside `CanChangeRole` before the rank comparison; concrete-role guard names all four grantable roles; creator `return true` after the target read and before the rank comparison; no `sleep_for`/`sleep_until` in the test file
- `test_gcs_membership_multinode` remains the documented expected red until 04-18; it was excluded from the full-suite run and is unaffected by these rank changes

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- VERIFICATION truth 29's role-model clauses can be re-verified as PASS: no Super Admin grant, no unset role, and no non-creator author acting above their rank.
- The creator's peer-Super-Admin demotion still reaches the dormant path and activates through `Approve`.
- 04-18 can proceed with the real-signer multinode proof; this plan changed no FFI or fixture surface.

---

*Phase: 04-membership-invites*
*Completed: 2026-10-19*

## Self-Check: PASSED

- FOUND: `.planning/workstreams/app/phases/04-membership-invites/04-17-SUMMARY.md`
- FOUND: `dfef1a0` (Task 1)
- FOUND: `372d3e4` (Task 2)
- FOUND: `src/lib/gcs_membership.cpp`, `src/lib/gcs_membership.hpp`, `test/test_gcs_membership.cpp`
