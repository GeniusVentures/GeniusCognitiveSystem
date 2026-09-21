---
status: partial
phase: 01-foundation
source: [01-01-SUMMARY.md, 01-02-SUMMARY.md, 01-03-SUMMARY.md, 01-04-SUMMARY.md, 01-05-SUMMARY.md, 01-07-SUMMARY.md, 01-08-SUMMARY.md, 01-09-SUMMARY.md, 01-10-SUMMARY.md]
started: 2026-08-29T18:00:00Z
updated: 2026-08-31T20:29:06Z
---

## Current Test
<!-- OVERWRITE each test - shows where we are -->

[testing complete]

## Tests

### 1. Full Build Green
expected: From build/OSX/Debug: `cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug && ninja` completes with zero errors and produces libgcs_ffi.dylib (under gcs_src/ffi/) alongside the gcs_test binaries. Post code-review-fix state (18 fixes landed on gcs_core_ffi.cpp, gcs_global_db, app CMake).
result: pass
verified: 2026-08-31T20:25:56Z — configure clean; ninja 24/24 zero errors; libgcs_ffi.dylib + 4 gcs_test binaries linked

### 2. C++ Smoke Suite Passes
expected: From build/OSX/Debug (BUILD_TESTS=ON): `ctest -R gcs` — all gcs test binaries pass: test_gcs_storage (GlobalDB lifecycle + graphsync-borrow regression), test_gcs_core_smoke (CoreSession + Put→Get), test_gcs_ffi (8 ABI tests incl. post-fix arg validation), test_gcs_global_db_sdk (SDK wiring; may skip if node cannot boot — documented upstream teardown flake is a known non-failure for the suite).
result: pass
verified: 2026-08-31T20:26:46Z — ctest -R gcs: 100% (6/6) passed, 0 failed, 15.17s total

### 3. Dart FFI Smoke Leg Passes
expected: From build/OSX/Debug: `ctest -R test_gcs_ffi_dart` — Passed. The Dart test loads libgcs_ffi.dylib via GCS_FFI_LIBRARY, Dart_InitializeApiDL returns 0, config bytes parse, and gcs_init returns a clean nullptr without a live GeniusSDK node (option-C depth; live-node depth is 01-11).
result: pass
verified: 2026-08-31T20:26:46Z — standalone ctest -R test_gcs_ffi_dart: Passed, 1.97s (also passed within Test 2 suite run)

### 4. Dart Analyze Gate Clean
expected: From build/OSX/Debug: `ninja app_analyze` (or `dart analyze --fatal-infos lib test` in src/app) reports zero issues. Pre-fix this was 52 issues (missing fixnum dep, MyApp/GeniusSwarmApp mismatch, scaffold/example sweep) — all fixed in the code-review fix run.
result: pass
verified: 2026-08-31T20:27:22Z — ninja app_analyze: dart analyze --fatal-infos lib test → "No issues found!" (exit 0)

### 5. Chat Component Codegen Drift-Free
expected: From build/OSX/Debug: `ninja app_generate_chat_components` generates the four composite triples (bubble, code_block, media, flow) into src/app/lib/generated/chat/; running it again changes nothing (byte-identical). Includes the post-fix flow cubit cache (WR-05) and state-casing pipeline (IN-07).
result: pass
verified: 2026-08-31T20:27:48Z — two consecutive codegen runs produced 12 byte-identical files (sha256); git status of src/app/lib/generated/chat clean (committed artifacts in sync)

### 6. FFI ABI Export Surface
expected: `nm -gU build/OSX/Debug/gcs_src/ffi/libgcs_ffi.dylib | grep gcs_` shows exactly four exports: _gcs_init, _gcs_publish, _gcs_shutdown, _gcs_subscribe — no dead gcs_on_message/gcs_join_topic/gcs_string_free reappear.
result: pass
verified: 2026-08-31T20:27:48Z — exactly four T-section C-ABI functions: _gcs_init/_gcs_publish/_gcs_shutdown/_gcs_subscribe; none of the dead symbols present. Note: literal `grep gcs_` also matches two protobuf data symbols (proto_2fgcs_5fchat_2eproto descriptor tables, D/S sections — linked gcs_chat.proto registration data, not FFI entry points)

### 7. Install Rule Places Artifacts
expected: From build/OSX/Debug: `ninja install` places libgcs_ffi.dylib under <prefix>/lib/ and ffi/gcs_core.h under <prefix>/include/ (single install rule, no duplicate header).
result: pass
verified: 2026-08-31T20:29:06Z — DESTDIR-staged `ninja install` (exit 0) places libgcs_ffi.dylib under <prefix>/lib/ and gcs_core.h under <prefix>/include/; header appears exactly once (no duplicate install rule)

### 8. Live-Node FFI Round-Trip
expected: With a booted GeniusSDK node in the app process: gcs_init succeeds, subscribe pushes non-empty RoomList + Readiness(ready), join_topic appends, send_text stores per-message records (room+message id keys — post-CR-01) and echoes ChatMessageState. This is the live-node depth the review fixes flagged for human verification; scoped to 01-11 app boot.
result: blocked
blocked_by: prior-phase
reason: "Requires app-process boot wiring built in plan 01-11 (cubits+shell), which is not yet executed; test is scoped to 01-11 verification by its own note. Re-run in 01-11 UAT."

## Summary

total: 8
passed: 7
issues: 0
pending: 0
skipped: 0
blocked: 1

## Gaps

[none yet]
