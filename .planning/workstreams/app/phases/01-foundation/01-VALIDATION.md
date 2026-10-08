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
| 01-01-01 | 01 | 1 | CORE-05 | — | Public pass-through declarations on GcsGlobalDb (no behavior change) | static / grep | `grep -c "AddBroadcastTopic\|AddListenTopic\|::Put(\|::Get(" src/lib/gcs_storage/gcs_global_db.hpp \| grep -q "^[4-9]\|^[1-9][0-9]"` | ❌ W0 | ⬜ pending |
| 01-01-02 | 01 | 1 | CORE-05 | T-01-04 (double-init / not-running misuse) | All four pass-throughs return `outcome::failure(Error::GcsDbError)` when `!m_running.load()` | static / grep + git diff | `grep -c "GcsGlobalDb::AddBroadcastTopic\|GcsGlobalDb::AddListenTopic\|GcsGlobalDb::Put\|GcsGlobalDb::Get" src/lib/gcs_storage/gcs_global_db.cpp \| grep -q "^4$"` | ❌ W0 | ⬜ pending |
| 01-01-03 | 01 | 1 | CORE-05 | — | `gcs_storage` STATIC target + `add_subdirectory(lib/gcs_storage)` wiring | static / grep | `grep -q "add_library(gcs_storage STATIC" src/lib/gcs_storage/CMakeLists.txt && grep -q "add_subdirectory(lib/gcs_storage)" src/CMakeLists.txt` | ❌ W0 | ⬜ pending |
| 01-01-04 | 01 | 1 | CORE-05 | — | Moved test + stub test/CMakeLists.txt staged in root test/ | static / grep | `test -f test/test_gcs_global_db.cpp && test -f test/CMakeLists.txt && grep -q 'gcs_storage/gcs_global_db.hpp' test/test_gcs_global_db.cpp` | ❌ W0 | ⬜ pending |
| 01-02-01 | 02 | 2 | CORE-05 | — | `gcs_core PUBLIC gcs_storage` link is hard-required (configure-time FATAL_ERROR if missing) | static / grep | `grep -A1 "if(TARGET gcs_storage" src/CMakeLists.txt \| grep -q "target_link_libraries(gcs_core PUBLIC gcs_storage)"` | ❌ W0 | ⬜ pending |
| 01-02-02 | 02 | 2 | CORE-05 | — | `gcs::CoreSession` interface declared (header-only) | static / grep | `grep -c "class CoreSession\|Initialize\|Shutdown\|IsRunning\|AddBroadcastTopic\|AddListenTopic\|Put\|Get" src/lib/gcs_core.hpp \| grep -q "^[8-9]\|^[1-9][0-9]"` | ❌ W0 | ⬜ pending |
| 01-02-03 | 02 | 2 | CORE-05 | — | `gcs_core` static lib builds cleanly on OSX Debug | build | `cmake --build build/OSX/Debug -j 2>&1 \| tail -20 \| grep -E "error\|FAILED" ; test ${PIPESTATUS[0]} -eq 0` | ❌ W0 | ⬜ pending |
| 01-03-01 | 03 | 3 | CORE-05 | — | `gcs_chat.proto` (GcsConfig/Codec + GcsCommand + GcsEvent envelopes with oneofs) + `gcs_proto` target via add_proto_library | static / grep | `test -f src/proto/gcs_chat.proto && grep -q "package gcs.chat" src/proto/gcs_chat.proto && grep -q "GcsCommand" src/proto/gcs_chat.proto && grep -c "oneof payload" src/proto/gcs_chat.proto \| grep -q "^2$" && grep -q "add_proto_library(gcs_proto" src/proto/CMakeLists.txt && grep -q "add_subdirectory(proto)" src/CMakeLists.txt` | ❌ W0 | ⬜ pending |
| 01-03-02 | 03 | 3 | CORE-05 | T-01-05 (FFI export surface) | Four-function opaque-handle C ABI declared (D-27/D-29: bytes payloads, never char*); `_WIN32` dllexport macro block is the ONLY OS preprocessor guard | static / grep | `grep -cE "GCS_FFI_API.*gcs_(init\|shutdown\|publish\|subscribe)" src/ffi/gcs_core.h \| grep -q "^4$" && ! grep -qE "gcs_(on_message\|join_topic\|string_free)" src/ffi/gcs_core.h && ! grep -q "GCS_ERROR_NOT_IMPLEMENTED" src/ffi/gcs_core.h && grep -q "GCS_ERROR_UNSUPPORTED_CODEC" src/ffi/gcs_core.h` | ❌ W0 | ⬜ pending |
| 01-03-03 | 03 | 3 | CORE-05 | T-01-03 (buffer ownership) / T-01-04 (double-shutdown) | All four `gcs_*` symbols exported from `libgcs_ffi.{dylib,so,dll}`; every function takes the global mutex; GcsCommand parse/dispatch + pushed GcsEvent (RoomList/Readiness/ErrorNotice) | build + nm | `cmake --build build/OSX/Debug -j 2>&1 \| tee /tmp/gcs_ffi_build.log \| tail -5 ; test -f build/OSX/Debug/gcs_src/ffi/libgcs_ffi.dylib && nm -gU build/OSX/Debug/gcs_src/ffi/libgcs_ffi.dylib \| grep -cE "_gcs_(init\|shutdown\|publish\|subscribe)" \| grep -q "^4$"` | ❌ W0 | ⬜ pending |
| 01-04-01 | 04 | 4 | CORE-05 | — | Test scaffolding (`gcs_test` macro + WaitForCondition template) present | static / file exists | `test -f test/CMakeLists.txt && test -f test/test_wait_condition.hpp && grep -q "gcs_test(test_gcs_core_smoke" test/CMakeLists.txt && grep -q "WaitForCondition" test/test_wait_condition.hpp` | ❌ W0 | ⬜ pending |
| 01-04-02 | 04 | 4 | CORE-05 | T-01-06 (test flakiness via sleep) | CORE-05 substrate smoke (lifecycle + topic + Put/Get round-trip over port-0 GossipPubSub); zero `sleep_for`/`sleep_until` | unit (gtest) | `grep -c "sleep_for\|sleep_until" test/test_gcs_core_smoke.cpp \| grep -q "^0$" && cmake --build build/OSX/Debug -j 2>&1 \| tail -10 \| grep -E "error\|FAILED" ; ctest --test-dir build/OSX/Debug -R test_gcs_core_smoke --output-on-failure 2>&1 \| tail -15 \| grep -E "Passed\|Failed"` | ❌ W0 | ⬜ pending |
| 01-04-03 | 04 | 4 | CORE-05 | T-01-01 (FFI boundary null/malformed input) / T-01-03 (buffer ownership) | FFI surface smoke; `gcs_init(configBytes, configLength)` returns `nullptr` gracefully (no crash, no exception) on null args / garbage bytes / unsupported codec / GeniusSDK-node unavailability; publish/subscribe argument validation + shutdown-safe-on-null tests do not require a live node; **no live-port round-trip tests in this plan** | unit (gtest) | `grep -c "sleep_for\|sleep_until" test/test_gcs_ffi.cpp \| grep -q "^0$" && ! grep -qE "gcs_(on_message\|join_topic\|string_free)" test/test_gcs_ffi.cpp && cmake --build build/OSX/Debug -j 2>&1 \| tail -5 \| grep -E "error\|FAILED" ; ctest --test-dir build/OSX/Debug -R test_gcs_ffi --output-on-failure 2>&1 \| tail -15 \| grep -E "Passed\|Failed\|tests passed"` | ❌ W0 | ⬜ pending |
| 01-05-01 | 05 | 5 | CORE-05 | — | Dart API_DL headers vendored; gcs_ffi sources updated | build | `test -f src/ffi/dart_native_api.h && test -f src/ffi/dart_api_dl.c && grep -q "dart_api_dl.c" src/ffi/CMakeLists.txt && cmake --build build/OSX/Debug -j 2>&1 \| tail -5 \| grep -E "error\|FAILED" ; echo "build-ok"` | ❌ W0 | ⬜ pending |
| 01-05-02 | 05 | 5 | CORE-05 | T-01-03 (use-after-free across FFI) | PostToDart posts Dart_CObject_kTypedData (uint8) of serialized GcsEvent bytes; gcs_shutdown clears port before teardown | build + ctest | `cmake --build build/OSX/Debug -j 2>&1 \| tail -5 \| grep -E "error\|FAILED" ; ctest --test-dir build/OSX/Debug -R gcs --output-on-failure 2>&1 \| tail -10 \| grep -E "tests passed\|Passed\|Failed" && grep -q "Dart_PostCObject" src/ffi/gcs_core_ffi.cpp` | ❌ W0 | ⬜ pending |
| 01-05-03 | 05 | 5 | CORE-05 | — | ffigen bindings + protobuf Dart bindings generated against the real src/ffi/gcs_core.h + src/proto/gcs_chat.proto | codegen + grep | `test -f src/app/ffigen.yaml && grep -q "gcs_core.h" src/app/ffigen.yaml && test -f src/app/lib/gcs_bindings_generated.dart && grep -q "gcs_subscribe" src/app/lib/gcs_bindings_generated.dart && grep -q "gcs_publish" src/app/lib/gcs_bindings_generated.dart && ! grep -q "gcs_on_message" src/app/lib/gcs_bindings_generated.dart && test -f src/app/lib/generated/proto/gcs_chat.pb.dart && grep -q "GcsEvent" src/app/lib/generated/proto/gcs_chat.pb.dart && grep -q "GcsCommand" src/app/lib/generated/proto/gcs_chat.pb.dart` | ❌ W0 | ⬜ pending |
| 01-05-04 | 05 | 5 | CORE-05 | — | Dart smoke: `gcs_init(GcsConfig bytes)` → `gcs_subscribe` → pushed RoomList/Readiness + `gcs_publish` of GcsCommand (join_topic + send_text) with echo decoded as ChatMessageState (skip-on-null-init only) | manual / flutter test | `cd src/app && flutter test test/gcs_native_port_smoke_test.dart` | ❌ W0 | ⬜ pending (human-verify) |
| 01-06-01 | 06 | 5 | CORE-05 | — | CI workflow YAML valid; resolve-runners present; GeniusSDK download present; zkLLVM step deleted; `-DBUILD_TESTS=ON` passed; ≥3 ctest invocations | static / yaml + grep | `test -f .github/workflows/cmake.yml && grep -q "resolve-runners" .github/workflows/cmake.yml && grep -q "GeniusVentures/GeniusSDK" .github/workflows/cmake.yml && ! grep -q "zkLLVM release\|Download zkLLVM" .github/workflows/cmake.yml && grep -q "BUILD_TESTS" .github/workflows/cmake.yml && grep -cE "ctest" .github/workflows/cmake.yml \| grep -qE "^[3-9]\|^[1-9][0-9]" && python3 -c "import yaml,sys; yaml.safe_load(open('.github/workflows/cmake.yml'))" && echo "yaml-valid"` | ❌ W0 | ⬜ pending |
| 01-06-02 | 06 | 5 | CORE-05 | — | First CI run on push to develop is green across the matrix (OSX/Linux/Windows ctest; Android/iOS build-only) | manual / gh run watch | `gh run watch` (user-driven; Plan 01-06 Task 2 is a `checkpoint:human-action`) | ❌ W0 | ⬜ pending (human-action) |
| 01-07-01 | 07 | 2 | CORE-05 | — | pubspec drops chat kit, KEEPS neoswarm_ffi (D-03), adds flutter_bloc + ffigen + protobuf + protoc_plugin | static / grep | `grep -q "flutter_bloc: \^9.0.0" src/app/pubspec.yaml && grep -q "ffigen: \^11.0.0" src/app/pubspec.yaml && grep -q "protobuf" src/app/pubspec.yaml && grep -q "protoc_plugin" src/app/pubspec.yaml && ! grep -q "flutter_chat_ui\|flutter_chat_core" src/app/pubspec.yaml && grep -q "neoswarm_ffi" src/app/pubspec.yaml && grep -q "frontend_scaffold:" src/app/pubspec.yaml` | ❌ W0 | ⬜ pending |
| 01-07-02 | 07 | 2 | CORE-05 | — | src/app/CMakeLists.txt FRONTEND_BUILD_ENABLED gate + cache vars BEFORE add_subdirectory(scaffold) | static / grep | `grep -q "option(FRONTEND_BUILD_ENABLED" src/app/CMakeLists.txt && grep -q "set(TEMPLATES_DIR" src/app/CMakeLists.txt && grep -q "set(GENERATED_DIR" src/app/CMakeLists.txt && grep -q "add_subdirectory(.*scaffold" src/app/CMakeLists.txt && ! grep -q "frontend_generate_api" src/app/CMakeLists.txt` | ❌ W0 | ⬜ pending |
| 01-08-01 | 08 | 3 | CORE-05 | — | bubble templates + vars (roles/states axes) authored (Dart triple only; no C++ half) | static / grep | `test -f src/app/templates/components/chat_message_bubble.dart.jinja2 && grep -q '"roles"' src/app/templates/components/chat_message_bubble_vars.json && grep -q '"states"' src/app/templates/components/chat_message_bubble_vars.json && grep -q "for role in roles" src/app/templates/components/chat_message_bubble.dart.jinja2` | ❌ W0 | ⬜ pending |
| 01-08-02 | 08 | 3 | CORE-05 | — | bubble codegen loop wired; generated bubble has 4 role classes | codegen + grep | `test -f src/app/lib/generated/chat/chat_message_bubble.dart && grep -q "ChatMessageBubbleUserSelf" src/app/lib/generated/chat/chat_message_bubble.dart && grep -q "app_generate_chat_components" src/app/CMakeLists.txt && ! grep -q "nlohmann" src/app/CMakeLists.txt` | ❌ W0 | ⬜ pending |
| 01-09-01 | 09 | 4 | CORE-05 | — | code_block + media templates authored (payload code/media, no bubble chrome) | static / grep | `test -f src/app/templates/components/chat_message_code_block.dart.jinja2 && test -f src/app/templates/components/chat_message_media.dart.jinja2 && grep -q '"payload": "code"' src/app/templates/components/chat_message_code_block_vars.json && grep -q '"payload": "media"' src/app/templates/components/chat_message_media_vars.json` | ❌ W0 | ⬜ pending |
| 01-09-02 | 09 | 4 | CORE-05 | — | code_block + media codegen loop rendered (no flow yet) | codegen + grep | `test -f src/app/lib/generated/chat/chat_message_code_block.dart && test -f src/app/lib/generated/chat/chat_message_media.dart && grep -q "chat_message_code_block" src/app/CMakeLists.txt && ! grep -q "chat_message_flow" src/app/CMakeLists.txt` | ❌ W0 | ⬜ pending |
| 01-10-01 | 10 | 5 | CORE-05 | — | flow template authored (sealed ChatFlowItem + item_types) | static / grep | `test -f src/app/templates/components/chat_message_flow.dart.jinja2 && grep -q "item_types" src/app/templates/components/chat_message_flow_vars.json && grep -q "ChatFlowItem" src/app/templates/components/chat_message_flow.dart.jinja2` | ❌ W0 | ⬜ pending |
| 01-10-02 | 10 | 5 | CORE-05 | — | flow codegen loop rendered (sealed class + 3 subclasses) | codegen + grep | `test -f src/app/lib/generated/chat/chat_message_flow.dart && grep -q "sealed class ChatFlowItem" src/app/lib/generated/chat/chat_message_flow.dart` | ❌ W0 | ⬜ pending |
| 01-11-01 | 11 | 6 | CORE-05 | — | shell layout (GCSChat: RoomRail + ChatMessageFlow + composer) | static / grep | `test -f src/app/lib/shell/gcs_shell.dart && test -f src/app/lib/shell/room_rail.dart && grep -q "GCSChat" src/app/lib/shell/gcs_shell.dart && grep -q "RoomRail" src/app/lib/shell/room_rail.dart` | ❌ W0 | ⬜ pending |
| 01-11-02 | 11 | 6 | CORE-05 | T-01-11-04 (handle misuse) | cubits: SessionCubit FFI lifecycle + GcsEvent decode; RailCubit setRooms (pushed); MessageFlowCubit append; ComposerCubit publishes GcsCommand bytes | static / grep | `grep -q "gcs_init\|gcs_shutdown\|gcs_subscribe" src/app/lib/cubits/session_cubit.dart && grep -q "GcsEvent.fromBuffer" src/app/lib/cubits/session_cubit.dart && grep -q "ChatFlowItem" src/app/lib/cubits/message_flow_cubit.dart && grep -q "gcs_publish" src/app/lib/cubits/composer_cubit.dart && grep -q "SendTextCommand" src/app/lib/cubits/composer_cubit.dart && grep -q "setRooms" src/app/lib/cubits/rail_cubit.dart` | ❌ W0 | ⬜ pending |
| 01-11-03 | 11 | 6 | CORE-05 | — | main.dart GCSChatApp + themed Material 3; widget_test pumps GCSChat | flutter test | `test -f src/app/lib/theme/gcs_theme.dart && ! grep -q "GeniusSwarmApp\|0xFF6C3CE1\|flutter_chat_ui" src/app/lib/main.dart && grep -q "GCSChat" src/app/lib/main.dart && cd src/app && flutter test test/widget_test.dart 2>&1 \| tail -5` | ❌ W0 | ⬜ pending |
| 01-11-04 | 11 | 6 | CORE-05 | — | Three smoke tests: theme registration (light+dark), shell render (rail+flow+composer + rail switch + composer gating), composites (per-role/state bubbles + interleaved flow) | flutter test | `cd src/app && flutter test test/theme_registration_test.dart test/chat_shell_test.dart test/generated/chat_composites_test.dart 2>&1 \| tail -5` | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `test/CMakeLists.txt` (GCS root) — registers smoke tests; gate on GTest found (Plan 01-04 Task 1)
- [ ] `test/test_wait_condition.hpp` — WaitForCondition template copied verbatim from neoswarm analog (Plan 01-04 Task 1)
- [ ] `test/test_gcs_core_smoke.cpp` — covers CORE-05 substrate (lifecycle + topic + Put/Get round-trip) (Plan 01-04 Task 2)
- [ ] `test/test_gcs_ffi.cpp` — covers FFI init/echo/shutdown, option C semantics (Plan 01-04 Task 3)
- [ ] `src/ffi/CMakeLists.txt` + `gcs_ffi` SHARED target (Plan 01-03 Task 2)
- [ ] `src/proto/gcs_chat.proto` + `gcs_proto` target (Plan 01-03 Task 1)
- [ ] `.github/workflows/cmake.yml` — CI workflow with `-DBUILD_TESTS=ON` (Plan 01-06 Task 1)
- [ ] `src/app/lib/generated/proto/gcs_chat.pb.dart` — Dart protobuf bindings (Plan 01-05 Task 3)

`wave_0_complete: false` until all land.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Dart protobuf NativePort round-trip smoke (`gcs_init(GcsConfig bytes)` → `gcs_subscribe` → pushed RoomList/Readiness + `gcs_publish` of GcsCommand with echo decoded as ChatMessageState) | CORE-05 success criterion 3 | Requires a Flutter runtime + a loaded dylib; cannot be driven from ctest. `skip:` only for the live-node init-success path when GeniusSDK is unavailable in the test env. | `cd src/app && flutter test test/gcs_native_port_smoke_test.dart` — expect "+1 passed" or skip-with-note; Plan 01-05 Task 4 human-verify checkpoint |
| First CI run green across the full matrix on push to develop | CORE-05 success criterion 5 | Only the user can authorize the push; requires watching GitHub Actions on self-hosted runners | `gh run watch` or GitHub Actions web UI; Plan 01-06 Task 2 human-action checkpoint |

All other phase behaviors have automated verification.

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references (Wave 0 deliverables listed above; complete on Plan 01-03 + 01-04 + 01-05 + 01-06 landing)
- [x] No watch-mode flags
- [x] Feedback latency < 90s
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending — wave 0 artifacts land across Plans 01-03, 01-04, 01-05, 01-06; sign-off after first green `ctest -R gcs` on OSX Debug.
