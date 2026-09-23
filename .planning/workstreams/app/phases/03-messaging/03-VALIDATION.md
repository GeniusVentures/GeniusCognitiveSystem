---
phase: 3
slug: messaging
status: draft
nyquist_compliant: true
wave_0_complete: true
created: 2026-09-23
---

# Phase 3 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.
> Propagated from RESEARCH.md "Validation Architecture" (2026-09-23).

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | GTest (C++, `test/` tree) + flutter_test (Dart, `src/app/test/`) |
| **Config file** | `test/CMakeLists.txt` (`gcs_test` macro + ctest registration); Dart has none (flutter_test default) |
| **Quick run command** | C++: `cd build/OSX/Debug && ctest -R test_gcs_messaging --output-on-failure` |
| **Full suite command** | C++: `cd build/OSX/Debug && ctest --output-on-failure`; Dart: `cd src/app && flutter test` |
| **Estimated runtime** | ~120 seconds (C++ messaging + storage targets; full ctest longer) |

---

## Sampling Rate

- **After every task commit:** `cd build/OSX/Debug && ninja gcs_core gcs_ffi && ctest -R test_gcs_messaging --output-on-failure` (C++), `cd src/app && flutter test test/cubits/` (Dart)
- **After every plan wave:** full `ctest` + `flutter test`
- **Before `/gsd:verify-work`:** Full C++ + Dart suites green, plus `dart analyze --fatal-infos` clean
- **Max feedback latency:** 120 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 03-01-01 | 01 | 1 | CORE-04 | T-03-01 / T-03-02 | Pin SuperGenius/GeniusSDK signatures before any implementation (no guessed contracts) | manual (grep record) | `bash -c 'grep -c "PutLocal" .../03-API-SIGNATURES.md ...'` | ✅ created here | ⬜ pending |
| 03-01-02 | 01 | 1 | CORE-04 | T-03-01 | Append-only proto evolution (existing tags unchanged) | static (grep) | `bash -c 'grep -c "string sender = 7" src/proto/gcs_chat.proto ...'` | ✅ | ⬜ pending |
| 03-02-01 | 02 | 2 | CORE-04 (D-01/D-02/D-03) | T-03-03 / T-03-04 | Topics-aware Put, PutLocal, prefix scan, raw full-value Publish/Subscribe, receive hook — raw bytes only, no unbounded state | build + unit | `bash -c 'cd build/OSX/Debug && ninja gcs_core'` | ✅ | ⬜ pending |
| 03-02-02 | 02 | 2 | CORE-04 (D-02/D-03) | T-03-03 | Prefix isolation; round-trip values; PutLocal visible in scan; Publish/Subscribe register + return success | unit (C++) | `bash -c 'cd build/OSX/Debug && ninja test_gcs_storage && ctest -R test_gcs_storage --output-on-failure'` | ❌ W0 | ⬜ pending |
| 03-03-01 | 03 | 2 | CORE-04 (D-06/D-07/D-04) | T-03-05 / T-03-06 | Upsert-by-id, replaceAll, sender truncation, capped list | analyze | `bash -c 'cd src/app && dart analyze --fatal-infos'` | ✅ | ⬜ pending |
| 03-03-02 | 03 | 2 | CORE-04 (D-06/D-07) | T-03-06 | Upsert/replaceAll/role-preservation/sender-label/composer-refusal | unit (Dart) | `bash -c 'cd src/app && flutter test test/cubits/shell_cubits_test.dart'` | ❌ extend | ⬜ pending |
| 03-04-01 | 04 | 3 | CORE-04 | T-03-07..T-03-11 | Send lifecycle (live full-value publish + archive), two-route dedupe (live + CRDT), sender stamp, peer role flip, history sort, persistence, history role flip (RED) | unit (C++) | `bash -c 'cd build/OSX/Debug && ninja test_gcs_messaging 2>&1 | tail -5'` | ❌ W0 | ⬜ pending |
| 03-04-02 | 04 | 3 | CORE-04 | T-03-07..T-03-11 | Send lifecycle (live publish + archive), two-route dedupe (live + CRDT), role flip (live + history), history sort, persistence (GREEN) | unit (C++) | `bash -c 'cd build/OSX/Debug && ninja gcs_core test_gcs_messaging && ctest -R test_gcs_messaging --output-on-failure'` | ❌ W0 | ⬜ pending |
| 03-05-01 | 05 | 4 | CORE-04 | T-03-12 / T-03-13 | Validate-then-delegate; wallet-address sender stamped C++-side | build | `bash -c 'cd build/OSX/Debug && ninja gcs_ffi'` | ✅ | ⬜ pending |
| 03-05-02 | 05 | 4 | CORE-04 (D-06/D-03) | T-03-14 | CRDT receive callback + raw live-subscribe handler both under `g_mutex` | unit/integration | `bash -c 'cd build/OSX/Debug && ninja gcs_core gcs_ffi && ctest -R "test_gcs_messaging|test_gcs_ffi" --output-on-failure'` | ✅ | ⬜ pending |
| 03-06-01 | 06 | 5 | CORE-04 SC1-4 | — | Two live app instances; SC2/SC3 live delivery + convergence; sender identification on replay; "Send failed" toast | manual (human-verify) | — | — | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

Wave 0 (first execution wave of this phase) creates the verification scaffolds every later task depends on. All are assigned to owning plans below:

- [ ] `test/test_gcs_messaging.cpp` — CORE-04 send/receive/history/dedupe/role-flip (new target `test_gcs_messaging`, links `gcs_core;gcs_storage;neoswarm_common`) — owned by 03-04 Task 1 (RED).
- [ ] `test/test_gcs_global_db.cpp` — extend for topics-aware Put + QueryKeyValues prefix scan — owned by 03-02 Task 2.
- [ ] `src/app/test/cubits/shell_cubits_test.dart` — extend for upsert + messageHistory dispatch + role preservation + composer refusal — owned by 03-03 Task 2.
- [ ] `.planning/workstreams/app/phases/03-messaging/03-API-SIGNATURES.md` — SuperGenius/GeniusSDK header signature record (A1/A3/A4/A5) — owned by 03-01 Task 1.

*(Existing infra reused: `test/test_wait_condition.hpp` wait-condition template; Tier-2 injected pubsub fixture in `test_gcs_core_smoke.cpp`/`test_gcs_entities.cpp`; Dart cubit tests in `src/app/test/cubits/shell_cubits_test.dart`.)*

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Live cross-peer delivery without refresh (SC2) | CORE-04 SC2 | Requires two simultaneously-live GossipSub nodes with peer discovery; the Tier-2 injected-pubsub fixture binds one host to port 0 (no peers). Component logic is auto-covered by 03-04. | 03-06: two live app instances in the same room; message sent in A appears in B with no refresh. |
| Two-session history convergence (SC3) | CORE-04 SC3 | Same two-live-node constraint; 03-04 Test 6 covers single-db restart convergence. | 03-06: restart both instances, rejoin; history order identical across both. |
| "Send failed" toast visual | CORE-04 (D-07) | Toast is an overlay visual; wiring is code-reviewed via 03-03 + dart analyze, but the overlay render is visual. | 03-06 step 6: trigger a transport failure and confirm the error tint + "Send failed" toast. |

*All other phase behaviors have automated verification (see map above).*

---

## Validation Sign-Off

- [x] All tasks have `<automated>` verify or Wave 0 dependencies
- [x] Sampling continuity: no 3 consecutive tasks without automated verify
- [x] Wave 0 covers all MISSING references
- [x] No watch-mode flags
- [x] Feedback latency < 120s
- [x] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
