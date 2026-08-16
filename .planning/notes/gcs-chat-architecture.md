---
title: GCS Chat Architecture
date: 2026-08-15
context: gs-explore --ws app
---

# GCS Chat Architecture

Multi-party group chat where one participant is the GCS system (ELM router / specialist models).

## Room Types

Single unified model. Per-participant `autoAnswer` policy controls bot behavior:

| Policy | Trigger | Use Case |
|--------|---------|----------|
| **None** | `@gcs` mention only | Multi-party human chat |
| **GCS** | Every message | Q&A mode, solo chat |

## Pub/Sub Topics

- **Room topic:** `ipfs-pubsub://gcs/chat/<roomname>`
- **Lobby topic:** `ipfs-pubsub://gcs/chat/lobby` — public room registry (CRDT)

Public rooms publish existence to lobby. Private rooms never publish to lobby.

## Privacy

App-layer encryption (not libp2p PSK). Per-room symmetric key encrypts message payloads before CRDT publish. Existence visible on pubsub network, content hidden. Key rotation deferred to post-MVP.

## Message Flow

1. User sends message → Flutter Cubit → FFI → C++ GCS core
2. C++ publishes encrypted payload to room topic via GossipPubSub
3. CRDT (GlobalDB) syncs op to all room participants
4. GCS bot participant filters message per `autoAnswer` policy
5. If triggered, bot generates response → publishes as new op

## MVP Scope

- Room creation (name → topic)
- Message send/receive with CRDT sync
- GCS bot participant with mention/every-message auto-answer
- No encryption yet (public rooms only)

## Deferred

- Private room key rotation
- Room archive / lifecycle management
- Room discovery beyond lobby (search, QR codes)
