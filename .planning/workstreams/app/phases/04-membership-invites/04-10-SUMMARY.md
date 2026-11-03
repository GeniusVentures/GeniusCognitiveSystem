---
phase: 04-membership-invites
plan: 10
subsystem: app-membership-ui
tags: [dart, flutter, protobuf, cubit, invite-link, scope-authority, wr-07]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: "04-09 InviteLink.scope_id and InviteRecord.is_space/authenticator wire fields"
provides:
  - "Regenerated Dart bindings exposing InviteLink.scopeId, InviteRecord.isSpace/isAuthenticator"
  - "Scope-keyed pending invite link in SessionCubit (pendingInviteLinkFor/clearPendingInviteLink)"
  - "Scope-correct invite view and clear-on-close pending-link lifecycle (WR-07)"
affects: [04-11, 04-12, wave-6 push consumers]

tech-stack:
  added: []
  patterns:
    - "Pushed authority value keyed by its C++-stamped scope id, never URL-parsed in Dart"
    - "Clear-on-close (not clear-on-read) so the push emission still delivers the link"

key-files:
  created: []
  modified:
    - src/app/lib/generated/proto/gcs_chat.pb.dart
    - src/app/lib/generated/proto/gcs_chat.pbjson.dart
    - src/app/lib/cubits/session_cubit.dart
    - src/app/lib/shell/members_dialog.dart
    - src/app/test/cubits/shell_cubits_test.dart
    - src/app/test/shell/members_dialog_test.dart

key-decisions:
  - "pendingInviteLinkFor(scopeId) replaces the unkeyed getter; the only readers were the invite view and its tests, so no second source of truth remains"
  - "A pushed InviteLink with an empty scope_id is skipped rather than filed under an empty key"
  - "The pending link is cleared from ResponsiveDrawer.show's onClose, not on read, so the push's own emission can still render the link"
  - "The invite view reads only its own scope's entry; another scope's link is unreachable and the view stays in its loading sub-state"

requirements-completed: [MEMB-01, MEMB-05]

# Metrics
duration: 2min
completed: 2026-10-18
---

# Phase 4 Plan 10: Scope-Keyed Invite Links Summary

**Scope-keyed invite rendering end to end: regenerated Dart bindings for `InviteLink.scope_id`, a per-scope pending-link map, scope threading through the invite drawer, and clear-on-close**

## Performance

- **Duration:** ~2 min
- **Started:** 2026-10-18T21:09:36Z
- **Completed:** 2026-10-18T21:11:24Z
- **Tasks:** 3
- **Files modified:** 6

## Accomplishments

- Regenerated the Dart protobuf bindings with the pinned `protoc_plugin 22.5.0`; `InviteLink.scopeId`, `InviteRecord.isSpace`, and `InviteRecord.authenticator` are exposed with no `BuilderInfo.aE` output.
- `SessionCubit` now stores pushed invite links in a scope-keyed map and exposes `pendingInviteLinkFor(scopeId)` / `clearPendingInviteLink(scopeId)`; the unkeyed accessor was removed because only the invite view and its tests read it.
- The invite drawer threads `scopeId` from `_startInvite` through `_showInviteView` into `_InviteLinkView`, so a link minted for scope A can never render in scope B's view (WR-07 / T-04-10-01).
- The pending entry is dropped from the drawer's `onClose`, so no stale capability survives the view (T-04-10-02).
- Full Dart suite passes (80 tests, 1 skipped); `flutter analyze lib test` is clean.

## Task Commits

Each task was committed atomically:

1. **Task 1: Regenerate Dart bindings with invite authority fields** - `e190113` (feat)
2. **Task 2: Key pending invite links by scope in SessionCubit** - `ca516b2` (feat)
3. **Task 3: Thread scope into the invite view and clear on close** - `42bb9d1` (feat)

## Files Created/Modified

- `src/app/lib/generated/proto/gcs_chat.pb.dart` - Regenerated `InviteLink.scopeId`, `InviteRecord.isSpace`, `InviteRecord.authenticator`
- `src/app/lib/generated/proto/gcs_chat.pbjson.dart` - Regenerated descriptor for the same fields
- `src/app/lib/cubits/session_cubit.dart` - Scope-keyed `_pendingInviteLinks`, `pendingInviteLinkFor`, `clearPendingInviteLink`, scope-guarded dispatch arm
- `src/app/lib/shell/members_dialog.dart` - `_showInviteView`/`_InviteLinkView` take `scopeId`; `onClose` clears the pending link
- `src/app/test/cubits/shell_cubits_test.dart` - Scope-keyed hold, cross-scope isolation, and clear cases
- `src/app/test/shell/members_dialog_test.dart` - Own-scope render, cross-scope non-render, and clear-on-close cases

## Decisions Made

- The unkeyed `pendingInviteLink` getter was deleted rather than kept as a convenience: a repo-wide grep found only the invite view and the shell cubit tests as readers, so the scope-keyed pair is the single source of truth (invariant 11).
- `clearPendingInviteLink` emits only when an entry was actually removed; a no-op close cannot spuriously rebuild unrelated invite views.
- The dispatch arm ignores a pushed link with an empty `scope_id` instead of storing it under `''`.
- `InviteLink.scopeId` is the only scope authority in Dart; no URL parsing was added (T-04-10-03).

## Deviations from Plan

None - plan executed exactly as written.

**Note:** Task 1 listed `gcs_chat.pbenum.dart` among the regenerated files, but the pinned plugin emitted no change for it because no enum changed; only `.pb.dart` and `.pbjson.dart` differ. This is expected regeneration output, not a deviation.

## Issues Encountered

- **Full `flutter analyze` reports 50 pre-existing errors in the `src/app/scaffold` submodule example tests** (`scaffold/example/test/capture_images_test.dart`, missing demo classes). The submodule is untouched by this plan and `flutter analyze lib test` for the app reports no issues. Out of scope per the deviation scope boundary.
- No signing or tooling failures; the pinned global plugin was activated at 22.5.0 before regeneration.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- WR-07 can be re-verified as PASS: the invite view renders the link minted for its own scope and no stale link survives the drawer.
- The Dart side now consumes `InviteLink.scope_id` exactly as C++ emits it; no URL-based scope inference exists.
- No new dependency was added and the generated bindings were not hand-edited.

---

*Phase: 04-membership-invites*
*Completed: 2026-10-18*

## Self-Check: PASSED

- All 6 modified files and this SUMMARY exist on disk.
- Commits `e190113`, `ca516b2`, and `42bb9d1` exist in the repository history.
- Full Dart suite passed (80 tests, 1 skipped); `flutter analyze lib test` clean; `BuilderInfo.aE` count is 0.
