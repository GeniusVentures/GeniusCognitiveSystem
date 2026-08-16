---
title: GCS Chat Architecture
date: 2026-08-15
context: gs-explore --ws app
---

# GCS Chat Architecture

Multi-party group chat where one participant is the GCS system (ELM router / specialist models).

## Entity Hierarchy

| Layer | Purpose | Topic | Access |
|-------|---------|-------|--------|
| **Space** | Container, invite management, room grouping | `gcs/spaces/<space_name>` | Public or private |
| **Room** | Chat, message sync | `gcs/chat/<room_name>` | Public or private |
| **DM** | 1:1 private chat | `gcs/chat/<dm_id>` | Private (auto-generated) |
| **Lobby** | Global discovery of public spaces | `gcs/spaces/lobby` | Public registry |

## Space Configuration

Space CRDT carries config ops controlling room inheritance:

| Config | Behavior |
|--------|----------|
| `autoJoinRooms: true` | Space members auto-join all rooms; space key unlocks everything |
| `autoJoinRooms: false` | Rooms independent; space membership ≠ room access |

Config ops are CRDT writes to the space topic. Clients process changes and update local join behavior.

## Invite Model

| Entity | Mechanism | Format |
|--------|-----------|--------|
| Space | Space key → unlocks all rooms (if autoJoin) | `gcs://invite/space/<id>?key=<key>` |
| Room | Room key → unlocks single room | `gcs://invite/room/<id>?key=<key>` |
| DM | Auto-generated room key | Implicit on creation |

Room invites work standalone or reference a parent space. A user can be in a space without being in a private room within it.

## Room Types (by auto-answer policy)

Single unified model. Per-participant `autoAnswer` policy controls bot behavior:

| Policy | Trigger | Use Case |
|--------|---------|----------|
| **None** | `@gcs` mention only | Multi-party human chat |
| **GCS** | Every message | Q&A mode, solo chat |

## Membership Types

| Level | Capabilities | Constraints |
|-------|-----------|-------------|
| **Super Admin** | Full control; cannot be demoted by other admins | Singular — creator only |
| **Admin** | Full control: config, membership, moderation | Multiple allowed |
| **Moderator** | Manage members, delete messages, pin content | Trusted participants |
| **Member** | Read, write, invite (if allowed) | Standard participant |
| **Guest** | Read-only | Temporary access |

### Invite Permission

Configurable per space/room: `membersCanInvite: true/false`

### Destructive Action Rules

| Condition | Required Approvals |
|-----------|------------------|
| Only 1 admin exists | 1 (that admin) |
| 2+ admins exist | Super Admin (creator) must approve |

No timeouts — actions execute immediately once threshold met.

### Membership CRDT Op

```
Op::Membership {
  target: <user_id>,
  action: Add | Remove | ChangeRole,
  role: SuperAdmin | Admin | Moderator | Member | Guest,
  scope: Space | Room,
  author: <actor_id>,
  approvals: [<super_admin_id>]?,  // required for destructive when >1 admin
  timestamp: <lamport>
}
```

### Moderation CRDT Op

```
Op::Moderation {
  action: DeleteMessage | PinMessage | KickUser | BanUser,
  target: <message_id | user_id>,
  reason: <optional_string>,
  author: <moderator_id>
}
```

Moderation ops are tombstones — content preserved in CRDT, filtered from display.

## Privacy

App-layer encryption (not libp2p PSK). Per-room symmetric key encrypts message payloads before CRDT publish. Existence visible on pubsub network, content hidden. Key rotation deferred to post-MVP.

Public rooms publish existence to lobby. Private rooms never publish to lobby.

## Message Flow

1. User sends message → Flutter Cubit → FFI → C++ GCS core
2. C++ publishes encrypted payload to room topic via GossipPubSub
3. CRDT (GlobalDB) syncs op to all room participants
4. GCS bot participant filters message per `autoAnswer` policy
5. If triggered, bot generates response → publishes as new op

## MVP Scope

- Space creation (public → lobby, private → invite link)
- Room creation within space or standalone
- Message send/receive with CRDT sync
- GCS bot participant with mention/every-message auto-answer
- Space/room invites via capability tokens
- No encryption yet (public spaces/rooms only)

## Deferred

- Private room/space key rotation
- Room archive / lifecycle management
- Space discovery beyond lobby (search, QR codes)
- DM auto-creation on first message
