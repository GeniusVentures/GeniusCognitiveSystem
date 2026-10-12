---
phase: 04-membership-invites
plan: 05
subsystem: ffi/dispatch
tags: [cpp17, ffi, dart-api-dl, membership, crdt, abba-deadlock, googletest, gcs-membership]

# Dependency graph
requires:
  - phase: 04-03
    provides: gcs::Membership component (roster, five-tier roles, invite mint/redeem, dormant-approve) + kMembersKeyCallbackPattern
  - phase: 04-04
    provides: EntityStore wallet-address constructor + stamped creator/members_can_invite on spaces and standalone rooms
provides:
  - gcs_init constructs g_membership (real wallet address + D-06 key-wrap seam) and registers the /gcs/members/.* heal callback with the ABBA-safe intake-gate discipline
  - BuildMemberListEvent / BuildInviteLinkEvent push builders; the MemberList carries self_address forwarded unchanged from Membership::MembersFor
  - Seven membership dispatch arms (invite/redeem/change_role/remove_member/leave/delete_scope/approve) that validate, delegate, and re-push
  - P5 retroactive role guards on send_text/create_room/update_space, enforced only once a scope has a roster
  - D-02 explicit_leave term applied to derived joins in RefreshDerivedJoins
  - Session-init replay of a MemberList per space / standalone room
affects: [04-06 (Flutter UI consumes the pushed MemberList/InviteLink), 04-07 (membership UI surfaces), any later phase extending the GcsCommand oneof]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Every Membership mutator runs in the unlocked in-flight window (g_inFlight -> lock.unlock() -> call -> lock.lock() -> notify) because mutators push through the EventSink; holding g_mutex across them is the 03-06 ABBA deadlock"
    - "Under-mutex Membership reads (MembersFor / LocalRoleFor / HasExplicitLeave) are the sanctioned ones — they are converged local prefix scans that never push through the sink"
    - "Retroactive (P5) guards gate a legacy command only once the scope has a roster, so pre-membership catalogs and smoke topics keep working"
    - "Adapter-layer rank predicate over the proto Role enum's DESCENDING privilege order (ROLE_SUPER_ADMIN=1 .. ROLE_GUEST=5), expressed once as RoleAtLeast so no arm can invert it"

key-files:
  created: []
  modified:
    - src/ffi/gcs_core_ffi.cpp
    - test/test_gcs_ffi.cpp

key-decisions:
  - "Implemented RoleAtLeast(role, minimum) as `role != ROLE_UNSPECIFIED && role <= minimum` — the plan text specified `>=`, which is inverted for the descending-privilege proto enum and would have granted Admin+ commands to Guests (T-04-05-EoP-2)"
  - "P5 guards use `ScopeHasRoster(scope) && !RoleAtLeast(LocalRoleFor(scope), MIN)` so the guard is skipped entirely for a scope with no member records"
  - "RedeemInvite already pushes the redeemed scope's MemberList through the sink (the scope id lives in the invite record, not the URL), so the kRedeem arm only re-pushes the SpaceTree and never derives the scope id from the URL"
  - "ScopeHasRoster/RoleAtLeast are file-scope helpers in the anonymous namespace placed beside the other event builders, keeping the public FFI ABI (4 functions) frozen"

patterns-established:
  - "Membership reachable from Dart only through the existing GcsCommand oneof — no new FFI ABI symbols"

requirements-completed: [MEMB-01, MEMB-02, MEMB-03, MEMB-04, MEMB-05]

# Metrics
duration: ~70min
completed: 2026-10-12
---

# Phase 4 Plan 5: Membership FFI dispatch plane Summary

**gcs::Membership is now reachable from Dart: gcs_init builds it with the real wallet and key-wrap seam, the /gcs/members/.* CRDT heal callback converges rosters into pushed MemberLists carrying self_address, seven new command arms dispatch invite/redeem/role/removal/leave/delete/approve, existing Phase 2/3 writes are role-gated once a roster exists, and derived joins honor D-02 explicit_leave.**

## Performance

- **Duration:** ~70 min (approximate — the session context was compacted mid-plan, so the exact start wall-clock was not captured; the commit window itself is 11:36–11:45 PT)
- **Started:** not recorded (mid-plan context compaction)
- **Completed:** 2026-10-12T18:46Z
- **Tasks:** 3
- **Files modified:** 2 (`src/ffi/gcs_core_ffi.cpp`, `test/test_gcs_ffi.cpp`)

## Accomplishments

- `g_membership` constructed in `gcs_init` from `GeniusSDKGetAddress().address` with the D-06 `KeyWrapSeam` (wrap/unwrap = `gcs::crypto::WrapRoomKey`/`UnwrapRoomKey`, enabled), and the `EntityStore` switched to its 2-arg wallet-injecting constructor so creator stamping gets the real address.
- The `/gcs/members/.*` heal callback registered with the identical teardown-intake-gate + copy-pointer-outside-lock + `g_inFlight` discipline as the messages callback (T-04-05-DoS).
- `BuildMemberListEvent` / `BuildInviteLinkEvent` builders; the roster event forwards `self_address` from `Membership::MembersFor` unchanged so Dart derives the "You" marker from authority.
- Seven dispatch arms added before `PAYLOAD_NOT_SET`, each validating the hostile command shape (V5) then delegating to the Membership write guard (T-04-05-EoP-1); each mutator call runs in the unlocked in-flight window.
- P5 retroactive guards on `send_text` (Member+), `create_room` (Admin+ on a nested parent), and `update_space` (Admin+), skipping the gate for roster-less legacy scopes (T-04-05-EoP-2); `create_space`/`create_room` stamp `members_can_invite`; nested-room key mint + standalone-room `OnScopeCreated` wired.
- `RefreshDerivedJoins` filters derived rooms whose parent space the local member explicitly left (D-02); `gcs_subscribe` replays a MemberList per space / standalone room.
- 6 new `test_gcs_ffi.cpp` cases (10 → 16 tests) using a fake Dart API_DL capture seam; no sleeps in the file.

## Task Commits

Each task was committed atomically:

1. **Task 1: Construct g_membership, register the members heal callback, add the MemberList/InviteLink builders** — `2e4e984` (feat)
2. **Task 2: Add the seven membership dispatch arms + P5 retroactive guards** — `c874167` (feat)
3. **Task 3: Apply the D-02 explicit_leave term in RefreshDerivedJoins** — `6314920` (feat)

**Plan metadata:** SUMMARY commit (docs: complete plan)

## Files Created/Modified

- `src/ffi/gcs_core_ffi.cpp` — `g_membership` global + construction, members heal callback, `BuildMemberListEvent`/`BuildInviteLinkEvent`, `ScopeHasRoster`/`RoleAtLeast`, seven command arms, P5 guards, D-02 derived-join filter, session-init MemberList replay.
- `test/test_gcs_ffi.cpp` — fake Dart API_DL capture seam (PushedEventLog/FakePostCObject/InstallFakeApiDlTable), fixture helpers (`InitSubscribedSession`, `PublishCommand`), and 6 new cases.

## Decisions Made

- **Role rank predicate** (`RoleAtLeast`) implemented once and used by all three P5 guard sites so the comparison cannot be inverted per-arm.
- **kRedeem does not derive the scope id from the URL** — `Membership::RedeemInvite` pushes the redeemed roster itself; the arm only re-pushes the SpaceTree.
- **Nested-room key mint stays in the arm; standalone-room key mint stays in `OnScopeCreated`** — never both, so no double mint.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug, security] Inverted role comparison in the plan's P5 guards**
- **Found during:** Task 2 (P5 retroactive guards)
- **Issue:** The plan specified `LocalRoleFor(...) >= ROLE_ADMIN` / `>= ROLE_MEMBER`. The proto `Role` enum is ordered by DESCENDING privilege (`ROLE_SUPER_ADMIN=1` … `ROLE_GUEST=5`), so `>=` grants the guarded commands to lower-ranked roles and denies admins — a T-04-05-EoP-2 elevation-of-privilege hole.
- **Fix:** Implemented `RoleAtLeast(role, minimum) { return role != ROLE_UNSPECIFIED && role <= minimum; }` and used `!RoleAtLeast(...)` at all three guard sites. `ROLE_UNSPECIFIED` is excluded so a malformed role never satisfies a guard.
- **Files modified:** `src/ffi/gcs_core_ffi.cpp`
- **Verification:** `test_gcs_messaging`, `test_gcs_membership`, `test_gcs_ffi` all green; grep confirms `RoleAtLeast` is the single comparison used by the guards.
- **Committed in:** `c874167` (Task 2 commit)

### Plan-internal sequencing adjustments

**2. Task 1's observable behaviour required the create_space creator-minting that the plan assigns to Task 2**
- **Found during:** Task 1 (first test run)
- **Issue:** Task 1's stated test — "`BuildMemberListEvent(scopeId)` returns a MemberList whose `self_address` equals the injected local wallet" — is only observable if some code path mints a member scope. The plan puts the `kCreateSpace` → `Membership::OnScopeCreated` wiring in Task 2, so Task 1 alone pushes no MemberList and its own test cannot pass.
- **Fix:** Moved the `kCreateSpace` creator-minting + `members_can_invite` stamping hunks into Task 1's commit (`2e4e984`), leaving the seven arms and the remaining P5 guards in Task 2 (`c874167`). No behaviour changed relative to the plan — only the commit boundary.
- **Files modified:** `src/ffi/gcs_core_ffi.cpp`
- **Verification:** Task 1's two new tests pass in isolation against the Task-1 commit; the final tree is byte-identical to the fully-verified source.
- **Committed in:** `2e4e984` (Task 1 commit)

**3. Task 3's D-02 negative test is unreachable from the FFI, so it was replaced with a positive derived-join test**
- **Found during:** Task 3 (D-02 test design)
- **Issue:** The plan asks to assert that after the local member explicitly leaves a space, `RefreshDerivedJoins` excludes that space's derived rooms. In a single-wallet FFI fixture the local wallet is always the space creator (Super Admin), and `Membership::Leave` rejects the Super Admin (`IsSuperAdmin` compares the entity `creator`), so `HasExplicitLeave` can never become true through the FFI. The FFI cannot inject a second wallet address (`GeniusSDKGetAddress().address` is the single identity stamp).
- **Fix:** Added `DerivedAutoJoinedRoomSurvivesExplicitLeaveFilter` — a positive test proving the D-02 filter's non-filtered branch (an autoJoinRooms parent's room with no explicit leave still reaches the pushed RoomList, so the filter never over-evicts). The `explicit_leave == true` branch is covered at the owning module's boundary in `test/test_gcs_membership.cpp` (`HasExplicitLeave` true/false cases, lines 305/666).
- **Files modified:** `test/test_gcs_ffi.cpp`
- **Verification:** `test_gcs_ffi` green (16/16); `test_gcs_membership` green.
- **Committed in:** `6314920` (Task 3 commit)

---

**Total deviations:** 3 (1 auto-fixed bug, 2 plan-internal sequencing/test-design adjustments)
**Impact on plan:** No behaviour deferred and no scope creep. The auto-fix closes a real elevation-of-privilege hole; the two adjustments keep each task commit self-consistent and testable given the plan's own internal ordering.

## Issues Encountered

- The pre-existing `test_gcs_ffi_dart` ctest entry fails in this worktree before any code runs: `flutter_app` depends on `frontend_scaffold` at `src/app/scaffold`, which has no `pubspec.yaml` in the worktree (submodule not initialized there). Unrelated to this plan; the verification run excludes it with `-E '^test_gcs_ffi_dart$'`.
- The build directory was configured with `BUILD_TESTS=OFF`; re-configured with `-DBUILD_TESTS=ON` to build and run the test targets. `test_gcs_ffi_sdk`, `test_gcs_ffi_coldboot`, `test_gcs_messaging`, `test_gcs_membership`, and `test_gcs_membership_multinode` were built to satisfy the plan's `-R` verification regex.

## Known Stubs

None — no placeholder values, dead branches, or unwired data sources were introduced.

## Threat Flags

None. The new surface (seven command arms + three P5 guards) is fully described by the plan's `<threat_model>` (T-04-05-EoP-1/EoP-2/I/DoS), and no new network endpoint, auth path, or trust boundary was added.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Dart can now drive the full membership lifecycle through the existing `GcsCommand` oneof; `MemberList` pushes carry `self_address` for the "You" marker, and `InviteLink` pushes carry the `gcs://invite/...` capability URL.
- Memberships converge from remote writes via the `/gcs/members/.*` heal callback; existing Phase 2/3 commands are role-gated once a scope has a roster.
- Verification command to re-run: `ctest --test-dir build/OSX/Debug -R "test_gcs_ffi|test_gcs_membership" -E '^test_gcs_ffi_dart$' --output-on-failure` (5/5 green).

---
*Phase: 04-membership-invites*
*Completed: 2026-10-12*

## Self-Check: PASSED

- `src/ffi/gcs_core_ffi.cpp` — FOUND
- `test/test_gcs_ffi.cpp` — FOUND
- `.planning/workstreams/app/phases/04-membership-invites/04-05-SUMMARY.md` — FOUND
- Task commits `2e4e984`, `c874167`, `6314920` — FOUND in `git log`

