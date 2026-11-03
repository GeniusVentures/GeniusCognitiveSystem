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
| 04-13 T1 | 04-13 | 7 | MEMB-01, MEMB-04 | T-04-13-01/02/03 | Redeem merges into a live record; tombstone rejected after the authenticator gate and before the unwrap loop (no member or room-key write) | unit | `ninja -C build/OSX/Debug test_gcs_membership && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` | extends | ⬜ pending |
| 04-13 T2 | 04-13 | 7 | MEMB-01, MEMB-04 | T-04-13-01/02 | Dormant removal survives redeem; tombstoned redeem rejected with byte-identical stored record | unit | `ninja -C build/OSX/Debug test_gcs_membership && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` | extends | ⬜ pending |
| 04-14 T1 | 04-14 | 8 | MEMB-04 | T-04-14-01 | ApplySpaceState/ApplyRoomState persist then publish; Put never held under `m_catalogMutex` | unit | `ninja -C build/OSX/Debug gcs_core test_gcs_entities && ctest --test-dir build/OSX/Debug -R "^test_gcs_entities$" --output-on-failure` | extends | ⬜ pending |
| 04-14 T2 | 04-14 | 8 | MEMB-04 | T-04-14-02/03/04 | DeleteScope/Approve routed through the entity mutators; AdminCount returns outcome and fails closed | unit | `ninja -C build/OSX/Debug gcs_core test_gcs_membership && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` | extends | ⬜ pending |
| 04-14 T3 | 04-14 | 8 | MEMB-04 | T-04-14-01/02/04/05 | Mutator catalog publish; dormant-delete-then-approve; AdminCount scan-failure abort | unit | `ninja -C build/OSX/Debug test_gcs_entities test_gcs_membership && ctest --test-dir build/OSX/Debug -R "^test_gcs_(entities|membership)$" --output-on-failure` | extends | ⬜ pending |
| 04-15 T1 | 04-15 | 9 | MEMB-01, MEMB-02, MEMB-05 | T-04-15-01/02/03 | Append-only `created_by`/`issuer_signature`; public hex codec; sign/verify-issuer seam | build | `ninja -C build/OSX/Debug gcs_proto gcs_core` | extends | ⬜ pending |
| 04-15 T2 | 04-15 | 9 | MEMB-01, MEMB-02, MEMB-05 | T-04-15-01/03/04/05/07 | Sign at mint; verify scope, signature, issuer authority, and wrapped-key scope at redeem | build | `ninja -C build/OSX/Debug gcs_core` | extends | ⬜ pending |
| 04-15 T3 | 04-15 | 9 | MEMB-01, MEMB-02, MEMB-05 | T-04-15-02/03/06/07 | Tampered/forged/revoked/scope-mismatch unit rejections + shared payload builder | unit | `ninja -C build/OSX/Debug test_gcs_membership && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` | extends | ⬜ pending |
| 04-16 T1 | 04-16 | 10 | MEMB-01 | T-04-16-01/02/03/04 | Account-backed `shared_ptr` signer in the FFI seam; reordered VerifySignature; fail-closed teardown | build | `ninja -C build/OSX/Debug gcs_ffi` | extends | ⬜ pending |
| 04-16 T2 | 04-16 | 10 | MEMB-01 | T-04-16-03/04 | Scoped link shape + in-process production signed mint/redeem proof | unit | `ninja -C build/OSX/Debug test_gcs_ffi && ctest --test-dir build/OSX/Debug -R "^test_gcs_ffi$" --output-on-failure` | extends | ⬜ pending |
| 04-16 T3 | 04-16 | 10 | MEMB-01 | T-04-16-05 | Pinned Dart pb regen + two-segment join-link gate | analyze/widget | `cd src/app && flutter analyze && flutter test test/shell/join_dialog_test.dart` | extends | ⬜ pending |
| 04-17 T1 | 04-17 | 11 | MEMB-02, MEMB-03 | T-04-17-01/02/03/04/05 | Concrete-role guard, target-liveness read, strict outrank in `CanChangeRole` | build | `ninja -C build/OSX/Debug gcs_core test_gcs_membership` | extends | ⬜ pending |
| 04-17 T2 | 04-17 | 11 | MEMB-02, MEMB-03 | T-04-17-01/02/03 | Role/rank rejection matrix incl. flipped Admin→Super Admin demotion expectation | unit | `ninja -C build/OSX/Debug test_gcs_membership && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` | extends | ⬜ pending |
| 04-18 T1 | 04-18 | 11 | MEMB-01, MEMB-02 | T-04-18-03/04 | Real signer identities in the fixture + real-crypto issuer seam | build | `ninja -C build/OSX/Debug test_gcs_membership_multinode` | extends | ⬜ pending |
| 04-18 T2 | 04-18 | 11 | MEMB-01, MEMB-02 | T-04-18-01/02/04 | Genuine cross-node signed redeem + forged-issuer wire rejection with baseline capture | integration | `ninja -C build/OSX/Debug test_gcs_membership_multinode && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership_multinode$" --output-on-failure` | extends | ⬜ pending |
| 04-19 T1 | 04-19 | 12 | MEMB-02, MEMB-03 | T-04-19-02/04/09/10 | Append-only `created_by`/`record_signature`/`leave_signature`/`prior_role`; canonical payload keeps `created_at_ms`/`updated_at_ms`/`prior_role` and clears only `record_signature`/`leave_signature`/`wrapped_room_key`/`explicit_leave`; subject-signed leave; hexsign/verify primitives | build | `ninja -C build/OSX/Debug gcs_proto gcs_core` | extends | ⬜ pending |
| 04-19 T2 | 04-19 | 12 | MEMB-02, MEMB-03, MEMB-04 | T-04-19-01/03/07/08 | Every write is author-signed or authority-preserving (`prior_role` derived in one place, `updated_at_ms = max(NowMs(), verified predecessor + 1)` stamped before signing, write-once verified creator root); no unsigned authority record; fail closed without the seam; accepted amplification-only re-broadcast | build | `ninja -C build/OSX/Debug gcs_core gcs_ffi` | extends | ⬜ pending |
| 04-19 T3 | 04-19 | 12 | MEMB-02, MEMB-03, MEMB-04, MEMB-05 | T-04-19-01/02/03/04/05/06/07/09/10 | Verified reads, self-signed Guest cap, unconditional creator-target rejection, creator-rooted chain, strict outrank vs signed `prior_role` with predecessor cache, pending transitions gated to admin-tier participants, D-07 creator gate for admin-tier destruction, tombstone/pending coherence, verified AdminCount/roster/heal that advances the cache before skipping tombstones | build (suite green in 04-20) | `ninja -C build/OSX/Debug gcs_core gcs_ffi test_gcs_membership` | extends | ⬜ pending |
| 04-20 T1 | 04-20 | 13 | MEMB-02, MEMB-03 | T-04-20-01/02/06 | Shared member/leave payload helpers; signed `SeedMember` with `created_by`/`prior_role`/`updated_at_ms`/`record_signature`; signed live and dead `OnMemberArrived` deliveries; flipped no-seam and peer-Super-Admin expectations | unit | `ninja -C build/OSX/Debug test_gcs_membership && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` | new + extends | ⬜ pending |
| 04-20 T2 | 04-20 | 13 | MEMB-02, MEMB-03, MEMB-04 | T-04-20-03/04/06 | Forged/tampered/insufficient-signer exclusion; self-signed caps; creator-target, admin-tier, and peer-Moderator rejection; replay (incl. pre-tombstone replay on the same Membership instance) and flipped-leave rejection; second `OnScopeCreated` does not rewrite the root; leave/redeem authority preservation incl. cold node | unit | `ninja -C build/OSX/Debug test_gcs_membership && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` | extends | ⬜ pending |
| 04-20 T3 | 04-20 | 13 | MEMB-01, MEMB-02 | T-04-20-01 | Production FFI seam remains green on signed member records | unit | `ninja -C build/OSX/Debug test_gcs_ffi && ctest --test-dir build/OSX/Debug -R "^test_gcs_ffi$" --output-on-failure` | extends | ⬜ pending |
| 04-21 T1 | 04-21 | 14 | MEMB-02, MEMB-03 | T-04-21-01/02/06 | Creator exception by `IsSuperAdmin` identity; `Approve` re-validates pending role and rejects a creator target; `Approve` REMOVE clears pending action/role and writes creator-signed so the tombstone verifies | build | `ninja -C build/OSX/Debug gcs_core test_gcs_membership` | extends | ⬜ pending |
| 04-21 T2 | 04-21 | 14 | MEMB-03, MEMB-04 | T-04-21-03/04/05/06 | Pending-REMOVE barrier; dormant removal clears pending_role; immediate removal clears all pending/approval fields; `RemoveMember` dormancy matches the verifier tier predicate (Moderator-on-Member is immediate); approved tombstone is strictly newer (`updated_at_ms`) and passes verification via an `OnMemberArrived` push | unit | `ninja -C build/OSX/Debug test_gcs_membership && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` | extends | ⬜ pending |
| 04-21 T3 | 04-21 | 14 | MEMB-02, MEMB-03, MEMB-04 | T-04-21-01 | STATE.md decisions (incl. `prior_role`, `leave_signature`, replay residual, accepted amplification, approved-tombstone rule) + validation map rows | static/grep | `grep -c "WriteMemberAsAuthor\|IsSuperAdmin( scopeId, m_sender )\|PENDING_ACTION_REMOVE\|prior_role\|leave_signature" .planning/workstreams/app/STATE.md` | extends | ⬜ pending |
| 04-22 T1 | 04-22 | 15 | MEMB-02, MEMB-03 | T-04-22-03/07 | Real-signer seam factory + cold-node entity-catalog mirror; fail-closed before metadata, re-verified admission after, heal-trigger convergence; existing convergence green | integration | `ninja -C build/OSX/Debug test_gcs_membership_multinode && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership_multinode$" --output-on-failure` | extends | ✅ green |
| 04-22 T2 | 04-22 | 15 | MEMB-02, MEMB-03, MEMB-04 | T-04-22-01/02/04/06 | Wire-injected valid self-signed SUPER_ADMIN and validly signed creator-target/admin-tier/peer-Moderator transitions excluded from both rosters and all commands while the hostile bytes win; raw bytes remain; each saved creator-signed record is restored to prove recovery | integration | `ninja -C build/OSX/Debug test_gcs_membership_multinode && ctest --test-dir build/OSX/Debug -R "^test_gcs_membership_multinode$" --output-on-failure` | extends | ✅ green |
| 04-22 T3 | 04-22 | 15 | MEMB-01..05 | T-04-22-05 | Full C++ suite + Flutter analyze/test gate; validation and state sign-off | full | `ctest --test-dir build/OSX/Debug --output-on-failure` | extends | ✅ green |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

*File Exists: `extends` = an existing test file is extended this task; `new` = the test file is created this task (alongside the implementation).*

*Expected intermediate reds after 04-15 (recorded in that plan, not regressions): `test_gcs_ffi` stays red until 04-16 wires the production seam; `test_gcs_membership_multinode` stays red until 04-18 supplies the issuer callables in `ArmMembership`. The "full C++ suite" checks in 04-16 (wave 10) and 04-17 (wave 11) therefore exclude `test_gcs_membership_multinode`; its green is asserted by 04-18 T2 (wave 11, after 04-16).*

*Re-verification cycle 3 (Waves 12-15): `test_gcs_membership` is expected red between 04-19 and 04-20 for two concrete reasons: `SeedMember` writes unsigned records, and the live/dead deliveries in `DeleteScopeTombstonesEntityAndOnMemberArrivedPushes` (test_gcs_membership.cpp 1405-1444) are unsigned and, for the dead delivery, still carry `address = kMemberAddress` under the `kDeadAddress` key — so both fail 04-19 verification and both `ASSERT_GT(events.size(), before)` fail. 04-20 T1 signs both deliveries and adds `deadArrived.set_address( kDeadAddress )`. `test_gcs_membership_multinode` is expected red between 04-19 and 04-22 because a cold node cannot resolve the creator until the fixture mirrors the entity catalog; 04-22 T1 also pins fail-closed-before-metadata and re-verification-after-metadata. These are expected intermediate states, not regressions. 04-20 and 04-22 assert their green; 04-22 T3 asserts the full suite.*

*Cycle 3 reds cleared (04-22 T3): the full C++ suite is green — 13/13 ctest targets, including `test_gcs_membership` (51/51) and `test_gcs_membership_multinode` (9/9). The 5 expected-red multinode tests are green, and the two new wire-level attack tests are green. The Flutter gate is green (`flutter test`: 80 passed, 1 skipped); `flutter analyze` reports 50 pre-existing errors confined to the vendored `scaffold/example/test/capture_images_test.dart`, which references demo packages that no longer exist and is outside this phase's scope.*

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
