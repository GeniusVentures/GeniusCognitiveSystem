---
phase: 04-membership-invites
plan: 23
subsystem: membership-security
tags: [scope-binding, raw-read, authority-preservation, cross-scope, regression, cr-01, truth-29]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: signed member authority and the raw/preserving write split (04-19), the signed test seed contract (04-20), the coherent pending/tombstone commands (04-21), and the wire-level exclusion contract (04-22)
provides:
  - Scope binding at both authority-preserving raw-read call sites (RedeemInvite, Leave)
  - A documented non-binding raw-read contract on TryReadRawMember with every caller accounted for
  - A byte-level regression proving no cross-scope propagation by leave or redeem
  - Green full C++ suite (13/13), including all 9 multinode membership cases
affects: []

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "A raw (unverified) read is bound to the requested scope by its caller before any write key is derived from it"
    - "A rejection between the address check and the tombstone check keeps the authority-preserving path's ordering explicit"
    - "Regression non-vacuity comes from making the hostile bytes differ from the protected baseline in role and timestamp"

key-files:
  created: []
  modified:
    - src/lib/gcs_membership.cpp
    - src/lib/gcs_membership.hpp
    - test/test_gcs_membership.cpp

key-decisions:
  - "The scope check lives at the two propagation call sites, not inside TryReadRawMember — a helper-level rejection is indistinguishable from 'absent' and would make RedeemInvite take the fresh-record branch and succeed"
  - "The raw-read helper's behavior and signature are unchanged; its Doxygen now states the non-binding contract and why each current caller is safe"
  - "RedeemInvite's new rejection sits after the address check and before the tombstone check; Leave's sits after the combined liveness/address rejection and before set_explicit_leave"

patterns-established:
  - "Bind a raw record's body to the key it was read from before deriving any write key from it (CR-01)"
  - "Account for every raw-reader call site in the helper's contract, rather than adding a tri-state return"

requirements-completed: [MEMB-02, MEMB-04]

# Metrics
duration: 14min
completed: 2026-10-21
---

# Phase 4 Plan 23: Cross-Scope Raw Record Binding Summary

**RedeemInvite and Leave now reject a raw member record whose body names another scope, so an honest node can no longer launder unverified bytes into another scope's member key or creator root**

## Performance

- **Duration:** 14 min
- **Started:** 2026-10-21T17:31:00Z
- **Completed:** 2026-10-21T17:45:00Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments

- `RedeemInvite` rejects a raw record whose body names another scope, positioned after the address-mismatch check and before the tombstone check, so the authority-preserving merge/write never derives its Put key from an unverified body.
- `Leave` rejects the same shape after the combined liveness/address rejection and before `set_explicit_leave`, signing, and any Put — a leave can no longer rewrite another scope's member record or creator root.
- `TryReadRawMember`'s contract is documented: it returns whatever sits at the key and does not bind the body to it, with every current caller accounted for (VerifiedRoleFor/TryReadMemberVerified bound by VerifyMemberRecord; OnScopeCreated deliberately treats any occupying record as final; RedeemInvite/Leave bind explicitly).
- A byte-level regression plants a validly signed B-bodied record under A/alice, proves `Leave(A)` and `RedeemInvite(A)` both fail, and proves `gcs/members/B/alice` is byte-identical afterwards while A/alice keeps the hostile bytes.
- Verification truth 29's remaining clause is closed; MEMB-02/MEMB-04 behavior is unchanged (full suite green).

## Task Commits

Each task was committed atomically:

1. **Task 1: Bind the authority-preserving write key to the requested scope** - `36a8fd9` (fix)
2. **Task 2: Add the cross-scope raw-record regression** - `4cec74d` (test)

**Planning metadata:** committed separately after this summary and the STATE/ROADMAP updates.

## Files Created/Modified

- `src/lib/gcs_membership.cpp` - Two scope bindings: `existing.scope_id() != urlScopeId` in RedeemInvite and `record.scope_id() != scopeId` in Leave, each fail-closed with a WHY comment and one warn log.
- `src/lib/gcs_membership.hpp` - `TryReadRawMember` Doxygen states the non-binding raw contract and carries a `@warning` naming each caller's binding strategy. Declaration unchanged.
- `test/test_gcs_membership.cpp` - `CrossScopeRawRecordIsNotPropagatedByLeaveOrRedeem` (test #52), reusing `TestMemberSignatureHex`, `PutMemberBytes`, and `ReadMemberRecord`.

## Decisions Made

- Kept the check at the two call sites rather than inside `TryReadRawMember`. A helper-level `false` is indistinguishable from "no record": redeem would take the fresh-record branch, write a Guest record at A/key, and succeed — the verifier explicitly required redeem to fail. `OnScopeCreated`'s write-once root would also be silently reversed. This is the same reasoning recorded in the plan's call-site table.
- No new helper, enum, or abstraction for two comparisons; no change to `WriteMemberPreservingAuthority` or to `VerifyMemberRecord`'s existing body/key clause.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- **`clang-format --dry-run --Werror` cannot pass with the installed tool (pre-existing).** The only `clang-format` on PATH is 22.1.7, while the repository files are formatted in an older style (the file is not namespace-indented). The check fails on the untouched HEAD revision of `src/lib/gcs_membership.cpp` as well (verified against `git show HEAD:...`), and it flags essentially every line of the file, not the edited regions. Reformatting was rejected because it would produce a whole-file diff unrelated to this plan and contradict the "change nothing else / do not reformat untouched regions" instruction. The added lines follow the surrounding file's established style. This affects all three edited files equally and is not specific to 04-23.
- No other issues. The plan's `ninja` and `ctest` gates ran against the existing `build/OSX/Debug` tree with no reconfiguration.

## Verification

- `ninja -C build/OSX/Debug gcs_core test_gcs_membership test_gcs_membership_multinode` — clean, no new warnings.
- `ctest --test-dir build/OSX/Debug -R "^test_gcs_membership(_multinode)?$" --output-on-failure` — 2/2 passed (single-node 52 cases, multinode 9 cases).
- `ctest --test-dir build/OSX/Debug --output-on-failure` — 13/13 passed (full C++ suite, ~200 s).
- Targeted `--gtest_filter='*CrossScopeRawRecord*'` — 1 test ran, passed.
- grep gates: one `existing.scope_id() != urlScopeId` (line 1602, inside RedeemInvite between the address and tombstone checks), two `record.scope_id() != scopeId` (the pre-existing `VerifyMemberRecord` clause at 528 plus Leave at 1961), one occurrence of the new test name.
- No `sleep_for`/`sleep_until`; no OS preprocessor guards; no raw stdio diagnostics.

## Next Phase Readiness

- The last known Phase 4 blocker (VERIFICATION truth 29 / CR-01) is closed at the code level. The next step is a re-run of `/gsd:verify-work` for phase 04 to confirm the gap is cleared.
- Still intentionally deferred (user decision 2026-10-20): the wire-level admin-immediate-tombstone dormancy divergence (`VerifyMemberRecord` ~698-725 vs command path ~1879-1884). It was not touched here and needs its own decision.
- Accepted residual (unchanged): an attacker can still occupy A/alice with a hostile record and make that wallet's leave/redeem fail on the permissionless membership topic (per-key LWW, exclusion not rollback, 04-22). The fix removes the strictly worse cross-scope corruption.

## Self-Check: PASSED

- FOUND: src/lib/gcs_membership.cpp
- FOUND: src/lib/gcs_membership.hpp
- FOUND: test/test_gcs_membership.cpp
- FOUND: 36a8fd9 (Task 1)
- FOUND: 4cec74d (Task 2)

---
*Phase: 04-membership-invites*
*Completed: 2026-10-21*
