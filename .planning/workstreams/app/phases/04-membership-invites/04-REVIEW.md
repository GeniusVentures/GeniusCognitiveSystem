---
phase: 04-membership-invites
reviewed: 2026-10-18T22:40:00Z
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
  critical: 1
  warning: 10
  info: 5
  total: 16
status: issues_found
---

# Phase 04: Code Review Report

**Reviewed:** 2026-10-18T22:40:00Z
**Depth:** standard
**Files Reviewed:** 30
**Status:** issues_found

## Summary

Reviewed the current Phase 04 membership/invites tree at standard depth (no structural pre-pass was supplied). The gap plans 04-13 through 04-18 are present and verified in the code: `RedeemInvite` now preserves live state and rejects tombstones before the unwrap loop; fresh records are stamped `ROLE_GUEST`; the issuer identity is C++-stamped, signed, hex-encoded, and verified against the claimed issuer before any write; the URL carries the scope and the stored record's `scope_id`/`is_space` are cross-checked; `AdminCount` fails closed; `ApplySpaceState`/`ApplyRoomState` Put outside `m_catalogMutex` and publish the exact stored state under it; the FFI issuer seam uses the real account signer and the multinode fixture signs with real `GeniusSigner` identities. CR-07 (process-local room keys) was explicitly deferred and is not re-raised here.

The remaining blocker is structural: `MemberRecord` is still an unauthenticated, permissionless-topic value. The invite record is now signed, but every authorization decision in `Membership` (`LocalRoleFor` -> `CanChangeRole`/`CanRemoveMember`/`CanDelete`) and every roster the UI trusts are derived from records that any peer may publish. The tests themselves treat direct member-record injection as the "hostile/forged-write path" and depend on it to seed admins (`test/test_gcs_membership.cpp:387-404`), so this is not a hypothetical. The rest of the findings are correctness/robustness gaps around the join/leave flow, the new role-authorization branch, pending-state transitions, and record-vs-key validation.

No hardcoded secrets, no `eval`/shell/`printf`/`cout` debug output, no empty catch blocks, no `std::this_thread::sleep_for` in any reviewed test, no OS preprocessor guards in the reviewed C++ sources, and no `void*` dependency injection were found.

---

## Narrative Findings (AI reviewer)

### Critical Issues

#### CR-01: `MemberRecord` is unsigned — any membership-topic peer can self-assert `ROLE_SUPER_ADMIN` and obtain destructive authority

**File:** `src/proto/gcs_chat.proto:236-250`; `src/lib/gcs_membership.cpp:371-390, 495-527, 529-537, 601-628, 642-676, 1536-1560`
**Issue:** `InviteRecord` gained `created_by`/`issuer_signature`, but `MemberRecord` still carries no signer and no signature, and nothing validates a member record against its writer or its key:
- `OnMemberArrived` (`:1536-1560`) parses whatever value arrives on the permissionless membership topic and pushes the roster.
- `TryReadMember`/`MembersFor` (`:371-390`, `:495-527`) return the stored bytes with no signature check and no check that `record.scope_id()`/`record.address()` match the key they were stored under.
- `LocalRoleFor` (`:529-537`) returns `record.role()` verbatim, and `CanChangeRole`/`CanRemoveMember`/`CanDelete` (`:601-628`, `:642-676`) authorize from that value.

Consequence, from any connected peer (the scope id is public in the catalog and the topic is the well-known `gcs/membership/replication`):

1. Publish `gcs/members/<victimScope>/<attackerWallet>` with `role = ROLE_SUPER_ADMIN`. The record replicates and is accepted (the multinode fixture proves wire-level `Put`s on this topic reach the other node: `test/test_gcs_membership_multinode.cpp:811-816`, `699-708`).
2. The attacker's own client now resolves `LocalRoleFor` to `ROLE_SUPER_ADMIN`, so `CanDelete` returns true and `CanRemoveMember`/`CanChangeRole` pass the strict-outrank test against every target. A `delete_scope`/`remove_member`/`change_role` command tombstones or rewrites the victim's records, which replicate back to the victim as authoritative LWW state.

`CanApprove` is the only guard that still holds (it uses `IsSuperAdmin`, `:678-681`), because it checks the resolved creator identity rather than a role. The D-03 "creator is the authority root" model is therefore not enforceable against a peer that can write the roster. The invite signature fix closed the grant path but not the authority path.

**Fix:** Make member authority verifiable end to end:
1. Append `string created_by = 12; string record_signature = 13;` to `MemberRecord` (append-only; field 12+ is free).
2. Sign the canonical serialization with `record_signature` cleared, using the existing `KeyWrapSeam::sign`, on every `WriteMember` path (self-redeem, role change, removal, leave, creator mint). Do not persist an unsigned member record.
3. On `OnMemberArrived`, `TryReadMember`, and `MembersFor`, verify `record_signature` against `created_by` and reject unverifiable records from both the roster and authorization.
4. Constrain what a self-signature may assert: a record signed by `address` may only claim the invite-derived tier (`ROLE_GUEST`) and must not clear a tombstone or a pending op. Any role grant/removal must be signed by an issuer whose own signed record proves the required rank, rooted at the creator's signed record. Until the chain exists, do not treat any pushed role as authority.
5. Add a regression test that injects `gcs/members/<scope>/<attacker>` with `role = ROLE_SUPER_ADMIN` at the wire level and asserts `MembersFor` excludes it and `DeleteScope`/`RemoveMember` reject the attacker's command.

### Warnings

#### WR-01: Membership changes never refresh the derived-join projection — a redeemer lands on a disabled room, and `leave` only takes effect after restart

**File:** `src/ffi/gcs_core_ffi.cpp:1736-1763, 1818-1843, 799-830`; `src/app/lib/cubits/rail_cubit.dart:159-166`; `src/app/lib/shell/room_rail.dart:219`
**Issue:** `RefreshDerivedJoins` is called only from `gcs_init` (`:1288`), `kCreateRoom` (`:1648`), and `kUpdateSpace` (`:1696`). Neither `kRedeem` nor `kLeave` calls it or pushes a `RoomList`:
- After a successful redeem of a *standalone-room* invite, the scope's topic is never registered (`DerivedJoinedTopics` explicitly excludes standalone rooms), so the room never appears in `g_roomTopics`.
- After a successful redeem of a *space* invite, the space's `auto_join_rooms` rooms are not recomputed, so they are absent from the pushed `RoomList`.
- In both cases `RoomList` is not pushed at all and `RailCubit.selectRoom` (`:159-166`) rejects topics outside the pushed list, while `_TreeRoomRow` renders `onPressed` disabled for `!joined` (`room_rail.dart:219`, `:756-758`). The new member can see the room but cannot open or read it until a later create/update or an app restart (init recomputes).
- Symmetrically, `Leave` writes `explicit_leave = true` (`gcs_membership.cpp:1312`) but never re-runs `RefreshDerivedJoins`, so the left space's derived rooms stay in `g_roomTopics` and keep receiving messages until restart or an unrelated create/update. The D-02 term has no observable effect at the time it is written.

This extends the previously reported IN-05; it now blocks the phase's primary entry path (join via invite) rather than only non-auto-join spaces.

**Fix:** In the `kRedeem` and `kLeave` arms, after the mutator returns success, call `RefreshDerivedJoins(lock)` and `PostToDart(BuildRoomListEvent())` (the redeem arm already holds the lock again at that point). For a room-invite redeem, explicitly register `gcs/chat/<roomId>` via the same listen-first/broadcast path as `kJoinTopic` and add it to `g_roomTopics`/`g_explicitTopics`. Add an FFI/multinode test: B redeems a standalone-room invite and a space invite, then asserts B's pushed `RoomList` contains the room topic(s) before any other command.

#### WR-02: `CanChangeRole`'s creator exception is role-based, not creator-identity based

**File:** `src/lib/gcs_membership.cpp:614-621`; contract at `src/lib/gcs_membership.hpp:444-448`
**Issue:** The header documents "The scope creator is the documented exception", but the implementation tests the resolved role:
```cpp
if ( author == chat::ROLE_SUPER_ADMIN )
{
    return true;
}
```
`author` comes from `LocalRoleFor(scopeId)` (`:601`), i.e. the local sender's stored record. Any record that resolves to `ROLE_SUPER_ADMIN` — including a forged peer record (CR-01) or a stale/corrupt local record — receives the creator exception over every member, with no identity check. `IsSuperAdmin` (`:392-397`) already implements the correct predicate and is used by `Leave`/`CanApprove`.

**Fix:**
```cpp
if ( IsSuperAdmin( scopeId, m_sender ) )
{
    return true; // D-03 creator exception, by identity
}
```
Add a test that seeds a non-creator `SUPER_ADMIN` record for the local sender and asserts `ChangeRole` is rejected (the current matrix only covers a non-creator Super Admin as a *target*).

#### WR-03: `Approve` applies `pending_role` without re-validating it, bypassing the CR-03 concrete-role guard

**File:** `src/lib/gcs_membership.cpp:1427-1432`
**Issue:** The activation path copies the stored `pending_role` straight into `role`:
```cpp
record.set_approved_by( m_sender );
record.set_role( record.pending_role() );
```
`ChangeRole` guards the grantable-role set (`:596-600`) and the creator-demotion rule (`:587-591`), but `Approve` does not re-run either. A record carrying `pending_action = ROLE_CHANGE` and `pending_role = ROLE_SUPER_ADMIN` (or `ROLE_UNSPECIFIED`) is therefore applied verbatim once the creator approves. The only writer that produces pending records is `ChangeRole`, which validates, but records arrive from the permissionless topic (CR-01), so the value is attacker-controlled. The intended invariant ("`ROLE_SUPER_ADMIN` is never grantable by any author, including the creator") is not enforced on this path.

**Fix:** Before applying, re-validate the same way `CanChangeRole` does:
```cpp
const chat::Role pending = record.pending_role();
if ( pending != chat::ROLE_ADMIN && pending != chat::ROLE_MODERATOR
     && pending != chat::ROLE_MEMBER && pending != chat::ROLE_GUEST )
{
    return outcome::failure( sgns::gcs::Error::GcsDbError );
}
if ( !scope.creator.empty() && target == scope.creator )
{
    return outcome::failure( sgns::gcs::Error::GcsDbError );
}
```
(Resolve the scope as `CanChangeRole` does.) Add a test that seeds a pending `ROLE_SUPER_ADMIN` change and asserts `Approve` rejects it.

#### WR-04: Member writes key off record-body fields, and reads never validate body-vs-key — a poisoned record redirects a write to another member/scope

**File:** `src/lib/gcs_membership.cpp:434-456, 371-390, 495-527, 1167-1189`
**Issue:** `WriteMember` builds the key from the record body:
```cpp
return m_session.Put( MemberKey( updated.scope_id(), updated.address() ), ... );
```
`TryReadMember` reads `MemberKey(scopeId, address)` but never checks `record.scope_id() == scopeId` or `record.address() == address`; `MembersFor` likewise includes any record found under the prefix. The redeem merge copies the stored record wholesale (`mine = existing;`) and then `WriteMember(mine)` writes it under the body's ids. A record stored at `gcs/members/A/alice` whose body names `{scope_id: B, address: creatorB, role: GUEST}` is therefore:
- included in A's roster (roster injection), and
- rewritten by `ChangeRole(A, alice, ...)` or by Alice's own redeem to `gcs/members/B/creatorB`, overwriting another scope's creator record with a mutable role.

This is the same class of validation the invite path already has (`invite.scope_id() != urlScopeId` is a hard reject, `:966-973`); member records have no equivalent.

**Fix:**
1. Make the write key explicit: `WriteMember(scopeId, address, record)` (or force `updated.set_scope_id(scopeId); updated.set_address(address);` inside a keyed variant) and update all call sites.
2. In `TryReadMember`/`MembersFor`, reject a record whose `scope_id()`/`address()` disagree with the key (skip + `spdlog::warn`, like the unparseable case) instead of returning it.
3. In `RedeemInvite`, after copying `existing`, overwrite `mine.set_scope_id(urlScopeId); mine.set_address(m_sender);`.
4. Add a test that stores a body/key-mismatched record and asserts it is not in the roster and that `ChangeRole` does not touch the foreign key.

#### WR-05: Mutators do not reconcile existing pending state — `ChangeRole` silently cancels a pending removal, `RemoveMember` leaves a stale `pending_role`

**File:** `src/lib/gcs_membership.cpp:1229-1241, 1276-1284`
**Issue:** The non-demotion branch of `ChangeRole` always executes:
```cpp
record.set_pending_action( chat::PENDING_ACTION_UNSPECIFIED );
record.set_pending_role( chat::ROLE_UNSPECIFIED );
```
regardless of what was pending. A second admin's dormant `PENDING_ACTION_REMOVE` (written by `RemoveMember`) is therefore cancelled by any role change that is not itself an admin demotion, without the creator's `Approve`. Conversely, `RemoveMember` sets `pending_action = REMOVE` but leaves the previous `pending_role` populated, so the record carries an incoherent pending state. D-07 says destructive ops are activated only by `Approve`; cancellation is a distinct transition and is currently implicit.

**Fix:** Define the transition explicitly: reject `ChangeRole` while `record.pending_action() == PENDING_ACTION_REMOVE` (the caller must approve or explicitly cancel), or implement a creator-only cancel that clears both fields and records `approved_by`. In `RemoveMember`, clear `pending_role` when setting `pending_action = REMOVE`. Add a test that seeds a pending REMOVE, calls `ChangeRole`, and asserts the pending removal survives (or is rejected).

#### WR-06: The invite bearer gate is optional and wrong-key redemption still writes a member record

**File:** `src/lib/gcs_membership.cpp:823-840, 987-1005, 1140-1156`
**Issue:** `MintInvite` logs a warning and stores an invite with no authenticator when `InviteAuthEnabled()` is false; `RedeemInvite` then skips the bearer gate entirely. This was previously reported (WR-08) and is still open. It matters more now because unwrap failures are skip-and-log (`:1147-1153`) and never abort: for a room invite, a redeemer holding only `scopeId` + `inviteId` (no key) passes the skipped gate, fails every unwrap, and still writes a live `ROLE_GUEST` member record (`:1167-1189`). The authenticator is the only thing that makes "possess the key" a precondition for membership; expressing it as an optional runtime seam means a partially wired seam silently drops it.

**Fix:** Fail closed in both directions: return an error from `MintInvite` when `mintAuth` is unset, and from `RedeemInvite` when `verifyAuth` is unset (mirroring the `IssuerAuthEnabled` posture, `:940-946`). Keep `enabled = false` only for the pre-membership test seam. Additionally, for a room invite whose wrapped keys are all undecryptable, reject the redeem rather than writing a keyless member record.

#### WR-07: `ScopeHasRoster` fails open on a scan error, disabling the D-08 write/config gates

**File:** `src/ffi/gcs_core_ffi.cpp:745-756` (call sites `:1473`, `:1588`, `:1668`, `:1440` region)
**Issue:** `return members.has_value() && members.value().member_size() > 0;` converts a `MembersFor` failure into "no roster", which makes every caller skip its `RoleAtLeast` check. A transient store read failure therefore downgrades Member+/Admin+ gates to no gate. This was previously reported (WR-01) and is unchanged; it remains a fail-open authorization path, which the project's "fail early and explicitly" invariant forbids.

**Fix:** Return `outcome::result<bool>`; on error, `PostErrorNotice` and reject the command (`GCS_ERROR_GENERIC`) instead of continuing ungated.

#### WR-08: A failed member scan is pushed as an empty roster, wiping the Dart roster

**File:** `src/ffi/gcs_core_ffi.cpp:596-611`; `src/app/lib/cubits/members_cubit.dart:204-219`
**Issue:** `BuildMemberListEvent` converts a scan failure into a `MemberList` with only `scope_id` set (comment: "An unreachable scan ... still emits a scoped list"). `SessionCubit._dispatchEvent` forwards it (`session_cubit.dart:520-529`) and `MembersCubit.setMembers` replaces the roster with the empty list. The UI then shows "No members yet", `viewerOf` returns null, and every role-gated affordance disappears until the next successful push. A read failure is thus rendered as a successful empty result, the same fail-open pattern as WR-07 but on the presentation path.

**Fix:** Do not synthesize a successful empty roster on error. Either push an `Error` event and let Dart keep its last good roster, or make the event carry a status the cubit honors (`setMembers` skips replacement on failure). If an empty push is retained for a genuinely empty scope, distinguish "scan failed" from "zero records" in the event.

#### WR-09: The members dialog offers "change role" on the viewer's own row, which C++ always rejects

**File:** `src/app/lib/shell/members_dialog.dart:318-324`
**Issue:** `showChangeRole` lacks the `!member.self` condition that `showRemove` has (`:320-324`). The row always offers it when the viewer can manage members, but `CanChangeRole` always denies self-targeting: the creator branch rejects any non-Super-Admin role for `target == scope.creator`, `ROLE_SUPER_ADMIN` is never grantable, and every other author fails the strict-outrank check (`PrivilegeRank(author) <= PrivilegeRank(targetRecord.role())`, `gcs_membership.cpp:625`). The affordance is guaranteed to produce an error toast.

**Fix:** Mirror the remove row: `viewerCanManage && !member.self && !targetIsSuperAdmin && !member.pending`. Add a widget test asserting the self row has no change-role affordance.

#### WR-10: Deleting a space does not cascade to its rooms, and a nested room cannot be deleted on its own

**File:** `src/lib/gcs_membership.cpp:1339-1391, 672-676`; `src/app/lib/cubits/rail_cubit.dart:196-211`
**Issue:** `DeleteScope` matches only `space.id() == scopeId` or `room.id() == scopeId`; child rooms are left live, and `RailCubit.setTree` degrades a room whose parent is absent to a standalone row (`rail_cubit.dart:196-211`), so the deleted space's rooms reappear as top-level rooms. Nested rooms also cannot be deleted directly: `CanDelete` resolves `LocalRoleFor(roomId)`, and no member records exist under a nested room id, so it always returns `GUEST`. This was previously reported (WR-02) and is unchanged.

**Fix:** In the space branch, apply the same pending/deleted state to every room with `parent_space_id() == scopeId` through `ApplyRoomState`, then push the tree once. For a nested-room delete, authorize against `parent_space_id` (the owning scope) rather than the room id.

### Info

#### IN-01: Stale `kRedeem` comment contradicts the 04-15 URL contract

**File:** `src/ffi/gcs_core_ffi.cpp:1740-1743`
**Issue:** The comment says "(the scope id lives in the invite record, NOT the URL)" and "it must never derive the scope id from the URL itself", but the URL has carried the scope segment since 04-15 (`gcs_membership.cpp:893-908`) and the arm now pushes the tree only. The comment no longer describes the code and invites a future regression.
**Fix:** Reword to state that the redeem arms do not need the scope id because `RedeemInvite` pushes the roster through the sink, while the URL is the authoritative scope for parsing.

#### IN-02: Unnamed numeric literals in `PrivilegeRank` and the base64url codec

**File:** `src/lib/gcs_membership.cpp:95-107, 169-189`
**Issue:** `PrivilegeRank` returns bare `5`/`4`/`3`/`2`/`1`/`0`, and the base64url codec uses bare `0x3F`, `18`, `12`, `6`, `16`, `8` shifts/masks. The project rule is named `constexpr kCamelCase` for every numeric literal (0/1/-1 excepted in trivial contexts); these are rank values and codec constants, not trivial indices.
**Fix:** Add `constexpr unsigned int kRankSuperAdmin = 5; ... kRankUnspecified = 0;` and `kBase64UrlChunkBits = 6`, `kBase64UrlMask = 0x3F`, etc., and use them.

#### IN-03: Membership failures are all reported as "permission denied"/`GCS_ERROR_INVALID_ARGUMENT`

**File:** `src/ffi/gcs_core_ffi.cpp:1759, 1786, 1812, 1838, 1891`
**Issue:** Every `outcome::failure` becomes "permission denied" (or the redeem/leave variants), including store write failures, missing targets, and "no dormant op". `GcsDbError` is a single code, so the arm cannot distinguish causes; debugging and user feedback both point at authorization when the real cause may be the store. Previously reported as IN-02 and unchanged.
**Fix:** Give the membership methods distinguishable error reasons and map them to distinct notices; at minimum reword the generic notice to "rejected (permission or target state)".

#### IN-04: Every membership mutation pushes `MemberList` twice

**File:** `src/lib/gcs_membership.cpp:458-473, 1247, 1290, 1318`; `src/ffi/gcs_core_ffi.cpp:1789, 1815, 1842, 1894`
**Issue:** Each mutator calls `PushMemberList` through the sink, and each FFI arm then calls `PostToDart(BuildMemberListEvent(...))` again; `delete_scope`/`approve` add a duplicate `SpaceTree` as well. Two identical full-roster events reach Dart per action (possibly three with the heal callback). Previously reported as IN-01 and unchanged.
**Fix:** Pick one owner: keep the sink push and drop the FFI posts, or drop the mutator pushes and keep the FFI posts.

#### IN-05: `VerifyInviteAuthenticator` accepts any non-empty key length while mint requires exactly 32 bytes

**File:** `src/lib/gcs_crypto.cpp:487-489, 519-527`
**Issue:** `MintInviteAuthenticator` rejects `inviteKey.size() != kRoomKeyLengthBytes`; verify only rejects an empty key. A short key still derives an HKDF key and verifies an authenticator minted with that same short key (no current caller can mint one), so the verify contract is weaker than the documented 32-byte invariant. Previously reported as IN-03 and unchanged.
**Fix:** Apply the `kRoomKeyLengthBytes` check in `VerifyInviteAuthenticator` and in `RedeemInvite` before using the decoded key.

---

_Reviewed: 2026-10-18T22:40:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
