---
phase: 1
slug: foundation
status: ready
nyquist_compliant: true
wave_0_complete: false
created: 2026-08-15
---

# Phase 1 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | Google Test (thirdparty-provided; `find_package(GTest CONFIG REQUIRED)` in root CMakeLists under `BUILD_TESTING`) + Dart `flutter_test` for the Dart-side smoke |
| **Config file** | `test/CMakeLists.txt` (GCS root) — created in Plan 01-04 Task 1; `build/CommonBuildParameters.cmake:146-151` already has the `add_subdirectory(${PROJECT_ROOT}/test ...)` hook gated on `BUILD_TESTING` |
| **Quick run command** | `cmake --build build/OSX/Debug -j && ctest --test-dir build/OSX/Debug -R gcs --output-on-failure` |
| **Full suite command** | `ctest --test-dir build/OSX/Debug --output-on-failure` (includes neoswarm tests, which also build in the parent build) |
| **Dart-side smoke** | `cd src/app && flutter test test/gcs_native_port_smoke_test.dart` (Plan 01-05 Task 4 — human-verify checkpoint) |
| **Estimated runtime** | ~90 seconds for the ctest -R gcs quick run; full parent-build suite longer |

---

## Sampling Rate

- **After every task commit:** Run `cmake --build build/OSX/Debug -j && ctest --test-dir build/OSX/Debug -R gcs --output-on-failure`
- **After every plan wave:** Run `ctest --test-dir build/OSX/Debug --output-on-failure` (full parent-build suite)
- **Before `/gsd:verify-work`:** Full suite must be green on OSX; CI green on all 5 platforms via Plan 01-06
- **Max feedback latency:** ~90 seconds (OSX Debug incremental build + `ctest -R gcs`)

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 01-01-01 | 01 | 1 | CORE-05 | — | Public pass-through declarations on GcsGlobalDb (no behavior change) | static / grep | `grep -c "AddBroadcastTopic\|AddListenTopic\|::Put(\|::Get(" GNUS-NEO-SWARM/src/storage/gcs_global_db.hpp \| grep -q "^[4-9]\|^[1-9][0-9]"` | ❌ W0 | ⬜ pending |
| 01-01-02 | 01 | 1 | CORE-05 | T-01-04 (double-init / not-running misuse) | All four pass-throughs return `outcome::failure(Error::GcsDbError)` when `!m_running.load()` | static / grep + git diff | `cd GNUS-NEO-SWARM && git diff --stat HEAD && grep -c "GcsGlobalDb::AddBroadcastTopic\|GcsGlobalDb::AddListenTopic\|GcsGlobalDb::Put\|GcsGlobalDb::Get" src/storage/gcs_global_db.cpp \| grep -q "^4$"` | ❌ W0 | ⬜ pending |
| 01-01-03 | 01 | 1 | CORE-05 | — | Local commit on `feature/gcs-globaldb-passthrough` (stacked on `feature/app-restructure`); GCS submodule pointer NOT staged | git | `git -C GNUS-NEO-SWARM log -1 --oneline \| grep -q "pass-throughs to GcsGlobalDb"` | ❌ W0 | ⬜ pending |
| 01-02-01 | 02 | 2 | CORE-05 | — | `gcs_core PUBLIC neoswarm_storage` link is hard-required (configure-time FATAL_ERROR if missing) | static / grep | `grep -A1 "if(TARGET neoswarm_storage" src/CMakeLists.txt \| grep -q "target_link_libraries(gcs_core PUBLIC neoswarm_storage)"` | ❌ W0 | ⬜ pending |
| 01-02-02 | 02 | 2 | CORE-05 | — | `gcs::CoreSession` interface declared (header-only) | static / grep | `grep -c "class CoreSession\|Initialize\|Shutdown\|IsRunning\|AddBroadcastTopic\|AddListenTopic\|Put\|Get" src/lib/gcs_core.hpp \| grep -q "^[8-9]\|^[1-9][0-9]"` | ❌ W0 | ⬜ pending |
| 01-02-03 | 02 | 2 | CORE-05 | — | `gcs_core` static lib builds cleanly on OSX Debug with Plan 01-01's submodule branch checked out | build | `cmake --build build/OSX/Debug -j 2>&1 \| tail -20 \| grep -E "error\|FAILED" ; test ${PIPESTATUS[0]} -eq 0` | ❌ W0 | ⬜ pending |
| 01-03-01 | 03 | 3 | CORE-05 | — | `gcs_ffi` SHARED CMake target wired; `add_subdirectory(ffi)` lands | static / grep | `grep -q "add_library(gcs_ffi SHARED" src/ffi/CMakeLists.txt && grep -q "add_subdirectory(ffi)" src/CMakeLists.txt` | ❌ W0 | ⬜ pending |
| 01-03-02 | 03 | 3 | CORE-05 | T-01-05 (FFI export surface) | Single opaque-handle C API declared; `_WIN32` dllexport macro block is the ONLY OS preprocessor guard | static / grep | `grep -cE "GCS_FFI_API.*gcs_(init\|shutdown\|join_topic\|publish\|on_message\|string_free)" src/ffi/gcs_core.h \| grep -q "^6$"` | ❌ W0 | ⬜ pending |
| 01-03-03 | 03 | 3 | CORE-05 | T-01-02 (heap mismatch) / T-01-04 (double-shutdown) | All six `gcs_*` symbols exported from `libgcs_ffi.{dylib,so,dll}`; every function takes the global mutex; `gcs_on_message` stubbed to `GCS_ERROR_NOT_IMPLEMENTED` | build + nm | `cmake --build build/OSX/Debug -j 2>&1 \| tee /tmp/gcs_ffi_build.log \| tail -5 ; test -f build/OSX/Debug/gcs_src/ffi/libgcs_ffi.dylib && nm -gU build/OSX/Debug/gcs_src/ffi/libgcs_ffi.dylib \| grep -cE "_gcs_(init\|shutdown\|join_topic\|publish\|on_message\|string_free)" \| grep -q "^6$"` | ❌ W0 | ⬜ pending |
| 01-04-01 | 04 | 3 | CORE-05 | — | Test scaffolding (`gcs_test` macro + WaitForCondition template) present | static / file exists | `test -f test/CMakeLists.txt && test -f test/test_wait_condition.hpp && grep -q "gcs_test(test_gcs_core_smoke" test/CMakeLists.txt && grep -q "WaitForCondition" test/test_wait_condition.hpp` | ❌ W0 | ⬜ pending |
| 01-04-02 | 04 | 3 | CORE-05 | T-01-06 (test flakiness via sleep) | CORE-05 substrate smoke (lifecycle + topic + Put/Get round-trip over port-0 GossipPubSub); zero `sleep_for`/`sleep_until` | unit (gtest) | `grep -c "sleep_for\|sleep_until" test/test_gcs_core_smoke.cpp \| grep -q "^0$" && cmake --build build/OSX/Debug -j 2>&1 \| tail -10 \| grep -E "error\|FAILED" ; ctest --test-dir build/OSX/Debug -R test_gcs_core_smoke --output-on-failure 2>&1 \| tail -15 \| grep -E "Passed\|Failed"` | ❌ W0 | ⬜ pending |
| 01-04-03 | 04 | 3 | CORE-05 | T-01-01 (FFI boundary null/malformed input) / T-01-03 (use-after-free) | FFI surface smoke; `gcs_init` returns `nullptr` gracefully (no crash, no exception) when GeniusSDK node unavailable; argument validation + version + shutdown-safe-on-null tests do not require a live node; **no `gcs_on_message` tests in this plan** | unit (gtest) | `grep -c "sleep_for\|sleep_until" test/test_gcs_ffi.cpp \| grep -q "^0$" && cmake --build build/OSX/Debug -j 2>&1 \| tail -5 \| grep -E "error\|FAILED" ; ctest --test-dir build/OSX/Debug -R test_gcs_ffi --output-on-failure 2>&1 \| tail -15 \| grep -E "Passed\|Failed\|tests passed"` | ❌ W0 | ⬜ pending |
| 01-05-01 | 05 | 4 | CORE-05 | — | Dart API_DL headers vendored; gcs_ffi sources updated | build | `test -f src/ffi/dart_native_api.h && test -f src/ffi/dart_api_dl.c && grep -q "dart_api_dl.c" src/ffi/CMakeLists.txt && cmake --build build/OSX/Debug -j 2>&1 \| tail -5 \| grep -E "error\|FAILED" ; echo "build-ok"` | ❌ W0 | ⬜ pending |
| 01-05-02 | 05 | 4 | CORE-05 | T-01-03 (use-after-free across FFI) | `gcs_on_message` registers port; `gcs_publish` echoes payload via `Dart_PostCObject`; `gcs_shutdown` clears port before teardown | build + ctest | `cmake --build build/OSX/Debug -j 2>&1 \| tail -5 \| grep -E "error\|FAILED" ; ctest --test-dir build/OSX/Debug -R gcs --output-on-failure 2>&1 \| tail -10 \| grep -E "tests passed\|Passed\|Failed"` | ❌ W0 | ⬜ pending |
| 01-05-03 | 05 | 4 | CORE-05 | — | ffigen bindings generated against the real `src/ffi/gcs_core.h` | codegen + grep | `test -f src/app/ffigen.yaml && grep -q "gcs_core.h" src/app/ffigen.yaml && test -f src/app/lib/gcs_bindings_generated.dart && grep -c "gcs_init\|gcs_shutdown\|gcs_on_message" src/app/lib/gcs_bindings_generated.dart \| grep -q "^[3-9]\|^[1-9][0-9]"` | ❌ W0 | ⬜ pending |
| 01-05-04 | 05 | 4 | CORE-05 | — | Dart smoke: `gcs_init` → `gcs_on_message` → `gcs_publish` → `ReceivePort` round-trip (skip-on-null-init only) | manual / flutter test | `cd src/app && flutter test test/gcs_native_port_smoke_test.dart` | ❌ W0 | ⬜ pending (human-verify) |
| 01-06-01 | 06 | 5 | CORE-05 | — | CI workflow YAML valid; resolve-runners present; GeniusSDK download present; zkLLVM step deleted; ≥3 ctest invocations | static / yaml + grep | `test -f .github/workflows/cmake.yml && grep -q "resolve-runners" .github/workflows/cmake.yml && grep -q "GeniusVentures/GeniusSDK" .github/workflows/cmake.yml && ! grep -q "zkLLVM release\|Download zkLLVM" .github/workflows/cmake.yml && grep -cE "ctest" .github/workflows/cmake.yml \| grep -qE "^[3-9]\|^[1-9][0-9]" && python3 -c "import yaml,sys; yaml.safe_load(open('.github/workflows/cmake.yml'))" && echo "yaml-valid"` | ❌ W0 | ⬜ pending |
| 01-06-02 | 06 | 5 | CORE-05 | — | First CI run on push to develop is green across the matrix (OSX/Linux/Windows ctest; Android/iOS build-only) | manual / gh run watch | `gh run watch` (user-driven; Plan 01-06 Task 2 is a `checkpoint:human-action`) | ❌ W0 | ⬜ pending (human-action) |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `test/CMakeLists.txt` (GCS root) — registers smoke tests; gate on GTest found (Plan 01-04 Task 1)
- [ ] `test/test_wait_condition.hpp` — WaitForCondition template copied verbatim from neoswarm analog (Plan 01-04 Task 1)
- [ ] `test/test_gcs_core_smoke.cpp` — covers CORE-05 substrate (lifecycle + topic + Put/Get round-trip) (Plan 01-04 Task 2)
- [ ] `test/test_gcs_ffi.cpp` — covers FFI init/echo/shutdown, option C semantics (Plan 01-04 Task 3)
- [ ] `src/ffi/CMakeLists.txt` + `gcs_ffi` SHARED target (Plan 01-03 Task 1)
- [ ] `.github/workflows/cmake.yml` — CI workflow (Plan 01-06 Task 1)

`wave_0_complete: false` until all six land.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Dart NativePort round-trip smoke (`gcs_init` → `gcs_on_message` → `gcs_publish` → `ReceivePort` receives echo) | CORE-05 success criterion 3 | Requires a Flutter runtime + a loaded dylib; cannot be driven from ctest. `skip:` only for the live-node init-success path when GeniusSDK is unavailable in the test env. | `cd src/app && flutter test test/gcs_native_port_smoke_test.dart` — expect "+1 passed" or skip-with-note; Plan 01-05 Task 4 human-verify checkpoint |
| First CI run green across the full matrix on push to develop | CORE-05 success criterion 5 | Only the user can authorize the push; requires watching GitHub Actions on self-hosted runners | `gh run watch` or GitHub Actions web UI; Plan 01-06 Task 2 human-action checkpoint |

All other phase behaviors have automated verification.

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references (Wave 0 deliverables listed above; complete on Plan 01-03 + 01-04 + 01-06 landing)
- [x] No watch-mode flags
- [x] Feedback latency < 90s
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending — wave 0 artifacts land across Plans 01-03, 01-04, 01-06; sign-off after first green `ctest -R gcs` on OSX Debug.
