---
phase: 01-foundation
plan: 01
subsystem: storage
tags: [crdt, globaldb, cmake, relocation, d-25]
dependency-graph:
  requires: [GNUS-NEO-SWARM submodule (neoswarm_common), SuperGenius (sgns::crdt_globaldb), GeniusSDK (sgns::GeniusSDK_shared)]
  provides: [gcs_storage target, GcsGlobalDb with data-plane pass-throughs]
  affects: [01-02 (gcs_core links gcs_storage), 01-04 (test registration)]
tech-stack:
  added: []
  patterns: [hard-required CMake linkage (FATAL_ERROR), build-tree dylib redirect, wait-condition tests]
key-files:
  created:
    - src/lib/gcs_storage/gcs_global_db.hpp
    - src/lib/gcs_storage/gcs_global_db.cpp
    - src/lib/gcs_storage/CMakeLists.txt
    - test/test_gcs_global_db.cpp
    - test/CMakeLists.txt
  modified:
    - src/CMakeLists.txt
decisions: []
metrics:
  duration: "<1 hour"
  completed: "2026-08-23T04:28:33Z"
---

# Phase 01 Plan 01: GcsGlobalDb Move + Pass-Through Accessors Summary

**One-liner:** Relocated GcsGlobalDb from the GNUS-NEO-SWARM submodule into the GCS root repo at `src/lib/gcs_storage/` behind a new `gcs_storage` CMake target and extended the class with four data-plane pass-throughs (`AddBroadcastTopic`, `AddListenTopic`, `Put`, `Get`) guarded by `m_running`.

## What Was Built

- **Move (D-25):** `GNUS-NEO-SWARM/src/storage/gcs_global_db.{hpp,cpp}` copied verbatim into `src/lib/gcs_storage/`. Header guard renamed `NEOSWARM_STORAGE_GCS_GLOBAL_DB_HPP` → `GCS_STORAGE_GCS_GLOBAL_DB_HPP`; self-include `"storage/gcs_global_db.hpp"` → `"gcs_storage/gcs_global_db.hpp"`. Namespace `sgns::neoswarm::storage` intentionally unchanged (minimal diff; namespace migration deferred).
- **Accessors:** Four public methods added to `GcsGlobalDb` immediately after `IsRunning()`, each guarded by `if ( !m_running.load() ) return outcome::failure( Error::GcsDbError );` and forwarding to the matching `m_db->` call:
  - `AddBroadcastTopic(const std::string&)` → `m_db->AddBroadcastTopic`
  - `AddListenTopic(const std::string&)` → `m_db->AddListenTopic`
  - `Put(const std::string& key, const std::string& value)` → converts via `crdt::HierarchicalKey{key}` + `Buffer::put(value)` → `m_db->Put`
  - `Get(const std::string& key)` → `m_db->Get(HierarchicalKey{key})`, converts result via `Buffer::toString()`
  - All underlying `outcome::failure` results propagate as `Error::GcsDbError` (D-14 mapping).
- **CMake:** New `src/lib/gcs_storage/CMakeLists.txt` defines `add_library(gcs_storage STATIC gcs_global_db.cpp)`; links `neoswarm_common`, hard-requires `sgns::crdt_globaldb` and `sgns::GeniusSDK_shared` (with the build-tree dylib redirect copied verbatim from `neoswarm_storage`), and re-declares the thirdparty/SuperGenius/GeniusSDK/wallet-core/zkLLVM/evmrelay/MNN include blocks. `neoswarm_proto` is intentionally NOT linked (the moved sources have no proto includes). `src/CMakeLists.txt` appends `add_subdirectory(lib/gcs_storage)` as the last line; the `gcs_core` block is untouched.
- **Test staging:** `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp` copied to `test/test_gcs_global_db.cpp` with only the include path updated. Stub `test/CMakeLists.txt` added (`enable_testing()`) so the parent's `add_subdirectory(test)` hook never fails configure before 01-04 replaces it with the real registration.

## Tasks Executed

| Task | Name | Commit | Files |
| ---- | ---- | ------ | ----- |
| 1 | Move GcsGlobalDb class + fix guard/include | 40f2368 | src/lib/gcs_storage/gcs_global_db.{hpp,cpp} |
| 2 | Add four data-plane pass-through accessors | e444705 | src/lib/gcs_storage/gcs_global_db.{hpp,cpp} |
| 3 | Create gcs_storage CMake target + wire src/CMakeLists | 31d4596 | src/lib/gcs_storage/CMakeLists.txt, src/CMakeLists.txt |
| 4 | Move test into root test/ tree | a144e36 | test/test_gcs_global_db.cpp, test/CMakeLists.txt |

## Deviations from Plan

**1. [Rule 3 - Verification pattern] Task 4 grep negation was ambiguous**

- **Found during:** Task 4 verification
- **Issue:** The plan's automated verification used `! grep -q 'storage/gcs_global_db.hpp' test/test_gcs_global_db.cpp`. The replacement string `gcs_storage/gcs_global_db.hpp` contains `storage/gcs_global_db.hpp` as a substring, so the negation would always fail even after the correct edit.
- **Fix:** Used anchored regex `! grep -qE '#include "storage/'` to verify the old include is gone, which matches the plan's intent ("the include now references `gcs_storage/gcs_global_db.hpp`; old `storage/` path absent").
- **Files modified:** None (verification only).
- **Commit:** a144e36

**2. [Plan gap] Task 2 declared tdd="true" but no RED test step exists in this plan**

- **Found during:** Task 2 planning
- **Issue:** The plan front-matter has `type: execute` (not `tdd`), and Task 2's `<action>` block describes declarations + implementations only — no failing-test-first step. Test registration and execution is deferred to plan 01-04 per the plan's own verification block ("Full build is validated in 01-04 (smoke tests); this plan does not itself run a configure"). Adding a RED test now would require the test infrastructure 01-04 owns.
- **Fix:** Executed Task 2 as a structural add per the action block. The moved test file already exercises lifecycle behavior; the four new accessors will be exercised by 01-04's smoke test (which calls Put/Get/AddBroadcastTopic/AddListenTopic on the moved component).
- **TDD gate compliance:** No RED `test(...)` commit exists in this plan. Flagged here per the plan-level TDD gate rule — the plan is `type: execute`, so the gate is not strictly required, but noted because Task 2 carried the `tdd="true"` attribute.

## Auth Gates

None.

## Threat Flags

None — no new network endpoints, auth paths, file access patterns, or schema changes beyond what the plan's `<threat_model>` already covers (T-01-01/02/03 accepted/mitigated, T-01-SC accepted).

## Known Stubs

- `test/CMakeLists.txt` is an intentional stub (`enable_testing()` only) per plan Task 4 — 01-04 overwrites it with the real test registration. Not a blocker for this plan's goal.

## Verification Performed

- Task 1 grep gate: header guard renamed, self-include updated, namespace preserved — PASS
- Task 2 grep gate: exactly 4 new member-function definitions, ≥4 `m_running.load()` guards (7 total) — PASS
- Task 3 grep gate: `add_library(gcs_storage STATIC` + `neoswarm_common` + `sgns::crdt_globaldb` + `add_subdirectory(lib/gcs_storage)` — PASS
- Task 4 file+include checks (with anchored regex per Deviation 1) — PASS

## Out of Scope (deliberately not done)

- Full CMake configure / build — per plan's `<verification>` block, deferred to 01-04.
- Removal of `GNUS-NEO-SWARM/src/storage/gcs_global_db.{hpp,cpp}` — owned by neoswarm Phase 03-02/03-03 with the pointer-update note.
- `gcs_core` linkage to `gcs_storage` — owned by plan 01-02.
- Test registration in the build — owned by plan 01-04.

## Self-Check: PASSED

- [x] `src/lib/gcs_storage/gcs_global_db.hpp` — FOUND
- [x] `src/lib/gcs_storage/gcs_global_db.cpp` — FOUND
- [x] `src/lib/gcs_storage/CMakeLists.txt` — FOUND
- [x] `test/test_gcs_global_db.cpp` — FOUND
- [x] `test/CMakeLists.txt` — FOUND
- [x] `src/CMakeLists.txt` modified with `add_subdirectory(lib/gcs_storage)` — FOUND
- [x] Commits 40f2368, e444705, 31d4596, a144e36 — FOUND in `git log`
