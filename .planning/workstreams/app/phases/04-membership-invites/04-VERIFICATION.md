---
phase: 04-membership-invites
verified: 2026-10-19T00:34:00Z
status: gaps_found
score: 28/30 must-haves verified
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 26/30
  gaps_closed:
    - "Truth 11 — dormant scope deletion now reaches the EntityStore catalog and Approve activates it (CR-06)"
    - "Truth 30 — a tombstoned wallet can no longer rejoin via a stale invite (CR-02)"
    - "Truth 5 sub-defects — redeem preserves a live member's role/pending/approved state (CR-01) and AdminCount fails closed (CR-05)"
    - "Truth 29 sub-defects — InviteRecord is issuer-signed and authority-checked (CR-08); CanChangeRole enforces concrete roles and strict outrank (CR-03/CR-04)"
  gaps_remaining:
    - "Truth 5 — the D-07 approval gate is command-level only: unauthenticated MemberRecords let any membership-topic writer publish a tombstone/role record directly, bypassing approval"
    - "Truth 29 — MemberRecord carries no issuer/signature and is trusted for every authorization decision; CanChangeRole's creator exception is role-based; Approve applies an unvalidated pending_role"
  regressions: []
gaps:
  - truth: "The five-tier role model governs what actions each participant can take — no self-granted escalation, and redeem does not rewrite stored role/pending state"
    status: failed
    reason: >-
      The invite-grant path is now authenticated end to end (created_by +
      issuer_signature, scoped URL, exact-key lookup, Guest cap, issuer
      authority), and redeem preserves live state and rejects tombstones.
      However, MemberRecord itself remains an unauthenticated value on the
      permissionless membership replication topic: it has no created_by or
      signature, TryReadMember/MembersFor return stored bytes without checking
      them against the key or an issuer, LocalRoleFor returns record.role()
      verbatim, and OnMemberArrived pushes whatever arrives. Any peer can
      publish gcs/members/<scope>/<attacker> with role = ROLE_SUPER_ADMIN and,
      on its own node, CanDelete/CanRemoveMember/CanChangeRole authorize from
      that record. The D-03 authority root is therefore not enforceable against
      a peer that can write the roster. Two command-path defects compound this:
      CanChangeRole's creator exception tests the resolved role rather than
      IsSuperAdmin (a forged SUPER_ADMIN record receives the exception), and
      Approve copies pending_role into role without re-validating it (a
      forged pending ROLE_SUPER_ADMIN/ROLE_UNSPECIFIED is applied).
    artifacts:
      - path: "src/proto/gcs_chat.proto"
        issue: "MemberRecord (236-250) has no created_by/record_signature fields; only InviteRecord (270-292) is signed."
      - path: "src/lib/gcs_membership.cpp"
        issue: "TryReadMember (371-390), MembersFor (495-527), and LocalRoleFor (529-537) trust stored bytes; OnMemberArrived (1536-1560) accepts any value on the permissionless topic and pushes it."
      - path: "src/lib/gcs_membership.cpp"
        issue: "CanChangeRole (618-621) grants the creator exception to any record resolving to ROLE_SUPER_ADMIN instead of using IsSuperAdmin."
      - path: "src/lib/gcs_membership.cpp"
        issue: "Approve (1427-1432) applies record.pending_role() without re-running the concrete-role/creator-demotion guards."
    missing:
      - "Append created_by and record_signature to MemberRecord (append-only), sign every WriteMember path, and verify on OnMemberArrived/TryReadMember/MembersFor — reject unverifiable records from both the roster and every authorization decision."
      - "Constrain what a self-signed member record may assert: no tier above the invite-derived Guest, and no tombstone/pending transition; role/removal authority must chain from the creator's signed record."
      - "Use IsSuperAdmin(scopeId, m_sender) for CanChangeRole's creator exception (WR-02)."
      - "Re-validate pending_role in Approve: concrete role only, never ROLE_SUPER_ADMIN, never a creator demotion (WR-03)."
      - "Add a wire-level regression test that injects a SUPER_ADMIN member record and asserts it is excluded from MembersFor and cannot drive DeleteScope/RemoveMember/ChangeRole."

  - truth: "When 2+ admins exist, destructive actions require Super Admin approval (SC5 / MEMB-04)"
    status: failed
    reason: >-
      The three destructive command paths now implement dormancy correctly:
      RemoveMember and ChangeRole(admin demotion) and DeleteScope write
      pending state with empty approved_by, AdminCount fails closed, DeleteScope
      routes through catalog mutators, and Approve's entity branch is reachable
      and tested. The prior CR-01/CR-02/CR-05/CR-06 bypasses are closed.
      The approval gate is nevertheless not enforceable against a hostile peer:
      because MemberRecord is unauthenticated (same root as truth 29), any peer
      can publish a tombstone or a demoting role record for an arbitrary wallet
      directly on the membership replication topic, performing a removal or
      demotion that was never approved. A peer can also self-assert
      ROLE_SUPER_ADMIN so CanDelete/CanRemoveMember return true. Legitimately,
      ChangeRole's non-demotion branch also clears any pending REMOVE without
      approval (WR-05), so a pending destructive op can be silently cancelled by
      a second admin's role change.
    artifacts:
      - path: "src/proto/gcs_chat.proto"
        issue: "MemberRecord is unauthenticated, so pending/tombstone state can be written directly by any topic writer."
      - path: "src/lib/gcs_membership.cpp"
        issue: "ChangeRole (1235-1241) unconditionally clears pending_action/pending_role, cancelling a pending REMOVE without Approve."
      - path: "src/lib/gcs_membership.cpp"
        issue: "RemoveMember (1276-1280) sets pending_action but leaves a stale pending_role."
    missing:
      - "Authenticate member records (truth 29 fix), so pending/tombstone transitions cannot be injected on the wire."
      - "Define the pending-state transition explicitly: reject ChangeRole while PENDING_ACTION_REMOVE is set, or add a creator-only cancel that records approved_by."
      - "Clear pending_role when RemoveMember writes PENDING_ACTION_REMOVE."

deferred:
  - truth: "Room keys survive restart and reach a second application process (CR-07)"
    addressed_in: "Explicitly deferred by user decision — no later phase assigned"
    evidence: "ROADMAP.md Phase 4: 'Deferred (recorded, not in this gap-closure set): CR-07 — room keys live in a process-local store, so a second application process (or a restart without the invite material) cannot decrypt a room's key material.' Not counted as an open gap for this phase."
---

# Phase 4: Membership & Invites Verification Report

**Phase Goal:** Users can invite others via capability tokens, and the five-tier role model (Super Admin → Guest) governs what actions each participant can take.
**Verified:** 2026-10-19T00:34:00Z
**Status:** gaps_found
**Re-verification:** Yes — after gap-closure waves 7-11 (plans 04-13 .. 04-18)

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
| --- | --- | --- | --- |
| 1 | SC1: User can generate a `gcs://invite/...` link for a space or room | ✓ VERIFIED | `MintInvite` (gcs_membership.cpp:683-869) emits `gcs://invite/{space\|room}/<scopeId>/<inviteId>?key=<IK>` and fails closed without the issuer seam (703-709); signed before Put (842-858). `InviteMintsSpaceInviteLinkWithCapabilityShape` and scoped-URL FFI tests pass. |
| 2 | SC2: Recipient can use the invite to join and appears in the member list | ✓ VERIFIED | `SpaceInviteConvergesWithoutTheRecipientJoiningTheSpace` (multinode:560) proves a cold node redeems on `kMembershipTopic` and both rosters show 2 members. Warning: the FFI redeem arm does not refresh derived joins or push `RoomList` (WR-01), so room access lag until restart/unrelated command. |
| 3 | SC3: Each participant has a visible five-tier role | ✓ VERIFIED | Five-tier `Role` enum (proto:208-214); `MemberList` carries roles + `self_address`; Dart 80 passed / 1 skipped. Caveat: the roster's trustworthiness is tracked under truth 29. |
| 4 | SC4: Super Admin (creator) cannot be demoted by other admins | ✓ VERIFIED | `CanChangeRole` rejects any non-SUPER_ADMIN change targeting the creator (gcs_membership.cpp:585-591); `CanRemoveMember` rejects the creator (650-653); `CreatorDemotionIsRejected` and `ChangeRoleRejectsDemotingCreator` pass. Caveat: the unauthenticated member record can be overwritten on the wire (truth 29 blocker). |
| 5 | SC5: When 2+ admins exist, destructive actions require Super Admin approval | ✗ FAILED | Dormancy/Approve now work for all three commands (RemoveMember 1269-1290, ChangeRole 1217-1241, DeleteScope 1326-1365, Approve 1397-1530; `DeleteScopeDormantThenApproved` passes) and AdminCount fails closed (399-432). Remaining: unauthenticated MemberRecords let any topic writer publish a tombstone/role record directly, bypassing approval; ChangeRole also clears a pending REMOVE (WR-05). |
| 6 | Proto carries Role/MemberRecord/MemberList/InviteLink/InviteRecord + creator/members_can_invite + 7 command / 2 event arms, append-only | ✓ VERIFIED | proto:203-318; command arms 6-12 and event arms 7-8 appended with no renumbering (proto:57-68, 104-111); `InviteRecord.created_by=6`, `issuer_signature=7` appended. |
| 7 | 32-byte room key AES-256-GCM wrap/unwrap; per-room key store; D-08 envelope/HKDF unchanged | ✓ VERIFIED | `test_gcs_crypto` passes; `WrapRoomKey`/`UnwrapRoomKey`/`RoomKeyFor` unchanged. Warning: store remains process-local (CR-07, deferred). |
| 8 | MintInvite persists ONE invite record, writes NO member record, wraps keys under IK | ✓ VERIFIED | gcs_membership.cpp:683-869; `MintPersistsInviteRecordAndWritesNoMember` passes. |
| 9 | RedeemInvite writes the redeemer's wallet-keyed member record and stores the key; multi-use | ✓ VERIFIED | gcs_membership.cpp:1140-1194; `RedeemWritesMemberStoresKeyAndIsMultiUse` passes. Merge semantics: existing live member keeps stored state (1167-1184). |
| 10 | ChangeRole rejects demotion targeting the scope creator | ✓ VERIFIED | gcs_membership.cpp:585-591; `CreatorDemotionIsRejected` passes. |
| 11 | RemoveMember/ChangeRole(demotion)/DeleteScope dormant when 2+ admins; Approve activates | ✓ VERIFIED | **Gap closed (04-14/CR-06).** DeleteScope routes through `ApplySpaceState`/`ApplyRoomState` (1360, 1385), which Put outside `m_catalogMutex` then publish the stored record (gcs_entity_store.cpp:352-389); Approve's entity branch re-reads authoritative bytes and applies (1477-1530). `DeleteScopeDormantThenApproved` + `Apply*StatePublishesTombstoneToCatalogAndStore` pass. |
| 12 | MintInvite gated by author role + members_can_invite (MEMB-05) | ✓ VERIFIED | `CanInvite`/`CanInviteFor` (549-574); `membersCanInviteGatesMemberMinting` passes. Caveat: authority source is an unauthenticated member record (truth 29). |
| 13 | MembersFor prefix-scans, skips tombstones, stamps self_address | ✓ VERIFIED | gcs_membership.cpp:495-527; unit + FFI tests. Warning: no body-vs-key validation (WR-04). |
| 14 | Creator + members_can_invite stamped; UpdateSpace preserves creator | ✓ VERIFIED | gcs_entity_store.cpp:200-253; entity suite passes. |
| 15 | gcs_init constructs Membership + heal callback so remote membership converges | ✓ VERIFIED | gcs_core_ffi.cpp:1001-1022 (topic registration), 1279-1286 (construction); multinode convergence tests pass. |
| 16 | The seven command arms validate, guard, delegate, and push | ✓ VERIFIED | gcs_core_ffi.cpp:1720-1900; FFI suite passes. |
| 17 | P5 guards on send_text/create_room/update_space; create paths stamp creator/members_can_invite | ✓ VERIFIED | gcs_core_ffi.cpp:1472-1484, 1587-1600, 1668-1680 (`ScopeHasRoster`/`RoleAtLeast`). Warning: ScopeHasRoster fails open (WR-07). |
| 18 | Derived joins respect explicit_leave (D-02) | ✓ VERIFIED | `HasExplicitLeave` (539-547); membership tests cover both branches. Warning: Leave does not refresh derived joins (WR-01). |
| 19 | Dart MembersCubit + dispatch arms + self flag | ✓ VERIFIED | members_cubit.dart; session_cubit.dart:525; Dart suite green. |
| 20 | UI surfaces: members dialog, invite view, join dialog, rail affordances, pending badges | ✓ VERIFIED | Dart suite green; visual checks remain human. Warnings WR-09 (self change-role), WR-01 (post-join room access). |
| 21 | Concurrent membership and entity-mutating commands do not corrupt shared state (prior CR-01) | ✓ VERIFIED | `m_catalogMutex` guards all catalog readers/writers; `ConcurrentCatalogReadsAndRoomCreatesDoNotRace` passes. |
| 22 | Rank-strict removal guard: Moderator cannot remove Admin; Admin cannot remove non-creator Super Admin (WR-02) | ✓ VERIFIED | `CanRemoveMember` strict outrank (642-670); `ModeratorCannotRemoveAdminAndAdminCannotRemoveSuperAdmin` passes. |
| 23 | Demoting a non-creator Super Admin with 2+ admins writes a dormant role change (WR-03) | ✓ VERIFIED | Rank-based `isAdminDemotion` (1226-1234); `SuperAdminDemotionIsDormantWhenTwoAdmins` passes. |
| 24 | The bearer key gates every redeem; no key/no authenticator writes no member record (WR-04) | ✓ VERIFIED | Unconditional `verifyAuth` before any write (987-1005); `ForgedOrKeylessSpaceInviteWritesNoMemberRecord` passes. Warning: gate is seam-optional (WR-06). |
| 25 | Tombstone arrival pushes the filtered roster; remote removal converges (WR-05) | ✓ VERIFIED | `OnMemberArrived` pushes (1536-1560); `RemovedMemberReceivesTheConvergedRoster` passes. |
| 26 | Membership replicates on a registered well-known topic; an unjoined recipient redeems (WR-06) | ✓ VERIFIED | `kMembershipTopic` registered listen-then-broadcast at init (gcs_core_ffi.cpp:1015-1022); multinode suite passes. |
| 27 | Teardown releases Membership before the session/catalog it borrows (WR-01 prior) | ✓ VERIFIED | gcs_core_ffi.cpp:358-425: globals evicted, `g_inviteAccount` reset, `membership.reset()` before `session->Shutdown()` and `entities.reset()`. |
| 28 | Invite view renders the link for its own scope; clear-on-close (WR-07) | ✓ VERIFIED | `_pendingInviteLinks[scopeId]`/`pendingInviteLinkFor`/`clearPendingInviteLink` (session_cubit.dart:164-185, 536-540); widget tests pass. |
| 29 | The role model governs what actions each participant can take — no self-granted escalation, and redeem does not rewrite stored role/pending state | ✗ FAILED | Invite grants are now issuer-signed and authority-checked (1021-1082), redeem merges live state and rejects tombstones (1007-1019, 1158-1194), and CanChangeRole enforces concrete roles + strict outrank (592-638). Remaining blocker: MemberRecord is unauthenticated and drives every authorization decision (truth-29 gap above); WR-02 role-based creator exception; WR-03 unvalidated `pending_role` in Approve. |
| 30 | A removed (tombstoned) member cannot rejoin via a stale invite | ✓ VERIFIED | **Gap closed (04-13/CR-02).** `RedeemInvite` rejects `existing.deleted()` after the authenticator gate and before the unwrap loop (1007-1019); `TombstonedMemberCannotRedeemStaleInvite` asserts stored bytes and scan count are unchanged. |

**Score:** 28/30 truths verified

### Deferred Items

| # | Item | Addressed In | Evidence |
| --- | --- | --- | --- |
| 1 | CR-07 — room keys live in a process-local store; they do not survive restart or reach another process | Deferred by user decision; no later phase assigned | ROADMAP.md Phase 4 deferral note (Waves 5-11 do not address it) |

### Required Artifacts

| Artifact | Expected | Status | Details |
| --- | --- | --- | --- |
| `src/proto/gcs_chat.proto` | Append-only membership wire contract | ✓ VERIFIED | Role/PendingAction/WrappedRoomKey/MemberRecord/MemberList/InviteLink/InviteRecord + creator/members_can_invite + 7 command / 2 event arms. `InviteRecord` gained `created_by=6`/`issuer_signature=7`; `MemberRecord` still has no issuer/signature (truth 29). |
| `src/lib/gcs_crypto.hpp/.cpp` | Key-wrap + per-room key store + ikm swap + invite authenticator + hex codec | ✓ VERIFIED | Wrap/Unwrap/RoomKeyFor/DeriveRoomKey/MintInviteAuthenticator/VerifyInviteAuthenticator/HexEncode/HexDecode present; crypto suite passes. Store is process-local (CR-07 deferred). |
| `src/lib/gcs_membership.hpp/.cpp` | Membership component (roster, roles, invites, approval) | ⚠️ PARTIAL | Redeem merge/tombstone rejection, issuer signing/verification, scoped lookup, role cap, AdminCount fail-closed, entity-mutator routing all present and tested. Member records remain unauthenticated; WR-02/WR-03/WR-05 remain (truths 5/29). |
| `src/lib/gcs_entity_store.hpp/.cpp` | Creator/members_can_invite stamping + synchronized catalog + deletion mutators | ✓ VERIFIED | `ApplySpaceState`/`ApplyRoomState` Put outside `m_catalogMutex` and publish the stored record under it (352-389); entity tests pass. |
| `src/ffi/gcs_core_ffi.cpp` | Membership construction + heal callback + 7 arms + guards + lifecycle + issuer seam | ✓ VERIFIED | Membership topic at init (1015-1022); account-backed sign/verifyIssuer with fail-closed posture (1195-1278); teardown order correct (358-425). Warnings WR-01/WR-07/WR-08. |
| `src/app/lib/cubits/members_cubit.dart` | Per-scope roster state + MemberInfo | ✓ VERIFIED | Dart tests pass. |
| `src/app/lib/cubits/session_cubit.dart` | Scope-keyed pending invite link | ✓ VERIFIED | `_pendingInviteLinks`, `pendingInviteLinkFor`, `clearPendingInviteLink`. |
| `src/app/lib/shell/members_dialog.dart` | Member list + scope-keyed invite view + confirmations | ✓ VERIFIED | Widget tests pass; WR-09 self change-role warning. |
| `src/app/lib/shell/join_dialog.dart` | Paste-invite redeem flow | ✓ VERIFIED | `kInviteLinkPattern` accepts two path segments (`space\|room/<scope>/<invite>?key=`); tests pass. |
| `src/app/lib/generated/proto/gcs_chat.pb.dart` | Regenerated Dart bindings | ✓ VERIFIED | Exposes `createdBy`/`issuerSignature` (2258-2379), `InviteLink.scopeId`. |
| `test/test_gcs_invite_helpers.hpp` | Canonical issuer-payload builder shared by unit and multinode suites | ✓ VERIFIED | `BuildInviteIssuerPayload` used by both suites. |

### Key Link Verification

| From | To | Via | Status | Details |
| --- | --- | --- | --- | --- |
| `MintInvite` | `KeyWrapSeam::sign` | issuer signature over canonical payload before Put | WIRED | gcs_membership.cpp:842-858. |
| `RedeemInvite` | `KeyWrapSeam::verifyIssuer` | verify against `created_by` before any member write | WIRED | gcs_membership.cpp:1021-1048. |
| `RedeemInvite` | `CanInviteFor` | issuer authority when the scope resolves locally | WIRED | gcs_membership.cpp:1073-1082. |
| `RedeemInvite` | `TryReadMember` | read-before-write merge + tombstone rejection | WIRED | gcs_membership.cpp:1011-1019, 1167-1189. |
| `DeleteScope` | `EntityStore::ApplySpaceState`/`ApplyRoomState` | catalog-synchronized destructive write | WIRED | gcs_membership.cpp:1360, 1385. |
| `Approve` (entity branch) | `EntityStore::ApplySpaceState`/`ApplyRoomState` | re-read authoritative bytes, apply tombstone, push | WIRED | gcs_membership.cpp:1477-1530. |
| `AdminCount` | `CoreSession::QueryKeyValues` | scan failure propagated, never 0 | WIRED | gcs_membership.cpp:399-432; three propagating call sites. |
| `CanChangeRole` | `TryReadMember` + `PrivilegeRank` | target's current role, strict outrank | WIRED | gcs_membership.cpp:607-628. |
| `MemberRecord` | authorization source | signed issuer/record binding | NOT_WIRED | No signature fields (proto:236-250); `LocalRoleFor` trusts stored bytes (529-537) — root of truth 29/5. |
| `kRedeem` / `kLeave` | `RefreshDerivedJoins` + `BuildRoomListEvent` | derived-join refresh after membership change | NOT_WIRED | `RefreshDerivedJoins` called only at init/create_room/update_space (gcs_core_ffi.cpp:1288, 1648, 1696) — WR-01. |
| `CanChangeRole` creator exception | `IsSuperAdmin` | creator identity, not resolved role | NOT_WIRED | gcs_membership.cpp:618-621 uses `author == ROLE_SUPER_ADMIN` — WR-02. |
| `Approve` | concrete-role guard | re-validate `pending_role` before applying | NOT_WIRED | gcs_membership.cpp:1427-1432 — WR-03. |
| `gcs_init` | `Membership` + `kMembershipTopic` | construction + listen-then-broadcast | WIRED | gcs_core_ffi.cpp:1015-1022, 1279-1286. |
| `gcs_shutdown` | `g_membership`/`g_inviteAccount` | evict/reset before session shutdown | WIRED | gcs_core_ffi.cpp:358-425. |
| `session_cubit.dart` | `MembersCubit.setMembers` / scope-keyed link | dispatch arms | WIRED | session_cubit.dart:520-540. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
| --- | --- | --- | --- | --- |
| members_dialog roster | `members.state.byScope[scopeId]` | Pushed `MemberList` → `Membership::MembersFor` prefix scan over CRDT member records | Yes (but records are unauthenticated — truth 29) | ⚠️ FLOWING / UNTRUSTED SOURCE |
| members_dialog invite view | `session.pendingInviteLinkFor(scopeId)` | Pushed `InviteLink` from `MintInvite`; scope stamped by C++ | Yes | ✓ FLOWING |
| room_rail pending deletion badge | `SpaceRecord.pending_action` / `deleted` | Pushed `SpaceTree` → `EntityStore::Spaces()` after `ApplySpaceState` | Yes — **gap closed** (04-14) | ✓ FLOWING |
| members_dialog pending removal badge | `MemberRecord.pending_action` | Pushed `MemberList` → member records; preserved across redeem | Yes — **gap closed** (04-13) | ✓ FLOWING |
| post-redeem room access | `RoomList` / derived-join topics | FFI `kRedeem` pushes only `SpaceTree`; no `RefreshDerivedJoins` | No | ✗ DISCONNECTED (WR-01) |
| post-leave derived joins | `explicit_leave` → `DerivedJoinedTopics` | FFI `kLeave` writes the flag but never recomputes/pushes | No | ✗ DISCONNECTED (WR-01) |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
| --- | --- | --- | --- |
| Full C++ suite | `ctest --test-dir build/OSX/Debug --output-on-failure` | 13/13 passed (190.47s) | ✓ PASS |
| Membership, multinode, entities, crypto, FFI | `ctest -R "test_gcs_(membership|membership_multinode|entities|crypto|ffi)$"` | 5/5 passed (114.53s) | ✓ PASS |
| Dart app suite | `cd src/app && flutter test` | 80 passed, 1 skipped, 0 failed | ✓ PASS |
| Dormant removal survives redeem | `test_gcs_membership --gtest_filter=*DormantRemovalSurvivesRedeem*` (within suite) | Passed | ✓ PASS |
| Tombstoned wallet rejected | `*TombstonedMemberCannotRedeemStaleInvite*` | Passed | ✓ PASS |
| Dormant scope delete → approve | `*DeleteScopeDormantThenApproved*` | Passed | ✓ PASS |
| AdminCount fail-closed | `*AdminCountScanFailurePropagatesAndFailsClosed*` | Passed | ✓ PASS |
| Forged issuer on the wire | `test_gcs_membership_multinode` (`*ForgedIssuerSignatureCannotGrantARoleOnTheWire*`) | Passed | ✓ PASS |
| Real-signer cross-node redeem | `*SpaceInviteConvergesWithoutTheRecipientJoiningTheSpace*` | Passed | ✓ PASS |
| Role authority matrix | `*AdminCannotInitiateSuperAdminDemotion*`, `*ChangeRoleRejectsSuperAdminGrantAndUnspecifiedRole*` | Passed | ✓ PASS |

### Probe Execution

No probe scripts are declared for this phase and none exist. Step 7c: SKIPPED (no probes).

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
| --- | --- | --- | --- | --- |
| MEMB-01 | 04-01,02,03,05,07,09,10,11,12,13,15,16,18 | User can invite others via capability token (`gcs://invite/...`) | ✓ SATISFIED | Scoped, bearer-authenticated, issuer-signed invite minted at 683-869; redeem verifies signature/scope/kind/role cap/authority (938-1136); cross-node positive and forgery-negative multinode proofs pass. |
| MEMB-02 | 04-01,02,03,05,06,07,08,09,12,15,17,18 | Space/room has roles: Super Admin, Admin, Moderator, Member, Guest | ✓ SATISFIED | Five-tier enum, roster pushes, UI labels/badges; enforcement caveat tracked under truth 29. |
| MEMB-03 | 04-01,03,04,05,09,12,17 | Super Admin (creator) cannot be demoted by other admins | ✓ SATISFIED | ChangeRole creator guard + strict outrank (585-638); `CreatorDemotionIsRejected`, `AdminCannotInitiateSuperAdminDemotion` pass. Caveat: roster writes are unauthenticated (truth 29 blocker). |
| MEMB-04 | 04-01,03,05,06,07,08,09,12,13,14 | Destructive actions require Super Admin approval when 2+ admins exist | ✗ BLOCKED | Command paths and catalog routing verified, but unauthenticated member records allow direct tombstone/role injection bypassing approval; ChangeRole can clear a pending REMOVE (WR-05). |
| MEMB-05 | 04-01,02,03,04,05,07,09,10,11,15 | Members can invite others if `membersCanInvite` is enabled | ✓ SATISFIED | `CanInviteFor` gate + persisted flag + issuer-authority check at redeem; tests pass. Caveat: the authority source is an unauthenticated member record (truth 29). |

No orphaned requirements: all five Phase 4 IDs appear in at least one plan's `requirements` field, and REQUIREMENTS.md maps exactly MEMB-01..05 to Phase 4.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
| --- | --- | --- | --- | --- |
| `src/proto/gcs_chat.proto`; `src/lib/gcs_membership.cpp` | 236-250; 371-390, 495-527, 529-537, 1536-1560 | `MemberRecord` has no issuer/signature; roster and every role decision trust stored bytes from a permissionless topic | 🛑 BLOCKER | Self-asserted `ROLE_SUPER_ADMIN` and direct tombstone/role injection; defeats the role model and the SC5 approval gate (truths 29/5). |
| `src/lib/gcs_membership.cpp` | 618-621 | Creator exception tests resolved role, not `IsSuperAdmin` | ⚠️ WARNING | A forged SUPER_ADMIN record receives the creator's unrestricted role-change authority (WR-02). |
| `src/lib/gcs_membership.cpp` | 1427-1432 | `Approve` applies `pending_role` without re-validation | ⚠️ WARNING | Attacker-controlled pending value can grant SUPER_ADMIN/UNSPECIFIED on approval (WR-03). |
| `src/lib/gcs_membership.cpp` | 1235-1241, 1276-1280 | Pending-state transitions not reconciled | ⚠️ WARNING | A role change cancels a pending REMOVE without approval; RemoveMember leaves a stale `pending_role` (WR-05). |
| `src/lib/gcs_membership.cpp` | 371-390, 434-456, 495-527 | Member write/read never validates body-vs-key | ⚠️ WARNING | A body/key-mismatched record can redirect a write to another scope/member key (WR-04). |
| `src/ffi/gcs_core_ffi.cpp` | 1736-1763, 1818-1843 | `kRedeem`/`kLeave` do not refresh derived joins or push `RoomList` | ⚠️ WARNING | Invitee cannot open auto-join/standalone rooms until restart; leave takes effect only after restart (WR-01). |
| `src/ffi/gcs_core_ffi.cpp` | 752-756 | `ScopeHasRoster` fails open | ⚠️ WARNING | A scan error downgrades Member+/Admin+ gates to no gate (WR-07). |
| `src/ffi/gcs_core_ffi.cpp` | 596-611 | `BuildMemberListEvent` pushes an empty roster on scan failure | ⚠️ WARNING | Read failure renders as "No members yet" and removes role-gated affordances (WR-08). |
| `src/lib/gcs_membership.cpp` | 823-840, 987-1005 | Invite bearer gate is seam-optional | ⚠️ WARNING | A partially wired seam silently produces/accepts ungated invites (WR-06). |
| `src/lib/gcs_membership.cpp` | 1339-1391 | Space deletion does not cascade to child rooms; nested rooms cannot be deleted | ⚠️ WARNING | Orphaned rooms reappear as top-level; nested-room DeleteScope always denied (WR-10). |
| `src/app/lib/shell/members_dialog.dart` | 318-324 | Change-role offered on the viewer's own row | ⚠️ WARNING | C++ always rejects self-targeting; guaranteed error toast (WR-09). |
| `src/lib/gcs_crypto.cpp` | 44-252 | Room-key store is process-local | ⚠️ WARNING | CR-07 — explicitly deferred by user decision. |

Debt-marker gate: no `TBD`/`FIXME`/`XXX` in any file modified by this phase (clean). No `sleep_for` in tests; no OS preprocessor guards in reviewed C++ sources.

### Human Verification Required

The following remain after automated checks (re-check after the gaps close):

1. **Invite link end-to-end (visual)** — Generate a space and a room invite; confirm the invite view shows a `gcs://invite/...` link with a Copy action and that copying preserves the URL. Why human: clipboard + rendering.
2. **Scope isolation of the invite view** — Open scope A's invite view, close it, open scope B's; confirm B never shows A's link. Why human: interactive drawer state.
3. **Join flow** — Paste a valid link; confirm Join enables, the dialog closes, and the member appears. Paste an invalid shape; confirm the inline error keeps the dialog open. Why human: interactive Flutter flow.
4. **Post-join room access (WR-01)** — After redeeming a standalone-room invite and a space invite with `autoJoinRooms`, confirm the joined room(s) are openable and receive messages without restarting the app. Expected after fix: the RoomList refresh makes them selectable immediately. Why human: multi-cubit live state.
5. **Role badges and gating** — Confirm five-tier badges, the "You" marker, and role-gated affordances match the UI spec for each tier. Why human: visual/presentation.
6. **Pending-op surface** — With 2+ admins, trigger a removal; confirm the Pending badge and Super-Admin Approve affordance activate the op; confirm a target re-redeeming an invite does not clear the badge. Why human: multi-actor UI state.
7. **Two live nodes (production config)** — Mint on A, redeem on B (not joined); confirm B joins as Guest and both rosters converge. Why human: live-node smoke check (automated multinode proof exists).

### Gaps Summary

Wave 7-11 closed the concrete defects from the previous verification: redeem now merges into an existing live member record and rejects tombstones before any side effect (CR-01/CR-02); `AdminCount` fails closed (CR-05); `DeleteScope`/`Approve` route through catalog-synchronized entity mutators, making dormant scope deletion reachable (CR-06); invites are issuer-signed with a scoped URL, exact-key lookup, Guest role cap, and issuer-authority check (CR-08); `CanChangeRole` enforces concrete roles and strict outrank (CR-03/CR-04). The full C++ suite (13/13), targeted suites (5/5), and the Dart suite (80 passed / 1 skipped) are green.

Two goal-level truths remain failed, and they share one root cause plus two command-path defects:

- **Member records are unauthenticated (BLOCKER, truth 29).** `InviteRecord` was signed, but `MemberRecord` still has no issuer or signature. Every authorization decision (`LocalRoleFor` → `CanDelete`/`CanRemoveMember`/`CanChangeRole`) and every pushed roster is derived from values any peer can publish on the membership replication topic. A peer can self-assert `ROLE_SUPER_ADMIN` and, separately, publish a tombstone or demoting role for any wallet. `CanChangeRole`'s creator exception also tests the role instead of `IsSuperAdmin` (WR-02), and `Approve` applies an unvalidated `pending_role` (WR-03).
- **The SC5 approval gate is not enforceable (BLOCKER, truth 5).** The three destructive commands now implement dormancy and approval correctly, but the unauthenticated-record bypass above lets a peer perform a removal/demotion directly without approval. A legitimate path also remains: a role change clears a pending REMOVE without approval (WR-05).

The remaining review warnings (WR-01, WR-04, WR-06..WR-10) are correctness/robustness gaps; WR-01 is the most user-visible (post-join room access). CR-07 is explicitly deferred and is not counted as an open gap.

**Suggested next step:** run `/gsd:plan-phase --gaps` for a gap-closure wave that (1) appends `created_by`/`record_signature` to `MemberRecord`, signs every member write, verifies on arrival/read, and constrains self-signed records to the Guest invite grant with no tombstone/pending transitions; (2) makes the creator exception identity-based and re-validates `pending_role` in `Approve`; (3) defines the pending-state transition so `ChangeRole` cannot silently cancel a pending REMOVE; and (4) refreshes derived joins and pushes `RoomList` on redeem/leave. If the project instead intends to treat the CRDT topic as trusted for this milestone, record an override in the frontmatter with the must-have text, rationale, and approver.

---

_Verified: 2026-10-19T00:34:00Z_
_Verifier: Claude (gsd-verifier)_
