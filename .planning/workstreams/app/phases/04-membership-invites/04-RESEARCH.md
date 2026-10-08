# Phase 4: Membership & Invites - Research

**Researched:** 2026-09-27
**Domain:** P2P membership + capability-token invites over a per-key-LWW CRDT (C++ core owns state; Flutter thin subscriber)
**Confidence:** HIGH

## Summary

Phase 4 introduces the member roster for the first time and, riding along, closes the Phase 3 WR-02 room-key defect (keys stop being derivable from the public topic). Every requirement in this phase is satisfied by **extending existing in-repo components** — no new libraries, no new algorithms, no new FFI ABI surface. The C++ side gains a membership component (mirroring `gcs_entity_store`/`gcs_messaging`), a mutex-guarded per-room key store + key-wrap functions in `gcs_crypto`, and new `GcsCommand`/`GcsEvent` oneof arms. The Dart side gains two shell widgets and a members cubit, all composed from existing `frontend_scaffold` atoms.

The single most load-bearing fact of this phase is the **per-key LWW storage model**: GlobalDB keeps only state with no union semantics, so every locked decision (per-member records not op-logs, embedded `approved_by` not a cross-key approval record, full-state role rewrites not patches) is a direct consequence. My research confirms all eight decisions (D-01..D-08) are implementable with the primitives already in the repo, and surfaces **seven concrete gaps** the planner must address — most importantly that `SpaceRecord`/`RoomRecord` lack a `creator` field today, that standalone rooms are member scopes yet D-03 only names `SpaceRecord`, and that `members_can_invite` (required by MEMB-05) has no storage field or config command yet.

**Primary recommendation:** Build a new `gcs::Membership` component (per-member LWW records under `gcs/members/<scopeId>/<wallet>`, enumerated by the Phase 3 `QueryKeyValues` prefix scan) with write-time permission guards that also retroactively gate the existing Phase 2/3 commands; add `creator` to **both** `SpaceRecord` and `RoomRecord`; persist an invite-key-addressed `InviteRecord` at invite time (the inviter is the only party who knows the room key to wrap) and write the redeemer's wallet-keyed member record at redeem (RESOLVED — see Open Questions Q1); and register a `RegisterNewElementCallback` on `/gcs/members/.*` so remote joins converge into a pushed `MemberList` instead of appearing only after session re-init.

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| MEMB-01 | User can invite others via capability token (`gcs://invite/...`) | D-05/D-06: bearer-key URL minted C++-side (`RAND_bytes`), room key AES-256-GCM-wrapped under the invite key. §Standard Stack (OpenSSL), §Pitfalls P4/P7, §Code Examples (key wrap). |
| MEMB-02 | Space/room has roles: Super Admin, Admin, Moderator, Member, Guest | D-01/D-03/D-08: per-member LWW `MemberRecord` + new `Role` enum + write-time permission matrix. §Architecture Patterns, §Security V4, Pitfall P1/P2 (creator gap). |
| MEMB-03 | Super Admin (creator) cannot be demoted by other admins | D-03: C++ write guard rejects any demotion targeting `creator`; requires a `creator` field that does not exist today (Pitfall P1/P2). |
| MEMB-04 | Destructive actions require Super Admin approval when 2+ admins exist | D-07: embedded `approved_by` on the target record, dormant-until-approved; single-signer overwrite survives per-key LWW. Pitfall P6. |
| MEMB-05 | Members can invite others if `membersCanInvite` is enabled | D-08: `membersCanInvite` gate in the C++ mint path. The config field does not exist today (Pitfall P3). |
</phase_requirements>

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

**Phase Boundary:** Users can invite others via capability-token links (`gcs://invite/...`); recipients redeem the invite, join, and appear in the member list; every participant has a visible five-tier role (Super Admin, Admin, Moderator, Member, Guest) that gates actions; the Super Admin (creator) cannot be demoted; destructive actions require Super Admin approval when 2+ admins exist (MEMB-01…MEMB-05). Riding along, because the member roster exists for the first time: the WR-02 room-key swap (Phase 3 deferred item — room keys stop being derivable from the public topic and become member-distributed) and the Phase 2 D-04 deferral (explicit-leave precedence for derived joins). No kick/ban/message-deletion flows (Phase 5), no GCS bot (Phase 6), no lobby/discovery (Phase 7), no key rotation on member leave (ENCR-02, v2), no sender signing (v1.1), no DM auto-invites.

- **D-01:** Per-member LWW record per scope. One serialized `MemberRecord` per member per scope under `gcs/members/<scopeId>/<wallet>` (scope = a space or a standalone room), enumerated by the Phase 3 `QueryKeyValues` prefix scan — no manifest (mirrors `gcs/messages/` at `gcs_messaging.hpp:82`). Rejected: the architecture note's op-based membership log (§Membership CRDT Op) — GlobalDB is per-key LWW with no union semantics, so a shared op-log key silently drops concurrent ops, and the merge reducer has no in-repo precedent.
- **D-02:** Room access stays derived, never per-room recorded. `joined(room) = parentSpace.autoJoinRooms && !member.explicitLeave` — the exact rule Phase 2 D-04 deferred here. `explicit_leave` is a bool on the space-scope member record; retoggle-resurrect (a leave resurrecting if config toggles false→true→false→true) is documented as intended. Rejected: explicit per-room membership records (event storm; the derived evaluator already exists).
- **D-03:** Role changes are full-state LWW rewrites; Super Admin protection is a C++ write guard. Promotion/demotion rewrites the member record (the `UpdateSpace` full-state pattern, `gcs_entity_store.cpp` ~265-294); GlobalDB keeps only state, so the "Super Admin cannot be demoted" invariant is enforced by a C++ write-time guard rejecting any demotion targeting the creator — never by CRDT semantics. `SpaceRecord` gains a `creator` field (load-bearing gap found by research: it does not exist today, and both this guard and the D-07 approval check verify against it). Member identity = wallet address (`GeniusNode::GetAddress()`, `gcs_core_ffi.cpp` ~885) — no new identity surface this phase.
- **D-04:** Member list reaches Dart as one flat `MemberList` event on the `GcsEvent` oneof (the `SpaceTree` push pattern): full replacement per scope, pushed on redeem, on any membership write, and at session init after CRDT replay.
- **D-05:** Bearer key in the URL — the architecture note's literal shape. `gcs://invite/{space|room}/<id>?key=<key>` where the key is a random (RAND_bytes) per-member symmetric key. Multi-use, no expiry, no consume records for MVP; revocation = removing the member record (tombstone). Role grant (Guest default) and `membersCanInvite` enforcement live in the C++ mint/redeem path writing the CRDT membership record — never in the token. Rejected: HMAC claim tokens — on a serverless network every member holds the shared room key, so any member could forge any role; and the bearer key doubles as the D-06 room-key delivery vehicle, which claim tokens don't solve.
- **D-06:** Per-member AES-GCM key-wrap replaces the topic-derived key. Each room gets a random 32-byte key minted at creation; that key becomes the HKDF input keying material instead of `roomTopic` (`gcs_crypto.cpp:46-49` — the WR-02 defect site). `kMessagesHkdfSalt`/`kMessagesHkdfInfo` stay STABLE and the D-08 envelope (nonce(12)‖ct‖tag on both live Publish and at-rest Put) is unchanged — only the ikm swaps. Each member record carries the room key AES-256-GCM-wrapped under that member's invite-borne key; decryption stays behind the injected CryptoSeam in the local Messaging layer; wrong-key records skip-and-log (unchanged). `gcs_crypto` gains a mutex-guarded per-room key store (a documented exception to its current statelessness). No identity keypairs this phase — Megolm-style session machinery only serves the deferred v1.1 signing / v2 rotation features.
- **D-07:** Embedded `approved_by` on the target record; dormant-until-approved. Phase 4 destructive scope = admin demotion, member removal, space/room deletion (kick/ban/message-delete are Phase 5). When 2+ admins exist, the author writes the op dormant (empty `approved_by`) and the Super Admin activates it by overwriting the same key with their id — single-signer approval means per-key LWW never drops a concurrent approval. Sole-admin case acts alone; no timeouts (locked rule table, architecture note §Destructive Action Rules). Enforcement is a write-time local check on every node plus tombstone-skip-style read gating for unapproved records — dormant, not reject-and-ignore, so the Super Admin always has something to approve. Rejected: a separate pending-approval record + flip command — its cross-key join can diverge under LWW and it only pays off with multi-approver quorum (not Phase 4).
- **D-08:** Permission matrix per the architecture note, gated at C++ write time. Super Admin (creator, singular): everything; Admin: config, membership, moderation-grant; Moderator: member management + message deletion (effect lands Phase 5); Member: read, write, invite when `membersCanInvite`; Guest: read-only — enforced as a send-path write guard. UI affordances render from the pushed role; they are presentation, not enforcement.

### Claude's Discretion

- Proto message/field naming and numbering within the append-only discipline (`MemberRecord`, `MemberList`, invite/redeem/membership commands, `SpaceRecord.creator`).
- Exact member-key prefix layout and wrapped-key encoding details.
- Whether a space invite wraps all room keys at redeem or lazily on first room join.
- Invite UI entry points and member-list surface (designed by the UI phase — `ui_phase` is enabled).
- How pending approvals surface in the pushed event plane (dedicated event arm vs error/toast reuse).
- Where the permission matrix lives in C++ (a table on the membership component vs per-call guards).
- Redeem-command shape and the invite-link clipboard/share mechanics on the Flutter side.

### Deferred Ideas (OUT OF SCOPE)

- Key rotation on member leave (ENCR-02) — v2; the D-06 record shape must not preclude it.
- Cryptographic sender signing + identity keypairs (Phase 3 D-04) — v1.1; revisit Megolm-style distribution then.
- Single-use / expiring invite tokens + nonce consume records — post-MVP if invite-leak abuse shows up.
- Kick/ban/message-deletion flows — Phase 5 Moderation (Moderator's deletion capability takes effect there).
- Member-list UI polish, invite share mechanics — UI phase design territory.
- Per-room join/leave audit history — only if a hard audit requirement lands.
- DM implicit invites — DMs don't exist yet.
- Lobby/discovery — Phase 7.
</user_constraints>

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Member record persistence | Database/Storage (GlobalDB CRDT) | API/Backend (C++ core) | `MemberRecord` serialized under `gcs/members/<scopeId>/<wallet>`, per-key LWW (D-01) |
| Role / permission enforcement | API/Backend (C++ write guard) | — | D-08: C++ owns state; Dart renders, never enforces |
| Super Admin protection | API/Backend (C++ write guard) | — | D-03: guard rejects demotion targeting `creator` |
| Approval (dormant/activate) | API/Backend (C++ write guard) | Database/Storage | D-07: write dormant (empty `approved_by`), Super Admin overwrites |
| Invite token minting | API/Backend (C++ `RAND_bytes`) | — | C++ stamps authority; key never from Dart (D-05/D-27) |
| Room-key wrap / unwrap / derivation | API/Backend (C++ `gcs_crypto`) | — | behind injected CryptoSeam; Messaging never touches OpenSSL (D-06) |
| Member enumeration | API/Backend (`QueryKeyValues`) | Database/Storage | prefix scan mirrors `QueryHistory` (`gcs_messaging.cpp:309-312`) |
| Member-list rendering | Browser/Client (Flutter) | — | dumb read of pushed `MemberList` (D-04) |
| Invite URL parse + clipboard | Browser/Client (Flutter) | API/Backend (re-validation) | UI parses for inline validation; C++ re-validates on redeem |
| Derived room access (`joined()`) | API/Backend (C++ evaluator) | — | D-02 extends Phase 2 `DerivedJoinedTopics` with `!explicitLeave` |

## Standard Stack

### Core

| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| OpenSSL (vendored, `OpenSSL::Crypto` imported target) | 3.3.3 | AES-256-GCM key-wrap, HKDF-SHA256, `RAND_bytes` | Already the sanctioned crypto handle (STATE.md: "OpenSSL::Crypto imported target (vendored 3.3.3) is the only sanctioned handle"); D-06 composes entirely from `gcs_crypto.cpp` primitives [VERIFIED: src/lib/gcs_crypto.cpp] |
| protobuf (`gcs_proto`, `add_proto_library`) | vendored | `MemberRecord`/`MemberList`/`Role`/command+event arms | Append-only wire contract; existing `GcsCommand`/`GcsEvent` oneofs extended [VERIFIED: src/proto/gcs_chat.proto] |
| spdlog | vendored | diagnostics | Mandated: never `fprintf`/`cout`; `spdlog::debug/error/warn` [VERIFIED: global CLAUDE.md directive] |
| GTest + `gcs::test::WaitForCondition` | vendored | tests | Wait-condition template replaces `sleep_for`; `gcs_test()` macro [VERIFIED: test/CMakeLists.txt, test/test_gcs_entities.cpp:186] |
| Flutter + `flutter_bloc` | 9.0.0 | cubits + pushed-event dispatch | `SessionCubit._dispatchEvent`, `RailCubit.setTree` patterns extended [VERIFIED: src/app/pubspec.yaml:42, session_cubit.dart] |
| `protobuf` (Dart) + `protoc_plugin` | 4.0.0 / 22.5.0 | generated `gcs_chat.pb.dart` | pre-existing; regenerated on proto change [VERIFIED: src/app/pubspec.yaml:46,61] |
| `frontend_scaffold` (in-tree submodule) | git-pinned | dialogs/badges/pressables | UI-SPEC contract: compose atoms, never edit `lib/` [CITED: 04-UI-SPEC.md, scaffold/CLAUDE.md] |

### Supporting

| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| `ffi` (Dart) | 2.2.0 | FFI bindings to `gcs_ffi` | already wired in `SessionCubit`; no change needed |
| `Clipboard` (`package:flutter/services.dart`) | SDK | copy invite link | "Copy link" action in invite view |

### Alternatives Considered

| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| AES-256-GCM key-wrap (D-06, locked) | RFC 3394 AES-KW (`EVP_aes_256_wrap`) | D-06 is locked to AES-GCM; it reuses the exact `EncryptPayload`/`DecryptPayload` envelope already in `gcs_crypto.cpp` — zero new OpenSSL API surface. AES-KW would be a second, unused primitive |
| Op-based membership log (architecture note) | (rejected by D-01) | per-key LWW drops concurrent ops on a shared log key; per-member records are the only LWW-safe shape |
| HMAC claim tokens | (rejected by D-05) | on a serverless network every member holds the room key → forge-any-role; bearer key doubles as key-delivery vehicle |

**Installation:** none. Zero new packages this phase. All dependencies are vendored (`thirdparty`/`OpenSSL::Crypto` target) or pub-cached (`ffi`, `flutter_bloc`, `protobuf`, `protoc_plugin`) and already in `pubspec.yaml`.

**Version verification:** vendored OpenSSL 3.3.3 confirmed via STATE.md "Accumulated Context" (Phase 03 note, 2026-09). Dart deps confirmed via `src/app/pubspec.yaml` read this session. No `npm view`/`pip`/`cargo` lookup required — no new installs.

## Package Legitimacy Audit

> No external packages are installed this phase. The audit is N/A — but recorded for the planner's verification gate.

| Package | Registry | Age | Downloads | Source Repo | slopcheck | Disposition |
|---------|----------|-----|-----------|-------------|-----------|-------------|
| *(none)* | — | — | — | — | — | — |

- **Packages removed due to slopcheck [SLOP] verdict:** none
- **Packages flagged as suspicious [SUS]:** none
- **Pre-existing dependencies (not new installs, no audit required):** OpenSSL 3.3.3 (vendored), protobuf (vendored), spdlog (vendored), GTest (vendored), Dart `ffi ^2.2.0`, `flutter_bloc ^9.0.0`, `protobuf ^4.0.0`, `protoc_plugin ^22.5.0`. All pinned in `pubspec.yaml` / CMake and already used by Phases 1–3.

## Architecture Patterns

### System Architecture Diagram

```
Invite mint (creator/admin)                    Redeem (recipient)
  gcs://invite/{space|room}/<inviteId>?key=<IK> ────────►  paste URL, publish redeem cmd
        │                                                    │
        ▼                                                    ▼
  C++ Membership::MintInvite                          C++ Membership::RedeemInvite
    - RAND_bytes → IK (invite key)                       - parse inviteId+IK (re-validate)
    - wrap room key K under IK (AES-256-GCM)             - scan InviteRecord by inviteId
    - write InviteRecord (role, scope, wrapped K)        - unwrap K under IK
        under gcs/invites/<scopeId>/<inviteId> ── Put ──►  - store K in per-room key store
    - push InviteLink event to Dart                      - write MemberRecord under
                                                           gcs/members/<scopeId>/<wallet>
                                                         - push MemberList event to Dart
        │                                                        │
        ▼                                                        ▼
  GlobalDB (per-key LWW)  ── converge via DAG ──►  RegisterNewElementCallback("/gcs/members/.*")
        │                                                        │
        ▼                                                        ▼
  Messaging::EncryptPayload/DecryptPayload            Messaging decrypts with per-room K
    room key = HKDF(ikm = K, salt/info stable)              (CryptoSeam, not topic-derived)
        │
        ▼
  GcsEvent oneof: member_list=7, invite_link=8  ──►  Dart SessionCubit._dispatchEvent
                                                          ├─ hasMemberList → MembersCubit (byScope)
                                                          └─ hasInviteLink → invite view / queue
```

File-to-implementation mapping lives in the Component Responsibilities table (below), not the diagram.

### Recommended Project Structure

```
src/lib/
├── gcs_membership.hpp/.cpp   # NEW — Membership component (mint/redeem/role/remove/leave/delete/approve)
├── gcs_entity_store.{hpp,cpp}# MODIFIED — SpaceRecord/RoomRecord gain creator; CreateSpace/… stamp creator
├── gcs_crypto.{hpp,cpp}      # MODIFIED — per-room key store (mutex-guarded) + WrapRoomKey/UnwrapRoomKey
├── gcs_messaging.{hpp,cpp}   # MODIFIED — decrypt path resolves key from store, not DeriveRoomKey(topic)
├── gcs_core.hpp              # UNCHANGED — session pass-throughs already expose QueryKeyValues/Put/Get
src/proto/gcs_chat.proto      # MODIFIED — append-only: Role enum, MemberRecord, MemberList, InviteLink, InviteRecord, command+event arms, creator/members_can_invite
src/ffi/gcs_core_ffi.cpp      # MODIFIED — new command dispatch arms + BuildMemberListEvent + members heal callback
src/app/lib/
├── cubits/members_cubit.dart # NEW — Map<String, List<MemberInfo>> byScope (mirrors RailCubit.setTree)
├── cubits/session_cubit.dart # MODIFIED — _dispatchEvent gains hasMemberList/hasInviteLink
├── shell/members_dialog.dart # NEW — member list + invite + role-change + confirm modes
├── shell/join_dialog.dart    # NEW — paste-invite redeem flow
└── shell/room_rail.dart      # MODIFIED — "Join"/"Members"/"Delete" affordances
test/
├── test_gcs_membership.cpp   # NEW — member record CRUD, guards, approval, key-wrap round-trip
├── test_gcs_crypto.cpp       # MODIFIED — key-store + wrap/unwrap tests
└── test_gcs_ffi.cpp          # MODIFIED — invite/redeem/role FFI arms
```

### Component Responsibilities (planner anchor)

| Component | Responsibility | File:line anchor |
|-----------|----------------|------------------|
| `EntityStore` | record + tombstone-skip + manifest pattern; full-state rewrite | `gcs_entity_store.cpp:188-294`, `:265-294` (UpdateSpace) |
| `Messaging` | CryptoSeam injection, `QueryKeyValues` scan, skip-and-log decrypt | `gcs_messaging.hpp:72-79`, `gcs_messaging.cpp:309-384` |
| `gcs::crypto` | HKDF + AES-GCM + RAND_bytes; stateless | `gcs_crypto.cpp:28-177` |
| `GcsGlobalDb` | Put/Get/Put(topics)/PutLocal/QueryKeyValues/RegisterNewElementCallback | `gcs_global_db.hpp:209-286` |
| FFI | command dispatch arms + INVALID_ARGUMENT guards + event builders | `gcs_core_ffi.cpp:995-1245` (switch), `:521-554` (BuildRoomList/SpaceTree) |
| `SessionCubit` | `_dispatchEvent` arms; `publishCommand` | `session_cubit.dart:426-467` |
| `RailCubit` | `setTree` full-replacement | `rail_cubit.dart:148-183` |

### Pattern 1: Per-member LWW record (D-01) — mirror the entity-store record

**What:** One serialized `MemberRecord` proto per member per scope, written full-state (never a patch), read back via prefix scan; tombstone = `deleted` flag, never key removal (mirror Phase 2 D-03).

**When to use:** Every membership mutation — mint, redeem, role change, explicit leave, removal, approve.

**Example** (member enumeration, mirrors `Messaging::QueryHistory` at `gcs_messaging.cpp:309-312`):
```cpp
// Source: pattern from src/lib/gcs_messaging.cpp:309-384 (QueryHistory scan)
outcome::result<chat::MemberList> Membership::MembersFor(const std::string &scopeId) {
  const std::string prefix = std::string(kMembersKeyPrefix) + scopeId + "/";
  auto scanResult = m_session.QueryKeyValues(prefix);   // raw keys, values carry the record
  if (!scanResult.has_value()) return scanResult.error();
  chat::MemberList list;
  list.set_scope_id(scopeId);
  list.set_self_address(m_senderAddress);               // D-04 pushed local wallet → Dart "You" marker
  for (const auto &entry : scanResult.value()) {
    chat::MemberRecord record;
    if (!record.ParseFromString(entry.second)) { spdlog::warn("...skip..."); continue; }
    if (record.deleted()) continue;                     // tombstone-skip
    *list.add_member() = record;
  }
  return list;
}
```

### Pattern 2: Write-time permission guard (D-08) — mirror the FFI validation arms

**What:** A C++ guard reads the caller's role in the scope before mutating. The FFI already stamps `GeniusSDKGetAddress().address` as the local identity (`gcs_core_ffi.cpp:885`); the guard looks that address up in the member roster and checks the role matrix.

**When to use:** Every new command arm AND retroactively the existing `create_room`/`update_space`/`send_text` arms (see Pitfall P5).

**Example** (shape; concrete matrix in §Security V4):
```cpp
// Source: shape of the INVALID_ARGUMENT validation arms, src/ffi/gcs_core_ffi.cpp:1139-1165
case gcs::chat::GcsCommand::kChangeRole: {
  const auto &cmd = command.change_role();
  if (cmd.scope_id().empty() || cmd.target().empty()) {
    PostErrorNotice("change_role rejected: scope_id and target must be non-empty");
    return GCS_ERROR_INVALID_ARGUMENT;
  }
  // D-03: reject demotion targeting the creator (write guard, not CRDT semantics)
  if (!g_membership->CanChangeRole(cmd.scope_id(), cmd.target(), cmd.role())) {
    PostErrorNotice("change_role rejected: permission denied");
    return GCS_ERROR_INVALID_ARGUMENT;
  }
  // ... write full-state rewrite of the target MemberRecord ...
}
```

### Pattern 3: Pushed flat event (D-04) — mirror `BuildSpaceTreeEvent`

**What:** `MemberList` is a flat, full-replacement roster pushed on the `GcsEvent` oneof, built under `g_mutex` and posted via `PostToDart`.

**Example** (mirrors `gcs_core_ffi.cpp:541-554`):
```cpp
// Source: shape of BuildSpaceTreeEvent, src/ffi/gcs_core_ffi.cpp:541-554
gcs::chat::GcsEvent BuildMemberListEvent(const std::string &scopeId) {
  gcs::chat::GcsEvent event;
  gcs::chat::MemberList *list = event.mutable_member_list();
  auto members = g_membership->MembersFor(scopeId);   // prefix scan
  if (members.has_value()) *list = std::move(members.value());
  return event;
}
```

### Pattern 4: Mutex-guarded key store (D-06) — the documented statelessness exception

**What:** `gcs_crypto` keeps a `std::mutex` + `std::unordered_map<std::string /*roomTopic*/, std::vector<unsigned char> /*32-byte key*/>`. Only the *bytes* are shared — never EVP contexts (the per-call EVP context rule in `gcs_crypto.hpp:8-14` stays intact). The store is consulted by `Messaging`'s decrypt path via a new `RoomKeyFor(roomTopic)` accessor that the injected CryptoSeam resolves through.

**When to use:** Room-key mint at room creation; room-key recovery at redeem; decrypt at send/receive/history.

### Anti-Patterns to Avoid

- **Op-log or cross-key join for membership/approval:** per-key LWW silently drops concurrent ops on a shared log key and lets a two-key (pending + flip) approval diverge. D-01/D-07 rejected these for exactly this reason; do not reintroduce.
- **Per-room membership records:** event storm; D-02 keeps room access derived from the space-scope record + `explicit_leave`.
- **Client-side enforcement:** rendering role-gated affordances from the pushed role is presentation; the C++ write guard is the source of truth. A hostile client can call any FFI arm directly.
- **Hand-rolled crypto or key derivation:** use the existing OpenSSL EVP surface only; never add a second crypto implementation.
- **`sleep_for` in tests:** use `gcs::test::WaitForCondition` (mandated).
- **New FFI ABI functions:** the four-function ABI (`gcs_init`/`gcs_publish`/`gcs_subscribe`/`gcs_shutdown`) is frozen; all new commands flow through the `GcsCommand` oneof.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Symmetric encryption / key wrap | custom AES | `EVP_aes_256_gcm()` (vendored OpenSSL 3.3.3) | authenticated encryption, nonce/tag handling, constant-time; already in `gcs_crypto.cpp` |
| Key derivation | custom KDF | `EVP_PKEY_HKDF` + `EVP_sha256()` | HKDF-SHA256 already wired (`gcs_crypto.cpp:28-66`) |
| Random keys/nonces | `rand()`/PRNG | `RAND_bytes` with checked return | crypto-grade randomness; D-05/D-06 key and nonce minting |
| Id minting | naive counter | `NextEntityId`/`NextMessageId` CR-01-safe idiom (seed + counter) | a bare counter restarts at 0 across launches and silently overwrites under LWW |
| CRDT enumeration | home-grown scan | `QueryKeyValues` prefix scan | only enumeration primitive GlobalDB offers; used by `QueryHistory` |
| URL key encoding | ad-hoc | hex or base64url (no padding) + `Uri.parse` on Dart | binary invite key must be URL-copy-safe; re-validated C++-side |
| Clipboard copy | custom | Flutter `Clipboard.setData` | standard, testable |

**Key insight:** The entire phase composes from primitives already verified in Phases 1–3. The only *new* code is (a) a membership component that layers write guards over the existing `Put`/`Get`/`QueryKeyValues` surface, and (b) two thin crypto helpers (`WrapRoomKey`/`UnwrapRoomKey`) that reuse the existing GCM envelope.

## Common Pitfalls

### Pitfall P1 (CRITICAL): `SpaceRecord` has no `creator` field today

**What goes wrong:** D-03 requires the Super Admin guard and the D-07 approval check to compare the target against the creator, but `SpaceRecord` (fields 1–8: `id, name, is_public, auto_join_rooms, created_at_ms, updated_at_ms, deleted, deleted_at_ms`) has no creator. [VERIFIED: src/proto/gcs_chat.proto:112-121]

**Why it happens:** Phase 2 shipped spaces without any membership concept; creator was never needed.

**How to avoid:** Add `string creator = 9;` to `SpaceRecord` (append-only) and stamp it in `EntityStore::CreateSpace` from the injected local wallet address. Note the EntityStore constructor currently receives only `CoreSession&` — it will need the local wallet address (or the membership component will need to own creator stamping).

**Warning signs:** A compile-time error referencing `creator()` on `SpaceRecord`; a guard that always rejects because `creator` is empty.

### Pitfall P2 (CRITICAL): Standalone rooms are member scopes, but D-03 only names `SpaceRecord`

**What goes wrong:** D-01 defines scope as "a space or a standalone room"; the UI-SPEC opens the member-list dialog for both. A standalone room's Super Admin (its creator) must be known, but `RoomRecord` (fields 1–7) has no creator and D-03 only mandates `SpaceRecord.creator`. [VERIFIED: src/proto/gcs_chat.proto:126-134; 04-UI-SPEC.md "member list per scope"]

**Why it happens:** D-03's gap analysis focused on spaces; standalone rooms inherit the same need.

**How to avoid:** Add `string creator = 8;` to `RoomRecord` and stamp it in `CreateRoom` when `parent_space_id` is empty (standalone). A nested room's Super Admin is its parent space's creator — no separate field needed for nested rooms, but the scope lookup must resolve "space scope" vs "standalone-room scope" by whether `parent_space_id` is empty.

**Warning signs:** Member-list dialog for a standalone room renders a Super Admin badge for an empty address; demotion guard is a no-op.

### Pitfall P3 (CRITICAL): `members_can_invite` (MEMB-05) has no storage field or config command

**What goes wrong:** MEMB-05 gates Member-initiated invites on `membersCanInvite`, but neither `SpaceRecord` nor `RoomRecord` carries it, and there is no update command for rooms. [VERIFIED: src/proto/gcs_chat.proto — no such field; only `UpdateSpaceCommand` exists, no `UpdateRoomCommand`]

**How to avoid:** Add `bool members_can_invite` to `SpaceRecord` (field 10) and `RoomRecord` (field 9); default `true` or `false` per product intent (recommend `false` to match "Members can invite *if enabled*"). Extend `CreateSpaceCommand`/`CreateRoomCommand`/`UpdateSpaceCommand`. For room-level toggling, either add an `UpdateRoomCommand` (append-only) or scope room-level invite config to creation time and let spaces own the toggle. This is a planner decision — flag it.

**Warning signs:** MEMB-05 is untestable because the config value has nowhere to live.

### Pitfall P4 (HIGH): The redeemer cannot recover the room key unless the inviter persists it at mint time

**What goes wrong:** The room key K is minted at room creation and held by existing members. At redeem, the recipient only holds the invite key IK from the URL — they do NOT know K. If the member record is first written at redeem (by the redeemer), nobody has wrapped K under IK, and the redeemer can never decrypt. [RESOLVED — see Open Questions Q1]

**How to avoid (RESOLVED — see Open Questions Q1):** Do NOT write a member record at mint (the recipient wallet is unknown). At mint, write ONE invite record under `gcs/invites/<scopeId>/<inviteId>` carrying the role grant (Guest default per D-05), the scope reference, and the room key(s) AES-256-GCM-wrapped under IK. Redeem = the recipient scans for the invite record by inviteId, unwraps K under IK, stores it in their per-room key store, and writes their OWN wallet-keyed member record (`gcs/members/<scopeId>/<redeemerWallet>`) carrying the wrapped key. Revocation stays "tombstone the member record" (D-05); multi-use/no-expiry is preserved because the invite record is never consumed.

**Warning signs:** Redeem succeeds but subsequent decrypts fail (wrong key); the recipient sees themselves in the member list before they've joined (resolved: no member record exists before redeem).

### Pitfall P5 (HIGH): The permission matrix retroactively gates existing Phase 2/3 commands

**What goes wrong:** `create_room`, `update_space`, and `send_text` currently run with no permission check (any local node can call them). D-08 gates config to Super Admin/Admin and write to Member+, but the existing arms are unguarded. [VERIFIED: src/ffi/gcs_core_ffi.cpp:1137-1245 — no role check in any arm]

**How to avoid:** Add write-time role checks to the existing arms: `create_space` = anyone (becomes Super Admin); `create_room` = standalone→anyone, nested→Admin+ in the parent space; `update_space` = Super Admin/Admin; `send_text` = Member+ (Guest read-only). `create_space` additionally stamps `creator`.

**Warning signs:** A Guest can send messages or toggle space config in a two-node test.

### Pitfall P6 (HIGH): Remote membership changes do not converge into a pushed `MemberList` today

**What goes wrong:** Entity records (`SpaceTree`) are pushed only on *local* mutations + session-init replay — there is no `RegisterNewElementCallback` for entities, so a remote node's `create_space` never live-updates another client's rail. If members follow the same model, a remote redeem/role-change would not update other members' lists until re-init. [VERIFIED: src/ffi/gcs_core_ffi.cpp:902-933 — the only heal callback registered is `/gcs/messages/.*`]

**How to avoid:** Register a `RegisterNewElementCallback` with pattern `/gcs/members/.*` (mirroring the messages heal callback), funnelling converged member records into a push of the affected scope's `MemberList`. The callback must follow the same teardown-intake-gate + copy-pointer-outside-lock discipline (`gcs_core_ffi.cpp:904-929`) to avoid the 03-06 ABBA deadlock.

**Warning signs:** Two-node test: node A redeems, node B's member list never updates without a restart.

### Pitfall P7 (LOW/MEDIUM): `gcs_crypto` statelessness exception must not leak EVP state across threads

**What goes wrong:** `gcs_crypto` is documented stateless — "OpenSSL context objects are mutable state machines and must never be shared" — and is called from three thread types (GossipSub strand, CRDT DagWorker, FFI command). The per-room key store adds shared mutable state. [VERIFIED: src/lib/gcs_crypto.hpp:8-14]

**How to avoid:** Store only the 32-byte key *bytes* in a `std::mutex`-guarded map; never cache EVP contexts. All derive/encrypt/decrypt calls still construct fresh contexts per call (unchanged). The key store is byte-level shared state only.

**Warning signs:** A data race or corrupted key when two threads mint/recover keys for the same room concurrently.

### Pitfall P8 (LOW): Invite-key encoding and `gcs://` URL parsing

**What goes wrong:** The invite key is 32 random bytes; embedding raw bytes in a URL breaks copy/paste. Dart's `Uri.parse` treats `gcs://invite/space/<id>?key=<key>` with authority `invite`, path `/space/<id>` — so segment extraction must be explicit, and C++ must re-validate the exact shape before acting.

**How to avoid:** Encode the key as hex or base64url (no padding) in the URL. Parse on Dart for inline validation ("Enter a valid invite link."), re-validate shape + length in the C++ redeem arm. The scope segment is either `space` or `room`; the id is the opaque invite id (`<inviteId>`).

## Code Examples

Verified patterns from official/authoritative in-repo sources:

### Key wrap / unwrap (D-06) — reuse the GCM envelope

```cpp
// Source: pattern from src/lib/gcs_crypto.cpp:68-177 (EncryptPayload/DecryptPayload).
// Difference: the key is the caller-supplied 32-byte invite key (NOT HKDF(roomTopic)),
// and the plaintext is the 32-byte room key. Envelope stays nonce(12)||ct||tag(16).
outcome::result<std::string> WrapRoomKey(const std::vector<unsigned char> &memberKey,
                                         const std::vector<unsigned char> &roomKey);
outcome::result<std::vector<unsigned char>> UnwrapRoomKey(
    const std::vector<unsigned char> &memberKey, const std::string &envelope);
// Implementation composes EVP_aes_256_gcm() exactly as EncryptPayload/DecryptPayload
// do, minus the DeriveRoomKey step; the member key is passed to EVP_EncryptInit_ex
// directly. Nonce is 12 fresh RAND_bytes per wrap (same as the D-08 envelope).
```

### Room-key derivation swap (WR-02) — ikm changes only

```cpp
// Source: src/lib/gcs_crypto.cpp:46-49 (the WR-02 defect site).
// CHANGE: EVP_PKEY_CTX_set1_hkdf_key(pctx, /*roomTopic→*/ roomKey.data(), roomKey.size())
// kMessagesHkdfSalt / kMessagesHkdfInfo STAY stable; envelope format unchanged.
```

### Member-list push arm (D-04) — Dart dispatch

```dart
// Source: pattern from src/app/lib/cubits/session_cubit.dart:426-467 (_dispatchEvent).
if (event.hasMemberList()) {
  _membersCubit?.setMembers(event.memberList.scopeId, event.memberList.member, event.memberList.selfAddress);
  return;
}
if (event.hasInviteLink()) {
  _pendingInviteLink = event.inviteLink.url; // route to open invite view or queue
  return;
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Room key = `HKDF(ikm = roomTopic)` | Room key = 32-byte RAND_bytes, `HKDF(ikm = key)`, distributed AES-GCM-wrapped under per-member invite key | Phase 4 (WR-02 closure) | Room topic is public; topic-derived keys gave obfuscation only. Member-distributed keys give actual membership confidentiality |
| Op-based membership log (§Membership CRDT Op) | Per-member LWW `MemberRecord` records | Phase 4 (D-01) | op-log loses concurrent ops under per-key LWW; per-member records are the LWW-safe shape |
| Cross-key pending-approval record + flip command | Embedded `approved_by` on the target record (dormant-until-approved) | Phase 4 (D-07) | cross-key join diverges under LWW; single-signer overwrite never drops a concurrent approval |
| Unguarded Phase 2/3 commands | Write-time permission guards (D-08) on all mutating arms | Phase 4 | Guests read-only; config gated to Super Admin/Admin |

**Deprecated/outdated:**
- `DeriveRoomKey(roomTopic)` as the live key source: kept only as the pre-membership fallback; replaced by the per-room key store lookup for any room minted after Phase 4.
- Architecture note §Membership CRDT Op and §Invite Model HMAC-claim shape: superseded by D-01/D-05 (read to know what was rejected and why).

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | The invite key is hex- or base64url-encoded in the URL (raw binary would break copy/paste); C++ re-validates | Standard Stack / P8 | Cosmetic — any reversible encoding works; planner just needs to pick one and keep C++/Dart in sync |
| A2 | RESOLVED (Open Q1): an `InviteRecord` is written at **mint** under `gcs/invites/<scopeId>/<inviteId>` (Guest grant + scope + room key wrapped under IK); the member record is written at **redeem** under `gcs/members/<scopeId>/<redeemerWallet>` | P4 / Open Q1 | Resolved by the invite-key-addressed invite record: no member record exists before redeem, so the recipient never appears in the member list before joining (roadmap success criterion 2 holds at redeem) |
| A3 | `members_can_invite` defaults to `false` and lives on SpaceRecord + RoomRecord (recommendation) | P3 | If default is `true`, MEMB-05's "if enabled" wording is inverted; if room-level toggling is required post-creation, an `UpdateRoomCommand` must be added |
| A4 | `GeniusSDKGetAddress().address` (used at `gcs_core_ffi.cpp:885`) is the same wallet identity as `GeniusNode::GetAddress()` named in D-03 | Architectural Responsibility Map | The FFI already stamps this as the sender; if the membership identity must differ, the stamp site changes |

**If this table is empty:** not applicable — see A1–A4 above.

## Open Questions (RESOLVED)

1. **Mint vs redeem timing (chicken-and-egg on the room key)** — RESOLVED
   - What we know: D-05/D-06 make the invite key the member's key and wrap the room key under it; the redeemer does not know the room key.
   - Resolution (user-confirmed via orchestrator, recorded in the revised plans 04-01/04-03/04-05): invite-key-addressed invite record + write-at-redeem member record. At mint, C++ mints an opaque invite id + a random bearer key IK and persists ONE `InviteRecord` under `gcs/invites/<scopeId>/<inviteId>` (role grant = Guest default per D-05, scope reference, room key(s) AES-256-GCM-wrapped under IK). NO member record is written at mint — the recipient wallet is unknown. The URL keeps D-05's literal shape `gcs://invite/{space|room}/<inviteId>?key=<IK>` (`<id>` = the invite id). At redeem, C++ looks up the invite record by inviteId, verifies/decrypts with IK (wrong-key = skip-and-log), writes the member record under `gcs/members/<scopeId>/<redeemerWallet>` (role C++-stamped from the invite record), and pushes `MemberList` (D-04). Multi-use/no-expiry preserved: the invite record is never consumed — N redeemers each write their own wallet-keyed member record. This lives under CONTEXT.md's Claude's-Discretion items (redeem-command shape, member-key prefix layout); documented in the revised plans, not edited into CONTEXT.md.

2. **Standalone-room creator field** — RESOLVED
   - What we know: D-03 only names `SpaceRecord.creator`, but D-01 makes standalone rooms a scope.
   - Resolution: add `RoomRecord.creator` (field 8) and stamp it for standalone rooms in `CreateRoom` (nested rooms leave it empty — their Super Admin is the parent space's creator). Plan 04-04.

3. **`members_can_invite` scope + update path** — RESOLVED
   - What we know: MEMB-05 requires it; no field or command exists today.
   - Resolution: field on both records, default `false`, extend `CreateSpaceCommand`/`CreateRoomCommand`/`UpdateSpaceCommand`; NO `UpdateRoomCommand` — room-level invite config is creation-time only (spaces own the post-creation toggle). Plans 04-01/04-04.

4. **Remote membership convergence** — RESOLVED
   - What we know: entities have no heal callback; messages do.
   - Resolution: register the `/gcs/members/.*` heal callback (P6) so remote joins converge into a pushed `MemberList` (plan 04-05).

5. **Invite-key encoding choice** — RESOLVED
   - What we know: 32 raw bytes must be URL-safe.
   - Resolution: base64url without padding (shorter than hex), documented once; C++ and Dart share the same codec (plan 04-03).

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| CMake | C++ build | ✓ | 3.29.2 | — |
| Dart SDK | Dart codegen + tests | ✓ | 3.11.5 | — |
| Flutter SDK | app build/test | ✓ | 3.41.9 | — |
| OpenSSL (vendored `OpenSSL::Crypto`) | key wrap/HKDF | ✓ | 3.3.3 (vendored) | — (never homebrew 3.6.3; STATE.md forbids) |
| protobuf / `add_proto_library` | proto regen | ✓ | vendored | — |
| GTest | unit tests | ✓ | vendored | — |
| `protoc` (standalone) | — | ✗ (not on PATH) | — | not needed — proto is compiled via the in-tree `add_proto_library` CMake function, not a standalone protoc |

**Missing dependencies with no fallback:** none.
**Missing dependencies with fallback:** `protoc` is absent from PATH but is not required (in-tree `add_proto_library`). Note: Dart-side proto regeneration (`src/app/lib/generated/proto/gcs_chat.pb.dart`) is a committed codegen output that must be regenerated via the app's codegen step after the `.proto` changes — confirm the exact codegen command from `src/app/CMakeLists.txt` (GENERATED_DIR) before the proto plan.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | GoogleTest (GTest), discovered via `gcs_test()` macro in `test/CMakeLists.txt` |
| Config file | `test/CMakeLists.txt` (`gcs_test(<name> <sources> "<libs>")`) |
| Quick run command | `ctest -R test_gcs_membership` (or the executable directly) |
| Full suite command | `ctest` (build tree) |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| MEMB-01 | Mint invite returns `gcs://invite/...` URL; redeem joins + unwraps key | unit | `test_gcs_membership` | ❌ Wave 0 |
| MEMB-02 | Five-tier role enum + member record CRUD + role assignment | unit | `test_gcs_membership` | ❌ Wave 0 |
| MEMB-03 | Demotion targeting creator rejected by guard | unit | `test_gcs_membership` | ❌ Wave 0 |
| MEMB-04 | Destructive op dormant when 2+ admins; Super Admin approve activates | unit | `test_gcs_membership` | ❌ Wave 0 |
| MEMB-05 | Member invite gated by `members_can_invite` | unit | `test_gcs_membership` | ❌ Wave 0 |
| (crypto) | Wrap/unwrap round-trip; wrong key fails; key store thread-safety | unit | `test_gcs_crypto` | ✅ (extend) |
| (FFI) | invite/redeem/role arms validate + push `MemberList`/`InviteLink` | unit | `test_gcs_ffi` | ✅ (extend) |
| (multinode) | Remote redeem converges member list on peer (P6) | integration | `test_gcs_messaging_multinode` (or new `test_gcs_membership_multinode`) | ❌ Wave 0 |

### Sampling Rate
- **Per task commit:** `ctest -R test_gcs_membership -R test_gcs_crypto` (fast, deterministic)
- **Per wave merge:** `ctest` full suite (C++); `flutter test` for cubits/widgets
- **Phase gate:** full C++ + Flutter suites green before `/gsd:verify-work`

### Wave 0 Gaps
- [ ] `test/test_gcs_membership.cpp` — covers MEMB-01..05 (member CRUD, guards, approval, key-wrap round-trip)
- [ ] `test/test_gcs_membership_multinode.cpp` (or extend existing multinode fixture) — remote convergence (P6)
- [ ] `test_gcs_crypto.cpp` extension — `WrapRoomKey`/`UnwrapRoomKey` + key store
- [ ] `test_gcs_ffi.cpp` extension — invite/redeem/role/leave/delete/approve arms
- [ ] Dart cubit test — `MembersCubit.setMembers` full-replacement + `_dispatchEvent` arms

## Security Domain

### Applicable ASVS Categories (Level 1)

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | No credential auth — identity is the wallet address (`GeniusSDKGetAddress()`); no account/password surface |
| V3 Session Management | no | No server session; CRDT state sync, no session tokens |
| V4 Access Control | yes | Role hierarchy enforced as C++ write-time guards (D-08). ASVS 4.1.1: enforce access control at a trusted layer — never client-side. Super Admin immutability (D-03), destructive-approval (D-07), Guest read-only |
| V5 Input Validation | yes | Invite URL shape + length re-validated C++-side; command field bounds mirror the existing INVALID_ARGUMENT arms (`kMaxTopicLength`, `kMaxEntityNameLength`). Protobuf parses structure; FFI adds semantic guards (ASVS 5.1.x) |
| V6 Cryptography | yes | AES-256-GCM (authenticated) + HKDF-SHA256 + `RAND_bytes` with checked returns; never hand-rolled; wrong-key skip-and-log. ASVS 6.2.x (keys), 6.3.x (key management — per-room keys, key store) |

### Known Threat Patterns for {CRDT membership / invite tokens}

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Forged role in a claim/op (any member holds the room key on a serverless network) | Elevation of Privilege | Role is C++-stamped, never read from a client-supplied field (D-05 rejected HMAC claims); role changes guarded by the author's own role |
| Super Admin demotion | Elevation of Privilege / Tampering | Write guard rejects any demotion targeting `creator` (D-03) |
| Unauthorized destructive op (remove/delete) | Tampering | Dormant-until-approved: `approved_by` empty → read-gated (D-07) |
| Invite link leakage → unauthorized join | Information Disclosure / Spoofing | Bearer key is the capability; revocation = tombstone the member record (D-05); no expiry is an accepted MVP risk (documented) |
| Guest sending messages | Elevation of Privilege | Send-path write guard rejects Guest (D-08) |
| Replay / tampering of a wrapped room key | Tampering | AES-GCM tag verification; wrong-key records skip-and-log (unchanged D-08 posture) |
| Undecryptable/garbage member record (hostile bytes) | DoS | Tombstone-skip + skip-and-log on parse failure, never abort |

## Sources

### Primary (HIGH confidence)
- `src/proto/gcs_chat.proto` — full append-only contract; `SpaceRecord` (112-121, no creator), `RoomRecord` (126-134, no creator), `GcsCommand` oneof (55-63, next free 6), `GcsEvent` oneof (95-104, next free 7)
- `src/lib/gcs_entity_store.{hpp,cpp}` — record/tombstone/manifest pattern; `UpdateSpace` full-state rewrite (cpp:265-294); `CreateSpace`/`CreateRoom` authority stamping (cpp:188-263); id-mint idiom (cpp:110-119)
- `src/lib/gcs_crypto.{hpp,cpp}` — HKDF/AES-GCM/RAND_bytes (cpp:28-177); WR-02 defect site `ikm = roomTopic` (cpp:46-49); statelessness contract (hpp:8-14); stable salt/info (hpp:50-52)
- `src/lib/gcs_messaging.{hpp,cpp}` — CryptoSeam (hpp:72-79); `QueryKeyValues` scan (cpp:309-384); skip-and-log decrypt; archive worker
- `src/ffi/gcs_core_ffi.cpp` — wallet-address stamp (885); command dispatch switch + INVALID_ARGUMENT arms (995-1245); `BuildRoomListEvent`/`BuildSpaceTreeEvent` (521-554); messages heal callback (902-933); g_mutex/ABBA discipline
- `src/lib/gcs_core.hpp` / `src/lib/gcs_storage/gcs_global_db.hpp` — Put/Get/Put(topics)/PutLocal/QueryKeyValues/RegisterNewElementCallback/Publish/Subscribe surface
- `src/app/lib/cubits/session_cubit.dart` — `_dispatchEvent` (426-467), `publishCommand`, `GcsCommandTransport`
- `src/app/lib/cubits/rail_cubit.dart` — `setTree` full-replacement (148-183)
- `src/app/lib/generated/proto/gcs_chat.pb.dart` + `src/app/CMakeLists.txt` — Dart codegen output + GENERATED_DIR
- `test/CMakeLists.txt` + `test/test_gcs_entities.cpp:186` — GTest `gcs_test()` macro + `gcs::test::WaitForCondition` template
- `src/app/pubspec.yaml` — `ffi ^2.2.0`, `flutter_bloc ^9.0.0`, `protobuf ^4.0.0`, `protoc_plugin ^22.5.0`

### Secondary (MEDIUM confidence)
- `.planning/workstreams/app/phases/04-membership-invites/04-CONTEXT.md` — locked decisions D-01..D-08 + Claude's Discretion + Deferred
- `.planning/workstreams/app/phases/04-membership-invites/04-UI-SPEC.md` — UI design contract (member list, invite view, join dialog, role badges, approval surfacing)
- `.planning/notes/gcs-chat-architecture.md` — §Invite Model, §Membership Types, §Destructive Action Rules, §Membership CRDT Op (superseded), §Privacy
- `.planning/workstreams/app/STATE.md` — vendored OpenSSL 3.3.3 (Phase 03 note); Phase 03 deadlock/ABBA history
- `.planning/workstreams/app/phases/03-messaging/deferred-items.md` — WR-02 scope ruling (swap HKDF input, keep salt/info stable)

### Tertiary (LOW confidence)
- Per-key LWW verification at `../SuperGenius/src/crdt/impl/crdt_datastore.cpp` (~866, ~1375, ~1546-1556) — cited from prior-phase verification in 04-CONTEXT.md; the SuperGenius submodule is not checked out in this working tree, so these line anchors were NOT re-opened this session. Treat as verified-by-prior-phase, not verified-this-session.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — zero new packages; every dependency read/confirmed in `pubspec.yaml`, CMake, and source this session.
- Architecture: HIGH — patterns directly mirror `gcs_entity_store`/`gcs_messaging`/`gcs_crypto`/FFI push builders read this session.
- Pitfalls: HIGH — P1/P2/P3/P5/P6 are direct code evidence (field absence + absent heal callback + unguarded arms); P4/P7/P8 are reasoning + OpenSSL/URL semantics, flagged accordingly.

**Research date:** 2026-09-27
**Valid until:** 2026-10-27 (stable — no external fast-moving dependencies; OpenSSL/Flutter versions are pinned/vendored)
