---
phase: 02-spaces-rooms
plan: 05
subsystem: dart-contract-bridge
tags: [flutter, rail, spaces-tree, d-05, d-04-dimming, widget-tests, scaffold-atoms]

# Dependency graph
requires:
  - phase: 02-spaces-rooms plan 03
    provides: RailState tree (spaces/standaloneRooms/rooms/activeRoom/treeReceived) + RailCubit.setTree
  - phase: 02-spaces-rooms plan 04
    provides: FROZEN showSpaceRoomDialog({context, transport, mode, space, parent}) + SpaceRoomDialogMode
provides:
  - Tree-rendering RoomRail: persistent toolbar "+", expandable space nodes (Private badge, edit/"+" affordances, view-local expansion), standalone Rooms section hidden when empty, joined/unjoined dimming, loading skeleton until first SpaceTree
  - 11/11 chat_shell widget tests covering every rail state incl. the criterion-3 dim/un-dim observable at the widget level
  - Phase 02 complete: CORE-01/CORE-02/CORE-03 user-visible surface shipped end to end (C++ catalog -> FFI push -> RailState -> rail)
affects: [Phase 03+ (messaging renders into rooms selected from the tree; rail selection contract unchanged)]

# Tech tracking
tech-stack:
  added: [] # zero new packages (T-02-SC: audit holds)
  patterns:
    - "View-local expansion as a collapsed-ids map (absent key = expanded) on the RoomRail State — new spaces default open, keyed by space id so expansion survives setTree full replacements, and never enters RailState (RailState mirrors pushed C++ truth only)"
    - "One _TreeRoomRow for nested AND standalone rows (joined/selected lookup + leadingInset parameter) — standalone rows sit at base space6 indent and are permanently dimmed emergently (never joined in Phase 2), not via a special case"
    - "ScaffoldDisclosure motion vocabulary composed directly (AnimatedSize medium + AnimatedRotation short, zero under reducedMotion) because the atom's String title slot cannot carry the badge/trailing actions"

key-files:
  created:
    - .planning/workstreams/app/phases/02-spaces-rooms/02-05-SUMMARY.md
  modified:
    - src/app/lib/shell/room_rail.dart
    - src/app/test/chat_shell_test.dart

key-decisions:
  - "Expansion state stores collapsed space ids (absent = expanded): default-open for newly pushed spaces falls out of the map lookup, and setTree full replacements cannot lose per-space expansion"
  - "Space-node row uses Flexible(name) + Spacer so the edit/\"+\" affordances stay end-aligned while the name ellipsizes; nested room rows keep the Phase 1 Expanded(text) row contract with one extra space6 leading inset"
  - "Unjoined rows pass onPressed to the same selectRoom path and gate with ScaffoldPressable(disabled: true) — the disabled overlay/Semantics is the first line of defense, the RailCubit joined-only selectRoom guard the second (T-02-11 defense in depth)"

requirements-completed: [CORE-01, CORE-02, CORE-03]

# Metrics
duration: 3min
completed: 2026-09-19
---

# Phase 2 Plan 5: Rail Spaces/Rooms Tree Summary

**RoomRail grown flat -> tree: persistent toolbar with accent "+", expandable space nodes (Private badge, edit/"+" wired to the frozen 02-04 dialog), standalone Rooms section hidden when empty, disabled/dimmed unjoined rows, loading skeleton until the first SpaceTree — 11/11 widget tests green, analyze clean, zero new packages**

## Performance

- **Duration:** 3 min
- **Started:** 2026-09-19T19:43:52Z
- **Completed:** 2026-09-19T19:46:49Z
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments
- `room_rail.dart` rewritten to the UI-SPEC rail contract: toolbar ("Spaces" titleSmall/textSecondary/0.5 label + 40px circular accent `ScaffoldSurface` "+" at 48px hit area, semanticLabel "Create space or room") persists in every rail state and opens `showSpaceRoomDialog(mode: createFromHeader)` with `context.read<SessionCubit>()` as transport
- Space nodes: whole-row tap toggles view-local expansion (AnimatedSize at `ScaffoldMotionDurations.medium` + chevron AnimatedRotation at `short`, zero-duration under `ScaffoldMotion.reducedMotion`); row = chevron -> Flexible name (Body, ellipsized, textPrimary) -> "Private" `ScaffoldBadge` (textSecondary fill, only when `!isPublic`) -> Spacer -> edit ("Edit space \<name\>") and "+" ("New room in \<name\>") 40px affordances at textSecondary wired to `editSpace`/`createRoomInSpace` modes
- Room rows (nested at one space6 indent beyond the space row, standalone at base space6): joined+selected = `btnFilterSelected` tint/radiusMd/textPrimary; joined-unselected = textSecondary, tap -> `selectRoom(topic)`; catalog-not-joined = `ScaffoldPressable(disabled: true)` (0.40 overlay + `Semantics(enabled: false)`, in-atom) — the T-02-11 mitigation on top of the cubit's joined-only guard; expanded empty space renders the inline non-interactive "No rooms yet" row
- Standalone Rooms section: `space12` break + header in the exact toolbar-label style, rendered ONLY when `standaloneRooms.isNotEmpty` (no empty-section noise)
- Startup states: `!treeReceived` -> `ScaffoldStateView(state: 'loading')` below the still-visible toolbar; received-but-empty catalog -> "No spaces yet" empty variant with accent "Create space" `emptyAction` pill reopening the createFromHeader dialog
- Tests: 5 new rail-tree cases (loading skeleton + toolbar, no-spaces CTA, Private-badge-on-private-only, nested + standalone sections with Rooms-header-hidden assert, unjoined-disabled -> setRooms un-dim -> tap selects) added to chat_shell_test.dart; all 6 pre-existing shell tests re-driven through the tree (see Deviations) — 11/11 in-file, 45 passed / 1 pre-existing skip full-suite, `flutter analyze lib test` clean

## Task Commits

Each task was committed atomically:

1. **Task 1: Render the spaces/rooms tree rail** - `f6792d1` (feat)
2. **Task 2: Rail tree widget tests** - `d5a078a` (test)

**Plan metadata:** (see final commit below)

## Files Created/Modified
- `src/app/lib/shell/room_rail.dart` - tree rail: toolbar, space nodes with badge/affordances/expansion, nested + standalone rows with dimming, loading/empty states (`roomDisplayName`/`kWidth`/`kHeaderLetterSpacing` kept; `roomDisplayName` still feeds the composer hint in gcs_shell.dart)
- `src/app/test/chat_shell_test.dart` - 5 rail-tree tests + `_seedJoinedRooms` tree/RoomList seeding helper; 6 existing tests updated to the tree surface

## Decisions Made
- Expansion map stores collapsed ids (absent = expanded) rather than expanded ids: default-open for newly pushed spaces falls out of the lookup, and being keyed by space id it automatically survives `setTree` full replacements without diffing
- Space-node trailing affordances are 40px circular textSecondary icon areas (no fill) — the accent reserved list allows accent ONLY on the header "+" (item 6); per-node icons at accent would break the 10% discipline as the catalog grows
- One `_TreeRoomRow` serves nested and standalone rows via a `leadingInset` parameter; standalone rows sit at the base space6 row indent (top-level rows, matching the UI-SPEC structure) and render dimmed emergently because standalone rooms are never in the pushed RoomList in Phase 2 (derived joins require a parent space)
- The empty-state `emptyBody` copy is a single unsplit string literal ("Spaces organize your rooms. Create your first space to get started.") to satisfy both the verbatim-copy contract and the plan's exact-string acceptance grep; dart format cannot split string literals so it stays intact

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Pre-existing shell tests broke under the tree rail**
- **Found during:** Task 2 (acceptance: `flutter test test/chat_shell_test.dart` exits 0)
- **Issue:** Six pre-existing tests drove the rail through the Phase 1 flat `setRooms` path and asserted flat-list rendering (e.g. `find.text('No rooms yet')` on a fresh cubit, 'smoke-test' rows from topic names). After Task 1 the rail renders the pushed tree — a fresh cubit now shows the loading skeleton, and row labels come from `RailRoom.name`, never topic-derived names
- **Fix:** Re-drove tests 1-4 and 6 through a `_seedJoinedRooms` helper (one space + named rooms via `setTree`, joined via `setRooms`); test 1's rail assert now checks the toolbar + loading state; test 2 retitled 'pushed tree + room list render rail rows (D-02/D-21)'. Test intent (selection projection, readiness gating, send publishing) unchanged — only the seeding surface moved
- **Files modified:** src/app/test/chat_shell_test.dart
- **Verification:** 11/11 in-file tests pass; full suite 45 passed / 1 pre-existing skip
- **Committed in:** d5a078a

---

**Total deviations:** 1 auto-fixed (bug)
**Impact on plan:** None on the rail contract — the plan's Task 2 acceptance (whole file green) required following through on the tests the Task 1 rewrite necessarily invalidated.

## Issues Encountered
None.

## User Setup Required
None - no external service configuration required.

## Known Stubs
None — every rail section renders from pushed RailState (SpaceTree/RoomList); every affordance opens the real frozen dialog with the SessionCubit transport. No placeholder data paths exist.

## Threat Flags
None — no new surface beyond the plan's threat model. T-02-11 mitigated as registered: unjoined rows are `ScaffoldPressable(disabled: true)` (in-atom ScaffoldDisabledOverlay 0.40 + `Semantics(enabled: false)`) layered on the untouched `selectRoom` joined-only guard; T-02-SC holds (zero new packages).

## Next Phase Readiness
- Phase 02 is complete end to end: C++ CRDT catalog -> SpaceTree push -> RailCubit tree -> tree rail with create/edit affordances; the criterion-3 autoJoinRooms dim/un-dim is observable and widget-test-proven
- Phase 03 (messaging) consumes the unchanged `activeRoom` selection contract; the composer hint and send path are untouched
- Space-node expansion is view-local and non-persistent by design (resets to expanded on app restart) — acceptable per UI-SPEC; persistence would be a future product decision

## Self-Check: PASSED

Both task commits verified in git log (f6792d1, d5a078a); both modified files verified on disk; this SUMMARY present in the plan directory.

---
*Phase: 02-spaces-rooms*
*Completed: 2026-09-19*
