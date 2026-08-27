---
phase: 01-foundation
plan: "05"
subsystem: ffi
tags: [dart, ffi, native-port, api-dl, protobuf, ffigen, ctest, flutter]

# Dependency graph
requires:
  - phase: 01-foundation plan 03
    provides: four-function gcs_ffi ABI + PostToDart seam + gcs_chat.proto wire types
  - phase: 01-foundation plan 04
    provides: option-C FFI ABI gtest proof (test_gcs_ffi) + GCS-root ctest harness
  - phase: 01-foundation plan 07
    provides: FRONTEND_BUILD_ENABLED gate + ffigen/protobuf/protoc_plugin pubspec deps
provides:
  - Vendored Dart API_DL set in src/ffi (dart_native_api.h / dart_api_dl.h / dart_api_dl.c) linked into gcs_ffi
  - PostToDart wired to Dart_PostCObject typed-data (uint8) protobuf-byte posting
  - ffigen bindings (src/app/lib/gcs_bindings_generated.dart) + Dart protobuf bindings from the shared gcs_chat.proto (D-24 single source of truth)
  - Dart-side NativePort smoke test registered in the SAME ctest harness as the gtests (test_gcs_ffi_dart, ctest #21)
  - CTest-driven Dart FFI harness pattern: CMake owns the artifact path ($<TARGET_FILE:gcs_ffi> via cmake -E env GCS_FFI_LIBRARY), install(TARGETS) places lib+header in <prefix>
affects: [01-06 CI, 01-11 app shell/cubits, 03 messaging]

# Tech tracking
tech-stack:
  added: []  # Dart API_DL is vendored source, not a thirdparty library
  patterns:
    - "CTest-driven Dart FFI testing: `cmake -E env \"GCS_FFI_LIBRARY=$<TARGET_FILE:lib>\"` in add_test COMMAND (generator expressions in the ENVIRONMENT property are NOT supported; COMMAND args are)"
    - "Dart test discovers the library ONLY via the injected env var; unset (bare IDE runs) → markTestSkipped with instructions, never per-platform path guessing"
    - "API_DL init contract: Dart_InitializeApiDL(NativeApi.initializeApiDLData) must be called on the Dart side BEFORE gcs_subscribe; the C++ half never self-initializes (nullptr segfaults in the vendored SDK source)"
    - "Host gate for Dart ctest legs is CMAKE_SYSTEM_NAME MATCHES Darwin|Linux|Windows — NOT NOT CMAKE_CROSSCOMPILING (this repo's apple toolchain forces CMAKE_CROSSCOMPILING=TRUE even on macOS host builds)"

key-files:
  created:
    - src/ffi/dart_native_api.h
    - src/ffi/dart_api_dl.h
    - src/ffi/dart_api_dl.c
    - src/ffi/dart/internal/dart_api_dl_impl.h (vendored internal include)
    - src/app/ffigen.yaml
    - src/app/lib/gcs_bindings_generated.dart
    - src/app/lib/generated/proto/gcs_chat.pb.dart (+ .pbenum/.pbjson)
    - src/app/test/gcs_native_port_smoke_test.dart
  modified:
    - src/ffi/gcs_core_ffi.cpp
    - src/ffi/CMakeLists.txt
    - src/app/CMakeLists.txt

key-decisions:
  - "Dart FFI tests are ctest tests, not IDE-only scripts: CMake owns path/suffix/config knowledge ($<TARGET_FILE:>), the test file contains zero platform paths (user-directed redesign after initial per-platform candidate-list draft was rejected)"
  - "gcs_ffi installs via install(TARGETS ... LIBRARY/RUNTIME/ARCHIVE/PUBLIC_HEADER) so consumers (Dart harness, plugin staging, packaging) never reach into the CMake build dir"
  - "Option-C skip is the accepted Task 4 outcome on this host (plan verification permits the skip marker); the live-node round-trip is scoped to 01-11's app-boot verification — user confirmed skip-for-now"
  - "protoc_plugin pinned at 22.5.0 (matching the committed generated bindings)"

patterns-established:
  - "Env-var-injected FFI library resolution for Dart tests (GCS_FFI_LIBRARY)"
  - "_EventQueue completer-based wait over ReceivePort (no polling; D-05)"

requirements-completed: [CORE-05]  # criterion 3 via functioning FFI entrypoint + proven call path; live round-trip legs to 01-11

# Metrics
duration: 2 sessions (Tasks 1-3 warm on 2026-08-26; Task 4 harness + registration debugging closed 2026-08-27)
completed: 2026-08-27
---

# Phase 1 Plan 5: Dart NativePort Summary

**Dart_PostCObject typed-data protobuf posting wired into the PostToDart seam, ffigen + Dart protobuf bindings generated from the shared proto, and the Dart-side NativePort smoke test registered as ctest test #21 — `ctest -R test_gcs_ffi_dart` green (option-C skip: dylib loads, API_DL init returns 0, config bytes parse, gcs_init returns clean nullptr without a live GeniusSDK node)**

## Performance
- **Tasks:** 4/4
- **ctest:** test_gcs_ffi_dart Passed 1.95s (24 tests total in tree)
- **Install:** ninja install places libgcs_ffi.dylib → `<prefix>/lib/`, gcs_core.h → `<prefix>/include/`

## Accomplishments
- Vendored the Dart API_DL set into src/ffi and linked it into gcs_ffi (resolves Dart_PostCObject/Dart_InitializeApiDL without a Flutter-engine link dependency)
- PostToDart now posts Dart_CObject kTypedData uint8 of the serialized GcsEvent — D-26 protobuf bytes over the port, decodable by package:protobuf on the Dart side
- ffigen bindings generated against the real src/ffi/gcs_core.h (ffigen.yaml); Dart protobuf bindings generated from the same src/proto/gcs_chat.proto as the C++ half (D-24)
- Discovered and documented the API_DL init contract (Dart-side initialize before subscribe; commit 19d9d64)
- Dart smoke test registered in the ctest harness alongside the gtests — `ninja && ctest` is the single entry point for C++ AND Dart legs
- CMake install rule for gcs_ffi (library + public header) verified end-to-end

## Task Commits
1. **Task 1: vendor Dart API_DL set + CMake wiring** - `b57a1dd`
2. **Task 2: PostToDart → Dart_PostCObject typed-data posting** - `1d39794` (+ docs follow-up `19d9d64` recording the Dart-side init contract)
3. **Task 3: ffigen.yaml + ffigen/Dart protobuf generation** - `78d44a8`
4. **Task 4: Dart NativePort smoke test + ctest registration** - this commit

## Files Created/Modified
- `src/app/test/gcs_native_port_smoke_test.dart` - full round-trip test (init → subscribe → RoomList/Readiness → join_topic → send_text → ChatMessageState echo assertions); env-var library discovery; completer-based _EventQueue
- `src/app/CMakeLists.txt` - add_test(test_gcs_ffi_dart) with `cmake -E env GCS_FFI_LIBRARY=$<TARGET_FILE:gcs_ffi>`; host gate by CMAKE_SYSTEM_NAME
- `src/ffi/CMakeLists.txt` - install(TARGETS gcs_ffi) with LIBRARY/RUNTIME/ARCHIVE/PUBLIC_HEADER destinations

## Decisions Made
- CTest-driven harness over hardcoded paths (user-directed; recorded as an ingestible cross-project pattern): CMake owns the path; env var is the only injection channel; skip-with-instructions when unset
- Host gate by platform NAME, not CMAKE_CROSSCOMPILING (toolchain artifact — see Deviations 2)
- Option-C skip accepted for Task 4; live-node legs belong to 01-11 (user confirmed)

## Deviations from Plan

### User-Directed Redesign
**1. Task 4 test registration — hardcoded library-path candidates rejected**
- **Found during:** Task 4 review (user: "horrible test. how will this work on Linux, Windows, OSX, android and IOs?")
- **Issue:** Initial draft guessed per-platform dylib/.so/.dll candidate paths from the build tree — unportable and config-coupled
- **Fix:** CTest-driven harness — CMake injects `GCS_FFI_LIBRARY=$<TARGET_FILE:gcs_ffi>` via `cmake -E env` in the add_test COMMAND (generator expressions work in COMMAND args on all supported CMake versions; the test ENVIRONMENT property does not support them); the Dart file marks the test skipped with instructions when the var is unset; install(TARGETS) gives consumers a stable per-platform artifact location
- **Files modified:** src/app/test/gcs_native_port_smoke_test.dart, src/app/CMakeLists.txt, src/ffi/CMakeLists.txt

### Auto-fixed Issues
**2. [Rule 3 - Blocking] `NOT CMAKE_CROSSCOMPILING` guard is always FALSE in this repo's build trees**
- **Found during:** Task 4 registration debugging (CTestTestfile mtime frozen; ctest never saw the test)
- **Issue:** build/apple.toolchain.cmake sets CMAKE_SYSTEM_NAME even for host-matching SDKs (MAC_UNIVERSAL), which makes CMake set CMAKE_CROSSCOMPILING=TRUE on a plain macOS host build (verified: `CMakeFiles/*/CMakeSystem.cmake(13): set(CMAKE_CROSSCOMPILING TRUE)`)
- **Fix:** Guard changed to `CMAKE_SYSTEM_NAME MATCHES "Darwin|Linux|Windows"` — true on desktop hosts, false for iOS/Android cross-builds; trap documented in the CMake comment
- **Verification:** configure green; test registers as ctest #21 of 24; ctest green

**3. [Rule 1 - Warning] install(TARGETS gcs_ffi) missing PUBLIC_HEADER destination**
- **Issue:** Target declares PUBLIC_HEADER gcs_core.h; the initial install rule omitted PUBLIC_HEADER DESTINATION (configure warning) and duplicated the header via install(FILES)
- **Fix:** Single rule — `install(TARGETS gcs_ffi LIBRARY/RUNTIME/ARCHIVE/PUBLIC_HEADER DESTINATION ...)`; verified by ninja install

**4. Option-C outcome on this host (expected, documented)**
- `gcs_init` returns clean nullptr: GeniusSDKGetNode() returns nullptr because the SDK init chain (D-20 ordering: SDK before GlobalDB) runs in the app process, not a bare test. Plan verification explicitly permits the skip marker for exactly this leg; the full live round-trip is 01-11's app-boot verification ("rail shows the pushed smoke topics")

**Total deviations:** 1 user-directed redesign + 2 auto-fixed (1 blocking) + 1 documented expected outcome. No scope creep beyond Task 4's registration surface.

## Issues Encountered
None in product code. One environment trap consumed most of Task 4's debugging budget: the CMAKE_CROSSCOMPILING artifact above (diagnosed via `cmake --trace-expand` after cache/guard/target checks all passed). Two self-inflicted repo-root in-source `cmake .` runs (missing `cd` in fresh shells) were cleaned up immediately (root CMakeCache.txt/CMakeFiles verified mine by timestamp + HOME_DIRECTORY, then removed).

## User Setup Required
None remaining. FLUTTER_ROOT was needed once (Task 1 vendoring) and is already consumed — the vendored headers are committed.

## Next Phase Readiness
- Plan 06 (CI): the Dart ctest leg runs wherever flutter is on PATH and the platform is a desktop host; CI must configure with -DBUILD_TESTS=ON and FRONTEND_BUILD_ENABLED=ON for the leg to register (it silently no-ops otherwise — by design)
- Plan 11 (app shell): SessionCubit's gcs_init/gcs_subscribe/gcs_publish call shapes are proven by the committed bindings + smoke test; the live-node round-trip lands there (SDK init chain boots in the app)
- The CTest-driven Dart FFI harness pattern is recorded above (tech-stack patterns) for ingestion into sibling projects

## Self-Check: PENDING HUMAN VERIFY
ctest green and install verified. ROADMAP success criterion 3 is satisfied to the option-C depth the plan's verification permits; the live-node depth is 01-11's. User gate: accept-and-close (recorded above) vs. insert a live-node test plan.

---
*Phase: 01-foundation*
*Completed: 2026-08-27*
