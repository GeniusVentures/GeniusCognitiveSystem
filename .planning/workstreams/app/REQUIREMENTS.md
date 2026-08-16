# Requirements: Genius Cognitive System — App Workstream

**Defined:** 2026-08-15
**Core Value:** Users can create a space, invite others, and have a group conversation where an AI participant (GCS) responds to questions — all synchronized via CRDT without central servers.

## v1 Requirements

Requirements for initial release. Each maps to roadmap phases.

### Chat Core

- [ ] **CORE-01**: User can create a space (public or private)
- [ ] **CORE-02**: User can create a room within a space or standalone
- [ ] **CORE-03**: User can configure space `autoJoinRooms` setting
- [ ] **CORE-04**: User can send and receive text messages in real-time
- [ ] **CORE-05**: Messages sync via CRDT across all room participants

### Membership

- [ ] **MEMB-01**: User can invite others via capability token (`gcs://invite/...`)
- [ ] **MEMB-02**: Space/room has roles: Super Admin, Admin, Moderator, Member, Guest
- [ ] **MEMB-03**: Super Admin (creator) cannot be demoted by other admins
- [ ] **MEMB-04**: Destructive actions require Super Admin approval when 2+ admins exist
- [ ] **MEMB-05**: Members can invite others if `membersCanInvite` is enabled

### Moderation

- [ ] **MODR-01**: Moderators can delete messages (tombstone op)
- [ ] **MODR-02**: Moderators can kick or ban users
- [ ] **MODR-03**: Deleted messages are preserved in CRDT but hidden from display

### GCS Bot

- [ ] **GCSB-01**: GCS bot joins rooms as a participant
- [ ] **GCSB-02**: Bot auto-answers every message when `autoAnswer: GCS`
- [ ] **GCSB-03**: Bot responds only to `@gcs` mentions when `autoAnswer: None`
- [ ] **GCSB-04**: Bot response routes through ELM specialist selection

### Discovery

- [ ] **DISC-01**: Public spaces publish existence to lobby topic
- [ ] **DISC-02**: Private spaces never appear in lobby
- [ ] **DISC-03**: User can browse public spaces in lobby

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
| CORE-01 | TBD | Pending |
| CORE-02 | TBD | Pending |
| CORE-03 | TBD | Pending |
| CORE-04 | TBD | Pending |
| CORE-05 | TBD | Pending |
| MEMB-01 | TBD | Pending |
| MEMB-02 | TBD | Pending |
| MEMB-03 | TBD | Pending |
| MEMB-04 | TBD | Pending |
| MEMB-05 | TBD | Pending |
| MODR-01 | TBD | Pending |
| MODR-02 | TBD | Pending |
| MODR-03 | TBD | Pending |
| GCSB-01 | TBD | Pending |
| GCSB-02 | TBD | Pending |
| GCSB-03 | TBD | Pending |
| GCSB-04 | TBD | Pending |
| DISC-01 | TBD | Pending |
| DISC-02 | TBD | Pending |
| DISC-03 | TBD | Pending |

**Coverage:**
- v1 requirements: 20 total
- Mapped to phases: 0
- Unmapped: 20 ⚠️

---
*Requirements defined: 2026-08-15*
*Last updated: 2026-08-15 after milestone v1.0 creation*
