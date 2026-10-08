# Phase 2: Spaces & Rooms - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-09-18
**Phase:** 2-Spaces & Rooms
**Areas discussed:** Topic namespacing, CRDT encoding + events, autoJoinRooms semantics, Creation UI flows

**Mode:** Advisor (research-backed comparison tables; 4 parallel gsd-advisor-researcher agents, minimal_decisive calibration, model sonnet)

---

## Topic namespacing & room-name collisions

| Option | Description | Selected |
|--------|-------------|----------|
| Opaque IDs | Topics `gcs/chat/<id>`; name is a display attribute on the entity record. Renames/reparenting = one metadata Put. Ids minted C++-side; standalone room = empty parent. | ✓ |
| Space-scoped topics | Topics `gcs/chat/<space>/<room>`. Readable on the wire, but renames/moves split the CRDT namespace and collisions recur at the space layer. | |

**User's choice:** Opaque IDs
**Notes:** Researcher dropped "globally-unique room names" before tabling — a registry-free gossip network cannot enforce global uniqueness. Architecture note's own artifacts (invite URLs `gcs://invite/room/<id>`, auto-generated DM ids) already presupposed opaque ids.

---

## CRDT entity encoding & hierarchy events

| Option | Description | Selected |
|--------|-------------|----------|
| Hybrid records + manifest | Per-entity keys `gcs/entities/{spaces,rooms}/<id>` + ID manifest key + new SpaceTree event arm. LWW confined to single-entity edits where it's correct; RoomList untouched. | ✓ |
| Space-blob + RoomList field | One blob per space embedding its rooms; RoomList gains a structured field. Simpler reads, but concurrent room creation permanently loses a room under LWW-per-key. | |

**User's choice:** Hybrid records + manifest — "but we should note tombstoning for deleting spaces or rooms or chats"
**Notes:** Tombstone-based deletion recorded as D-03: deletion is always a tombstone write on the record (GcsGlobalDb has no Delete; removal would not converge under per-key LWW). Readers treat tombstoned records as absent; applies uniformly to spaces, rooms, and message records. Load-bearing verified fact: GlobalDB is per-key last-writer-wins, priority = head_height + 1, no union semantics (crdt_datastore.cpp).

---

## autoJoinRooms semantics

| Option | Description | Selected |
|--------|-------------|----------|
| Derived join state | `joined = parentSpace && autoJoinRooms`, recomputed locally from synced CRDT state. Retroactive by construction; toggle = one config write; convergent with zero new op types. | ✓ |
| Explicit join ops | Toggle emits compensating Join/Leave ops per room per member. Auditable history, but event storms, ordering races, and divergence risk — all overhead in Phase 2. | |

**User's choice:** Derived join state
**Notes:** Matches the architecture note's contract verbatim ("clients process changes and update local join behavior" = local recomputation). Phase 2 observable: creator's joined-room set in the rail changes live on toggle. Tombstoned rooms drop out of the derived set with no special case.

---

## Creation UI flows

| Option | Description | Selected |
|--------|-------------|----------|
| Rail + dialog | "+" affordance on rail header and space nodes; one reusable create/edit dialog from existing atoms (ResponsiveDrawer, text field, radios, toggle, toasts); rail renders the pushed tree. | ✓ |
| Full-screen views | Dedicated creation screens with routes. Room to grow, but overbuilt for 2-3 field forms and fights the locked rail-centric shell. | |

**User's choice:** Rail + dialog
**Notes:** All atoms verified present in `src/app/scaffold/lib/components/`. Same dialog serves create AND edit (CORE-03's toggle-later criterion requires an edit surface). Dialog publishes a GcsCommand and closes on confirm/toasts on error — no local optimism.

---

## Claude's Discretion

- Opaque id format (seed+counter vs UUID); proto message/field naming/numbering; manifest key name; tombstone field shape (bool vs timestamp); startup replay ordering; "+" affordance interaction details; new Dart file layout under `src/app/lib/shell/`.

## Deferred Ideas

- Explicit-leave precedence + retoggle-resurrect semantics — Phase 4.
- Per-room join audit history — Phase 4 only if a hard requirement emerges.
- Manifest lost-update healing under concurrent multi-node creation — accepted v1 risk.
- Lobby publish for public spaces; invite links — Phase 4+.
- Message tombstone UI (delete/edit) — Phase 3 (principle lands now).
- `autoAnswer` per-participant policy — Phase 5+.
