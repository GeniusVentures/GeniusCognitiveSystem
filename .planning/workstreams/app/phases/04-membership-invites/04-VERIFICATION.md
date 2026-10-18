---
phase: 04-membership-invites
verified: 2026-10-18T21:58:07Z
status: gaps_found
score: 26/30 must-haves verified
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 16/21
  gaps_closed:
    - "Recipient can use the invite to join and appears in the member list — WR-06 closed: member and invite records replicate on the registered kMembershipTopic; proven by SpaceInviteConvergesWithoutTheRecipientJoiningTheSpace"
    - "Concurrent membership and entity-mutating commands do not corrupt shared state (CR-01) — EntityStore catalog is internally mutex-guarded; stress test passes"
    - "Each participant's role governs which members they may act on (WR-02) — CanRemoveMember enforces strict rank outrank; ModeratorCannotRemoveAdminAndAdminCannotRemoveSuperAdmin passes"
    - "When 2+ admins exist, destructive actions require Super Admin approval (WR-03) — admin-demotion dormancy is now privilege-rank based; SuperAdminDemotionIsDormantWhenTwoAdmins passes"
    - "Remote membership converges into a pushed MemberList (WR-05) — OnMemberArrived pushes on tombstone arrival; RemovedMemberReceivesTheConvergedRoster passes"
    - "The invite bearer key authenticates the redeem (WR-04) — every-invite HKDF/AES-GCM authenticator, verified before any member write; wire-level proof passes"
    - "Teardown releases all membership-owned resources (WR-01) — g_membership is evicted and reset before the session and catalog it borrows"
    - "The invite view renders the pushed link for the scope it belongs to (WR-07) — pending links are keyed by C++-stamped InviteLink.scope_id and cleared on close"
  gaps_remaining:
    - "MEMB-04: a target can cancel a dormant removal by re-redeeming an invite (CR-01) and a removed wallet can resurrect itself (CR-02); dormant scope deletion can never be approved in-process because DeleteScope bypasses the EntityStore catalog (CR-06)"
    - "Role-model authority: invite grants are not issuer-authenticated, so any peer can forge a Super Admin grant (CR-08); the Super Admin can grant ROLE_SUPER_ADMIN and ROLE_UNSPECIFIED is accepted (CR-03); CanChangeRole omits the strict-outrank check (CR-04)"
  regressions: []
gaps:
  - truth: "When 2+ admins exist, destructive actions require Super Admin approval (SC5 / MEMB-04)"
    status: failed
    reason: >-
      The dormancy write exists, but the gate is not effective. RedeemInvite
      constructs a fresh MemberRecord and writes it unconditionally: a live
      member with a dormant PENDING_ACTION_REMOVE can redeem any still-valid
      invite and reset pending_action/pending_role/approved_by, cancelling the
      destructive operation without approval (CR-01). A wallet carrying a
      tombstone (already removed) redeems the same way with deleted=false and
      rejoins, so removal is reversible (CR-02). For scope deletion, DeleteScope
      writes the pending/deleted record with a raw session Put and never updates
      the EntityStore catalog; Approve reads the stale catalog, never sees
      pending_action, and always fails in-process (CR-06). AdminCount also fails
      open (returns 0 on a read error), which turns the 2+ admin gate off (CR-05).
    artifacts:
      - path: "src/lib/gcs_membership.cpp"
        issue: "RedeemInvite (918-943) builds a fresh record for m_sender with role from the invite, deleted=false, and cleared pending/approved state — it never TryReadMember first."
      - path: "src/lib/gcs_membership.cpp"
        issue: "DeleteScope (1058-1124) writes pending/deleted via m_session.Put and never updates m_entities; PushSpaceTree then re-emits the stale live catalog."
      - path: "src/lib/gcs_membership.cpp"
        issue: "Approve (1172-1225) reads m_entities.Spaces()/Rooms(), which never received DeleteScope's pending_action, so the entity-level branch is unreachable in-process."
      - path: "src/lib/gcs_membership.cpp"
        issue: "AdminCount (370-377) returns 0 when QueryKeyValues fails, failing open."
    missing:
      - "RedeemInvite must TryReadMember first: reject a tombstoned wallet, and for a live member preserve role/approved_by/pending_action/pending_role while refreshing keys."
      - "Route DeleteScope and the entity branch of Approve through an EntityStore mutator that updates the store and the catalog under m_catalogMutex; push the tree only after the catalog is updated."
      - "Add tests for: pending removal cancelled by redeem (must not); removed wallet re-redeem (must reject); delete_scope -> pushed pending state -> approve -> pushed tombstone."

  - truth: "The five-tier role model governs what actions each participant can take (phase goal / MEMB-02..05)"
    status: failed
    reason: >-
      The local guards do not bind authority to the issuer. The invite
      authenticator is AES-GCM(HKDF(IK), SHA-256(prefix||scopeId||inviteId));
      every input is known to the link holder, and InviteRecord carries no
      issuer identity or signature. Any peer can write a forged invite record
      on the permissionless replication topic with role=ROLE_SUPER_ADMIN for an
      arbitrary scope and recompute a valid authenticator from a key it minted
      itself, then redeem it (CR-08). Separately, CanChangeRole returns true
      unconditionally for a Super Admin author (allowing ROLE_SUPER_ADMIN
      grants and ROLE_UNSPECIFIED), and never compares the author's rank with
      the target's current role, so a Moderator can initiate a demotion of an
      Admin and an Admin can initiate a demotion of a non-creator Super Admin
      (CR-03/CR-04). Redeem also overwrites an existing member's stored role
      (CR-01).
    artifacts:
      - path: "src/lib/gcs_membership.cpp"
        issue: "RedeemInvite (880-896) verifies only that the record's authenticator matches the presented bearer key; the record itself is unauthenticated and attacker-writable."
      - path: "src/proto/gcs_chat.proto"
        issue: "InviteRecord (270-283) has no created_by/issuer and no signature field."
      - path: "src/lib/gcs_membership.cpp"
        issue: "CanChangeRole (528-560): unconditional Super Admin return; no target-record rank comparison; no rejection of ROLE_UNSPECIFIED/unknown roles."
      - path: "src/lib/gcs_crypto.cpp"
        issue: "MintInviteAuthenticator/VerifyInviteAuthenticator (498-568) are public deterministic predicates over the bearer key — not an issuer authority."
    missing:
      - "Bind the grant to the issuer: add InviteRecord.created_by plus an issuer signature over the canonical record, verify both at redeem, and require created_by to hold CanInvite authority in that scope."
      - "Include the scope id in the invite URL and require invite.scope_id() == urlScopeId; validate wrapped_room_key.room_id against the invite's scope."
      - "Reject ROLE_SUPER_ADMIN and non-concrete roles in CanChangeRole; require the author to strictly outrank the target record (mirror CanRemoveMember)."
      - "Do not rewrite an existing member's tier or dormant state on redeem."

  - truth: "A removed (tombstoned) member cannot rejoin via a stale invite"
    status: failed
    reason: >-
      RedeemInvite writes mine.set_deleted(false) for any wallet, including one
      that already carries a tombstone. Removal is therefore a reversible
      suggestion for anyone who kept the (never-expiring, multi-use) invite
      URL. The multinode removal test proves the pushed roster converges but
      never proves the removed wallet cannot re-enter. This defeats the
      tombstone model behind RemoveMember/Leave and the MEMB-04 destructive
      action.
    artifacts:
      - path: "src/lib/gcs_membership.cpp"
        issue: "RedeemInvite (918-938) unconditionally writes deleted=false and clears explicit_leave for the redeemer."
      - path: "test/test_gcs_membership.cpp"
        issue: "RedeemWritesMemberStoresKeyAndIsMultiUse (448-501) only redeems as new wallets; no negative re-redeem case exists."
    missing:
      - "Reject redeem when the wallet already has a tombstoned record; reinstatement must be an explicit Admin+ action."
      - "Add a regression test that removes a member, re-redeems the old link, and asserts no live record is written."

human_verification:
  - test: "Generate an invite for a space and a room in the app; confirm the invite view shows a gcs://invite/... link with a Copy action, and that the link copies/pastes intact."
    expected: "A capability URL is rendered for the exact scope the view was opened from; copying preserves the URL."
    why_human: "Clipboard and rendered link cannot be asserted by grep."
  - test: "Open the invite view for scope A, close it, then open the invite view for scope B; confirm B never shows A's link."
    expected: "Each invite view renders only the link minted for its own scope; no stale link survives the drawer."
    why_human: "Interactive Flutter drawer state and timing."
  - test: "Paste an invite link in the join dialog; confirm a valid link enables Join (dialog closes, member appears) and an invalid shape keeps the dialog open with the inline error."
    expected: "publish closes the dialog on success; the roster updates with the pushed MemberList."
    why_human: "Interactive Flutter flow."
  - test: "Confirm five-tier role badges, the 'You' marker, and role-gated affordances (change role / remove / approve / delete) match the UI spec for each tier."
    expected: "Badges render correctly; only permitted roles see destructive affordances. Note: review WR-03/WR-04/WR-05 report edit/new-room ungated and Moderator/composer gaps that C++ rejects — confirm intended behavior."
    why_human: "Visual/presentation and multi-actor UI state."
  - test: "With 2+ admins, trigger a removal; confirm the 'Pending removal' badge and a Super-Admin Approve affordance."
    expected: "The pending action is visible and approval activates it."
    why_human: "Multi-actor UI state. Note: code path can be cancelled by the target re-redeeming (CR-01); confirm after the gap is fixed."
  - test: "Two live nodes: mint a space invite on A, redeem on B (which has not joined the space)."
    expected: "B receives the invite on the membership topic, joins as Guest, and both rosters converge."
    why_human: "Requires two live nodes; automated multinode proof exists, but a production-config smoke check is still worthwhile."
---

# Phase 4: Membership & Invites Verification Report

**Phase Goal:** Users can invite others via capability tokens, and the five-tier role model (Super Admin → Guest) governs what actions each participant can take.
**Verified:** 2026-10-18T21:58:07Z
**Status:** gaps_found
**Re-verification:** Yes — after gap-closure waves 5-6 (plans 04-08..04-12)

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
| --- | --- | --- | --- |
| 1 | SC1: User can generate a `gcs://invite/...` link for a space or room | VERIFIED | `MintInvite` (gcs_membership.cpp:603-761) returns `gcs://invite/{space\|room}/<id>?key=<IK>`; `kInvite` arm pushes `BuildInviteLinkEvent(url, scopeId)` (gcs_core_ffi.cpp:1613-1627); `InviteMintsSpaceInviteLinkWithCapabilityShape` passes. |
| 2 | SC2: Recipient can use the invite to join and appears in the member list | VERIFIED | Space and room invites replicate on `Membership::kMembershipTopic`; `SpaceInviteConvergesWithoutTheRecipientJoiningTheSpace` proves B redeems and both rosters show 2 members; `RemoteMemberListConvergesAfterRedeem`/`RemoteRoleChangeConverges` pass. |
| 3 | SC3: Each participant has a visible five-tier role | VERIFIED | `enum Role` (proto:207-214) has 5 tiers; `MemberList` carries roles + `self_address`; `MembersCubit`/`members_dialog.dart` render labels/badges; Dart suite 80 passed. |
| 4 | SC4: Super Admin (creator) cannot be demoted by other admins | VERIFIED | `CanChangeRole` rejects any non-SUPER_ADMIN change targeting the creator (gcs_membership.cpp:537-543); `CreatorDemotionIsRejected` and `ChangeRoleRejectsDemotingCreator` pass. Caveat: redeem can overwrite the creator's own member-record role (truth 29), but not through ChangeRole by another admin. |
| 5 | SC5: When 2+ admins exist, destructive actions require Super Admin approval | FAILED | Dormancy is written for removal/role change, but redeem clears a pending removal without approval (CR-01); a tombstoned wallet resurrects (CR-02); dormant scope deletion can never be approved because DeleteScope bypasses the catalog (CR-06); `AdminCount` fails open (CR-05). |
| 6 | Proto carries Role/MemberRecord/MemberList/InviteLink/InviteRecord + creator/members_can_invite + 7 command / 2 event arms, append-only | VERIFIED | proto:203-318; command arms 6-12 and event arms 7-8 appended with no renumbering (proto:57-68, 104-111). |
| 7 | 32-byte room key AES-256-GCM wrap/unwrap; per-room key store; D-08 envelope/HKDF unchanged | VERIFIED | `WrapRoomKey`/`UnwrapRoomKey` in gcs_crypto.cpp; `RoomKeyFor` mutex-guarded; `test_gcs_crypto` 19 cases pass. Warning: the store is process-local and never persisted/distributed (CR-07). |
| 8 | MintInvite persists ONE invite record, writes NO member record, wraps keys under IK | VERIFIED | gcs_membership.cpp:603-761; `MintPersistsInviteRecordAndWritesNoMember` passes. |
| 9 | RedeemInvite writes the redeemer's wallet-keyed member record and stores the key; multi-use | VERIFIED | gcs_membership.cpp:900-943; `RedeemWritesMemberStoresKeyAndIsMultiUse` passes. Caveat: the write is an unconditional upsert (truths 29/30). |
| 10 | ChangeRole rejects demotion targeting the scope creator | VERIFIED | gcs_membership.cpp:528-543; `CreatorDemotionIsRejected` passes. |
| 11 | RemoveMember/ChangeRole(demotion)/DeleteScope dormant when 2+ admins; Approve activates | FAILED | Removal and role-change paths are dormant and tested (`TwoAdminRemovalIsDormantUntilApproved`, `SuperAdminDemotionIsDormantWhenTwoAdmins`), but DeleteScope's dormant state never reaches the EntityStore catalog, so the entity-level Approve branch is unreachable (CR-06). |
| 12 | MintInvite gated by author role + members_can_invite (MEMB-05) | VERIFIED | `CanInvite` (gcs_membership.cpp:513-526); `membersCanInviteGatesMemberMinting` passes. |
| 13 | MembersFor prefix-scans, skips tombstones, stamps self_address | VERIFIED | gcs_membership.cpp:453-486; unit + FFI tests. |
| 14 | Creator + members_can_invite stamped; UpdateSpace preserves creator | VERIFIED | gcs_entity_store.cpp:200-253; 13 entity cases pass. |
| 15 | gcs_init constructs Membership + heal callback so remote membership converges | VERIFIED | gcs_core_ffi.cpp:1001-1008 (topic), 1093-1180 (callback + construction); multinode convergence tests pass. |
| 16 | The seven command arms validate, guard, delegate, and push | VERIFIED | gcs_core_ffi.cpp:1602-1790; 16 FFI cases pass. |
| 17 | P5 guards on send_text/create_room/update_space; create paths stamp creator/members_can_invite | VERIFIED | gcs_core_ffi.cpp:1365-1373, 1480-1484, 1561-1565; `RoleAtLeast` helper. |
| 18 | Derived joins respect explicit_leave (D-02) | VERIFIED | `RefreshDerivedJoins` uses `HasExplicitLeave`; membership unit tests cover both branches. FFI-level filtered branch is still unreachable from the single-wallet fixture (not a regression). |
| 19 | Dart MembersCubit + dispatch arms + self flag | VERIFIED | members_cubit.dart; session_cubit.dart:525; Dart suite green. |
| 20 | UI surfaces: members dialog, invite view, join dialog, rail affordances, pending badges | VERIFIED | members_dialog.dart, join_dialog.dart, room_rail.dart, gcs_shell.dart; widget tests pass. Programmatic only; visual checks remain human. |
| 21 | Concurrent membership and entity-mutating commands do not corrupt shared state (prior CR-01) | VERIFIED | `m_catalogMutex` guards all catalog readers/writers (gcs_entity_store.cpp:126-415); `ConcurrentCatalogReadsAndRoomCreatesDoNotRace` passes; no FFI unlock site reads an unguarded map. |
| 22 | Rank-strict removal guard: Moderator cannot remove Admin; Admin cannot remove non-creator Super Admin (WR-02) | VERIFIED | `CanRemoveMember` strict outrank (gcs_membership.cpp:581-589); `ModeratorCannotRemoveAdminAndAdminCannotRemoveSuperAdmin` passes. |
| 23 | Demoting a non-creator Super Admin with 2+ admins writes a dormant role change (WR-03) | VERIFIED | Rank-based `isAdminDemotion` (gcs_membership.cpp:966-977); `SuperAdminDemotionIsDormantWhenTwoAdmins` passes. |
| 24 | The bearer key gates every redeem; no key/no authenticator writes no member record (WR-04) | VERIFIED | Unconditional verification before any write (gcs_membership.cpp:878-896); zero-key childless space invite case and forged-record case pass (`ForgedOrKeylessSpaceInviteWritesNoMemberRecord`). Note: this authenticates the bearer key, not the issuer (truth 29). |
| 25 | Tombstone arrival pushes the filtered roster; remote removal converges (WR-05) | VERIFIED | `OnMemberArrived` no longer early-returns (gcs_membership.cpp:1233-1257); `RemovedMemberReceivesTheConvergedRoster` passes. |
| 26 | Membership replicates on a registered well-known topic; an unjoined recipient redeems (WR-06) | VERIFIED | `kMembershipTopic` (gcs_membership.hpp:119); registered listen-then-broadcast at init (gcs_core_ffi.cpp:1001-1008); multinode fixture registers only this topic. |
| 27 | Teardown releases Membership before the session/catalog it borrows (WR-01 prior) | VERIFIED | gcs_core_ffi.cpp:338-396 evicts `g_membership` and resets it before `session->Shutdown()`/`entities.reset()`. |
| 28 | Invite view renders the link for its own scope; clear-on-close (WR-07) | VERIFIED | `_pendingInviteLinks[scopeId]`, `pendingInviteLinkFor`, `clearPendingInviteLink` (session_cubit.dart:164-185, 536-540); scope threaded through `members_dialog.dart:600-651`; tests cover own-scope render, cross-scope isolation, clear-on-close. |
| 29 | The role model governs what actions each participant can take — no self-granted escalation, and redeem does not rewrite stored role/pending state | FAILED | Invite grants are not issuer-authenticated: any peer can write a forged InviteRecord with `role=ROLE_SUPER_ADMIN` and recompute a valid authenticator from a key it minted (CR-08). `CanChangeRole` also allows Super Admin grants/ROLE_UNSPECIFIED and omits the target-rank check (CR-03/CR-04); redeem overwrites an existing member's role/pending state (CR-01). |
| 30 | A removed (tombstoned) member cannot rejoin via a stale invite | FAILED | `RedeemInvite` writes `deleted=false` unconditionally (gcs_membership.cpp:918-938); removed wallets can rejoin with an old link (CR-02). No negative test exists. |

**Score:** 26/30 truths verified

### Deferred Items

None. No identified gap is explicitly assigned to a later milestone phase. Phase 5 covers message tombstones, kick, and ban (MODR-01..03), which does not cover invite-based reinstatement of a Phase 4 removal or invite-grant authority.

### Required Artifacts

| Artifact | Expected | Status | Details |
| --- | --- | --- | --- |
| `src/proto/gcs_chat.proto` | Append-only membership wire contract | VERIFIED | Role/PendingAction/WrappedRoomKey/MemberRecord/MemberList/InviteLink/InviteRecord + creator/members_can_invite + 7 command / 2 event arms. InviteRecord lacks issuer/signature (truth 29). |
| `src/lib/gcs_crypto.hpp/.cpp` | Key-wrap + per-room key store + ikm swap + invite authenticator | VERIFIED | Wrap/Unwrap/RoomKeyFor/DeriveRoomKey/MintInviteAuthenticator/VerifyInviteAuthenticator present; crypto tests pass. Store is process-local (CR-07 warning). |
| `src/lib/gcs_membership.hpp/.cpp` | Membership component (roster, roles, invites, approval) | PARTIAL | Real mutators and guards exist and are exercised; redeem upsert, DeleteScope/Approve catalog bypass, and CanChangeRole rank gap remain (truths 5/11/29/30). |
| `src/lib/gcs_entity_store.hpp/.cpp` | Creator/members_can_invite stamping + synchronized catalog | VERIFIED | Stamping/preservation + `m_catalogMutex` present; concurrent stress test passes. No deletion mutator exists (root of CR-06). |
| `src/ffi/gcs_core_ffi.cpp` | Membership construction + heal callback + 7 arms + guards + lifecycle | VERIFIED | g_membership, kMembershipTopic registration, all four seam callables, eviction order, P5 guards, scoped InviteLink push. |
| `src/app/lib/cubits/members_cubit.dart` | Per-scope roster state + MemberInfo | VERIFIED | setMembers/byScope/self/viewer predicates; Dart tests pass. |
| `src/app/lib/cubits/session_cubit.dart` | Scope-keyed pending invite link | VERIFIED | `_pendingInviteLinks`, `pendingInviteLinkFor`, `clearPendingInviteLink`. |
| `src/app/lib/shell/members_dialog.dart` | Member list + scope-keyed invite view + confirmations | VERIFIED | Scope threaded through the drawer; onClose clears the pending link. |
| `src/app/lib/shell/join_dialog.dart` | Paste-invite redeem flow | VERIFIED | Publishes RedeemCommand after inline shape validation. |
| `src/app/lib/shell/room_rail.dart` | Join/Members/Delete rail affordances | VERIFIED | Shows Join/Members/Delete + pending/approve affordances. Review WR-03/WR-04/WR-05 note ungated edit/new-room and Moderator/composer gaps. |
| `src/app/lib/generated/proto/gcs_chat.pb.dart` | Regenerated Dart bindings | VERIFIED | Exposes `InviteLink.scopeId`, `InviteRecord.isSpace`, `InviteRecord.authenticator`. |

### Key Link Verification

| From | To | Via | Status | Details |
| --- | --- | --- | --- | --- |
| gcs_init | Membership | make_unique + KeyWrapSeam (all four callables) + EventSink | WIRED | gcs_core_ffi.cpp:1165-1180. |
| gcs_init | Membership::kMembershipTopic | AddListenTopic then AddBroadcastTopic before g_session install | WIRED | gcs_core_ffi.cpp:1001-1008; failure fails init. |
| gcs_shutdown | g_membership | move into local unique_ptr, reset before session/entities | WIRED | gcs_core_ffi.cpp:338-396. |
| dispatch switch | Membership mutators | 7 arms, unlocked in-flight window | WIRED | gcs_core_ffi.cpp:1602-1790. |
| MintInvite | crypto::WrapRoomKey / MintInviteAuthenticator | KeyWrapSeam.wrap / mintAuth | WIRED | gcs_membership.cpp:644-760; FFI seam at 1167-1170. |
| RedeemInvite | VerifyInviteAuthenticator | verifyAuth before any write | WIRED | gcs_membership.cpp:880-896. |
| Membership | EntityStore catalog | by-value snapshots under m_catalogMutex | WIRED | ResolveScope reads locking accessors; CR-01 race closed. |
| Membership::DeleteScope | EntityStore catalog | direct m_session.Put, no catalog write | NOT_WIRED | gcs_membership.cpp:1087-1093, 1113-1119; stale catalog re-pushed. Root of CR-06. |
| InviteRecord | issuer authority | signature/created_by + scope check | NOT_WIRED | No issuer field or signature in proto:270-283; CR-08. |
| session_cubit.dart | MembersCubit.setMembers | hasMemberList arm | WIRED | session_cubit.dart:525. |
| session_cubit.dart | scope-keyed pending link | inviteLink.scopeId | WIRED | session_cubit.dart:536-540. |
| members_dialog.dart | pendingInviteLinkFor(scopeId) / clearPendingInviteLink | _InviteLinkView + onClose | WIRED | members_dialog.dart:600-651. |
| join_dialog.dart | publishCommand | RedeemCommand | WIRED | join_dialog.dart redeem path. |

### Data-Flow Trace (Level 4)

| Artifact | Data Variable | Source | Produces Real Data | Status |
| --- | --- | --- | --- | --- |
| members_dialog roster | `members.state.byScope[scopeId]` | Pushed `MemberList` -> `Membership::MembersFor` prefix scan over CRDT member records | Yes | FLOWING |
| members_dialog invite view | `session.pendingInviteLinkFor(scopeId)` | Pushed `InviteLink` from `MintInvite`; scope stamped by C++ | Yes | FLOWING |
| room_rail pending deletion badge | `SpaceRecord.pending_action` / `deleted` | Pushed `SpaceTree` -> `EntityStore::Spaces()` in-memory catalog | No — DeleteScope writes the store but not the catalog, so the pushed tree still carries the live record | HOLLOW (CR-06) |
| members_dialog pending removal badge | `MemberRecord.pending_action` | Pushed `MemberList` -> member records | Yes, but the target can clear it by re-redeeming an invite | FLOWING (bypassable; truth 5) |

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
| --- | --- | --- | --- |
| Full C++ suite | `ctest --test-dir build/OSX/Debug --output-on-failure` | 13/13 Passed (184.7s) | PASS |
| Membership + entities + crypto | `ctest -R "test_gcs_(membership|membership_multinode|entities|crypto)$"` | 4/4 Passed (9.4s) | PASS |
| Dart app suite | `cd src/app && flutter test` | +80 ~1: All tests passed | PASS |
| Space invite across nodes | `test_gcs_membership_multinode` case 3 | Passed | PASS |
| Bearer gate on the wire | `test_gcs_membership_multinode` case 4 | Passed | PASS |
| Remote removal pushed roster | `test_gcs_membership_multinode` case 5 | Passed | PASS |

### Probe Execution

No probe scripts are declared for this phase and none exist (`find scripts -name 'probe-*.sh'`; `find . -maxdepth 4 -name 'probe-*.sh'` -> none). Step 7c: SKIPPED (no probes).

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
| --- | --- | --- | --- | --- |
| MEMB-01 | 04-01,02,03,05,07,09,10,11,12 | User can invite others via capability token (`gcs://invite/...`) | PARTIAL | Mint/redeem and cross-node space convergence work, but the token is not an authority-bearing capability: any peer can forge an invite record and grant itself any role (CR-08). |
| MEMB-02 | 04-01,02,03,05,06,07,08,09,12 | Space/room has roles: Super Admin, Admin, Moderator, Member, Guest | SATISFIED | Five-tier enum, roster pushes, UI labels/badges. Enforcement gaps are tracked under MEMB-04/truth 29. |
| MEMB-03 | 04-01,03,04,05,09,12 | Super Admin (creator) cannot be demoted by other admins | SATISFIED | ChangeRole creator guard + tests. Caveat: redeem can overwrite the creator's own member record (truth 29). |
| MEMB-04 | 04-01,03,05,06,07,08,09,12 | Destructive actions require Super Admin approval when 2+ admins exist | BLOCKED | Removal/role-change dormancy exists, but redeem cancels a pending removal and resurrects tombstones (CR-01/CR-02); dormant scope deletion cannot be approved (CR-06); AdminCount fails open (CR-05). |
| MEMB-05 | 04-01,02,03,04,05,07,09,10,11 | Members can invite others if `membersCanInvite` is enabled | SATISFIED | `CanInvite` gate + persisted `members_can_invite` + test. Note: the gate is bypassable by writing a forged invite record directly (CR-08). |

No orphaned requirements: all five Phase 4 requirement IDs appear in at least one plan's `requirements` field, and REQUIREMENTS.md maps exactly MEMB-01..05 to Phase 4. The traceability checkboxes are now checked.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
| --- | --- | --- | --- | --- |
| `src/lib/gcs_membership.cpp` | 918-943 | Redeem upserts without reading the existing member record | BLOCKER | Overwrites role/pending state and resurrects tombstones (CR-01/CR-02); bypasses MEMB-04. |
| `src/lib/gcs_membership.cpp` | 1058-1124, 1172-1225 | DeleteScope/Approve bypass the EntityStore catalog | BLOCKER | Dormant scope deletion can never be approved in-process; sole-admin deletion re-pushes the live tree (CR-06). |
| `src/lib/gcs_membership.cpp`, `src/proto/gcs_chat.proto` | 528-560, 880-896, 270-283 | Invite grant has no issuer identity/signature; CanChangeRole lacks strict outrank | BLOCKER | Any peer can self-grant Super Admin; lower tiers can initiate demotions (CR-08/CR-03/CR-04). |
| `src/lib/gcs_membership.cpp` | 370-377 | `AdminCount` returns 0 on a scan error | WARNING | Fails open, disabling the 2+ admin approval gate (CR-05). |
| `src/lib/gcs_membership.cpp` | 44-252 | `ClearRoomKey` only called from tests; key store is process-local | WARNING | Keys do not survive restart or reach other nodes; revoked room keys remain resident (CR-07/WR-10). |
| `src/ffi/gcs_core_ffi.cpp` | 738-742 | `ScopeHasRoster` fails open | WARNING | A store error downgrades Member+/Admin+ gates to no gate (WR-01 review). |
| `src/app/lib/shell/room_rail.dart` | 515-535 | Edit/new-room affordances not role-gated | WARNING | Guests/Moderators see controls C++ always rejects (WR-03 review). |
| `src/app/lib/cubits/members_cubit.dart` | 153-154 | Moderator cannot manage members in the UI | WARNING | Third tier has no visible membership capability (WR-04 review). |
| `src/app/lib/shell/gcs_shell.dart` | 231-244 | Composer not role-gated | WARNING | Guests can submit messages C++ rejects (WR-05 review). |
| `src/ffi/gcs_core_ffi.cpp` | 1097-1128 | Heal-callback registration failure is logged only | WARNING | Silent half-initialized state if registration fails (WR-06 review). |
| `src/lib/gcs_membership.cpp` | 848-856 | Global invite-id scan enables a one-write redemption DoS | WARNING | Any peer can collide an id and block a legitimate redeem (WR-07 review). |
| `src/lib/gcs_membership.cpp` | 730-747, 880 | Invite auth is optional by configuration | WARNING | A mis-wired seam silently produces ungated invites (WR-08 review). |
| `src/lib/gcs_membership.cpp` | 857-943 | Redeem does not require the scope to be live | WARNING | An invite for a deleted/forged scope still writes membership (WR-09 review). |
| `src/lib/gcs_membership.cpp` | 1069-1094 | Space deletion does not cascade to child rooms | WARNING | Orphaned rooms reappear as top-level (WR-02 review). |
| `src/lib/gcs_crypto.cpp` | 502-505, 534-537 | Verify accepts any non-empty key length | INFO | Asymmetric with Mint's 32-byte check (IN-03 review). |

Debt-marker gate: no `TBD`/`FIXME`/`XXX` in any file modified by this phase (clean).

### Human Verification Required

1. **Invite link end-to-end (visual)** — Generate an invite for a space and a room; confirm the invite view shows a `gcs://invite/...` link with a Copy action. Expected: the link copies/pastes intact. Why human: clipboard + rendering.
2. **Scope isolation of the invite view** — Open scope A's invite view, close it, open scope B's; confirm B never shows A's link. Expected: clear-on-close and per-scope rendering. Why human: interactive drawer state.
3. **Join flow** — Paste a valid link in the join dialog; confirm Join enables, the dialog closes, and the member appears. Paste an invalid shape; confirm the inline error keeps the dialog open. Why human: interactive Flutter flow.
4. **Role badges and gating** — Confirm five-tier badges, the "You" marker, and role-gated affordances match the spec. Expected: permitted roles only. Why human: visual/presentation; also confirm the WR-03/WR-04/WR-05 UI gaps after they are fixed.
5. **Pending-op surface** — With 2+ admins, trigger a removal; confirm the Pending badge and Super-Admin Approve affordance. Expected: approval activates the op. Why human: multi-actor UI state; re-check after CR-01 is fixed.
6. **Space invite across two nodes** — Mint on A, redeem on B (which has not joined). Expected: B joins as Guest and rosters converge. Why human: live-node smoke check (automated multinode proof exists).

### Gaps Summary

Wave-5/6 closed all eight gaps recorded in the 2026-10-12 verification: the EntityStore catalog is mutex-guarded (CR-01 prior), space membership replicates on a registered `kMembershipTopic`, the removal guard is rank-strict, admin-demotion dormancy is rank-based, tombstone arrivals push the roster, the bearer key gates every redeem, teardown evicts `g_membership`, and the invite view is scope-keyed. The full C++ suite (13/13) and Dart suite (80 passed / 1 skipped) are green.

However, the post-execution review surfaced goal-level defects that the narrow must-have wording did not catch:

- **MEMB-04 is bypassable (BLOCKER):** `RedeemInvite` unconditionally upserts a fresh member record. A target of a dormant removal can redeem any valid invite and erase its own pending removal; a tombstoned (removed) wallet can redeeming its way back in. For scope deletion, `DeleteScope` writes the pending/deleted record with a raw `session->Put` and never updates the `EntityStore` catalog, so `Approve` never sees the pending state and the deletion can never be approved in-process; with one admin, the pushed tree still shows the live scope.
- **The role model does not govern authority (BLOCKER):** `InviteRecord` carries no issuer identity or signature. The authenticator is a deterministic function of inputs the link holder already knows (invite key, scope id, invite id), and the invite records replicate on a permissionless topic. Any peer can write a forged record with `ROLE_SUPER_ADMIN` for any scope, recompute a valid authenticator, and redeem it. Additionally, `CanChangeRole` allows a Super Admin to grant `ROLE_SUPER_ADMIN` (and accepts `ROLE_UNSPECIFIED`), and never requires the author to outrank the target's current role.
- **Warnings:** `AdminCount` and `ScopeHasRoster` fail open; room keys are process-local and never persisted/distributed; several UI affordances are not role-gated or hide capabilities the C++ authority grants; the global invite-id scan enables a redemption DoS; invite auth is optional by configuration.

Recommend `/gsd:plan-phase --gaps` for a wave 7 that (1) makes redeem preserve existing member state and reject tombstones, (2) gives `EntityStore` a deletion mutator and routes `DeleteScope`/`Approve` through it, (3) adds issuer `created_by` + signature to `InviteRecord` with an authority check at redeem, and (4) completes the `CanChangeRole` rank/role validation.

---

_Verified: 2026-10-18T21:58:07Z_
_Verifier: Claude (gsd-verifier)_
