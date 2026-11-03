---
phase: 04-membership-invites
plan: 19
subsystem: membership-security
tags: [signatures, secp256k1, authority-chain, transition-matrix, predecessor-cache, crdt]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: invite issuer-signing seam and redeemed-member record baseline (04-15/04-16/04-18)
provides:
  - append-only MemberRecord authority fields created_by/record_signature/leave_signature/prior_role
  - canonical member authority payload and subject-leave payload
  - author-signed versus authority-preserving member write paths
  - verified read paths for every roster and authorization decision
  - creator/outrank/approval transition matrix with a node-local predecessor gate
affects: [04-20, 04-21, 04-22]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Canonical authority payload excludes self/signature/material/subject-owned fields"
    - "Author-signed write derives prior_role and a monotonic signed updated_at_ms in one place"
    - "Authority-preserving write keeps authority bytes and the signed timestamp verbatim"
    - "Node-local verified-predecessor cache as a fail-closed read gate, never a merge rule"

key-files:
  created: []
  modified:
    - src/proto/gcs_chat.proto
    - src/lib/gcs_membership.hpp
    - src/lib/gcs_membership.cpp

key-decisions:
  - "Member authority signs authority fields only; explicit_leave is a separate subject-signed attestation"
  - "prior_role is derived in exactly one helper from the verified pre-write record"
  - "updated_at_ms is stamped max(NowMs(), verified + 1) before signing so writes are strictly ordered"
  - "MembersFor verifies before the tombstone skip so a verified tombstone advances the cache"
  - "AdminCount counts only verified live records so a forged admin cannot change D-07 dormancy"
  - "The predecessor cache is in-memory/non-replicated; cold-start rollback residual is documented"

patterns-established:
  - "Fail-closed read gate: an unverifiable record is not a member and never influences authority decisions"
  - "Two write verbs: WriteMemberAsAuthor signs; WriteMemberPreservingAuthority never re-signs"

requirements-completed: [MEMB-01, MEMB-02, MEMB-03, MEMB-04, MEMB-05]

# Metrics
duration: ~20min active (session interrupted by a machine crash; resumed 2026-10-20T00:05Z)
completed: 2026-10-19
---

# Phase 4 Plan 19: Signed Member Authority Summary

**Member records are now signed authority artifacts: every write is signed, every read verifies, and a creator-rooted transition matrix rejects forged roles, unauthorized destruction, flipped leaves, and replayed predecessors.**

## Performance

- **Duration:** ~20 min active (wall clock spans a machine crash; Task 1 at 20:47Z, Task 3 completed 00:09Z)
- **Started:** 2026-10-19T20:47:35Z (Task 1 commit)
- **Completed:** 2026-10-20T00:09:00Z
- **Tasks:** 3
- **Files modified:** 3

## Accomplishments

- Appended `created_by`, `record_signature`, `leave_signature`, and `prior_role` to `MemberRecord` without touching fields 1-11.
- Added canonical authority and leave payloads plus sign/verify primitives; the canonical member payload clears only the self-signature, the subject leave signature, bearer room-key material, and the subject-owned leave flag.
- Replaced `WriteMember` with `WriteMemberAsAuthor` (derives `prior_role`, stamps a monotonic signed `updated_at_ms`, signs as the acting wallet, fails closed without the seam) and `WriteMemberPreservingAuthority` (never re-signs; keeps authority bytes and timestamp for leave and key refresh).
- Routed every read through `VerifyMemberRecord`: `TryReadMember`/`LocalRoleFor`/`HasExplicitLeave`, `MembersFor` (verify before tombstone skip), `AdminCount` (verified live only), and `OnMemberArrived` (no push on failure).
- Enforced the authority matrix: self-signed records capped at Guest (Super Admin only for the resolved creator), unconditional creator-target rejection for third-party records, strict outrank against the signed `prior_role`, a creator gate on every admin-tier tombstone/demotion/dormant request, and a node-local predecessor/staleness gate.
- Made the creator root write-once: a second `OnScopeCreated` returns success without rewriting a verified root and fails closed on an occupying unverifiable record.

## Task Commits

Each task was committed atomically:

1. **Task 1: Append member authority fields and add the canonical payload plus signature primitives** - `85b909b` (feat)
2. **Task 2: Sign every member write path; preserve authority on leave and key refresh** - `aba8166` (feat)
3. **Task 3: Verify every read path and enforce the creator/outrank/approval transition matrix** - `3319e80` (feat)

**Plan metadata:** pending final docs commit (SUMMARY.md, STATE.md, ROADMAP.md)

_Note: Task 3's core matrix landed inside `aba8166` before the crash; its read-path routing and Doxygen remained uncommitted and were audited, completed, and committed as `3319e80`._

## Files Created/Modified

- `src/proto/gcs_chat.proto` - Append-only `MemberRecord` fields 12-15 with mirrored comment style.
- `src/lib/gcs_membership.cpp` - Canonical payloads, signature/leave verification, predecessor gate, transition matrix, two write verbs, verified reads, write-once creator root.
- `src/lib/gcs_membership.hpp` - Trust-chain contract, chain-depth bound, verified-member state cache, and verified-read/write Doxygen.

## Decisions Made

- The authority signature covers authority fields only. `record_signature`, `leave_signature`, `wrapped_room_key`, and `explicit_leave` are excluded because they are respectively self-referential, subject-owned, bearer material, or a subject transition.
- `created_at_ms`, `updated_at_ms`, and `prior_role` are signed: immutable history, signed write ordering, and an authenticated predecessor claim.
- `prior_role` is assigned in exactly one function (`WriteMemberAsAuthor`) from the verified pre-write record; no caller can fabricate a predecessor.
- `updated_at_ms` is `max(NowMs(), verified + 1)` so two same-millisecond authority writes remain strictly ordered for the predecessor gate.
- Authority-preserving writes never re-sign and preserve the signed timestamp; verification there is advisory and warns on cold nodes rather than blocking a legitimate leave or redeem.
- `MembersFor` verifies before the tombstone skip so a verified tombstone advances the cache and a replay of the exact pre-tombstone bytes cannot pass the identical-signature shortcut.
- `AdminCount` counts only verified live records, preserving its fail-closed scan behavior while removing the forged-admin dormancy lever.
- The verified-predecessor cache is an in-memory, non-replicated read gate; a cold start trusts the signed `prior_role` and the residual rollback risk is documented (T-04-19-09).

## Deviations from Plan

None - plan executed exactly as written.

The in-flight, crash-interrupted Task 3 edits were audited line-by-line against the plan and matched the specification (wallet-from-key derivation, verify-before-tombstone-skip, verified-only admin count, verify-before-push arrival handling, and the verified `TryReadMember` wrapper). They were kept, header Doxygen was completed, and the whole set was committed as `3319e80`.

## Issues Encountered

- **Expected red unit suite:** `test_gcs_membership` compiles and runs 30 tests: 14 pass, 16 fail. The failures are the plan-documented expected state until 04-20 signs `SeedMember` and the `OnMemberArrived` arrival records; no test was modified here. `test_gcs_membership_multinode` compiles.
- **clang-format gate:** `clang-format --dry-run --Werror` is not clean on these files, but it is equally not clean at the plan base commit (`87ac25d`, 7535 violation lines). Applying it would reformat the entire file and violate the minimal-change scope, so it was not applied. No new formatting divergence was introduced by this plan beyond the existing file style.
- **clang-tidy gate:** no `.clang-tidy` exists at the GCS root (the only config in the tree belongs to the `GNUS-NEO-SWARM` submodule), so a project-configured clang-tidy run was not possible. The canonical Ninja build of `gcs_proto`, `gcs_core`, `gcs_ffi`, `test_gcs_membership`, and `test_gcs_membership_multinode` completed with no warnings.
- **Acceptance-grep form:** the pending-REMOVE tier clause is written as the negated predicate `signerRank < PrivilegeRank(ROLE_ADMIN)`, so the literal `grep "signerRank >= ADMIN"` is empty while the required `priorRank >= ADMIN || signerRank >= ADMIN` semantics are exact.
- **Deferred (owned by 04-21 T3):** `Approve`'s REMOVE branch still writes `deleted = true` while `pending_action = REMOVE`; the verifier correctly treats that combination as incoherent, so the approved tombstone is excluded (fail-closed) but does not advance the predecessor cache. The plan explicitly leaves those field assignments to 04-21.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- 04-20 can now re-sign `SeedMember` and the arrival records and assert the unit suite green; the trust-chain exclusion and authority-preservation tests can build on `VerifyMemberRecord`.
- 04-21 consumes the same pending-REMOVE predicate in `RemoveMember` and clears the pending fields on approval.
- 04-22 can mirror entity records to the cold node and prove wire-level forged Super Admin exclusion through the real-crypto seam.

---

*Phase: 04-membership-invites*
*Completed: 2026-10-19*

## Self-Check: PASSED

- All three task commits exist (`85b909b`, `aba8166`, `3319e80`).
- All modified source files and this SUMMARY exist on disk.
- ROADMAP reports 19/22 plans executed for Phase 4.
