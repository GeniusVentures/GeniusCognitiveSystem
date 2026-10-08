---
phase: 2
slug: spaces-rooms
status: ready
nyquist_compliant: true
wave_0_complete: false
created: 2026-09-19
---

# Phase 2 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | C++: Google Test via `gcs_test` macro (test/CMakeLists.txt). Dart: `flutter_test` (`flutter test`) with plain fakes — no bloc_test package (verified dev_dependencies) |
| **Config file** | test/CMakeLists.txt (C++); src/app/CMakeLists.txt `app_test` / `app_analyze` targets (Dart) |
| **Quick run command** | `ninja -C build/OSX/Debug && ctest --test-dir build/OSX/Debug -R "gcs" --output-on-failure` |
| **Full suite command** | `ctest --test-dir build/OSX/Debug --output-on-failure` + `cd src/app && flutter test` (via `ninja -C build/OSX/Debug app_test`) |
| **Estimated runtime** | ~90 seconds (OSX Debug incremental build + `ctest -R gcs`); full parent-build suite longer |

---

## Sampling Rate

- **After every task commit:** Run `ninja -C build/OSX/Debug && ctest --test-dir build/OSX/Debug -R "gcs" --output-on-failure` and/or targeted `flutter test <file>`
- **After every plan wave:** Run full ctest + `ninja -C build/OSX/Debug app_analyze app_test`
- **Before `/gsd:verify-work`:** Full suite must be green
- **Max feedback latency:** ~90 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 02-01-01 | 01 | 1 | CORE-01/02/03 | T-02-SC (no packages) | Append-only proto extension; existing oneof tags untouched (D-24/D-26); tombstone fields present from creation | build / grep | `ninja -C build/OSX/Debug gcs_proto` | ✅ | ⬜ pending |
| 02-01-02 | 01 | 1 | CORE-01/02/03 | T-02-01 (corrupt record parse) / T-02-02 (id minting) / T-02-03 (manifest N+1, accept) | LoadFromStore skips hostile/corrupt records (never aborts startup); seed+counter id minting; UpdateSpace preserves created_at_ms/deleted; derived-join pure recomputation | build | `ninja -C build/OSX/Debug gcs_core` | ❌ W0 | ⬜ pending |
| 02-01-03 | 01 | 1 | CORE-01/02/03 | T-02-01 / T-02-02 | Create space/room, parent validation, derived-join toggle (true→false→true), tombstone skip, restart persistence proven; id salt-separator shape asserted; zero sleep_for | unit (gtest) | `ninja -C build/OSX/Debug test_gcs_entities && ctest --test-dir build/OSX/Debug -R test_gcs_entities --output-on-failure` | ❌ W0 | ⬜ pending |
| 02-02-01 | 02 | 2 | CORE-01/02/03 | T-02-04 (command parse) / T-02-05 (arg validation) / T-02-06 (size narrowing) / T-02-07 (error strings, accept) | create_space/create_room/update_space validate empty name / unknown parent / empty space_id before any Put; payloadLength narrowing guard inherited; SpaceTree pushed subscribe-first | build | `ninja -C build/OSX/Debug gcs_ffi` | ✅ | ⬜ pending |
| 02-02-02 | 02 | 2 | CORE-01/02/03 | T-02-04 / T-02-05 | Create + derived-join + restart persistence proven through the real ABI (fake Dart port capture); two-session cycles with shutdown between | unit / integration (gtest) | `ninja -C build/OSX/Debug test_gcs_ffi_sdk && ctest --test-dir build/OSX/Debug -R test_gcs_ffi_sdk --output-on-failure` | ✅ | ⬜ pending |
| 02-03-01 | 03 | 2 | CORE-01/02/03 | T-02-SC (pinned plugin) | Dart pb regenerated with PINNED protoc_plugin 22.5.0 (no `BuilderInfo.aE`); no new packages | codegen + analyze | `cd src/app && flutter analyze lib/generated/proto` | ✅ | ⬜ pending |
| 02-03-02 | 03 | 2 | CORE-01/02/03 | — | RailState tree data-only mapping (parent grouping + standalone split + treeReceived); rooms/activeRoom joined view untouched | analyze | `cd src/app && flutter analyze lib/cubits/rail_cubit.dart` | ✅ | ⬜ pending |
| 02-03-03 | 03 | 2 | CORE-01/02/03 | T-02-08 (event decode) | handlePushedBytes SpaceTree arm reached only after successful decode (existing try/catch guard); grouping + treeReceived proven | flutter test (unit) | `cd src/app && flutter test test/cubits/shell_cubits_test.dart` | ✅ | ⬜ pending |
| 02-04-01 | 04 | 3 | CORE-01/02/03 | T-02-09 (name input) / T-02-10 (is_public metadata, accept) | Empty-name inline validation (no publish); publish-false error toast (no local optimism); maxLength 64 + C++ re-validation in depth | analyze | `cd src/app && flutter analyze lib/shell/space_room_dialog.dart` | ❌ W0 | ⬜ pending |
| 02-04-02 | 04 | 3 | CORE-01/02/03 | T-02-09 | Create space/room, edit space, empty-name, publish-false paths proven against fake transport | flutter test (widget) | `cd src/app && flutter test test/shell/space_room_dialog_test.dart` | ❌ W0 | ⬜ pending |
| 02-05-01 | 05 | 4 | CORE-01/02/03 | T-02-11 (unjoined row selection) | Catalog-but-not-joined rows disabled (ScaffoldPressable disabled + Semantics enabled false); private badge; loading skeleton; affordances wired | analyze | `cd src/app && flutter analyze lib/shell/room_rail.dart` | ✅ | ⬜ pending |
| 02-05-02 | 05 | 4 | CORE-01/02/03 | T-02-11 | Loading skeleton, empty CTA, private badge, nested + standalone rendering, disabled-row → tappable transition proven (criterion-3 dim/un-dim observable) | flutter test (widget) | `cd src/app && flutter test test/chat_shell_test.dart` | ✅ | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `test/test_gcs_entities.cpp` — EntityStore unit tests (CORE-01/02/03 + tombstone skip + restart); register via `gcs_test(test_gcs_entities test_gcs_entities.cpp "gcs_core;gcs_storage;neoswarm_common")` (02-01 Task 3)
- [ ] FFI-level create/update two-session coverage — extend `test/test_gcs_ffi_sdk.cpp` (preferred; its fixture already owns the node + fake Dart port) (02-02 Task 2)
- [ ] `src/app/test/shell/space_room_dialog_test.dart` — dialog widget test (fake GcsCommandTransport recorder, per shell_cubits_test fake pattern) (02-04 Task 2)
- [ ] Dart pb regeneration early in the wave order (02-03 Task 1) — all Dart work depends on the new oneof arms

`wave_0_complete: false` until all land.

---

## Manual-Only Verifications

All phase behaviors have automated verification.

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references (Wave 0 deliverables listed above; land across 02-01 Task 3, 02-02 Task 2, 02-04 Task 2)
- [x] No watch-mode flags
- [x] Feedback latency < 90s
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending — Wave 0 test files land across 02-01 Task 3, 02-02 Task 2, 02-04 Task 2; sign-off after first green `ctest -R gcs` on OSX Debug + `flutter test`.
