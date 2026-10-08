---
phase: 01-foundation
plan: "04"
subsystem: testing
tags: [gtest, ctest, cmake, ffi, crdt, gossipsub, soralog, protobuf]

# Dependency graph
requires:
  - phase: 01-foundation plan 01
    provides: moved test/test_gcs_global_db.cpp + gcs_storage target with Put/Get/topic accessors
  - phase: 01-foundation plan 02
    provides: gcs::CoreSession with injected-pubsub Initialize() test seam
  - phase: 01-foundation plan 03
    provides: four-function gcs_ffi ABI (gcs_init config-bytes / gcs_publish / gcs_subscribe / gcs_shutdown) + gcs_chat.proto GcsConfig/Codec
provides:
  - GCS-root test tree (test/CMakeLists.txt with GTest discovery + gcs_test macro)
  - Shared wait-condition template header (gcs::test::WaitForCondition, no raw thread sleeps)
  - CORE-05 substrate proof (test_gcs_core_smoke: lifecycle + gcs/chat topic join + Put→Get round-trip over injected port-0 GossipPubSub)
  - FFI ABI null-safety/argument-validation proof (test_gcs_ffi: 8 option-C tests incl. CODEC_JSON rejection and garbage-bytes rejection)
  - Moved GlobalDB lifecycle test registered against root gcs_storage (target test_gcs_storage)
affects: [01-05 Dart spike, 01-06 CI, 03 messaging]

# Tech tracking
tech-stack:
  added: []  # GTest already in thirdparty; no new libraries
  patterns:
    - "gcs_test macro: GTest::Main must precede ${libs} (Boost unit-test main() conflict)"
    - "Soralog YAML SetUpTestSuite is mandatory before GossipPubSub construction in every fixture"
    - "Option-C FFI testing: assert graceful nullptr from gcs_init without a live GeniusSDK node"

key-files:
  created:
    - test/CMakeLists.txt
    - test/test_wait_condition.hpp
    - test/test_gcs_core_smoke.cpp
    - test/test_gcs_ffi.cpp
  modified: []

key-decisions:
  - "GCS-root registration of the moved test uses target name test_gcs_storage — the submodule's test tree still registers test_gcs_global_db, and duplicate CMake target names are a configure error (CMP0002) when both test trees build"
  - "BUILD_TESTS (not BUILD_TESTING) gates the GCS-root test/ add_subdirectory hook; this build dir was reconfigured with -DBUILD_TESTS=ON (cache-only, untracked) — CI (Plan 06) must pass -DBUILD_TESTS=ON"
  - "test_gcs_ffi links gcs_ffi + gcs_proto so the test serializes a real gcs::chat::GcsConfig via proto/gcs_chat.pb.h (the generated header path under ${CMAKE_BINARY_DIR}/generated)"

patterns-established:
  - "gcs_test(name sources libs) registration macro for all GCS-root smoke tests"
  - "gcs::test::WaitForCondition shared header replaces per-file anonymous-namespace copies"

requirements-completed: [CORE-05]

# Metrics
duration: 5min
completed: 2026-08-26
---

# Phase 1 Plan 4: GCS Test Tree Summary

**GTest smoke suite proving the Phase 1 wiring: CoreSession lifecycle + gcs/chat topic join + CRDT Put→Get over an injected port-0 GossipPubSub, and 8 option-C FFI ABI tests (config-bytes gcs_init failure classes, publish/subscribe arg validation, null-safe shutdown) — ctest -R gcs 4/4 green**

## Performance

- **Duration:** 5 min (warm build tree; full configure+build+ctest cycles included)
- **Started:** 2026-08-26T23:29:48Z
- **Completed:** 2026-08-26T23:35:02Z
- **Tasks:** 3/3
- **Files modified:** 4 (all in test/)

## Accomplishments
- test/ tree stands up under the existing BUILD_TESTS hook: GTest discovery (verbatim NEO-SWARM block), gcs_test macro, three registrations — 4/4 green under `ctest -R gcs` (test_gcs_storage, test_gcs_core_smoke, test_gcs_ffi + the submodule's own test_gcs_global_db)
- CORE-05 substrate proven: CoreSession Initialize via injected GossipPubSub (port 0), AddBroadcastTopic+AddListenTopic on `gcs/chat/smoke-test`, Put→Get round-trip — 2/2 gtest cases, ~3.2s
- FFI ABI proven loadable/callable/null-safe under option C: 8/8 gtest cases — valid-PROTOBUF-config-without-node → graceful nullptr, null/zero-length/garbage config rejection, CODEC_JSON per-store-binding rejection, repeated init, shutdown-null no-op, publish/subscribe INVALID_ARGUMENT validation
- Zero `sleep_for`/`sleep_until` anywhere in test/ (plan verification grep returns no hits)

## Task Commits

Each task was committed atomically:

1. **Task 1: test/CMakeLists.txt + test/test_wait_condition.hpp (scaffolding + registrations)** - `16b89e0` (test)
2. **Task 2: test/test_gcs_core_smoke.cpp (CORE-05 substrate proof)** - `bf2a2a3` (test)
3. **Task 3: test/test_gcs_ffi.cpp (FFI config-bytes init / arg validation / shutdown)** - `3e3a62d` (test)

Plus one post-verification fix commit: `a0bf9b1` (fix — target rename + comment-only grep hits; see Deviations 2-4).

**Plan metadata:** this commit (docs: complete plan)

## Files Created/Modified
- `test/CMakeLists.txt` - GTest discovery + gcs_test macro + three test registrations (test_gcs_storage / test_gcs_core_smoke / test_gcs_ffi)
- `test/test_wait_condition.hpp` - gcs::test::WaitForCondition (kWaitTimeout 25s, kPollInterval 10ms), extracted verbatim from the moved test
- `test/test_gcs_core_smoke.cpp` - GcsCoreSmokeTest fixture (mandatory soralog SetUpTestSuite, salted temp dirs, MakeStartedPubSub) + lifecycle and CRDT round-trip tests
- `test/test_gcs_ffi.cpp` - GcsFFI fixture (conditional gcs_shutdown TearDown, MakeConfigBytes via gcs_chat.pb.h) + 8 option-C ABI tests

## Decisions Made
- Moved-test target name: `test_gcs_storage` (see Deviations — plan-mandated `test_gcs_global_db` collides with the submodule registration)
- Kept the NEO-SWARM macro's link order (GTest::Main before ${libs}) rather than PATTERNS' reversed order — the analog's Boost unit_test main() conflict comment is load-bearing and the plan says copy the analog verbatim
- Round-trip test waits on db-dir existence via WaitForCondition before Put/Get (satisfies the both-tests-use-WaitForCondition acceptance; mirrors the moved test's proven pattern)

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Plan-mandated target name `test_gcs_global_db` collides (CMP0002)**
- **Found during:** Task 3 runtime verification (first configure of the root test tree)
- **Issue:** Task 1 mandates `gcs_test(test_gcs_global_db ...)` but GNUS-NEO-SWARM/test/CMakeLists.txt:61 still registers its own `test_gcs_global_db` (the D-25 move copied the file; submodule removal is deferred to the neoswarm workstream). Any configure with both test trees active fails: "add_executable cannot create target ... because another target with the same name already exists"
- **Fix:** GCS-root registration renamed to `test_gcs_storage` — same source file (test/test_gcs_global_db.cpp), same gcs_storage/neoswarm_common/sgns::crdt_globaldb linkage, matches `ctest -R gcs`
- **Files modified:** test/CMakeLists.txt
- **Verification:** configure + build green; ctest 4/4 (`test_gcs_storage` Passed in 3.6s)
- **Committed in:** `a0bf9b1`

**2. [Rule 3 - Blocking] BUILD_TESTS gate was OFF — root test tree inert in this build dir**
- **Found during:** Task 3 runtime verification
- **Issue:** The plan's interfaces block says the add_subdirectory(test) hook fires "when BUILD_TESTING is on"; the actual gate in cmake/CommonBuildParameters.cmake:478 is `BUILD_TESTS` (default FALSE, OFF in this build dir's cache). Without flipping it, no GCS test target ever builds and every ctest verify returns "No tests were found"
- **Fix:** Reconfigured the existing build dir with `-DBUILD_TESTS=ON` (cache-only, untracked; no cmake/, build/, or src/ file changes). Note for Plan 06: CI must pass -DBUILD_TESTS=ON
- **Files modified:** none (build-dir cache only)
- **Verification:** configure green; test targets built and registered as ctest #21-23
- **Committed in:** n/a (no tracked change)

**3. [Rule 3 - Blocking] Task 2/3 verify commands share one build boundary**
- **Found during:** Task 2 verification
- **Issue:** Task 1 registers all three test targets, so configure fails on the missing test_gcs_ffi.cpp until Task 3's file exists — Task 2's build/ctest verify cannot pass at Task-2 commit time (plan sequencing defect, not a code failure)
- **Fix:** Static acceptance (sleep-grep=0, kSmokeTopic, WaitForCondition presence) ran at Task 2 commit; the shared runtime verification (build + ctest) executed immediately after Task 3's file landed — 2/2 smoke cases green, retroactively closing Task 2's runtime acceptance
- **Files modified:** none
- **Verification:** `./build/OSX/Debug/gcs_test/test_gcs_core_smoke` → `[ PASSED ] 2 tests`
- **Committed in:** n/a (sequencing only)

**4. [Rule 1 - Bug] Plan verification greps caught comment-only hits in my own files**
- **Found during:** Task 1/2 verification and plan-level verification
- **Issue:** (a) Explanatory comments in test/CMakeLists.txt contained the literal strings the acceptance greps forbid (neoswarm_storage/neoswarm_test/SUPERGENIUS_TEST_DATA_DIR); (b) test_wait_condition.hpp's Doxygen (copied from PATTERNS' "complete file") mentions the raw-sleep API by name, tripping the plan's zero-hit `grep -r sleep_for test/`; (c) `gcs/chat/*` in the smoke header raised -Wcomment
- **Fix:** Comment rewording only — zero logic changes (WaitForCondition body remains verbatim)
- **Files modified:** test/CMakeLists.txt, test/test_wait_condition.hpp, test/test_gcs_core_smoke.cpp
- **Verification:** all plan greps return zero hits; rebuild warning-free
- **Committed in:** `16b89e0` (partly) and `a0bf9b1`

---

**Total deviations:** 4 auto-fixed (3 blocking, 1 bug-class comment fix)
**Impact on plan:** All fixes required to make the plan's own verification commands pass; no scope creep, no src/ changes. Two carry-forward notes: (1) CI (Plan 06) must configure with -DBUILD_TESTS=ON; (2) the test_gcs_global_db name collision resolves permanently when the neoswarm workstream removes the submodule-side duplicate (D-25 pointer-update note).

## Issues Encountered
None beyond the deviations above. No product bugs in src/ surfaced — every gcs_ffi/gcs_core behavior under test matched the plan's option-C expectations (including the GcsGlobalDb error-log line on the nullptr-node path, which is the designed failure logging, not a test failure).

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Plan 05 (Dart spike): the ABI is proven loadable and callable; the live-node init-success path, gcs_subscribe port registration, and Dart_PostCObject wiring remain Plan 05's to prove (as planned)
- Plan 06 (CI): must add `-DBUILD_TESTS=ON` to the cmake configure step or the GCS tests will not build; ctest -R gcs is the verification entry point (Linux may want the dbus/gnome-keyring wrapper per the SuperGenius precedent — harmless here)
- Plan 01-09 (wave 4 sibling): test tree shares the build dir; both plans' targets coexist cleanly
- Test binaries land in `build/OSX/Debug/gcs_test/` (the plan's verification bullet said `gcs_src/test/` — the hook's binary dir is `gcs_test` per cmake/CommonBuildParameters.cmake:482)

## Self-Check: PASSED

All 4 created files exist on disk; all 4 task/fix commits (16b89e0, bf2a2a3, 3e3a62d, a0bf9b1) present in git log; `ctest -R gcs` re-run green (4/4).

---
*Phase: 01-foundation*
*Completed: 2026-08-26*
