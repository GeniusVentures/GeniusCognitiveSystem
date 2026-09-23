---
title: Design message CRDT schema for chat room ops
date: 2026-08-15
priority: high
context: gs-explore --ws app
resolves_phase: 3
---

# Design message CRDT schema

Define the CRDT operation structure for chat messages.

## Questions to resolve

- What fields does a message op carry? (sender, timestamp, content, room, reply-to?)
- How do we handle message ordering across clients? (CRDT vector clocks, or simple lamport timestamp?)
- Do we need message deletion/editing ops, or append-only?
- How does the schema accommodate encrypted payloads for private rooms?

## Dependencies

- GCS GlobalDB CRDT from Phase 3 (neoswarm workstream)
- GossipPubSub topic structure from architecture note

## Acceptance

Schema documented in `src/lib/` or planning docs, with example ops for create/edit/delete.
