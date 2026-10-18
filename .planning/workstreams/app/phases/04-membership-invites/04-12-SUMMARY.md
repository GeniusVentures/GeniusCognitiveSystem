---
phase: 04-membership-invites
plan: 12
subsystem: membership-security
tags: [multinode, crdt, gossipsub, membership, invites, bearer-gate, convergence, wr-04, wr-05, wr-06]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: "04-09 single kMembershipTopic replication, every-invite bearer gate, tombstone-arrival roster push; 04-11 production topic registration and seam parity"
provides:
  - "Space-invite end-to-end convergence on a node that never joined the space (WR-06 network proof)"
  - "Wire-level every-invite bearer-gate proof: wrong key and forged/no-authenticator records write no member record (WR-04)"
  - "Remote-removal convergence asserted on the pushed MemberList sink, not only the local CRDT scan (WR-05)"
affects: [wave-6 verification, 04-VERIFICATION]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Negative convergence assertions compare against a baseline captured AFTER the positive sub-case"
    - "Forged records are written through session->Put on the production topic, never through a test-only mint hook"
    - "Push-plane assertions drain the captured sink before acting so a pre-action push cannot satisfy the wait"

key-files:
  created: []
  modified:
    - test/test_gcs_membership_multinode.cpp

key-decisions:
  - "The keyless sub-case uses a syntactically valid base64url key with different bytes so the URL parses and the rejection comes from authenticator verification, not URL validation"
  - "The post-baseline invariant is 'record count/bytes unchanged', never 'only the creator' — the authentic redeem legitimately adds B's record"
  - "Removal convergence is asserted through the newest pushed member_list event after draining B's sink, pinning OnMemberArrived's push rather than MembersFor's local scan"
  - "No case registers a topic; a case needing one would evidence an incomplete 04-09 fixture"

patterns-established:
  - "Baseline-capture pattern for negative wire assertions (members, record count, own-record bytes)"
  - "Locked latest-push snapshot helper (LatestMemberListFor) that releases the sink mutex before any wait"

requirements-completed: [MEMB-01, MEMB-02, MEMB-03, MEMB-04]

# Metrics
duration: 6min
completed: 2026-10-18
---

# Phase 4 Plan 12: Network-Boundary Membership Convergence Proofs Summary

**Three multinode proofs on the membership replication topic alone: a space invite reaches an unjoined recipient, a forged/keyless redeem writes no member record, and a remote removal arrives as a pushed roster**

## Performance

- **Duration:** ~6 min
- **Started:** 2026-10-18T21:26:32Z
- **Completed:** 2026-10-18T21:32:19Z
- **Tasks:** 3
- **Files modified:** 1

## Accomplishments

- WR-06 proven at the network boundary: `SpaceInviteConvergesWithoutTheRecipientJoiningTheSpace` creates a space on A, mints a `gcs://invite/space/` URL, and B — which registered nothing derived from the space — receives both the invite and creator member records on `Membership::kMembershipTopic`, redeems, and converges on a 2-member roster with B as Guest.
- WR-04 proven on the wire: `ForgedOrKeylessSpaceInviteWritesNoMemberRecord` establishes an authentic childless-space baseline (non-empty authenticator, zero wrapped room keys), captures the post-redeem member baseline, then rejects a wrong-key URL and a forged record with a cleared authenticator written through `session->Put` on the membership topic — neither changes the member-record count nor B's serialized record bytes.
- WR-05 proven at the push boundary: `RemovedMemberReceivesTheConvergedRoster` drains B's sink, removes B on A, and waits for B's newest pushed `MemberList` to exclude B while still carrying A.
- `test_gcs_membership_multinode` grew from 2 to 5 cases; it passed 3 consecutive runs after each task, and the full C++ suite passes 13/13.

## Task Commits

Each task was committed atomically:

1. **Task 1: Space invite converges on a recipient that never joined the space (WR-06)** - `91a5433` (test)
2. **Task 2: The bearer key gates every redeem on the wire (WR-04)** - `2112209` (test)
3. **Task 3: Remote removal converges as a pushed MemberList (WR-05)** - `4efad25` (test)

**Follow-up:** `6999667` (style) keeps the new invite-key constant on one line per `clang-format`.
**Plan metadata:** (docs commit created after this summary)

## Files Created/Modified

- `test/test_gcs_membership_multinode.cpp` - Adds the three convergence cases plus `LatestMemberListFor`; adds `<optional>` and `<unordered_set>` includes and the well-formed invite-key constant. No production, proto, or fixture change.

## Decisions Made

- The keyless sub-case presents a same-shape, different-bytes base64url key so the URL parses cleanly and the rejection is attributable to authenticator verification.
- Negative assertions compare against a post-sub-case-1 baseline (2 member records, B's own record bytes) rather than asserting "only the creator", which would be false by design.
- The forged invite record is a copy of the replicated authentic record with `clear_authenticator()`, written under a distinct invite id via `session->Put(..., { kMembershipTopic })`; no test-only mint helper was added.
- The removal case drains B's captured events under `sinkMutex` before calling `RemoveMember`, so only the post-removal arrival push can satisfy the wait.

## Deviations from Plan

None - plan executed exactly as written. No fixture, source, proto, or FFI change was needed.

## Issues Encountered

- **clang-format environment mismatch (known):** Homebrew `clang-format 22.1.7` reports 72 violations on the untouched baseline of this file and 116 after the additions, so `--dry-run --Werror` is not a usable gate here (same mismatch documented by 04-08/04-09/04-11). The one complaint in newly added code (the constant's line wrap) was applied; the remaining reports are the formatter disagreeing with the repository's existing Allman/continuation style.
- **clang-tidy:** no repository-root `.clang-tidy` config applies (the only one lives inside the untouched `GNUS-NEO-SWARM` submodule), so it was not run as a gate.
- **Regression-pin inspection:** the "fails against the pre-fix path" acceptance criteria (WR-06 per-scope topic, WR-04 skip-gate-when-no-keys, WR-05 pre-fix early return) were reasoned from the current implementation rather than exercised by reverting source, which is outside the test-only scope of this plan.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- All three verification gaps that required network-boundary evidence now have executable proofs: WR-06, WR-04 (including the zero-key-material space invite), and WR-05.
- The multinode fixture never registers a scope-derived topic; the membership topic is the sole replication channel, matching production init wiring from 04-11.
- 04-VERIFICATION can cite `test_gcs_membership_multinode` (5 cases) for these truths.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-18*

## Self-Check: PASSED

- All three task commits (`91a5433`, `2112209`, `4efad25`) and the style follow-up (`6999667`) exist in the repository history.
- `test/test_gcs_membership_multinode.cpp` lists 5 cases, including the three added by this plan.
- Full C++ suite: 13/13 tests passed.
