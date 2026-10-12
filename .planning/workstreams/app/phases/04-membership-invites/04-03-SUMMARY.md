---
phase: 04-membership-invites
plan: 03
subsystem: membership
tags: [protobuf, crdt, membership, roles, invites, googletest, gcs-membership, multinode, gossipsub]

# Dependency graph
requires:
  - phase: 04-01
    provides: MemberRecord/InviteRecord proto messages + gcs/members, gcs/invites key conventions
  - phase: 04-02
    provides: CoreSession::QueryKeyValues prefix scan + RegisterNewElementCallback CRDT heal hook
  - phase: 04-04
    provides: EntityStore creator + members_can_invite stamping (the gate's persisted input)
provides:
  - Membership component (gcs_membership.hpp/.cpp) — the single owner of the member roster, the five-tier role rules, and the invite mint/redeem lifecycle
  - Per-member LWW MemberRecord under gcs/members/<scopeId>/<wallet> (D-01), prefix-scanned, no manifest
  - Capability-token invites (D-05): InviteRecord written at mint under gcs/invites/<scopeId>/<inviteId>, member record written at redeem (write-at-redeem, multi-use, no expiry)
  - Write-time guards — creator-demotion reject (D-03), role/members_can_invite mint gate (D-08), dormant-until-approved (D-07)
  - 10 unit cases (test_gcs_membership) + 2 live two-node convergence cases (test_gcs_membership_multinode)
affects: [04-05 (FFI wiring calls MintInvite/RedeemInvite/ChangeRole/RemoveMember/Approve), Dart membership state layer, membership UAT]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Replication is a write-time responsibility: every roster/invite write passes the scope's room topic (ScopeTopics) to CoreSession::Put — a bare Put(key,value) broadcasts on NO topic and never leaves the node"
    - "Injected seams (EventSink + KeyWrapSeam) follow the CryptoSeam precedent: default-constructed disabled, built by member assignment, so the component is SDK-free and unit-testable"
    - "Write-at-redeem invite semantics: mint persists only the capability record; the member record materializes at redeem, so an invite is a reusable bearer capability rather than a consumed ticket"

key-files:
  created:
    - src/lib/gcs_membership.hpp
    - src/lib/gcs_membership.cpp
    - test/test_gcs_membership.cpp
    - test/test_gcs_membership_multinode.cpp
  modified:
    - test/CMakeLists.txt
    - src/CMakeLists.txt

key-decisions:
  - "Base64url value encoding (no padding) for the D-05 invite URL, so the IK bears no URL-reserved characters and round-trips through a plain query-string parse"
  - "MembersFor stamps self_address from the injected sender address rather than trusting any record field — the local wallet is pushed identity, never replicated state"
  - "Key-form normalization: ScopeIdFromKey and SegmentAfterMarker handle both the logical callback key (/gcs/members/...) and the raw datastore key (/crdt/k/.../v) that QueryKeyValues returns"
  - "Hostile/unreadable records are skipped with spdlog::warn, never abort the scan — one malformed CRDT value must not take down the roster"

patterns-established:
  - "The SSOT-owning component declares its own kInvitesKeyPrefix/kMembersKeyCallbackPattern so callers never spell the key layout themselves"

requirements-completed: [MEMB-01, MEMB-02, MEMB-03, MEMB-04, MEMB-05]

# Metrics
duration: 55min
completed: 2026-10-02
---

# Phase 4 Plan 3: Membership component (roster, roles, invites) Summary

**A CRDT-backed Membership component now owns the five-tier role roster, the write-time permission guards, and the capability-token invite lifecycle — 12 green tests including two live two-node convergence cases over real GossipSub.**

## Performance

- **Duration:** ~55 min (approximate — plan start was not instrumented across the prior session boundary)
- **Completed:** 2026-10-02 (final task commit 90712f3 at 13:29:42-07:00)
- **Tasks:** 3
- **Files modified:** 6 (2822 insertions, 0 deletions)

## Accomplishments
- `Membership` (src/lib/gcs_membership.hpp/.cpp) is the single owner of the membership domain: roster reads, role rules, invite mint/redeem, role change, removal, leave, scope deletion, and approval
- `MembersFor` prefix-scans `gcs/members/<scopeId>/` via `CoreSession::QueryKeyValues`, skips tombstoned records, and stamps the pushed `MemberList.self_address` (D-01/D-04)
- `MintInvite` writes exactly one `InviteRecord` under `gcs/invites/<scopeId>/<inviteId>` carrying the role grant + room key wrapped under the bearer IK; **no member record is written at mint** (D-05, write-at-redeem)
- `RedeemInvite` parses the `gcs://invite/{space|room}/<inviteId>?key=<IK>` URL, unwraps the room key into the per-room key store, and only then writes the redeemer's member record — an invite is therefore multi-use and without expiry
- Write-time guards: creator demotion rejected outright (D-03/MEMB-03); mint gated by role and `members_can_invite` (D-08/MEMB-05); `RemoveMember`/admin-demoting `ChangeRole`/`DeleteScope` write **dormant** (empty `approved_by`) when 2+ admins exist, and `Approve` activates by overwriting the same key (D-07/MEMB-04)
- `OnScopeCreated` writes the creator's Super Admin record and, for a standalone room, mints/stores/wraps the room key
- 10 unit cases cover the full matrix including malformed-URL/bad-envelope rejection, tombstone skip, explicit-leave, and the dormant-until-approved path; 2 live cases prove remote roster and role-change convergence over the real transport

## Task Commits

Each task was committed atomically:

1. **Task 1: Write the Membership interface contract (gcs_membership.hpp)** - `51e649a` (feat)
2. **Task 2: Implement member CRUD, permission matrix, invite mint/redeem, D-03/D-07/D-08 guards + unit tests** - `6cfc563` (feat)
3. **Task 3: Write test_gcs_membership_multinode.cpp and register it** - `90712f3` (test)

**Plan metadata:** committed by orchestrator (this executor does not write STATE.md/ROADMAP.md in worktree mode)

## Files Created/Modified
- `src/lib/gcs_membership.hpp` (411 lines) — the public contract: `kMembersKeyPrefix`/`kInvitesKeyPrefix`/`kMembersKeyCallbackPattern`, `EventSink`/`WrapFn`/`UnwrapFn`/`KeyWrapSeam`, and the full method surface (MembersFor, LocalRoleFor, HasExplicitLeave, MintInvite, RedeemInvite, ChangeRole, RemoveMember, Leave, DeleteScope, Approve, OnMemberArrived, OnScopeCreated)
- `src/lib/gcs_membership.cpp` (1233 lines) — the implementation: key derivation + ScopeIdFromKey/SegmentAfterMarker normalization, base64url encode/decode, NowMs, PrivilegeRank, WriteMember, and every guard and lifecycle path
- `test/test_gcs_membership.cpp` (734 lines) — `GcsMembershipTest` fixture over the injected-pubsub seam (`MembershipRig`), 10 TEST_F cases
- `test/test_gcs_membership_multinode.cpp` (434 lines) — `MultinodeNode`/`StartTransport`/`StartSession`/`ArmMembership`/`ShutdownNode` helpers + the 2-node fixture and its 2 convergence tests
- `test/CMakeLists.txt` — registers `test_gcs_membership` and `test_gcs_membership_multinode` via `gcs_test(..., "gcs_core;gcs_storage;neoswarm_common")`
- `src/CMakeLists.txt` — adds `lib/gcs_membership.cpp` to the `gcs_core` source list (**deviation, see below**)

## Decisions Made
- **Replication requires an explicit topic.** The CRDT store's two-argument `Put(key, value)` broadcasts on `{}` — nothing replicates. Every roster and invite write passes the scope's room topic, which is the only way Task 3's convergence criterion is reachable (see Deviation 2).
- **`self_address` is pushed, never read.** `MembersFor` stamps the caller's injected wallet rather than trusting a replicated field, so a remote record can never spoof another node's identity.
- **Two key shapes, one normalizer.** CRDT callbacks deliver logical keys (`/gcs/members/...`, `regex_match`ed in full) while `QueryKeyValues` returns raw datastore keys (`/crdt/k/.../v`); `SegmentAfterMarker`/`ScopeIdFromKey` accept both so the same parsing serves the heal path and the scan.
- **Skip-and-log, never abort.** A malformed or hostile record is warned and skipped so a single bad CRDT value cannot deny the whole roster (T-04-03-DoS).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Registered gcs_membership.cpp in src/CMakeLists.txt (outside plan's files_modified)**
- **Found during:** Task 2
- **Issue:** The plan's `files_modified` lists only the header, the two test files, and `test/CMakeLists.txt`. But Task 2's own verification is `ctest -R test_gcs_membership`, and that target links `gcs_core` — the new .cpp is never compiled unless it is added to the `gcs_core` source list. The task is unverifiable without the edit.
- **Fix:** Added `lib/gcs_membership.cpp` to `add_library(gcs_core ...)` in `src/CMakeLists.txt`, beside the other component sources.
- **Commit:** `6cfc563`

**2. [Rule 2 - Missing critical functionality] Member/invite writes replicated on no topic**
- **Found during:** Task 3 (both multinode tests failed — node B never saw the creator's member record or the invite within the wait window, so `RedeemInvite`/`ChangeRole` reported "member not found")
- **Issue:** `GcsGlobalDb::Put(key, value)` forwards to `Put(key, value, /*topics=*/{})`. Tracing `CrdtDatastore::PutKey` → `Publish(delta, topics)` → `CreateDAGNode(delta, topics)` confirms an empty topic set means the delta is stored locally and never broadcast. Correctness — not just the test — was broken: a minted invite could never be redeemed by any other node, and no roster would ever converge. This is the plan's headline P6 outcome (MEMB-01/MEMB-02 across nodes).
- **Fix:** Added a `ScopeTopics(scopeId)` helper (the `gcs/chat/<scopeId>` room topic) and passed it to `CoreSession::Put` in `WriteMember` and in the `MintInvite` invite-record write. No header change was needed.
- **Commit:** `6cfc563`

**3. [Rule 3 - Blocking] Multinode test nodes never subscribed to the room topic**
- **Found during:** Task 3
- **Issue:** Even with the fix above, neither node had registered the room topic, so the broadcast had no subscriber. Production wiring does this at join (D-07: listen first, then broadcast); the test fixture had to mirror it or the convergence assertions were unreachable.
- **Fix:** `CreateRoomAndJoin` now calls `AddListenTopic` + `AddBroadcastTopic` on **both** nodes for the room topic before minting.
- **Commit:** `90712f3`

---

**Total deviations:** 3 auto-fixed (2 Rule 3 blocking, 1 Rule 2 missing critical functionality); 0 requiring user decision
**Impact on plan:** Deviation 2 is a genuine behaviour fix in the shipped component, not test scaffolding — without it capability-token invites and remote rosters do not work at all. Deviations 1 and 3 are wiring required to make the plan's own verification criteria executable. No plan requirement was dropped or weakened.

## Issues Encountered

- **clang-tidy could not run.** No `.clang-tidy` exists at the GCS root (only `GNUS-NEO-SWARM/.clang-tidy`), so a plain run reports `Error: no checks enabled.` Passing `--config-file=GNUS-NEO-SWARM/.clang-tidy` instead fails earlier: `compile_commands.json` carries **two** compiler jobs per TU (x86_64 + arm64), and clang-tidy rejects it with "unable to handle compilation, expected exactly one compiler job". Not run — environment/config limitation, matching the 04-04 precedent.
- **clang-format v22.1.7 was not applied.** Only clang-format 22.1.7 is available, and against the checked-in `.clang-format` it would rewrite 251/1872/199/103 lines of the four new files (2-space vs 4-space, Allman namespaces, `( x )` spacing) — a mechanical reformat far larger than the change itself, breaking the minimal-change rule and the established file style. Added lines were hand-matched to the surrounding repo style instead (same resolution as 04-04).
- **Compiler gate passed instead:** `gcs_core` and both test targets build with `-Wall -Wextra -Werror=sign-compare -Werror=type-limits -Werror=unused-lambda-capture` and no warnings or errors.

## Verification

- `ctest --test-dir build/OSX/Debug -R "test_gcs_membership|test_gcs_membership_multinode"` → **100% tests passed, 0 failed out of 2** (8.19 s): `test_gcs_membership` 10/10 cases, `test_gcs_membership_multinode` 2/2 cases.
- Regression sweep over every suite linking `gcs_core` → **100% passed, 0 failed out of 5** (5.86 s): `test_gcs_core_smoke`, `test_gcs_entities`, `test_gcs_crypto`, `test_gcs_messaging`, `test_gcs_messaging_multinode`.
- No `sleep_for`/`sleep_until` anywhere in either test file (grep clean); all waits use the `gcs::test::WaitForCondition` template.

## TDD Gate Compliance

**Warning — no standalone RED commit exists.** The plan is `type: execute`, and Tasks 1–2 carry `tdd="true"`, but each task bundles its deliverable into a single committing unit: Task 1 is the header alone (no test), and Task 2's action writes the implementation and `test_gcs_membership.cpp` together, so there was no point at which a failing test could be committed separately and then satisfied. The git history therefore contains no `test(04-03): add failing test ...` commit preceding the `feat(04-03): implement ...` commit; the only `test(04-03)` commit (`90712f3`) is the Task 3 integration suite, which is additive rather than a RED gate.

Gate sequence actually observed: `feat` (51e649a) → `feat` (6cfc563) → `test` (90712f3) — RED gate missing, GREEN gate present, REFACTOR gate not needed. The behaviour is nevertheless specified by tests: the 10 unit cases were authored against the plan's 9 listed behaviours and were confirmed to fail against the un-topic'd write path while diagnosing Deviation 2 (the multinode suite failed precisely because the component behaviour was wrong, then passed after the fix).

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Plan 04-05 (FFI wiring) can construct `Membership(*session, *store, address, sink, seam)` and forward `MintInvite`/`RedeemInvite`/`ChangeRole`/`RemoveMember`/`Leave`/`DeleteScope`/`Approve` plus `MembersFor`/`LocalRoleFor` to the Dart layer.
- The FFI join path must register the scope's room topic (listen-then-broadcast) before minting or redeeming — the multinode fixture now demonstrates the exact sequence production needs.
- Room-key storage is the D-06 seam over the vendored-OpenSSL adapter; a production caller must supply the enabled seam, since the default-constructed one is inert.

### Stub tracking

No stubs introduced. Every code path (mint, redeem, role change, removal, approval, tombstone scan, room-key store) is real, persisted through the CRDT store, and asserted through a read-back in the unit or multinode suite. No placeholder text, no hardcoded empty collections flowing to output, no unwired data source.

### Threat surface scan

No new surface beyond the plan's threat model. The changes are the mitigations themselves:

| Plan threat | Where mitigated |
|---|---|
| T-04-03-EoP-1 (non-admin mints) | `MintInvite` role + `PrivilegeRank` gate, asserted by `membersCanInviteGatesMemberMinting` |
| T-04-03-EoP-2 (demote the creator) | creator-demotion reject in `ChangeRole`, asserted by `CreatorDemotionIsRejected` |
| T-04-03-EoP-3 (unapproved removal of a peer admin) | dormant-when-`AdminCount > 1` write path, asserted by `TwoAdminRemovalIsDormantUntilApproved` |
| T-04-03-T-1/T-2 (roster tampering) | all authority fields C++-stamped from the injected identity; `self_address` never read from a record |
| T-04-03-DoS (hostile record aborts enumeration) | skip-and-log in `MembersFor` and in the redeem unwrap path |
| T-04-SC (cross-scope read/write) | keys are scope-qualified (`gcs/members/<scopeId>/`, `gcs/invites/<scopeId>/`); a scan can only see its own prefix |

No new network endpoint, auth path, or trust boundary was introduced by this plan.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-02*

## Self-Check: PASSED

- FOUND: src/lib/gcs_membership.hpp
- FOUND: src/lib/gcs_membership.cpp
- FOUND: test/test_gcs_membership.cpp
- FOUND: test/test_gcs_membership_multinode.cpp
- FOUND: .planning/workstreams/app/phases/04-membership-invites/04-03-SUMMARY.md
- FOUND: 51e649a (Task 1)
- FOUND: 6cfc563 (Task 2)
- FOUND: 90712f3 (Task 3)
