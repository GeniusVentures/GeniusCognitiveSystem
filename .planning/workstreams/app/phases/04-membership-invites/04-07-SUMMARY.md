---
phase: 04-membership-invites
plan: 07
subsystem: ui
tags: [dart, flutter, cubit, protobuf, membership, invites, redeem, approve, dialogs, rail]

# Dependency graph
requires:
  - phase: 04-membership-invites
    plan: 06
    provides: MembersCubit/MemberInfo/MembersState.byScope, SessionCubit memberList/inviteLink dispatch arms, pendingInviteLink, RailSpace/RailRoom approvedBy/pendingAction, the five kRoleLabel* constants
  - phase: 04-membership-invites
    plan: 01
    provides: InviteCommand/RedeemCommand/ApproveCommand/DeleteScopeCommand/ChangeRoleCommand/RemoveMemberCommand/LeaveCommand oneof arms, PendingAction enum
  - phase: 02-spaces-rooms
    provides: ResponsiveDrawer.show dialog pattern, space_room_dialog.dart scaffold-atom idiom, RoomRail tree + _NodeIconAffordance
provides:
  - members_dialog.dart (roster, invite view, role change, remove/leave/delete confirms, approve publish)
  - join_dialog.dart (paste-invite redeem flow with inline link-shape validation)
  - room_rail.dart Join/Members/Delete affordances, pending-deletion surface, Super-Admin approve-for-delete
  - gcs_shell.dart MembersCubit provider wiring (optional injection, owned by default)
  - MembersCubit.shared presentation gates (viewerOf, viewerCanManageMembers, viewerIsSuperAdmin, viewerCanDeleteScope) + kPendingDeletionLabel
affects: [04-08]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Publish-once dialogs: private `_DialogForm extends ChangeNotifier` + `_submitHandled` guard, so a double tap cannot double-publish (Phase 2 D-05 discipline)"
    - "One home for a shared publish: the rail's approve-for-delete and the roster row's approve both call members_dialog.approvePendingOp"
    - "Role gates live in members_cubit.dart as top-level predicates over the pushed role label; rail/dialog only read them (D-08 presentation-only)"
    - "Badges render on their own line under a rail node (_NodeMetadataLine) so the 280px header row always fits its 48px touch targets"

key-files:
  created:
    - src/app/lib/shell/members_dialog.dart
    - src/app/lib/shell/join_dialog.dart
    - src/app/test/shell/members_dialog_test.dart
    - src/app/test/shell/join_dialog_test.dart
  modified:
    - src/app/lib/shell/room_rail.dart
    - src/app/lib/shell/gcs_shell.dart
    - src/app/lib/cubits/members_cubit.dart
    - src/app/lib/cubits/rail_cubit.dart
    - src/app/lib/cubits/session_cubit.dart
    - src/app/lib/cubits/message_flow_cubit.dart
    - src/app/test/chat_shell_test.dart

key-decisions:
  - "Rail node badges (Private / Pending deletion) render on their own metadata line rather than inside the header trailing cluster: a text badge plus three 48px affordances cannot fit the rail's 256px content width, so the cluster would overflow (see Deviations)"
  - "viewerCanDeleteScope is the one home for 'a dormant deletion offers approve, not a second Delete' -- mirrors the member-row rule and bounds the node cluster to four affordances"
  - "The approve-for-delete publish reuses members_dialog.approvePendingOp so the command envelope + failure toast have a single home (invariant 11); room_rail only supplies the scope id as the target"
  - "Member-row address display reuses MessageFlowCubit's truncateSender (made public in Task 1) -- one home for the short-address convention"
  - "session_cubit emits a fresh copyWith() after storing a pushed invite link so the invite view rebuilds on the push instead of showing its loading sub-state forever"

requirements-completed: [MEMB-01, MEMB-02, MEMB-04, MEMB-05]

# Metrics
duration: ~25min
completed: 2026-10-02
---

# Phase 4 Plan 07: Membership UI Surfaces Summary

**Member list, invite view, join/redeem dialog, and the rail's Join/Members/Delete + pending-deletion and Super-Admin approve-for-delete affordances -- every surface a dumb read of pushed state that publishes data-only `GcsCommand` envelopes, with the rail node badges moved to a metadata line so the 280px header row fits its 48px touch targets.**

## Performance

- **Duration:** ~25 min (two executor sessions; the first was cut short mid-Task-2 and this session finished Tasks 2-3)
- **Started:** 2026-10-02T20:12:00Z
- **Completed:** 2026-10-02T20:37:00Z
- **Tasks:** 3
- **Files modified:** 11 (4 created, 7 modified)

## Accomplishments

- `members_dialog.dart` (created): `showMembersDialog(...)` opens a `ResponsiveDrawer` whose `BlocBuilder<MembersCubit, MembersState>` renders `state.byScope[scopeId]` as loading / empty ("No members yet") / one row per `MemberInfo` -- truncated address, "You" marker, "Pending removal" / "Pending: <role>" warning badge, five-tier role badge (accent only for Super Admin) -- with the gated trailing affordances (change role, remove, approve) and the footer ("Leave <name>", Cancel, accent "Invite members"). Invite view, role-change selector, and the remove/leave/delete `statusError` confirmations are modes of the same drawer; every action publishes a data-only envelope and `approvePendingOp` is the single home for the approve publish + `kErrorCouldNotApprove` toast.
- `join_dialog.dart` (created): `showJoinDialog(...)` with one autofocused field (hint "Paste invite link"), inline `gcs://invite/{space|room}/<id>?key=<key>` shape validation ("Enter a valid invite link." keeps the drawer open with no publish), and an accent "Join" pill that publishes `RedeemCommand(inviteUrl:)` and closes; a refused publish keeps the drawer open with the "Couldn't join" toast.
- `room_rail.dart`: toolbar Join affordance left of the accent "+"; space-node and standalone-row Members affordances; role-gated Delete (`statusError`); the pushed pending-deletion badge; and the Super-Admin approve-for-delete affordance -- all rendered from the pushed `RailSpace`/`RailRoom` plus the pushed roster via `BlocBuilder<MembersCubit, MembersState>`.
- `gcs_shell.dart`: `GCSChat` takes an optional `membersCubit`, threads the owned one into `SessionCubit.openDefault(membersCubit:)`, provides it via `BlocProvider<MembersCubit>.value`, and closes it with the shell when owned.
- 16 new widget tests across `members_dialog_test.dart` (5), `join_dialog_test.dart` (2), and the extended `chat_shell_test.dart` (rail Join/Members affordances + provided `MembersCubit`).

## Task Commits

Each task was committed atomically:

1. **Task 1: Build members_dialog.dart -- roster, invite view, role-change, and destructive confirmations** - `8474c7c` (feat)
2. **Task 2: Build join_dialog.dart and add the rail Join/Members/Delete affordances plus the pending-deletion surface** - `ad61340` (feat), layout correction in `ff6753e` (feat)
3. **Task 3: Wire MembersCubit into gcs_shell.dart and add widget tests** - `a753548` (feat, shell wiring) + `5982f39` (test, widget tests)

## Files Created/Modified

- `src/app/lib/shell/members_dialog.dart` (created) — roster/invite/role-change/confirm modes, `approvePendingOp`, the `_DialogForm` publish-once discipline
- `src/app/lib/shell/join_dialog.dart` (created) — paste-invite redeem flow with the link-shape validator
- `src/app/lib/shell/room_rail.dart` (modified) — Join/Members/Delete affordances, approve-for-delete, pending-deletion surface, `_NodeMetadataLine`
- `src/app/lib/shell/gcs_shell.dart` (modified) — `membersCubit` field, own-vs-injected lifecycle, `openDefault` pass-through, provider entry
- `src/app/lib/cubits/members_cubit.dart` (modified) — `viewerOf`, `viewerCanManageMembers`, `viewerIsSuperAdmin`, `viewerCanDeleteScope`, `kPendingDeletionLabel`
- `src/app/lib/cubits/rail_cubit.dart` (modified) — `RailSpace`/`RailRoom.membersCanInvite` surfaced from the pushed records
- `src/app/lib/cubits/session_cubit.dart` (modified) — emit a fresh state after storing a pushed invite link so the invite view rebuilds
- `src/app/lib/cubits/message_flow_cubit.dart` (modified) — `truncateSender` made public (shared address display rule)
- `src/app/test/shell/members_dialog_test.dart` (created) — 5 tests
- `src/app/test/shell/join_dialog_test.dart` (created) — 2 tests
- `src/app/test/chat_shell_test.dart` (modified) — rail affordance + provided-cubit test

## Decisions Made

- **Rail node badges live on their own metadata line.** Forced by the rail's width budget; see Deviations #1 for the arithmetic. The badge copy and warning colour are unchanged, and both badges stay visible.
- **`viewerCanDeleteScope` rather than an inline conjunction.** The "a dormant deletion offers approve, not a second Delete" rule is stated once (mirroring the member-row presence table) and is read by both the space node and the standalone row.
- **The rail's approve-for-delete calls `members_dialog.approvePendingOp`.** The command envelope (`ApproveCommand(scopeId:, target:)`) and its failure toast have one home; the rail supplies `target: <scope id>` per the plan.
- **`truncateSender` is shared, not re-implemented.** The member-row address renders exactly like the Phase 3 sender label, so the Phase 3 rule was promoted from private to public instead of copied.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Rail node badges moved to a metadata line under the node**

- **Found during:** Task 3 verification (`flutter test`)
- **Issue:** The plan places the "Pending deletion" badge in the space node's trailing cluster alongside the Members / Delete / approve / edit / "+" affordances. With 48px touch targets, the header `Row`'s fixed content exceeds the rail's usable width: `flutter test` failed with `A RenderFlex overflowed by 27 pixels on the right ... room_rail.dart:444` (constraints `0..256`, size `256x48`). The arithmetic is forced rather than a tuning problem -- under widget-test font metrics each glyph is 1em wide, so a 7-character "Private" badge is ~97px: chevron + gap (30) + badge (97) + three affordances (48+4+48+4+48 = 152) = 283 > 256. Even with all padding and inter-affordance gaps removed a text badge plus three affordances does not fit, and in production a 16-character "Pending deletion" badge plus four affordances is ~340px.
- **Fix:** Added `_NodeMetadataLine` and render the Private and Pending-deletion badges on their own indented line immediately under the space node's header (and under a standalone room's row). Added `MembersCubit.viewerCanDeleteScope` so a dormanted deletion shows approve instead of a second Delete -- which also bounds the header cluster to four affordances. All affordances keep their 48px hit targets and every badge stays visible; the badge copy, colours, and role gating are unchanged.
- **Files modified:** `src/app/lib/shell/room_rail.dart`, `src/app/lib/cubits/members_cubit.dart`
- **Commit:** `ff6753e`

**2. [Rule 3 - Blocking] The members-dialog approve assertion uses the pressable's accessible name**

- **Found during:** Task 3 (`flutter test`) -- `find.bySemanticsLabel('Approve removal of 0xbeef')` found 0 widgets
- **Issue:** The drawer's roster scroll node merges all descendant semantics into one label (dumping the semantics tree showed a single node labelled `"0xself\nYou\n0xbeef\nApprove removal of 0xbeef"`), so an exact-label finder can never isolate an affordance inside this drawer. The label itself is produced correctly by the shipped `ScaffoldPressable`; only the finder is unusable here.
- **Fix:** Assert `ScaffoldPressable.semanticLabel == 'Approve removal of 0xbeef'` via `find.byWidgetPredicate`, with a comment recording why. The same property is verified (an icon-only affordance carries an accessible name, WCAG 4.1.2).
- **Files modified:** `src/app/test/shell/members_dialog_test.dart`
- **Commit:** `5982f39`

### Plan-wording notes (no behaviour impact)

- **`grep "ApproveCommand" room_rail.dart` does not match; the OR-pair does.** The rail's approve-for-delete publishes through the shared `approvePendingOp` in `members_dialog.dart` (one home for the envelope), so the literal `ApproveCommand` lives there. The plan's acceptance line is `grep -n "ApproveCommand\|Icons.check_circle_outline" src/app/lib/shell/room_rail.dart`, which matches via `Icons.check_circle_outline` (3 occurrences).
- **`grep "Pending deletion" room_rail.dart` does not match; the OR-pair does.** The copy is owned by `kPendingDeletionLabel` in `members_cubit.dart` (per plan 04-06's "no inlined display strings" decision); the rail references the constant. The acceptance line's OR matches via `statusWarningText` (2 occurrences).
- **Files outside the plan's `files_modified` list were touched.** `members_cubit.dart` (presentation gates), `rail_cubit.dart` (`membersCanInvite`), `session_cubit.dart` (invite-link emit), and `message_flow_cubit.dart` (`truncateSender`) carry shared rules the new surfaces must read. Extending the owners rather than inlining keeps invariants 10/11; no behaviour of the existing surfaces changed.

## Issues Encountered

- **Inherited uncommitted Task-2 edits.** The worktree arrived with uncommitted changes to `members_cubit.dart`, `rail_cubit.dart`, `members_dialog.dart`, `room_rail.dart` plus an untracked `join_dialog.dart`. Reviewed with `git diff`, then salvaged: `join_dialog.dart` was complete and correct; the `viewerOf` extraction, `membersCanInvite` surfacing, and the `members_dialog` shared-gate swap were kept. A leftover unused `toast_manager.dart` import in `room_rail.dart` (the toast moved into `approvePendingOp`) was removed.
- **`members_dialog.dart`'s approve publish was de-duplicated.** The private `_approve` became the public `approvePendingOp` so the rail and the roster row publish through one function.

## Verification

- **`cd src/app && dart analyze --fatal-infos lib test`** → `No issues found!` (exit 0)
- **`cd src/app && flutter test test/shell/members_dialog_test.dart test/shell/join_dialog_test.dart test/chat_shell_test.dart`** → `+19: All tests passed!` (plan's Task 3 verify command)
- **`cd src/app && flutter test`** (full app suite) → `+75 ~1: All tests passed!`
- **Acceptance greps (all matched):** `showMembersDialog|MembersDialog` (members_dialog.dart); `Invite members|Anyone with this link joins as a Guest.|Invite link copied|No members yet|Pending removal|Pending: ` (members_dialog.dart); `ScaffoldBadge` (members_dialog.dart); `InviteCommand|ChangeRoleCommand|RemoveMemberCommand|LeaveCommand|DeleteScopeCommand|ApproveCommand` (members_dialog.dart); `showJoinDialog` + `Paste invite link|Enter a valid invite link.|RedeemCommand` (join_dialog.dart); `Icons.person_add_outlined` ×2, `Icons.group_outlined` ×3, `Icons.delete_outline` ×3, `statusWarningText` ×2, `Icons.check_circle_outline` ×3, `showMembersDialog` + `showJoinDialog` (room_rail.dart); `MembersCubit` ×5 and `membersCubit: _membersCubit` (gcs_shell.dart).
- **No file deletions:** `git log --diff-filter=D --name-only` over the plan's commits is empty; working tree clean with no untracked files after the final commit.

## Known Stubs

None -- every rendered value is read from the pushed `MemberList` / `SpaceTree` state or from the user's own input; no placeholder copy, hardcoded rosters, or unwired data sources were introduced.

## Threat Flags

None beyond the plan's `<threat_model>`. The new surfaces add no trust boundary: they publish data-only `GcsCommand` envelopes on the existing `GcsCommandTransport` seam (T-04-07-EoP stays `accept` -- role gating in Dart is presentation, the C++ guard is the enforcement) and add the planned inline invite-link shape validation (T-04-07-I `mitigate`), re-validated C++-side on redeem. The invite view renders and copies the pushed link only (T-04-07-InfoDisc `accept`).

## Next Phase Readiness

- The Phase 4 membership surface is complete: roster, invite view, join/redeem, role change, remove/leave/delete confirms, the rail entry points, and the pending-op approve affordances all read pushed state and publish data-only commands.
- `MembersCubit` is the single home for the role/pending presentation gates (`viewerOf`, `viewerCanManageMembers`, `viewerIsSuperAdmin`, `viewerCanDeleteScope`) and for the pending-copy constants -- future membership surfaces should import them rather than re-deriving role strings.
- `room_rail.dart` now requires a `MembersCubit` above it in the tree (the shell provides it); any future test harness that mounts `RoomRail` directly must supply one.

---

*Phase: 04-membership-invites*
*Completed: 2026-10-02*

## Self-Check: PASSED

- FOUND: `src/app/lib/shell/members_dialog.dart`
- FOUND: `src/app/lib/shell/join_dialog.dart`
- FOUND: `src/app/test/shell/members_dialog_test.dart`
- FOUND: `src/app/test/shell/join_dialog_test.dart`
- FOUND: `8474c7c` (Task 1)
- FOUND: `ad61340` (Task 2)
- FOUND: `ff6753e` (Task 2 layout correction)
- FOUND: `a753548` (Task 3 shell wiring)
- FOUND: `5982f39` (Task 3 tests)
