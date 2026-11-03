# Roadmap: Genius Cognitive System — App Workstream (v1.0 GCS Chat)

## Overview

Build a working multi-party chat application where users create spaces and rooms, invite participants via capability tokens, chat in real-time via CRDT sync, and interact with the GCS bot as an AI participant — all without central servers. The roadmap progresses from C++ core foundation through entity management (spaces/rooms), messaging, membership, moderation, bot integration, and finally public discovery.

## Phases

**Phase Numbering:**

- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

**Inserted milestone v1.0.1 (Coupling Remediation):** Phases 4.1–4.4 execute between Phase 4 and Phase 5. Phase 4 is complete; v1.0 Phases 5–7 remain planned and resume after v1.0.1.

- [x] **Phase 1: Foundation** - C++ core scaffolding, GlobalDB CRDT integration, FFI bridge to Flutter, and cross-platform CI/CD (macOS, Linux, Windows, iOS, Android)
- [x] **Phase 2: Spaces & Rooms** - Space and room creation with configurable inheritance (completed 2026-09-19)
- [x] **Phase 3: Messaging** - Real-time text messaging with CRDT sync across participants (completed 2026-09-24)
- [x] **Phase 4: Membership & Invites** - Roles, capability tokens, and permission model (completed 2026-10-21)
- [ ] **Phase 4.1: Room-Session Service (INSERTED)** - Joined-room state, the composed join predicate, the role gates, and topic grammar move from the FFI into a `src/lib` room-session service
- [ ] **Phase 4.2: Storage & Prefix Single-Homing (INSERTED)** - `Membership::Approve` reads through `EntityStore`; the `"gcs/chat/"` prefix has one C++ definition
- [ ] **Phase 4.3: Dart Predicate Single-Homing (INSERTED)** - Capability and dormancy predicates have one Dart home
- [ ] **Phase 4.4: Enforcement Gate (INSERTED)** - Canonical-payload tripwire plus a lib-only test target that drives the room lifecycle without `gcs_ffi`
- [ ] **Phase 5: Moderation** - Message tombstones, kick/ban, and admin approval flows
- [ ] **Phase 6: GCS Bot** - Bot participant with auto-answer policies and Expert Model routing
- [ ] **Phase 7: Discovery** - Public space lobby and private space isolation

## Phase Details

### Phase 1: Foundation

**Goal**: The C++ chat core compiles, links against GlobalDB, and exposes a working FFI surface that Flutter can call.
**Depends on**: Nothing (first phase)
**Requirements**: CORE-05
**Success Criteria** (what must be TRUE):

  1. C++ core library builds on macOS, Linux, Windows, iOS, and Android without errors
  2. GlobalDB CRDT instance can be initialized and a test op can be published/subscribed locally
  3. Flutter app can call into C++ via FFI and receive a response
  4. GossipSub topics can be created and joined from the C++ core
  5. CI/CD pipeline builds and tests on self-hosted runners for macOS, Linux, Windows, iOS, Android

**Plans:** 11/11 plans executed

Plans:
**Wave 1**

- [x] 01-01-PLAN.md — Wave 1 — Move GcsGlobalDb to root src/lib/gcs_storage/ + four pass-through accessors (D-25) + staged test

**Wave 2** *(blocked on Wave 1 completion)*

- [x] 01-02-PLAN.md — Wave 2 (depends: 01-01) — Wire gcs_core to gcs_storage; create gcs::CoreSession class
- [x] 01-07-PLAN.md — Wave 2 (depends: 01-01) — Flutter app build wiring (FRONTEND_BUILD_ENABLED gate) + pubspec (keep neoswarm_ffi, add flutter_bloc/ffigen/protobuf/protoc_plugin)

**Wave 3** *(blocked on Wave 2 completion)*

- [x] 01-03-PLAN.md — Wave 3 (depends: 01-01, 01-02, 01-07) — protobuf wire contract (gcs_chat.proto + gcs_proto) + gcs_ffi SHARED lib + C ABI (D-26 push-not-pull) + add_subdirectory(app) wiring
- [x] 01-08-PLAN.md — Wave 3 (depends: 01-07) — chat_message_bubble composite codegen (Dart triple; C++ half = proto)

**Wave 4** *(blocked on Wave 3 completion)*

- [x] 01-04-PLAN.md — Wave 4 (depends: 01-01, 01-02, 01-03) — Test scaffolding + CORE-05 smoke tests (lifecycle, CRDT round-trip, FFI option-C init)
- [x] 01-09-PLAN.md — Wave 4 (depends: 01-08) — code_block + media composite codegen (top-level flow items)

**Wave 5** *(blocked on Wave 4 completion)*

- [x] 01-05-PLAN.md — Wave 5 (depends: 01-03, 01-04, 01-07) — Dart_PostCObject protobuf-byte posting + ffigen/protobuf Dart bindings + Dart smoke test
- [x] 01-06-PLAN.md — Wave 5 (depends: 01-03, 01-04) — CI workflow (.github/workflows/cmake.yml, -DBUILD_TESTS=ON) for all 5 platforms
- [ ] 01-10-PLAN.md — Wave 5 (depends: 01-08, 01-09) — chat_message_flow envelope (sealed ChatFlowItem)

**Wave 6** *(blocked on Wave 5 completion)*

- [x] 01-11-PLAN.md — Wave 6 (depends: 01-05, 01-10) — GCSChat app shell (rail + flow + composer) + pushed-event cubits + theming

### Phase 2: Spaces & Rooms

**Goal**: Users can create spaces and rooms with configurable inheritance, and the entity hierarchy is persisted via CRDT.
**Depends on**: Phase 1
**Requirements**: CORE-01, CORE-02, CORE-03
**Success Criteria** (what must be TRUE):

  1. User can create a public or private space and see it in their local space list
  2. User can create a room within a space or as a standalone room
  3. User can toggle `autoJoinRooms` on a space and observe the join behavior change
  4. Space/room metadata survives app restart (CRDT persistence)

**Plans:** 5/5 plans complete

Plans:
**Wave 1**

- [x] 02-01-PLAN.md — Wave 1 — Extend gcs_chat.proto (entity records/commands/SpaceTree) + gcs::EntityStore + unit tests

**Wave 2** *(blocked on Wave 1 completion)*

- [x] 02-02-PLAN.md — Wave 2 (depends: 02-01) — FFI dispatch (create/update arms, SpaceTree push, derived joins) + two-session persistence test
- [x] 02-03-PLAN.md — Wave 2 (depends: 02-01) — Dart pb regen (pinned 22.5.0) + RailCubit tree + SpaceTree dispatch + cubit tests

**Wave 3** *(blocked on Wave 2 completion)*

- [x] 02-04-PLAN.md — Wave 3 (depends: 02-03) — Create/edit dialog (space_room_dialog.dart) + dialog tests + scaffold contrast ticket

**Wave 4** *(blocked on Wave 3 completion)*

- [x] 02-05-PLAN.md — Wave 4 (depends: 02-03, 02-04) — Rail tree render (sections/badges/affordances/dimming) + rail widget tests

**UI hint**: yes

### Phase 3: Messaging

**Goal**: Users can send and receive text messages in real-time, with all room participants converging on the same message history via CRDT.
**Depends on**: Phase 2
**Requirements**: CORE-04
**Success Criteria** (what must be TRUE):

  1. User can send a text message to a room and see it appear locally
  2. A second user in the same room receives the message without manual refresh
  3. Message history is identical across all participants after sync
  4. Messages display in chronological order with sender identification

**Plans:** 6/6 plans complete

Plans:
**Wave 1**

- [x] 03-01-PLAN.md — Wave 1 — SuperGenius/GeniusSDK signature verification (QueryKeyValues/PutLocal/RegisterNewElementCallback/GetAddress/Publish/Subscribe) + append-only proto additions (sender/deleted/deleted_at_ms + MessageHistory)

**Wave 2** *(blocked on Wave 1 completion)*

- [x] 03-02-PLAN.md — Wave 2 (depends: 03-01) — Storage widening (topics-aware Put, PutLocal, QueryKeyValues, RegisterNewElementCallback, raw full-value Publish/Subscribe + CoreSession pass-throughs) + storage tests
- [x] 03-03-PLAN.md — Wave 2 (depends: 03-01) — Dart pb regen + MessageFlowCubit upsert/replaceAll + SessionCubit messageHistory dispatch + cubit tests

**Wave 3** *(blocked on Wave 2 completion)*

- [x] 03-04-PLAN.md — Wave 3 (depends: 03-01, 03-02) — gcs::Messaging component (live full-value publish + archive send, two-route dedupe, history) + test_gcs_messaging (TDD)

**Wave 4** *(blocked on Wave 3 completion)*

- [x] 03-05-PLAN.md — Wave 4 (depends: 03-04) — FFI wiring (send_text delegate, join-time history replay, live subscribe + CRDT receive bridge)

**Wave 5** *(blocked on Wave 4 completion)*

- [x] 03-06-PLAN.md — Wave 5 (depends: 03-03, 03-05) — Human-verify checkpoint (live send/receive + history convergence)

**UI hint**: yes

### Phase 4: Membership & Invites

**Goal**: Users can invite others via capability tokens, and the five-tier role model (Super Admin → Guest) governs what actions each participant can take.
**Depends on**: Phase 3
**Requirements**: MEMB-01, MEMB-02, MEMB-03, MEMB-04, MEMB-05
**Success Criteria** (what must be TRUE):

  1. User can generate an invite link (`gcs://invite/...`) for a space or room
  2. Recipient can use the invite to join and appears in the member list
  3. Each participant has a visible role (Super Admin, Admin, Moderator, Member, Guest)
  4. Super Admin cannot be demoted by other admins
  5. When 2+ admins exist, destructive actions require Super Admin approval

**Plans:** 23/23 plans complete

Plans:
**Wave 1**

- [x] 04-01-PLAN.md — Wave 1 — Append-only proto contract (Role/MemberRecord/MemberList/InviteLink + creator/members_can_invite/approved_by + 7 command/2 event arms)
- [x] 04-02-PLAN.md — Wave 1 — Crypto key-wrap + per-room key store + WR-02 HKDF-ikm swap (D-06)

**Wave 2** *(blocked on Wave 1 completion)*

- [x] 04-04-PLAN.md — Wave 2 (depends: 04-01) — EntityStore creator + members_can_invite stamping (P1/P2/P3) + UpdateSpace preservation
- [x] 04-06-PLAN.md — Wave 2 (depends: 04-01) — Dart pb regen + MembersCubit + SessionCubit dispatch arms

**Wave 3** *(blocked on Wave 2 completion)*

- [x] 04-03-PLAN.md — Wave 3 (depends: 04-01, 04-02, 04-04) — gcs::Membership component (mint/redeem/role/remove/leave/delete/approve + D-03/D-07/D-08 guards) + unit/multinode tests
- [x] 04-07-PLAN.md — Wave 3 (depends: 04-06) — Membership UI (members dialog, join dialog, rail affordances, MembersCubit wiring)

**Wave 4** *(blocked on Wave 3 completion)*

- [x] 04-05-PLAN.md — Wave 4 (depends: 04-03, 04-04) — FFI wiring (membership arms + /gcs/members/ heal callback + P5 retroactive guards + D-02 explicit_leave)

**Wave 5** *(blocked on Wave 4 completion — gap closure)*

- [x] 04-08-PLAN.md — Wave 5 (gap CR-01) — EntityStore catalog synchronization (mutex-guarded maps, never held across a Put) + concurrent reader/writer stress test
- [x] 04-09-PLAN.md — Wave 5 (gaps WR-02/03/04/05/06) — Membership rank guards, every-invite bearer authenticator (HKDF/AES-GCM + seam callables) with kind cross-check and collision rejection, tombstone-arrival roster push, well-known replication topic + appended proto fields, and the multinode fixture moved onto that topic

**Wave 6** *(blocked on Wave 5 completion — gap closure)*

- [x] 04-10-PLAN.md — Wave 6 (gap WR-07) — Dart: scope-keyed pending invite link + pinned 22.5.0 pb regen
- [x] 04-11-PLAN.md — Wave 6 (gaps WR-01/06/07) — FFI: g_membership teardown eviction, membership-topic registration at init before the session global is installed, production invite-auth seam wiring, scope-stamped InviteLink push
- [x] 04-12-PLAN.md — Wave 6 (gaps WR-06/04/05) — Multinode cases only (fixture lives in 04-09): space-invite convergence with no per-scope registration, wire-level every-invite bearer-gate proof, remote-removal push

**Wave 7** *(blocked on Wave 6 completion — re-verification gap closure)*

- [x] 04-13-PLAN.md — Wave 7 (gaps CR-01/CR-02) — RedeemInvite preserves live member state (role/pending/approved_by) and rejects tombstoned wallets, with dormant-removal and no-write regression tests

**Wave 8** *(blocked on Wave 7 completion — gap closure)*

- [x] 04-14-PLAN.md — Wave 8 (gaps CR-06/CR-05) — EntityStore ApplySpaceState/ApplyRoomState deletion mutators (catalog-synchronized, Put outside the mutex) + DeleteScope/Approve routed through them + AdminCount fail-closed with an injected-scan regression test

**Wave 9** *(blocked on Wave 8 completion — gap closure)*

- [x] 04-15-PLAN.md — Wave 9 (gap CR-08) — Issuer-signed invite records: append-only created_by/issuer_signature, public hex codec, sign/verify seam, scoped URL + exact-key lookup, redeem signature/authority/scope checks, unit forgery/authority/scope proofs

**Wave 10** *(blocked on Wave 9 completion — gap closure)*

- [x] 04-16-PLAN.md — Wave 10 (gap CR-08 production) — FFI account-backed issuer seam (fail closed) + scoped-link FFI tests + pinned Dart pb regen + two-segment join-link gate

**Wave 11** *(blocked on Wave 10 completion — gap closure)*

- [x] 04-17-PLAN.md — Wave 11 (gaps CR-03/CR-04) — CanChangeRole concrete-role and strict-rank guards (creator exception preserved) + role/rank authorization tests
- [x] 04-18-PLAN.md — Wave 11 (gap CR-08 wire proof) — Multinode real-signer fixture: genuine cross-node redeem accepted, forged issuer with a valid bearer authenticator rejected with baseline capture

**Wave 12** *(blocked on Wave 11 completion — re-verification gap closure, truth 29)*

- [x] 04-19-PLAN.md — Wave 12 — Signed member records: append-only created_by/record_signature, canonical payload, author-signed vs authority-preserving writes, verified reads, self-signed Guest cap, creator-rooted authority chain

**Wave 13** *(blocked on Wave 12 completion)*

- [x] 04-20-PLAN.md — Wave 13 (depends: 04-19) — Signed SeedMember + shared test payload helper, flipped no-seam/peer-Super-Admin expectations, trust-chain exclusion and authority-preservation tests

**Wave 14** *(blocked on Wave 13 completion)*

- [x] 04-21-PLAN.md — Wave 14 (depends: 04-20) — WR-02 identity-based creator exception, WR-03 approval re-validation, WR-05 pending-removal barrier and canonical clearing

**Wave 15** *(blocked on Wave 14 completion)*

- [x] 04-22-PLAN.md — Wave 15 (depends: 04-19, 04-20, 04-21) — Multinode cold-node scope mirror + wire-level forged self-claimed Super Admin exclusion and command rejection + full-suite gate

**Wave 16** *(blocked on Wave 15 completion — verification gap closure)*

- [x] 04-23-PLAN.md — Wave 16 (depends: 04-19, 04-20, 04-21) — Reject cross-scope raw member records in RedeemInvite/Leave before the authority-preserving write (CR-01, truth 29) + byte-level regression

**Deferred (recorded, not in this gap-closure set):** CR-07 — room keys live in a process-local store, so a second application process (or a restart without the invite material) cannot decrypt a room's key material. Warning-level; no Phase 4 requirement depends on cross-process key persistence, and no plan in Waves 5-11 addresses it.

**UI hint**: yes

### Phase 4.1: Room-Session Service (INSERTED)

**Goal**: A room-session service in `src/lib` owns which rooms a session is joined to and why — the joined-topic registry, the composed join predicate, the send/create/update permission gates, and the topic grammar/length policies — while `gcs_core_ffi.cpp` validates arguments and delegates with no room-topic registry state.
**Depends on**: Phase 4 (complete; inserted before v1.0 Phase 5)
**Requirements**: CPLX-01, CPLX-02, CPLX-03
**Success Criteria** (what must be TRUE):

  1. Joined-room state is owned by one `src/lib` service: join dedup, re-join (refresh-only), D-07 listen-then-broadcast ordering, derived/explicit partition, eviction, and the send-requires-joined check; `src/ffi/gcs_core_ffi.cpp` holds no room-topic registry globals (a grep for the registry names finds none).
  2. The composed join predicate — `(room.creator == self || parentSpace.autoJoinRooms) && !explicitLeave` — is assembled in `src/lib` from `EntityStore::DerivedJoinedTopics` and `Membership::HasExplicitLeave`, never in the adapter.
  3. The send/create/update permission gates (including `RoleAtLeast` and scope/roster resolution) are computed in `src/lib`; each FFI arm validates arguments and delegates with no local rank or policy expression.
  4. Room-topic grammar, topic/name/message length limits, and topic↔id derivation have a single definition in `src/lib`, referenced by the adapter.
  5. Full regression is green and behavior is unchanged: `ctest --test-dir build/OSX/Debug` passes 13/13 (plus the new lib-only room-session suite) and the Flutter suite (`flutter test` in `src/app`) passes; the joined set, pushed event sequence/payloads, and error results are identical to the pre-phase baseline.

**Plans:** 4 plans

Plans:
- [ ] 04.1-01-PLAN.md — Wave 1 — Lib policy predicates: `EntityStore::IsNameTooLong` (64 code points), `Messaging::IsMessageTextTooLong` (4096 bytes), `Membership::RoleAtLeast` (D-08 rank) with boundary tests
- [ ] 04.1-02-PLAN.md — Wave 2 (depends: 04.1-01) — `src/lib/gcs_room_session.hpp/.cpp`: registry, listen-then-broadcast join, derived recompute + leave term + eviction, three permission denials
- [ ] 04.1-03-PLAN.md — Wave 3 (depends: 04.1-02) — `test/test_gcs_room_session.cpp` lib-only suite (no `gcs_ffi`) + shared key-wrap seam double; full regression
- [ ] 04.1-04-PLAN.md — Wave 4 (depends: 04.1-03) — FFI handoff: install RoomSession at init, delegate join/send/derived/gates, delete registry globals, grammar helpers, rank/roster helpers, and name/text caps

### Phase 4.2: Storage & Prefix Single-Homing (INSERTED)

**Goal**: The lib's only cross-domain raw-key read and the duplicated room-topic prefix are gone — `Membership::Approve` reads entity state through `EntityStore` functions, and `"gcs/chat/"` has one C++ definition.
**Depends on**: Phase 4.1
**Requirements**: CPLX-04, CPLX-05
**Success Criteria** (what must be TRUE):

  1. `Membership::Approve` obtains space/room state through `EntityStore` functions (the existing `ApplySpaceState`/`ApplyRoomState` path or an equivalent accessor on the same owner); `gcs_membership.cpp` no longer builds raw `gcs/entities/...` keys or parses `SpaceRecord`/`RoomRecord` itself.
  2. A key-literal grep finds the entity key prefixes only in the `EntityStore` translation unit, and Approve's outcomes are unchanged: same approval/dormancy decisions, same error codes, existing membership tests passing with no expectation edits.
  3. `"gcs/chat/"` has exactly one C++ definition in `src/lib`; `gcs_entity_store.cpp`, `gcs_membership.cpp`, and `gcs_core_ffi.cpp` all reference it and no other TU restates it.
  4. Full regression is green and behavior is unchanged: `ctest --test-dir build/OSX/Debug` passes 13/13 and the Flutter suite passes; error results, event ordering, and event payloads are identical.

*Scope note: the Dart `'gcs/chat/$id'` getter is an interpolated client-side derivation, not a prefix constant definition; it is unchanged because pushing a per-room topic field would alter event payloads.*

**Plans**: TBD

### Phase 4.3: Dart Predicate Single-Homing (INSERTED)

**Goal**: The Flutter app no longer re-derives capability and dormancy rules in multiple homes — each predicate has exactly one Dart definition that every call site uses, while C++ remains the enforcement authority.
**Depends on**: Phase 4.1 (settled pushed-event surface; the Dart work itself is independent of the C++ moves)
**Requirements**: CPLX-06
**Success Criteria** (what must be TRUE):

  1. Capability predicates — `viewerCanManageMembers`, `viewerIsSuperAdmin`, `viewerCanDeleteScope`, the MEMB-05 invite gate, and the row/affordance gates including D-03 Super-Admin immunity — each have exactly one Dart definition; `members_dialog.dart` and `room_rail.dart` call it instead of restating the matrix.
  2. The D-07 dormancy predicate (empty `approvedBy` with a non-unspecified `pendingAction`) has exactly one Dart definition used by the members cubit and both rail call sites.
  3. Every capability/dormancy decision is unchanged for the role and record states covered by the existing Flutter tests (no expectation edits), and presentation still only displays pushed state — no rule is recomputed from wire bytes, and no business logic enters templates.
  4. Full regression is green and behavior is unchanged: the Flutter suite in `src/app` passes, `ctest --test-dir build/OSX/Debug` passes 13/13, and pushed event payloads are unchanged (no proto field added).

*Route: single-home in Dart. The push-computed-flags alternative would add proto fields and change event payload bytes, which the milestone's behavior-preserving gate forbids.*

**Plans**: TBD

### Phase 4.4: Enforcement Gate (INSERTED)

**Goal**: The demotion is enforced, not promised — a lib-only test target drives the room lifecycle without `gcs_ffi`, and a canonical-payload tripwire makes payload drift fail the test suite.
**Depends on**: Phases 4.1, 4.2 (the gate must exercise the consolidated lib shape)
**Requirements**: CPLX-07, CPLX-08
**Success Criteria** (what must be TRUE):

  1. A new test target links `gcs_core` (and its storage/support libraries) with no `gcs_ffi` dependency and registers in ctest.
  2. That target drives a room lifecycle through the lib service alone — create a space with `autoJoinRooms`, create a nested room, explicit leave, re-derive — and asserts the joined set; it cannot pass by going through the adapter because the adapter is not linked.
  3. The room-session service's tests run in the lib-only target, not only in FFI-level tests.
  4. A canonical-payload tripwire compares the production member/entity canonical payload builders with the test-helper mirrors; a change to either side alone makes the test fail. The production builders are reachable from the lib-only target without the FFI.
  5. Full regression is green and behavior is unchanged: every registered ctest test passes (13/13 pre-existing plus the new lib-only target) and the Flutter suite passes; error results, event ordering, and event payloads are identical.

**Plans**: TBD

### Phase 5: Moderation

**Goal**: Moderators and admins can manage room content and membership through tombstone-based CRDT ops that preserve history while hiding removed content.
**Depends on**: Phase 4
**Requirements**: MODR-01, MODR-02, MODR-03
**Success Criteria** (what must be TRUE):

  1. Moderator can delete a message and it disappears from all participants' views
  2. Deleted message content is preserved in the CRDT (tombstone, not removal)
  3. Moderator can kick a user from a room; the user loses access immediately
  4. Moderator can ban a user; the ban persists across rejoin attempts

**Plans**: TBD
**UI hint**: yes

### Phase 6: GCS Bot

**Goal**: The GCS system joins rooms as a participant and responds to messages according to the room's `autoAnswer` policy, routing requests through capability-first Expert Model selection (ELM/EJM/EDM contracts).
**Depends on**: Phase 3
**Requirements**: GCSB-01, GCSB-02, GCSB-03, GCSB-04
**Success Criteria** (what must be TRUE):

  1. GCS bot appears in the room participant list
  2. When `autoAnswer: GCS`, the bot responds to every message automatically
  3. When `autoAnswer: None`, the bot responds only when `@gcs` is mentioned
  4. Bot responses are routed through the correct Expert Model (ELM/EJM/EDM) for the query type
  5. Bot messages appear in the same CRDT message stream as human messages

**Plans**: TBD
**UI hint**: yes

### Phase 7: Discovery

**Goal**: Users can browse public spaces in a global lobby, while private spaces remain invisible to the network.
**Depends on**: Phase 2
**Requirements**: DISC-01, DISC-02, DISC-03
**Success Criteria** (what must be TRUE):

  1. Creating a public space publishes its existence to the lobby topic
  2. Private spaces never appear in the lobby
  3. User can open the lobby and see a list of public spaces
  4. User can join a public space directly from the lobby

**Plans**: TBD
**UI hint**: yes

## Progress

**Execution Order:**
Phases execute in numeric order: 1 → 2 → 3 → 4 → 4.1 → 4.2 → 4.3 → 4.4 → 5 → 6 → 7. Phases 4.1–4.4 are the inserted v1.0.1 milestone; v1.0 resumes at Phase 5.

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Foundation | 11/11 | Complete | 2026-09-16 |
| 2. Spaces & Rooms | 5/5 | Complete   | 2026-09-19 |
| 3. Messaging | 6/6 | Complete   | 2026-09-24 |
| 4. Membership & Invites | 23/23 | Complete   | 2026-10-21 |
| 4.1. Room-Session Service | 0/TBD | Not started | - |
| 4.2. Storage & Prefix Single-Homing | 0/TBD | Not started | - |
| 4.3. Dart Predicate Single-Homing | 0/TBD | Not started | - |
| 4.4. Enforcement Gate | 0/TBD | Not started | - |
| 5. Moderation | 0/TBD | Not started | - |
| 6. GCS Bot | 0/TBD | Not started | - |
| 7. Discovery | 0/TBD | Not started | - |
