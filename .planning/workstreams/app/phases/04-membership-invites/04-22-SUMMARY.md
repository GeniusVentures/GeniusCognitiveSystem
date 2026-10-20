---
phase: 04-membership-invites
plan: 22
subsystem: membership-security
tags: [real-crypto, wire-injection, cold-node, entity-catalog, crdt-convergence, multi-node, regression]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: signed member authority and the creator/outrank/approval verification matrix (04-19), the signed test seed/arrival contract (04-20), and coherent pending/tombstone command behavior (04-21)
provides:
  - A real-signer KeyWrapSeam factory shared by every multinode fixture
  - A cold-node entity-catalog mirror that pins fail-closed-before-metadata and re-verified admission after
  - A late-metadata convergence proof through OnMemberArrived
  - Wire-level regression coverage for a validly self-signed Super Admin and five hostile authority transitions
  - Green full C++ suite (13/13) and Flutter gate (80 passed, 1 skipped)
affects: []

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Real secp256k1 identities sign every wire-injected record; rejection is proven with genuine signatures, not malformed bytes"
    - "A cold node's catalog is mirrored from the warm node as fixture setup, modeling production's LoadFromStore-before-Membership ordering"
    - "Rejection means exclusion from verified rosters, never rollback of the raw CRDT value; recovery is proven by restoring the saved creator-signed record"

key-files:
  created: []
  modified:
    - test/test_gcs_membership_multinode.cpp
    - .planning/workstreams/app/phases/04-membership-invites/04-VALIDATION.md
    - .planning/workstreams/app/STATE.md

key-decisions:
  - "A cold node verifies nothing until the entity catalog resolves the scope; the fixture mirrors manifest + space/room records and reloads the catalog"
  - "Wire-injected authority is excluded, not rolled back: hostile bytes remain the winning raw value while rosters omit the target and all commands fail"
  - "The signer-bound Membership must warm its predecessor cache before the hostile publish; a never-synced instance trusts the signed prior_role (documented cold-start residual)"

patterns-established:
  - "MakeSeam(GeniusSigner&): one real-crypto seam used by every multinode membership fixture"
  - "MirrorScopesFromAToB(): explicit cold-node catalog synchronization before roster assertions"
  - "PublishSignedRecord + HostileCase: a reusable publish/assert/restore shape for wire-level authority attacks"

requirements-completed: [MEMB-02, MEMB-03, MEMB-04]

# Metrics
duration: ~25min
completed: 2026-10-19
---

# Phase 4 Plan 22: Cold-Node Catalog and Wire-Level Authority Summary

**The multinode membership suite is fully green: real-crypto seam extraction, cold-node catalog mirroring with late-metadata convergence, and wire regression proofs that validly signed hostile records grant no authority while remaining in the raw CRDT.**

## Performance

- **Duration:** ~25 min
- **Started:** 2026-10-19T17:35:00Z (approx.)
- **Completed:** 2026-10-19T18:05:00Z (approx.)
- **Tasks:** 3
- **Files modified:** 3

## Accomplishments

- Extracted `MakeSeam( sgns::GeniusSigner & )` as the single real-crypto seam factory and `MirrorScopesFromAToB()` as the explicit cold-node catalog mirror; the previously red convergence tests now resolve the creator on node B.
- Added `ColdNodeConvergesWhenEntityMetadataArrivesAfterMemberRecord`: membership is excluded before metadata arrives (target `ROLE_UNSPECIFIED`, local `ROLE_GUEST`, raw bytes present), admitted after the mirror (`ROLE_SUPER_ADMIN`), and the next `OnMemberArrived` pushes a converged roster.
- Added `WireInjectedSelfSignedSuperAdminIsNotAMember`: a genuine self-signed SUPER_ADMIN record is installed as the winning raw CRDT value but appears in neither roster, resolves to GUEST, and grants no DeleteScope/RemoveMember/ChangeRole; the genuine creator retains authority and a positive-control role change still converges.
- Added `WireInjectedAuthorityTransitionsAreRejected` with five genuine-signature hostile cases — creator overwrite, peer-admin tombstone, peer-admin pending demotion, honest peer-Moderator demotion, and a lying `prior_role` — each excluded from both rosters and each recovered by restoring the saved creator-signed record.
- Ran the full C++ suite (13/13 green, including `test_gcs_membership` 51/51 and `test_gcs_membership_multinode` 9/9) and the Flutter gate (`flutter test`: 80 passed, 1 skipped).
- Marked 04-22 T1-T3 green in `04-VALIDATION.md` and recorded the cleared cycle-3 reds.

## Task Commits

Each task was committed atomically:

1. **Task 1: Real-signer seam factory, cold-node catalog mirror, and late-metadata convergence** - `3f960b7` (feat)
2. **Task 2: Wire-injected self-signed authority and hostile transition regressions** - `862b49a` (feat)
3. **Task 3: Validation sign-off and state decisions** - `48f34f9` (docs)

**Plan metadata:** pending final docs commit (SUMMARY.md, STATE.md, ROADMAP.md)

## Files Created/Modified

- `test/test_gcs_membership_multinode.cpp` - `MakeSeam`, `MirrorScopesFromAToB`, the cold-node re-evaluation test, and the two wire-level attack tests (nine tests total).
- `.planning/workstreams/app/phases/04-membership-invites/04-VALIDATION.md` - 04-22 T1-T3 marked green; cycle-3 reds cleared with the recorded suite and Flutter results.
- `.planning/workstreams/app/STATE.md` - Two decisions covering the catalog-mirror/late-metadata contract and exclusion-not-rollback semantics.

## Decisions Made

- **Catalog before verification.** The cold node cannot resolve the creator without the scope entity, so the fixture mirrors the manifest and space/room records and reloads the catalog. Production already calls `LoadFromStore()` before constructing `Membership`; the mirror models that ordering rather than adding a sync service.
- **Exclusion, not rollback.** A rejected record is not deleted: per-key LWW installs the hostile bytes as the winning raw value. Correctness is asserted at the verified roster and command boundary, and recovery is proven by restoring the saved creator-signed record via a topic Put.
- **Warm the predecessor cache for the lying-predecessor case.** A freshly constructed `Membership` has no verified history and therefore trusts the signed `prior_role` — the documented cold-start residual. The test warms the signer-bound instance with `MembersFor(roomId)` before publishing so the live-node path is exercised; a never-synced node accepting the signed prior role remains the accepted residual (T-04-19-09).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Warmed the signer-bound Membership before the lying-prior_role publish**
- **Found during:** Task 2 (`WireInjectedAuthorityTransitionsAreRejected`)
- **Issue:** The first run constructed the attacker's `Membership` after publishing the hostile record; with an empty predecessor cache it trusted the signed `prior_role = GUEST` and accepted both the demotion and the removal, so the case-5 assertions failed and the restoration wait timed out.
- **Fix:** Construct `hostileMembership` before publishing and warm its cache with `MembersFor(roomId)`. This matches a live node with verified history; the cold-start trust of a signed `prior_role` remains the documented residual.
- **Files modified:** `test/test_gcs_membership_multinode.cpp`
- **Verification:** Case 5 rejects both commands and restoration converges; the full multinode suite is 9/9 green.
- **Committed in:** `862b49a` (Task 2 commit)

---

**Total deviations:** 1 auto-fixed (1 bug)
**Impact on plan:** The fix made the test exercise the intended live-node behavior instead of the known cold-start residual. No production code changed and no scope creep.

## Issues Encountered

- **Pre-existing clang-format divergence:** `clang-format --dry-run --Werror` reports violations across the test file at its plan base (thousands of diagnostic lines). Applying it would reformat the entire file against the minimal-change scope, so, following the documented 04-19/04-20/04-21 precedent, the file was not reformatted and no new formatting divergence was introduced.
- **Pre-existing Flutter analyze errors:** `flutter analyze` reports 50 errors, all confined to the vendored `scaffold/example/test/capture_images_test.dart`, which references demo packages that do not exist. These are outside this plan's files and pre-date it. `flutter test` is green.
- **Build/lint gates actually run:** `ninja -C build/OSX/Debug` (no work to do, clean), `ctest --test-dir build/OSX/Debug --output-on-failure` (13/13 passed), `flutter analyze`, and `flutter test` (80 passed, 1 skipped).

## Threat Flags

None — this plan changes tests and planning artifacts only; no new network, auth, file-access, or schema surface was introduced.

## Known Stubs

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Phase 04 is complete: the full C++ suite is green and the multinode membership suite no longer carries an expected red.
- The remaining cold-start rollback residual (a never-synced node trusts a signed `prior_role`) is documented in `STATE.md` and `04-VALIDATION.md`; it requires anti-rollback ordering beyond per-key LWW (D-01) and is not a phase-04 blocker.
- The Flutter analyzer's 50 pre-existing vendored-example errors should be triaged separately from the app package.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-19*

## Self-Check: PASSED

- Task commits exist: `3f960b7`, `862b49a`, `48f34f9`.
- All modified files and this SUMMARY exist on disk.
- `test_gcs_membership_multinode` ran 9/9 green; `test_gcs_membership` ran 51/51 green; full ctest ran 13/13 green.
