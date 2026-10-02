---
phase: 04-membership-invites
plan: 06
subsystem: ui
tags: [dart, cubit, protobuf, membership, roster, self-address, pending-action, invites]

# Dependency graph
requires:
  - phase: 04-membership-invites
    plan: 01
    provides: Role/PendingAction enums, MemberRecord/MemberList (self_address), InviteLink/InviteRecord, SpaceRecord/RoomRecord approved_by/pending_action, GcsCommand/GcsEvent oneof arms
  - phase: 03-messaging
    provides: SessionCubit._dispatchEvent pushed-event pipeline, RailCubit.setTree full-replacement pattern, shell_cubits_test fixture
  - phase: 02-spaces-rooms
    provides: RailSpace/RailRoom models, in-tree protoc + pinned protoc_plugin regen precedent
provides:
  - Regenerated committed Dart protobuf bindings carrying the Phase 4 membership types
  - MembersCubit with MemberInfo/MembersState/setMembers (full-replacement per-scope roster)
  - SessionCubit hasMemberList/hasInviteLink dispatch arms + MembersCubit injection (ctor + openDefault)
  - pendingInviteLink getter for the plan-04-07 invite view
  - RailSpace/RailRoom approvedBy/pendingAction surfaced from the pushed SpaceTree
affects: [04-07]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Dart bytes fields: protoc_plugin 22.5.0 emits `$core.List<$core.int>` for proto3 `bytes` (PbFieldType.OY) — never a String"
    - "Pushed-event cubit: full-replacement map entry (`{...state.byScope, scopeId: roster}`) mirrors RailCubit.setTree"
    - "Cubit injection via optional constructor param threaded through SessionCubit.openDefault"

key-files:
  created:
    - src/app/lib/cubits/members_cubit.dart
  modified:
    - src/app/lib/generated/proto/gcs_chat.pb.dart
    - src/app/lib/generated/proto/gcs_chat.pbenum.dart
    - src/app/lib/generated/proto/gcs_chat.pbjson.dart
    - src/app/lib/cubits/session_cubit.dart
    - src/app/lib/cubits/rail_cubit.dart
    - src/app/test/cubits/shell_cubits_test.dart

key-decisions:
  - "Regenerated the Dart bindings with the pinned protoc_plugin 22.5.0 resolved from the app's own package_config, never via a global pub activation (the global shim had drifted to 25.1.0)"
  - "pendingLabel for a dormant role change uses the record's pending_role (the D-07 target), so the badge reads informatively ('Pending: Member') per the UI-SPEC example"
  - "RailSpace/RailRoom approvedBy/pendingAction are optional named params with safe defaults, preserving the Phase 2 space_room_dialog_test constructions"
  - "MembersCubit owns the role-label and pending-badge copy as named constants (kRoleLabel*, kPendingRemovalLabel, kPendingRoleChangeLabelPrefix) — no inlined display strings"

patterns-established:
  - "Per-scope pushed roster: MembersCubit.setMembers replaces state.byScope[scopeId] wholesale; other scopes are preserved by map spread"
  - "Pushed authority over local resolution: the 'You' marker always comes from MemberList.selfAddress, never from session state"

requirements-completed: [MEMB-02, MEMB-04]

# Metrics
duration: 7min
completed: 2026-10-02
---

# Phase 4 Plan 06: Dart Membership State Layer Summary

**Regenerated the Phase 4 Dart protobuf bindings and added the membership state layer — MembersCubit holding a full-replacement per-scope roster keyed off the pushed MemberList (with the pushed selfAddress driving the "You" marker), SessionCubit memberList/inviteLink dispatch arms, and RailSpace/RailRoom surfacing of the pushed approved_by/pending_action.**

## Performance

- **Duration:** ~7 min
- **Started:** 2026-10-02T20:05:00Z
- **Completed:** 2026-10-02T20:11:00Z
- **Tasks:** 2
- **Files modified:** 7 (6 modified, 1 created)

## Accomplishments

- `gcs_chat.pb.dart` / `.pbenum.dart` / `.pbjson.dart` regenerated from the plan-04-01 proto: `Role` (5 tiers) and `PendingAction` enums, `WrappedRoomKey` (bytes), `MemberRecord` (11 fields incl. `pendingRole`/`wrappedRoomKey`), `MemberList` with `selfAddress`, `InviteLink`, `InviteRecord`, `SpaceRecord`/`RoomRecord` `creator`/`membersCanInvite`/`approvedBy`/`pendingAction`, and the `hasMemberList`/`hasInviteLink` oneof accessors. Diff is 1209+/2, 58+/0, 414+/8 — the only deletions are the two oneof field-index lists, re-emitted wider by the extended proto.
- `MembersCubit` added: `MemberInfo.fromRecord(record, selfAddress:)` maps the pushed record onto `address`/`role`/`explicitLeave`/`pending`/`pendingLabel`/`self`; `MembersState.byScope` holds owned-unmodifiable rosters; `setMembers` replaces one scope's roster wholesale and leaves the others intact.
- `SessionCubit` now injects `MembersCubit` (constructor + all five `openDefault` construction sites), dispatches `hasMemberList → setMembers(scopeId, member, selfAddress)` and `hasInviteLink → _pendingInviteLink` immediately before the unchanged `hasError` arm, and exposes `pendingInviteLink` for plan 04-07.
- `RailCubit.setTree`/`_withRooms` carry the pushed `approvedBy`/`pendingAction` onto `RailSpace`/`RailRoom`, so plan 04-07 can render the D-07 "Pending deletion" badge from pushed state alone (no Dart CRDT reads, no derived role/creator logic).
- 7 new tests in `shell_cubits_test.dart` (4 MembersCubit + 3 SessionCubit dispatch/surfacing); the full app suite is green.

## Task Commits

Each task was committed atomically:

1. **Task 1: Regenerate the Dart protobuf bindings and create MembersCubit** - `fd1df94` (feat)
2. **Task 2: Wire SessionCubit dispatch arms + membersCubit injection, surface the RailSpace/RailRoom pending fields, and extend the cubit tests** - `b828ae3` (feat)

## Files Created/Modified

- `src/app/lib/cubits/members_cubit.dart` (created) — MemberInfo/MembersState/MembersCubit, role-label and pending-badge constants
- `src/app/lib/generated/proto/gcs_chat.pb.dart` (modified) — regenerated message bindings
- `src/app/lib/generated/proto/gcs_chat.pbenum.dart` (modified) — regenerated Role/PendingAction enums
- `src/app/lib/generated/proto/gcs_chat.pbjson.dart` (modified) — regenerated JSON descriptors
- `src/app/lib/cubits/session_cubit.dart` (modified) — MembersCubit injection + two dispatch arms + `pendingInviteLink`
- `src/app/lib/cubits/rail_cubit.dart` (modified) — RailSpace/RailRoom pending fields carried through `setTree`/`_withRooms`
- `src/app/test/cubits/shell_cubits_test.dart` (modified) — 7 new tests

## Decisions Made

- **`pendingLabel` for a dormant role change uses `pending_role`, not the member's current role.** The plan named the label ("Pending: <role>") without naming which role supplies it. The UI-SPEC's example ("Pending: Member") and the proto contract (`pending_role = 10` is "D-07 target role of a dormant role change") both make the *target* role the only informative reading — a demotion Admin→Member would otherwise render "Pending: Admin". The record's own `role` still holds the pre-approval tier, so this is the only choice that makes the badge meaningful.
- **`_pendingLabel` returns `''` for any action outside the member-scope contract.** D-07 `PENDING_ACTION_DELETE` rides on the *scope's* own record, never a member record, so the default branch is unreachable for pushed member rosters. Consequence if it ever were reached: `pending == true` with an empty label, and plan 04-07 would render a blank badge. Documented rather than papered over with invented copy.
- **Rail pending fields are optional named params with safe defaults** (`this.approvedBy = ''`, `this.pendingAction = PendingAction.PENDING_ACTION_UNSPECIFIED`), mirroring the existing `this.rooms = const <RailRoom>[]` idiom, so the Phase 2 `space_room_dialog_test.dart` constructions keep compiling.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Global `protoc-gen-dart` shim had drifted off the pinned 22.5.0**
- **Found during:** Task 1 (pre-regen environment check)
- **Issue:** The plan (and the repo README) assume `$HOME/.pub-cache/bin/protoc-gen-dart` is pinned to `protoc_plugin 22.5.0`. It was actually the 25.1.0 global activation. `protoc_plugin` 25.x emits `BuilderInfo.aE` against a `protobuf 4.2.0` runtime — the exact break the plan's pin exists to prevent.
- **Fix:** Avoided mutating shared global pub state (concurrent worktree agents). Ran `flutter pub get` in `src/app` (which resolves the locked `protoc_plugin 22.5.0`) and generated through a `/tmp` wrapper invoking the pristine cache entry point directly:
  ```
  dart --packages=src/app/.dart_tool/package_config.json \
       ~/.pub-cache/hosted/pub.dev/protoc_plugin-22.5.0/bin/protoc_plugin.dart
  ```
  No `dart pub global activate` was run. Verified the resolved versions first (`protoc_plugin 22.5.0`, `protobuf 4.2.0`, `fixnum 1.1.1`).
- **Files modified:** none in-tree (wrapper lives at `/tmp/gsd-protoc-gen-dart-22.5.0.sh`)
- **Commit:** n/a (tooling only)

**2. [Rule 3 - Blocking] `lib/generated/chat/` was absent in the fresh worktree**
- **Found during:** Task 1 verification (`dart analyze` reported 123 errors in `message_flow_cubit.dart` and `test/generated/`)
- **Issue:** `lib/generated/chat/` is gitignored codegen output, normally produced by the CMake target `app_generate_chat_components`. A fresh worktree has no CMake build tree, so `ChatFlowItemTextBubble`/`ChatMessageFlow` were undefined and neither analyze nor the test suite could run.
- **Fix:** Ran the scaffold engine directly for all 12 composite triples (same argv the CMake custom command uses: `python3 scaffold/tools/scaffold_codegen/engine.py --template components/<c>[_cubit|_state].dart.jinja2 --tokens scaffold/design_tokens.json --output lib/generated/chat/... --template-dir templates --template-dir scaffold/templates --vars templates/components/<c>_vars.json`). Required initializing the `src/app/scaffold` submodule first.
- **Files modified:** `lib/generated/chat/*` (gitignored, not committed)
- **Commit:** n/a (generated, gitignored)

### Plan-wording notes (no behaviour impact)

- **`WrappedRoomKey.envelope` type.** The plan expects `Uint8List get envelope`. `protoc_plugin` 22.5.0 emits `$core.List<$core.int> get envelope` for a proto3 `bytes` field (`PbFieldType.OY`, `$_setBytes`). The bytes semantics the plan cares about hold (it is a byte list, never a `String`); the literal `Uint8List` wording matches newer plugin output. `MemberInfo` does not consume `envelope`.
- **Test count.** The plan named 5 test behaviours; 7 tests were added — the 5 named behaviours are all covered, plus a per-scope isolation test and the `pending`/`pendingLabel` derivation test.
- **Task 1 commit was amended once** (`fd1df94`) to fold in the `dart format` pass over `members_cubit.dart`; no other task-1 content changed.

## Issues Encountered

- **Worktree base drift (corrected before any work):** the worktree branch was created with a base whose merge-base with `bf0b73c` was `dda513b` rather than `bf0b73c`. The mandated HEAD assertion passed (agent-branch namespace, no protected ref), so the prescribed `git reset --hard bf0b73c` was applied to a clean working tree — nothing was lost.
- **`protoc` is not on PATH** (unchanged from 03-03). Used the build-wired in-tree host tool resolved from `cmake/CommonBuildParameters.cmake` + `THIRDPARTY_BUILD_DIR` (`.../thirdparty/build/OSX/Debug/protobuf/bin/protoc`, symlink → `protoc-34.0.0`) — no system/homebrew protoc.
- **`dart format` reformatted 2 of the 4 touched files** (`members_cubit.dart` map-unmodifiable wrap, `shell_cubits_test.dart` long test signatures). Both re-verified clean afterwards.

## Verification

- **`cd src/app && dart analyze --fatal-infos lib test`** → `No issues found!` (run after the codegen prerequisite was restored; scope-limited to `lib test` per the plan)
- **`cd src/app && flutter test test/cubits/shell_cubits_test.dart`** → `+39: All tests passed!` (32 pre-existing + 7 new)
- **`cd src/app && flutter test`** (full app suite, per the "run all dependent tests" rule) → `+67 ~1: All tests passed!`
- **Acceptance greps (all matched):** `class MemberRecord` ×1; `class MemberList|class InviteLink|class InviteRecord` ×3; `class Role|class PendingAction` ×2 (pbenum); `hasMemberList|hasInviteLink` ×2; `selfAddress` ×5 (pb.dart); `class MembersCubit` ×1, `setMembers|byScope|MemberInfo|selfAddress` ×25 (members_cubit.dart); `membersCubit` ×11, `hasMemberList` at line 502, `event.memberList.selfAddress` at line 509, `hasInviteLink`/`pendingInviteLink` ×4, `approvedBy|pendingAction` ×11 (rail_cubit.dart); `selfAddress` ×3, `pendingAction` ×7 (test file).
- **Dispatch ordering verified:** `hasMemberList` (502) and `hasInviteLink` (513) precede the unchanged `hasError` arm (519).
- **No deletions:** `git diff --diff-filter=D --name-only HEAD~1 HEAD` is empty; working tree clean, no untracked files.

## Known Stubs

None — every field wired reads the pushed `MemberList`/`SpaceTree` records; no placeholder values, hardcoded empty rosters, or unwired data sources were introduced.

## Threat Flags

None beyond the plan's `<threat_model>`. Both new dispatch arms store pushed authority verbatim (`MemberList.selfAddress`, `InviteLink.url`) and add no new trust boundary: T-04-06-EoP stays `accept` (no permission logic added to Dart) and T-04-06-InfoDisc stays `accept` (the invite link and pending fields were already pushed on the shared events).

## Next Phase Readiness

- Plan 04-07 can read `state.byScope[scopeId]` for the roster rows, `member.self` for the "You" marker, `member.pending`/`member.pendingLabel` for the D-07 badge, `pendingInviteLink` for the invite view, and `RailSpace`/`RailRoom.pendingAction`+`approvedBy` for the rail "Pending deletion" badge — all from pushed state.
- `roleLabel`/`kRoleLabels` plus the five `kRoleLabel*` constants are the single home for the five-tier display strings; 04-07 should import them rather than re-inlining `'Super Admin'` etc.
- Dart remains a thin subscriber (D-04/D-08): no roster synthesis, no permission enforcement, no CRDT reads.

---

*Phase: 04-membership-invites*
*Completed: 2026-10-02*

## Self-Check: PASSED

- FOUND: `src/app/lib/cubits/members_cubit.dart`
- FOUND: `src/app/lib/generated/proto/gcs_chat.pb.dart`
- FOUND: `src/app/lib/cubits/session_cubit.dart`
- FOUND: `src/app/lib/cubits/rail_cubit.dart`
- FOUND: `src/app/test/cubits/shell_cubits_test.dart`
- FOUND: `fd1df94` (Task 1)
- FOUND: `b828ae3` (Task 2)
