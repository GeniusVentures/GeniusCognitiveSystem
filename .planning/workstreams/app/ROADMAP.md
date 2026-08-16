# Roadmap: Genius Cognitive System — App Workstream (v1.0 GCS Chat)

## Overview

Build a working multi-party chat application where users create spaces and rooms, invite participants via capability tokens, chat in real-time via CRDT sync, and interact with the GCS bot as an AI participant — all without central servers. The roadmap progresses from C++ core foundation through entity management (spaces/rooms), messaging, membership, moderation, bot integration, and finally public discovery.

## Phases

**Phase Numbering:**
- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

- [ ] **Phase 1: Foundation** - C++ core scaffolding, GlobalDB CRDT integration, FFI bridge to Flutter, and cross-platform CI/CD
- [ ] **Phase 2: Spaces & Rooms** - Space and room creation with configurable inheritance
- [ ] **Phase 3: Messaging** - Real-time text messaging with CRDT sync across participants
- [ ] **Phase 4: Membership & Invites** - Roles, capability tokens, and permission model
- [ ] **Phase 5: Moderation** - Message tombstones, kick/ban, and admin approval flows
- [ ] **Phase 6: GCS Bot** - Bot participant with auto-answer policies and ELM routing
- [ ] **Phase 7: Discovery** - Public space lobby and private space isolation

## Phase Details

### Phase 1: Foundation
**Goal**: The C++ chat core compiles, links against GlobalDB, and exposes a working FFI surface that Flutter can call.
**Depends on**: Nothing (first phase)
**Requirements**: CORE-05
**Success Criteria** (what must be TRUE):
  1. C++ core library builds on macOS, Linux, and Windows without errors
  2. GlobalDB CRDT instance can be initialized and a test op can be published/subscribed locally
  3. Flutter app can call into C++ via FFI and receive a response
  4. GossipSub topics can be created and joined from the C++ core
  5. CI/CD pipeline builds and tests on self-hosted runners for macOS, Linux, Windows
**Plans**: TBD

### Phase 2: Spaces & Rooms
**Goal**: Users can create spaces and rooms with configurable inheritance, and the entity hierarchy is persisted via CRDT.
**Depends on**: Phase 1
**Requirements**: CORE-01, CORE-02, CORE-03
**Success Criteria** (what must be TRUE):
  1. User can create a public or private space and see it in their local space list
  2. User can create a room within a space or as a standalone room
  3. User can toggle `autoJoinRooms` on a space and observe the join behavior change
  4. Space/room metadata survives app restart (CRDT persistence)
**Plans**: TBD
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
**Plans**: TBD
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
**Plans**: TBD
**UI hint**: yes

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
**Goal**: The GCS system joins rooms as a participant and responds to messages according to the room's `autoAnswer` policy, routing requests through ELM specialist selection.
**Depends on**: Phase 3
**Requirements**: GCSB-01, GCSB-02, GCSB-03, GCSB-04
**Success Criteria** (what must be TRUE):
  1. GCS bot appears in the room participant list
  2. When `autoAnswer: GCS`, the bot responds to every message automatically
  3. When `autoAnswer: None`, the bot responds only when `@gcs` is mentioned
  4. Bot responses are routed through the correct ELM specialist for the query type
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
Phases execute in numeric order: 1 → 2 → 3 → 4 → 5 → 6 → 7

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Foundation | 0/TBD | Not started | - |
| 2. Spaces & Rooms | 0/TBD | Not started | - |
| 3. Messaging | 0/TBD | Not started | - |
| 4. Membership & Invites | 0/TBD | Not started | - |
| 5. Moderation | 0/TBD | Not started | - |
| 6. GCS Bot | 0/TBD | Not started | - |
| 7. Discovery | 0/TBD | Not started | - |
