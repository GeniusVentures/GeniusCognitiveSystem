---
phase: 04-membership-invites
reviewed: 2026-10-12T19:07:33Z
depth: standard
files_reviewed: 30
files_reviewed_list:
  - src/lib/gcs_membership.hpp
  - src/lib/gcs_membership.cpp
  - src/lib/gcs_crypto.hpp
  - src/lib/gcs_crypto.cpp
  - src/lib/gcs_entity_store.hpp
  - src/lib/gcs_entity_store.cpp
  - src/proto/gcs_chat.proto
  - src/ffi/gcs_core_ffi.cpp
  - src/CMakeLists.txt
  - test/CMakeLists.txt
  - test/test_gcs_membership.cpp
  - test/test_gcs_membership_multinode.cpp
  - test/test_gcs_ffi.cpp
  - test/test_gcs_crypto.cpp
  - test/test_gcs_entities.cpp
  - src/app/lib/cubits/members_cubit.dart
  - src/app/lib/cubits/message_flow_cubit.dart
  - src/app/lib/cubits/rail_cubit.dart
  - src/app/lib/cubits/session_cubit.dart
  - src/app/lib/shell/gcs_shell.dart
  - src/app/lib/shell/join_dialog.dart
  - src/app/lib/shell/members_dialog.dart
  - src/app/lib/shell/room_rail.dart
  - src/app/lib/generated/proto/gcs_chat.pb.dart
  - src/app/lib/generated/proto/gcs_chat.pbenum.dart
  - src/app/lib/generated/proto/gcs_chat.pbjson.dart
  - src/app/test/shell/join_dialog_test.dart
  - src/app/test/shell/members_dialog_test.dart
  - src/app/test/chat_shell_test.dart
  - src/app/test/cubits/shell_cubits_test.dart
findings:
  critical: 1
  warning: 7
  info: 4
  total: 12
status: issues_found
---

# Phase 04: Code Review Report

**Reviewed:** 2026-10-12T19:07:33Z
**Depth:** standard
**Files Reviewed:** 30
**Status:** issues_found

## Summary

The Phase 4 membership domain (five-tier roles, bearer-key invites, write-time guards, CRDT roster records) is substantively implemented: the C++ domain owner carries thorough write-time guards, the crypto key-wrap seam is clean (fresh EVP contexts, checked returns, mutex-guarded key store), the proto and generated Dart are regeneration-consistent, and both C++ and Dart test suites follow the wait-condition discipline. The design decisions D-01 through D-08 are traceable in code and tests.

However, one Critical concurrency defect and several authorization/validation gaps must be fixed before this ships:

1. **CR-01** — the Phase 4 FFI dispatch arms run `Membership` mutators with `g_mutex` released (correct per the ABBA rule), but those mutators iterate the `EntityStore` catalog (`ResolveScope`), which entity-mutating arms (`kCreateSpace`/`kCreateRoom`/`kUpdateSpace`) mutate under `g_mutex`. Concurrent `std::map` iteration + insertion is undefined behavior on any interleaving of, e.g., a room creation with a role change.
2. `g_membership` is never evicted in `TeardownSessionAndNode`, violating the documented eviction invariant and leaving it holding references to destroyed objects.
3. The role guards miss target-privilege checks: a Moderator can remove an Admin, and demoting a Super Admin bypasses the D-07 dormant-approval rule entirely.
4. Space-scope rosters replicate to a topic no node subscribes to, so the Members dialog for a space only ever shows local truth — the replication gap is documented in a comment but is a real functional hole for the phase's stated CRDT-roster goal.

## Critical Issues

### CR-01: Data race — Membership mutators read the EntityStore catalog outside `g_mutex` while entity arms mutate it under `g_mutex`

**File:** `src/ffi/gcs_core_ffi.cpp:1559-1745` (unlock sites at 1570, 1600, 1626, 1652, 1679, 1705, 1732; also 1392), racing `src/ffi/gcs_core_ffi.cpp:1371-1373, 1443-1444, 1535`; read side `src/lib/gcs_membership.cpp:306-333`

**Issue:** Every Phase 4 arm copies `g_membership.get()` under the lock, calls `lock.unlock()`, then invokes the mutator (`MintInvite`, `RedeemInvite`, `ChangeRole`, `RemoveMember`, `LeaveScope`, `DeleteScope`, `Approve`, and `OnScopeCreated` from `kCreateSpace` at line 1393). Each of these mutators calls `Membership::ResolveScope` (gcs_membership.cpp:306-333), which iterates `m_entities.Spaces()` and `m_entities.Rooms()` — the `EntityStore`'s internal `std::map` members — with no lock held. Meanwhile `kCreateSpace` (line 1371), `kCreateRoom` (line 1443), and `kUpdateSpace` (line 1535) call `CreateSpace`/`CreateRoom`/`UpdateSpace`, which insert/modify those same `std::map`s under `g_mutex`. A concurrent publish of `create_room` while another thread executes `change_role` therefore performs concurrent read-iteration and write-insertion of the same `std::map` — undefined behavior (heap corruption/crash), not a benign race. The receive-callback path also reads the catalog under `g_mutex`, adding a third party to the race.

**Fix:** Do not let the unlocked Membership mutators touch live catalog structures. The smallest behavior-preserving fix is to snapshot what `ResolveScope` needs before releasing the lock, e.g. give `Membership` a scope-resolution input captured under `g_mutex`:

```cpp
// In each Phase 4 arm, while the lock is still held:
const gcs::Membership::ScopeInfo scope = g_membership->ResolveScope(scopeId); // const, reads only — safe under lock
gcs::Membership* membership = g_membership.get();
g_inFlight += 1;
lock.unlock();
const bool ok = membership->ChangeRoleResolved(scope, target, role).has_value();
lock.lock();
```

Alternatively, make `EntityStore::Spaces()`/`Rooms()` return a copy of the catalog taken under an internal `std::mutex` owned by `EntityStore` (the catalog maps are private; the accessor can lock internally and return by value). Either approach removes the shared-structure read from the unlocked window; the copy approach is the smaller diff and protects every current and future unlocked caller.

## Warnings

### WR-01: `g_membership` is never evicted in `TeardownSessionAndNode` — dangling references to destroyed session/entities and a per-cycle leak

**File:** `src/ffi/gcs_core_ffi.cpp:338-350` (eviction block), `src/ffi/gcs_core_ffi.cpp:384-387` (destruction), `src/ffi/gcs_core_ffi.cpp:1127-1128` (reference binding)

**Issue:** The teardown invariant documented at lines 333-337 is "evict the guarded globals … any late callback or command that acquires `g_mutex` afterwards sees null globals and no-ops." The eviction block moves out `g_session`, `g_entities`, and `g_messaging` only; `g_membership` (declared at line 105, constructed at line 1127 binding `*g_session` and `*g_entities` by reference) is never reset. After `messaging.reset(); session->Shutdown(); entities.reset(); session.reset();` (lines 384-387), the surviving `g_membership` holds references to destroyed objects — any path that reads `g_membership` after teardown and before the next `gcs_init` overwrites it (lines 577, 723, 789) dereferences freed memory. The object also leaks across every init/shutdown cycle until the next init destroys it incidentally.

**Fix:** Add `g_membership` to the same eviction-and-destruction chain, destroyed first (it only reads the others):

```cpp
std::unique_ptr<gcs::CoreSession> session;
std::unique_ptr<gcs::EntityStore> entities;
std::unique_ptr<gcs::Messaging> messaging;
std::unique_ptr<gcs::Membership> membership;
if ( g_session != nullptr )
{
    g_dartPort = 0;
    session    = std::move( g_session );
    entities   = std::move( g_entities );
    messaging  = std::move( g_messaging );
    membership = std::move( g_membership );   // NEW
    // ... topic-set clears
}
// ... later, in the unlocked destruction phase (before messaging.reset()):
membership.reset();
```

### WR-02: `CanRemoveMember` has no target-privilege check — a Moderator can remove an Admin, an Admin can remove a non-creator Super Admin

**File:** `src/lib/gcs_membership.cpp:556-571`

**Issue:** The guard checks only the author's tier (`SUPER_ADMIN || ADMIN || MODERATOR`) plus the creator exemption (D-03). It never compares the author's tier against the target's current role. Consequences: a Moderator removes an Admin or a non-creator Super Admin; an Admin removes a non-creator Super Admin; a Moderator can tombstone the sole Admin of a scope. The D-08 five-tier model implies rank-ordered authority; removal should respect it just as `CanChangeRole` does (line 546 explicitly denies Admin granting the Super Admin tier, showing the intended ordering).

**Fix:** Read the target's record and require the author to strictly outrank (or equal, per the intended policy) the target:

```cpp
const chat::Role author = LocalRoleFor( scopeId );
chat::MemberRecord targetRecord;
if ( !TryReadMember( scopeId, target, targetRecord ) || targetRecord.deleted() )
{
    return false;
}
if ( author != chat::ROLE_SUPER_ADMIN
     && PrivilegeRank( targetRecord.role() ) <= PrivilegeRank( author ) )
{
    return false; // may only remove strictly lower-tier members
}
return author == chat::ROLE_SUPER_ADMIN || author == chat::ROLE_ADMIN
       || author == chat::ROLE_MODERATOR;
```

(If Moderators are intended to remove only Members/Guests, the rank check encodes exactly that.) Add unit tests covering Moderator-vs-Admin and Admin-vs-SuperAdmin removal attempts.

### WR-03: Demoting a Super Admin bypasses the D-07 dormant-approval rule

**File:** `src/lib/gcs_membership.cpp:879-887`

**Issue:** `isAdminDemotion` is computed as `record.role() == chat::ROLE_ADMIN && PrivilegeRank(role) < PrivilegeRank(ROLE_ADMIN)`. A non-creator Super Admin demoted to Member (or anything below) fails the equality test, so the `else` branch applies the demotion immediately even when `AdminCount(scopeId) > 1`. D-07 says destructive ops with multiple admins go dormant pending Super Admin approval; demoting a Super Admin is at least as destructive as demoting an Admin, so the bypass is a policy hole, not just an edge case.

**Fix:** Test privilege rank, not enum equality:

```cpp
const bool isAdminDemotion = PrivilegeRank( record.role() )
                                 <= PrivilegeRank( chat::ROLE_ADMIN )
                             && PrivilegeRank( role )
                                    < PrivilegeRank( record.role() );
```

Add a unit test: two admins plus a second Super Admin; demote the Super Admin; expect a dormant `PENDING_ACTION_ROLE_CHANGE` record, not an immediate role write.

### WR-04: `RedeemInvite` matches the inviteId across all scopes, never cross-checks the URL's scope kind, and the bearer key does not gate the membership write

**File:** `src/lib/gcs_membership.cpp:726-808` (kind parsed and validated lexically at 726-739, then unused), `777-794` (cross-scope scan), `810-849` (key used only for unwrap; member record written unconditionally at 838+)

**Issue:** Three compounding validation gaps on the redeem path:

1. The URL's `{space|room}` kind segment is parsed and lexically validated (lines 732-739) but never compared against anything in the located `InviteRecord` — the record's own `scope_id` silently wins. The kind segment is dead weight as an authority input.
2. The scan at lines 777-794 matches ONLY the inviteId segment across every record under `gcs/invites/` and takes the first hit in unordered iteration order. Invite IDs are `inv-<seed>-<seq>` where `seed` is per-process (line 288-298), so two nodes minting concurrently can, with a seed collision, produce colliding IDs — and the redeemer then joins whichever scope iteration happens to yield first. There is also no check that the invite's scope still exists.
3. The bearer key from the query string is exercised only inside the room-key unwrap loop; a failed unwrap is skip-and-log (by design, T-04-03-T-2) and the redeemer's member record is still written at lines 838-849. The invite URL's secret therefore never authenticates the redeem — knowledge of the inviteId alone grants membership with the invite's stamped role.

**Fix:** In `RedeemInvite`: (a) after locating the record, cross-check the record against the URL — `const bool kindIsSpace = scopeKind == kScopeKindSpace; if (kindIsSpace != ResolveScope(invite.scope_id()).isSpace) reject;` (b) key the scan by the full composite key (`InviteKey(invite.scopeId, inviteId)` is only derivable if the URL carries the scope; at minimum compare `SegmentAfterMarker(entry.first, kInvitesKeyPrefix, 0)` against a scope hint when one is available, and reject collisions); (c) if `KeyWrapEnabled()`, require at least one successful unwrap before writing the member record, or store a keyed hash of the invite key in the `InviteRecord` and verify it — the bearer secret must gate the join itself, not just key transport.

### WR-05: `OnMemberArrived` early-returns on tombstones without pushing the roster — remote removals never converge in the UI

**File:** `src/lib/gcs_membership.cpp:1159-1162`

**Issue:** When a replicated member record arrives with `deleted() == true`, the function returns before `PushMemberList`. A remote `RemoveMember` therefore never triggers a local `MemberList` push, so every other node's Members dialog keeps rendering the removed member indefinitely (until some unrelated roster write happens to push). The local `RemoveMember` path pushes on its own write, so tests pass, but the convergence path for the tombstone arrival is silently dropped.

**Fix:** Push on tombstones too — the roster snapshot computed by `PushMemberList` presumably filters deleted records, so the push after a tombstone is exactly what conveys the removal:

```cpp
if ( record.deleted() )
{
    const auto tombstonePush = PushMemberList( scopeId );  // was: return;
    if ( !tombstonePush.has_value() )
    {
        spdlog::warn( "gcs_membership: member-list push failed for scope '{}'",
                      scopeId );
    }
    return;
}
```

(If `PushMemberList` includes tombstoned records, filter them there — either way the tombstone arrival must produce a push.) Cover with the multinode test: remove on node A, expect node B's roster to converge without a local write.

### WR-06: Space-scope member records replicate to `gcs/chat/<spaceId>`, a topic no node subscribes to — space rosters never leave the minting node

**File:** `src/lib/gcs_membership.cpp:56-67` (`ScopeTopics`)

**Issue:** `ScopeTopics` returns `gcs/chat/<scopeId>` for every scope. Rooms have registered topics (subscribe/join machinery), but a space's id never does — the header comment at lines 56-59 admits "A space scope's id has no registered room topic today, so its records land locally only." Consequences for this phase's stated goal (CRDT-replicated roster records): a space's `MemberList`, invite records, dormant-op approvals, and explicit-leave tombstones never replicate; a second node joining the space sees an empty roster, cannot resolve roles for nested-room guards (`LocalRoleFor` reads only the local DB), and its Members dialog shows only its own wallet. This is a functional hole in the phase's core deliverable, not a documentation nit.

**Fix:** Register a replication channel for space scopes and use it in `ScopeTopics`. The minimal change consistent with D-02: when a space is created (or its roster first written), subscribe the node to a dedicated members topic (e.g. `gcs/members/<spaceId>` or the space's room topic once the entity model gains one) and have `ScopeTopics` return that set for space scopes. If a deliberate scope restriction defers space-roster replication to a later phase, the guard arms that depend on it (`CanChangeRole`, `CanRemoveMember`, nested-room creation gates in `gcs_core_ffi.cpp:1435-1436`) must treat an empty local roster as "no authority" rather than "open" — today a node that simply hasn't seen the roster grants Moderator-tier powers it was never given.

### WR-07: Stale invite link rendered in the invite view — `pendingInviteLink` is global, unscoped, and never cleared

**File:** `src/app/lib/cubits/session_cubit.dart:163-167, 516`; `src/app/lib/shell/members_dialog.dart:560-585, 635`

**Issue:** `_startInvite` publishes the mint for the tapped scope and immediately opens the invite view titled "Invite to `<scopeName>`", which renders `session.pendingInviteLink`. That field is set on every pushed `InviteLink` (line 516) and never cleared or associated with a scope. Two failure modes: (a) the user mints for space A, later mints for room B — B's view opens before B's link arrives and renders A's link under B's title (the user copies a link that joins the wrong scope); (b) after any mint, the stale link persists forever and re-surfaces in the next mint's view if the new push is slow or lost. The "view never invents a link" doc comment is true but insufficient — it can render the previous scope's real link.

**Fix:** Key the pending link by scope and clear-on-read or expire it. Minimal fix:

```dart
// session_cubit.dart
final Map<String, String> _pendingInviteLinks = <String, String>{};
String? inviteLinkFor(String scopeId) => _pendingInviteLinks.remove(scopeId);

// dispatch site (line 516):
_pendingInviteLinks[event.inviteLink.scopeId] = event.inviteLink.url;

// members_dialog.dart _showInviteView: read session.inviteLinkFor(scopeId)
// and render the loading sub-state while null (never a previous scope's link).
```

If `InviteLink` does not carry `scopeId`, add it to the proto event (it already exists on the command side) rather than inferring.

## Info

### IN-01: `Base64UrlDecode` accepts non-canonical encodings (trailing bits not validated)

**File:** `src/lib/gcs_membership.cpp:228-250`

**Issue:** The decoder discards the trailing bits of a non-canonical final character instead of rejecting them. Two different encodings of the same invite key therefore both decode successfully, and minter/redeemer base64url variants that differ in canonicality silently interoperate while a strict peer would reject. Not exploitable here (keys are high-entropy random), but it weakens the "exactly one encoding" property at a security-relevant boundary.

**Fix:** In the final-character branch, verify the discarded bits are zero and fail (return `std::nullopt`) otherwise: `if ((decoded << usedBits) != 0) return std::nullopt;` adapted to the bit-buffer implementation.

### IN-02: Exit-path `g_tearingDown` latch permanently refuses re-init in a continuing process

**File:** `src/ffi/gcs_core_ffi.cpp:290-291, 318-328, 389-393`; gate consumer `gcs_core_ffi.cpp:861-864`

**Issue:** `g_tearingDown.store(true)` at line 291 runs on every path, but the reset at lines 389-393 executes only when `lock != nullptr` (the `gcs_shutdown` path). On the process-exit path the flag stays true — harmless at true process death, but if the exit hook fires in an embedder that continues running (Flutter hot restart is exactly this shape), every subsequent `gcs_init` returns nullptr at line 861 with no recovery and no log.

**Fix:** Reset the flag on the exit path as well, after the unlocked destruction phase completes (e.g. a `std::lock_guard<std::mutex> relock( g_mutex ); g_tearingDown.store( false );` tail, or an atomic exchange guarded so only the exit path that completed teardown clears its own latch). At minimum, log at line 862 so a wedged embedder is diagnosable.

### IN-03: `roleLabel` maps `ROLE_UNSPECIFIED` to "Member"

**File:** `src/app/lib/cubits/members_cubit.dart:36`

**Issue:** `kRoleLabels[role] ?? kRoleLabelMember` renders an unspecified role as "Member". An unspecified role in a roster record indicates protocol drift or a partially-written record; displaying it as a real tier masks the anomaly from the user and from support triage.

**Fix:** Give unspecified its own label: `const String kRoleLabelUnknown = 'Unknown';` and map `Role.ROLE_UNSPECIFIED` (and the null fallback) to it, so drift is visible rather than silently normalized.

### IN-04: `truncateSender` unconditionally prepends `'0x'` and drops the first two characters of any sender that lacks the prefix

**File:** `src/app/lib/cubits/message_flow_cubit.dart:63-73`

**Issue:** The truncation branch returns `'0x${address.substring(2, ...)}…${suffix}'` — the literal `0x` and the `substring(2, …)` hardcode the EVM convention. A long sender identity without the `0x` prefix (future identity scheme, test fixture, mock wallet) is displayed with a bogus `0x` and its true first two characters silently dropped. Display-only, but the same helper is the shared home for the Phase 3 sender label and the Phase 4 member-row address, so the assumption propagates everywhere.

**Fix:** Preserve the actual prefix: `return '${address.substring(0, 2 + kSenderShortPrefixLength)}…' '${address.substring(address.length - kSenderShortSuffixLength)}';` — or guard the `0x` fast path behind `address.startsWith('0x')`.

---

Files with no findings: `src/lib/gcs_crypto.hpp/.cpp` (clean — fresh EVP contexts, checked returns, mutex-guarded key store), `src/lib/gcs_entity_store.hpp/.cpp`, `src/proto/gcs_chat.proto`, all three generated `gcs_chat.*.dart` files (regeneration-consistent — every Phase 4 message, enum, and 12-arm oneof verified present), both `CMakeLists.txt` files (membership sources and both test targets registered), `src/app/lib/cubits/rail_cubit.dart`, `src/app/lib/shell/gcs_shell.dart`, `src/app/lib/shell/join_dialog.dart`, `src/app/lib/shell/room_rail.dart`, and all five test files (wait-condition discipline throughout, meaningful assertions; no reliability defects).

_Reviewed: 2026-10-12T19:07:33Z_
_Reviewer: Claude (gsd-code-reviewer)_
_Depth: standard_
