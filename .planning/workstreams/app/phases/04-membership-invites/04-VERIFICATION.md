---
phase: 04-membership-invites
verified: 2026-10-21T18:03:05Z
status: human_needed
score: 30/30 must-haves verified
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 29/30
  gaps_closed:
    - "Truth 29 / CR-01 — RedeemInvite and Leave now reject a raw member record whose body names another scope before any authority-preserving write, so an honest node can no longer launder unverified bytes into another scope's member key or creator root (04-23, commits 36a8fd9 + 4cec74d; regression CrossScopeRawRecordIsNotPropagatedByLeaveOrRedeem passes and both rejection paths were observed firing)."
  gaps_remaining: []
  regressions: []
deferred:
  - truth: "CR-07 — room keys live in a process-local store, so they do not survive restart or reach another application process"
    addressed_in: "Explicitly deferred by user decision — no later phase assigned"
    evidence: "ROADMAP.md Phase 4 deferral note; Waves 5-16 do not address it; room keys remain in the process-local gcs_crypto store."
  - truth: "Wire-level admin-authored immediate tombstone of a below-admin target is accepted by VerifyMemberRecord while the command path makes the same removal dormant when 2+ admins exist"
    addressed_in: "Explicitly deferred by user decision 2026-10-20 — no later phase assigned"
    evidence: "04-23-PLAN.md:102 ('Do NOT touch the wire-level admin-immediate-tombstone dormancy divergence ... Explicitly deferred to a separate decision') and 04-23-SUMMARY.md:110. Note: STATE.md's Deferred Items table still reads '(none)'; the decision is recorded in the plan/summary, not in that table."
  - truth: "Exclusion-not-rollback — a hostile raw record stays at its original key while rosters and authority exclude it"
    addressed_in: "Accepted design residual (04-22-SUMMARY.md) — not a gap for this phase"
    evidence: "Recovery is proven by restoring the saved creator-signed record; the 04-23 fix removes the strictly worse cross-scope corruption."
  - truth: "A never-synced node trusts a signed prior_role (cold-start predecessor gap)"
    addressed_in: "Accepted design residual (04-22-SUMMARY.md) — not a gap for this phase"
    evidence: "The node-local predecessor cache warms on first verified read; a live node's verified history rejects a lying prior_role."
human_verification:
  - test: "Invite link end-to-end (visual)"
    expected: "Generate a space and a room invite; the invite view shows a gcs://invite/... link with a Copy action and copying preserves the URL."
    why_human: "Clipboard integration and rendered link presentation."
  - test: "Scope isolation of the invite view"
    expected: "Open scope A's invite view, close it, open scope B's; B never shows A's link."
    why_human: "Interactive drawer state across scopes."
  - test: "Join flow"
    expected: "Paste a valid link: Join enables, the dialog closes, and the member appears. Paste an invalid shape: the inline error keeps the dialog open."
    why_human: "Interactive Flutter flow."
  - test: "Post-join room access (review WR-01)"
    expected: "After redeeming a standalone-room invite and a space invite with autoJoinRooms, the joined room(s) become selectable and receive messages without restarting the app. This is expected to FAIL by inspection: kRedeem does not refresh derived joins or push RoomList, and kLeave does not recompute the derived projection."
    why_human: "Live multi-cubit state; confirm the user-visible impact and whether the warning still warrants a follow-up plan."
  - test: "Role badges and gating"
    expected: "Five-tier badges, the 'You' marker, and role-gated affordances match the UI spec for each tier."
    why_human: "Visual/presentation checks."
  - test: "Pending-op surface"
    expected: "With 2+ admins, a removal shows a Pending badge and the Super Admin Approve affordance activates it; a target re-redeeming an invite does not clear the badge."
    why_human: "Multi-actor UI state."
  - test: "Two live nodes (production config)"
    expected: "Mint on A, redeem on B (not joined); B joins as Guest and both rosters converge."
    why_human: "Live-node smoke check (automated multinode proof exists)."
---

# Phase 4: Membership & Invites Verification Report

**Phase Goal:** Users can invite others via capability tokens, and the five-tier role model (Super Admin -> Guest) governs what actions each participant can take.
**Verified:** 2026-10-21T18:03:05Z
**Status:** human_needed
**Re-verification:** Yes — after gap-closure plan 04-23 (cross-scope raw-record binding)

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
| --- | --- | --- | --- |
| 1 | SC1: User can generate a `gcs://invite/...` link for a space or room | VERIFIED | `MintInvite` (gcs_membership.cpp:1260-1446) emits `gcs://invite/{space\|room}/<scopeId>/<inviteId>?key=<IK>` and fails closed without the issuer seam. `MintStampsIssuerAndSignatureAndScopedUrl` passes. |
| 2 | SC2: Recipient can use the invite to join and appears in the member list | VERIFIED | `SpaceInviteConvergesWithoutTheRecipientJoiningTheSpace` passes; both rosters show 2 members. Warning WR-01: the FFI redeem arm still does not refresh derived joins/push RoomList. |
| 3 | SC3: Each participant has a visible five-tier role | VERIFIED | Five-tier `Role` enum (proto:207-214); `MemberList` carries roles + `self_address`; Dart suite 80 passed / 1 skipped; role badges in `members_dialog.dart`. Roster authenticity is verified (truths 13/29). |
| 4 | SC4: Super Admin (creator) cannot be demoted by other admins | VERIFIED | `CanChangeRole` rejects any non-SUPER_ADMIN change targeting the creator (1181-1186); `CanRemoveMember` rejects the creator (1220-1228); `CreatorDemotionIsRejected` and `CreatorIdentityIsTheOnlyCreatorException` pass. |
| 5 | SC5: When 2+ admins exist, destructive actions require Super Admin approval | VERIFIED | Command paths: `RemoveMember` dormant predicate (1862-1922), `ChangeRole` admin demotion dormant (1799-1860), `DeleteScope` `needsApproval` (1986-2059), `Approve` activates and re-validates (2062-2233); `AdminCount` fails closed (866-919); pending state coherent (ChangeRole rejects pending REMOVE; RemoveMember clears pending_role). `TwoAdminRemovalIsDormantUntilApproved`, `DeleteScopeDormantThenApproved`, `ChangeRoleRejectedWhileRemovalIsPending` pass. Caveats: WR-11 (update_space drops pending fields) and the wire-level admin-immediate-tombstone divergence, which is explicitly deferred by user decision (see Deferred Items). |
| 6 | Proto carries Role/MemberRecord/MemberList/InviteLink/InviteRecord + creator/members_can_invite + 7 command / 2 event arms, append-only | VERIFIED | proto:203-310; `MemberRecord` fields 12-15 appended without touching 1-11; `InviteRecord` 6-7 appended; command arms 6-12 (proto:62-68) and event arms 7-8 (proto:110-111). |
| 7 | 32-byte room key AES-256-GCM wrap/unwrap; per-room key store; D-08 envelope/HKDF unchanged | VERIFIED | `test_gcs_crypto` passes; Wrap/Unwrap/RoomKeyFor/DeriveRoomKey present. Store remains process-local (CR-07, deferred). |
| 8 | MintInvite persists ONE invite record, writes NO member record, wraps keys under IK | VERIFIED | gcs_membership.cpp:1260-1446; `MintPersistsInviteRecordAndWritesNoMember` passes. |
| 9 | RedeemInvite writes the redeemer's wallet-keyed member record and stores the key; multi-use | VERIFIED | gcs_membership.cpp:1448-1798; `RedeemWritesMemberStoresKeyAndIsMultiUse` passes. Existing live member keeps stored authority (1752-1795). |
| 10 | ChangeRole rejects demotion targeting the scope creator | VERIFIED | gcs_membership.cpp:1181-1186; `CreatorDemotionIsRejected` passes. |
| 11 | RemoveMember/ChangeRole(demotion)/DeleteScope dormant when 2+ admins; Approve activates | VERIFIED | Command predicates + AdminCount fail-closed + catalog mutators; `DeleteScopeDormantThenApproved`, `AdminDemotionIsDormantWhenTwoAdmins`, `TwoAdminRemovalIsDormantUntilApproved` pass. Warning WR-11. |
| 12 | MintInvite gated by author role + members_can_invite (MEMB-05) | VERIFIED | `CanInvite`/`CanInviteFor` (1123-1148); `membersCanInviteGatesMemberMinting` passes. |
| 13 | MembersFor prefix-scans, skips tombstones, stamps self_address, verifies records | VERIFIED | gcs_membership.cpp:1048-1101 verifies before the tombstone skip and derives the wallet from the key; unit/FFI tests pass. |
| 14 | Creator + members_can_invite stamped; UpdateSpace preserves creator | VERIFIED | gcs_entity_store.cpp:195-235 (creator) and UpdateSpace preserves creator (293-350); entity suite passes. WR-11: approved_by/pending_action are not preserved. |
| 15 | gcs_init constructs Membership + heal callback so remote membership converges | VERIFIED | gcs_core_ffi.cpp:1105-1132 (callback), 1279-1291 (construction); multinode convergence tests pass. |
| 16 | The seven command arms validate, guard, delegate, and push | VERIFIED | gcs_core_ffi.cpp:1701-1900; FFI suite passes. Warnings WR-01/WR-08. |
| 17 | P5 guards on send_text/create_room/update_space; create paths stamp creator/members_can_invite | VERIFIED | `RoleAtLeast` (774-777) + send_text gate (1458-1483), create paths at 1587/1668; entity/FFI tests pass. Warning WR-07. |
| 18 | Derived joins respect explicit_leave (D-02) | VERIFIED | `HasExplicitLeave` (1113-1121) and the `RefreshDerivedJoins` filter (gcs_core_ffi.cpp:820); the owning-module branch is covered (`LeavePreservesPromotedRoleAuthority`/HasExplicitLeave assertion at test:1559). Warning WR-01: leave does not trigger a refresh, so the effect lands on the next projection recomputation. |
| 19 | Dart MembersCubit + dispatch arms + self flag | VERIFIED | `members_cubit.dart` byScope/setMembers/self (76-219); `session_cubit.dart:540`; Dart suite green. |
| 20 | UI surfaces: members dialog, invite view, join dialog, rail affordances, pending badges | VERIFIED | Dart suite green (`members_dialog.dart`, `join_dialog.dart`); visual checks remain human. Warning WR-09 (self change-role). |
| 21 | Concurrent membership and entity-mutating commands do not corrupt shared state | VERIFIED | `m_catalogMutex` guards catalog readers/writers; `ConcurrentCatalogReadsAndRoomCreatesDoNotRace` (test_gcs_entities.cpp:814) passes. |
| 22 | Rank-strict removal guard: Moderator cannot remove Admin; Admin cannot remove non-creator Super Admin | VERIFIED | `CanRemoveMember` strict outrank (1219-1247); `ModeratorCannotRemoveAdminAndAnUnverifiableSuperAdminGrantsNothing` passes. |
| 23 | Demoting a non-creator Super Admin with 2+ admins writes a dormant role change | VERIFIED | Rank-based `isAdminDemotion` (ChangeRole 1799-1860); `AdminDemotionIsDormantWhenTwoAdmins` passes. |
| 24 | The bearer key gates every redeem; no key/no authenticator writes no member record | VERIFIED | Unconditional `InviteAuthEnabled()` gate before any write (1566-1582); `ForgedOrKeylessSpaceInviteWritesNoMemberRecord`, `ChildlessSpaceInviteIsBearerGated` pass. Warning WR-06: gate is seam-optional. |
| 25 | Tombstone arrival pushes the filtered roster; remote removal converges | VERIFIED | `OnMemberArrived` (2235-2272); `RemovedMemberReceivesTheConvergedRoster` passes. |
| 26 | Membership replicates on a registered well-known topic; an unjoined recipient redeems | VERIFIED | `kMembershipTopic` registered listen-then-broadcast at init (gcs_core_ffi.cpp:1015-1022); multinode suite passes. |
| 27 | Teardown releases Membership before the session/catalog it borrows | VERIFIED | gcs_core_ffi.cpp:350-380 evicts globals and resets membership before session shutdown/entities; `sdkBootedHere` teardown present. Warning WR-12: failed init paths do not pair the embedded SDK boot. |
| 28 | Invite view renders the link for its own scope; clear-on-close | VERIFIED | `_pendingInviteLinks[scopeId]` / `pendingInviteLinkFor` / `clearPendingInviteLink` (session_cubit.dart:167-181, 540); widget tests pass. |
| 29 | The role model governs what actions each participant can take — no self-granted escalation, and redeem does not rewrite stored role/pending state | VERIFIED | Signed writes/verified reads/matrix/write-once root are implemented and the wire-level injected-authority tests pass. CR-01 is closed: `RedeemInvite` rejects `existing.scope_id() != urlScopeId` (1602, after the address check and before the tombstone check) and `Leave` rejects `record.scope_id() != scopeId` (1961, before `set_explicit_leave`/signing/Put), so `WriteMemberPreservingAuthority` (982-1008) can no longer derive its Put key from a body naming another scope. `CrossScopeRawRecordIsNotPropagatedByLeaveOrRedeem` passes, both rejection warnings were observed firing, and B's baseline bytes plus A's hostile bytes are asserted unchanged. |
| 30 | A removed (tombstoned) member cannot rejoin via a stale invite | VERIFIED | `RedeemInvite` rejects `existing.deleted()` before unwrap/store (1610-1617); `TombstonedMemberCannotRedeemStaleInvite` asserts stored bytes and scan count unchanged. |

**Score:** 30/30 truths verified

### Deferred Items

Items not yet met but explicitly deferred/accepted, not open gaps.

| # | Item | Addressed In | Evidence |
| --- | --- | --- | --- |
| 1 | CR-07 — room keys live in a process-local store; they do not survive restart or reach another process | Deferred by user decision; no later phase assigned | ROADMAP.md Phase 4 deferral note; not addressed by Waves 5-16 |
| 2 | Wire-level admin-authored immediate tombstone is accepted by the verifier while the command path is dormant when 2+ admins | Deferred by user decision 2026-10-20; no later phase assigned | 04-23-PLAN.md:102; 04-23-SUMMARY.md:110 (STATE.md's Deferred Items table was not updated) |
| 3 | Exclusion-not-rollback — a hostile raw record stays at its original key while rosters and authority exclude it | Accepted design residual (04-22-SUMMARY.md) | Recovery is proven by restoring the saved creator-signed record |
| 4 | A never-synced node trusts a signed `prior_role` (cold-start predecessor gap) | Accepted design residual (04-22-SUMMARY.md) | The node-local predecessor cache warms on first verified read |

### Required Artifacts

| Artifact | Expected | Status | Details |
| --- | --- | --- | --- |
| `src/proto/gcs_chat.proto` | Append-only membership wire contract | VERIFIED | Role/PendingAction/WrappedRoomKey/MemberRecord/MemberList/InviteLink/InviteRecord + creator/members_can_invite + 7 command / 2 event arms. `MemberRecord` fields 12-15 appended without touching 1-11. |
| `src/lib/gcs_crypto.hpp/.cpp` | Key-wrap + per-room key store + ikm swap + invite authenticator + hex codec | VERIFIED | Wrap/Unwrap/RoomKeyFor/DeriveRoomKey/MintInviteAuthenticator/VerifyInviteAuthenticator/HexEncode/HexDecode present; crypto suite passes. Store is process-local (CR-07 deferred). IN-05: verify accepts any non-empty key length. |
| `src/lib/gcs_membership.hpp/.cpp` | Membership component (roster, roles, invites, approval, signed authority, scope-bound preserving writes) | VERIFIED | Signed writes (`WriteMemberAsAuthor` 921-980), verified reads (`VerifyMemberRecord` 518-819; `TryReadMember`, `MembersFor` 1048-1101, `AdminCount` 866-919, `OnMemberArrived` 2235-2272), transition matrix, write-once root (2274-2358), coherent pending state, identity creator exception (1194), validated `Approve` (2062-2233). Both authority-preserving paths now bind the raw body to the requested scope (1602, 1961); `TryReadRawMember` documents its non-binding contract (hpp:648-663). |
| `src/lib/gcs_entity_store.hpp/.cpp` | Creator/members_can_invite stamping + synchronized catalog + deletion mutators | VERIFIED | CreateSpace/CreateRoom stamp creator; `ApplySpaceState`/`ApplyRoomState` Put outside `m_catalogMutex` then publish under it (352-389). Warning WR-11: `UpdateSpace` resets `approved_by`/`pending_action`. |
| `src/ffi/gcs_core_ffi.cpp` | Membership construction + heal callback + 7 arms + guards + lifecycle + issuer seam | VERIFIED | Membership topic at init (1015-1022); construction/seam (1105-1291); teardown (350-380). Warnings WR-01 (no derived-join refresh on redeem/leave), WR-07 (ScopeHasRoster fails open), WR-08 (scan failure pushed as empty roster), WR-12 (failed-init SDK teardown), WR-13 (orphan scope on creator-root failure). |
| `src/app/lib/cubits/members_cubit.dart` | Per-scope roster state + MemberInfo | VERIFIED | byScope, setMembers, self flag (76-219); Dart tests pass. |
| `src/app/lib/cubits/session_cubit.dart` | Scope-keyed pending invite link | VERIFIED | `_pendingInviteLinks`, `pendingInviteLinkFor`, `clearPendingInviteLink` (167-181, 540). |
| `src/app/lib/shell/members_dialog.dart` | Member list + scope-keyed invite view + confirmations | VERIFIED | Widget tests pass. Warning WR-09: change-role affordance on the viewer's own row. |
| `src/app/lib/shell/join_dialog.dart` | Paste-invite redeem flow | VERIFIED | `kInviteLinkPattern` accepts the two-segment scoped URL; tests pass. |
| `src/app/lib/generated/proto/gcs_chat.pb.dart` | Regenerated Dart bindings | VERIFIED | Exposes `createdBy`/`recordSignature`/`priorRole`/`issuerSignature`; Dart suite compiles and passes. |
| `test/test_gcs_member_helpers.hpp` | Canonical issuer/leave payload + hex signature helpers shared by unit and multinode suites | VERIFIED | `BuildMemberIssuerPayload`, `BuildMemberLeavePayload`, `TestMemberSignatureHex`, `TestLeaveSignatureHex`. |
| `test/test_gcs_membership.cpp` | Cross-scope raw-record regression | VERIFIED | `CrossScopeRawRecordIsNotPropagatedByLeaveOrRedeem` (test:1390-1478) plants a validly signed B-bodied record under A/alice, asserts both operations fail, and asserts `gcs/members/B/<sender>` keeps its baseline bytes while A keeps the hostile bytes; the hostile record differs in role/timestamp so a propagated write would change the asserted bytes. |

### Key Link Verification

| From | To | Via | Status | Details |
| --- | --- | --- | --- | --- |
| `MintInvite` | `KeyWrapSeam::sign` | issuer signature over canonical payload before Put | WIRED | gcs_membership.cpp:1419-1446. |
| `RedeemInvite` | `KeyWrapSeam::verifyIssuer` | verify against `created_by` before any member write | WIRED | gcs_membership.cpp:1605-1632. |
| `RedeemInvite` | `CanInviteFor` | issuer authority when the scope resolves locally | WIRED | gcs_membership.cpp:1657-1666. |
| `RedeemInvite` raw read | `WriteMemberPreservingAuthority` | reject `existing.scope_id() != urlScopeId` before merge/write | WIRED | gcs_membership.cpp:1602 (CR-01 closure, 04-23); raw read at 1589. |
| `Leave` raw read | `WriteMemberPreservingAuthority` | reject `record.scope_id() != scopeId` before mutation/signing/Put | WIRED | gcs_membership.cpp:1961 (CR-01 closure, 04-23); raw read at 1944. |
| `CrossScopeRawRecordIsNotPropagatedByLeaveOrRedeem` | `gcs/members/<scope>/<address>` store keys | PutMemberBytes/Get byte assertions | WIRED | test/test_gcs_membership.cpp:1390-1478; `kMembersKeyPrefix` keys asserted byte-identical. |
| `DeleteScope` | `EntityStore::ApplySpaceState`/`ApplyRoomState` | catalog-synchronized destructive write | WIRED | gcs_membership.cpp:1996-2029 (shifted by the 04-23 insertion). |
| `Approve` (entity branch) | `EntityStore::ApplySpaceState`/`ApplyRoomState` | re-read authoritative bytes, apply tombstone, push | WIRED | gcs_membership.cpp:2146-2199. |
| `AdminCount` | `CoreSession::QueryKeyValues` | scan failure propagated, never 0 | WIRED | gcs_membership.cpp:866-919; three propagating call sites. |
| `CanChangeRole` | `TryReadMember` + `PrivilegeRank` | target's current role, strict outrank | WIRED | gcs_membership.cpp:1150-1217. |
| `MemberRecord` | authorization source | signed issuer/record binding verified on every read | WIRED | VerifyMemberRecord (518-819) called from TryReadMember, MembersFor, AdminCount, OnMemberArrived, OnScopeCreated. |
| `CanChangeRole` creator exception | `IsSuperAdmin` | creator identity, not resolved role | WIRED | gcs_membership.cpp:1194. |
| `Approve` | concrete-role guard | re-validate `pending_role`/creator target before applying | WIRED | gcs_membership.cpp:2074-2112. |
| `gcs_init` | `Membership` + `kMembershipTopic` | construction + listen-then-broadcast | WIRED | gcs_core_ffi.cpp:1015-1022, 1279-1291. |
| `gcs_shutdown` | `g_membership`/`g_inviteAccount` | evict/reset before session shutdown | WIRED | gcs_core_ffi.cpp:350-380. |
| `session_cubit.dart` | `MembersCubit.setMembers` / scope-keyed link | dispatch arms | WIRED | session_cubit.dart:520-540. |
| `kRedeem` / `kLeave` | `RefreshDerivedJoins` + `BuildRoomListEvent` | derived-join refresh after membership change | NOT_WIRED (warning — not a plan must-have) | `RefreshDerivedJoins` only at init/create_room/update_space; kRedeem pushes SpaceTree only, kLeave pushes MemberList only (gcs_core_ffi.cpp:1736-1764, 1818-1844). Tracked as review WR-01 and surfaced in Human Verification; no plan declares this as a must-have key link and no roadmap SC depends on it. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
| --- | --- | --- | --- | --- |
| members_dialog roster | `members.state.byScope[scopeId]` | Pushed `MemberList` -> `Membership::MembersFor` prefix scan over verified member records | Yes (unverifiable records excluded) | FLOWING |
| members_dialog invite view | `session.pendingInviteLinkFor(scopeId)` | Pushed `InviteLink` from `MintInvite`; scope stamped by C++ | Yes | FLOWING |
| room_rail pending deletion badge | `SpaceRecord.pending_action` / `deleted` | Pushed `SpaceTree` -> `EntityStore::Spaces()` after `ApplySpaceState` | Yes | FLOWING |
| members_dialog pending removal badge | `MemberRecord.pending_action` | Pushed `MemberList` -> verified member records; preserved across redeem | Yes | FLOWING |
| post-redeem room access | `RoomList` / derived-join topics | FFI `kRedeem` pushes only `SpaceTree`; no `RefreshDerivedJoins` | No | DISCONNECTED (WR-01) |
| post-leave derived joins | `explicit_leave` -> `DerivedJoinedTopics` | FFI `kLeave` writes the flag but never recomputes/pushes | No | DISCONNECTED (WR-01) |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
| --- | --- | --- | --- |
| Build affected targets | `ninja -C build/OSX/Debug gcs_core test_gcs_membership test_gcs_membership_multinode` | "no work to do" (tree current at HEAD) | PASS |
| Full C++ suite | `ctest --test-dir build/OSX/Debug --output-on-failure` | 13/13 passed (209.87 s) — incl. test_gcs_ffi 91.07 s, test_gcs_ffi_sdk 49.90 s, test_gcs_ffi_coldboot 16.89 s | PASS |
| Membership unit suite | `ctest -R "^test_gcs_membership$"` | Passed (8.67 s); 52/52 TEST_F cases (binary list) | PASS |
| Membership multinode suite | `ctest -R test_gcs_membership_multinode` | Passed (14.40 s); 9/9 TEST_F cases | PASS |
| Cross-scope raw-record regression (non-vacuity) | `test_gcs_membership --gtest_filter='*CrossScopeRawRecord*'` | 1 test ran, PASSED (157 ms); log shows BOTH new rejection warnings firing ("leave rejected — ... names scope 'cross-scope-b'" and "redeem rejected — ... names scope 'cross-scope-b'"), so the new code paths are exercised | PASS |
| Dart app suite | `thirdparty/flutter/bin/flutter test` (src/app) | 80 passed, 1 skipped, 0 failed | PASS |
| Wire-injected self-signed Super Admin excluded | `WireInjectedSelfSignedSuperAdminIsNotAMember` (within multinode suite) | Passed | PASS |
| Wire-injected hostile transitions excluded | `WireInjectedAuthorityTransitionsAreRejected` (within multinode suite) | Passed | PASS |
| Cold-node late-metadata convergence | `ColdNodeConvergesWhenEntityMetadataArrivesAfterMemberRecord` | Passed | PASS |

### Probe Execution

No probe scripts are declared for this phase and no `scripts/*/tests/probe-*.sh` files exist. Step 7c: SKIPPED (no probes).

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
| --- | --- | --- | --- | --- |
| MEMB-01 | 04-01,02,03,05,07,09,10,11,12,13,15,16,18 | User can invite others via capability token (`gcs://invite/...`) | SATISFIED | Scoped, bearer-authenticated, issuer-signed invite minted at 1260-1446; redeem verifies signature/scope/kind/role cap/authority (1448-1798); cross-node positive and forgery-negative multinode proofs pass. |
| MEMB-02 | 04-01,02,03,05,06,07,08,09,12,15,17,18,19,20,21,22,23 | Space/room has roles: Super Admin, Admin, Moderator, Member, Guest | SATISFIED | Five-tier enum, signed/verified roster, UI labels/badges; the cross-scope authority-laundering path is closed (04-23), and the full membership unit suite (52 cases) is green. |
| MEMB-03 | 04-01,03,04,05,09,12,17,19,20,21,22 | Super Admin (creator) cannot be demoted by other admins | SATISFIED | ChangeRole creator guard + strict outrank (1150-1247); wire-level creator-target rejection; tests pass. |
| MEMB-04 | 04-01,03,05,06,07,08,09,12,13,14,19,20,21,22,23 | Destructive actions require Super Admin approval when 2+ admins exist | SATISFIED | Command paths enforce dormancy, AdminCount fails closed, Approve re-validates, pending state is coherent; cross-scope creator-root corruption is closed. Caveats: WR-11 and the user-deferred wire-level admin-immediate-tombstone divergence. |
| MEMB-05 | 04-01,02,03,04,05,07,09,10,11,15,19 | Members can invite others if `membersCanInvite` is enabled | SATISFIED | `CanInviteFor` gate + persisted flag + issuer-authority check at redeem; tests pass. |

No orphaned requirements: all five Phase 4 IDs appear in at least one plan's `requirements` field, and REQUIREMENTS.md maps exactly MEMB-01..05 to Phase 4. REQUIREMENTS.md marks all five Complete.

### Anti-Patterns Found

Debt-marker gate: no `TBD`/`FIXME`/`XXX` in any phase-modified C++/Dart/test file (clean). No `sleep_for`/`sleep_until` in membership tests. No OS preprocessor guards in reviewed C++ sources. No hardcoded secrets, `eval`, shell, or raw stdio debug output found.

| File | Line | Pattern | Severity | Impact |
| --- | --- | --- | --- | --- |
| `src/lib/gcs_membership.cpp` | 1602, 1961 | Cross-scope authority laundering (prior CR-01) | RESOLVED | Both raw-read propagation paths now reject a body whose `scope_id` disagrees with the requested scope; byte-level regression passes. Scoped re-review (04-REVIEW.md) confirms 0 Critical. |
| `src/lib/gcs_membership.cpp` | 579-589, 957-960 | Unbounded signed `updated_at_ms`; `+ 1` can overflow and an INT64_MAX stamp freezes the predecessor gate | WARNING | New in the 04-23 scoped re-review (04-REVIEW WR-01). A signature-valid hostile writer can freeze a member key; does not affect the phase goal. |
| `src/lib/gcs_membership.cpp` | 698-725 vs ~1892-1914 | Verifier accepts an admin-authored immediate tombstone of a below-admin target while the command path makes the same removal dormant when 2+ admins | WARNING (DEFERRED by user decision 2026-10-20) | Requires a design decision; does not affect unsigned/hostile peers. See Deferred Items. |
| `src/lib/gcs_membership.cpp` | 1314-1338; `RedeemInvite` key store | `MintInvite` wraps a topic-derived key for child rooms the minter does not hold; redeem overwrites an existing stored key | WARNING | Confidentiality degrades; an existing member's working key can be replaced (prior WR-14). |
| `src/lib/gcs_membership.cpp` | 1752-1770, 1942-1952; 494-497, 1116-1120 | `explicit_leave` is never cleared by a later redeem/authority write | WARNING | A re-invited member is permanently excluded from derived joins (prior WR-15). |
| `src/lib/gcs_membership.cpp` | 1400-1417, 1566-1582 | Invite bearer gate is seam-optional (fails open when unset) | WARNING | A partially wired seam silently removes the bearer-key precondition (prior WR-06). |
| `src/lib/gcs_membership.cpp` | 1986-2059 | Space deletion does not cascade to child rooms; a nested room cannot be deleted alone | WARNING | Orphaned rooms reappear as top-level; nested-room DeleteScope is always denied (prior WR-10). |
| `src/lib/gcs_membership.cpp` | 2168-2171, 2188-2191, 1992 | Entity `pending_action` is not cleared when deletion is activated | WARNING | Stored `deleted && pending_action == DELETE` contradicts the member-path coherence rule (prior IN-07). |
| `src/lib/gcs_membership.cpp` | 641-645 | Unreachable `record.created_by() == address` disjunct in the third-party branch | INFO | Dead code in a security-critical verifier (new IN-01 / prior IN-06). |
| `src/lib/gcs_membership.cpp` | 1479-1485 vs 1923, 1048-1051, 1103, 1799, 1862 | Scope-id shape validation is inconsistent at the API edge | INFO | A `"a/b"` scope id can address a confused key coordinate today; body/key binding and catalog resolution fail closed (new IN-02). |
| `src/lib/gcs_membership.hpp` | 648-663 | The raw-read contract is enforced by prose/caller discipline only | INFO | All five current callers comply; YAGNI governs adding a bound variant (new IN-03). |
| `src/ffi/gcs_core_ffi.cpp` | 1736-1764, 1818-1844, 799-879 | `kRedeem`/`kLeave` do not refresh derived joins or push `RoomList` | WARNING | Invitee cannot open auto-join/standalone rooms until restart; leave takes effect only after restart (prior WR-01). |
| `src/ffi/gcs_core_ffi.cpp` | 752-756 | `ScopeHasRoster` fails open on a scan error | WARNING | A transient read failure downgrades Member+/Admin+ gates to no gate (prior WR-07). |
| `src/ffi/gcs_core_ffi.cpp` | 596-612 | `BuildMemberListEvent` pushes an empty roster on scan failure | WARNING | Read failure renders as "No members yet" and removes role-gated affordances (prior WR-08). |
| `src/ffi/gcs_core_ffi.cpp` | 997-1022 | Failed init paths after the smoke/membership topic joins do not shut down the embedded SDK or clear `g_roomTopics` | WARNING | Resource leak for the process lifetime; a retry can duplicate smoke topics (prior WR-12). |
| `src/ffi/gcs_core_ffi.cpp` | 1542-1555, 1627-1643 | Scope creation is not transactional; a failed creator root leaves an orphan scope | WARNING | The orphan can never acquire an authority root; standalone rooms also retain freshly minted key material (prior WR-13). |
| `src/lib/gcs_entity_store.cpp` | 293-350 | `UpdateSpace` resets `approved_by`/`pending_action` on a full-state rewrite | WARNING | An Admin can cancel a dormant deletion by renaming the space without approval (prior WR-11). |
| `src/app/lib/shell/members_dialog.dart` | 318-319 | Change-role affordance is offered on the viewer's own row | WARNING | C++ always rejects self-targeting; guaranteed error toast (prior WR-09). |
| `src/lib/gcs_crypto.cpp` | 44-252 | Room-key store is process-local | WARNING (DEFERRED) | CR-07 — explicitly deferred by user decision. |
| `src/lib/gcs_crypto.cpp` | 515-521 | `VerifyInviteAuthenticator` accepts any non-empty key length | INFO | Verify contract is weaker than the documented 32-byte invariant (prior IN-05). |
| `src/ffi/gcs_core_ffi.cpp` | 1786, 1812, 1838, 1864, 1891 | All membership failures map to one error | INFO | Causes cannot be distinguished by the user or logs (prior IN-03). |
| `src/lib/gcs_membership.cpp` | 1011-1046, and FFI arms | Every membership mutation pushes `MemberList` twice | INFO | Two identical roster events per action (prior IN-04). |

### Human Verification Required

1. **Invite link end-to-end (visual)** — Generate a space and a room invite; confirm the invite view shows a `gcs://invite/...` link with a Copy action and that copying preserves the URL. Why human: clipboard + rendering.
2. **Scope isolation of the invite view** — Open scope A's invite view, close it, open scope B's; confirm B never shows A's link. Why human: interactive drawer state.
3. **Join flow** — Paste a valid link; confirm Join enables, the dialog closes, and the member appears. Paste an invalid shape; confirm the inline error keeps the dialog open. Why human: interactive Flutter flow.
4. **Post-join room access (review WR-01)** — After redeeming a standalone-room invite and a space invite with `autoJoinRooms`, confirm the joined room(s) are openable and receive messages without restarting the app. Currently expected to FAIL by inspection (see Data-Flow Trace). Why human: multi-cubit live state; confirm the user-visible impact and whether a follow-up plan is warranted.
5. **Role badges and gating** — Confirm five-tier badges, the "You" marker, and role-gated affordances match the UI spec for each tier. Why human: visual/presentation.
6. **Pending-op surface** — With 2+ admins, trigger a removal; confirm the Pending badge and Super Admin Approve affordance activate the op; confirm a target re-redeeming an invite does not clear the badge. Why human: multi-actor UI state.
7. **Two live nodes (production config)** — Mint on A, redeem on B (not joined); confirm B joins as Guest and both rosters converge. Why human: live-node smoke check (automated multinode proof exists).

### Gaps Summary

The single gap from the prior verification is closed.

- **Truth 29 / CR-01 is resolved.** Commit `36a8fd9` added `existing.scope_id() != urlScopeId` in `RedeemInvite` (after the address check, before the tombstone check; line 1602) and `record.scope_id() != scopeId` in `Leave` (before `set_explicit_leave`, signing, and any Put; line 1961). `WriteMemberPreservingAuthority` can therefore no longer derive its Put key from an unverified body that names another scope. `TryReadRawMember`'s deliberately non-binding contract is documented, and every call site is accounted for: `VerifiedRoleFor`/`TryReadMemberVerified` are bound by `VerifyMemberRecord`'s body/key clause (line 528), `OnScopeCreated` treats any occupying record as final, and the two propagation paths now bind explicitly. `WriteMemberAsAuthor`'s other callers construct records from validated scope/address inputs.
- **The regression is non-vacuous.** `CrossScopeRawRecordIsNotPropagatedByLeaveOrRedeem` plants a validly signed B-bodied record under A/alice whose role and timestamps differ from B's baseline; `Leave(A)` and `RedeemInvite(A)` both fail, B's bytes stay identical, A keeps the hostile bytes, and A's roster excludes the joiner. The run log shows both new rejection warnings firing, confirming the exercised path rather than an unrelated rejection.
- **No regressions.** The full C++ suite is 13/13 (209.87 s), the membership unit suite is 52/52 cases, the multinode suite is 9/9, and the Dart suite is 80 passed / 1 skipped. Positive controls for legitimate same-scope leave/redeem (promoted-role preservation, cold-node leave, dormant-removal survival) remain in the suite.
- **No open blockers.** All 30 truths are verified; no required artifact is missing or a stub; all plan-declared key links are wired. The remaining review warnings (WR-01, WR-06..WR-15, IN-01..IN-07) are recorded above and do not breach the phase's roadmap success criteria. The kRedeem/kLeave derived-join refresh is a non-must-have integration warning surfaced for human verification. CR-07 and the wire-level admin-immediate-tombstone dormancy divergence are explicit user deferrals, not open gaps.

The phase goal is achieved; the remaining items require human confirmation of the interactive/visual flows (status `human_needed`).

**Suggested next step:** complete the human verification items, then proceed. If desired, schedule a separate plan for review WR-01 (post-join room access) and the 04-REVIEW WR-01 timestamp bound.

**Bookkeeping note (not a gap):** the 2026-10-20 user decision deferring the wire-level admin-immediate-tombstone dormancy divergence is recorded in `04-23-PLAN.md` and `04-23-SUMMARY.md`, but `STATE.md`'s Deferred Items table still reads `(none)`.

---

_Verified: 2026-10-21T18:03:05Z_
_Verifier: Claude (gsd-verifier)_
