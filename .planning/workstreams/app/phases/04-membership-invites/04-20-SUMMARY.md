---
phase: 04-membership-invites
plan: 20
subsystem: membership-security
tags: [signatures, authority-chain, transition-matrix, predecessor-cache, regression-tests, crdt]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: signed member authority, verified read/write paths, and the creator/outrank/approval transition matrix (04-19)
provides:
  - One test-side canonical member issuer/leave payload and hex signature helper
  - Signed SeedMember and signed OnMemberArrived delivery records
  - Fifteen trust-chain/authority-matrix regression tests pinning exclusion and authority preservation
  - Verified FFI suite against the signed-record contract with no fixture changes
affects: [04-21, 04-22]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Test-side invariant 11: one shared payload builder and one hex signature helper, never copied into a test"
    - "Re-announcing member bytes on the production replication topic makes the CRDT create a new DAG node instead of deduplicating identical content"
    - "Exclusion tests assert both MembersFor absence and raw-store presence"

key-files:
  created:
    - test/test_gcs_member_helpers.hpp
  modified:
    - test/test_gcs_membership.cpp

key-decisions:
  - "The shared test helper mirrors MemberIssuerPayload/MemberLeavePayload so every seeded record is byte-identical to what the production verifier signs and reads"
  - "SeedMember signs as the creator by default; tombstones carry prior_role == role and fresh grants carry ROLE_UNSPECIFIED"
  - "Seed and hostile writes use the 3-arg Put with kMembershipTopic because a 2-arg re-write of previously written bytes produces the same CRDT node CID and is silently deduplicated"
  - "The forged creator-root case pins fail-closed GUEST for every creator-rooted chain rather than the drafted MODERATOR: the verifier re-reads the root, so a forged root grants nobody authority, including its author"
  - "The FFI suite needed no fixture or expectation changes"

patterns-established:
  - "Save/hostile/assert-absent/restore/assert-restored LWW tests keep the rejected bytes in the CRDT while proving authority is unchanged"
  - "Per-instance predecessor-cache tests construct the target-bound Membership before the first target read and reuse it"

requirements-completed: [MEMB-02, MEMB-03, MEMB-04, MEMB-05]

# Metrics
duration: ~20min
completed: 2026-10-20
---

# Phase 4 Plan 20: Signed Member Test Contract Summary

**The membership unit suite now models only trust-chain-constructible records, with signed seeds and arrivals plus fifteen regression tests that pin self-signed caps, tampering, creator-target rejection, strict outrank, replay rejection, authority-preserving leave/redeem, and the write-once creator root.**

## Performance

- **Duration:** ~20 min
- **Started:** 2026-10-20T00:12:00Z
- **Completed:** 2026-10-20T00:32:03Z
- **Tasks:** 3
- **Files modified:** 2

## Accomplishments

- Added `test/test_gcs_member_helpers.hpp`, the single test-side implementation of the canonical member issuer payload, subject-leave payload, and their hex-encoded seam signature tokens.
- Signed `SeedMember` and both `OnMemberArrived` delivery records (live and tombstone) so the suite exercises the same authority bytes production verifies.
- Replaced the pre-signing expectations: the disabled-seam `OnScopeCreated` now fails closed; the peer-SUPER_ADMIN cases assert unverifiability and exclusion rather than a permissive demotion path; the dormancy case uses an ADMIN target.
- Added fifteen named regression tests covering self-signed caps, tampered role/timestamp, insufficient signer rank, creator-target rejection, peer-admin tombstone/demotion, peer-moderator demotion and lying predecessor, replay of a pre-demotion record, flipped explicit leave in both directions, authority-preserving leave/redeem (including a cold node), pre-tombstone replay, and the write-once creator root.
- `test_gcs_membership`: 46/46 green (31 existing + 15 new); `test_gcs_ffi`: green with no fixture changes.

## Task Commits

Each task was committed atomically:

1. **Task 1: Sign every membership seed and arrival record** - `7df2829` (feat)
2. **Task 2: Pin the signed member trust chain with fifteen regression tests** - `3a8c0fc` (test)
3. **Task 3: Run the FFI suite against the signed-record contract** - no code changes required (no commit)

**Plan metadata:** pending final docs commit (SUMMARY.md, STATE.md, ROADMAP.md)

## Files Created/Modified

- `test/test_gcs_member_helpers.hpp` - The one test-side `BuildMemberIssuerPayload`, `BuildMemberLeavePayload`, `TestMemberSignatureHex`, and `TestLeaveSignatureHex`.
- `test/test_gcs_membership.cpp` - Signed `SeedMember`/arrival records, rewritten pre-signing expectations, `PutMemberBytes`, and the fifteen new trust-chain tests.

## Decisions Made

- The canonical payload builder clears only the self-signature, subject leave signature, bearer room-key material, and subject-owned leave flag, exactly mirroring `MemberIssuerPayload`; no test copies the builder.
- `SeedMember` defaults its signer to the creator. A tombstone sets `prior_role == role`; a fresh grant sets `ROLE_UNSPECIFIED`. That is exactly the set of records the production matrix accepts as creator direct grants/tombstones.
- Member seeds and hostile/restored writes use `PutMemberBytes` (3-arg `Put` on `kMembershipTopic`). The 2-arg `Put` write of a byte-identical record produces the same CRDT DAG node (same key/value/priority/no links) and is deduplicated, so restoring saved bytes was a silent no-op; the replication topic gives the write a tracked head and a distinct node.
- The forged creator-root test asserts `LocalRoleFor == ROLE_GUEST`. Production re-reads the creator root while verifying any third-party record, so the forged root makes every creator-rooted chain unverifiable. This is stronger than the plan's drafted "attacker still resolves MODERATOR" and matches the fail-closed design.
- Exclusion cases assert the record is gone from `MembersFor` while the raw bytes remain the winning value, so rejection never becomes rollback.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Restoring/replaying saved member bytes was deduplicated by the CRDT**
- **Found during:** Task 2 (trust-chain regression tests)
- **Issue:** `SeedMember` wrote through the 2-arg `Put`. Re-writing the exact saved bytes later produced the same CRDT DAG node (same key/value/priority and no topic links), so the datastore kept the hostile value and the restore/replay assertions failed even though `Put` returned success.
- **Fix:** Added `PutMemberBytes`, which uses the 3-arg `Put` on `kMembershipTopic` exactly as production authority writes do, and routed seeds plus all hostile/restored writes through it. The tracked head makes a re-announce a distinct DAG node.
- **Files modified:** `test/test_gcs_membership.cpp`
- **Verification:** All 46 tests pass, including every save/hostile/restore/replay case; `ctest -R "^test_gcs_membership$"` passes.
- **Committed in:** `3a8c0fc` (Task 2 commit)

**2. [Rule 1 - Test expectation] Forged creator root fails closed for the attacker too**
- **Found during:** Task 2 (creator-target test)
- **Issue:** The plan expected the attacker (a creator-signed MODERATOR) to still resolve MODERATOR after the creator key was overwritten with an unverifiable record. Production's verifier re-reads the creator root for every third-party record, so the attacker's own chain is unverifiable too.
- **Fix:** The test now asserts `LocalRoleFor == ROLE_GUEST` and documents that a forged root grants nobody authority, including its author. The creator-target commands still fail.
- **Files modified:** `test/test_gcs_membership.cpp`
- **Verification:** Test passes; no production code was changed.
- **Committed in:** `3a8c0fc` (Task 2 commit)

---

**Total deviations:** 2 auto-fixed (1 blocking test-infrastructure fix, 1 test-expectation correction)
**Impact on plan:** Both changes are confined to the test file. No production behavior changed; the pinned behavior is the fail-closed contract 04-19 specified.

## Issues Encountered

- The CRDT write path required investigation: `CrdtDatastore::PutKey` builds a DAG node from the key/value plus a height derived from tracked topic heads. With no destination topic there is no head to track, so identical content maps to the same node CID and is never re-merged. Seeding on the membership topic removed the ambiguity.
- The existing `test_gcs_membership.cpp` is not clang-format clean at the plan base (1179 violation lines). Reformatting it would violate the minimal-change scope, so it was not applied; the new helper header is clean. This mirrors the documented 04-19 precedent.

## Threat Flags

None — this plan changes tests only; no new network, auth, file-access, or schema surface was introduced.

## Known Stubs

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- 04-21 can rely on the pending-REMOVE/dormancy regression coverage and on the signed seed/arrival helpers when it fixes `Approve`'s REMOVE branch.
- 04-22 can mirror the same signed records to the cold node and reuse the trust-chain expectations for wire-level forged-record tests.
- The unit suite is fully green; the multinode suite remains owned by 04-22.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-20*

## Self-Check: PASSED

- Task commits exist: `7df2829` (Task 1) and `3a8c0fc` (Task 2).
- `test/test_gcs_member_helpers.hpp` and `test/test_gcs_membership.cpp` exist on disk.
- `test_gcs_membership` ran 46/46 green; `test_gcs_ffi` passed.
