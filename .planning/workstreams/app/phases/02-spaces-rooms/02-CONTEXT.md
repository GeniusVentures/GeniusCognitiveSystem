# Phase 2: Spaces & Rooms - Context

**Gathered:** 2026-09-18
**Status:** Ready for planning

<domain>
## Phase Boundary

Users can create spaces (public/private) and rooms (within a space or standalone), configure a space's `autoJoinRooms`, and see the hierarchy in the app's left rail. The entity hierarchy is persisted as CRDT records in GlobalDB and survives app restart. This phase replaces Phase 1's in-memory room list (`g_roomTopics` vector, flat `RoomList` event, dies on restart) with the CRDT-backed entity catalog. No messaging features (Phase 3), no membership/invites/roles (Phase 4), no lobby publish for public spaces (later discovery phase), no encryption (deferred v1.1).

</domain>

<decisions>
## Implementation Decisions

### Entity Identity & Topic Namespacing
- **D-01:** **Opaque entity IDs; name is a display attribute.** Pub/sub topics and CRDT identities carry immutable opaque ids (`gcs/chat/<id>` for rooms, space equivalent for spaces); the human name lives on the entity record as mutable display metadata. **C++ mints the id at creation** — Dart commands stay data-only per Phase 1 D-27 (create command carries `{name, parent, config}`, never an id). A standalone room is the same record shape with an empty parent (locked unified room model). Renames and reparenting are single metadata Puts — never namespace changes. Supersedes the architecture note's bare-`<room_name>` topic sketch (its own invite URLs `gcs://invite/room/<id>?key=<key>` and auto-generated DM ids already presupposed an id distinct from the name). Rejected: space-scoped topics `gcs/chat/<space>/<room>` (mutable names embedded in the CRDT namespace — rename/move splits history; collision problem recurs at the space layer) and globally-unique room names (a registry-free gossip network cannot enforce them).

### CRDT Entity Encoding & Hierarchy Events
- **D-02:** **Hybrid: per-entity records + ID manifest + new `SpaceTree` event.** KV layout: `gcs/entities/spaces/<id>` and `gcs/entities/rooms/<id>` each hold one serialized entity-state proto (id, name, parent space, isPublic, autoJoinRooms, timestamps); a known manifest key (`gcs/index/manifest`) lists known entity ids. Startup: Get manifest → Get each entity (N+1 reads) → C++ rebuilds the tree after CRDT replay → pushes a new `SpaceTree` message added to the `GcsEvent` oneof (append-only safe per D-24/D-26). Manifest writes follow a read-union-write convention (append-only membership). **`RoomList` is NOT restructured** — it remains the joined-session view; the persistent catalog (`SpaceTree`) and session membership (`RoomList`) stay separate lifecycles. Grounding (verified in SuperGenius source): GlobalDB resolves every key per-key last-writer-wins with no union semantics (published Put deltas stamped `priority = head_height + 1`; merge keeps higher priority — `../SuperGenius/src/crdt/impl/crdt_datastore.cpp`). Per-entity keys confine LWW to single-entity edits where it is exactly correct; the space-blob alternative (rooms embedded in the space record) has a structural permanent-lost-room path under concurrent creation and was rejected.
- **D-03:** **Tombstone-based deletion — general CRDT design principle from day one.** Deleting a space, room, or message is a tombstone write on the record (deleted flag/timestamp), never a physical key removal — `GcsGlobalDb` has no Delete API, and removal would not converge under per-key LWW anyway. Readers treat tombstoned records as absent; the manifest may retain tombstoned ids (readers skip them). Entity-record protos carry the tombstone field at creation (greenfield now; retrofitting later would be an append-only-compatible but avoidable migration). Tombstoned rooms drop out of the derived join set (D-04) with no special case. *(User note, 2026-09-18: "we should note tombstoning for deleting spaces or rooms or chats.")*

### autoJoinRooms Semantics
- **D-04:** **Derived join state.** `joined(user, room) = room.parentSpace && space.autoJoinRooms` — a pure local recomputation over synced CRDT state (entity records + space config), never emitted ops. Retroactive by construction: existing rooms are re-evaluated on toggle, and new rooms in an autoJoin space join at creation with no creation-time hook. Toggling is a single config CRDT write. Phase 2 observable behavior (no membership until Phase 4): the creator auto-joins every room in their autoJoin space — toggle true→false and the derived joins evaporate from the rail (rooms remain visible as space children); false→true joins them. Success criterion 3 is exactly this. Phase 4 extends additively (`autoJoin && !explicitLeave`) and must define explicit-leave precedence plus the expected retoggle-resurrect semantics (a leave can resurrect if config toggles false→true→false→true — document as intended). Rejected: explicit per-member join ops (event storm N×M per toggle, convergence depends on every client reacting correctly, pure overhead pre-membership).

### Creation UI Flows
- **D-05:** **Rail-affordance + one reusable create/edit dialog.** A "+" affordance in the rail header (new space / new standalone room) and on each space node (new room in that space) opens a dialog composed entirely from existing scaffold atoms: `ResponsiveDrawer.show` (dialog on wide screens, bottom sheet on narrow), `TextEntryFieldWidget` for the name, `ScaffoldSelectionIndicatorRadio` pair for public/private, `ScaffoldSelectionIndicatorToggle` for autoJoinRooms, `showToast` for feedback/errors. **The same dialog serves create and edit** — CORE-03's "toggle autoJoinRooms and observe" requires an edit surface, not creation-only controls. The rail grows two pushed sections — Spaces (expandable nodes, `ScaffoldBadge` private badge) and standalone Rooms — with `RailState` evolving from flat list to tree. The dialog is a dumb form: it publishes a `GcsCommand` through the existing `GcsCommandTransport` seam and closes on confirmation or toasts on failure; creation is only "done" when C++ pushes the updated `SpaceTree` (no local optimism — C++ remains the sole source of truth per D-04 Phase 1). Rejected: full-screen creation routes (overbuilt for 2-3 field forms; fights the locked rail-centric shell D-21; duplicate edit surface still required).

### Claude's Discretion
- Opaque id format specifics (seed+counter string per the existing `NextMessageId()` idiom vs UUID) — planner/researcher design.
- Proto message/field naming and numbering within the append-only discipline (`SpaceRecord`/`RoomRecord`/`SpaceTree` shapes).
- Exact manifest key name and its serialized representation.
- Tombstone field shape (bool vs timestamp vs both).
- Startup replay ordering details (manifest read → entity Gets → tree push sequence).
- Space-node "+" affordance interaction details (hover/long-press/inline) within scaffold capabilities.
- File layout of the new Dart files under `src/app/lib/shell/`.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Architecture
- `.planning/notes/gcs-chat-architecture.md` — entity/topic model, Space Configuration (autoJoinRooms contract: "clients process changes and update local join behavior" — realized as D-04 derived state), Room Types, Core user flows. NOTE: its bare-`<room_name>` topic format is superseded by D-01 (opaque ids); container/autoJoin/unified-room semantics are retained as locked.

### Locked Phase 1 constraints this phase inherits
- `.planning/workstreams/app/phases/01-foundation/01-CONTEXT.md` — D-04 (C++ owns state), D-21 (left-rail space→room shell), D-24/D-26 (append-only proto discipline), D-27/D-29 (topic pub/sub FFI, data-only Dart commands, protobuf envelopes, codec-tagged bytes, raw error strings).

### Verified CRDT semantics (load-bearing for D-02/D-03)
- `../SuperGenius/src/crdt/impl/crdt_datastore.cpp` — per-key last-writer-wins with priority = head_height + 1 stamping (~lines 1546-1556; PutKey path ~line 1375). No union/set semantics: concurrent writes to a shared key silently drop the loser's entire value.

### Wire contract and existing session code
- `src/proto/gcs_chat.proto` — the append-only contract being extended: `GcsCommand`/`GcsEvent` oneofs, flat `RoomList`, `JoinTopicCommand`, `ChatMessageState`.
- `src/ffi/gcs_core_ffi.cpp` — `g_roomTopics` in-memory session view being replaced by the CRDT catalog; `NextMessageId()` seed+counter id idiom (line ~149); per-record message key precedent `room_topic + "/" + message-id` (line ~355).
- `src/lib/gcs_storage/gcs_global_db.hpp` — the entire storage surface Phase 2 builds on: string `Put`/`Get` only (no Delete, no enumeration, no prefix queries), `AddBroadcastTopic`/`AddListenTopic`, `Initialize`/`Shutdown`.

### Flutter consumption surface
- `src/app/lib/cubits/rail_cubit.dart` + `src/app/lib/cubits/session_cubit.dart` — `RailCubit.setRooms` full-replacement rebuild pattern; `GcsCommandTransport.publishCommand` seam.
- `src/app/scaffold/CLAUDE.md` — scaffold submodule contract (lib/ read-only, never edit generated families).
- Verified-available atoms for D-05: `src/app/scaffold/lib/components/` — `ResponsiveDrawer` (`bottom_drawer/responsive_drawer.dart`), `TextEntryFieldWidget`/`TextFormFieldLogic`, `ScaffoldSelectionIndicatorRadio`, `ScaffoldSelectionIndicatorToggle`, `ScaffoldBadge`, `ScaffoldStateView`, global `showToast` (`toast_manager.dart`).

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `GcsCommandTransport.publishCommand` (implemented by `SessionCubit`) — the seam D-05's dialog publishes create/update commands through; zero new transport work.
- `RailCubit.setRooms` — full-replacement rebuild pattern; the tree version reuses the same shape (`RailState` list → tree).
- `NextMessageId()` idiom (`gcs_core_ffi.cpp`) — C++-side opaque id minting for D-01.
- Per-record message keys (`room_topic + "/" + msg-id`) — the Phase 1 precedent D-02 generalizes to entity records.
- `GcsGlobalDb::Put/Get/AddListenTopic/AddBroadcastTopic` — the complete storage API; everything in D-02/D-03 composes from just these.

### Established Patterns
- Append-only proto evolution (only add fields/messages; never retype/remove/reorder).
- C++ owns state, Dart Cubits are thin subscribers rendering pushed events.
- Missing required libraries fail at CMake configure time (FATAL_ERROR), never stubs.
- Tests use wait-condition templates (no `sleep_for`); C++17 ceiling; no OS `#ifdef` in source.

### Integration Points
- `GcsCommand` oneof gains create/update commands (space + room variants); `GcsEvent` oneof gains `SpaceTree`.
- FFI session init: after CRDT replay, read manifest + entity records, build tree, push `SpaceTree` before/with `Readiness`.
- `RailState` flat → tree; rail renders Spaces section + standalone Rooms section.
- `RoomList` push path unchanged — remains the joined-session view fed by the derived-join evaluator (D-04).

</code_context>

<specifics>
## Specific Ideas

- "we should note tombstoning for deleting spaces or rooms or chats" (2026-09-18) — D-03: deletion is always a tombstone write on the record, never key removal; applies uniformly to spaces, rooms, and message records.
- Advisor research (4 parallel gsd-advisor-researcher runs, 2026-09-18) verified in-repo: GlobalDB LWW-per-key semantics read directly from `crdt_datastore.cpp`; all D-05 scaffold atoms confirmed present in `src/app/scaffold/lib/components/`; the id-vs-alias split matches Matrix/Discord/Slack precedent for decentralized chat.

</specifics>

<deferred>
## Deferred Ideas

- Explicit-leave precedence and retoggle-resurrect semantics for derived joins — Phase 4 (membership); one Boolean rule (`autoJoin && !explicitLeave`).
- Per-room join/leave audit history (the rejected explicit-ops model) — revisit only if Phase 4 introduces a hard audit requirement.
- Manifest lost-update healing under concurrent multi-node creation — accepted risk for v1 (entity records persist independently; read-union-write heals on next write).
- Lobby publish for public spaces (`gcs/spaces/lobby` topic) and private-space invite links (`gcs://invite/...` capability tokens) — discovery/membership phases (Phase 4+).
- Message-record tombstone UI (delete/edit messages) — Phase 3 Messaging; the design principle lands now via D-03.
- `autoAnswer` per-participant bot policy — Phase 5+ (GCS bot).

</deferred>

---

*Phase: 2-Spaces & Rooms*
*Context gathered: 2026-09-18*
