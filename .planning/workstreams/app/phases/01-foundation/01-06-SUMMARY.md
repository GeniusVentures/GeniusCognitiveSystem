---
phase: 01-foundation
plan: 06
subsystem: infra
tags: [ci, github-actions, cmake, self-hosted-runners, release-downloads]

# Dependency graph
requires:
  - phase: 01-foundation/01-03
    provides: gcs_core FFI data plane built by CI
  - phase: 01-foundation/01-04
    provides: gcs_storage target in root src/lib
provides:
  - .github/workflows/cmake.yml — 15-cell CI matrix (5 platforms × Debug/Release) on self-hosted runners
  - All-static GeniusSDK linkage across the GCS + GNUS-NEO-SWARM build (shared-lib workaround machinery removed)
affects: [01-11, PR #12, every future GCS push]

# Tech tracking
tech-stack:
  added: [github-actions resolve-runners job pattern from GeniusSDK template]
  patterns:
    - "CI consumes prebuilt thirdparty/SuperGenius/GeniusSDK release tarballs (tag <Target>[-<ABI>]-develop-<BuildType>) — never builds deps from source"
    - "Static-first GeniusSDK: sgns::GeniusSDK before sgns::GeniusSDK_shared in every consumer CMakeLists"

key-files:
  created: [.github/workflows/cmake.yml]
  modified:
    - cmake/CommonBuildParameters.cmake
    - GNUS-NEO-SWARM/src/core/CMakeLists.txt
    - GNUS-NEO-SWARM/src/storage/CMakeLists.txt
    - src/lib/gcs_storage/CMakeLists.txt

key-decisions:
  - "All-static GeniusSDK linkage (user directive; cherry-pick c32649b3) — CI-6 Linux shared-lib redirect deleted, GCS_SDK_SINGLE_PROVIDER no longer set"
  - "No direct libed25519.a link in neoswarm_core — the static GeniusSDK chain embeds the same orlp ed25519/sha3 code (wallet-core TrezorCrypto + sgns keccak)"
  - "Vulkan_INCLUDE_DIR seeded from Vulkan-Headers/include when the loader dir ships no include/ (split thirdparty layout, Windows configure fix)"
  - "SGProcessingManager install-tree fallback widened — dev machines with sibling checkouts now link real mode like CI (closed the local-stub verification gap)"
  - "Windows cells accepted red: GeniusSDK Windows assets stale (2026-08-27, bool-vs-WitnessVerdict signature) + zkLLVM has no Debug artifacts (Release LLVM → LNK2038 in MSVC Debug). Owned by another engineer (Windows-zkLLVM linking); may become moot if zkLLVM dependency is dropped"

patterns-established:
  - "Local verification must build ALL targets (incl. neo-swarm exe) — stub-mode divergence let 15/15 red slip through once"

requirements-completed: [CORE-05]

# Metrics
duration: multi-session (Task 1: 2026-08-30; Task 2: 2026-09-14..16)
completed: 2026-09-16
---

# Plan 01-06: CI Workflow Summary

**15-cell CI matrix live on PR #12 — 13/15 green (Linux ×4, OSX ×2, iOS ×2, Android ×4); 2 Windows cells red and accepted (separate engineer owns Windows-zkLLVM linking).**

## Performance

- **Started:** 2026-08-30 (Task 1), 2026-09-14 (Task 2 observation loop)
- **Completed:** 2026-09-16
- **Tasks:** 2 (1 auto, 1 human-action gate)
- **Files modified:** 5 (+1 workflow created)

## Accomplishments

- CI workflow authored from the GeniusSDK template with the four planned deltas (GeniusSDK download step, no zkLLVM step, workspace root, SuperGenius ctest steps)
- CI driven from 11/15 → 0/15 (all-static transition) → **13/15** across three observed runs
- All-static GeniusSDK linkage landed end-to-end: cherry-pick into GNUS-NEO-SWARM, parent gcs_storage flip, CI-6 redirect machinery deleted
- Local-vs-CI linkage divergence eliminated (SGProcessingManager stub-mode gap)

## Task Commits

1. **Task 1: Author .github/workflows/cmake.yml** — `90233f9` (feat)
2. **Task 2: Push + observe CI** — parent `b282ede`, `7dd1f9c`, `65e4aa0`; submodule `4019000` (pick), `120f6b7`, `d388fbe` (fix)

**Plan metadata:** this summary.

## Files Created/Modified

- `.github/workflows/cmake.yml` — the CI workflow (Task 1)
- `cmake/CommonBuildParameters.cmake` — Vulkan moltenvk/split-layout seeding; CI-6 redirect block deleted
- `GNUS-NEO-SWARM/src/{core,storage}/CMakeLists.txt` — static-first GeniusSDK, ed25519 drop, install-tree fallback widening
- `src/lib/gcs_storage/CMakeLists.txt` — static-first GeniusSDK

## Decisions Made

- **All-static GeniusSDK** (user): the shared dylib existed only to dodge link issues; the thin static wrapper + sgns archives is the intended consumption model.
- **Windows-red acceptance** (user, 2026-09-16): another engineer is working Windows↔zkLLVM linking and the zkLLVM dependency may be dropped entirely; blocking PR #12 on 15/15 would couple it to that work.

## Deviations from Plan

None in plan structure. The observe loop ran longer than one run because the dependency stack was rebuilt mid-flight (user rebuilt SuperGenius/thirdparty/GeniusSDK 2026-09-16), changing the linkage ground truth under the workflow.

## Issues Encountered

1. **run 33461331605 (e0802b4): 11/15** — Linux Release ×2 GCS_SDK_SHARED_LIB-NOTFOUND; Windows Release ctest 10/24; Linux aarch64 Debug boot flake. → Resolved by dependency re-releases + all-static transition.
2. **run 35156413723 (b282ede): 0/15** — (a) duplicate ed25519_*/sha3_* symbols at the neo-swarm exe link (thirdparty libed25519.a vs wallet-core TrezorCrypto + sgns keccak) on all platforms; (b) Windows configure missing Vulkan_INCLUDE_DIR (thirdparty split headers/loader layout). → Fixed in `7dd1f9c` + submodule `d388fbe`.
3. **run 35157684003 (7dd1f9c): 13/15** — Windows Debug LNK2038 (zkLLVM Release-only LLVM mixed into MSVC Debug link) and Windows Release LNK2001 (GeniusSDK Windows asset from 2026-08-27 references pre-`WitnessVerdict` `bool ValidateWitness`; current SuperGenius defines the `WitnessVerdict` overload — verified via llvm-nm on the release tarball). → **Accepted red** per user decision; owned by the Windows-zkLLVM engineer.
4. **test_gcs_global_db_sdk** SEGFAULTs under `ctest -j4` (boot contention), passes serially — documented upstream flake, not plan-blocking.

## User Setup Required

Existing (verified by green resolve-runners cells): GNUS_TOKEN_1 secret + self-hosted runners (sg-ubuntu-linux, sg-arm-linux, SG-WIN11, gv-OSX-Large).

## Next Phase Readiness

- Only 01-11 (wave 6: GCSChat shell + cubits) remains in phase 01
- PR #12 (draft) carries all phase-01 work; merge decision open — Windows cells do not block per user decision
- Re-arm note: if GeniusSDK Windows assets are rebuilt, the Windows-Release LNK2001 should clear without any GCS change; Windows-Debug additionally needs the zkLLVM Debug/MSVC story from the Windows engineer

---
*Phase: 01-foundation*
*Completed: 2026-09-16*
