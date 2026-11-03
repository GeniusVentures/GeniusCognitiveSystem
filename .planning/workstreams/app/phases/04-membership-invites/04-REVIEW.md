---
phase: 04-membership-invites
reviewed: 2026-10-21T17:55:39Z
depth: standard
files_reviewed: 3
files_reviewed_list:
  - src/lib/gcs_membership.cpp
  - src/lib/gcs_membership.hpp
  - test/test_gcs_membership.cpp
findings:
  critical: 0
  warning: 1
  info: 3
  total: 4
status: issues_found
---

# Phase 04: Code Review Report (scoped re-review, plan 04-23)

**Reviewed:** 2026-10-21T17:55:39Z
**Depth:** standard
**Files Reviewed:** 3
**Status:** issues_found

## Summary

This is the post-execution scoped review for gap-closure plan 04-23 (commits 36a8fd9, 4cec74d), covering exactly the three changed files. The full-phase review (2026-10-19, 32 files) is superseded by this report for these three files; its findings for other files remain as recorded in git history (commit 6486295).

**The gap-closure delta is correct and complete.** Verified against the 04-23 plan and the prior review's CR-01:

- `RedeemInvite` (`src/lib/gcs_membership.cpp:1597-1609`) rejects a raw record whose body names another scope, placed after the address check (1590) and before the tombstone check (1610) — exactly as the commit message and plan claim. No mutation, unwrap, or write precedes the rejection.
- `Leave` (`src/lib/gcs_membership.cpp:1955-1967`) rejects the same shape before `set_explicit_leave`, before the subject signature, and before the preserving write. The leave payload now signs the same scope the write targets.
- The `TryReadRawMember` Doxygen `@warning` (`src/lib/gcs_membership.hpp:651-659`) is accurate against every call site: `VerifiedRoleFor` (cpp:830) and `TryReadMemberVerified` (cpp:847) are bound by `VerifyMemberRecord`'s body/key clause (cpp:528-529); `OnScopeCreated` (cpp:2283-2298) treats any occupying record as final; `RedeemInvite` and `Leave` now bind explicitly. No remaining caller feeds `WriteMemberPreservingAuthority` an unbound body, and `WriteMemberAsAuthor`'s callers all construct the record from bound inputs — the laundering primitive described in prior CR-01 is closed.
- The regression `CrossScopeRawRecordIsNotPropagatedByLeaveOrRedeem` (`test/test_gcs_membership.cpp:1390-1478`) is a genuine regression test, not a tautology: the hostile record differs from B's baseline in role, timestamps, and signature, so a propagated write would observably change B's bytes; it asserts both operations fail, both scopes' bytes are unchanged, and A's roster excludes the joiner. Positive controls against over-rejection already exist in the suite (`LeavePreservesPromotedRoleAuthority`, `ColdNodeLeavePreservesAuthorityWithoutLocalScope`, `RedeemOfExistingPromotedMemberPreservesAuthority`, `DormantRemovalSurvivesRedeem`). The test helper `TestMemberSignatureHex` mirrors the production canonical payload exactly (`test/test_gcs_member_helpers.hpp:43-91`), with `SignedSeedIsAdmitted` as the drift tripwire.

No Critical findings. One Warning and three Info items follow, none of which block the 04-23 change.

## Warnings

### WR-01: Unbounded signed timestamps — `updated_at_ms + 1` is UB at INT64_MAX and a verified INT64_MAX stamp permanently bricks a member key

**File:** `src/lib/gcs_membership.cpp:957-960` (with `:579-589`)
**Issue:** `WriteMemberAsAuthor` stamps `updated_at_ms = max(NowMs(), verifiedExisting.updated_at_ms() + 1)`. Nothing in `VerifyMemberRecord` bounds `updated_at_ms`, so a record signed by any authority-holding wallet can carry `updated_at_ms == INT64_MAX` and still verify. Two consequences:

1. The `+ 1` is signed-integer overflow — undefined behavior in C++ (with two's-complement wrap it becomes `INT64_MIN`, silently inverting the monotonic stamping intent).
2. Once a node verifies the INT64_MAX record, its predecessor gate (`record.updated_at_ms() <= cached.updatedAtMs` at `:581`) rejects every future write on that member key: a new signature can never be strictly newer than INT64_MAX. The key is frozen for all authority writes on every warmed node.

The file's own threat model treats signature-valid-but-hostile writers as in scope (see the D-05 GUEST-cap rationale at `:1647-1660`, defending against exactly this class), so "requires a verified signer" is not a sufficient dismissal here.

**Fix:** Bound the timestamp at verification time — reject a record whose `updated_at_ms` is implausible (e.g., greater than `NowMs() + kMaxClockSkewMs`), or clamp the increment:

```cpp
record.set_updated_at_ms( hasVerified
                              ? std::max( NowMs(),
                                          verifiedExisting.updated_at_ms()
                                              >= std::numeric_limits<int64_t>::max() - 1
                                                  ? verifiedExisting.updated_at_ms()
                                                  : verifiedExisting.updated_at_ms() + 1 )
                              : NowMs() );
```

The verification-side bound is preferred (fail closed at the read gate); the clamp alone still leaves the predecessor gate frozen.

## Info

### IN-01: Unreachable re-check of `record.created_by() == address` in the third-party branch

**File:** `src/lib/gcs_membership.cpp:641-645`
**Issue:** The condition `depth >= kMaxMemberChainDepth || record.created_by() == address` sits inside the `else` branch that is only entered when `record.created_by() != address` (branch at `:592`). The second disjunct can never be true — dead code in a security-critical verifier.
**Fix:** Remove the redundant disjunct (keep the depth check), or comment it as an explicitly re-asserted invariant so a future reader does not mistake it for a live guard.

### IN-02: Scope-id validation is inconsistent at the API edge

**File:** `src/lib/gcs_membership.cpp:1479-1485` (validated) vs `:1923`, `:1048-1051`, `:1103`, `:1799`, `:1862` (unvalidated)
**Issue:** `RedeemInvite` rejects a `urlScopeId` containing `/` (`:1481`), but `Leave`, `MembersFor`, `ChangeRole`, `RemoveMember`, and `HasExplicitLeave` accept arbitrary scope-id strings. Because `MemberKey` concatenates with `/`, a scope id like `"a/b"` silently addresses the key namespace of scope `a` with wallet `"b/<address>"`. The consequence is contained today — body/key binding (`:528-529`), the address check, and catalog resolution all fail closed, and forging a useful confused-coordinate record requires a signer to have signed those exact coordinates — but the edge validation should be uniform (project rule: fail fast, validate at edges).
**Fix:** Extract the `/`-free non-empty scope-id shape check `RedeemInvite` already performs into a small shared guard and apply it at the public entry points.

### IN-03: The new `TryReadRawMember` contract relies on caller discipline only

**File:** `src/lib/gcs_membership.hpp:651-659`
**Issue:** The `@warning` added by 36a8fd9 correctly documents that callers deriving authorization or a write key must bind the body to the requested scope themselves, and all five current call sites comply. Nothing structural, however, prevents a future caller from repeating the exact CR-01 mistake — the mistake is only prevented by prose. The mirror of the payload builder in `test/test_gcs_member_helpers.hpp` is an accepted, documented duplicate (invariant 11 argument in that header) with a positive-control tripwire, so it is not re-raised here.
**Fix (suggestion only, YAGNI governs):** if a third raw-read write path ever appears, add a bound variant (e.g., `TryReadRawMemberBound` checking `scope_id`/`address` against the key) and reserve the unbound reader for the cold-node authority-preservation case by name. Do not add it speculatively now.

## Preserved backlog — full-phase review (2026-10-19, commit 6486295)

The scoped re-review above supersedes the full-phase report only for the three 04-23 files. The full-phase findings for all other files remain open and are preserved here so the live REVIEW.md stays the single ledger. Full detail for each item: `git show 6486295:.planning/workstreams/app/phases/04-membership-invites/04-REVIEW.md`. The two passes use independent ID namespaces — cite prior-pass items as "prior WR-11" etc. Note: new IN-01 above is the same finding as prior IN-06 (do not double-count in a `--fix` pass).

**Resolved:**

| ID | Finding | Status |
|----|---------|--------|
| prior CR-01 | `RedeemInvite`/`Leave` take the write key from an unverified record body (cross-scope laundering) | **RESOLVED by 04-23** (`36a8fd9`, `4cec74d`); closure confirmed by this re-review |

**Open — Warnings (11):**

| ID | Finding | Primary file(s) |
|----|---------|-----------------|
| prior WR-01 | Membership changes never refresh the derived-join projection | `src/ffi/gcs_core_ffi.cpp:799-879`, `:1762-1764`, `:1818-1844` |
| prior WR-06 | Invite bearer gate is optional; wrong-key redemption still writes a member record | `src/lib/gcs_membership.cpp:1400-1417`, `:1566-1582`; `src/ffi/gcs_core_ffi.cpp:1187-1188` |
| prior WR-07 | `ScopeHasRoster` fails open on a scan error | `src/ffi/gcs_core_ffi.cpp:752-756` |
| prior WR-08 | A failed member scan is pushed as an empty roster | `src/ffi/gcs_core_ffi.cpp:596-612`; `src/app/lib/cubits/members_cubit.dart:204-219` |
| prior WR-09 | Members dialog offers "change role" on the viewer's own row | `src/app/lib/shell/members_dialog.dart:318-319` |
| prior WR-10 | Deleting a space does not cascade to its rooms; a nested room cannot be deleted alone | `src/lib/gcs_membership.cpp:1960-2034`, `:1249-1253`; `src/app/lib/cubits/rail_cubit.dart:196-211` |
| prior WR-11 | `update_space` silently cancels a dormant deletion (D-07 coherence gap) | `src/ffi/gcs_core_ffi.cpp:1681-1687`; `src/lib/gcs_entity_store.cpp:293-350` |
| prior WR-12 | Failed `gcs_init` paths leave the embedded SDK booted and smoke topics latched in `g_roomTopics` | `src/ffi/gcs_core_ffi.cpp:985`, `:997-1002`, `:1015-1022` |
| prior WR-13 | Scope creation is not transactional — a failed creator root leaves an orphan scope | `src/ffi/gcs_core_ffi.cpp:1523-1555`, `:1595-1643`; `src/lib/gcs_membership.cpp:2257-2271` |
| prior WR-14 | `MintInvite` wraps a topic-derived key for child rooms the minter does not hold; `RedeemInvite` overwrites the redeemer's real key | `src/lib/gcs_membership.cpp:1314-1338`, `:1738`; `src/lib/gcs_crypto.cpp:222-238` |
| prior WR-15 | Explicit leave is irreversible — a re-invited member is silently left out of the derived join | `src/lib/gcs_membership.cpp:1942-1952`, `:1752-1770`, `:494-497`, `:1116-1120` |

**Open — Info (7):**

| ID | Finding | Primary file(s) |
|----|---------|-----------------|
| prior IN-01 | Stale `kRedeem` comment | `src/ffi/gcs_core_ffi.cpp:1740-1743` |
| prior IN-02 | Unnamed numeric literals in `PrivilegeRank` | `src/lib/gcs_membership.cpp:98-113` |
| prior IN-03 | All membership failures map to one error | `src/ffi/gcs_core_ffi.cpp:1786`, `:1812`, `:1838`, `:1864`, `:1891` |
| prior IN-04 | Every membership mutation pushes `MemberList` twice | `src/lib/gcs_membership.cpp:1011-1046`; `src/ffi/gcs_core_ffi.cpp:1789`, `:1815`, `:1842`, `:1894` |
| prior IN-05 | `VerifyInviteAuthenticator` accepts any non-empty key length | `src/lib/gcs_crypto.cpp:515-521` |
| prior IN-06 | Dead/redundant clause in the third-party verification branch (= new IN-01 above) | `src/lib/gcs_membership.cpp:641-645` |
| prior IN-07 | Entity-level `pending_action` is never cleared on activation | `src/lib/gcs_membership.cpp:2168-2171`, `:2188-2191`, `:1992` |

---

_Reviewed: 2026-10-21T17:55:39Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
