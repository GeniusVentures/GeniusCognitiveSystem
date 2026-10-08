---
phase: 02-spaces-rooms
fixed_at: 2026-09-21T22:25:48Z
review_path: .planning/workstreams/app/phases/02-spaces-rooms/02-REVIEW.md
iteration: 2
findings_in_scope: 5
fixed: 4
skipped: 1
status: partial
---

# Phase 02: Code Review Fix Report

**Fixed at:** 2026-09-19T20:45:00Z
**Source review:** .planning/workstreams/app/phases/02-spaces-rooms/02-REVIEW.md
**Iteration:** 1

**Summary:**
- Findings in scope: 11 (0 Critical, 4 Warning, 7 Info; --all scope)
- Fixed: 9
- Skipped: 2 (IN-03, IN-06)

All fixes were applied and committed in an isolated worktree
(`.rf-02-reviewfix-ANrwWv`, branch `gsd-reviewfix/02-96980`, fast-forwarded to
`develop`). One commit per finding, `fix(02): <ID> <description>` format.

**Verification evidence (run after every fix, and on the final state):**
- C++: `ninja -C build/OSX/Debug` clean; `ctest --test-dir build/OSX/Debug
  -R "gcs" -E "test_gcs_global_db_sdk"` — 100% (5/5 binaries) green,
  including two new regression tests in test_gcs_ffi_sdk
  (OverLengthNamesRejectedAcrossCreateArms, ExplicitJoinSurvivesAutoJoinToggleOff).
- Dart: `cd src/app && flutter test` — 47 passed, 1 skipped (baseline was
  45+1; +2 new widget tests: DoubleConfirmPublishesExactlyOnce,
  ModeParamsAssertOnMisuse).

## Fixed Issues

### WR-01: Documented C++ name-length re-validation does not exist

**Files modified:** `src/ffi/gcs_core_ffi.cpp`, `test/test_gcs_ffi_sdk.cpp`
**Commit:** df16d9e
**Applied fix:** Added `constexpr size_t kMaxEntityNameLength = 64` (mirror
of the Dart dialog kMaxNameLength, T-02-09) in the FFI anonymous namespace;
the create_space/create_room/update_space arms now reject names longer than
64 chars with `PostErrorNotice("... rejected: name exceeds maximum length")`
+ `GCS_ERROR_INVALID_ARGUMENT`, matching the existing empty-name rejection
shape. Regression test rejects 65-char names on all three arms and accepts
the 64-char boundary (asserted in the pushed SpaceTree). No proto changes.

### WR-02: Two-key create is partial-write orphan-prone

**Files modified:** `src/lib/gcs_entity_store.cpp`, `src/lib/gcs_entity_store.hpp`
**Commit:** b216bef
**Applied fix:** Reversed the write order in CreateSpace/CreateRoom —
AppendToManifest first, then the record Put, with a comment explaining the
failure direction (a failed record Put degrades to a manifest id that
LoadFromStore skips gracefully; never unreachable orphan record bytes).
Updated the class doc comment in the hpp that described the old order.
Verified LoadFromStore's missing-record skip remains the behavior; existing
reload/restart tests (test_gcs_entities, test_gcs_ffi_sdk persistence) stay
green. No new test: the public seam has no Put-failure injection point
(adding one would be an architectural change); the success path is fully
covered by existing persistence tests.

### WR-03: RefreshDerivedJoins silently revokes an explicitly-joined topic

**Files modified:** `src/ffi/gcs_core_ffi.cpp`, `test/test_gcs_ffi_sdk.cpp`
**Commit:** 5de4c12
**Applied fix:** Added `g_explicitTopics` (guarded by g_mutex); the
join_topic arm records successful explicit joins into it; the
RefreshDerivedJoins erase loop now skips topics present in g_explicitTopics
(explicit membership outranks derivation and survives autoJoinRooms
toggle-off). gcs_shutdown clears the new set. Corrected the function comment
that claimed explicit join_topic topics are "never touched here". Regression
test ExplicitJoinSurvivesAutoJoinToggleOff: derived topic explicitly joined
via join_topic stays in the pushed RoomList after update_space toggles
autoJoinRooms off (fails on the pre-fix code).

### WR-04: Dialog confirm is re-entrant (duplicate commands)

**Files modified:** `src/app/lib/shell/space_room_dialog.dart`,
`src/app/test/shell/space_room_dialog_test.dart`
**Commit:** bb600e6
**Applied fix:** Added `_submitHandled` one-shot guard to _DialogForm;
`submit()` returns early when already handled and sets the flag only on the
path that publishes and pops (a refused publish still allows retry).
Regression test DoubleConfirmPublishesExactlyOnce taps the confirm pill
twice back-to-back and asserts exactly one published command.

### IN-01: Orphaned rooms render as standalone (undocumented fallback)

**Files modified:** `src/app/lib/cubits/rail_cubit.dart`
**Commit:** 9b17eb0
**Applied fix:** Document-only: the setTree doc comment now states that a
room whose parent id matches no pushed space (e.g. tombstoned parent) also
degrades to standalone — graceful fallback so a live room is never dropped.
No behavior change; no test needed.

### IN-02: `_collapsedSpaceIds` never pruned across setTree replacements

**Files modified:** `src/app/lib/shell/room_rail.dart`
**Commit:** 90c45da
**Applied fix:** `_buildTree` now prunes collapse keys whose space id is
absent from the pushed `rail.spaces` (Map.removeWhere) before rendering.
Safe mid-build without setState: a pruned key cannot affect the current
frame because its space is not rendering. No dedicated test: the map is
private view-local state whose growth has no observable behavior for ids
still present (existing rail/chat_shell tests stay green).

### IN-04: Mode-required dialog params enforced only by doc

**Files modified:** `src/app/lib/shell/space_room_dialog.dart`,
`src/app/test/shell/space_room_dialog_test.dart`
**Commit:** 0d3f3f6
**Applied fix:** `showSpaceRoomDialog` now asserts
`createRoomInSpace => parent != null` and `editSpace => space != null` with
clear messages, so misuse fails loudly at the call site in debug builds
instead of crashing later in `dialogTitle`/`_buildCommand` null-checks.
Test ModeParamsAssertOnMisuse covers both omissions.

### IN-05: `gcs_init` idempotent path silently ignores a differing config

**Files modified:** `src/ffi/gcs_core.h`
**Commit:** 58f5c01
**Applied fix:** Document-only option chosen (rejecting mismatched config
would require retaining the original config bytes — new ABI-level state):
the gcs_init contract now documents that a second call while a session lives
returns the existing handle and IGNORES configBytes (first call's db
path/codec stay bound). Doc-only; C++ rebuild + suite green.

### IN-07: Test nits — unused include and POSIX-only path

**Files modified:** `test/test_gcs_entities.cpp`,
`src/app/test/cubits/shell_cubits_test.dart`
**Commit:** 38b7a5c
**Applied fix:** Dropped the unused `#include <cstdlib>`; the cubit test's
dbPath now uses `Directory.systemTemp.path` (matching SessionCubit's own
default pattern) in both the construction and the assertion, with
`import 'dart:io'` added.

## Skipped Issues

### IN-03: Dialog title stays "New space" when the type selector flips to "Standalone room"

**File:** `src/app/lib/shell/space_room_dialog.dart:92-99, 156-165`
**Reason:** Requires a user-level design decision. Making the title reactive
needs a ResponsiveDrawer/scaffold-atom change, but frontend_scaffold lives in
the read-only scaffold submodule (hard boundary); the copy alternative
(type-neutral "New") deviates from the UI-SPEC copy table that specifies
'New space' for createFromHeader. Flag for the designer/owner.
**Original issue:** Dialog title is computed once at open while hint and
confirm label flip with the type selector — "New space" title over a
"Create room" pill.

### IN-06: Manifest convention is lost-update-prone and grows unboundedly

**File:** `src/lib/gcs_entity_store.cpp:55-102`
**Reason:** Forward-looking Phase 3 planning note, explicitly accepted in the
review ("Acceptable under the accepted 'local-restart persistence only this
phase' scope — noting for Phase 3 planning"). Manifest versioning /
per-entity index keys / tombstone GC are multi-writer-era architectural
work, out of scope for a review fix pass.
**Original issue:** Read-union-write on gcs/index/manifest assumes a single
writer; concurrent sessions can lose entries; manifest ids grow forever.

---

# Phase 02: Code Review Fix Report — Iteration 2

**Fixed at:** 2026-09-21T22:25:48Z
**Source review:** .planning/workstreams/app/phases/02-spaces-rooms/02-REVIEW.md
(re-review commit `12b09f7`: 0 Critical / 1 Warning / 4 Info)
**Iteration:** 2 (--all scope)

**Summary:**
- Findings in scope: 5 (WR-05, IN-03, IN-06, IN-08, IN-09)
- Fixed: 4 (one atomic commit each, `fix(02): <ID> <description>`)
- Deferred by instruction: 1 (IN-06 — recorded in deferred-items.md, no code change)

Fixes were applied and committed in an isolated worktree (branch
`gsd-reviewfix/02-68567`, fast-forwarded to `develop`):
`dd718b1`, `548e7e8`, `545b692`, `2864865`.

**Verification evidence (final state, on `develop`):**
- C++: `ninja -C build/OSX/Debug` clean; `ctest --test-dir build/OSX/Debug
  -R "gcs" -E "test_gcs_global_db_sdk"` — 6/6 binaries green. Direct run of
  `--gtest_filter='*OverLength*'` confirms both regression tests executed
  (not option-C skipped).
- Dart: `cd src/app && flutter test` — 47 passed, 1 skipped (unchanged
  baseline; IN-03 extended an existing test rather than adding one).

## Fixed Issues (iteration 2)

### WR-05: Name-length cap counts bytes (C++) vs characters (Dart)

**Files modified:** `src/ffi/gcs_core_ffi.cpp`, `test/test_gcs_ffi_sdk.cpp`
**Commit:** dd718b1
**Applied fix:** Added `Utf8CodePointCount` (counts non-`0b10xxxxxx`
continuation bytes) in the FFI anonymous namespace; all three name-cap arms
(create_space/create_room/update_space) now count Unicode code points, keeping
the 64-unit cap equivalent to the Dart dialog's character-based
`kMaxNameLength`. The constant's comment no longer claims a raw byte mirror.
Regression coverage extended in `OverLengthNamesRejectedAcrossCreateArms`: a
22-CJK-code-point name (66 UTF-8 bytes — sub-cap in code points, over-cap in
bytes) must be accepted and appear in the pushed SpaceTree, while 65 CJK code
points stay rejected; the ASCII 64/65 boundary assertions are unchanged.
Classification note: logic-unit fix — semantics verified by the regression
test above, which fails against the pre-fix byte count.

### IN-03: Dialog title stays "New space" when the type selector flips to "Standalone room"

**Files modified:** `src/app/lib/shell/space_room_dialog.dart`,
`src/app/test/shell/space_room_dialog_test.dart`,
`.planning/.../02-UI-SPEC.md` (copy table, uncommitted — orchestrator commit)
**Commit:** 548e7e8 (code + test)
**Applied fix:** Owner-approved type-neutral copy: the header-launch
`dialogTitle` is now `'New'`, correct for both selector states; the
skip-decision comment at the call site was replaced with the rationale, and
the UI-SPEC copy table row now specifies the type-neutral title (supersedes
the per-type "New space"/"New room" rows). No test previously asserted "New
space"; `CreateStandaloneRoomPublishesEmptyParent` now asserts the title
stays 'New' after flipping the selector.

### IN-08: `send_text.text` and topic strings remain unbounded at the FFI boundary

**Files modified:** `src/ffi/gcs_core_ffi.cpp`, `test/test_gcs_ffi_sdk.cpp`
**Commit:** 545b692
**Applied fix:** Added named constexpr bounds in the entity-name-constant
style: `kMaxTopicLength = 128` bytes (join_topic and send_text `room_topic`;
topics are ASCII by construction — derived topics are `gcs/chat/` + entity id,
~60 bytes max, so the cap always admits minted topics) and
`kMaxMessageTextLength = 4096` bytes (send_text `text`; documented as a
transport-size bound, not a character contract). Over-length values are
rejected with `PostErrorNotice` + `GCS_ERROR_INVALID_ARGUMENT`, mirroring the
WR-01 rejection shape. New regression test
`OverLengthTopicAndTextRejectedInMessagingArms`: over-length topic (join and
send arms) and over-length text are rejected with the pushed ErrorNotices
observed, while exactly-boundary values are accepted (128-byte topic joins,
4096-byte text publishes and echoes).

### IN-09: Library-resolution error copy misleads when `GCS_FFI_LIBRARY` is set to a bad path

**Files modified:** `src/app/lib/cubits/session_cubit.dart`
**Commit:** 2864865
**Applied fix:** `openDefault` reads the env value alongside
`_resolveLibraryPath`; when resolution fails while `GCS_FFI_LIBRARY` is
non-empty (which by construction means the configured file does not exist),
the initial error names the path: `gcs_ffi library not found at <path> (check
GCS_FFI_LIBRARY)`. The set-the-variable copy now appears only when the
variable is genuinely unset. Not unit-tested: `openDefault` short-circuits to
the inert session under the Flutter test harness (`FLUTTER_TEST=true`) before
any resolution runs, and a Dart test process cannot mutate
`Platform.environment` at runtime — driving this path requires a real process
env (integration run), out of scope for a minimal fix.

## Deferred Issues (iteration 2)

### IN-06: Manifest convention is lost-update-prone and grows unboundedly

**File:** `src/lib/gcs_entity_store.cpp:55-102`
**Reason:** Deferred by owner instruction (accepted forward-looking Phase 3
territory: "local-restart persistence only this phase"). Manifest versioning,
per-entity index keys, and tombstone GC are multi-writer-era architectural
work. Now durably recorded in
`.planning/workstreams/app/phases/02-spaces-rooms/deferred-items.md` so it
reaches Phase 3 planning independently of this report.
**Original issue:** Read-union-write on `gcs/index/manifest` assumes a single
writer; concurrent sessions can lose entries; manifest ids grow forever.

---

_Fixed: 2026-09-21T22:25:48Z_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 2_
