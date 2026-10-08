---
phase: 01-foundation
verified: 2026-09-17T01:49:35Z
status: verified_with_deviations
score: 5/5 success criteria verified (2 with documented deviations/contracts)
deviations:
  - must_have: "C++ core library builds on macOS, Linux, Windows, iOS, Android without errors (SC1)"
    reason: "Windows Debug+Release CI cells red: upstream GeniusSDK Windows release assets stale (2026-08-27, bool-vs-WitnessVerdict signature mismatch, LNK2001) and zkLLVM publishes no Debug artifacts (Release LLVM -> LNK2038 in MSVC Debug). Externally owned by the Windows-zkLLVM engineer; not caused by GCS code. macOS/Linux/iOS/Android are 10/10 green on the HEAD commit."
    accepted_by: user (decision recorded in 01-06-SUMMARY.md, 2026-09-16)
    accepted_at: 2026-09-16
  - must_have: "Flutter app can call into C++ via FFI and receive a response (SC3)"
    reason: "Full Dart-side event round-trip (subscribe -> pushed RoomList/Readiness -> send_text echo) executes only when a GeniusSDK node is embedded in the process; in the ctest harness gcs_init returns null and the Dart smoke test takes the documented option-C skip. Dart->C++ FFI calling IS exercised (DynamicLibrary.open, Dart_InitializeApiDL==0, gcs_init call with config bytes returning the contract-specified graceful null), and the identical data plane is proven end-to-end by test_gcs_ffi_sdk with a live node. Option C locked at planning time in 01-04-PLAN.md lines 224-226 and 01-05-PLAN.md line 355."
    accepted_by: planner (option C locked at planning time; phase contract)
    accepted_at: 2026-08-23
deferred:
  - truth: "Messages propagate over the network to other participants (real pub/sub flow; gcs_core_ffi.cpp line ~351 stores locally and pushes echo only)"
    addressed_in: "Phase 3 (Messaging)"
    evidence: "Phase 3 goal: 'Real-time text messaging with CRDT sync across participants'; SC2: 'A second user in the same room receives the message without manual refresh'. Also matches CORE-04 requirement."
  - truth: "Topic-graded event delivery (per-topic push filtering; gcs_core_ffi.cpp line ~394 notes single-stream delivery in Phase 1)"
    addressed_in: "Phase 3 (Messaging)"
    evidence: "Phase 3 success criteria require per-room message delivery semantics."
re_verification: false
---

# Phase 01: Foundation — Verification Report

**Phase Goal:** The C++ chat core compiles, links against GlobalDB, and exposes a working FFI surface that Flutter can call.
**Verified:** 2026-09-17T01:49:35Z
**Status:** verified_with_deviations
**Re-verification:** No — initial verification (no previous VERIFICATION.md existed)

## Per-Criterion Verification

| # | Success Criterion | Status | Evidence |
|---|-------------------|--------|----------|
| 1 | C++ core library builds on macOS, Linux, Windows, iOS, Android without errors | VERIFIED (macOS/Linux/iOS/Android); DEVIATION (Windows, externally owned) | CI run 35171573951 on HEAD commit 8a24c10 (all 15 cells complete): OSX Debug+Release PASS, Linux x86_64+aarch64 Debug+Release PASS (4/4), iOS Debug+Release PASS, Android arm64-v8a+armeabi-v7a Debug+Release PASS (4/4). Windows Debug+Release FAIL in the "Build GCS" step — the documented LNK2001/LNK2038 upstream asset issue (see Deviation 1). Same 13/15 pattern on prior completed run 35169234234. Local macOS build tree also builds/tests clean (build/OSX/Debug). |
| 2 | GlobalDB CRDT instance can be initialized and a test op published/subscribed locally | VERIFIED | `src/lib/gcs_storage/gcs_global_db.{hpp,cpp}` (237/279 lines, real sgns::crdt::GlobalDB lifecycle + Put/Get/AddListenTopic/AddBroadcastTopic). ctest run 2026-09-17 in build/OSX/Debug: **26/26 passed** (full suite, exit 0), incl. `test_gcs_global_db` (#4, lifecycle/double-init/borrowed-Network/topics), `test_gcs_storage` (#22), `test_gcs_core_smoke` (#24: `CrdtPutGetRoundTripsOnGcsChatTopic` — AddBroadcastTopic+AddListenTopic on a real GossipPubSub port 0, then Put->Get round-trip via WaitForCondition), `test_gcs_ffi_sdk` (#26: two full init/send-text/shutdown cycles on a live GeniusSDK node + record persistence re-read through a second GcsGlobalDb). |
| 3 | Flutter app can call into C++ via FFI and receive a response | VERIFIED (with documented option-C contract on the full round-trip) | `src/app/test/gcs_native_port_smoke_test.dart` (ctest #21 `test_gcs_ffi_dart`, PASS): opened libgcs_ffi.dylib via ctest-injected GCS_FFI_LIBRARY, `Dart_InitializeApiDL` returned 0 (assert passed pre-skip), `gcs_init(configBytes)` invoked and returned the contract-specified graceful null -> option-C skip ("All tests skipped" per plan contract; Dart FFI calling proven through init). The same four-function ABI data plane is exercised end-to-end with a live node by `test_gcs_ffi_sdk.SendTextRecordsSurviveAcrossSessionCyclesOnSharedDb` (PASS, 8.9s: GeniusSDKInit -> gcs_init -> gcs_subscribe -> send_text publish -> persistence). App-side runtime wiring: `SessionCubit.openDefault` (src/app/lib/cubits/session_cubit.dart:145) resolves libgcs_ffi via GCS_FFI_LIBRARY env var then exe-relative `../Frameworks/` and exe dir, initializes API_DL, hands GcsBindings to the session; called from `src/app/lib/shell/gcs_shell.dart:85`. |
| 4 | GossipSub topics can be created and joined from the C++ core | VERIFIED | `src/ffi/gcs_core_ffi.cpp` lines 219-245: kSmokeTopicA/B ("gcs/chat/smoke-test", "gcs/chat/smoke-test-2") pre-joined listen+broadcast during gcs_init; init FAILS if no smoke topic joins (non-empty RoomList contract). Tests: `test_gcs_core_smoke` topic join + Put->Get over real GossipPubSub (PASS); `test_gcs_ffi_sdk` RunSessionCycle subscribes/publishes over the live node (PASS); `test_gcs_global_db` topic + lifecycle tests (PASS). |
| 5 | CI/CD pipeline builds and tests on self-hosted runners for macOS, Linux, Windows, iOS, Android | VERIFIED (Windows cells red per Deviation 1) | `.github/workflows/cmake.yml` (626 lines): 5-platform x Debug/Release matrix (Android/iOS/OSX/Linux/Windows) on self-hosted runners (sg-ubuntu-linux, sg-arm-linux, SG-WIN11, gv-OSX-Large) via resolve-runners job; BUILD_TESTS=ON + ctest steps on OSX/Linux/Windows (Android/iOS are cross-compiles, no ctest per D-08). Live proof: run 35171573951 on HEAD 8a24c10 — resolve-runners PASS, 13/15 build cells green, 2 Windows cells red (documented deviation). |

**Score:** 5/5 criteria verified (SC1 and SC3 carry the documented deviations above)

## Behavioral Spot-Checks (executed 2026-09-17)

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| Full local ctest suite | `ctest` in build/OSX/Debug | 26/26 passed, 0 failed (36.7s) | PASS |
| GCS-specific suites | `ctest -R gcs` | 7/7 passed on re-run (22.3s) | PASS (see flake note) |
| Dart FFI smoke via ctest | `ctest -R test_gcs_ffi_dart -V` | Test PASS; took documented option-C skip after real FFI calls (DynamicLibrary.open + Dart_InitializeApiDL==0 + gcs_init->null) | PASS (contract skip) |
| CI matrix on HEAD | `gh pr checks 12` (run 35171573951, commit 8a24c10) | 13/15 cells green; OSX/Linux/iOS/Android 10/10 green; Windows 2 red | PASS w/ deviation |

## Requirements Coverage

| Requirement | Source | Status | Evidence |
|-------------|--------|--------|----------|
| CORE-05: "Messages sync via CRDT across all room participants" | Phase 1 (REQUIREMENTS.md, marked Complete) | SATISFIED at Phase-1 substrate scope | CRDT store init + topic pub/sub + Put->Get round-trip + cross-session record persistence all proven (SC2/SC4 evidence). Full multi-participant network sync is explicitly Phase 3 roadmap work (deferred items below); the requirement checkbox in REQUIREMENTS.md reflects the Phase-1 substrate delivery. |

## Anti-Pattern Scan

No TBD/FIXME/XXX/TODO/HACK/PLACEHOLDER markers in any phase key file scanned (src/ffi/gcs_core_ffi.cpp, gcs_core.h, src/lib/gcs_storage/*, src/lib/gcs_core.*, src/app/lib/cubits/session_cubit.dart, test/test_gcs_{core_smoke,ffi,ffi_sdk}.cpp, .github/workflows/cmake.yml). Zero blocker-level, zero warning-level findings.

## Observations / Warnings (non-blocking)

1. **test_gcs_global_db_sdk flake (WARNING):** aborted once ("Subprocess aborted") in a serial `ctest -R gcs` batch during this verification, then passed in isolation, in the batch re-run, and in the full 26/26 suite run. Documented in 01-06-SUMMARY.md (issue 4) as a boot-contention flake ("SEGFAULTs under ctest -j4, passes serially") — my observation shows it can flake in a serial batch as well. Not a phase-goal blocker (CI green, deterministic on retry), but worth a follow-up with the owning team.
2. **ROADMAP.md progress table stale (INFO):** says "Plans: 1/11 plans executed" and the Progress table shows "1/11", but all 11 plans have SUMMARY files and checkboxes for 01-01/01-06/01-11 are checked while 01-02..01-05, 01-07..01-10 are not. The codebase shows all 11 plans' artifacts present and tested. Recommend the orchestrator sync ROADMAP.md checkboxes at phase close.

## Human Verification Suggested (non-blocking)

### 1. Packaged Flutter app runtime on macOS

**Test:** Build the macOS app bundle with libgcs_ffi.dylib placed in `Contents/Frameworks/`, launch the packaged app (not `flutter run`).
**Expected:** Shell renders; SessionCubit.openDefault resolves the exe-relative library and initializes (or surfaces a clear error string when no GeniusSDK node is embedded, per option C).
**Why human:** Packaged-bundle layout and runtime library resolution cannot be exercised by ctest; automation proved the resolution logic and the ctest-injected env-var path only.

## Gaps Summary

None. All five success criteria are delivered and evidenced in the codebase, tests, and CI. Two documented deviations apply (Windows CI cells — externally owned, accepted by the user 2026-09-16; option-C skip on the Dart full round-trip — locked at planning time). Two items are consciously deferred to Phase 3 by roadmap design (network message propagation, topic-graded delivery). One warning-level flake (test_gcs_global_db_sdk boot contention) is documented and non-blocking.

---

_Verified: 2026-09-17T01:49:35Z_
_Verifier: Claude (gsd-verifier)_
_Do not commit — leave for the orchestrator._
