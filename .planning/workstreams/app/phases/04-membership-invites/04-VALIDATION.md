---
phase: 4
slug: membership-invites
status: draft
nyquist_compliant: false
wave_0_complete: true
created: 2026-09-27
---

# Phase 4 — Validation Strategy

> Per-phase validation contract for feedback sampling during execution.

---

## Test Infrastructure

| Property | Value |
|----------|-------|
| **Framework** | GoogleTest (GTest) via the `gcs_test()` macro; Flutter `flutter test` for Dart cubits/widgets |
| **Config file** | `test/CMakeLists.txt` (`gcs_test(<name> <sources> "<libs>")`) |
| **Quick run command** | `ctest -R "test_gcs_membership|test_gcs_crypto"` |
| **Full suite command** | `ctest` (build tree) + `flutter test` (src/app) |
| **Estimated runtime** | ~60 seconds full C++ suite; quick subset ~10 s |

---

## Sampling Rate

- **After every task commit:** Run the task's mapped automated command (fast, deterministic)
- **After every plan wave:** Run `ctest` (full C++) + `flutter test` for cubits/widgets
- **Before `/gsd:verify-work`:** Full suite must be green (C++ + Flutter)
- **Max feedback latency:** ~60 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| 04-01 T1 | 04-01 | 1 | MEMB-01, MEMB-02 | T-04-01-EoP | Role/PendingAction enums + MemberRecord/InviteRecord + creator/members_can_invite/approved_by fields | build | `cmake --build build --target gcs_proto` | extends | ⬜ pending |
| 04-01 T2 | 04-01 | 1 | MEMB-01, MEMB-05 | T-04-02-T | 7 command + 2 event oneof arms + members_can_invite on create/update commands | build | `cmake --build build --target gcs_proto` | extends | ⬜ pending |
| 04-02 T1 | 04-02 | 1 | MEMB-01, MEMB-02 | T-04-02-DoS | Per-room key store + MintRoomKey + DeriveRoomKey ikm swap (topic fallback) | unit | `ctest -R test_gcs_crypto` | extends | ⬜ pending |
| 04-02 T2 | 04-02 | 1 | MEMB-01 | T-04-02-T | WrapRoomKey/UnwrapRoomKey round-trip / wrong-key / tamper / truncation | unit | `ctest -R test_gcs_crypto` | extends | ⬜ pending |
| 04-04 T1 | 04-04 | 2 | MEMB-03, MEMB-05 | T-04-04-T | Two-arg EntityStore stamps creator + members_can_invite on create | unit | `ctest -R test_gcs_entities` | extends | ⬜ pending |
| 04-04 T2 | 04-04 | 2 | MEMB-03 | T-04-04-T | UpdateSpace preserves stored creator | unit | `ctest -R test_gcs_entities` | extends | ⬜ pending |
| 04-06 T1 | 04-06 | 2 | MEMB-02 | T-04-06-EoP | Dart pb regen (selfAddress/InviteRecord/approvedBy/pendingAction) + MembersCubit | analyze | `cd src/app && dart analyze --fatal-infos lib test` | extends | ⬜ pending |
| 04-06 T2 | 04-06 | 2 | MEMB-02, MEMB-04 | T-04-06-EoP | SessionCubit hasMemberList/hasInviteLink dispatch + pushed selfAddress ("You" marker) + RailSpace/RailRoom surface approvedBy/pendingAction | widget | `cd src/app && flutter test test/cubits/shell_cubits_test.dart` | extends | ⬜ pending |
| 04-03 T1 | 04-03 | 3 | MEMB-01..05 | T-04-03-EoP-1 | Membership interface contract declaration | static/grep | `cmake --build build --target gcs_proto` (compile proven in T2) | new | ⬜ pending |
| 04-03 T2 | 04-03 | 3 | MEMB-01..05 | T-04-03-EoP-1/2/3, T-04-03-T-1/2, T-04-03-DoS | Member CRUD + guards + invite record + dormant/approve (9 cases) | unit | `ctest -R test_gcs_membership` | new | ⬜ pending |
| 04-03 T3 | 04-03 | 3 | MEMB-01, MEMB-02 | T-04-03-DoS | Two-node redeem/role-change convergence | integration | `ctest -R test_gcs_membership_multinode` | new | ⬜ pending |
| 04-07 T1 | 04-07 | 3 | MEMB-01, MEMB-02, MEMB-04 | T-04-07-EoP | Members dialog roster/invite/role/confirm surfaces + approve publishes ApproveCommand | analyze | `cd src/app && dart analyze --fatal-infos lib test` | new | ⬜ pending |
| 04-07 T2 | 04-07 | 3 | MEMB-01, MEMB-04 | T-04-07-I, T-04-07-EoP | Join dialog inline invite-link validation + rail Join/Members/Delete affordances + Pending deletion badge/approve | analyze | `cd src/app && dart analyze --fatal-infos lib test` | new | ⬜ pending |
| 04-07 T3 | 04-07 | 3 | MEMB-01, MEMB-02 | T-04-07-EoP | MembersCubit wiring + dialog/rail widget tests (incl. approve publish) | widget | `cd src/app && flutter test test/shell/members_dialog_test.dart test/shell/join_dialog_test.dart test/chat_shell_test.dart` | new + extends | ⬜ pending |
| 04-05 T1 | 04-05 | 4 | MEMB-02 | T-04-05-DoS | g_membership construction + members heal callback + MemberList/InviteLink builders | unit | `ctest -R test_gcs_ffi` | extends | ⬜ pending |
| 04-05 T2 | 04-05 | 4 | MEMB-01..05 | T-04-05-EoP-1/2, T-04-05-I | Seven dispatch arms + P5 retroactive guards + creator/members_can_invite stamping | unit | `ctest -R test_gcs_ffi` | extends | ⬜ pending |
| 04-05 T3 | 04-05 | 4 | MEMB-02 (D-02) | T-04-05-EoP-2 | RefreshDerivedJoins explicit_leave term | unit | `ctest -R "test_gcs_ffi|test_gcs_messaging"` | extends | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

*File Exists: `extends` = an existing test file is extended this task; `new` = the test file is created this task (alongside the implementation).*

---

## Wave 0 (Scaffolding)

No separate Wave 0 scaffolding phase — every implementation task writes its own test file alongside the code (each plan's `<action>` lists the test file in `<files>` before the `<verify>` runs). There are no MISSING test references to pre-create, so the Wave 0 gap list is cleared.

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Invite link copies to clipboard and pastes into join dialog on a second device | MEMB-01 | Requires two running nodes with UI + OS clipboard | Mint invite on node A; copy link; paste into join dialog on node B; verify member appears in both member lists |

---

## Validation Sign-Off

- [ ] All tasks carry an `<automated>` verify (tests accompany implementation — no Wave 0 dependencies)
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] No watch-mode flags
- [ ] Feedback latency < 60s
- [ ] `nyquist_compliant: true` set in frontmatter (set at sign-off during execution)

**Approval:** pending
