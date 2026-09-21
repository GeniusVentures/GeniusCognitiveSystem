---
phase: 02-spaces-rooms
plan: 04
subsystem: dart-contract-bridge
tags: [flutter, dialog, d-05, scaffold-atoms, gcs-command, widget-tests, scaffold-ticket]

# Dependency graph
requires:
  - phase: 02-spaces-rooms plan 03
    provides: regenerated Dart pb bindings (CreateSpaceCommand/CreateRoomCommand/UpdateSpaceCommand arms) + RailSpace model
  - phase: 02-spaces-rooms plan 02
    provides: GcsCommandTransport.publishCommand seam (verified by 02-02/02-03 tests)
provides:
  - FROZEN showSpaceRoomDialog({context, transport, mode, space, parent}) + SpaceRoomDialogMode enum — the exact entry contract Plan 02-05 rail affordances call
  - Create/edit dialog publishing createSpace / createRoom (incl. standalone empty-parent + room-in-space parent id) / updateSpace with prefills
  - Empty-name inline validation + publish-false error toast, 6/6 widget tests green
  - Scaffold-workstream contrast ticket (submodule commit 7760ee2) for TextEntryFieldWidget light-mode entered-text contrast
affects: [02-05 (rail toolbar/space-node affordances open this dialog; empty-state Create space CTA)]

# Tech tracking
tech-stack:
  added: [] # zero new packages (T-02-SC: audit holds)
  patterns:
    - "Dialog form state in a private ChangeNotifier shared by the children/footer subtrees — ResponsiveDrawer.show mounts them as separate subtrees, so one State cannot rebuild both; disposal rides the drawer's onClose after both unmount"
    - "Confirm pill disables on raw-empty name only; whitespace-only input keeps it enabled so the inline 'Enter a name.' validation is reachable (disabled taps teach nothing)"
    - "Bool-radio pairs write their row's value from both the row pressable and the radio's own onChanged (never toggle) — coordinated pair, D-05"

key-files:
  created:
    - .planning/workstreams/app/phases/02-spaces-rooms/02-04-SUMMARY.md
    - src/app/lib/shell/space_room_dialog.dart
    - src/app/test/shell/space_room_dialog_test.dart
    - src/app/scaffold/.planning/workstreams/scaffold/TICKET-text-entry-field-light-mode-contrast.md # committed in the scaffold submodule (7760ee2)
  modified: []

key-decisions:
  - "Form state lives in a private _DialogForm ChangeNotifier (not a single StatefulWidget State) because ResponsiveDrawer.show mounts fields (children) and actions (footer) as separate subtrees; the form disposes via the drawer's onClose callback after both subtrees unmount"
  - "Confirm disables only while the name is raw-empty; whitespace-only input stays enabled and hits the inline 'Enter a name.' validation — the disabled overlay and the validation each do their own job (test-proven by EmptyNameDoesNotPublish)"
  - "Dialog title is static per open (the atom's title is a String): createFromHeader opens as 'New space' on the default Space type; the hint and confirm label — inside the form's subtrees — DO flip with the type selector"

requirements-completed: [CORE-01, CORE-02, CORE-03]

# Metrics
duration: 2min
completed: 2026-09-19
---

# Phase 2 Plan 4: Space/Room Create-Edit Dialog Summary

**One reusable D-05 dialog composed from scaffold atoms (ResponsiveDrawer + TextEntryFieldWidget + radio pairs + toggle + toast) publishing the correct GcsCommand arm per mode with zero local optimism — 6/6 widget tests green, analyze clean, scaffold contrast ticket filed**

## Performance

- **Duration:** 2 min
- **Started:** 2026-09-19T19:35:44Z
- **Completed:** 2026-09-19T19:37:47Z
- **Tasks:** 2
- **Files modified:** 3 (dialog, widget tests, scaffold ticket)

## Accomplishments
- `space_room_dialog.dart`: FROZEN `showSpaceRoomDialog({context, transport, mode, space, parent})` + `SpaceRoomDialogMode { createFromHeader, createRoomInSpace, editSpace }` shipped exactly as specified — the contract Plan 02-05 calls
- Mode → command mapping implemented and test-proven: createFromHeader publishes `createSpace(name, isPublic, autoJoinRooms)` or `createRoom(name, parentSpaceId: '')` per type selector; createRoomInSpace publishes `createRoom(parentSpaceId: parent.id)`; editSpace publishes `updateSpace(spaceId, name, isPublic, autoJoinRooms)` with all three fields prefilled from the passed `RailSpace`
- Behavior contract per UI-SPEC: empty/whitespace name → inline "Enter a name." under the field, dialog open, no publish; valid confirm → publish then immediate pop (no spinner, no SpaceTree wait); `publishCommand == false` → error toast ("Couldn't create space/room" / "Couldn't update space" + "The command didn't reach the chat core. Try again."), dialog stays open; no success toast (rail update is the confirmation); pushed ErrorNotice errors stay on the SessionState.error path (accepted deviation, plan note)
- Composition per D-05: `ResponsiveDrawer.show` (title/children/footer), `TextEntryFieldWidget` + `TextFormFieldLogic` (autofocus, `maxLength: 64`, Enter submits), two bool-coordinated `ScaffoldSelectionIndicatorRadio` pairs (type + visibility) with whole-row `ScaffoldPressable`s, `ScaffoldSelectionIndicatorToggle` + Body label + Label description at textSecondary, end-aligned footer with transparent Cancel and accent 40px pill confirm (`ScaffoldColors.btnText`, disabled while raw-empty), spacing `space4`/`space2` from dimens
- 6/6 dialog widget tests: CreateSpacePublishesCreateSpace (incl. drawer-closes assert), CreateStandaloneRoomPublishesEmptyParent (incl. config controls hidden), CreateRoomInSpacePublishesParentId, EditSpacePublishesUpdateSpace (incl. Private-radio/autoJoin-toggle prefill asserts), EmptyNameDoesNotPublish, PublishFalseShowsToast (incl. stays-open assert)
- Scaffold contrast ticket filed inside the submodule (`7760ee2`, docs commit on scaffold develop): TextEntryFieldWidget hardcodes `Colors.white` input text on `grayPrimary` fill (`text_entry_field_widget.dart:18-25`) — white-on-near-white on the light palette, entered-text-only, hint unaffected; fix owned by the scaffold workstream

## Task Commits

Each task was committed atomically:

1. **Task 1: Build space_room_dialog.dart + file contrast ticket** - `2534295` (feat, parent repo); ticket `7760ee2` (docs, scaffold submodule — gitlink bumps in this plan's final docs commit)
2. **Task 2: Dialog widget tests** - `36ea1b5` (test)

**Plan metadata:** (see final commit below)

## Files Created/Modified
- `src/app/lib/shell/space_room_dialog.dart` - the one create/edit dialog (D-05): frozen entry point, `_DialogForm` state, fields + footer subtrees
- `src/app/test/shell/space_room_dialog_test.dart` - 6 widget tests + `_RecordingTransport` double + `_pumpUntil` wait-condition helper
- `src/app/scaffold/.planning/workstreams/scaffold/TICKET-text-entry-field-light-mode-contrast.md` - scaffold-workstream handoff (committed in the submodule, not the parent repo)

## Decisions Made
- Form state lives in a private `_DialogForm extends ChangeNotifier` shared by the fields and footer widgets (both `ListenableBuilder`s) instead of a single private StatefulWidget State: `ResponsiveDrawer.show` mounts `children` and `footer` as two separate subtrees, so no single State can rebuild both; a State-mixed-in ChangeNotifier was rejected on disposal-order grounds (body unmounts before footer → dispose-while-subscribed assert). The form disposes via the drawer's `onClose`, which fires after the route's subtree fully unmounts
- The confirm pill disables only while the name is raw-empty (`text.isEmpty`); whitespace-only input keeps it enabled so confirm reaches the inline "Enter a name." validation — the disabled overlay communicates "type something", the validation explains "what you typed is empty after trim" (both surfaces keep a reachable job; `EmptyNameDoesNotPublish` drives exactly this path)
- The dialog title is static per open because the frozen `ResponsiveDrawer.show` API takes a `String` title; `createFromHeader` opens as "New space" (the default Space type) while the hint and confirm label — inside the form's subtrees — flip live with the type selector
- Radio rows write their row's value from both tap paths (row `ScaffoldPressable.onPressed` and the radio's own `onChanged`, whose bool argument is ignored): the atoms are bool-coordinated pairs, never toggles (tapping the already-selected "Public" radio must not flip to Private)

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Form state shape: ChangeNotifier instead of a StatefulWidget State**
- **Found during:** Task 1 (plan says "a private StatefulWidget holding the dialog's local form state")
- **Issue:** `ResponsiveDrawer.show` mounts the fields (`children`) and the action row (`footer`) as two separate widget subtrees; a single StatefulWidget State cannot rebuild the footer when the name/type changes (confirm label + disabled state must react to body state)
- **Fix:** View-local form state moved into a private `_DialogForm` ChangeNotifier owned by `showSpaceRoomDialog` and disposed via the drawer's `onClose`; both subtrees rebuild through `ListenableBuilder`. Semantics preserved exactly: view-local state, never in Cubits (UI-SPEC state contracts)
- **Files modified:** src/app/lib/shell/space_room_dialog.dart
- **Verification:** flutter analyze clean; all 6 widget tests green (footer disabled-state and label-flip behavior exercised by EmptyNameDoesNotPublish / CreateStandaloneRoomPublishesEmptyParent)
- **Committed in:** 2534295

---

**Total deviations:** 1 auto-fixed (blocking)
**Impact on plan:** None on contracts — the frozen `showSpaceRoomDialog` signature, behavior, and copy are exactly as planned.

## Issues Encountered
None.

## User Setup Required
None - no external service configuration required.

## Known Stubs
None — every mode publishes a real GcsCommand through the injected transport; no placeholder data paths exist. (Pre-existing, documented, ticketed limitation: TextEntryFieldWidget entered-text contrast on the light palette — scaffold ticket `7760ee2`.)

## Threat Flags
None — no new surface beyond the plan's threat model. T-02-09 mitigated as registered (`maxLength: 64` + inline validation client-side; C++ re-validates per Plan 02-02); T-02-10 unchanged (is_public stays display-only).

## Next Phase Readiness
- Plan 02-05 wires the rail toolbar "+", space-node "+", space-node edit, and the no-spaces empty-state CTA to `showSpaceRoomDialog` — the signature is frozen and test-pinned
- The toast-on-publish-false path is test-proven; pushed-rejection toast remains a Phase 3+ candidate per the plan's accepted-deviation note (SessionState.error carries pushed ErrorNotice today)

## Self-Check: PASSED

Task commits verified in git log (2534295, 36ea1b5 parent; 7760ee2 scaffold submodule); all three artifacts verified on disk; this SUMMARY present in the plan directory.

---
*Phase: 02-spaces-rooms*
*Completed: 2026-09-19*
