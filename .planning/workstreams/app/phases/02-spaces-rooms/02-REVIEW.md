---
phase: 02-spaces-rooms
reviewed: 2026-09-19T19:57:29Z
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
  warning: 4
  info: 7
  total: 11
status: issues_found
---

# Phase 02: Code Review Report

**Reviewed:** 2026-09-19T19:57:29Z
**Depth:** standard
**Files Reviewed:** 18
**Status:** issues_found

## Summary

Phase 2 (spaces/rooms catalog) reviewed across the full stack: proto contract, C++
entity store + FFI thunk, Dart cubits/shell widgets, and the C++/Dart test suites.

Overall the implementation is solid: the append-only proto discipline is respected
(all Phase 2 additions are new fields/messages; nothing retyped, removed, or
reordered); C++ stamps every authority field (ids salted per process, timestamps,
tombstones); the FFI ABI validates pointers, sizes (INT_MAX narrowing), session
identity, and topic at every entry; all mutations happen under `g_mutex`; error
paths push `ErrorNotice` per D-29; the generated Dart proto files match the
`.proto` (pinned-plugin output consistent, no pipeline anomalies); tests use
wait-condition templates with zero `sleep_for`; no `printf`/`cout`/`print`
artifacts; no OS `#ifdef`; Allman bracing, `m_` prefix, Doxygen headers, and
`outcome::result` throughout the C++.

No Critical findings. Four Warnings: a documented C++ name-length re-validation
that does not exist, a partial-write orphan in the two-key create path, a
derived/explicit join overlap that silently revokes explicitly-joined rooms, and a
re-entrancy gap in the dialog confirm handler. Seven Info items cover robustness
and polish.

## Narrative Findings (AI reviewer)

### Warnings

#### WR-01: Documented C++ name-length re-validation does not exist (unbounded names persist)

**File:** `src/app/lib/shell/space_room_dialog.dart:57-58`, `src/ffi/gcs_core_ffi.cpp:475-478, 499-502, 532-536`
**Issue:** The Dart constant documents "T-02-09: client-side cap; C++ re-validates
as defense in depth." The C++ side performs no length validation whatsoever —
`kCreateSpace`, `kCreateRoom`, and `kUpdateSpace` check only `name().empty()`.
Any FFI client (not just this Flutter app) can publish a multi-megabyte name that
is serialized into the CRDT record and the manifest-adjacent store, and echoed
into `PostErrorNotice` strings, with no bound. The 64-char cap exists only in the
Dart dialog.
**Fix:** Add a named constant and reject oversized names in the FFI handlers
(keeps the Dart comment truthful and closes the trust-boundary gap):

```cpp
// src/ffi/gcs_core_ffi.cpp (anonymous namespace)
constexpr size_t kMaxEntityNameLength = 64; // T-02-09 (mirror of Dart kMaxNameLength)

// in kCreateSpace / kCreateRoom / kUpdateSpace arms, next to the empty-name check:
if ( createSpace.name().size() > kMaxEntityNameLength )
{
    PostErrorNotice( "create_space rejected: name exceeds maximum length" );
    return GCS_ERROR_INVALID_ARGUMENT;
}
```

#### WR-02: Two-key create is partial-write orphan-prone (record persisted, manifest not)

**File:** `src/lib/gcs_entity_store.cpp:204-215, 243-254`
**Issue:** `CreateSpace`/`CreateRoom` write the record key first, then
read-union-write the manifest. If the manifest `Put` fails, the function returns
the error and does NOT emplace into `m_spaces`/`m_rooms` — but the record bytes
are already persisted under `gcs/entities/{spaces,rooms}/<id>`. Because
GlobalDB has no enumeration and the manifest is the only index, those bytes are
unreachable forever (invisible on every reload, no cleanup path). The current
process also reports failure while bytes actually landed, so store and returned
result disagree.
**Fix:** Reverse the write order — manifest first, then the record.
`LoadFromStore` already tolerates a manifest id whose record is missing
("skipping space '{}' — record read failed"), so the failure direction becomes
the already-implemented, already-tested graceful skip instead of an unhealable
orphan:

```cpp
// CreateSpace (same shape for CreateRoom):
auto manifestResult = AppendToManifest( m_session, true, id );
if ( !manifestResult.has_value() )
{
    return manifestResult.error();   // nothing persisted yet
}
auto putResult = m_session.Put( std::string( kSpacesKeyPrefix ) + id,
                                record.SerializeAsString() );
if ( !putResult.has_value() )
{
    return putResult.error();        // manifest entry skipped by readers (D-02)
}
m_spaces.emplace( id, record );
```

#### WR-03: RefreshDerivedJoins silently revokes an explicitly-joined topic that is also derived

**File:** `src/ffi/gcs_core_ffi.cpp:162-209` (comment at 158-161, erase loop 195-208)
**Issue:** The function comment claims "Smoke and explicit join_topic topics are
never in g_derivedTopics and are never touched here." The first half is true;
the second half is not when a topic is BOTH derived and explicitly joined: a
derived topic `gcs/chat/<room-id>` that a client also joined via `join_topic`
lives in `g_roomTopics` once, and when `autoJoinRooms` is later toggled off, the
erase loop removes it from `g_roomTopics` — silently revoking the explicit join
from the pushed RoomList and (via `RailCubit.setRooms`) clearing the user's
active-room selection. No Phase 2 UI triggers this, but the ABI allows it and the
comment documents the opposite behavior.
**Fix:** Track provenance so derived eviction never removes explicit membership —
e.g. keep a separate `g_explicitTopics` that `kJoinTopic` appends to, and in the
erase loop skip topics present in that set:

```cpp
// erase loop in RefreshDerivedJoins:
else if ( std::find( g_explicitTopics.begin(), g_explicitTopics.end(), topic )
          == g_explicitTopics.end() )
{
    g_roomTopics.erase( std::remove( g_roomTopics.begin(), g_roomTopics.end(), topic ),
                        g_roomTopics.end() );
}
```
(At minimum, correct the comment so the accepted-sticky-ports item is not read as
covering explicit-join revocation.)

#### WR-04: Dialog confirm is re-entrant — Enter-key repeat can publish duplicate commands

**File:** `src/app/lib/shell/space_room_dialog.dart:260-279`
**Issue:** `submit()` has no re-entrancy guard, and it is reachable from two
synchronous sources: the confirm pill `onPressed` and
`onFieldSubmitted: (_) => form.submit(context)` on the text field. A held Enter
key (key-repeat) or a fast Enter-then-tap can invoke `submit` more than once
before the route finishes popping — each invocation publishes another
`CreateSpace`/`CreateRoom`/`UpdateSpace`. Duplicate `create_space` in particular
persists two same-named spaces (no idempotency key exists), violating the
"publish -> close immediately" contract's intent of exactly one command per
confirm.
**Fix:** Guard with a one-shot flag set before publishing:

```dart
bool _submitHandled = false;
void submit(BuildContext context) {
  if (_submitHandled) { return; }
  final String name = nameController.text.trim();
  if (name.isEmpty) { ... }
  _submitHandled = true;           // set only on the path that publishes/pops
  ...
}
```

### Info

#### IN-01: Rooms with an unresolvable parent render as standalone (undocumented fallback)

**File:** `src/app/lib/cubits/rail_cubit.dart:159-166`
**Issue:** A `RoomRecord` whose `parentSpaceId` is non-empty but matches no pushed
space (e.g. tombstoned parent — `Spaces()` filters it out while `Rooms()` keeps
the live room, per `test_gcs_entities.cpp` `TombstonedRecordsAreSkipped`) is
grouped into `standaloneRooms`. Reasonable graceful degradation, but it promotes
an orphaned room to top level with no signal, and D-01 only specifies the
empty-parent case.
**Fix:** Document the fallback in the `setTree` doc comment, or render orphans
under a distinct "ungrouped" treatment if the UI spec wants one.

#### IN-02: `_collapsedSpaceIds` never pruned across setTree replacements

**File:** `src/app/lib/shell/room_rail.dart:71-81`
**Issue:** The view-local expansion map is keyed by space id and never has stale
ids removed. Since ids are unique per creation and spaces are never deleted in
Phase 2, every space ever seen in the session lifetime keeps an entry forever.
Not a correctness bug (absent key = expanded), just unbounded stale-state growth
in a long-lived session.
**Fix:** On each `setTree`-driven rebuild, drop keys not present in the pushed
space ids (e.g. in `didUpdateWidget`/build when `rail.spaces` identity changes).

#### IN-03: Dialog title stays "New space" when the type selector flips to "Standalone room"

**File:** `src/app/lib/shell/space_room_dialog.dart:92-99, 156-165`
**Issue:** `title: form.dialogTitle` is computed once at open
(`createFromHeader` -> "New space"), while the hint and confirm label do flip with
the type selector. Selecting "Standalone room" yields a dialog titled "New space"
with a "Create room" pill — the comment acknowledges the mechanism, but the
mismatch is a visible UX inconsistency (project UX directive: visual/UX quality is
part of correctness).
**Fix:** If `ResponsiveDrawer.show` can accept a reactive title (or be re-shown),
flip the title with `_isSpaceType`; otherwise reconcile the copy so the static
title is type-neutral (e.g. "New").

#### IN-04: Mode-required dialog params enforced only by doc — `parent!`/`space!` crash on misuse

**File:** `src/app/lib/shell/space_room_dialog.dart:161, 308, 313`
**Issue:** `showSpaceRoomDialog` is a frozen public API whose `createRoomInSpace`
mode requires `parent` and `editSpace` requires `space`, but nothing validates
this: `dialogTitle` (`parent!.name`) evaluates immediately at open and
`_buildCommand` (`parent!.id`, `space!.id`) on submit — a caller omitting the
param gets a null-check crash rather than a clear assertion error.
**Fix:** Add `assert(mode != SpaceRoomDialogMode.createRoomInSpace || parent != null)`
and the `editSpace`/`space` equivalent in `showSpaceRoomDialog` so misuse fails
loudly at the call site in debug builds.

#### IN-05: `gcs_init` idempotent path silently ignores a differing config

**File:** `src/ffi/gcs_core_ffi.cpp:276-279`
**Issue:** A second `gcs_init` while a session lives returns the existing handle
without comparing `configBytes` — a caller passing a different `db_path` (or
codec) silently gets the first session's store. Correct for the current
single-session Dart lifecycle, but surprising at the ABI level.
**Fix:** Either document the ignores-config behavior on the `gcs_init` contract in
`gcs_core.h`, or reject mismatched config with `GCS_ERROR_INVALID_ARGUMENT`.

#### IN-06: Manifest convention is lost-update-prone and grows unboundedly (forward-looking)

**File:** `src/lib/gcs_entity_store.cpp:55-102`
**Issue:** Read-union-write on `gcs/index/manifest` assumes a single writer.
Two concurrent sessions (or a future multi-writer phase) can lose manifest
entries, and once tombstone deletes exist the manifest retains ids forever
(N+1 `Get`s per load grow without bound). Acceptable under the accepted
"local-restart persistence only this phase" scope — noting for Phase 3 planning.
**Fix:** When multi-writer support lands, version the manifest (or move to a
per-entity index key) so writers union instead of overwrite; consider pruning
manifest ids whose records are tombstoned past a GC horizon.

#### IN-07: Test nits — unused include and POSIX-only path

**File:** `test/test_gcs_entities.cpp:23`, `src/app/test/cubits/shell_cubits_test.dart:291,296`
**Issue:** (a) `#include <cstdlib>` is unused in `test_gcs_entities.cpp` (no
`atoi`/`exit`/`malloc`/`rand` symbols). (b) `dbPath: '/tmp/gcs-cubit-test-db'`
hardcodes a POSIX path; the assertion on it fails on Windows runners if the Dart
suite ever executes there.
**Fix:** Drop the include; use `Directory.systemTemp.path` (already the pattern in
`session_cubit.dart`) for the test db path.

---

_Reviewed: 2026-09-19T19:57:29Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_

---

**Fixes applied 2026-09-19:** see [02-REVIEW-FIX.md](02-REVIEW-FIX.md) — 9 fixed / 2 skipped (IN-03 needs owner decision, IN-06 forward-looking). Commits df16d9e..38b7a5c. Post-fix gates: gcs ctest 5/5, Flutter 47 passed / 1 skip.
