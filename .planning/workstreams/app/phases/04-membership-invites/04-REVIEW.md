---
phase: 04-membership-invites
reviewed: 2026-10-18T21:49:04Z
depth: standard
files_reviewed: 30
files_reviewed_list:
  - src/CMakeLists.txt
  - src/app/lib/cubits/members_cubit.dart
  - src/app/lib/cubits/message_flow_cubit.dart
  - src/app/lib/cubits/rail_cubit.dart
  - src/app/lib/cubits/session_cubit.dart
  - src/app/lib/generated/proto/gcs_chat.pb.dart
  - src/app/lib/generated/proto/gcs_chat.pbenum.dart
  - src/app/lib/generated/proto/gcs_chat.pbjson.dart
  - src/app/lib/shell/gcs_shell.dart
  - src/app/lib/shell/join_dialog.dart
  - src/app/lib/shell/members_dialog.dart
  - src/app/lib/shell/room_rail.dart
  - src/app/test/chat_shell_test.dart
  - src/app/test/cubits/shell_cubits_test.dart
  - src/app/test/shell/join_dialog_test.dart
  - src/app/test/shell/members_dialog_test.dart
  - src/ffi/gcs_core_ffi.cpp
  - src/lib/gcs_crypto.cpp
  - src/lib/gcs_crypto.hpp
  - src/lib/gcs_entity_store.cpp
  - src/lib/gcs_entity_store.hpp
  - src/lib/gcs_membership.cpp
  - src/lib/gcs_membership.hpp
  - src/proto/gcs_chat.proto
  - test/CMakeLists.txt
  - test/test_gcs_crypto.cpp
  - test/test_gcs_entities.cpp
  - test/test_gcs_ffi.cpp
  - test/test_gcs_membership.cpp
  - test/test_gcs_membership_multinode.cpp
findings:
  critical: 8
  warning: 10
  info: 5
  total: 23
status: issues_found
---

# Phase 04: Code Review Report

**Reviewed:** 2026-10-18T21:49:04Z
**Depth:** standard
**Files Reviewed:** 30
**Status:** issues_found

## Summary

Reviewed the full Phase 04 membership/invites change set: the C++ membership engine, CRDT key/value conventions, HKDF/AES-GCM crypto helpers, EntityStore catalog, FFI command surface, proto contract, Dart cubits/shell widgets, and the C++/Dart tests. No structural pre-pass findings were supplied.

The positive baseline: no hardcoded secrets, no `eval`/shell/`printf` debug artifacts, no empty catch blocks, no `std::this_thread::sleep_for` in any reviewed test, no `void*` dependency injection, no OS preprocessor guards in C++, and the FFI mutex/unlocked-window discipline is consistently applied across all seven membership arms. The proto `Role` enum ordering, `PrivilegeRank`, and `RoleAtLeast` are mutually consistent (descending enum, `role <= minimum`, `UNSPECIFIED` excluded).

The implementation is nevertheless not shippable. The invite redeem path is an unconditional upsert that can overwrite the creator's role, clear pending destructive operations, and resurrect removed members from a still-valid link. `CanChangeRole` omits the strict-outrank check that `CanRemoveMember` has and lets the Super Admin mint additional Super Admins. The invite authenticator authenticates the *bearer key*, not the *record*, so the role and target scope in a replicated invite record are attacker-controlled by anyone holding any invite link. Scope deletion and approval bypass the EntityStore catalog, which makes a dormant scope deletion permanently unapprovable in-process. Root keys for random-keyed rooms exist only in a process-global in-memory map and are never persisted or distributed. Each of these is independently release-blocking.

---

## Critical Issues

### CR-01: `RedeemInvite` unconditionally overwrites an existing member's tier and clears pending state

**File:** `src/lib/gcs_membership.cpp:918-938`
**Issue:** The redeem path builds a fresh `chat::MemberRecord` and `WriteMember`s it without reading the existing record first. Unlike every other mutator (`ChangeRole` and `RemoveMember` both `TryReadMember` and preserve the stored fields), redeem replaces `role`, `deleted`, `approved_by`, `pending_action`, and `pending_role` with invite-derived defaults. Consequences:

- A multi-use invite link held by a member (or by the creator's own wallet on a second device) re-stamps `role = ROLE_GUEST` on redeem. The scope creator can be silently demoted from `ROLE_SUPER_ADMIN`; because `CanApprove` requires the creator wallet and `IsSuperAdmin` has no role fallback, the creator then loses approval authority and cannot restore the role (`CanChangeRole` refuses to demote the creator, but it does not prevent redeem from doing it).
- Any dormant D-07 operation on that member is erased: `approved_by` is cleared, `pending_action`/`pending_role` are reset, so an in-flight removal or role change is silently cancelled by the target simply redeeming a link.
- The invite is explicitly multi-use (line 920), so the only precondition for this is possession of a link the member already received.

No test covers re-redeeming as an existing member (`test/test_gcs_membership.cpp:448-510` only redeems as a new wallet), which is why the regression is invisible.

**Fix:**
```cpp
// src/lib/gcs_membership.cpp — replace the record construction at 921-938
chat::MemberRecord existing;
if ( TryReadMember( scopeId, m_sender, existing ) )
{
    if ( existing.deleted() )
    {
        // A revoked member is not reinstated by a bearer link (see CR-02).
        spdlog::warn( "gcs_membership: redeem rejected — wallet is not a live "
                      "member of scope '{}'", scopeId );
        return outcome::failure( sgns::gcs::Error::GcsDbError );
    }
    // Already a live member: redeem may refresh room keys (done above), but it
    // must never rewrite the tier or clear a dormant D-07 operation.
    return PushMemberList( scopeId );
}

chat::Role role = invite.role();
if ( role == chat::ROLE_UNSPECIFIED ) { role = chat::ROLE_GUEST; }
// ... build `mine` and WriteMember() exactly as today for the new-member case
```
Add a regression test that redeems twice (once as creator, once as an admin with a pending removal) and asserts the role and pending fields are unchanged.

---

### CR-02: `RedeemInvite` resurrects tombstoned (removed) members

**File:** `src/lib/gcs_membership.cpp:918-938`
**Issue:** Setting `mine.set_deleted( false )` on a wallet-keyed record that already carries a tombstone turns removal into a reversible suggestion. Any removed member who still has the invite URL (or a screenshot, or a link re-shared by another member) can redeem again and rejoin with the invite's role. `Leave`'s `explicit_leave` is likewise reset by the fresh record. Because the link is explicitly never consumed and authenticates by bearer key only, there is no revocation path. This defeats D-03's tombstone model and the "removed" convergence tests (`test/test_gcs_membership_multinode.cpp` proves the pushed roster converges, but never that the removed wallet cannot re-enter).

**Fix:** Covered by the `existing.deleted()` rejection in CR-01. Reinstatement must be an explicit Admin+ action (a role change or a fresh invite redeemed with an explicit re-admit command), never an automatic side effect of the old link. Optionally add an `invite_epoch`/`reinstated_after_ms` field so a deliberately re-issued invite can be distinguished from a stale one, but a tombstone must default to "deny".

---

### CR-03: `CanChangeRole` lets the Super Admin grant `ROLE_SUPER_ADMIN` and accepts invalid role values

**File:** `src/lib/gcs_membership.cpp:544-552`
**Issue:** The `ROLE_SUPER_ADMIN` branch returns `true` unconditionally, so the creator can promote any member to Super Admin — directly contradicting D-03 ("creator-only Super Admin") and the comment two lines below it ("An Admin may not grant the Super Admin tier"). Any promoted account then satisfies `IsSuperAdmin`-adjacent paths that key off `PrivilegeRank`, including `CanApprove`'s entity branch gate and the D-07 `AdminCount` dormancy. The same branch (and the Admin branch) also accepts `ROLE_UNSPECIFIED` and any future/unknown enum value: `PrivilegeRank` maps those to `0`, `ChangeRole` writes them verbatim, and the Dart `roleLabel` falls back to "Member" (`members_cubit.dart:36`), so a corrupted record displays as a normal Member while ranking below everything.

**Fix:** Reject non-grantable roles before the author check, then drop the unconditional Super Admin grant:
```cpp
// A role grant must be a concrete, non-Super-Admin tier (D-03).
if ( role != chat::ROLE_ADMIN && role != chat::ROLE_MODERATOR
     && role != chat::ROLE_MEMBER && role != chat::ROLE_GUEST )
{
    return false;
}
const chat::Role author = LocalRoleFor( scopeId );
if ( author == chat::ROLE_SUPER_ADMIN )
{
    return true; // creator may grant any non-Super-Admin tier
}
if ( author == chat::ROLE_ADMIN )
{
    return target != scope.creator; // Super Admin tier already excluded above
}
```
Add tests for `SUPER_ADMIN -> SUPER_ADMIN` grant rejection and `-> ROLE_UNSPECIFIED` rejection.

---

### CR-04: `CanChangeRole` omits the strict-outrank check that `CanRemoveMember` enforces

**File:** `src/lib/gcs_membership.cpp:528-560` (contrast `:584-587`)
**Issue:** `CanRemoveMember` correctly requires the author to strictly outrank the target record. `CanChangeRole` never reads the target's current role. Therefore:

- a `ROLE_MODERATOR` can change an `ROLE_ADMIN` or `ROLE_SUPER_ADMIN` to `ROLE_MEMBER`/`ROLE_GUEST` (line 557 checks only the *requested* role);
- an `ROLE_ADMIN` can demote a peer Admin (line 552 checks only that the requested role is not Super Admin and the target is not the creator).

The demotion is not even reliably dormant: `isAdminDemotion` (`:969-977`) does route it through the D-07 approval gate when 2+ admins exist, but the pending record's `pending_role` is attacker-chosen and any Super Admin approval then executes it. A Moderator who is also the sole approver path in a small scope can therefore remove the admin tier entirely. This is an authorization gap, not a UI preference.

**Fix:** Mirror `CanRemoveMember` before the author-role branches:
```cpp
chat::MemberRecord targetRecord;
if ( !TryReadMember( scopeId, target, targetRecord ) || targetRecord.deleted() )
{
    return false;
}
if ( PrivilegeRank( author ) <= PrivilegeRank( targetRecord.role() ) )
{
    return false; // may only act on strictly lower tiers
}
```
Note this also makes self-targeting a no-op (equal rank), which is correct.

---

### CR-05: `AdminCount` fails open, bypassing the D-07 dormancy gate

**File:** `src/lib/gcs_membership.cpp:370-377`
**Issue:** On a store read error, `AdminCount` returns `0`. Both destructive callers interpret `0` as "sole admin — act immediately": `ChangeRole:972` skips dormancy and writes the demotion directly, and `RemoveMember:1013` tombstones the target directly. A transient `QueryKeyValues` failure therefore converts every two-admin destructive operation into an immediate, unapproved one — the exact failure D-07 exists to prevent. The header documents the two-admin rule but not this fail-open behavior. Project invariant 12 ("fail fast … never swallow errors") is violated.

**Fix:** Return the error and fail the operation:
```cpp
// header
outcome::result<unsigned int> AdminCount( const std::string &scopeId ) const;

// implementation
if ( !scanResult.has_value() ) { return scanResult.error(); }

// callers (ChangeRole:972, RemoveMember:1013, DeleteScope:1067)
auto adminCount = AdminCount( scopeId );
if ( !adminCount.has_value() ) { return adminCount.error(); }
if ( adminCount.value() > 1 ) { /* dormant path */ }
```
Add a test with a session double whose `QueryKeyValues` fails and assert both `remove_member` and `change_role` return an error rather than writing.

---

### CR-06: `DeleteScope` and `Approve` bypass the EntityStore catalog — a dormant scope deletion can never be approved in-process

**File:** `src/lib/gcs_membership.cpp:1087-1093, 1113-1119, 1186-1223`
**Issue:** Both paths mutate the entity record with a raw `m_session.Put(...)` and never update `m_entities`. `DeleteScope` only *reads* `m_entities.Spaces()/Rooms()`, writes the pending/deleted record to the store, and immediately calls `PushSpaceTree()`, which re-emits the stale in-memory catalog. `Approve` then reads the same stale catalog, sees `pending_action() == UNSPECIFIED`, and falls through to "no dormant op", so:

1. With 2+ admins, `delete_scope` writes `PENDING_ACTION_DELETE` but `Approve(scopeId, scopeId)` always fails in the same process; the deletion is permanently stuck.
2. With one admin, `delete_scope` tombstones the store record but `PushSpaceTree` re-pushes the live record, so the UI keeps showing the deleted scope (and `Spaces()`/`Rooms()` keep returning it for every other guard).
3. The "Pending deletion" badge (`room_rail.dart:426-434`) never renders because the pushed record never carries `pending_action`.

The test suite encodes the bug rather than catching it: `test/test_gcs_membership.cpp:868-870` comments that "the delete is a direct store write, not an EntityStore mutator" and lines 872-874 construct a *fresh* `EntityStore` to observe the tombstone. There is no entity-level approve test anywhere.

**Fix:** Give `EntityStore` the deletion mutator it is missing and route both paths through it, so the store write and the catalog replacement happen together under `m_catalogMutex`:
```cpp
// gcs_entity_store.hpp
outcome::result<void> MarkDeleted( const std::string &scopeId,
                                   bool               pending,
                                   const std::string &approvedBy );

// gcs_entity_store.cpp — look up the record (space first, then room), set
// deleted/deleted_at_ms/pending_action/approved_by, Put through the session,
// then replace the catalog entry under m_catalogMutex.
```
`Membership::DeleteScope` and the entity branch of `Membership::Approve` must call this instead of `m_session.Put` and must not push the tree before the catalog is updated. Add a same-process test: `delete_scope` -> assert the pushed tree carries `PENDING_ACTION_DELETE` -> `approve` -> assert the pushed tree carries `deleted == true`.

---

### CR-07: Room message keys exist only in a process-global in-memory map and are never persisted or distributed

**File:** `src/lib/gcs_crypto.cpp:197-253`; `src/ffi/gcs_core_ffi.cpp:1507-1511`; `src/lib/gcs_membership.cpp:1277-1311`
**Issue:** `RoomKeyStore()` is a function-local static `unordered_map` with no persistence, no replication, and no load-on-init path. Two creation flows mint random 32-byte keys into it:

- a nested room (`gcs_core_ffi.cpp:1507-1511`) stores `"gcs/chat/"+roomId` in memory only; nothing about that key ever leaves the process;
- a standalone room (`gcs_membership.cpp:1277-1285`) mints a key, stores it, then mints a throwaway `memberKey` (1287), wraps the room key under it (1297-1303), and writes the envelope into the creator's `MemberRecord` — then discards `memberKey`. The envelope is unrecoverable by anyone, including the creator after restart.

`DeriveRoomKey` (`gcs_crypto.cpp:237-253`) prefers the stored random key and falls back to the plain room topic when absent. The resulting behavior:

- after a restart, every message in a random-keyed room is undecryptable (the fallback derives a different key);
- on any other node, the same room is undecryptable from the start;
- `MintInvite` wraps keys for child rooms from the *local* catalog and *local* key store (`:644-684`): a room this process did not create is wrapped with the topic-derived fallback key (`:652-668`), and a room created after the invite is absent from the record entirely — either way the invited member still cannot read it;
- invite redemption stores unwrapped keys only into the redeemer's own process map (`:914-915`), never into a replicated record other members can use.

The crypto header's "member-distributed 32-byte room key" contract (`gcs_crypto.hpp:46-64`) is therefore not implemented. This is silent data loss from the user's perspective.

**Fix:** The key material must be derivable by every member or replicated in a form they can open. Pick one and implement it end to end:

1. **Deterministic derivation (simplest):** derive the room key from replicated scope material (e.g. HKDF over the scope's invite/room-key secret that every member receives once and whose wrapped copies live in `MemberRecord.wrapped_room_key`), so `DeriveRoomKey` needs no process-local state. Remove `MintRoomKey`/`StoreRoomKey` from the room-creation paths.
2. **Persisted envelopes:** write a replicated key record (e.g. `gcs/roomkeys/<roomId>/<wallet>` or one envelope per member inside `MemberRecord.wrapped_room_key`), wrapped under a per-member wrap key established when the member joins. On `gcs_init`/`Initialize`, enumerate those records and `StoreRoomKey` each envelope so `DeriveRoomKey` resolves after restart. Do not discard the creator's wrap key (`gcs_membership.cpp:1287`).

Whichever is chosen, the envelope written at `OnScopeCreated` must be openable by at least one recoverable key, and `MintInvite` must not fall back to a topic-derived key for a room whose real key the inviter does not hold (`:659-668` should log and skip the room instead of shipping a wrong key).

---

### CR-08: The invite authenticator authenticates the bearer key, not the invite record — any invite holder can redirect the grant and escalate to Super Admin

**File:** `src/lib/gcs_membership.cpp:848-876, 918-938`; `src/lib/gcs_crypto.cpp:474-568`
**Issue:** The authenticator is `AES-GCM(HKDF(IK), SHA-256(prefix || scopeId || inviteId))`. Both inputs to the marker are chosen by the *record author*: `scopeId` is read from `invite.scope_id()` (line 857) and `inviteId` is the URL segment. The record itself carries no issuer identity and no signature (see `message InviteRecord`, `src/proto/gcs_chat.proto:270-283`), and it is stored on a permissionless, last-writer-wins CRDT topic that any peer can write. The redeemer necessarily knows `IK` (it is in the URL), so the redeemer can recompute a valid authenticator for any `(scopeId, inviteId)` pair. `MintInviteAuthenticator` and `VerifyInviteAuthenticator` are deterministic public functions, not a secret.

Attack, requiring only a normal Guest invite link `gcs://invite/room/<I>?key=<IK>` for scope A:

1. Overwrite the replicated record (key `gcs/invites/A/<I>`, or any key whose segment 1 is `<I>`) with `scope_id = B`, `is_space` matching the URL kind, `role = ROLE_SUPER_ADMIN`, and `authenticator = MintInviteAuthenticator(IK, B, I)`.
2. Redeem the original URL. The global scan matches by invite id only (`:824`), so `matchCount == 1`; `RedeemInvite` trusts `invite.scope_id() == B` and `invite.role() == ROLE_SUPER_ADMIN`.
3. The authenticator verifies under `IK` for `(B, I)`, so the bearer gate passes and a `ROLE_SUPER_ADMIN` member record for scope B is written for the attacker — full control of B, including delete/approve and, with CR-03, minting further Super Admins.

The same record substitution lets the attacker place arbitrary `wrapped_room_key[].room_id` entries; `:900-916` stores each unwrapped key under `RoomTopicFor(wrapped.room_id())` without checking that the room belongs to the invite's scope, so the attacker can overwrite the local key for any room topic name with material they control.

The authenticator does provide one useful property (a random peer cannot mint a redeemable invite without `IK`), but it cannot be the authority for *what* is granted.

**Fix:** Bind the grant to the issuer and to the URL, and verify both at redeem:

1. Add `bytes signature = 6;` and `string created_by = 7;` to `InviteRecord`. `MintInvite` signs the canonical serialization of the record (all fields except `signature`) with the inviter's wallet key.
2. In `RedeemInvite`, verify that signature against `created_by`, then verify `created_by` holds `CanInvite`-equivalent authority in `scope_id` at redeem time (an Admin+, or a Member only when `membersCanInvite`). Reject unsigned records.
3. Put the scope id in the URL (`gcs://invite/{space|room}/<scopeId>/<inviteId>?key=...`) and require `invite.scope_id() == urlScopeId`. This is what makes the `{space|room}` kind cross-check meaningful.
4. Validate every `wrapped_room_key.room_id` against the invite's scope before calling `StoreRoomKey` (resolve it in the EntityStore when available; otherwise drop it with a warning).

If signing is deferred, the minimum safe interim change is to move the role and scope into the bearer-encrypted payload and treat the store record as a hint only — but even then the bearer can re-seal a different role, so an issuer signature is the actual fix.

---

## Warnings

### WR-01: `ScopeHasRoster` fails open on a scan error, disabling the D-08 write/config gates

**File:** `src/ffi/gcs_core_ffi.cpp:738-742, 1365-1366, 1480-1481, 1561-1562`
**Issue:** `return members.has_value() && members.value().member_size() > 0;` turns a `MembersFor` error into "scope has no roster", which makes every caller skip the `RoleAtLeast` check. A store read failure therefore downgrades Member+/Admin+ gates to no gate. The retroactive-guard comment above the function makes this explicit but does not justify failing open. This is the same class of defect as CR-05 and should be fixed the same way.
**Fix:** Return `outcome::result<bool>` and treat an error as a denial (`return outcome::failure(...)` from the command arm with a "membership unavailable" notice), or at minimum deny when the scan failed but the scope has a local record.

### WR-02: Deleting a space does not cascade to its child rooms

**File:** `src/lib/gcs_membership.cpp:1069-1094`; `src/app/lib/cubits/rail_cubit.dart:196-211`
**Issue:** `DeleteScope` handles a space or a room but never walks `room.parent_space_id() == scopeId`. After a space is deleted, its child rooms remain live and the pushed `SpaceTree` still contains them; on the Dart side `setTree` falls back to treating a room whose parent is absent as standalone (`rail_cubit.dart:205-209`), so the deleted space's rooms reappear as top-level rooms and remain sendable. This contradicts the "scope deleted" model and makes the delete look partially applied.
**Fix:** In the space branch, after the space record is marked (pending or deleted), apply the same state to every room with `parent_space_id() == scopeId` through the new `EntityStore::MarkDeleted` mutator (CR-06), and push the tree once after all writes. Add a test asserting no surviving room references the deleted space.

### WR-03: The rail renders edit/new-room affordances without any role gate

**File:** `src/app/lib/shell/room_rail.dart:515-535` (contrast the comment at `:129-132` and `:424`)
**Issue:** `canDelete` and `canApproveDeletion` are computed from the viewer's pushed role, and the comment claims this subtree "hides every gated affordance". But `_NodeIconAffordance(edit)` and `_NodeIconAffordance(add)` are unconditional. C++ rejects both (`update_space` requires Admin+, `create_room` in a space requires Admin+ — `gcs_core_ffi.cpp:1561-1565, 1480-1484`), so every Guest/Member/Moderator sees two controls that always fail with "permission denied". The D-08 gating is therefore UI-incomplete in exactly the place the comment says it is complete.
**Fix:** Compute `final bool canAdminister = viewerRole == kRoleLabelSuperAdmin || viewerRole == kRoleLabelAdmin;` from the same `viewerOf(rosters[space.id])` already in scope and wrap both affordances in `if (canAdminister) ...<Widget>[]`.

### WR-04: Moderators cannot manage members in the UI even though the C++ authority permits it

**File:** `src/app/lib/cubits/members_cubit.dart:153-154`; `src/app/lib/shell/members_dialog.dart:313-325`; `src/lib/gcs_membership.cpp:554-558`
**Issue:** `viewerCanManageMembers` is true only for Super Admin/Admin. `CanChangeRole` explicitly grants Moderators management of Member/Guest targets, and `CanRemoveMember`'s strict-outrank check does the same. The UI hides change-role/remove from Moderators, so the third tier exists in the model but has no user-visible capability (other than the composer). The same helper also gates `viewerCanDeleteScope`, so this is the single place to fix.
**Fix:** Expand the predicate to include `ROLE_MODERATOR` for member-row affordances and keep the rank restriction in the row: a Moderator may only act on rows whose `role` rank is strictly lower. If client-side rank logic is undesirable, publish a `can_manage_members`/`can_delete_scope` boolean per scope in `MemberList.self_*` fields from C++ and render from that — which also removes the duplicated role matrix from Dart.

### WR-05: The composer is not role-gated, so Guests can submit messages C++ always rejects

**File:** `src/app/lib/shell/gcs_shell.dart:231-244`; `src/ffi/gcs_core_ffi.cpp:1365-1373`
**Issue:** `disabled: !session.isReady` is the only gate on the composer and the send button. `send_text` additionally requires Member+ (`RoleAtLeast(..., ROLE_MEMBER)`), so a Guest or a member record with an unknown role gets an enabled composer, a retained draft, and a generic "send failed" toast on every submission. This is a broken-state UX defect, not merely polish: the user has no signal why the control does not work.
**Fix:** Gate on the viewer's role for the active room (the rail already computes `viewerOf(rosters[roomId])`), e.g. `disabled: !session.isReady || !viewerCanSend(viewerRole)`, and use the disabled hint to explain ("Read-only: you are a Guest in this room"). Prefer a C++-published capability flag over reimplementing `PrivilegeRank` in Dart.

### WR-06: Member-heal callback registration failure is logged only, leaving cross-node roster convergence dead

**File:** `src/ffi/gcs_core_ffi.cpp:1097-1128` (contrast `:1001-1008`)
**Issue:** The membership-topic registration fails `gcs_init` on error, but the `OnMemberArrived` callback registration on the members key pattern only logs. If that registration fails, local mutations still work and the session reports ready, but remote member records never push a refreshed roster: removals, role changes, and invites from other nodes never surface until some unrelated push happens. This is a silent half-initialized state, which the project's "fail early and explicitly" invariant forbids.
**Fix:** Treat registration failure like the membership-topic failure: log at error level and return `GCS_ERROR_GENERIC` from the command (and tear down), or set a reported capability flag so the UI does not claim membership is live. At minimum surface the failure on the push port via `PostErrorNotice`.

### WR-07: Invite-id ambiguity is an attacker-controlled denial of redemption

**File:** `src/lib/gcs_membership.cpp:848-856`
**Issue:** The scan is global (`QueryKeyValues(kInvitesKeyPrefix)`) and matches on the invite id alone (`:824`). Any peer can publish a second record under a different scope with the same segment, which makes `matchCount > 1` and permanently rejects every legitimate redemption of that link. The ambiguity check is correct as a safety property, but as written it hands a one-write denial-of-service to any topic participant.
**Fix:** Make the lookup unambiguous by construction: include the scope id in the URL (see CR-08 step 3) and scan the per-scope prefix (`InviteKey(scopeId, "")`-style) instead of the global prefix, so a foreign record cannot collide. If the global scan must remain (cross-node redeem by invite id only), resolve collisions by verifying the authenticator/issuer signature against each candidate over its *own* scope id and selecting the unique one that verifies, rather than rejecting all.

### WR-08: Invite authentication is optional by configuration and mints unauthenticated invites

**File:** `src/lib/gcs_membership.cpp:730-747, 880`
**Issue:** When `InviteAuthEnabled()` is false, `MintInvite` logs a warning and stores a record with no authenticator, and `RedeemInvite` skips the bearer gate entirely. Production currently wires the seam with `enabled = true`, but the security property is expressed as a runtime option rather than an invariant: a mis-wired seam (or a future caller constructing `Membership` without the auth callables) silently produces links anyone can redeem with any role via the CR-08 substitution. "Gates, not promises" argues for failing closed.
**Fix:** Make the authenticator mandatory for membership-enabled builds: return `GcsDbError` from `MintInvite` when `m_keyWrap.mintAuth` is not set, and return `GcsDbError` from `RedeemInvite` when the seam cannot verify rather than skipping the check. Keep a single explicit `enabled` flag only for the pre-membership test seam, and assert it is on in the FFI init path.

### WR-09: Redeem does not require the scope to be live

**File:** `src/lib/gcs_membership.cpp:857-943`
**Issue:** After resolving `scopeId` from the record, redeem writes a member record and stores room keys without checking whether the scope exists locally or has been tombstoned. An invite for a deleted space (or a forged record naming any deleted scope) still produces a live membership and key material, and `MembersFor` will then push a roster for a dead scope. The comment at `:866-868` explains why a catalog lookup cannot be used for the *kind* check (cross-node redeem), but the same argument does not apply to local liveness: when the entity record is present locally, a tombstone should refuse the write.
**Fix:** If `ResolveScope(scopeId).found` and the record's `deleted` (or `pending_action == PENDING_ACTION_DELETE`) is set, reject the redeem with a clear log line. Scope deletion must also cascade over invites (tombstone or stop honoring `gcs/invites/<scopeId>/` records), otherwise a deleted scope's links remain valid forever.

### WR-10: `ClearRoomKey` is production-dead, so revoked/deleted scopes keep their keys for the process lifetime

**File:** `src/lib/gcs_crypto.hpp:103-110`; `src/lib/gcs_crypto.cpp:231-235`
**Issue:** The header documents `ClearRoomKey` as the removal-path cleanup ("removal paths (D-06)"), but grep shows the only call sites are tests (`test/test_gcs_crypto.cpp:202-332`, `test/test_gcs_membership.cpp:467`). No membership mutator, delete path, leave path, or shutdown clears a key. The process-global map therefore grows monotonically and a deleted room's key remains resident and derivable by any code that can name its topic. Combined with CR-07 (nothing is persisted), the documented revocation mechanism does not exist at all.
**Fix:** Call it from the paths the header names: tombstone/approve removal, `Leave`, `DeleteScope` (including child rooms), and membership teardown. Better, once CR-07 is fixed, key lifetime should be owned by the replicated key record and the in-process map should be a cache populated on read and invalidated on delete.

---

## Info

### IN-01: Every membership mutator pushes `MemberList` twice

**File:** `src/ffi/gcs_core_ffi.cpp:1682, 1708, 1735, 1761, 1787`
**Issue:** The C++ side already pushes a full `MemberList` from `PushMemberList` inside each mutator (`Membership::ChangeRole:990`, `RemoveMember:1027`, `Leave:1055`, etc.), and each FFI arm then calls `PostToDart(BuildMemberListEvent(...))` again. `delete_scope` similarly posts `BuildSpaceTreeEvent` on top of `PushSpaceTree`. Two identical full-roster events reach Dart for every action (and the `OnMemberArrived` heal callback can add a third).
**Fix:** Delete the FFI-side duplicate posts for the membership arms and rely on the sink, or remove the sink push and keep the FFI post as the single owner. Pick one home.

### IN-02: Membership failures are all reported as "permission denied"

**File:** `src/ffi/gcs_core_ffi.cpp:1679, 1705, 1757, 1784`
**Issue:** The arms collapse every `GcsDbError` into "permission denied", including "member not found", "scope not found", and "no dormant op". With CR-06, the scope approve path always fails and always reports a permission problem, which will send debugging in the wrong direction. `GcsDbError` is a single code, so the arm cannot distinguish causes.
**Fix:** Have the membership methods return distinguishable error codes (or an `outcome::result` error type with a reason enum) and map them to distinct notices; at minimum, reword the generic notice to "rejected (permission or target state)".

### IN-03: `VerifyInviteAuthenticator` accepts any non-empty key length while `MintInviteAuthenticator` requires exactly 32 bytes

**File:** `src/lib/gcs_crypto.cpp:502-505, 534-537`
**Issue:** Mint enforces `inviteKey.size() == kRoomKeyLengthBytes`; verify only rejects empty. A short key still derives a usable HKDF key, so the verify contract is weaker than the documented 32-byte invariant. No current caller produces a short key (the URL decode accepts any non-empty base64url), so this is not exploitable today, but the asymmetry invites a future bypass.
**Fix:** Apply the same `kRoomKeyLengthBytes` check in `VerifyInviteAuthenticator` (and in `RedeemInvite` before using the decoded key), returning `false`/`GcsDbError`.

### IN-04: Pending invite links are keyed by scope and never expire

**File:** `src/app/lib/cubits/session_cubit.dart:164-185`
**Issue:** `_pendingInviteLinks` holds a full capability URL per scope until the invite drawer's `onClose` clears it. If the user mints an invite and never opens/never closes the drawer (e.g. the scope is deleted, or the app navigates away), the URL remains in memory for the session; the map is also never bounded by scope count. `test/shell/members_dialog_test.dart` covers the close path only.
**Fix:** Clear the entry on scope deletion, on session shutdown, and when a newer link for the same scope is minted (overwrite is already the behavior); consider a short TTL since the link is a bearer capability.

### IN-05: There is no UI path to join a non-auto-join room

**File:** `src/app/lib/cubits/rail_cubit.dart:159-166`; `src/app/lib/shell/room_rail.dart:741-825`
**Issue:** `selectRoom` rejects any topic not present in the pushed `RoomList`, and the rail renders non-derived rooms as non-selectable. For a space with `auto_join_rooms == false`, membership is replicated but the room topic is never joined and no affordance offers to join it, so the member can see the room and cannot open it. This predates the invite work but becomes visible now that invites make membership the entry path.
**Fix:** Add a "Join room" action for rooms in the tree that are absent from `RoomList`, publishing the existing `join_topic` command; or derive the join set from membership in C++ so the `RoomList` push already includes rooms the member may join.

---

_Reviewed: 2026-10-18T21:49:04Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
