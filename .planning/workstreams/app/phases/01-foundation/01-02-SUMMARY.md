---
phase: 01-foundation
plan: 02
subsystem: app
tags: [gcs_core, session, cmake]
requires: ["01-01 (GcsGlobalDb moved to src/lib/gcs_storage)"]
provides: ["gcs::CoreSession interface for gcs_ffi (01-03) and smoke tests (01-04)"]
affects: [src/CMakeLists.txt, src/lib/gcs_core.hpp, src/lib/gcs_core.cpp]
tech-stack:
  added: []
  patterns: [hard-required CMake link idiom, deferred-registration lifecycle, injected-pubsub test seam]
key-files:
  created: [src/lib/gcs_core.hpp]
  modified: [src/CMakeLists.txt, src/lib/gcs_core.cpp]
decisions:
  - gcs_core links gcs_storage PUBLIC only; no neoswarm_common (gcs_storage is self-contained)
  - added `namespace outcome = libp2p::outcome;` alias inside namespace gcs (GcsGlobalDb's alias is scoped to its own namespace)
metrics:
  duration: ~15 min
  completed: 2026-08-23
---

# Phase 01 Plan 02: gcs_core CoreSession Summary

gcs::CoreSession class owning sgns::neoswarm::storage::GcsGlobalDb via unique_ptr, with lifecycle (Initialize/Shutdown/IsRunning) and four CRDT pass-throughs, wired into the build via a hard-required gcs_storage PUBLIC link.

## What Was Built

- **Task 1 (0ea9cc4):** src/CMakeLists.txt — `if(TARGET gcs_storage)` hard-required link idiom, `target_link_libraries(gcs_core PUBLIC gcs_storage)`. No neoswarm_storage anywhere.
- **Task 2 (4e2038e):** src/lib/gcs_core.hpp — `gcs::CoreSession` with Config, both Initialize overloads (incl. pubsub test seam), void noexcept Shutdown(), IsRunning, AddBroadcastTopic/AddListenTopic/Put/Get. Copy/move deleted (T-01-02-03). Full Doxygen.
- **Task 3 (02163d8):** src/lib/gcs_core.cpp — placeholder replaced with thin delegation to m_db; deferred init in ctor, defensive shutdown in dtor.

## Verification

- Full build green: `cmake .. -G Ninja && ninja` in build/OSX/Debug, run before every commit.
- Full test suite: **20/20 passed, 0 failed** (ran before each of the 3 commits).
- `nm gcs_src/libgcs_core.a | grep -c CoreSession` → 16 symbols.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] add_subdirectory ordering in src/CMakeLists.txt**
- **Found during:** Task 1
- **Issue:** The plan said to place the `if(TARGET gcs_storage)` block after the spdlog/fmt link and to NOT touch the `add_subdirectory(lib/gcs_storage)` line — but that line came after the gcs_core target, so gcs_storage did not exist at configure time and the FATAL_ERROR branch fired.
- **Fix:** Moved `add_subdirectory(lib/gcs_storage)` above the hard-required link block. Same lines, reordered; content unchanged.
- **Commit:** 0ea9cc4

**2. [Rule 3 - Blocking] outcome namespace alias in gcs_core.hpp**
- **Issue:** Plan's header sketch used `outcome::result` inside `namespace gcs`, but GcsGlobalDb's `namespace outcome = libp2p::outcome;` alias is scoped to `sgns::neoswarm::storage`.
- **Fix:** Added the same alias inside `namespace gcs`. Error domain remains sgns::gcs::Error via gcs_storage's vendored common/ (no second copy, no NEO-SWARM headers).

### Stale plan truths (pre-flagged by orchestrator)

- Must-have truth claiming gcs_storage transitively brings neoswarm_common is stale — gcs_storage is self-contained. gcs_core → gcs_storage PUBLIC alone is sufficient; neoswarm_common was NOT added.

## TDD Gate Compliance

Plan tasks 2/3 carried tdd="true" but no tests exist yet — smoke tests are Plan 01-04 per the plan's own sequencing ("this plan only builds gcs_core"). Verified by full build + symbol check instead. No test(...) commit exists; flagged here for the verifier.

## Self-Check: PASSED

- src/lib/gcs_core.hpp, src/lib/gcs_core.cpp exist; src/CMakeLists.txt contains gcs_storage block.
- Commits 0ea9cc4, 4e2038e, 02163d8 present on feature/app-restructure.
- Build + 20/20 tests green at each commit.
