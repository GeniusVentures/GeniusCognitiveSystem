---
phase: 04-membership-invites
verified: 2026-10-20T01:22:51Z
status: gaps_found
score: 29/30 must-haves verified
overrides_applied: 0
re_verification:
  previous_status: gaps_found
  previous_score: 28/30
  gaps_closed:
    - "Truth 5 — the D-07 approval gate is no longer bypassable by an unauthenticated topic writer: every member record is signed and verified, so forged roles/tombstones are excluded; ChangeRole rejects while a removal is pending, RemoveMember clears pending_role, and Approve re-validates the pending role before making it irreversible (04-19/04-21)."
    - "Truth 29 — MemberRecord is now an authenticated authority artifact (created_by/record_signature/leave_signature/prior_role); every read path verifies it (TryReadMember, MembersFor, LocalRoleFor, AdminCount, OnMemberArrived), CanChangeRole's creator exception is IsSuperAdmin identity, and the creator root is write-once (04-19/04-20/04-21/04-22)."
  gaps_remaining:
    - "Truth 29 — RedeemInvite and Leave copy an UNVERIFIED raw record whose body names another scope, and WriteMemberPreservingAuthority derives the write key from the record body. A hostile membership-topic writer can make an honest node write attacker bytes into another scope's member key (including a creator root), collapsing that scope's verified authority. This is the new Critical CR-01 in 04-REVIEW.md (remainder of prior WR-04)."
  regressions: []
gaps:
  - truth: "The role model governs what actions each participant can take — no self-granted escalation, and redeem does not rewrite stored role/pending state"
    status: failed
    reason: >-
      The signed-authority trust chain itself is implemented and proven: every
      authority write signs, every authority read verifies, forged/tampered/
      wire-injected records are excluded, the creator root is write-once, and
      the transition matrix rejects unauthorized destruction. However, the two
      authority-preserving paths still take their write key from an unverified
      record body. RedeemInvite reads the raw record at the URL scope and
      requires only that its body address equal the sender; Leave reads the raw
      record and requires only address/deleted. Neither checks
      record.scope_id() == scopeId. WriteMemberPreservingAuthority then writes
      to MemberKey(record.scope_id(), record.address()). A hostile peer can
      publish { scope_id: B, address: alice, ... } under A/alice; the read paths
      correctly exclude it from rosters, but Alice's leave/redeem in A copies
      those bytes to B/alice. If Alice is B's creator, the write clobbers B's
      creator root with bytes that cannot verify for B, so every B member
      resolves to GUEST and delete/role/invite authority collapses until the
      creator re-signs the root. Even for a non-creator, an existing valid
      record in B is overwritten by unverifiable bytes, removing that member
      from B without approval. This propagates a rejected record instead of
      excluding it, violating the signed-authority contract.
    artifacts:
      - path: "src/lib/gcs_membership.cpp"
        issue: "RedeemInvite (1589-1596, 1752-1778) reads with TryReadRawMember and checks only address/deleted; no record.scope_id() == urlScopeId binding before WriteMemberPreservingAuthority."
      - path: "src/lib/gcs_membership.cpp"
        issue: "Leave (1931-1952) checks deleted/address only; no record.scope_id() == scopeId binding before WriteMemberPreservingAuthority."
      - path: "src/lib/gcs_membership.cpp"
        issue: "WriteMemberPreservingAuthority (982-1008) requires only non-empty created_by/record_signature, runs an advisory VerifyMemberRecord against the record's OWN scope_id/address (1000), and Puts to MemberKey(record.scope_id(), record.address()) (1007)."
    missing:
      - "Reject the raw record when record.scope_id() != the requested scope in both RedeemInvite (after the address check, before the tombstone check) and Leave. Prefer moving the binding check into TryReadRawMember so no future raw caller can repeat the defect."
      - "Add a unit regression: store a record at gcs/members/A/<sender> whose body names scope B, call Leave(A) and RedeemInvite for an A invite, and assert both fail and gcs/members/B/<sender> is unchanged."
deferred:
  - truth: "Room keys survive restart and reach a second application process (CR-07)"
    addressed_in: "Explicitly deferred by user decision — no later phase assigned"
    evidence: "ROADMAP.md Phase 4 deferral note; Waves 5-11 and 04-19..04-22 do not address it. Room keys remain in the process-local gcs_crypto store."
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
  - test: "Post-join room access (WR-01)"
    expected: "After redeeming a standalone-room invite and a space invite with autoJoinRooms, the joined room(s) become selectable without restarting the app. This currently FAILS by inspection: kRedeem does not refresh derived joins or push RoomList."
    why_human: "Live multi-cubit state; verify the user-visible impact and confirm the warning is still open."
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
**Verified:** 2026-10-20T01:22:51Z
**Status:** gaps_found
**Re-verification:** Yes — after gap-closure waves 04-19 through 04-22 (signed member authority, coerced pending state, wire-level attacker exclusion)

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
| --- | --- | --- | --- |
| 1 | SC1: User can generate a `gcs://invite/...` link for a space or room | VERIFIED | `MintInvite` (gcs_membership.cpp:1260-1446) emits `gcs://invite/{space\|room}/<scopeId>/<inviteId>?key=<IK>` and fails closed without the issuer seam. `MintStampsIssuerAndSignatureAndScopedUrl` passes. |
| 2 | SC2: Recipient can use the invite to join and appears in the member list | VERIFIED | `SpaceInviteConvergesWithoutTheRecipientJoiningTheSpace` passes; both rosters show 2 members. Warning WR-01: the FFI redeem arm still does not refresh derived joins/push RoomList. |
| 3 | SC3: Each participant has a visible five-tier role | VERIFIED | Five-tier `Role` enum (proto:207-214); `MemberList` carries roles + `self_address`; Dart 80 passed / 1 skipped. Roster authenticity is verified (truths 13/29). |
| 4 | SC4: Super Admin (creator) cannot be demoted by other admins | VERIFIED | `CanChangeRole` rejects any non-SUPER_ADMIN change targeting the creator (1161-1165); `CanRemoveMember` rejects the creator (1227-1230); `CreatorDemotionIsRejected` and `CreatorIdentityIsTheOnlyCreatorException` pass. |
| 5 | SC5: When 2+ admins exist, destructive actions require Super Admin approval | VERIFIED | Command paths: RemoveMember dormant predicate (1879-1901), ChangeRole admin demotion dormant (1825-1840), DeleteScope needsApproval (1976-2029), Approve activates and re-validates (2036-2206); AdminCount fails closed (866-919); pending state coherent (ChangeRole rejects pending REMOVE at 1808; RemoveMember clears pending_role). `TwoAdminRemovalIsDormantUntilApproved`, `DeleteScopeDormantThenApproved`, `ChangeRoleRejectedWhileRemovalIsPending` pass. Caveats: WR-11 (update_space drops pending fields) and a wire-level admin-immediate-tombstone divergence are listed under Anti-Patterns. |
| 6 | Proto carries Role/MemberRecord/MemberList/InviteLink/InviteRecord + creator/members_can_invite + 7 command / 2 event arms, append-only | VERIFIED | proto:203-310; `MemberRecord` fields 12-15 appended without touching 1-11; `InviteRecord` 6-7 appended; command arms 6-12 and event arms 7-8 appended. |
| 7 | 32-byte room key AES-256-GCM wrap/unwrap; per-room key store; D-08 envelope/HKDF unchanged | VERIFIED | `test_gcs_crypto` passes; Wrap/Unwrap/RoomKeyFor/DeriveRoomKey present. Store remains process-local (CR-07, deferred). |
| 8 | MintInvite persists ONE invite record, writes NO member record, wraps keys under IK | VERIFIED | gcs_membership.cpp:1260-1446; `MintPersistsInviteRecordAndWritesNoMember` passes. |
| 9 | RedeemInvite writes the redeemer's wallet-keyed member record and stores the key; multi-use | VERIFIED | gcs_membership.cpp:1724-1783; `RedeemWritesMemberStoresKeyAndIsMultiUse` passes. Existing live member keeps stored authority (1752-1770). |
| 10 | ChangeRole rejects demotion targeting the scope creator | VERIFIED | gcs_membership.cpp:1161-1165; `CreatorDemotionIsRejected` passes. |
| 11 | RemoveMember/ChangeRole(demotion)/DeleteScope dormant when 2+ admins; Approve activates | VERIFIED | Command predicates + AdminCount fail-closed + catalog mutators; `DeleteScopeDormantThenApproved`, `AdminDemotionIsDormantWhenTwoAdmins`, `TwoAdminRemovalIsDormantUntilApproved` pass. Warning WR-11. |
| 12 | MintInvite gated by author role + members_can_invite (MEMB-05) | VERIFIED | `CanInvite`/`CanInviteFor` (1123-1148); `membersCanInviteGatesMemberMinting` passes. |
| 13 | MembersFor prefix-scans, skips tombstones, stamps self_address, verifies records | VERIFIED | gcs_membership.cpp:1048-1101 verifies before the tombstone skip and derives the wallet from the key; unit/FFI tests pass. |
| 14 | Creator + members_can_invite stamped; UpdateSpace preserves creator | VERIFIED | gcs_entity_store.cpp:195-235 (creator) and UpdateSpace preserves creator (293-350); entity suite passes. WR-11: approved_by/pending_action are not preserved. |
| 15 | gcs_init constructs Membership + heal callback so remote membership converges | VERIFIED | gcs_core_ffi.cpp:1105-1132 (callback), 1279-1291 (construction); multinode convergence tests pass. |
| 16 | The seven command arms validate, guard, delegate, and push | VERIFIED | gcs_core_ffi.cpp:1701-1900; FFI suite passes. Warnings WR-01/WR-08. |
| 17 | P5 guards on send_text/create_room/update_space; create paths stamp creator/members_can_invite | VERIFIED | gcs_core_ffi.cpp:1473, 1588, 1669; entity/FFI tests pass. Warning WR-07. |
| 18 | Derived joins respect explicit_leave (D-02) | VERIFIED | `HasExplicitLeave` (1113-1121); membership tests cover both branches. Warning WR-01: leave does not refresh derived joins. |
| 19 | Dart MembersCubit + dispatch arms + self flag | VERIFIED | `members_cubit.dart` byScope/setMembers/self (76-219); `session_cubit.dart:540`; Dart suite green. |
| 20 | UI surfaces: members dialog, invite view, join dialog, rail affordances, pending badges | VERIFIED | Dart suite green; visual checks remain human. Warning WR-09 (self change-role). |
| 21 | Concurrent membership and entity-mutating commands do not corrupt shared state | VERIFIED | `m_catalogMutex` guards catalog readers/writers; `ConcurrentCatalogReadsAndRoomCreatesDoNotRace` passes. |
| 22 | Rank-strict removal guard: Moderator cannot remove Admin; Admin cannot remove non-creator Super Admin | VERIFIED | `CanRemoveMember` strict outrank (1231-1247); `ModeratorCannotRemoveAdminAndAnUnverifiableSuperAdminGrantsNothing` passes. |
| 23 | Demoting a non-creator Super Admin with 2+ admins writes a dormant role change | VERIFIED | Rank-based `isAdminDemotion` (1825-1833); `AdminDemotionIsDormantWhenTwoAdmins` passes. |
| 24 | The bearer key gates every redeem; no key/no authenticator writes no member record | VERIFIED | Unconditional `verifyAuth` before any write (1564-1582); `ForgedOrKeylessSpaceInviteWritesNoMemberRecord`, `ChildlessSpaceInviteIsBearerGated` pass. Warning WR-06: gate is seam-optional. |
| 25 | Tombstone arrival pushes the filtered roster; remote removal converges | VERIFIED | `OnMemberArrived` (2209-2246); `RemovedMemberReceivesTheConvergedRoster` passes. |
| 26 | Membership replicates on a registered well-known topic; an unjoined recipient redeems | VERIFIED | `kMembershipTopic` registered listen-then-broadcast at init (gcs_core_ffi.cpp:1004-1022); multinode suite passes. |
| 27 | Teardown releases Membership before the session/catalog it borrows | VERIFIED | gcs_core_ffi.cpp:350-425 evicts globals and resets membership before session shutdown/entities. Warning WR-12: failed init paths do not pair the embedded SDK boot. |
| 28 | Invite view renders the link for its own scope; clear-on-close | VERIFIED | `_pendingInviteLinks[scopeId]` / `pendingInviteLinkFor` / `clearPendingInviteLink` (session_cubit.dart:167-181, 540); widget tests pass. |
| 29 | The role model governs what actions each participant can take — no self-granted escalation, and redeem does not rewrite stored role/pending state | FAILED | Signed writes/verified reads/matrix/write-once root are implemented and the wire-level injected-authority tests pass. Remaining blocker: RedeemInvite/Leave copy an unverified raw record whose body can name another scope, and WriteMemberPreservingAuthority keys the write from that body — an honest node can be made to overwrite another scope's member/creator record with unverifiable bytes (04-REVIEW CR-01). |
| 30 | A removed (tombstoned) member cannot rejoin via a stale invite | VERIFIED | `RedeemInvite` rejects `existing.deleted()` before unwrap/store (1597-1603); `TombstonedMemberCannotRedeemStaleInvite` asserts stored bytes and scan count unchanged. |

**Score:** 29/30 truths verified

### Deferred Items

| # | Item | Addressed In | Evidence |
| --- | --- | --- | --- |
| 1 | CR-07 — room keys live in a process-local store; they do not survive restart or reach another process | Deferred by user decision; no later phase assigned | ROADMAP.md Phase 4 deferral note; not addressed by Waves 5-11 or 04-19..04-22 |
| 2 | Exclusion-not-rollback — a hostile raw record stays at its original key while rosters and authority exclude it | Accepted design residual (04-22-SUMMARY.md) | Not a gap; recovery is proven by restoring the saved creator-signed record |
| 3 | A never-synced node trusts a signed `prior_role` (cold-start predecessor gap) | Accepted design residual (04-22-SUMMARY.md) | Not a gap for this phase; the node-local predecessor cache warms on first verified read |

### Required Artifacts

| Artifact | Expected | Status | Details |
| --- | --- | --- | --- |
| `src/proto/gcs_chat.proto` | Append-only membership wire contract | VERIFIED | Role/PendingAction/WrappedRoomKey/MemberRecord/MemberList/InviteLink/InviteRecord + creator/members_can_invite + 7 command / 2 event arms. `MemberRecord` fields 12-15 (created_by, record_signature, leave_signature, prior_role) appended without touching 1-11. |
| `src/lib/gcs_crypto.hpp/.cpp` | Key-wrap + per-room key store + ikm swap + invite authenticator + hex codec | VERIFIED | Wrap/Unwrap/RoomKeyFor/DeriveRoomKey/MintInviteAuthenticator/VerifyInviteAuthenticator/HexEncode/HexDecode present; crypto suite passes. Store is process-local (CR-07 deferred). IN-05: verify accepts any non-empty key length. |
| `src/lib/gcs_membership.hpp/.cpp` | Membership component (roster, roles, invites, approval, signed authority) | PARTIAL | Signed writes (`WriteMemberAsAuthor` 921-980), verified reads (433-459, 518-857, 1048-1101), transition matrix (518-819), write-once root (2248-2330), coherent pending state, identity creator exception (1195), validated Approve (2074-2112) all present and tested. Raw RedeemInvite/Leave paths lack the scope binding and propagate unverified bytes (CR-01, truth 29). |
| `src/lib/gcs_entity_store.hpp/.cpp` | Creator/members_can_invite stamping + synchronized catalog + deletion mutators | VERIFIED | CreateSpace/CreateRoom stamp creator; `ApplySpaceState`/`ApplyRoomState` Put outside `m_catalogMutex` then publish under it (352-389). Warning WR-11: `UpdateSpace` resets `approved_by`/`pending_action`. |
| `src/ffi/gcs_core_ffi.cpp` | Membership construction + heal callback + 7 arms + guards + lifecycle + issuer seam | VERIFIED | Membership topic at init (1004-1022); construction/seam (1105-1291); teardown (350-425). Warnings WR-01 (no derived-join refresh on redeem/leave), WR-07 (ScopeHasRoster fails open), WR-08 (scan failure pushed as empty roster), WR-12 (failed-init SDK teardown), WR-13 (orphan scope on creator-root failure). |
| `src/app/lib/cubits/members_cubit.dart` | Per-scope roster state + MemberInfo | VERIFIED | byScope, setMembers, self flag (76-219); Dart tests pass. |
| `src/app/lib/cubits/session_cubit.dart` | Scope-keyed pending invite link | VERIFIED | `_pendingInviteLinks`, `pendingInviteLinkFor`, `clearPendingInviteLink` (167-181, 540). |
| `src/app/lib/shell/members_dialog.dart` | Member list + scope-keyed invite view + confirmations | VERIFIED | Widget tests pass. Warning WR-09: change-role affordance on the viewer's own row. |
| `src/app/lib/shell/join_dialog.dart` | Paste-invite redeem flow | VERIFIED | `kInviteLinkPattern` accepts the two-segment scoped URL; tests pass. |
| `src/app/lib/generated/proto/gcs_chat.pb.dart` | Regenerated Dart bindings | VERIFIED | Exposes `createdBy`/`recordSignature`/`priorRole`/`issuerSignature`; Dart suite compiles and passes. |
| `test/test_gcs_member_helpers.hpp` | Canonical issuer/leave payload + hex signature helpers shared by unit and multinode suites | VERIFIED | `BuildMemberIssuerPayload`, `BuildMemberLeavePayload`, `TestMemberSignatureHex`, `TestLeaveSignatureHex`. |

### Key Link Verification

| From | To | Via | Status | Details |
| --- | --- | --- | --- | --- |
| `MintInvite` | `KeyWrapSeam::sign` | issuer signature over canonical payload before Put | WIRED | gcs_membership.cpp:1419-1446. |
| `RedeemInvite` | `KeyWrapSeam::verifyIssuer` | verify against `created_by` before any member write | WIRED | gcs_membership.cpp:1605-1632. |
| `RedeemInvite` | `CanInviteFor` | issuer authority when the scope resolves locally | WIRED | gcs_membership.cpp:1657-1666. |
| `RedeemInvite` / `Leave` | `WriteMemberPreservingAuthority` | write-key binding to the requested scope | NOT_WIRED | Raw record read at 1589/1931; only address/deleted checked; write key derived from `record.scope_id()` at 1000/1007. CR-01. |
| `DeleteScope` | `EntityStore::ApplySpaceState`/`ApplyRoomState` | catalog-synchronized destructive write | WIRED | gcs_membership.cpp:1996-2029. |
| `Approve` (entity branch) | `EntityStore::ApplySpaceState`/`ApplyRoomState` | re-read authoritative bytes, apply tombstone, push | WIRED | gcs_membership.cpp:2146-2199. |
| `AdminCount` | `CoreSession::QueryKeyValues` | scan failure propagated, never 0 | WIRED | gcs_membership.cpp:866-919; three propagating call sites. |
| `CanChangeRole` | `TryReadMember` + `PrivilegeRank` | target's current role, strict outrank | WIRED | gcs_membership.cpp:1181-1216. |
| `MemberRecord` | authorization source | signed issuer/record binding verified on every read | WIRED | VerifyMemberRecord (518-819) called from TryReadMember, MembersFor, AdminCount, OnMemberArrived, OnScopeCreated. |
| `kRedeem` / `kLeave` | `RefreshDerivedJoins` + `BuildRoomListEvent` | derived-join refresh after membership change | NOT_WIRED | `RefreshDerivedJoins` only at init/create_room/update_space; kRedeem pushes SpaceTree only, kLeave pushes MemberList only (gcs_core_ffi.cpp:1736-1764, 1818-1844). WR-01. |
| `CanChangeRole` creator exception | `IsSuperAdmin` | creator identity, not resolved role | WIRED | gcs_membership.cpp:1195. |
| `Approve` | concrete-role guard | re-validate `pending_role`/creator target before applying | WIRED | gcs_membership.cpp:2074-2112. |
| `gcs_init` | `Membership` + `kMembershipTopic` | construction + listen-then-broadcast | WIRED | gcs_core_ffi.cpp:1004-1022, 1279-1291. |
| `gcs_shutdown` | `g_membership`/`g_inviteAccount` | evict/reset before session shutdown | WIRED | gcs_core_ffi.cpp:350-425. |
| `session_cubit.dart` | `MembersCubit.setMembers` / scope-keyed link | dispatch arms | WIRED | session_cubit.dart:520-540. |

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
| Full C++ suite | `ctest --test-dir build/OSX/Debug --output-on-failure` | 13/13 passed (200.46s) | PASS |
| Membership unit suite | `ctest -R test_gcs_membership` | Passed (12.17s); 51/51 TEST_F cases | PASS |
| Membership multinode suite | `ctest -R test_gcs_membership_multinode` | Passed (11.08s); 9/9 TEST_F cases | PASS |
| Dart app suite | `cd src/app && flutter test` | 80 passed, 1 skipped, 0 failed | PASS |
| Wire-injected self-signed Super Admin excluded | `WireInjectedSelfSignedSuperAdminIsNotAMember` (within multinode suite) | Passed | PASS |
| Wire-injected hostile transitions excluded | `WireInjectedAuthorityTransitionsAreRejected` (within multinode suite) | Passed | PASS |
| Cold-node late-metadata convergence | `ColdNodeConvergesWhenEntityMetadataArrivesAfterMemberRecord` | Passed | PASS |
| Cross-scope raw-record write | No test exists | Code inspection confirms the defect (CR-01) | FAIL |

### Probe Execution

No probe scripts are declared for this phase and no `scripts/*/tests/probe-*.sh` files exist. Step 7c: SKIPPED (no probes).

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
| --- | --- | --- | --- | --- |
| MEMB-01 | 04-01,02,03,05,07,09,10,11,12,13,15,16,18 | User can invite others via capability token (`gcs://invite/...`) | SATISFIED | Scoped, bearer-authenticated, issuer-signed invite minted at 1260-1446; redeem verifies signature/scope/kind/role cap/authority (1448-1783); cross-node positive and forgery-negative multinode proofs pass. |
| MEMB-02 | 04-01,02,03,05,06,07,08,09,12,15,17,18,19,20,21,22 | Space/room has roles: Super Admin, Admin, Moderator, Member, Guest | SATISFIED | Five-tier enum, signed/verified roster, UI labels/badges; wiring caveat tracked under truth 29 (CR-01). |
| MEMB-03 | 04-01,03,04,05,09,12,17,19,20,21,22 | Super Admin (creator) cannot be demoted by other admins | SATISFIED | ChangeRole creator guard + strict outrank (1161-1216); wire-level creator-target rejection; tests pass. |
| MEMB-04 | 04-01,03,05,06,07,08,09,12,13,14,19,20,21,22 | Destructive actions require Super Admin approval when 2+ admins exist | SATISFIED | Command paths enforce dormancy, AdminCount fails closed, Approve re-validates, pending state is coherent. Caveats: WR-11 (update_space drops pending fields) and the wire-level admin-immediate-tombstone divergence noted under Anti-Patterns. |
| MEMB-05 | 04-01,02,03,04,05,07,09,10,11,15,19 | Members can invite others if `membersCanInvite` is enabled | SATISFIED | `CanInviteFor` gate + persisted flag + issuer-authority check at redeem; tests pass. |

No orphaned requirements: all five Phase 4 IDs appear in at least one plan's `requirements` field, and REQUIREMENTS.md maps exactly MEMB-01..05 to Phase 4.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
| --- | --- | --- | --- | --- |
| `src/lib/gcs_membership.cpp` | 1000-1007, 1589-1596, 1752-1778, 1931-1952 | Raw-record copy derives the write key from an unverified body; no scope-id binding | BLOCKER | Cross-scope write: a hostile peer can make an honest node overwrite another scope's member/creator record with unverifiable bytes, collapsing that scope's verified authority (CR-01; truth 29). |
| `src/lib/gcs_membership.cpp` | 698-725 vs 1879-1884 | Verifier accepts an admin-authored immediate tombstone of a below-admin target; the command path makes the same removal dormant when 2+ admins | WARNING | An authenticated Admin can publish a removal on the topic without Super Admin approval. Requires a design decision: enforce the author-admin-tier dormancy predicate in VerifyMemberRecord, or document that wire-level admin removals of lower-tier members are intentional. This warning does not affect unsigned/hostile peers. |
| `src/lib/gcs_membership.cpp` | 1327-1338, 1738 | MintInvite wraps a topic-derived key for child rooms the minter does not hold; RedeemInvite overwrites an existing stored key | WARNING | A room can be registered under a topic-derived key; confidentiality degrades and an existing member's working key can be replaced (WR-14). |
| `src/lib/gcs_membership.cpp` | 1752-1770, 1942-1952 | `explicit_leave` is never cleared by a later redeem/authority write | WARNING | A re-invited member is permanently excluded from derived joins (WR-15). |
| `src/lib/gcs_membership.cpp` | 1400-1417, 1566-1582 | Invite bearer gate is seam-optional (fails open when unset) | WARNING | A partially wired seam silently removes the bearer-key precondition (WR-06). |
| `src/lib/gcs_membership.cpp` | 1978-2030 | Space deletion does not cascade to child rooms; a nested room cannot be deleted alone | WARNING | Orphaned rooms reappear as top-level; nested-room DeleteScope is always denied (WR-10). |
| `src/lib/gcs_membership.cpp` | 2168-2171, 2188-2191, 1992 | Entity `pending_action` is not cleared when deletion is activated | WARNING | Stored state `deleted && pending_action == DELETE` contradicts the member-path coherence rule (IN-07). |
| `src/ffi/gcs_core_ffi.cpp` | 1736-1764, 1818-1844, 799-879 | `kRedeem`/`kLeave` do not refresh derived joins or push `RoomList` | WARNING | Invitee cannot open auto-join/standalone rooms until restart; leave takes effect only after restart (WR-01). |
| `src/ffi/gcs_core_ffi.cpp` | 752-756 | `ScopeHasRoster` fails open on a scan error | WARNING | A transient read failure downgrades Member+/Admin+ gates to no gate (WR-07). |
| `src/ffi/gcs_core_ffi.cpp` | 596-612 | `BuildMemberListEvent` pushes an empty roster on scan failure | WARNING | Read failure renders as "No members yet" and removes role-gated affordances (WR-08). |
| `src/ffi/gcs_core_ffi.cpp` | 997-1022 | Failed init paths after the smoke/membership topic joins do not shut down the embedded SDK or clear `g_roomTopics` | WARNING | Resource leak for the process lifetime; a retry can duplicate smoke topics (WR-12). |
| `src/ffi/gcs_core_ffi.cpp` | 1542-1555, 1627-1643 | Scope creation is not transactional; a failed creator root leaves an orphan scope | WARNING | The orphan can never acquire an authority root; standalone rooms also retain freshly minted key material (WR-13). |
| `src/lib/gcs_entity_store.cpp` | 293-350 | `UpdateSpace` resets `approved_by`/`pending_action` on a full-state rewrite | WARNING | An Admin can cancel a dormant deletion by renaming the space without approval (WR-11). |
| `src/app/lib/shell/members_dialog.dart` | 318-319 | Change-role affordance is offered on the viewer's own row | WARNING | C++ always rejects self-targeting; guaranteed error toast (WR-09). |
| `src/lib/gcs_crypto.cpp` | 44-252 | Room-key store is process-local | WARNING | CR-07 — explicitly deferred by user decision. |
| `src/lib/gcs_crypto.cpp` | 515-521 | `VerifyInviteAuthenticator` accepts any non-empty key length | WARNING | Verify contract is weaker than the documented 32-byte invariant (IN-05). |
| `src/ffi/gcs_core_ffi.cpp` | 1786, 1812, 1838, 1864, 1891 | All membership failures map to one error | INFO | Causes cannot be distinguished by the user or logs (IN-03). |
| `src/lib/gcs_membership.cpp` | 1011-1046, and FFI arms | Every membership mutation pushes `MemberList` twice | INFO | Two identical roster events per action (IN-04). |

Debt-marker gate: no `TBD`/`FIXME`/`XXX` in any file modified by this phase (clean). No `sleep_for`/`sleep_until` in tests. No OS preprocessor guards in reviewed C++ sources. No hardcoded secrets, `eval`, shell, or raw stdio debug output found.

### Human Verification Required

1. **Invite link end-to-end (visual)** — Generate a space and a room invite; confirm the invite view shows a `gcs://invite/...` link with a Copy action and that copying preserves the URL. Why human: clipboard + rendering.
2. **Scope isolation of the invite view** — Open scope A's invite view, close it, open scope B's; confirm B never shows A's link. Why human: interactive drawer state.
3. **Join flow** — Paste a valid link; confirm Join enables, the dialog closes, and the member appears. Paste an invalid shape; confirm the inline error keeps the dialog open. Why human: interactive Flutter flow.
4. **Post-join room access (WR-01)** — After redeeming a standalone-room invite and a space invite with `autoJoinRooms`, confirm the joined room(s) are openable and receive messages without restarting the app. Currently expected to FAIL by inspection. Why human: multi-cubit live state.
5. **Role badges and gating** — Confirm five-tier badges, the "You" marker, and role-gated affordances match the UI spec for each tier. Why human: visual/presentation.
6. **Pending-op surface** — With 2+ admins, trigger a removal; confirm the Pending badge and Super Admin Approve affordance activate the op; confirm a target re-redeeming an invite does not clear the badge. Why human: multi-actor UI state.
7. **Two live nodes (production config)** — Mint on A, redeem on B (not joined); confirm B joins as Guest and both rosters converge. Why human: live-node smoke check (automated multinode proof exists).

### Gaps Summary

Waves 04-19 through 04-22 closed the two prior blockers at their stated scope:

- **Truth 29's original clauses are implemented.** `MemberRecord` now carries `created_by`, `record_signature`, `leave_signature`, and `prior_role` (proto 249-266). Every authority write signs (`WriteMemberAsAuthor`, 921-980) and every authority read verifies (`VerifyMemberRecord`, 518-819; `TryReadMember`, `MembersFor`, `LocalRoleFor`, `AdminCount`, `OnMemberArrived`, `OnScopeCreated`). A validly self-signed SUPER_ADMIN record, a tampered role/timestamp, an insufficiently ranked signer, a creator-target transition, a peer-admin tombstone/demotion, a lying predecessor, a replayed pre-demotion/pre-tombstone record, and a flipped leave are all excluded from rosters and grants; the wire-level multinode proofs use real secp256k1 signatures and pass.
- **Truth 5's command path is coherent.** Dormant removal/role-change/scope-delete, fail-closed `AdminCount`, re-validated `Approve`, pending-REMOVE write barrier, and pending-field clearing all pass their regression tests. The prior unauthenticated-topic-writer bypass is closed because unverifiable records never enter an authorization decision.

One goal-level blocker remains, and it is a new Critical in the fresh review:

- **Unverified raw records are propagated into another scope (truth 29, CR-01).** `RedeemInvite` (1589-1596) and `Leave` (1931-1932) read the raw record and check only address/deleted, not `record.scope_id()`. `WriteMemberPreservingAuthority` (982-1008) then writes under `MemberKey(record.scope_id(), record.address())`, and its advisory verification uses the record's own scope/address, so it cannot detect the mismatch. A hostile peer publishes `{scope_id: B, address: alice}` under `A/alice`; Alice's leave/redeem in A copies those bytes to `B/alice`, overwriting a valid record (or B's creator root) with bytes that cannot verify for B. The destination scope's role model then fails closed for everyone until the creator re-signs the root; the member is effectively removed from B without approval. The fix is a two-line scope binding in both call sites (or inside `TryReadRawMember`) plus a regression test.

The review's remaining warnings (WR-01, WR-06..WR-15, IN-01..IN-07) are correctness/robustness gaps. WR-01 is the most user-visible (post-join room access). WR-11 also undermines D-07 coherence (an `update_space` can cancel a dormant delete). CR-07 is explicitly deferred and is not counted as an open gap. The 04-22 accepted residuals (exclusion-not-rollback; a never-synced node trusting a signed `prior_role`) are documented design decisions, not gaps.

**Suggested next step:** run `/gsd:plan-phase --gaps` for a focused gap-closure plan that binds the raw-record write key to the requested scope in `RedeemInvite` and `Leave` (ideally inside `TryReadRawMember`), adds the cross-scope regression test, and then decides the wire-level admin-dormancy question surfaced above.

---

_Verified: 2026-10-20T01:22:51Z_
_Verifier: Claude (gsd-verifier)_
