---
phase: 04-membership-invites
verified: 2026-10-12T19:30:00Z
status: gaps_found
score: 16/21 must-haves verified
overrides_applied: 0
gaps:
  - truth: "Recipient can use the invite to join and appears in the member list (space scope)"
    status: failed
    reason: >-
      Space-scope member and invite records are written with broadcast topics
      `{gcs/chat/<spaceId>}` (Membership::ScopeTopics), but no node ever
      registers a space id as a listen/broadcast topic. Only ROOM topics are
      registered (join_topic, derived joins) and smoke topics. Consequently an
      invite minted for a SPACE never leaves the minting node, so a recipient on
      any other node cannot locate the InviteRecord and RedeemInvite fails with
      "invite not found". Space rosters likewise never converge. Room-scope
      cross-node redeem only works because the multinode fixture manually
      pre-subscribes both nodes (test_gcs_membership_multinode.cpp:326-333); no
      production path subscribes a recipient to a room topic it has not joined.
      Deliverable gap, not a documentation nit (REVIEW WR-06).
    artifacts:
      - path: "src/lib/gcs_membership.cpp"
        issue: "ScopeTopics (lines 53-67) returns gcs/chat/<scopeId> for every scope; a space id has no registered topic."
      - path: "src/ffi/gcs_core_ffi.cpp"
        issue: "create_space (around line 1371) stamps creator/roster but never registers a replication topic for the space id; nothing subscribes gcs/chat/<spaceId>."
      - path: "test/test_gcs_membership_multinode.cpp"
        issue: "Only ROOM scope convergence is tested; the fixture pre-registers the room topic on both nodes, masking the missing production subscription."
    missing:
      - "A registered replication channel for space scopes, wired into ScopeTopics (e.g. subscribe a members topic on space creation)."
      - "A convergence test that exercises a SPACE invite end to end across two nodes without manual topic registration."

  - truth: "Concurrent membership and entity-mutating commands do not corrupt shared state"
    status: failed
    reason: >-
      (CR-01, Critical) The Phase 4 FFI arms call Membership mutators inside the
      unlocked in-flight window (g_mutex released) because the mutators push
      through the sink. But every mutator calls Membership::ResolveScope, which
      iterates EntityStore::Spaces()/Rooms() — copy-constructing the private
      std::maps — with no synchronization. Meanwhile kCreateSpace / kCreateRoom /
      kUpdateSpace call CreateSpace/CreateRoom/UpdateSpace, which insert/modify
      those same std::maps under g_mutex. A concurrent create_room and
      change_role therefore reads one std::map while another thread writes it:
      undefined behaviour (not a benign race). A third participant, the CRDT
      receive callback, also reads the catalog under g_mutex.
    artifacts:
      - path: "src/ffi/gcs_core_ffi.cpp"
        issue: "Unlock sites at 1392, 1570, 1600, 1626, 1652, 1679, 1705, 1732 release g_mutex around mutators that read the entity catalog; writers hold g_mutex at 1371-1373, 1443-1444, 1535."
      - path: "src/lib/gcs_membership.cpp"
        issue: "ResolveScope (lines 306-333) iterates m_entities.Spaces()/Rooms() with no lock."
    missing:
      - "Snapshot the scope resolution under g_mutex before unlocking (or make EntityStore::Spaces()/Rooms() lock internally and return by value)."

  - truth: "Each participant's role governs which members they may act on (five-tier model)"
    status: failed
    reason: >-
      (REVIEW WR-02) CanRemoveMember checks only the author's tier plus the
      creator exemption; it never compares the author's rank against the
      target's current role. A Moderator can remove an Admin (or a non-creator
      Super Admin), and an Admin can remove a non-creator Super Admin — the
      five-tier model does not govern the destructive action. The phase goal
      explicitly states the role model "governs what actions each participant
      can take".
    artifacts:
      - path: "src/lib/gcs_membership.cpp"
        issue: "CanRemoveMember (lines 556-571) has no target-privilege comparison; contrast CanChangeRole (line 546) which does deny Admin granting the Super Admin tier."
    missing:
      - "A rank comparison against the target record in CanRemoveMember (author must outrank the target)."
      - "Unit tests for Moderator-vs-Admin and Admin-vs-SuperAdmin removal attempts."

  - truth: "When 2+ admins exist, destructive actions require Super Admin approval"
    status: failed
    reason: >-
      (REVIEW WR-03) ChangeRole computes isAdminDemotion as
      `record.role() == ROLE_ADMIN && PrivilegeRank(role) < PrivilegeRank(ROLE_ADMIN)`.
      A demotion of a SUPER_ADMIN target fails the equality test, so the else
      branch applies the demotion immediately even when AdminCount > 1 — the
      D-07 dormant-approval requirement is bypassed for Super Admin demotion.
      Reachable because the creator (Super Admin) may grant ROLE_SUPER_ADMIN to
      another member (CanChangeRole returns true unconditionally for a Super
      Admin author), producing a second Super Admin.
    artifacts:
      - path: "src/lib/gcs_membership.cpp"
        issue: "ChangeRole (lines 879-887): isAdminDemotion uses enum equality instead of privilege-rank comparison."
    missing:
      - "Compare privilege ranks: `PrivilegeRank(record.role()) <= PrivilegeRank(ROLE_ADMIN) && PrivilegeRank(role) < PrivilegeRank(record.role())`."
      - "A unit test demoting a non-creator Super Admin with 2+ admins and expecting a dormant PENDING_ACTION_ROLE_CHANGE record."

  - truth: "Remote membership converges into a pushed MemberList within a session (including removals)"
    status: failed
    reason: >-
      (REVIEW WR-05) OnMemberArrived early-returns when a replicated member
      record is a tombstone (deleted() == true) without calling PushMemberList.
      A remote RemoveMember therefore never triggers a local MemberList push, so
      every other node's member list keeps rendering the removed member until an
      unrelated roster write happens to push. The local remove path pushes on
      its own write, so tests pass, but the tombstone-arrival convergence path is
      silently dropped.
    artifacts:
      - path: "src/lib/gcs_membership.cpp"
        issue: "OnMemberArrived (lines 1159-1162) returns before PushMemberList for deleted records."
    missing:
      - "Push the (tombstone-filtered) roster on tombstone arrival; add a multinode test that removes on A and expects B's roster to converge without a local write."

  - truth: "The invite bearer key authenticates the redeem"
    status: failed
    reason: >-
      (REVIEW WR-04) RedeemInvite matches the inviteId across ALL scopes in
      unordered iteration order, parses the URL's {space|room} kind but never
      cross-checks it against the located record, and exercises the bearer key
      only inside the room-key unwrap loop (a failed unwrap is skip-and-log).
      The redeemer's member record is written unconditionally, so knowledge of
      the inviteId alone (without the key) grants membership with the invite's
      stamped role.
    artifacts:
      - path: "src/lib/gcs_membership.cpp"
        issue: "RedeemInvite (lines 718-855): kind unused as authority (726-739); cross-scope inviteId scan (777-794); member record written regardless of unwrap success (838-850)."
    missing:
      - "Cross-check the URL kind against the located record's scope; reject inviteId collisions."
      - "Require the bearer key to gate the membership write (e.g. require a successful unwrap when the D-06 path is enabled)."

  - truth: "Teardown releases all membership-owned resources (no dangling references)"
    status: failed
    reason: >-
      (REVIEW WR-01) TeardownSessionAndNode evicts g_session/g_entities/
      g_messaging but never g_membership, which was constructed binding
      *g_session and *g_entities by reference. After session->Shutdown();
      entities.reset(); session.reset(); the surviving g_membership holds
      references to destroyed objects; any read before the next gcs_init
      dereferences freed memory. The object also leaks across each
      init/shutdown cycle.
    artifacts:
      - path: "src/ffi/gcs_core_ffi.cpp"
        issue: "Eviction block (338-350) omits g_membership; destruction chain (384-387) never resets it; construction at 1127-1134 binds *g_session/*g_entities."
    missing:
      - "Move g_membership into the eviction chain and destroy it first (it only reads the others)."

  - truth: "The invite view renders the pushed link for the scope it belongs to"
    status: failed
    reason: >-
      (REVIEW WR-07) SessionCubit._pendingInviteLink is set on every pushed
      InviteLink and is never cleared or keyed by scope. The invite view opened
      for scope B can therefore render a stale link that belongs to scope A (a
      link that joins the wrong scope). The doc comment "the view never invents
      a link" is true but insufficient — it can render a previously minted real
      link from another scope.
    artifacts:
      - path: "src/app/lib/cubits/session_cubit.dart"
        issue: "_pendingInviteLink global + never cleared (lines 163-167, 516)."
      - path: "src/app/lib/shell/members_dialog.dart"
        issue: "Invite view reads session.pendingInviteLink unkeyed (line 635)."
    missing:
      - "Key the pending link by scopeId and clear-on-read (or expire it); add scopeId to the InviteLink event if not already available."
---

# Phase 4: Membership & Invites Verification Report

**Phase Goal:** Users can invite others via capability tokens, and the five-tier role model (Super Admin → Guest) governs what actions each participant can take.
**Verified:** 2026-10-12T19:30:00Z
**Status:** gaps_found
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| #   | Truth   | Status     | Evidence       |
| --- | ------- | ---------- | -------------- |
| 1 | User can generate a `gcs://invite/...` link for a space or room (ROADMAP SC1) | ✓ VERIFIED | `MintInvite` (gcs_membership.cpp:584-716) returns `gcs://invite/{space\|room}/<inviteId>?key=<IK>`; FFI arm kInvite pushes InviteLink (gcs_core_ffi.cpp:1559-1583); `test_gcs_ffi` `InviteMintsSpaceInviteLinkWithCapabilityShape` passes. |
| 2 | Recipient can use the invite to join and appears in the member list (SC2) | ✗ FAILED | Room scope works only when both nodes are pre-subscribed (`test_gcs_membership_multinode` `RemoteMemberListConvergesAfterRedeem`); SPACE-scope invite records are broadcast on `gcs/chat/<spaceId>`, a topic nothing subscribes to → remote redeem cannot find the record. |
| 3 | Each participant has a visible role (5 tiers) (SC3) | ✓ VERIFIED | `enum Role` (proto:207-215) 5 tiers; `MemberList` pushes carry roles; `MembersCubit.MemberInfo` + `roleLabel` render them; Dart suite green (+75). |
| 4 | Super Admin (creator) cannot be demoted by other admins (SC4/MEMB-03) | ✓ VERIFIED | `CanChangeRole` rejects any non-SUPER_ADMIN change targeting the creator (gcs_membership.cpp:533-537); `test_gcs_membership` `CreatorDemotionIsRejected` and `test_gcs_ffi` `ChangeRoleRejectsDemotingCreator` pass. |
| 5 | When 2+ admins exist, destructive actions require Super Admin approval (SC5/MEMB-04) | ✗ FAILED | Removal (922-927) and scope deletion (977) go dormant, and admin demotion does — but a SUPER_ADMIN-target demotion bypasses D-07 (WR-03, 879-887). |
| 6 | (04-01) Proto carries Role/MemberRecord/MemberList/InviteLink/InviteRecord + creator/members_can_invite + 7 command / 2 event arms, append-only | ✓ VERIFIED | proto:207-300; oneof command arms 6-12, event arms 7-8 with no renumbering (proto:53-115); existing arms 1-5 unchanged. |
| 7 | (04-02) 32-byte room key AES-256-GCM wrap/unwrap; per-room key store; DeriveRoomKey ikm swap; D-08 envelope/HKDF unchanged | ✓ VERIFIED | `WrapRoomKey`/`UnwrapRoomKey` (gcs_crypto.cpp:254,308, fresh EVP ctx, checked returns); `RoomKeyFor` mutex-guarded store; `test_gcs_crypto` 19 cases pass. |
| 8 | (04-03) MintInvite persists ONE invite record, writes NO member record, wraps keys under IK | ✓ VERIFIED | gcs_membership.cpp:613-716; `test_gcs_membership` `MintPersistsInviteRecordAndWritesNoMember` passes. |
| 9 | (04-03) RedeemInvite writes the redeemer's wallet-keyed member record and stores the key; multi-use | ✓ VERIFIED | gcs_membership.cpp:718-856; `RedeemWritesMemberStoresKeyAndIsMultiUse` passes. (Bearer-key authorization gap noted separately as WR-04.) |
| 10 | (04-03) ChangeRole rejects demotion targeting the scope creator | ✓ VERIFIED | gcs_membership.cpp:522-554, 862-868; `CreatorDemotionIsRejected` passes. |
| 11 | (04-03) RemoveMember/ChangeRole(demotion)/DeleteScope dormant when 2+ admins; Approve activates | ✗ PARTIAL | Dormancy implemented (922-927, 977, 882-887) and `TwoAdminRemovalIsDormantUntilApproved` passes, but Super-Admin demotion escapes the dormancy rule (WR-03). |
| 12 | (04-03) MintInvite gated by author role + members_can_invite (MEMB-05) | ✓ VERIFIED | `CanInvite` (507-520) — Member allowed only when `membersCanInvite`; `membersCanInviteGatesMemberMinting` passes. |
| 13 | (04-03) MembersFor prefix-scans the roster, skips tombstones, stamps self_address | ✓ VERIFIED | gcs_membership.cpp:453-486; D-01/D-04 truth checks in `test_gcs_membership`. |
| 14 | (04-04) creator + members_can_invite stamped; UpdateSpace preserves creator | ✓ VERIFIED | gcs_entity_store.cpp:201-202, 247-253, 299; `test_gcs_entities` 11 cases pass. |
| 15 | (04-05) gcs_init constructs Membership + /gcs/members/.* heal callback so remote membership converges | ✗ PARTIAL | Constructed and callback registered (gcs_core_ffi.cpp:1060-1071, 1127-1135); converges for ROOM scopes, but space scopes never converge (see truth 2) and tombstone arrivals don't push (WR-05). |
| 16 | (04-05) The seven command arms validate, guard, delegate, and push | ✓ VERIFIED | gcs_core_ffi.cpp:1559-1745; 16 `test_gcs_ffi` cases pass (functional). |
| 17 | (04-05) P5 guards on send_text/create_room/update_space; create paths stamp creator/members_can_invite | ✓ VERIFIED | gcs_core_ffi.cpp:1307-1321, 1435-1437, 1516-1518; `RoleAtLeast` helper (742). |
| 18 | (04-05) Derived joins respect explicit_leave (D-02) | ✓ VERIFIED | `RefreshDerivedJoins` uses `HasExplicitLeave` (789); membership unit tests cover both branches; FFI test covers the non-filtered branch only. |
| 19 | (04-06) Dart MembersCubit + dispatch arms + self flag | ✓ VERIFIED | members_cubit.dart (setMembers/byScope/self from pushed selfAddress); session_cubit.dart:502-516 arms; Dart suite green. |
| 20 | (04-07) UI surfaces: members dialog, invite view, join dialog, rail affordances, pending badges | ✓ VERIFIED | members_dialog.dart (1196 lines), join_dialog.dart, room_rail.dart; 19 widget tests pass; programmatic only (visual needs human). |
| 21 | Concurrent membership and entity-mutating commands do not corrupt shared state | ✗ FAILED | CR-01 data race: unlocked Membership mutators read the EntityStore catalog while entity arms mutate it under g_mutex. |

**Score:** 16/21 truths verified

### Required Artifacts

| Artifact | Expected    | Status | Details |
| -------- | ----------- | ------ | ------- |
| `src/proto/gcs_chat.proto` | Append-only membership wire contract | ✓ VERIFIED | Role/PendingAction/WrappedRoomKey/MemberRecord/MemberList/InviteLink/InviteRecord + 7 command / 2 event arms; no renumbering. |
| `src/lib/gcs_crypto.hpp/.cpp` | Key-wrap + per-room key store + ikm swap | ✓ VERIFIED | WrapRoomKey/UnwrapRoomKey/MintRoomKey/StoreRoomKey/RoomKeyFor/ClearRoomKey; mutex-guarded store; D-08 envelope unchanged. |
| `src/lib/gcs_membership.hpp/.cpp` | Membership component (roster, roles, invites, approval) | ✓ VERIFIED | 411 + 1233 lines; all mutators + guards present and executed by tests. |
| `src/lib/gcs_entity_store.hpp/.cpp` | creator + members_can_invite stamping | ✓ VERIFIED | Defaulted 2-arg constructor; stamping + UpdateSpace preservation. |
| `src/ffi/gcs_core_ffi.cpp` | Membership construction + heal callback + 7 arms + guards | ✓ VERIFIED | g_membership, /gcs/members/.* callback, BuildMemberListEvent/BuildInviteLinkEvent, 7 arms, P5 guards. |
| `src/app/lib/cubits/members_cubit.dart` | Per-scope roster state + MemberInfo | ✓ VERIFIED | 220 lines; setMembers/byScope/self/viewer predicates. |
| `src/app/lib/shell/members_dialog.dart` | Member list + invite view + role change + confirms | ✓ VERIFIED | 1196 lines; real command envelopes (invite/redeem/changeRole/removeMember/leave/deleteScope/approve). |
| `src/app/lib/shell/join_dialog.dart` | Paste-invite redeem flow | ✓ VERIFIED | Publishes RedeemCommand after inline shape validation. |
| `src/app/lib/shell/room_rail.dart` | Join/Members/Delete rail affordances | ✓ VERIFIED | Shows Join/Members/Delete + pending-deletion + approve affordances. |
| `src/app/lib/generated/proto/gcs_chat.pb.dart` | Regenerated Dart bindings | ✓ VERIFIED | Carries Phase 4 messages/enums; app analyzes clean, suite green. |

### Key Link Verification

| From | To  | Via | Status | Details |
| ---- | --- | --- | ------ | ------- |
| gcs_core_ffi.cpp gcs_init | gcs::Membership | make_unique + KeyWrapSeam + EventSink | ✓ WIRED | gcs_core_ffi.cpp:1127-1134. |
| gcs_core_ffi.cpp dispatch switch | g_membership->MintInvite/RedeemInvite/... | 7 new arms | ✓ WIRED | 1559-1745. |
| gcs_membership.cpp MembersFor | CoreSession::QueryKeyValues | kMembersKeyPrefix | ✓ WIRED | 453-486. |
| gcs_membership.cpp MintInvite | crypto::WrapRoomKey | KeyWrapSeam.wrap | ✓ WIRED | 650, 688. |
| gcs_membership.cpp WriteMember/MintInvite Put | ScopeTopics | replication topic | ✗ PARTIAL | Wired, but for space scopes the topic is subscribed by no node (WR-06). |
| session_cubit.dart _dispatchEvent | members_cubit.dart setMembers | hasMemberList arm | ✓ WIRED | session_cubit.dart:502-511. |
| join_dialog.dart | GcsCommandTransport.publishCommand | RedeemCommand | ✓ WIRED | join_dialog.dart:136. |
| room_rail.dart | members_dialog.approvePendingOp | ApproveCommand | ✓ WIRED | members_dialog.dart:945. |
| gcs_shell.dart | SessionCubit.openDefault(membersCubit:) | MembersCubit provider | ✓ WIRED | gcs_shell.dart:114, 151. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
| -------- | ------------- | ------ | ------------------ | ------ |
| members_dialog.dart roster | `members.state.byScope[scopeId]` | Pushed `MemberList` via SessionCubit.hasMemberList → MembersCubit.setMembers | Yes (C++ `MembersFor` prefix scan over CRDT records) | ✓ FLOWING (local scope) |
| members_dialog.dart invite view | `session.pendingInviteLink` | Pushed `InviteLink` from `Membership::MintInvite` | Yes, but not scope-keyed → can render a stale link (WR-07) | ✗ HOLLOW (stale/unkeyed) |
| room_rail.dart pending badge | `RailSpace/RailRoom.pendingAction` | Pushed `SpaceTree` | Yes (C++ entity records) | ✓ FLOWING |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
| -------- | ------- | ------ | ------ |
| Full C++ suite | `ctest` 13 entries (run individually; slow `test_gcs_ffi_sdk` 49.5s) | 13/13 Passed | ✓ PASS |
| Membership unit + multinode | `ctest -R "test_gcs_membership\|test_gcs_membership_multinode"` | 2/2 Passed (12 cases) | ✓ PASS |
| FFI membership arms | `ctest -R "test_gcs_ffi$"` | Passed (16 cases) | ✓ PASS |
| Dart app suite | `flutter test` | `+75 ~1: All tests passed!` | ✓ PASS |
| Dart FFI smoke | `ctest -R "^test_gcs_ffi_dart$"` | Passed (11.0s) | ✓ PASS |

### Probe Execution

No probe scripts declared for this phase (`find scripts -name 'probe-*.sh'` → none). Step 7c: SKIPPED (no probes).

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
| ----------- | ----------- | ----------- | ------ | -------- |
| MEMB-01 | 04-01,02,03,05,07 | User can invite others via capability token (`gcs://invite/...`) | ✓ SATISFIED (local) / ✗ BLOCKED (cross-node space scope) | Mint/redeem URL implemented and tested; space-scope invite records never replicate (WR-06). |
| MEMB-02 | 04-01,02,03,05,06,07 | Space/room has roles: Super Admin, Admin, Moderator, Member, Guest | ✓ SATISFIED | `enum Role` 5 tiers; roster pushes; UI badges. |
| MEMB-03 | 04-01,03,04,05 | Super Admin (creator) cannot be demoted by other admins | ✓ SATISFIED | `CanChangeRole` creator guard; tests. |
| MEMB-04 | 04-01,03,05,06,07 | Destructive actions require Super Admin approval when 2+ admins exist | ✗ PARTIAL | Removal/delete dormant; Super-Admin demotion bypasses (WR-03). |
| MEMB-05 | 04-01,02,03,04,05,07 | Members can invite others if `membersCanInvite` is enabled | ✓ SATISFIED | `CanInvite` gate + persisted `members_can_invite`; tests. |

No orphaned requirements: all five Phase 4 requirement IDs appear in plan frontmatter.

**Tracking note (not a gap):** `REQUIREMENTS.md` still shows MEMB-01/MEMB-03/MEMB-05 as `[ ]`/Pending; it was last regenerated at commit `fe69e1c` (2026-10-02, during plan 04-06) before plans 04-03/04-05 completed. The checkboxes are stale relative to the code and SUMMARYs. Recommend updating the traceability table.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
| ---- | ---- | ------- | -------- | ------ |
| src/ffi/gcs_core_ffi.cpp, src/lib/gcs_membership.cpp | 1559-1745 / 306-333 | Unlocked iteration of shared `std::map` catalog (CR-01) | 🛑 Blocker | Undefined behaviour under concurrent create_room + change_role. |
| src/lib/gcs_membership.cpp | 1159-1162 | Early return skips roster push on tombstone (WR-05) | ⚠️ Warning | Remote removals never converge in other nodes' member lists. |
| src/lib/gcs_membership.cpp | 556-571 | Missing target-privilege check (WR-02) | ⚠️ Warning | Lower-tier author can remove a higher-tier member. |
| src/ffi/gcs_core_ffi.cpp | 338-350 | Global not evicted on teardown (WR-01) | ⚠️ Warning | Dangling references + per-cycle leak. |
| src/lib/gcs_membership.cpp | 223-250 | Non-canonical base64url accepted (IN-01) | ℹ️ Info | Weakens the single-encoding property. |
| src/app/lib/cubits/message_flow_cubit.dart | 63-73 | `truncateSender` hardcodes `0x` (IN-04) | ℹ️ Info | Display-only; wrong prefix for non-EVM identities. |

Debt-marker gate: no `TBD`/`FIXME`/`XXX` in any file modified by this phase (clean).

### Confirmation Bias Counter (disconfirmation pass)

1. **Partially met requirement:** MEMB-04 — dormancy is implemented for removal/deletion/admin-demotion but not for a Super-Admin-target demotion (WR-03).
2. **Test that passes without testing the stated behavior:** `test_gcs_ffi.cpp` `DerivedAutoJoinedRoomSurvivesExplicitLeaveFilter` asserts only the *non-filtered* branch of the D-02 filter; the `explicit_leave == true` filtering branch is unreachable from the single-wallet FFI fixture (04-05 deviation #3) and is only covered at the membership unit level.
3. **Error path with no coverage:** the tombstone-arrival push in `OnMemberArrived` is never exercised (WR-05); the multinode removal-convergence path has no test.

### Human Verification Required

1. **Invite link end-to-end (visual)** — Generate an invite for a space and a room in the app; confirm the invite view shows a `gcs://invite/...` link with a Copy action. Expected: link copies and pastes intact. Why human: clipboard + rendering cannot be asserted by grep.
2. **Join flow** — Paste an invite link in the join dialog; confirm a valid link enables Join and an invalid shape keeps the dialog open with the inline error. Expected: publish closes the dialog and the member appears in the roster. Why human: interactive Flutter flow.
3. **Role badges and gating** — Confirm five-tier badges render, the "You" marker appears on the local user, and role-gated affordances (change role / remove / approve) show only for permitted roles. Expected: matches the UI spec. Why human: visual/presentation.
4. **Pending-op surface** — With 2+ admins, trigger a removal; confirm a "Pending removal" badge and a Super-Admin Approve affordance. Expected: Approve activates the op. Why human: multi-actor UI state.
5. **Space invite across two nodes** — Mint a space invite on node A and attempt redeem on node B (networked). Expected per current code: FAILS (WR-06). Why human: requires two live nodes; this is the functional hole in truth 2.

### Gaps Summary

The membership domain is substantively implemented: the proto contract is append-only and complete (7 command + 2 event arms), the crypto key-wrap/key-store substrate is clean, the `Membership` component owns the roster/roles/invites with real write-time guards, the EntityStore stamps creator/`members_can_invite`, the FFI plane wires all seven arms plus the heal callback, and the Dart/Flutter layer (cubit + dialogs + rail) is real and tested. All 13 C++ ctest targets and the 75-test Dart suite pass.

However, six must-have truths do not hold as stated:

- **Space-scope replication hole (2/15/MEMB-01):** space member and invite records are broadcast on `gcs/chat/<spaceId>`, which no node ever subscribes to. A recipient on another node cannot redeem a space invite, and space rosters never converge. This directly defeats ROADMAP SC2 for the space case and is the phase's "CRDT-replicated roster" deliverable (REVIEW WR-06).
- **CR-01 data race (21):** unlocked Membership mutators read the EntityStore catalog while entity arms mutate it under `g_mutex` — undefined behaviour (Critical, REVIEW CR-01).
- **Role model does not fully govern actions (goal statement):** `CanRemoveMember` omits a target-privilege check (WR-02), and a Super-Admin-target demotion bypasses the D-07 dormancy rule (WR-03) — weakening MEMB-04 and the phase's core "roles govern actions" claim.
- **Convergence and lifecycle warnings:** tombstone arrivals never push a roster (WR-05); `g_membership` is never evicted at teardown (WR-01); the invite view can render a stale, unscoped link (WR-07); the invite bearer key does not gate the redeem (WR-04).

None of these are stubs or missing artifacts — the code is real and the tests green — but the phase goal ("users can invite others" network-wide, and "the role model governs actions") is only partially achieved. Recommend `/gsd:plan-phase --gaps` to close the space-scope replication channel, the CR-01 locking, and the role-guard gaps before proceeding to Phase 5.

---

_Verified: 2026-10-12T19:30:00Z_
_Verifier: Claude (gsd-verifier)_
