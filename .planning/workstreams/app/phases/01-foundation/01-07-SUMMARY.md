---
phase: 01-foundation
plan: 07
subsystem: app
tags: [cmake, flutter, build-wiring, pubspec]
requires:
  - "01-01: GcsGlobalDb relocation (branch base)"
provides:
  - "src/app/CMakeLists.txt gate + cache vars + scaffold include + pub/analyze/test targets (D-16)"
  - "pubspec dependency set for scaffold + flutter_bloc + ffigen + protobuf + protoc_plugin"
affects:
  - "01-03: will add the add_subdirectory(app) line in src/CMakeLists.txt"
  - "01-08: will add DEPENDS app_generate_chat_components to app_analyze/app_test"
  - "01-11: will rewrite main.dart/widget_test.dart (closes the analyze-broken window)"
tech-stack:
  added: []
  patterns:
    - "FRONTEND_BUILD_ENABLED option gate, default OFF (D-08)"
    - "Cache vars set BEFORE add_subdirectory(scaffold) (Pitfall U1)"
key-files:
  created:
    - src/app/CMakeLists.txt
  modified:
    - src/app/pubspec.yaml
decisions:
  - "Dropped flutter_chat_ui/flutter_chat_core; kept neoswarm_ffi (D-03) and frontend_scaffold"
  - "app_analyze/app_test depend only on app_pub_get; codegen edge deferred to 01-08"
metrics:
  duration: "~15 min"
  completed: 2026-08-23
---

# Phase 01 Plan 07: App Build Wiring (D-16) Summary

Gated src/app CMake wiring (FRONTEND_BUILD_ENABLED default OFF, cache vars before scaffold include,
pub/analyze/test targets) plus pubspec rewrite to the scaffold + flutter_bloc + ffigen + protobuf +
protoc_plugin stack.

## Tasks Completed

| Task | Name | Commit | Key Files |
| ---- | ---- | ------ | --------- |
| 1 | Rewrite src/app/pubspec.yaml | 3e46004 | src/app/pubspec.yaml |
| 2 | Create src/app/CMakeLists.txt (gate + cache vars + scaffold) | 68323fa | src/app/CMakeLists.txt |

## Verification Results

- Plan grep verifications: all pass (flutter_bloc ^9.0.0, ffigen ^11.0.0, protobuf, protoc_plugin
  present; chat kit absent; neoswarm_ffi + frontend_scaffold kept; gate option + 3 cache vars +
  add_subdirectory(scaffold); no frontend_all/frontend_generate_api strings).
- Full build + ctest with default gate (OFF): configure green, build green, 20/20 tests pass.
- Configure with -DFRONTEND_BUILD_ENABLED=ON in the main build: green, 20/20 tests pass. Zero app
  targets appear because src/CMakeLists.txt does not yet include app (that line is owned by 01-03) —
  expected.
- Gate wiring validated with a throwaway /tmp parent project that add_subdirectory's src/app:
  - ON: app_pub_get / app_analyze / app_test targets present.
  - OFF (default): zero app targets.

## Deviations from Plan

### Deferred / Overrides

**1. [Orchestrator override] `flutter pub get` not run**
- Plan Task 1 asked to run `flutter pub get`; the orchestrator explicitly prohibited network-touching
  flutter/dart commands and lockfile writes for this plan. Deps are pubspec edits only; resolution
  will be exercised when the app targets first run (01-03+ / dev loop).

**2. [Accepted per plan] Broken analyze/test window (Pitfall U6)**
- main.dart still imports the dropped chat kit and widget_test.dart still references MyApp;
  app_analyze/app_test fail until 01-11. C++-only CI (gate OFF) never runs them. Documented, accepted.

**3. [Rule 3 - blocking fix] Comment wording adjusted**
- The plan's automated verify greps for absence of the strings `frontend_all`/`frontend_generate_api`;
  an explanatory comment mentioned them by name and would have failed the literal check. Reworded the
  comment to describe the OpenAPI aggregate targets without naming them.

## Known Stubs

None — build wiring only.

## Self-Check: PASSED

Commits 3e46004, 68323fa present; src/app/CMakeLists.txt, src/app/pubspec.yaml, and this summary exist.

## TDD Gate Compliance

Not a TDD plan; no test commits required.
