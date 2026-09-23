---
phase: 02-spaces-rooms
reviewed: 2026-09-21T22:13:44Z
depth: standard
files_reviewed: 18
files_reviewed_list:
  - src/CMakeLists.txt
  - src/app/lib/cubits/rail_cubit.dart
  - src/app/lib/cubits/session_cubit.dart
  - src/app/lib/generated/proto/gcs_chat.pb.dart
  - src/app/lib/generated/proto/gcs_chat.pbenum.dart
  - src/app/lib/generated/proto/gcs_chat.pbjson.dart
  - src/app/lib/shell/room_rail.dart
  - src/app/lib/shell/space_room_dialog.dart
  - src/app/test/chat_shell_test.dart
  - src/app/test/cubits/shell_cubits_test.dart
  - src/app/test/shell/space_room_dialog_test.dart
  - src/ffi/gcs_core_ffi.cpp
  - src/lib/gcs_entity_store.cpp
  - src/lib/gcs_entity_store.hpp
  - src/proto/gcs_chat.proto
  - test/CMakeLists.txt
  - test/test_gcs_entities.cpp
  - test/test_gcs_ffi_sdk.cpp
findings:
  critical: 0
  warning: 1
  info: 4
  total: 5
status: issues_found
---

# Phase 02: Code Review Report (Re-review, iteration 2)

**Reviewed:** 2026-09-21T22:13:44Z
**Depth:** standard
**Files Reviewed:** 18 (plus `src/ffi/gcs_core.h`, read to verify the IN-05 doc fix)
**Status:** issues_found

## Summary

Re-review of Phase 2 (spaces/rooms catalog) after the fix pass (commits df16d9e..38b7a5c,
all nine confirmed present on `develop`). Scope: proto contract, C++ entity store + FFI
thunk, Dart cubits/shell widgets, C++/Dart test suites, generated Dart proto output.

**All 9 claimed fixes verified present and correct in the current code** (evidence below).
The fix code itself was reviewed for regressions: WR-02's manifest-first order degrades in
the documented direction; WR-03's `g_explicitTopics` is correctly mutex-guarded, cleared on
teardown, and its re-derivation path re-registers idempotently (same pattern `join_topic`
already uses); WR-04's one-shot flag is set only on the publish-and-pop path so refused
publishes stay retryable. The generated Dart proto files still match `gcs_chat.proto`
exactly (field numbers, types, oneof tags); tests use wait-condition templates with zero
`sleep_for`; no debug artifacts; no OS `#ifdef` in source (the header's `_WIN32` export
macro is the documented IN-02 exemption).

One new Warning surfaced by reviewing the fix code: the WR-01 mirror cap counts different
units on each side of the trust boundary — C++ counts UTF-8 **bytes**, the Dart dialog's
`maxLength` counts **characters** — so legitimate non-ASCII names accepted by the client
are rejected by the FFI with a misleading failure surface. Two prior Info items remain
(IN-03 skipped pending an owner design decision, IN-06 accepted as forward-looking), and
two new minor Info items were found in the fresh pass.

## Fix Verification (re-review — read from code, not the fix report)

| Prior ID | Claim | Verified at | Verdict |
|----------|-------|-------------|---------|
| WR-01 | C++ name-length rejection | `src/ffi/gcs_core_ffi.cpp:53-56` (constant), `:680-684` (create_space), `:709-713` (create_room), `:747-751` (update_space); regression test `test/test_gcs_ffi_sdk.cpp:594-655` covers all three arms + the 64-char boundary | Fixed (but see WR-05 — unit mismatch) |
| WR-02 | Manifest-first write order | `src/lib/gcs_entity_store.cpp:204-218` (CreateSpace), `:247-260` (CreateRoom); hpp doc updated `gcs_entity_store.hpp:38-41`; failure direction is the already-tested LoadFromStore skip | Fixed |
| WR-03 | `g_explicitTopics` protects explicit joins | `src/ffi/gcs_core_ffi.cpp:74-75` (decl), `:620-624` (join_topic records), `:379-384` (erase loop skips explicit), `:155` (teardown clears); corrected comment `:330-338`; regression test `test_gcs_ffi_sdk.cpp:662-742` | Fixed |
| WR-04 | `_submitHandled` one-shot guard | `src/app/lib/shell/space_room_dialog.dart:170` (decl), `:280-282` (early return), `:299` (set after accepted publish, before pop — refused publishes stay retryable); test `space_room_dialog_test.dart:259-277` | Fixed |
| IN-01 | Orphan-fallback documented | `src/app/lib/cubits/rail_cubit.dart:142-146` (setTree doc comment) | Fixed |
| IN-02 | Collapse-key pruning | `src/app/lib/shell/room_rail.dart:148-156` (`removeWhere` in `_buildTree`; mutation-during-build is safe here — a pruned key's space is not rendering this frame) | Fixed |
| IN-04 | Mode-param asserts | `src/app/lib/shell/space_room_dialog.dart:89-96`; test `space_room_dialog_test.dart:279-305` | Fixed |
| IN-05 | `gcs_init` ignores-config documented | `src/ffi/gcs_core.h:77-84` (doc-only option; file outside the listed scope, read for this verification) | Fixed |
| IN-07 | Test nits | `#include <cstdlib>` gone from `test/test_gcs_entities.cpp:21-26`; `Directory.systemTemp.path` in `shell_cubits_test.dart:291-296` with `dart:io` import at line 16 | Fixed |

## Narrative Findings (AI reviewer)

### Warnings

#### WR-05: Name-length cap counts bytes (C++) vs characters (Dart) — non-ASCII names dead-end

**Status (fix pass 2):** FIXED — commit `dd718b1`. The cap now counts UTF-8 code points
(`Utf8CodePointCount`) in all three arms; the constant's comment documents the
code-point semantic. Regression coverage extended in
`OverLengthNamesRejectedAcrossCreateArms`: a 22-CJK-code-point name (66 bytes) is
accepted, 65 CJK code points stay rejected, ASCII 64 boundary unchanged.

**File:** `src/ffi/gcs_core_ffi.cpp:53-56, 680-684, 709-713, 747-751`; `src/app/lib/shell/space_room_dialog.dart:59, 382`
**Issue:** The WR-01 fix mirrors the Dart cap as `kMaxEntityNameLength = 64` counted via
`name().size()` — UTF-8 **bytes**. The Dart dialog enforces `maxLength: kMaxNameLength`
(64), which Flutter counts in **characters** (UTF-16 code units / graphemes — never
bytes). For any non-ASCII name the C++ cap is therefore effectively smaller: 30 CJK
characters are 90 UTF-8 bytes — accepted by the dialog, then rejected by the FFI with
`GCS_ERROR_INVALID_ARGUMENT`. The user sees the refused-publish toast "The command didn't
reach the chat core. Try again." (misleading — it did reach and was rejected) while the
real reason lands on the raw `SessionState.error` surface; retrying can never succeed.
The constant's own comment ("in bytes — mirror of the Dart dialog's kMaxNameLength")
misstates the mirroring. The regression test exercises only ASCII (`'x'`/`'y'`), so the
divergence is untested. Introduced by the WR-01 fix; the pre-fix code accepted any length.
**Fix:** Count code points, not bytes, in the FFI arms (minimal UTF-8-aware count — a code
point begins at every byte that is not a `0b10xxxxxx` continuation byte):

```cpp
// src/ffi/gcs_core_ffi.cpp (anonymous namespace, next to kMaxEntityNameLength)
constexpr size_t kMaxEntityNameCodePoints = 64; // T-02-09 mirror of Dart kMaxNameLength

size_t Utf8CodePointCount( const std::string &s )
{
    return static_cast<size_t>( std::count_if( s.begin(), s.end(),
        []( unsigned char c ) { return ( c & 0xC0 ) != 0x80; } ) );
}

// in each arm, replacing the .size() check:
if ( Utf8CodePointCount( createSpace.name() ) > kMaxEntityNameCodePoints )
{
    PostErrorNotice( "create_space rejected: name exceeds maximum length" );
    return GCS_ERROR_INVALID_ARGUMENT;
}
```

Alternatively keep the byte cap and enforce bytes on the Dart side — but the code-point
count preserves the visible "64 characters" contract on both sides. Extend
`OverLengthNamesRejectedAcrossCreateArms` with a multibyte boundary case (e.g. 22 CJK
chars = 66 bytes, 22 code points → accepted under code points, rejected under bytes).

### Info

#### IN-03: Dialog title stays "New space" when the type selector flips to "Standalone room" (carried, still present)

**Status (fix pass 2):** FIXED (owner-approved type-neutral copy) — commit `548e7e8`.
The header-launch title is now "New", correct for both type-selector states; the
UI-SPEC copy table row was updated to match, and the widget test pins the title after
flipping to Standalone room.

**File:** `src/app/lib/shell/space_room_dialog.dart:105-108, 173-182`
**Issue:** Unchanged behavior from the prior review: `title: form.dialogTitle` is computed
once at open (`createFromHeader` -> "New space") while the hint and confirm label flip
with the type selector, yielding a "New space" title over a "Create room" pill. Current
status: SKIPPED by owner decision (a reactive title needs a `ResponsiveDrawer` change in
the read-only scaffold submodule; type-neutral copy deviates from the UI-SPEC table). The
skip is now explicitly documented in the code comment at lines 105-108, but the decision
is tracked only in 02-REVIEW-FIX.md — it is absent from
`.planning/workstreams/app/phases/02-spaces-rooms/deferred-items.md`, so nothing durable
reminds the owner it is pending.
**Fix:** Add an entry to `deferred-items.md` referencing the design decision (static title
vs type-flipping selector vs type-neutral copy) so it survives this phase's artifacts.

#### IN-06: Manifest convention is lost-update-prone and grows unboundedly (carried, still present — accepted)

**Status (fix pass 2):** DEFERRED by owner (Phase 3 territory, no code change this
phase) — now durably tracked in `deferred-items.md` (this phase's artifacts) instead of
only REVIEW-FIX.md.

**File:** `src/lib/gcs_entity_store.cpp:55-102`
**Issue:** Unchanged from the prior review: read-union-write on `gcs/index/manifest`
assumes a single writer; concurrent sessions can lose entries, and manifest ids accumulate
forever once tombstone deletes exist. Current status: SKIPPED as accepted forward-looking
Phase 3 territory ("local-restart persistence only this phase" scope). No action required
this phase; carrying so it reaches Phase 3 planning.
**Fix:** When multi-writer support lands, version the manifest or move to per-entity index
keys; prune ids whose records are tombstoned past a GC horizon.

#### IN-08: `send_text.text` and topic strings remain unbounded at the FFI boundary (new)

**Status (fix pass 2):** FIXED — commit `545b692`. Named bounds `kMaxTopicLength`
(128 bytes, join_topic + send_text room_topic) and `kMaxMessageTextLength` (4096
bytes, send_text text) reject with `PostErrorNotice` + `GCS_ERROR_INVALID_ARGUMENT`;
regression test `OverLengthTopicAndTextRejectedInMessagingArms` covers rejections,
pushed ErrorNotices, and boundary acceptance.

**File:** `src/ffi/gcs_core_ffi.cpp:589-670`
**Issue:** WR-01 closed the trust-boundary gap only for entity display names. The
`join_topic`/`send_text` arms perform no length validation on `room_topic` or `text` —
the only bound is the INT_MAX payload-narrowing guard, so any FFI client can publish a
multi-megabyte text that is copied, serialized into a `ChatMessageState`, persisted under
the CRDT key, and echoed to the port. No Dart comment promises a cap here (unlike WR-01),
so this is a hardening gap rather than a broken contract.
**Fix:** Add named max-length constants for `room_topic` and `text` in the FFI arms,
rejected with `GCS_ERROR_INVALID_ARGUMENT` + `PostErrorNotice`, mirroring the WR-01 shape
(and count bytes for topics, which are ASCII by construction).

#### IN-09: Library-resolution error copy misleads when `GCS_FFI_LIBRARY` is set to a bad path (new)

**Status (fix pass 2):** FIXED — commit `2864865`. `openDefault` now reports
`'gcs_ffi library not found at <path> (check GCS_FFI_LIBRARY)'` when the env var is
set but its file is absent, keeping the set-the-variable copy only for the unset case.

**File:** `src/app/lib/cubits/session_cubit.dart:157-163, 202-219`
**Issue:** `_resolveLibraryPath` treats "env var set but file absent" the same as "env var
unset" — it falls through to the packaged probes and, when those fail, `openDefault`
reports `'gcs_ffi library not found (set GCS_FFI_LIBRARY)'`. A developer who HAS set the
variable (to a stale/wrong path) is told to do the thing they already did, hiding the
actual diagnosis (the configured path does not exist).
**Fix:** Distinguish the cases in `openDefault`: when `fromEnv` was non-empty but
`File(fromEnv).existsSync()` was false, surface
`'gcs_ffi library not found at $fromEnv (check GCS_FFI_LIBRARY)'` instead of the
set-the-variable copy.

---

_Reviewed: 2026-09-21T22:13:44Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
_Re-review iteration: 2 (prior: 2026-09-19T19:57:29Z, 0 Critical / 4 Warning / 7 Info; 9 fixed, 2 skipped)_
