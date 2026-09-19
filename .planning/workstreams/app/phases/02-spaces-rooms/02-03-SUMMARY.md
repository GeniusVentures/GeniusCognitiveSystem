---
phase: 02-spaces-rooms
plan: 03
subsystem: dart-contract-bridge
tags: [protobuf, dart, protoc-plugin-pin, rail-tree, cubits, space-tree, flutter]

# Dependency graph
requires:
  - phase: 02-spaces-rooms plan 01
    provides: extended gcs_chat.proto (SpaceRecord/RoomRecord/EntityManifest/commands/SpaceTree, arms 3-5/5)
  - phase: 02-spaces-rooms plan 02
    provides: frozen FFI push contract (SpaceTree -> RoomList -> Readiness ordering)
provides:
  - Regenerated Dart pb bindings (pinned protoc_plugin 22.5.0) exposing all Phase 2 messages + oneof accessors
  - RailState tree (spaces/standaloneRooms/treeReceived) + RailCubit.setTree with parentSpaceId grouping
  - SessionCubit hasSpaceTree dispatch arm (first, above hasRoomList) proven by 3 new cubit tests
affects: [02-04 (dialog publishes through regenerated pb commands), 02-05 (rail renders tree fields)]

# Tech tracking
tech-stack:
  added: [] # zero new packages (02-RESEARCH audit holds; plugin PIN, not an addition)
  patterns:
    - "Full-replacement setTree mirrors setRooms; tree lifecycle and joined lifecycle never overwrite each other (D-02)"
    - "treeReceived loading-vs-empty flag set only by a pushed SpaceTree"
    - "Backtick-wrap <id> tokens in proto comments: raw angle brackets flow into generated Dart doc comments and trip unintended_html_in_doc_comment under --fatal-infos"

key-files:
  created:
    - .planning/workstreams/app/phases/02-spaces-rooms/02-03-SUMMARY.md
  modified:
    - src/proto/gcs_chat.proto
    - src/app/lib/generated/proto/gcs_chat.pb.dart
    - src/app/lib/generated/proto/gcs_chat.pbenum.dart
    - src/app/lib/generated/proto/gcs_chat.pbjson.dart
    - src/app/lib/cubits/rail_cubit.dart
    - src/app/lib/cubits/session_cubit.dart
    - src/app/test/cubits/shell_cubits_test.dart

key-decisions:
  - "Proto comments backtick-wrap `<id>` key tokens — the fix lives at the proto source, never in generated files (comment-only edit; field numbers/types untouched)"
  - "setTree routes a room whose parentSpaceId matches no pushed space to standaloneRooms — C++ FFI validation makes orphans unreachable, but catalog data is never silently dropped"
  - "Dispatch arm order encodes D-02: hasSpaceTree checked first so the catalog renders before membership"

requirements-completed: [CORE-01, CORE-02, CORE-03]

# Metrics
duration: 3min
completed: 2026-09-19
---

# Phase 2 Plan 3: Dart pb Regen + Rail Tree Summary

**Pinned 22.5.0 pb regeneration exposing all 7 Phase 2 messages, RailState evolved flat → tree (RailSpace/RailRoom + parentSpaceId grouping + treeReceived), SessionCubit SpaceTree dispatch arm — 19/19 cubit tests green, analyze clean under --fatal-infos**

## Performance

- **Duration:** 3 min
- **Started:** 2026-09-19T19:21:37Z
- **Completed:** 2026-09-19T19:24:55Z
- **Tasks:** 3
- **Files modified:** 7

## Accomplishments
- protoc_plugin re-pinned at 22.5.0 (header-verified) and the exact 01-05 regen command run verbatim: `gcs_chat.pb.dart`/`.pbenum.dart`/`.pbjson.dart` regenerated with SpaceRecord, RoomRecord, EntityManifest, CreateSpaceCommand, CreateRoomCommand, UpdateSpaceCommand, SpaceTree, `hasSpaceTree` (1 occurrence each), zero `BuilderInfo.aE`
- `RailState` gained `spaces`/`standaloneRooms` (unmodifiable, full replacement) + `treeReceived` (loading-vs-empty flag, default false); `copyWith` extended with nullable-list replacement and a treeReceived override — `rooms`/`activeRoom`/`setRooms`/`selectRoom` byte-identical (joined view untouched, D-02)
- `RailCubit.setTree(List<SpaceRecord>, List<RoomRecord>)` maps records to immutable `RailSpace`/`RailRoom` (derived `topic` getter = `gcs/chat/<id>`), groups rooms by `parentSpaceId` preserving pushed order, emits `treeReceived: true` as a full replacement
- `SessionCubit._dispatchEvent` routes `hasSpaceTree → RailCubit.setTree` FIRST (above `hasRoomList`), matching the frozen C++ push ordering SpaceTree → RoomList → Readiness; library doc table updated
- 3 new shell_cubits_test cases: grouped-room tree population (space + grouped room + derived topic + treeReceived), empty-parent standalone grouping (D-01), RoomList-after-SpaceTree independence (joined view replaced, tree fields intact) — 19/19 pass, `flutter analyze lib test` clean

## Task Commits

Each task was committed atomically:

1. **Task 1: Regenerate Dart pb with pinned protoc_plugin 22.5.0** - `b56926a` (feat)
2. **Task 2: RailState flat → tree (RailSpace/RailRoom + setTree)** - `ff499dd` (feat)
3. **Task 3: SpaceTree dispatch arm + cubit tests** - `5081073` (test)

**Plan metadata:** (see final commit below)

## Files Created/Modified
- `src/app/lib/generated/proto/gcs_chat.pb.dart` - regenerated: all 7 new messages + oneof accessors (committed by design)
- `src/app/lib/generated/proto/gcs_chat.pbenum.dart`, `.pbjson.dart` - regenerated enum/registry halves
- `src/proto/gcs_chat.proto` - comment-only edit (backticked `<id>` tokens; deviation fix, see below)
- `src/app/lib/cubits/rail_cubit.dart` - RailRoom/RailSpace models, RailState tree fields, setTree grouping
- `src/app/lib/cubits/session_cubit.dart` - hasSpaceTree dispatch arm (first) + doc table line
- `src/app/test/cubits/shell_cubits_test.dart` - 3 SpaceTree tests

## Decisions Made
- Proto comment `<id>` tokens are now backtick-wrapped at the SOURCE (`gcs_chat.proto`), because raw angle brackets flow into generated Dart doc comments and trip `unintended_html_in_doc_comment` — fatal under the CI gate's `dart analyze --fatal-infos lib test`. Generated files stay unedited ("do not edit" header honored); the comment-only change touches no field numbers, names, or types, so the append-only contract and frozen C++ side are untouched
- `setTree` sends a room with a non-empty but unmatched `parentSpaceId` to `standaloneRooms` rather than dropping it — unreachable via the validated C++ push path, but the defensive branch guarantees no catalog record can silently vanish from the rail
- The dispatch arm is written multi-line per dart format (80-col); semantically identical to the plan's single-line form and verified above `hasRoomList`

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Regenerated pb tripped fatal doc-comment lints**
- **Found during:** Task 1 (acceptance: `flutter analyze lib/generated/proto` must exit 0)
- **Issue:** 02-01's proto comments contain raw `<id>` tokens (`gcs/entities/spaces/<id>`, `gcs/entities/rooms/<id>`, `gcs/chat/<id>`); the 22.5.0 generator copies them into Dart `///` docs where they raise 3 `unintended_html_in_doc_comment` infos — and `flutter analyze` exits 1 on infos (the `app_analyze` CI gate passes `--fatal-infos`)
- **Fix:** Backtick-wrapped the three key tokens in the proto comments (comment-only), regenerated — analyze clean, all class/accessor greps still pass
- **Files modified:** src/proto/gcs_chat.proto (+ regenerated pb files)
- **Verification:** `flutter analyze lib/generated/proto` exit 0; 7 classes + hasSpaceTree grep counts unchanged
- **Committed in:** b56926a (Task 1 commit)

---

**Total deviations:** 1 auto-fixed (blocking)
**Impact on plan:** Comment-only proto change; no contract, arm, or field-number movement. Required for the plan's own analyze acceptance criterion.

## Issues Encountered
None.

## User Setup Required
None - no external service configuration required.

## Known Stubs
None — every dispatch arm routes to real setTree state mapping; no placeholder data paths exist.

## Threat Flags
None — no new surface beyond the plan's threat model. T-02-08 stands as registered: `handlePushedBytes` decodes inside the existing try/catch (malformed bytes → raw error string, test still green), and the SpaceTree arm is reached only after a successful decode.

## Next Phase Readiness
- Plans 02-04/02-05 can import the regenerated pb commands (`GcsCommand()..createSpace = ...` etc.) and render from `RailState.spaces`/`standaloneRooms` gated on `treeReceived`
- Orphan-room fallback and grouping order are test-pinned; `selectRoom` joined-only guard (Pitfall 7) remains for 02-05's disabled-row resolution

## Self-Check: PASSED

All three task commits verified in git log (b56926a, ff499dd, 5081073); all modified files verified on disk; this SUMMARY present in the plan directory.

---
*Phase: 02-spaces-rooms*
*Completed: 2026-09-19*
