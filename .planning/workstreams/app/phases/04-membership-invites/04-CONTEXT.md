# Phase 4: Membership & Invites - Context

**Gathered:** 2026-09-27
**Status:** Ready for planning

<domain>
## Phase Boundary

Users can invite others via capability-token links (`gcs://invite/...`); recipients redeem the invite, join, and appear in the member list; every participant has a visible five-tier role (Super Admin, Admin, Moderator, Member, Guest) that gates actions; the Super Admin (creator) cannot be demoted; destructive actions require Super Admin approval when 2+ admins exist (MEMB-01…MEMB-05). Riding along, because the member roster exists for the first time: the WR-02 room-key swap (Phase 3 deferred item — room keys stop being derivable from the public topic and become member-distributed) and the Phase 2 D-04 deferral (explicit-leave precedence for derived joins). No kick/ban/message-deletion flows (Phase 5), no GCS bot (Phase 6), no lobby/discovery (Phase 7), no key rotation on member leave (ENCR-02, v2), no sender signing (v1.1), no DM auto-invites.

</domain>

<decisions>
## Implementation Decisions

### Member Records & Role State
- **D-01:** **Per-member LWW record per scope.** One serialized `MemberRecord` per member per scope under `gcs/members/<scopeId>/<wallet>` (scope = a space or a standalone room), enumerated by the Phase 3 `QueryKeyValues` prefix scan — no manifest (mirrors `gcs/messages/` at `gcs_messaging.hpp:82`). Rejected: the architecture note's op-based membership log (§Membership CRDT Op) — GlobalDB is per-key LWW with no union semantics, so a shared op-log key silently drops concurrent ops, and the merge reducer has no in-repo precedent.
- **D-02:** **Room access stays derived, never per-room recorded.** `joined(room) = parentSpace.autoJoinRooms && !member.explicitLeave` — the exact rule Phase 2 D-04 deferred here. `explicit_leave` is a bool on the space-scope member record; retoggle-resurrect (a leave resurrecting if config toggles false→true→false→true) is documented as intended. Rejected: explicit per-room membership records (event storm; the derived evaluator already exists).
- **D-03:** **Role changes are full-state LWW rewrites; Super Admin protection is a C++ write guard.** Promotion/demotion rewrites the member record (the `UpdateSpace` full-state pattern, `gcs_entity_store.cpp` ~265-294); GlobalDB keeps only state, so the "Super Admin cannot be demoted" invariant is enforced by a C++ write-time guard rejecting any demotion targeting the creator — never by CRDT semantics. `SpaceRecord` gains a `creator` field (load-bearing gap found by research: it does not exist today, and both this guard and the D-07 approval check verify against it). Member identity = wallet address (`GeniusNode::GetAddress()`, `gcs_core_ffi.cpp` ~885) — no new identity surface this phase.
- **D-04:** **Member list reaches Dart as one flat `MemberList` event** on the `GcsEvent` oneof (the `SpaceTree` push pattern): full replacement per scope, pushed on redeem, on any membership write, and at session init after CRDT replay.

### Invite Tokens
- **D-05:** **Bearer key in the URL — the architecture note's literal shape.** `gcs://invite/{space|room}/<id>?key=<key>` where the key is a random (RAND_bytes) per-member symmetric key. Multi-use, no expiry, no consume records for MVP; revocation = removing the member record (tombstone). Role grant (Guest default) and `membersCanInvite` enforcement live in the C++ mint/redeem path writing the CRDT membership record — never in the token. Rejected: HMAC claim tokens — on a serverless network every member holds the shared room key, so any member could forge any role; and the bearer key doubles as the D-06 room-key delivery vehicle, which claim tokens don't solve.

### Room-Key Distribution (WR-02)
- **D-06:** **Per-member AES-GCM key-wrap replaces the topic-derived key.** Each room gets a random 32-byte key minted at creation; that key becomes the HKDF input keying material instead of `roomTopic` (`gcs_crypto.cpp:46-49` — the WR-02 defect site). `kMessagesHkdfSalt`/`kMessagesHkdfInfo` stay STABLE and the D-08 envelope (nonce(12)‖ct‖tag on both live Publish and at-rest Put) is unchanged — only the ikm swaps. Each member record carries the room key AES-256-GCM-wrapped under that member's invite-borne key; decryption stays behind the injected CryptoSeam in the local Messaging layer; wrong-key records skip-and-log (unchanged). `gcs_crypto` gains a mutex-guarded per-room key store (a documented exception to its current statelessness). No identity keypairs this phase — Megolm-style session machinery only serves the deferred v1.1 signing / v2 rotation features.

### Approval Flow & Permission Matrix
- **D-07:** **Embedded `approved_by` on the target record; dormant-until-approved.** Phase 4 destructive scope = admin demotion, member removal, space/room deletion (kick/ban/message-delete are Phase 5). When 2+ admins exist, the author writes the op dormant (empty `approved_by`) and the Super Admin activates it by overwriting the same key with their id — single-signer approval means per-key LWW never drops a concurrent approval. Sole-admin case acts alone; no timeouts (locked rule table, architecture note §Destructive Action Rules). Enforcement is a write-time local check on every node plus tombstone-skip-style read gating for unapproved records — dormant, not reject-and-ignore, so the Super Admin always has something to approve. Rejected: a separate pending-approval record + flip command — its cross-key join can diverge under LWW and it only pays off with multi-approver quorum (not Phase 4).
- **D-08:** **Permission matrix per the architecture note, gated at C++ write time.** Super Admin (creator, singular): everything; Admin: config, membership, moderation-grant; Moderator: member management + message deletion (effect lands Phase 5); Member: read, write, invite when `membersCanInvite`; Guest: read-only — enforced as a send-path write guard. UI affordances render from the pushed role; they are presentation, not enforcement.

### Claude's Discretion
- Proto message/field naming and numbering within the append-only discipline (`MemberRecord`, `MemberList`, invite/redeem/membership commands, `SpaceRecord.creator`).
- Exact member-key prefix layout and wrapped-key encoding details.
- Whether a space invite wraps all room keys at redeem or lazily on first room join.
- Invite UI entry points and member-list surface (designed by the UI phase — `ui_phase` is enabled).
- How pending approvals surface in the pushed event plane (dedicated event arm vs error/toast reuse).
- Where the permission matrix lives in C++ (a table on the membership component vs per-call guards).
- Redeem-command shape and the invite-link clipboard/share mechanics on the Flutter side.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Architecture and requirements
- `.planning/notes/gcs-chat-architecture.md` — §Invite Model (URL shapes), §Membership Types (five-tier capability table), §Invite Permission (`membersCanInvite`), §Destructive Action Rules (approval table), §Membership CRDT Op (superseded by D-01/D-07 record model — read to know what was rejected and why), §Privacy (app-layer key model).
- `.planning/workstreams/app/REQUIREMENTS.md` — MEMB-01…MEMB-05 definitions (Phase 4 scope anchor).
- `.planning/workstreams/app/ROADMAP.md` — Phase 4 goal + success criteria 1-5.

### Locked prior-phase constraints this phase inherits
- `.planning/workstreams/app/phases/01-foundation/01-CONTEXT.md` — D-04 (C++ owns state; Dart thin subscriber), D-21 (rail shell), D-24/D-26 (append-only proto discipline), D-27/D-29 (topic pub/sub FFI plane: data-only Dart commands, protobuf envelopes, codec-tagged bytes, raw error strings).
- `.planning/workstreams/app/phases/02-spaces-rooms/02-CONTEXT.md` — D-01 (opaque C++-minted ids), D-02 (per-entity KV records; per-key LWW, no union), D-03 (tombstones, never key removal), D-04 (derived joins — D-02 here completes its deferred extension).
- `.planning/workstreams/app/phases/03-messaging/03-CONTEXT.md` — D-04 (wallet-address sender, unsigned until v1.1), D-08 (encrypted envelope contract the key swap must not break; CryptoSeam injection).
- `.planning/workstreams/app/phases/03-messaging/deferred-items.md` — WR-02 scope ruling: swap the HKDF input for membership-distributed key material; keep salt/info stable so this phase key-rotates without breaking archive framing.

### Verified code surface (load-bearing)
- `src/lib/gcs_entity_store.{hpp,cpp}` — the record pattern D-01/D-03 reuse (full-state LWW rewrite at ~265-294; tombstone-skip reads at ~301/314/336); header documents per-key LWW at ~108.
- `src/lib/gcs_crypto.{hpp,cpp}` — WR-02 fix site (`DeriveRoomKey` ikm = roomTopic, ~46-49); EVP AES-256-GCM + HKDF surface the key-wrap reuses; documented statelessness (~7-14) that D-06's key store excepts.
- `src/lib/gcs_messaging.{hpp,cpp}` — CryptoSeam (~72-79); `QueryKeyValues` prefix-scan usage (~309-312); wrong-key skip-and-log posture.
- `src/proto/gcs_chat.proto` — append-only contract being extended: `GcsCommand`/`GcsEvent` oneofs, `SpaceRecord` (creator-field gap, ~112-121), `SpaceTree` push precedent (~95-104).
- `src/ffi/gcs_core_ffi.cpp` — FFI dispatch arms this phase extends; wallet address stamp site (~885); write-guard placement precedent (existing INVALID_ARGUMENT validation arms).
- `src/lib/gcs_storage/gcs_global_db.hpp` — topics-aware Put + QueryKeyValues exposure from Phase 3.
- `../SuperGenius/src/crdt/impl/crdt_datastore.cpp` — per-key LWW verification (~866, ~1375, ~1546-1556) — why D-01/D-07 reject op-logs and cross-key joins.

### Flutter consumption surface
- `src/app/lib/cubits/session_cubit.dart` — `GcsCommandTransport.publishCommand` seam; event-dispatch table D-04's `MemberList` arm extends.
- `src/app/lib/cubits/rail_cubit.dart` — full-replacement rebuild pattern for the member list / role rendering.
- `src/app/scaffold/lib/components/` — `ResponsiveDrawer`, `TextEntryFieldWidget`, selection atoms, `showToast` — the invite/membership dialog toolkit (Phase 2 D-05 precedent).
- `src/app/scaffold/CLAUDE.md` — scaffold submodule contract (lib/ read-only, never edit generated families).

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `gcs_entity_store` record + tombstone-skip pattern — D-01/D-03/D-07 are its direct extensions.
- Phase 3 `QueryKeyValues` prefix scan (`gcs/messages/` precedent) — member enumeration needs no new storage API.
- `gcs_crypto` EVP AES-256-GCM + HKDF + RAND_bytes — the entire D-06 key-wrap composes from existing primitives; zero new algorithms.
- `GcsCommandTransport.publishCommand` + `ResponsiveDrawer` dialog atoms — invite/membership UI with no new transport or dialog work.
- `RailCubit` full-replacement rebuild + `SpaceTree` push — the `MemberList` event copies both shapes.

### Established Patterns
- Append-only proto evolution; C++ owns state / Dart thin subscriber; tombstones not removal; per-key LWW with no union (constrains every D here — the reason op-logs and cross-key approval joins are rejected).
- Write-time C++ validation guards (INVALID_ARGUMENT arms) — the Super Admin / Guest / approval guards follow.
- Tests use wait-condition templates; C++17 ceiling; no OS `#ifdef`; spdlog for diagnostics.

### Integration Points
- `GcsCommand` oneof gains invite/redeem + membership (role change, explicit leave, removal) commands; `GcsEvent` gains `MemberList`.
- `SpaceRecord` gains `creator`; member records materialize under `gcs/members/<scopeId>/`.
- Derived-join evaluator (Phase 2 D-04) gains the `!explicitLeave` term; `RoomList` refresh rides the same path.
- `Messaging`/CryptoSeam key resolution switches from `DeriveRoomKey(roomTopic)` to the per-room key store; invite redeem is the wrap/write path.
- FFI session init: after CRDT replay, push member lists with `SpaceTree`/`Readiness`.

</code_context>

<specifics>
## Specific Ideas

- User picked the recommended option on all four gray areas plus the invite-lifecycle follow-up (2026-09-26/27): per-member records; bearer key in URL; per-member key-wrap; embedded `approved_by`; multi-use/no-expiry invites.
- Advisor research (4 parallel gsd-advisor-researcher runs, 2026-09-26) verified in-repo: HMAC claim tokens are forge-any-role on a serverless network; the architecture note's op-log is incompatible with per-key LWW; `SpaceRecord` lacks a creator field (both the Super Admin guard and approval check need it); `gcs_crypto` is stateless-by-design today, so the per-room key store is a documented exception.

</specifics>

<deferred>
## Deferred Ideas

- Key rotation on member leave (ENCR-02) — v2; the D-06 record shape must not preclude it.
- Cryptographic sender signing + identity keypairs (Phase 3 D-04) — v1.1; revisit Megolm-style distribution then.
- Single-use / expiring invite tokens + nonce consume records — post-MVP if invite-leak abuse shows up.
- Kick/ban/message-deletion flows — Phase 5 Moderation (Moderator's deletion capability takes effect there).
- Member-list UI polish, invite share mechanics — UI phase design territory.
- Per-room join/leave audit history — only if a hard audit requirement lands.
- DM implicit invites — DMs don't exist yet.
- Lobby/discovery — Phase 7.

</deferred>

---

*Phase: 4-Membership & Invites*
*Context gathered: 2026-09-27*
