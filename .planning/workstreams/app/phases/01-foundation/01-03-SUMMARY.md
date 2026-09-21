---
phase: 01-foundation
plan: 03
subsystem: ffi
tags: [protobuf, ffi, c-abi, pubsub, flutter, gossipsub]

# Dependency graph
requires:
  - phase: 01-foundation (01-01)
    provides: gcs_storage target (GcsGlobalDb in src/lib/gcs_storage) that gcs_core links
  - phase: 01-foundation (01-02)
    provides: gcs::CoreSession (src/lib/gcs_core.hpp) — lifecycle + CRDT ops the FFI thunk drives
  - phase: 01-foundation (01-07)
    provides: src/app/CMakeLists.txt (FRONTEND_BUILD_ENABLED gate) that add_subdirectory(app) consumes
provides:
  - gcs_proto static target + authored src/proto/gcs_chat.proto wire contract (GcsConfig/Codec, GcsCommand/GcsEvent oneof envelopes, ChatMessageState/RoomList/Readiness/ErrorNotice, MessageRole/MessageState enums)
  - gcs_ffi SHARED library (libgcs_ffi.dylib) exposing the four-function topic pub/sub C ABI (gcs_init[config bytes] / gcs_shutdown / gcs_publish[bytes] / gcs_subscribe[port])
  - GcsCommand parse/dispatch (join_topic/send_text) with C++-stamped authoritative ChatMessageState
  - GcsEvent push path (message/room_list/readiness/error) with an inert PostToDart seam for Plan 05's Dart_PostCobject wiring
affects: [01-04-test-gcs-ffi, 01-05-dart-bindings-spike, 01-08-app-dev-loop, 03-messaging, 06-gcs-bot]

# Tech tracking
tech-stack:
  added: [protobuf codegen via existing add_proto_library (no new thirdparty deps)]
  patterns:
    - "codec-tagged bytes ABI: const uint8_t* + size_t payloads, codec bound per store at creation (D-29)"
    - "commands are topic publishes: Dart publishes GcsCommand envelopes to gcs/command; no per-feature call functions (D-27)"
    - "push-not-pull event stream: RoomList/Readiness/messages/raw-error-strings all arrive on the subscribed NativePort (D-05/D-26)"
    - "FFI thunk discipline: lock_guard first statement, payload copied inside the call, handle-equality guard, no exceptions across the ABI"

key-files:
  created:
    - src/proto/gcs_chat.proto
    - src/proto/CMakeLists.txt
    - src/ffi/CMakeLists.txt
    - src/ffi/gcs_core.h
    - src/ffi/gcs_core_ffi.cpp
  modified:
    - src/CMakeLists.txt

key-decisions:
  - "Scoped CMAKE_CURRENT_BINARY_DIR alignment in src/proto/CMakeLists.txt before add_proto_library — keeps the shared helper unmodified while matching protoc's generated/ output layout (see Deviations #1)"
  - "Include spelling proto/gcs_chat.pb.h (generated-layout path, mirrors neoswarm's proto/genius_*.pb.h precedent)"
  - "Smoke-topic joins append to g_roomTopics only when both AddBroadcastTopic + AddListenTopic succeed (outcome results checked, RoomList reflects reality)"
  - "publish/subscribe return GCS_ERROR_NOT_RUNNING when the handle matches but CoreSession::IsRunning() is false"

patterns-established:
  - "Four-function ABI is frozen: init/shutdown/publish/subscribe; gcs_on_message/gcs_join_topic/gcs_string_free are dead symbols that must not reappear"
  - "Generated proto include style: #include \"proto/<name>.pb.h\" via gcs_proto's PUBLIC ${CMAKE_BINARY_DIR}/generated/ include dir"

requirements-completed: [CORE-05]

# Metrics
duration: 15min
completed: 2026-08-26
---

# Phase 01 Plan 03: Protobuf Wire Contract + gcs_ffi Topic Pub/Sub C ABI Summary

**Authored gcs_chat.proto wire contract (GcsCommand/GcsEvent oneof envelopes) and shipped libgcs_ffi.dylib exporting exactly four C symbols (gcs_init/gcs_shutdown/gcs_publish/gcs_subscribe) with codec-tagged protobuf bytes and a Plan-05-ready PostToDart push seam**

## Performance

- **Duration:** 15 min
- **Started:** 2026-08-26T22:52:32Z
- **Completed:** 2026-08-26T23:07:47Z
- **Tasks:** 3/3
- **Files modified:** 6

## Accomplishments
- Protobuf wire contract authored verbatim per plan (append-only, broken-up messages): Codec/GcsConfig, MessageRole/MessageState enums, JoinTopicCommand/SendTextCommand + GcsCommand oneof, ChatMessageState (6 fields), RoomList, Readiness, ErrorNotice, GcsEvent oneof (4 variants)
- gcs_proto static target builds green; protoc generates `build/OSX/Debug/generated/proto/gcs_chat.pb.{h,cc}`
- gcs_ffi SHARED library links gcs_core + gcs_proto (PRIVATE — no C++ types leak into the C ABI); `nm -gU` shows exactly the four exports: `_gcs_init`, `_gcs_publish`, `_gcs_shutdown`, `_gcs_subscribe`
- GcsCommand dispatch implemented: join_topic joins broadcast+listen and pushes RoomList; send_text stamps the authoritative ChatMessageState (id/timestamp/role/state — Dart's payload is data-only), Put()s it, and pushes the echo event; parse failures push an ErrorNotice (raw string) and return GCS_ERROR_INVALID_ARGUMENT
- gcs_subscribe stores the NativePort and immediately pushes RoomList + Readiness(ready=true); gcs_shutdown clears the port before teardown with a handle-equality guard against double-free

## Task Commits

Each task was committed atomically:

1. **Task 1: gcs_chat.proto + gcs_proto target + add_subdirectory(proto) wiring** - `bf4cb06` (feat)
2. **Task 2: gcs_ffi target + gcs_core.h C ABI + add_subdirectory(ffi)/(app) wiring** - `6a3d8a1` (feat)
3. **Task 3: gcs_core_ffi.cpp — GcsCommand parse/dispatch + GcsEvent push** - `f57bf3e` (feat)

**Plan metadata:** see final docs commit (this file).

## Files Created/Modified
- `src/proto/gcs_chat.proto` - the append-only wire contract (D-26/D-29), authored source of truth
- `src/proto/CMakeLists.txt` - gcs_proto via add_proto_library + scoped binary-dir path alignment
- `src/ffi/CMakeLists.txt` - gcs_ffi SHARED target (PRIVATE gcs_core/gcs_proto, GCS_FFI_EXPORTS, Android 16k page option)
- `src/ffi/gcs_core.h` - four-function extern "C" ABI + GcsStatus enum, Doxygen per function
- `src/ffi/gcs_core_ffi.cpp` - FFI thunk: mutex-guarded global session, command dispatch, event push
- `src/CMakeLists.txt` - subdirectory order now proto → gcs_storage → ffi → app(last)

## Decisions Made
- Kept `add_proto_library` as the single proto compilation path (plan artifact contract) and fixed the GCS-specific output-path mismatch locally via a scoped `CMAKE_CURRENT_BINARY_DIR` alignment rather than editing the shared `build/cmake/functions.cmake` helper (details in Deviations #1)
- Include spelling `proto/gcs_chat.pb.h` to match the actual generated layout and the neoswarm precedent
- Smoke-topic join failures do not fail init; the topic simply does not appear in the pushed RoomList
- `GCS_ERROR_NOT_RUNNING` returned when the session handle is valid but the underlying CoreSession is not running (distinct from invalid-argument)

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] add_proto_library output-path mismatch under the gcs_src binary-dir mapping**
- **Found during:** Task 1 (pre-build path analysis, confirmed by an early `cmake --build build/OSX/Debug -j` run at the task that introduced the wiring)
- **Issue:** `compile_proto_to_cpp` (build/cmake/functions.cmake) computes the declared rule-output location from `file(RELATIVE_PATH ${CMAKE_BINARY_DIR}/src ${CMAKE_CURRENT_BINARY_DIR})`. GNUS-NEO-SWARM's src occupies `${CMAKE_BINARY_DIR}/src` (its generated files at `generated/proto/*.pb.h` prove the helper's assumption); this repo's src maps to `gcs_src` (cmake/CommonBuildParameters.cmake). With the default binary dir (`gcs_src/proto`) the declared outputs would land at `gcs_src/proto/*.pb.*` while protoc writes `generated/proto/*.pb.*` — a guaranteed ninja failure. The only directory that yields the required `SCHEMA_REL=proto` (`${CMAKE_BINARY_DIR}/src/proto`) is owned by neoswarm, and CMake hard-errors on binary-dir reuse (verified with a scratch project).
- **Fix:** One scoped `set(CMAKE_CURRENT_BINARY_DIR "${CMAKE_BINARY_DIR}/src/proto")` in `src/proto/CMakeLists.txt` immediately before the plan-verbatim `add_proto_library(gcs_proto ...)` call, with an explanatory comment. This aligns the helper's path math with protoc's actual `--cpp_out` layout without touching the shared `build/` submodule helper; the install path also comes out correct (`include/proto/`).
- **Files modified:** src/proto/CMakeLists.txt
- **Verification:** build step log shows `Generating ../../generated/proto/gcs_chat.pb.h, .pb.cc` and `Building CXX object gcs_src/proto/CMakeFiles/gcs_proto.dir/__/__/generated/proto/gcs_chat.pb.cc.o` — declared outputs and protoc writes now coincide; `libgcs_proto.a` links
- **Committed in:** bf4cb06 (Task 1 commit)

**2. [Rule 3 - Blocking] add_subdirectory(app) relocation instead of append**
- **Found during:** Task 2
- **Issue:** The plan instructed appending `add_subdirectory(app)` as the LAST line assuming it did not yet exist; 01-07 (merged in PR #11) had already wired it mid-file with a two-line comment. Appending a second `add_subdirectory(app)` would hit CMake's binary-dir-reuse error / duplicate targets.
- **Fix:** Moved the existing comment + `add_subdirectory(app)` block verbatim to the end of `src/CMakeLists.txt`, after the appended `add_subdirectory(ffi)`. No existing line content was modified.
- **Files modified:** src/CMakeLists.txt
- **Verification:** `grep -n add_subdirectory src/CMakeLists.txt` → proto(18) → lib/gcs_storage(20) → ffi(32) → app(36, last line); configure + full build green
- **Committed in:** 6a3d8a1 (Task 2 commit)

**3. [Rule 3 - Blocking] Include spelling + include-list trim in gcs_core_ffi.cpp**
- **Found during:** Task 3
- **Issue:** Plan text said `#include "gcs_chat.pb.h"`, but the generated header is reachable via gcs_proto's PUBLIC `${CMAKE_BINARY_DIR}/generated/` include dir as `proto/gcs_chat.pb.h` (neoswarm precedent: `proto/genius_reputation.pb.h`). Also `<chrono>` was missing (epoch-millis timestamp) while `<cstdlib>` is unused (no heap-string functions survive in the four-function ABI).
- **Fix:** `#include "proto/gcs_chat.pb.h"`; added `<chrono>`; kept `<cstring>` (strcmp for the command-topic check); dropped `<cstdlib>`.
- **Files modified:** src/ffi/gcs_core_ffi.cpp
- **Verification:** compiles clean (no warnings), links, exports verified
- **Committed in:** f57bf3e (Task 3 commit)

---

**Total deviations:** 3 auto-fixed (3 blocking/path-wiring; none architectural — the shared `build/` submodule helper and the ABI contract were left untouched)
**Impact on plan:** All fixes were required for the build to be green and for the plan's own verify commands to pass. No scope creep; the proto contract and C ABI are verbatim per plan.

## TDD Note

Task 3 carries `tdd="true"` but defines no `<behavior>` block and lists no test file in `<files>`; the plan's objective explicitly assigns the smoke tests to Plan 04 (`test/test_gcs_ffi.cpp`) and there is no GCS test registration in the tree yet (Plan 04 creates `test/CMakeLists.txt`). The RED phase is therefore deferred to Plan 04 by design; this plan's verification gate (build green + dylib + `nm` four-export check + content greps) passed on the first attempt.

## Known Stubs

- `PostToDart` in `src/ffi/gcs_core_ffi.cpp` is an intentionally inert seam (serializes the event, then `(void)bytes` with a TODO marker). **Resolving plan: 01-05** wires `Dart_PostCobject` (Dart_CObject_kTypedData of the serialized bytes). This is planned, not accidental — pushed events are verified by Plan 04/05 once the seam is filled.

## Issues Encountered
None beyond the deviations above — all three task verify commands and the plan-level verification (`cmake --build build/OSX/Debug -j` green; `nm -gU ... | grep gcs_` = exactly the four exports) passed without a fix-retry cycle.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Plan 04 (test_gcs_ffi.cpp): link against `gcs_ffi` (`target_link_libraries(test_gcs_ffi PRIVATE gcs_ffi)` — the PUBLIC src/ include dir exposes `ffi/gcs_core.h`); exercise init/publish/subscribe/shutdown incl. parse-failure paths
- Plan 05 (Dart spike): fill the PostToDart seam with Dart_PostCobject; ffigen runs against `src/ffi/gcs_core.h` (C ABI is stable)
- Note for later phases: send_text currently Put()s the serialized event under the room topic (Phase 1 echo); the real GossipSub pub/sub flow lands in Phase 3

## Self-Check: PASSED

- All 5 created files exist on disk (`src/proto/gcs_chat.proto`, `src/proto/CMakeLists.txt`, `src/ffi/CMakeLists.txt`, `src/ffi/gcs_core.h`, `src/ffi/gcs_core_ffi.cpp`) — verified
- Commits bf4cb06 / 6a3d8a1 / f57bf3e present on `feature/app-ffi-data-plane` — verified
- Plan-level verification re-run: build green; `nm -gU build/OSX/Debug/gcs_src/ffi/libgcs_ffi.dylib | grep -cE "_gcs_(init|shutdown|publish|subscribe)"` = 4 — verified

---
*Phase: 01-foundation*
*Completed: 2026-08-26*
