# Requirements: Genius Cognitive System — App Workstream

**Defined:** 2026-08-15
**Core Value:** Users can create a space, invite others, and have a group conversation where an AI participant (GCS) responds to questions — all synchronized via CRDT without central servers.

## v1 Requirements

Requirements for initial release. Each maps to roadmap phases.

### Chat Core

- [x] **CORE-01**: User can create a space (public or private)
- [x] **CORE-02**: User can create a room within a space or standalone
- [x] **CORE-03**: User can configure space `autoJoinRooms` setting
- [x] **CORE-04**: User can send and receive text messages in real-time
- [x] **CORE-05**: Messages sync via CRDT across all room participants

### Membership

- [x] **MEMB-01**: User can invite others via capability token (`gcs://invite/...`)
- [x] **MEMB-02**: Space/room has roles: Super Admin, Admin, Moderator, Member, Guest
- [x] **MEMB-03**: Super Admin (creator) cannot be demoted by other admins
- [x] **MEMB-04**: Destructive actions require Super Admin approval when 2+ admins exist
- [x] **MEMB-05**: Members can invite others if `membersCanInvite` is enabled

### Moderation

- [ ] **MODR-01**: Moderators can delete messages (tombstone op)
- [ ] **MODR-02**: Moderators can kick or ban users
- [ ] **MODR-03**: Deleted messages are preserved in CRDT but hidden from display

### GCS Bot

- [ ] **GCSB-01**: GCS bot joins rooms as a participant
- [ ] **GCSB-02**: Bot auto-answers every message when `autoAnswer: GCS`
- [ ] **GCSB-03**: Bot responds only to `@gcs` mentions when `autoAnswer: None`
- [ ] **GCSB-04**: Bot response routes through capability-first Expert Model selection (ELM/EJM/EDM contracts; rule-based MVP router per `docs/architecture/model-and-router.md` §6.2)

### Discovery

- [ ] **DISC-01**: Public spaces publish existence to lobby topic
- [ ] **DISC-02**: Private spaces never appear in lobby
- [ ] **DISC-03**: User can browse public spaces in lobby

## v1.0.1 Requirements (Coupling Remediation)

Behavior-preserving refactor from the [whole-project coupling audit](../../notes/whole-project-coupling-audit.md) (2026-11-03). D-02a creator auto-join (the one sanctioned behavior change) shipped first — develop `e4a548c`. Cross-cutting acceptance for every phase: full regression green (13/13 ctest + Flutter suite) and zero observable behavior change (error results, ordering, event payloads identical).

### Coupling Remediation

- [ ] **CPLX-01**: Joined-room state (registry dedup, re-join, D-07 listen-then-broadcast ordering, derived/explicit partition, eviction) is owned by a service in `src/lib`; `gcs_core_ffi.cpp` retains no room-topic registry globals
- [ ] **CPLX-02**: The composed join predicate (D-02/D-02a + explicit-leave) and the send/create/update permission gates (incl. RoleAtLeast) are computed in `src/lib`; FFI arms validate arguments and delegate
- [ ] **CPLX-03**: Topic grammar and length policies have a single home in `src/lib`
- [ ] **CPLX-04**: `Membership::Approve` reads entity state through `EntityStore` functions; `src/lib` contains no cross-domain raw entity-key access
- [ ] **CPLX-05**: The `"gcs/chat/"` topic prefix constant has exactly one definition in the codebase
- [ ] **CPLX-06**: Dart capability/dormancy predicates have a single home or become computed flags pushed from C++ (the `self_address` precedent)
- [ ] **CPLX-07**: A canonical-payload tripwire test pins the member/entity record payload shapes and fails on unauthorized drift
- [ ] **CPLX-08**: A lib-only test target links `gcs_core` without `gcs_ffi`, and lib tests run in it (the audit's structural gate)

## v2 Requirements

Deferred to future release. Tracked but not in current roadmap.

### Encryption

- **ENCR-01**: Private spaces/rooms use app-layer symmetric key encryption
- **ENCR-02**: Key rotates when members leave
- **ENCR-03**: DMs auto-created with encryption on first message

### Rich Content

- **CONT-01**: User can attach files to messages
- **CONT-02**: User can reply in threads
- **CONT-03**: User can react to messages with emoji

### Genius-Tube Integration

- **GTUB-01**: Channel extends Space with creator branding
- **GTUB-02**: Stream/VOD extends Room with video metadata
- **GTUB-03**: Subscriber extends Member with payment state

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Feature | Reason |
|---------|--------|
| Video/audio streaming | Deferred to genius-tube extension |
| End-to-end encryption | MVP uses app-layer symmetric keys only |
| Message editing | Append-only CRDT for MVP |
| File attachments | Text-only for MVP |
| Mobile push notifications | Desktop-first |
| Centralized user accounts | P2P identity via libp2p peer ID |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Phase | Status |
|-------------|-------|--------|
| CORE-01 | Phase 2 | Complete |
| CORE-02 | Phase 2 | Complete |
| CORE-03 | Phase 2 | Complete |
| CORE-04 | Phase 3 | Complete |
| CORE-05 | Phase 1 | Complete |
| MEMB-01 | Phase 4 | Complete |
| MEMB-02 | Phase 4 | Complete |
| MEMB-03 | Phase 4 | Complete |
| MEMB-04 | Phase 4 | Complete |
| MEMB-05 | Phase 4 | Complete |
| MODR-01 | Phase 5 | Pending |
| MODR-02 | Phase 5 | Pending |
| MODR-03 | Phase 5 | Pending |
| GCSB-01 | Phase 6 | Pending |
| GCSB-02 | Phase 6 | Pending |
| GCSB-03 | Phase 6 | Pending |
| GCSB-04 | Phase 6 | Pending |
| DISC-01 | Phase 7 | Pending |
| DISC-02 | Phase 7 | Pending |
| DISC-03 | Phase 7 | Pending |

**Coverage:**
- v1 requirements: 20 total
- Mapped to phases: 20
- Unmapped: 0

---
*Requirements defined: 2026-08-15*
*Last updated: 2026-08-15 after roadmap creation*
