---
phase: 4
slug: membership-invites
status: draft
nyquist_compliant: false
wave_0_complete: false
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

- **After every task commit:** Run `ctest -R "test_gcs_membership|test_gcs_crypto"` (fast, deterministic)
- **After every plan wave:** Run `ctest` (full C++) + `flutter test` for cubits/widgets
- **Before `/gsd:verify-work`:** Full suite must be green (C++ + Flutter)
- **Max feedback latency:** ~60 seconds

---

## Per-Task Verification Map

| Task ID | Plan | Wave | Requirement | Threat Ref | Secure Behavior | Test Type | Automated Command | File Exists | Status |
|---------|------|------|-------------|------------|-----------------|-----------|-------------------|-------------|--------|
| TBD (assigned at plan time) | — | — | MEMB-01 | — | Mint returns `gcs://invite/...` URL; redeem joins + unwraps room key | unit | `ctest -R test_gcs_membership` | ❌ W0 | ⬜ pending |
| TBD (assigned at plan time) | — | — | MEMB-02 | — | Five-tier role enum + member record CRUD + role assignment | unit | `ctest -R test_gcs_membership` | ❌ W0 | ⬜ pending |
| TBD (assigned at plan time) | — | — | MEMB-03 | — | Demotion targeting creator rejected by C++ write guard | unit | `ctest -R test_gcs_membership` | ❌ W0 | ⬜ pending |
| TBD (assigned at plan time) | — | — | MEMB-04 | — | Destructive op dormant when 2+ admins; Super Admin approve activates | unit | `ctest -R test_gcs_membership` | ❌ W0 | ⬜ pending |
| TBD (assigned at plan time) | — | — | MEMB-05 | — | Member invite gated by `members_can_invite` | unit | `ctest -R test_gcs_membership` | ❌ W0 | ⬜ pending |
| TBD (assigned at plan time) | — | — | (crypto) | — | Wrap/unwrap round-trip; wrong key fails; key store thread-safety | unit | `ctest -R test_gcs_crypto` | ✅ extend | ⬜ pending |
| TBD (assigned at plan time) | — | — | (FFI) | — | invite/redeem/role arms validate + push `MemberList`/`InviteLink` | unit | `ctest -R test_gcs_ffi` | ✅ extend | ⬜ pending |
| TBD (assigned at plan time) | — | — | (multinode) | — | Remote redeem converges member list on peer | integration | `ctest -R test_gcs_membership_multinode` | ❌ W0 | ⬜ pending |

*Status: ⬜ pending · ✅ green · ❌ red · ⚠️ flaky*

---

## Wave 0 Requirements

- [ ] `test/test_gcs_membership.cpp` — covers MEMB-01..05 (member CRUD, guards, approval, key-wrap round-trip)
- [ ] `test/test_gcs_membership_multinode.cpp` (or extend the existing multinode fixture) — remote convergence
- [ ] `test/test_gcs_crypto.cpp` extension — `WrapRoomKey`/`UnwrapRoomKey` + per-room key store
- [ ] `test/test_gcs_ffi.cpp` extension — invite/redeem/role/leave/delete/approve arms
- [ ] Dart cubit test — `MembersCubit.setMembers` full-replacement + `_dispatchEvent` arms

---

## Manual-Only Verifications

| Behavior | Requirement | Why Manual | Test Instructions |
|----------|-------------|------------|-------------------|
| Invite link copies to clipboard and pastes into join dialog on a second device | MEMB-01 | Requires two running nodes with UI + OS clipboard | Mint invite on node A; copy link; paste into join dialog on node B; verify member appears in both member lists |

---

## Validation Sign-Off

- [ ] All tasks have `<automated>` verify or Wave 0 dependencies
- [ ] Sampling continuity: no 3 consecutive tasks without automated verify
- [ ] Wave 0 covers all MISSING references
- [ ] No watch-mode flags
- [ ] Feedback latency < 60s
- [ ] `nyquist_compliant: true` set in frontmatter

**Approval:** pending
