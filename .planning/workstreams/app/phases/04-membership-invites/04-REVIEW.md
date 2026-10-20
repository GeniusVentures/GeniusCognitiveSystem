---
phase: 04-membership-invites
reviewed: 2026-10-19T18:45:00Z
depth: standard
files_reviewed: 32
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
  - src/lib/gcs_storage/gcs_global_db.cpp
  - src/proto/gcs_chat.proto
  - test/CMakeLists.txt
  - test/test_gcs_crypto.cpp
  - test/test_gcs_entities.cpp
  - test/test_gcs_ffi.cpp
  - test/test_gcs_member_helpers.hpp
  - test/test_gcs_membership.cpp
  - test/test_gcs_membership_multinode.cpp
findings:
  critical: 1
  warning: 11
  info: 7
  total: 19
status: issues_found
---

# Phase 04: Code Review Report

**Reviewed:** 2026-10-19T18:45:00Z
**Depth:** standard
**Files Reviewed:** 32
**Status:** issues_found

## Summary

Post-gap-closure review of the Phase 04 tree at standard depth (no structural pre-pass was supplied). The gap waves 04-19 through 04-22 were read against the previous review's findings:

- **CR-01 (unsigned `MemberRecord`) is closed.** Every authority write path goes through `WriteMemberAsAuthor`, which stamps `created_by = m_sender`, derives `prior_role` from the *verified* pre-write record, and signs the canonical payload (`gcs_membership.cpp:921-980`). Every authority read (`TryReadMember`, `MembersFor`, `LocalRoleFor`, `AdminCount`, `OnScopeCreated`) verifies the signature and the body/key binding (`:528-529`, `:1048-1101`). The verification matrix now enforces creator-root write-once, the transition table, dormancy, and the approval gates (`:518-819`). The accepted residuals documented in `04-22-SUMMARY.md` (exclusion-not-rollback; a never-synced node trusts a signed `prior_role`) are not re-raised here.
- **Prior WR-02, WR-03 and WR-05 are closed** (`IsSuperAdmin` creator exception at `:1195`; `Approve` re-validates `pending_role`/creator target; `ChangeRole` rejects while a removal is pending).
- **Prior WR-04 is partially closed.** Authoring writes now key off a verified record, so the body/key mismatch that used to redirect `ChangeRole`/`RemoveMember`/`Approve` is gone. The two *raw* read paths (`RedeemInvite` merge and `Leave`) still copy an unverified record to a key derived from its body; that remainder is re-reported as **CR-01** below.
- **Prior WR-01, WR-06, WR-07, WR-08, WR-09, WR-10 and IN-01 through IN-05 remain open** and are listed with their current evidence.

New findings beyond the prior review: a cross-scope write primitive in the raw-read paths (CR-01), a dormant-delete cancellation through `update_space` (WR-11), unbalanced embedded-SDK teardown on failed init (WR-12), non-transactional scope creation (WR-13), a wrong-room-key invite wrap (WR-14), and an irreversible explicit leave (WR-15).

Counts in the frontmatter include the prior still-open items so the fixer sees the full backlog; prior findings keep their original IDs and are marked `(prior, still open)`.

No hardcoded secrets, no `eval`/shell/`printf`/`cout` debug output, no empty catch blocks, no `std::this_thread::sleep_for` in reviewed tests, no OS preprocessor guards, and no `void*` dependency injection were found. Generated Dart protobuf files were treated as codegen output and reviewed only for misuse (none found).

---

## Prior Findings — Closure Status

| ID | Status | Evidence |
|----|--------|----------|
| CR-01 (unsigned `MemberRecord`) | **Closed** | Signed writes `gcs_membership.cpp:921-980`; verified reads `:518-559`, `:1048-1101`; wire-level regressions in `test/test_gcs_membership_multinode.cpp` |
| WR-02 (role-based creator exception) | **Closed** | `CanChangeRole` uses `IsSuperAdmin(scopeId, m_sender)` at `:1195` |
| WR-03 (`Approve` applies unvalidated `pending_role`) | **Closed** | `:2080-2105` re-validates the grantable-role set and rejects a creator target |
| WR-04 (body/key mismatch redirects a write) | **Partially closed** | Verified authoring writes are key-bound; raw `RedeemInvite`/`Leave` paths re-reported as CR-01 |
| WR-05 (pending-state reconciliation) | **Closed** | `ChangeRole` rejects on `PENDING_ACTION_REMOVE` (`:1808`); `RemoveMember` clears `pending_role` (`:1889-1900`) |
| WR-01 (no derived-join refresh on redeem/leave) | **Still open** | New WR-01 below |
| WR-06 (optional bearer gate) | **Still open** | New WR-06 below |
| WR-07 (`ScopeHasRoster` fails open) | **Still open** | New WR-07 below |
| WR-08 (scan failure pushed as empty roster) | **Still open** | New WR-08 below |
| WR-09 (self change-role affordance) | **Still open** | New WR-09 below |
| WR-10 (no room cascade on space delete) | **Still open** | New WR-10 below |
| IN-01 (stale `kRedeem` comment) | **Still open** | New IN-01 below |
| IN-02 (unnamed rank/codec literals) | **Still open** | New IN-02 below |
| IN-03 (all membership failures mapped to one error) | **Still open** | New IN-03 below |
| IN-04 (duplicate `MemberList` pushes) | **Still open** | New IN-04 below |
| IN-05 (`VerifyInviteAuthenticator` length) | **Still open** | New IN-05 below |

---

## Narrative Findings (AI reviewer)

### Critical Issues

#### CR-01: `RedeemInvite` and `Leave` take the write key from an **unverified** record body — a body/key-mismatched record is laundered into another scope

**File:** `src/lib/gcs_membership.cpp:1589-1596`, `:1752-1778`, `:1930-1952`, `:1007-1008`
**Issue:** Both paths deliberately read the *raw* stored value (`TryReadRawMember`) and then hand it to `WriteMemberPreservingAuthority`, which derives the destination key from the record body:

```cpp
const bool hasExisting = TryReadRawMember( urlScopeId, m_sender, existing );  // :1589
...
if ( hasExisting && existing.address() != m_sender ) { reject }               // :1590
...
mine = existing;                                                              // :1754
auto writeResult = hasExisting ? WriteMemberPreservingAuthority( mine ) : ... // :1777
```

```cpp
if ( !TryReadRawMember( scopeId, m_sender, record ) || record.deleted()
     || record.address() != m_sender )                                        // :1931-1932
...
WriteMemberPreservingAuthority( record );                                     // :1952
```

```cpp
return m_session.Put( MemberKey( record.scope_id(), record.address() ), ... ); // :1007
```

`WriteMemberPreservingAuthority` only requires a non-empty `created_by`/`record_signature` (`:986`); its advisory `VerifyMemberRecord` call passes the record's *own* `scope_id()`/`address()` (`:1000`), so it is self-consistent and can never detect a key mismatch. Nothing checks `record.scope_id() == scopeId`. A hostile peer on the permissionless `gcs/membership/replication` topic writes `{ scope_id: B, address: alice, created_by: <anything>, record_signature: <anything>, role: ... }` under the key `gcs/members/A/alice`. The read path correctly excludes it (CR-01 of the prior review is closed). Then:

- Alice calls `leave` on scope A. Her node copies the hostile bytes and writes them to `gcs/members/B/alice` — a scope she may never have touched. If Alice is B's creator, the write clobbers B's creator root with bytes that have no valid signature for B, collapsing B's verified authority (every B member resolves to GUEST, `delete_scope`/role changes/invites all fail) until the creator re-signs the root by hand.
- Or Alice redeems an invite for A. The same hostile bytes are written to B, now carrying the invite's wrapped room-key material as well, and the redeem reports success even though A's roster still excludes her.

The write requires no signature valid for B and no authority in B. It also violates the phase's stated contract that a rejected record is *excluded*, never *propagated*: the honest node itself performs the cross-scope write. The accepted "exclusion, not rollback" residual covers leaving hostile bytes at their original key; it does not cover an automatic rewrite into a different key.

**Fix:** Reject the raw record when it does not belong to the requested key, in both call sites (or bind the write key explicitly):

```cpp
if ( record.scope_id() != scopeId )
{
    spdlog::warn( "gcs_membership: raw member record in scope '{}' names "
                  "scope '{}' — rejecting the rewrite", scopeId, record.scope_id() );
    return outcome::failure( sgns::gcs::Error::GcsDbError );
}
```

Do the same for the `existing` read in `RedeemInvite` after the address check (`:1590`), and consider moving the binding check into `TryReadRawMember` (`:441`) so no future raw caller can repeat this. Add a unit test that stores a record at `gcs/members/A/<sender>` whose body names scope B and asserts `Leave(A)` fails and `gcs/members/B/<sender>` is unchanged.

### Warnings

#### WR-11: `update_space` silently cancels a dormant deletion (D-07 coherence gap)

**File:** `src/ffi/gcs_core_ffi.cpp:1681-1687`; `src/lib/gcs_entity_store.cpp:293-350`
**Issue:** `kUpdateSpace` builds a fresh `SpaceRecord desired` with only `id`, `name`, `is_public`, `auto_join_rooms` and `members_can_invite`. `EntityStore::UpdateSpace` preserves `created_at_ms`, `deleted`, `deleted_at_ms` and `creator` from the stored record (comment at `:323-328`), but **not** `approved_by` or `pending_action`. The stored pending fields are therefore reset to their proto defaults on every update. A scope with an Admin count > 1 whose deletion was parked as `pending_action = DELETE` (`gcs_membership.cpp:1987-1988`, `:2015-2016`) has that request erased by any Admin renaming the space — no `Approve`, no creator involvement. This is the same class of state-coherence defect 04-21 fixed for member records; the entity mutator was missed. It also means an Admin can suppress another Admin's deletion request without possessing the approval authority D-07 reserves for the creator.

**Fix:** Carry the stored authority fields through the rewrite, exactly as the other immutable fields are carried:

```cpp
std::string storedApprovedBy;
chat::PendingAction storedPending = chat::PENDING_ACTION_UNSPECIFIED;
...
storedApprovedBy = existingIt->second.approved_by();
storedPending    = existingIt->second.pending_action();
...
updated.set_approved_by( storedApprovedBy );
updated.set_pending_action( storedPending );
```

Add an entities test: park a DELETE, call `UpdateSpace`, assert `pending_action` survives.

#### WR-12: Failed `gcs_init` paths leave the embedded SDK booted and smoke topics latched in `g_roomTopics`

**File:** `src/ffi/gcs_core_ffi.cpp:985`, `:997-1002`, `:1015-1022`; contrast `:959-974`
**Issue:** `EnsureSdkBooted` sets `g_sdkBootedHere` and boots the embedded `GeniusSDK`. The `Initialize()` failure path correctly pairs that boot with `GeniusSDKShutdown()` (`:959-974`, IN-02). The two later failure paths do not:

```cpp
if ( !joinedAnySmokeTopic )
{
    session->Shutdown();   // :1000
    return nullptr;        // embedded SDK stays booted
}
...
session->Shutdown();       // :1020
return nullptr;
```

A caller that never retries leaks the node's wallet, threads and ports for the process lifetime — the exact condition the `:961-966` comment says must not happen. Additionally, the smoke loop pushes into the global `g_roomTopics` at `:985` *before* these failure checks, and the teardown that clears it is gated on `g_session != nullptr`, which is never assigned on this path. A retry therefore appends `kSmokeTopicA`/`kSmokeTopicB` a second time and the pushed `RoomList` shows duplicate rooms.

**Fix:** Mirror the `Initialize()` failure path in both arms:

```cpp
session->Shutdown();
g_roomTopics.clear();
if ( g_sdkBootedHere )
{
    g_sdkBootedHere = false;
    GeniusSDKShutdown();
}
return nullptr;
```

Add an FFI test that forces the membership-topic join to fail, asserts `gcs_init` returns null, and asserts a subsequent successful init pushes each smoke topic exactly once.

#### WR-13: Scope creation is not transactional — a failed creator root leaves an orphan scope that can never acquire an authority root

**File:** `src/ffi/gcs_core_ffi.cpp:1523-1555`, `:1595-1643`; `src/lib/gcs_membership.cpp:2257-2271`
**Issue:** Both `kCreateSpace` and standalone `kCreateRoom` persist the entity first and only then call `OnScopeCreated`. If the membership write fails, the arm returns `GCS_ERROR_GENERIC` but the entity record remains. Failure is reachable: an occupying unverifiable record at the creator key makes `OnScopeCreated` reject (`:2257-2271`), and a store/`Put` failure does too. The result is a space or standalone room with no member records at all:

- `LocalRoleFor` returns GUEST for everyone, so no one can change roles, remove members or delete the scope (`CanDelete` is role-based, `:1249-1253`).
- Creation is the only caller of `OnScopeCreated`, and a retry generates a new id, so the orphan is permanent.

For standalone rooms the failure also leaves the freshly minted room key stored under the new topic (`:2296`), so a later entity with the same name/key reuse starts from stale key material.

**Fix:** Do not leave the entity behind on a creator-root failure. Either (a) write the creator root first and only then create the entity, or (b) on `creatorOk == false`, apply a tombstone through `ApplySpaceState`/`ApplyRoomState` (or `DeleteScope` semantics) and `StoreRoomKey`-clear, then return the error. Add a test that forces `OnScopeCreated` to fail and asserts the catalog contains no live entity.

#### WR-14: `MintInvite` wraps a topic-derived key for child rooms the minter does not hold, and `RedeemInvite` overwrites the redeemer's real key with it

**File:** `src/lib/gcs_membership.cpp:1314-1338`; `src/lib/gcs_crypto.cpp:222-238`; `src/lib/gcs_membership.cpp:1738`
**Issue:** For a space invite, every child room is wrapped at mint time. When the minter has no stored key for a room, the else branch calls `crypto::DeriveRoomKey(topic)`, which itself falls back to the topic string as HKDF input (`gcs_crypto.cpp:226-233`). The minter therefore wraps `HKDF(topic)` and stamps it into the signed invite as if it were the room's real random key. This is not hypothetical: room keys are process-local and distributed only through invites, so an Admin who joined the space before a child room was created — or any node after a restart (CR-07, deferred) — has no stored key for rooms it did not create. On the redeemer side `StoreRoomKey` is called unconditionally (`:1738`), so an existing member who redeems an invite overwrites a working stored key with the topic-derived one; every message subsequently encrypted or decrypted with the real key fails, and the room's confidentiality degrades to the publicly derivable topic key.

**Fix:** Wrap only keys the minter actually holds. Skip the entry (and log) when `crypto::RoomKeyFor(topic)` has no value for a nested-room wrap, keeping the pre-membership fallback only for the standalone-room branch where the topic derivation *is* the room key. In `RedeemInvite`, do not overwrite a stored key: `if ( !crypto::RoomKeyFor(topic).has_value() ) { StoreRoomKey(...) }`. Add a test that mints with an empty key store for a nested room and asserts the invite carries no wrapped key for it.

#### WR-15: Explicit leave is irreversible — a re-invited member is silently left out of the derived join

**File:** `src/lib/gcs_membership.cpp:1942-1952`, `:1752-1770`, `:494-497`, `:1116-1120`
**Issue:** Nothing ever clears `explicit_leave`. `Leave` sets it with a subject signature (`:1942-1951`). `RedeemInvite` on an existing record preserves the field verbatim by design (`:1748`, `:1752-1755`), tombstones are rejected before the merge, and every authoring write (`ChangeRole`, `RemoveMember`, `Approve`, `OnScopeCreated`) copies the record without clearing it. `HasExplicitLeave` then permanently excludes the member from `DerivedJoinedTopics` (`gcs_core_ffi.cpp:820`). A member who leaves and is later re-invited redeems successfully, is told they joined, receives room keys — and never has `auto_join_rooms` membership again. D-02 defines leave as a derived-join toggle, not a lifetime ban, and no command, approval or invite path can undo it.

**Fix:** Treat a successful redeem by the subject as an explicit re-join: when `hasExisting` and `existing.explicit_leave()`, clear `explicit_leave` and `leave_signature` on the record before `WriteMemberPreservingAuthority`. Both fields are excluded from the authority payload (`:311-317`), so the existing authority signature stays valid, and the redeem is already executed by the subject's own wallet, preserving the "only the subject can flip the flag" rule (`:490-516`). Add a test: leave, then redeem a fresh invite, and assert `HasExplicitLeave` is false and the derived topic is present.

#### WR-01 (prior, still open): Membership changes never refresh the derived-join projection

**File:** `src/ffi/gcs_core_ffi.cpp:1762-1764`, `:1818-1844`, `:799-879`
**Issue:** `RefreshDerivedJoins` is still called only from `gcs_init` (`:1288`), `kCreateRoom` (`:1648`) and `kUpdateSpace` (`:1696`). `kRedeem` pushes only `SpaceTree` (`:1762-1764`) and `kLeave` pushes only `MemberList` (`:1842`); both mutators push through the sink but neither recomputes or pushes `RoomList`. A redeemer of a standalone-room invite is never registered on the room topic; a space-invite redeemer does not get the `auto_join_rooms` rooms in the pushed list; a leaver's derived rooms stay joined until restart. `RailCubit.selectRoom` rejects unlisted topics and the row is rendered disabled.

**Fix:** After a successful redeem/leave, call `RefreshDerivedJoins(lock)` and `PostToDart(BuildRoomListEvent())` (both arms re-acquire the lock). For a standalone-room invite, register `gcs/chat/<roomId>` listen-first/broadcast and add it to `g_roomTopics`/`g_explicitTopics`. Add an FFI/multinode test asserting the pushed `RoomList` contains the room before any other command.

#### WR-06 (prior, still open): The invite bearer gate is optional and wrong-key redemption still writes a member record

**File:** `src/lib/gcs_membership.cpp:1400-1417`, `:1566-1582`; `src/ffi/gcs_core_ffi.cpp:1187-1188`
**Issue:** Production wires `mintAuth`/`verifyAuth` (`gcs_core_ffi.cpp:1187-1188`), but both sides fail open when the seam is absent: `MintInvite` logs a warning and persists an invite with no authenticator, and `RedeemInvite` skips the gate entirely. A partially wired seam silently removes the "possess the bearer key" precondition for membership. All undecryptable wrapped keys are still skip-and-log, so a redeemer holding only the scope/invite ids writes a live GUEST record.

**Fix:** Fail closed in both directions when the seam is unset (mirroring `IssuerAuthEnabled`), and reject a room invite whose wrapped keys are all undecryptable instead of writing a keyless member record.

#### WR-07 (prior, still open): `ScopeHasRoster` fails open on a scan error

**File:** `src/ffi/gcs_core_ffi.cpp:752-756` (call sites `:1668`, `:1588`, `:1473` region)
**Issue:** `return members.has_value() && members.value().member_size() > 0;` maps a failed `MembersFor` scan to "no roster", which makes every caller skip its `RoleAtLeast` gate. A transient store read failure downgrades Member+/Admin+ checks to no check.

**Fix:** Return `outcome::result<bool>`; on error, post an error notice and reject the command (`GCS_ERROR_GENERIC`) instead of continuing ungated.

#### WR-08 (prior, still open): A failed member scan is pushed as an empty roster

**File:** `src/ffi/gcs_core_ffi.cpp:596-612`; `src/app/lib/cubits/members_cubit.dart:204-219`
**Issue:** `BuildMemberListEvent` converts a scan failure into a `MemberList` carrying only `scope_id`; `MembersCubit.setMembers` performs a full replacement, so the Dart roster is wiped and every role-gated affordance disappears until the next successful push.

**Fix:** Do not synthesize a successful empty roster on error. Push an error event, or carry a status the cubit honors by keeping the last good roster, and distinguish "scan failed" from "zero records".

#### WR-09 (prior, still open): The members dialog offers "change role" on the viewer's own row

**File:** `src/app/lib/shell/members_dialog.dart:318-319`
**Issue:** `showChangeRole = viewerCanManage && !targetIsSuperAdmin && !member.pending` still lacks the `!member.self` condition that `showRemove` has (`:320-325`). `CanChangeRole` always denies self-targeting, so the affordance is guaranteed to produce an error toast.

**Fix:** `viewerCanManage && !member.self && !targetIsSuperAdmin && !member.pending`. Add a widget test asserting the self row has no change-role affordance.

#### WR-10 (prior, still open): Deleting a space does not cascade to its rooms, and a nested room cannot be deleted alone

**File:** `src/lib/gcs_membership.cpp:1960-2034`, `:1249-1253`; `src/app/lib/cubits/rail_cubit.dart:196-211`
**Issue:** `DeleteScope` matches only the space record itself; child rooms stay live and `RailCubit.setTree` degrades a room with an absent parent to a standalone top-level row. A nested room also cannot be deleted directly because `CanDelete` resolves `LocalRoleFor(roomId)` and no member records exist under a nested room id.

**Fix:** In the space branch, apply the same pending/deleted state to every room with `parent_space_id() == scopeId` through `ApplyRoomState`, then push once. Authorize a nested-room delete against its parent scope.

### Info

#### IN-01 (prior, still open): Stale `kRedeem` comment

**File:** `src/ffi/gcs_core_ffi.cpp:1740-1743`
**Issue:** The comment says the scope id lives in the invite record, not the URL, and that the arm must never derive it from the URL; the URL has carried the scope segment since 04-15 and the arm only pushes the tree. The comment invites a regression.
**Fix:** Reword to state that the URL is the authoritative parse source while `RedeemInvite` owns the roster push.

#### IN-02 (prior, still open): Unnamed numeric literals in `PrivilegeRank`

**File:** `src/lib/gcs_membership.cpp:98-113`
**Issue:** `PrivilegeRank` still returns bare `5`/`4`/`3`/`2`/`1`/`0`. These are rank values, not the trivial `0`/`1`/`-1` the project rule exempts.
**Fix:** `constexpr unsigned int kRankSuperAdmin = 5;` … `kRankUnspecified = 0;` and return those.

#### IN-03 (prior, still open): All membership failures map to one error

**File:** `src/ffi/gcs_core_ffi.cpp:1786`, `:1812`, `:1838`, `:1864`, `:1891`
**Issue:** Every `outcome::failure` from `ChangeRole`/`RemoveMember`/`Leave`/`DeleteScope`/`Approve` becomes "permission denied" or `GCS_ERROR_INVALID_ARGUMENT`, including store write failures, missing targets and "no dormant op". `GcsDbError` is a single code, so neither the user nor the logs can distinguish causes.
**Fix:** Give membership mutators distinguishable error reasons and map them to distinct notices; at minimum reword the generic notice.

#### IN-04 (prior, still open): Every membership mutation pushes `MemberList` twice

**File:** `src/lib/gcs_membership.cpp:1011-1046` (sink push) and `src/ffi/gcs_core_ffi.cpp:1789`, `:1815`, `:1842`, `:1894`
**Issue:** Each mutator calls `PushMemberList` through the sink and each FFI arm posts `BuildMemberListEvent` again; `delete_scope`/`approve` also duplicate `SpaceTree`. Two identical full-roster events reach Dart per action.
**Fix:** Pick one owner: keep the sink push and drop the FFI posts, or the reverse.

#### IN-05 (prior, still open): `VerifyInviteAuthenticator` accepts any non-empty key length

**File:** `src/lib/gcs_crypto.cpp:515-521`
**Issue:** `MintInviteAuthenticator` rejects `size() != kRoomKeyLengthBytes` (`:487-489`); verify only rejects empty. A short key still derives an HKDF key, so the verify contract is weaker than the documented 32-byte invariant.
**Fix:** Apply the `kRoomKeyLengthBytes` check in `VerifyInviteAuthenticator` and before use in `RedeemInvite`.

#### IN-06: Dead/redundant clause in the third-party verification branch

**File:** `src/lib/gcs_membership.cpp:641-645`
**Issue:** Inside the `else` of `record.created_by() == address` (`:592`), the condition `record.created_by() == address` can never be true, so half of the guard is dead. The `depth >= kMaxMemberChainDepth` half duplicates the bound already enforced by `VerifiedRoleFor(... depth + 1)`.
**Fix:** Keep one explicit depth check (or document that the helper owns it) and drop the unreachable comparison.

#### IN-07: Entity-level `pending_action` is never cleared on activation

**File:** `src/lib/gcs_membership.cpp:2168-2171`, `:2188-2191`, `:1992`
**Issue:** `Approve` sets `approved_by`, `deleted` and `deleted_at_ms` but leaves `pending_action = DELETE` on the now-deleted record. The immediate `DeleteScope` branch likewise sets `deleted` without clearing a pre-existing pending DELETE. The stored state `deleted == true && pending_action == DELETE` contradicts the coherence rule the member path enforces (`:551-555`) and makes a record's pending status depend on which path activated it.
**Fix:** Clear `pending_action` (and `pending_role` where present) whenever `deleted` is set, in both the immediate and approved branches.

---

_Reviewed: 2026-10-19T18:45:00Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
