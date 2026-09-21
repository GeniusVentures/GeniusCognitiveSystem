# Phase 2: Spaces & Rooms - Research

**Researched:** 2026-09-19
**Domain:** CRDT-backed entity catalog (spaces/rooms) over GcsGlobalDb string KV + append-only protobuf contract extension + Flutter rail tree UI
**Confidence:** HIGH (every load-bearing claim verified by reading this repo's source this session)

## User Constraints (from CONTEXT.md)

### Locked Decisions

- **D-01: Opaque entity IDs; name is a display attribute.** Pub/sub topics and CRDT identities carry immutable opaque ids (`gcs/chat/<id>` for rooms, space equivalent for spaces); the human name lives on the entity record as mutable display metadata. **C++ mints the id at creation** — Dart commands stay data-only per Phase 1 D-27 (create command carries `{name, parent, config}`, never an id). A standalone room is the same record shape with an empty parent (locked unified room model). Renames and reparenting are single metadata Puts — never namespace changes. Supersedes the architecture note's bare-`<room_name>` topic sketch. Rejected: space-scoped topics and globally-unique room names.
- **D-02: Hybrid: per-entity records + ID manifest + new `SpaceTree` event.** KV layout: `gcs/entities/spaces/<id>` and `gcs/entities/rooms/<id>` each hold one serialized entity-state proto; a known manifest key (`gcs/index/manifest`) lists known entity ids. Startup: Get manifest → Get each entity (N+1 reads) → C++ rebuilds the tree after CRDT replay → pushes a new `SpaceTree` message added to the `GcsEvent` oneof (append-only safe). Manifest writes follow a read-union-write convention. **`RoomList` is NOT restructured** — it remains the joined-session view; persistent catalog (`SpaceTree`) and session membership (`RoomList`) stay separate lifecycles. Grounding verified in `../SuperGenius/src/crdt/impl/crdt_datastore.cpp`: per-key LWW, `priority = head_height + 1`, no union semantics. Space-blob alternative rejected (permanent lost-room path under concurrent creation).
- **D-03: Tombstone-based deletion from day one.** Deleting is a tombstone write on the record (deleted flag/timestamp), never physical key removal — `GcsGlobalDb` has no Delete API and removal would not converge under per-key LWW. Readers treat tombstoned records as absent; the manifest may retain tombstoned ids (readers skip them). Entity-record protos carry the tombstone field at creation. Tombstoned rooms drop out of the derived join set (D-04) with no special case.
- **D-04: Derived join state.** `joined(user, room) = room.parentSpace && space.autoJoinRooms` — pure local recomputation over synced CRDT state, never emitted ops. Retroactive by construction. Toggling is a single config CRDT write. Phase 2 observable: creator auto-joins every room in their autoJoin space; toggle true→false and derived joins evaporate from the rail (rooms remain visible as space children); false→true joins them. Success criterion 3 is exactly this. Phase 4 extends additively (`autoJoin && !explicitLeave`). Rejected: explicit per-member join ops.
- **D-05: Rail-affordance + one reusable create/edit dialog.** "+" affordance in rail header (new space / standalone room) and on each space node (new room in that space) opens a dialog composed entirely from existing scaffold atoms: `ResponsiveDrawer.show`, `TextEntryFieldWidget`, `ScaffoldSelectionIndicatorRadio` pair (public/private), `ScaffoldSelectionIndicatorToggle` (autoJoinRooms), `showToast` for feedback/errors. **Same dialog serves create and edit.** Rail grows two sections — Spaces (expandable, `ScaffoldBadge` private badge) and standalone Rooms — with `RailState` evolving from flat list to tree. Dialog is a dumb form: publishes a `GcsCommand` through `GcsCommandTransport` and closes on confirmation or toasts on failure; creation only "done" when C++ pushes the updated `SpaceTree` (no local optimism). Rejected: full-screen creation routes.

### Claude's Discretion

- Opaque id format specifics (seed+counter string per `NextMessageId()` idiom vs UUID).
- Proto message/field naming and numbering within append-only discipline (`SpaceRecord`/`RoomRecord`/`SpaceTree` shapes).
- Exact manifest key name and serialized representation.
- Tombstone field shape (bool vs timestamp vs both).
- Startup replay ordering details (manifest read → entity Gets → tree push sequence).
- Space-node "+" affordance interaction details within scaffold capabilities.
- File layout of new Dart files under `src/app/lib/shell/`.

### Deferred Ideas (OUT OF SCOPE)

- Explicit-leave precedence and retoggle-resurrect semantics — Phase 4.
- Per-room join/leave audit history — revisit only with a hard Phase 4 audit requirement.
- Manifest lost-update healing under concurrent multi-node creation — accepted v1 risk.
- Lobby publish for public spaces and private-space invite links — Phase 4+.
- Message-record tombstone UI — Phase 3 (design principle lands now via D-03).
- `autoAnswer` per-participant bot policy — Phase 5+.

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| CORE-01 | User can create a space (public or private) | `CreateSpaceCommand` proto arm + C++ id minting (NextEntityId, section below) + EntityStore record/manifest Put + SpaceTree push + rail Spaces section + create dialog (D-05 atoms verified present) |
| CORE-02 | User can create a room within a space or standalone | `CreateRoomCommand` with empty `parent_space_id` = standalone (unified record shape per D-01); parent-existence validation; `gcs/chat/<id>` topic derivation; rail tree rendering |
| CORE-03 | User can configure space `autoJoinRooms` setting | `UpdateSpaceCommand` (edit surface via same dialog per D-05) + derived-join evaluator recomputing RoomList over entity records (D-04); test strategy covers true→false→true |
</phase_requirements>

## Summary

Phase 2 replaces the in-memory `g_roomTopics` session view's catalog role with a CRDT-persisted entity store, extends the append-only `gcs_chat.proto` with entity records/commands/`SpaceTree`, and grows the Dart rail from a flat topic list to a spaces/rooms tree driven by pushed catalog events. Every storage operation composes from the four existing `GcsGlobalDb` calls (`Put`/`Get`/`AddListenTopic`/`AddBroadcastTopic`) — the storage component itself needs **zero surface changes**. The C++ side gains one new unit-testable class (entity store: manifest + records + tree + derived-join evaluator) plus new command arms in the FFI dispatch; the Dart side gains the `SpaceTree` dispatch arm, a tree-shaped `RailState`, and one dialog widget.

Three verified storage-layer facts shape the design. First, `GcsGlobalDb::Put` writes with `kNoTopics` — **local-only, no gossip** (src/lib/gcs_storage/gcs_global_db.cpp:260) — which is exactly right for Phase 2 (restart persistence is the only success criterion involving sync), and entity records must NOT be assumed to propagate cross-node until a topics-bearing Put exists (Phase 4+). Second, `GcsGlobalDb::Get` collapses every underlying error into `Error::GcsDbError` — a missing manifest key on first launch is indistinguishable from DB failure, so startup replay must treat Get failure as "empty catalog" (the store is already running at that point, so failure ≈ absent). Third, there is **no RemoveListenTopic/RemoveBroadcastTopic** — topic registrations are sticky, so a false autoJoin toggle removes rooms from the `RoomList` projection only, never from pubsub; this is harmless pre-messaging and must be documented, not "fixed."

**Primary recommendation:** Extend the proto append-only (5 new messages + 3 command arms + 1 event arm, next oneof tags 3–5 / 5), implement a new `gcs::EntityStore` class in `src/lib/` (files added to the existing `gcs_core` target) owning manifest/records/tree/derived-joins, wire create/update commands into `gcs_publish`'s dispatch under the existing `g_mutex`, push `SpaceTree` from `gcs_subscribe` before `RoomList`/`Readiness`, regenerate the Dart pb files with the **pinned** protoc_plugin 22.5.0, and grow `RailState` to a tree with a full-replacement `setTree` mirroring `setRooms`.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Entity id minting | C++ core (FFI layer) | — | D-04 Phase 1: C++ owns all authority fields; Dart commands data-only (D-27) |
| Entity persistence (records + manifest) | C++ core (EntityStore over GcsGlobalDb) | — | CRDT LWW per-key semantics verified; string Put/Get is the entire surface |
| Tree building + derived-join evaluation | C++ core (EntityStore) | — | C++ owns state; Dart is thin subscriber (Phase 1 D-04) |
| SpaceTree event push | C++ FFI (PostToDart) | — | Existing push port; append-only oneof arm |
| Rail tree state | Dart cubit (RailCubit) | — | Existing thin-subscriber pattern; full-replacement setRooms generalized to setTree |
| Create/edit dialog UI | Dart shell (new file under src/app/lib/shell/) | — | D-05 locked; dumb form publishing through GcsCommandTransport |
| Join state projection (RoomList) | C++ FFI (evaluator output) | Dart RailCubit (renders) | Separate lifecycle from catalog per D-02 |
| Proto contract | src/proto (single source) | Dart regen (committed) | D-24 single-source; C++ auto-regenerates via add_proto_library, Dart manual pinned step |

## Standard Stack

No new packages. Phase 2 composes entirely from the verified existing stack.

### Core
| Library / Component | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| protobuf (C++, via `add_proto_library` → thirdparty protoc) | libprotoc 34.0 (thirdparty/build/OSX/Debug/protobuf/bin/protoc) | Wire contract extension; C++ halves regenerate automatically on rebuild | Locked D-26/D-29; already wired |
| protobuf (Dart) + protoc_plugin | protobuf ^4.0.0 (resolved 4.2.0) + **protoc_plugin pinned 22.5.0** | Dart pb regeneration (manual, committed) | Phase 1 discovered pin: latest plugin (25.x) emits `BuilderInfo.aE` calls absent from protobuf 4.2.0 — generated Dart fails to compile |
| flutter_bloc | ^9.0.0 | RailState tree + dialog wiring | Existing cubit pattern |
| GTest (test tree) | thirdparty, `gcs_test` macro | Entity-store unit tests + two-session persistence | Wait-condition template at test/test_wait_condition.hpp |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| scaffold atoms (frontend_scaffold) | submodule | Dialog + rail composition (D-05) | `ResponsiveDrawer.show`, `TextEntryFieldWidget` + `TextFormFieldLogic`, `ScaffoldSelectionIndicatorRadio`/`Toggle`, `ScaffoldBadge`, `showToast`, `ScaffoldPressable`, `ScaffoldStateView` — all verified present with compatible APIs |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Seed+counter opaque ids (NextMessageId idiom) | UUIDv4 | UUID adds a dependency/impl for no benefit; seed idiom already has a regression test (CR-01 salt check) and matches house style |
| Manifest proto at single key | Per-entity keys only + prefix enumeration | Rejected by API reality: GcsGlobalDb has NO enumeration/prefix query — the manifest exists precisely because enumeration is unavailable |
| Nested SpaceTree (tree in proto) | Flat records in SpaceTree | Flat reuses the same record protos as KV values (one type, no drift); Dart builds the tree — recommended |

**Installation:** none.

**Version verification:** performed this session on this machine — dart 3.11.5, flutter 3.41.9, ninja 1.13.2, cmake 3.29.2, thirdparty protoc libprotoc 34.0, globally activated protoc-gen-dart = **22.5.0** (correct pin, `~/.pub-cache/bin/protoc-gen-dart` header confirms). [VERIFIED: local toolchain]

## Package Legitimacy Audit

This phase installs **zero external packages** — all work extends in-repo code against the existing locked stack. No registry installs → slopcheck gate not applicable.

| Package | Registry | Age | Downloads | Source Repo | slopcheck | Disposition |
|---------|----------|-----|-----------|-------------|-----------|-------------|
| *(none — no new packages)* | — | — | — | — | — | N/A |

**Packages removed due to slopcheck [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

## Architecture Patterns

### System Architecture Diagram

```
Dart (Flutter shell)                      C++ (libgcs_ffi.dylib)                 CRDT store (GcsGlobalDb)
─────────────────────                     ──────────────────────                 ────────────────────────
Create/Edit dialog (D-05)
  │ publishCommand(GcsCommand)            gcs_publish (under g_mutex)
  │   oneof: create_space ──────────────►   ├─ validate (name, parent exists)
  │        create_room                      ├─ EntityStore.Create*:
  │        update_space                     │    mint opaque id (seed+counter)
  │                                         │    Put gcs/entities/{spaces|rooms}/<id> ──────► LWW per-key write (local-only)
  │                                         │    read-union-write manifest
  │                                         │    Put gcs/index/manifest ─────────────────► LWW per-key write
  │                                         │    recompute derived joins
  │                                         └─ push SpaceTree + RoomList ──┐
  │                                                                            │ (push port)
RoomRail (tree: Spaces + standalone Rooms)                                    ▼
  ▲      SessionCubit._dispatchEvent: hasSpaceTree → RailCubit.setTree
  │                                                                gcs_subscribe (startup)
  │                                            gcs_init: EntityStore.LoadFromStore()
  │                                              Get gcs/index/manifest ◄─────────── read (absent = empty)
  │                                              Get gcs/entities/.../<id> (N+1) ◄── read
  │                                            rebuild tree + derived joins, then push:
  ◄──────────────────────────────────────────  SpaceTree → RoomList → Readiness
  │
  └─ RailState tree (full replacement per push; activeRoom still a topic string)
       RoomList (joined view: smoke ∪ explicit ∪ derived) — NOT restructured (D-02)
```

### Recommended Project Structure

```
src/
├── proto/gcs_chat.proto              # EXTEND append-only: SpaceRecord, RoomRecord,
│                                     #   EntityManifest, Create*/UpdateSpace commands, SpaceTree
├── lib/
│   ├── gcs_entity_store.hpp/.cpp     # NEW — manifest + records + tree + derived-join evaluator
│   │                                 #   (add .cpp to gcs_core target in src/CMakeLists.txt — one line)
│   └── gcs_core.hpp/.cpp             # UNCHANGED (pass-throughs already sufficient)
├── ffi/gcs_core_ffi.cpp              # EXTEND: entity-store global, 3 new command arms,
│                                     #   SpaceTree push in gcs_subscribe + after mutations
└── app/
    ├── lib/generated/proto/          # REGEN (manual pinned protoc step; commit output)
    ├── lib/cubits/
    │   ├── rail_cubit.dart           # EXTEND: RailState tree (spaces + standaloneRooms), setTree
    │   └── session_cubit.dart        # EXTEND: hasSpaceTree dispatch arm
    ├── lib/shell/
    │   ├── room_rail.dart            # EXTEND: two sections, badges, "+" affordances, edit entry
    │   └── space_room_dialog.dart    # NEW — create/edit dumb form (D-05)
    └── test/cubits/ + test/          # EXTEND: rail tree tests, dispatch tests, dialog widget test
test/
├── test_gcs_entities.cpp             # NEW — EntityStore unit tests (injected-pubsub CoreSession seam)
└── (extend or new) FFI two-session persistence test (test_gcs_ffi_sdk.cpp pattern)
```

### Pattern 1: Append-only proto extension (exact shapes)

**What:** Add messages + oneof arms only; never retype, remove, or renumber. Current oneof tags: `GcsCommand` has `join_topic=1, send_text=2` (next: 3,4,5); `GcsEvent` has `message=1, room_list=2, readiness=3, error=4` (next: 5).

**When to use:** This is the only legal way this contract evolves (D-24/D-26).

**Example** (recommended shapes — naming/numbering is Claude's discretion per CONTEXT, this is a concrete sound proposal):

```protobuf
// Source: derived from src/proto/gcs_chat.proto current state (read this session)

// === Phase 2: Spaces & Rooms (append-only additions) ===

// Persistent space entity record (D-01/D-02). Stored as the value bytes under
// gcs/entities/spaces/<id>. Name is mutable display metadata; id is identity.
message SpaceRecord {
  string id = 1;             // opaque C++-minted id (D-01)
  string name = 2;
  bool is_public = 3;
  bool auto_join_rooms = 4;  // D-04 derived-join source of truth
  int64 created_at_ms = 5;   // wall-clock ms (same stamp as ChatMessageState)
  int64 updated_at_ms = 6;
  bool deleted = 7;          // D-03 tombstone (present from creation)
  int64 deleted_at_ms = 8;   // 0 = alive
}

// Persistent room entity record. Standalone room = empty parent_space_id
// (locked unified room model, D-01). Topic is derived: gcs/chat/<id>.
message RoomRecord {
  string id = 1;
  string name = 2;
  string parent_space_id = 3; // empty = standalone
  int64 created_at_ms = 4;
  int64 updated_at_ms = 5;
  bool deleted = 6;
  int64 deleted_at_ms = 7;
}

// Manifest of known entity ids (D-02). Value bytes under gcs/index/manifest.
// Exists because GcsGlobalDb has no enumeration/prefix query.
message EntityManifest {
  repeated string space_id = 1;
  repeated string room_id = 2;
}

// Data-only create commands (D-27; C++ mints id/timestamps).
message CreateSpaceCommand {
  string name = 1;
  bool is_public = 2;
  bool auto_join_rooms = 3;
}

message CreateRoomCommand {
  string name = 1;
  string parent_space_id = 2; // empty = standalone
}

// Edit surface (D-05: same dialog serves create and edit; CORE-03 requires it).
// Full desired state, not a patch: per-key LWW replaces the whole value, so
// partial patches buy nothing (verified crdt_datastore.cpp semantics).
message UpdateSpaceCommand {
  string space_id = 1;
  string name = 2;
  bool is_public = 3;
  bool auto_join_rooms = 4;
}

// Pushed entity-catalog event (D-02): FLAT records — Dart builds the tree.
// Reuses the same record protos stored in the KV values (one type, no drift).
message SpaceTree {
  repeated SpaceRecord space = 1;
  repeated RoomRecord room = 2;
}

// GcsCommand oneof — append:
//   CreateSpaceCommand create_space = 3;
//   CreateRoomCommand  create_room  = 4;
//   UpdateSpaceCommand update_space = 5;
// GcsEvent oneof — append:
//   SpaceTree space_tree = 5;
```

Tombstone shape discretion resolved as bool + timestamp: `deleted` is the fast reader check; `deleted_at_ms` is free ordering information for later phases. Neither is set by any Phase 2 command (field lands now per D-03; reader-skip behavior is unit-testable by writing a tombstoned record directly).

### Pattern 2: EntityStore component (where it lives, what it owns)

**What:** One new C++ class, `gcs::EntityStore` (suggested), header + impl flat under `src/lib/` beside `gcs_core.*`, its .cpp added to the `gcs_core` target (one-line change to src/CMakeLists.txt — no new target, no new hard-required-library block).

**When to use:** All catalog logic (manifest, records, tree build, derived joins) — the FFI layer stays a thin dispatch, matching how `GcsGlobalDb` keeps `CoreSession` thin.

**Example:**

```cpp
// Source: pattern sketch grounded in gcs_core.hpp (CoreSession pass-throughs) —
// the planner finalizes signatures.
namespace gcs {

class EntityStore {
public:
  explicit EntityStore(CoreSession& session);  // operates over Put/Get pass-throughs

  // Startup replay (discretion item: exact ordering). Get(manifest) failure
  // MUST be treated as "empty catalog" (GcsGlobalDb::Get conflates missing
  // key with DB error — verified gcs_global_db.cpp:267-280).
  outcome::result<void> LoadFromStore();

  outcome::result<SpaceRecord> CreateSpace(std::string name, bool isPublic, bool autoJoinRooms);
  outcome::result<RoomRecord> CreateRoom(std::string name, std::string parentSpaceId);
  outcome::result<SpaceRecord> UpdateSpace(SpaceRecord desired); // full-state rewrite

  // Pure recomputation over in-memory state (D-04) — never emits ops.
  std::vector<SpaceRecord> Spaces() const;              // tombstoned skipped
  std::vector<RoomRecord> Rooms() const;                // tombstoned skipped
  std::vector<std::string> DerivedJoinedTopics() const; // rooms whose parent
                                                        // space has autoJoinRooms
  bool IsValidParentSpace(const std::string& id) const; // exists && !deleted
private:
  CoreSession& m_session;
  std::map<std::string, SpaceRecord> m_spaces;
  std::map<std::string, RoomRecord> m_rooms;
  // ids minted via the NextMessageId seed idiom (CR-01-safe)
};

} // namespace gcs
```

**FFI integration:** `gcs_core_ffi.cpp` gains `std::unique_ptr<gcs::EntityStore> g_entities` beside `g_session` (guarded by `g_mutex`, reset in `gcs_shutdown`). Constructed + `LoadFromStore()` in `gcs_init` after smoke-topic join; all three new command arms in `gcs_publish`'s switch mutate it under the existing lock; `BuildSpaceTreeEvent()` mirrors `BuildRoomListEvent()`.

**Why standalone class over folding into the FFI file or CoreSession:** the FFI anonymous namespace is untestable except through the ABI; a plain class over `CoreSession&` is unit-testable with the injected-pubsub seam (test_gcs_core_smoke pattern) with no node boot. CoreSession stays a pass-through per its own header's staging note ("Message logic, rooms, and spaces land in Phases 2-3").

### Pattern 3: Id minting (opaque entity ids)

Generalize `NextMessageId()` (gcs_core_ffi.cpp:149-158): per-process seed = wall-clock ms + `std::random_device` token, then counter. Distinct prefixes make ids self-describing in keys and logs and satisfy the CR-01 salt test shape:

```cpp
// Source: NextMessageId idiom, src/ffi/gcs_core_ffi.cpp:149-158 (read this session)
std::string NextEntityId(const char* prefix)  // "space-", "room-"
{
    static const std::string seed = [] { /* wallclock-ms + random_device, as CR-01 */ }();
    return std::string(prefix) + seed + "-" + std::to_string(g_entitySeq.fetch_add(1));
}
```

Room topic derivation: `topic = "gcs/chat/" + room.id` — matches the Phase 1 message-key precedent (`room_topic + "/" + message-id` → keys like `gcs/chat/<id>/msg-...`, gcs_core_ffi.cpp:355). No space topic is needed for any Phase 2 behavior (no lobby, no membership); store the id only.

### Pattern 4: Derived joins and the RoomList projection

`RoomList` stays a flat topic list (D-02). Its contents become: smoke topics ∪ explicit `join_topic` topics ∪ derived joined topics. Derived set = every non-tombstoned room whose parent space exists, is non-tombstoned, and has `autoJoinRooms == true`. Recompute (cheap: iterate rooms, map-lookup parent) after every entity mutation and at LoadFromStore; push `RoomList` whenever the set changes. For each derived-joined room, call `AddListenTopic` + `AddBroadcastTopic` (listen first, broadcast second — the D-07 ordering used throughout). **Toggling false removes topics from the pushed RoomList only** — there is no un-join API (verified: GcsGlobalDb surface has no Remove*Topic), so pubsub registrations persist; harmless pre-messaging, revisit in Phase 3/4.

`send_text`'s joined-room check (gcs_core_ffi.cpp:331) keeps working unchanged because derived-joined rooms are in the same joined set.

### Pattern 5: Startup replay + push ordering (discretion item resolved)

- `gcs_init`: session Initialize → smoke topics → `EntityStore::LoadFromStore()` (manifest Get; failure ⇒ empty catalog + `spdlog::debug`; then N+1 record Gets, each failure ⇒ skip that id + debug log). No pushes here — the port is never registered yet at init (posts would be silent no-ops).
- `gcs_subscribe`: push `SpaceTree` → `RoomList` → `Readiness` (tree before membership before readiness — Dart can render the catalog while not ready; mirrors the existing RoomList-then-Readiness contract).
- After every create/update command: push `SpaceTree`, and additionally `RoomList` when the derived set changed (create_room under an autoJoin space, update_space toggling).

### Pattern 6: Dart rail tree (RailState evolution)

Full-replacement, mirroring `setRooms`:

```dart
// Source: pattern grounded in rail_cubit.dart setRooms (read this session)
class RailRoom { final String id, name; String get topic => 'gcs/chat/$id'; }
class RailSpace { final String id, name; final bool isPublic, autoJoinRooms; final List<RailRoom> rooms; }

class RailState {
  final List<RailSpace> spaces;        // catalog tree (SpaceTree pushes)
  final List<RailRoom> standaloneRooms; // rooms with empty parent
  final List<String> rooms;            // joined topics (RoomList pushes — KEPT)
  final String? activeRoom;            // still a topic string (composer/flow unchanged)
}
// RailCubit.setTree(List<RailSpace>, List<RailRoom>) — full replacement
// setRooms stays exactly as-is (joined view, clears dangling selection)
```

`SessionCubit._dispatchEvent` gains `if (event.hasSpaceTree()) { _railCubit?.setTree(...); return; }` before the existing arms. `RoomRail` renders: "Spaces" section (space rows with `ScaffoldBadge` private badge + per-row "+" and edit affordances + nested room rows) then "Rooms" section for standalone rooms; empty state stays `ScaffoldStateView`.

### Pattern 7: The create/edit dialog (D-05)

One stateless widget (e.g. `src/app/lib/shell/space_room_dialog.dart`) invoked via `ResponsiveDrawer.show(context, title: ..., children: [...], footer: confirmButton)`. Verified atom APIs this session:
- `TextEntryFieldWidget(logic: TextFormFieldLogic(controller: ..., hintText: ..., ...))` — owns its controller via logic.
- `ScaffoldSelectionIndicatorRadio(value: bool, onChanged: ValueChanged<bool>?)` — **bool-based, not group-value**: a public/private pair is two radios with coordinated booleans (onChanged(true) on one sets the other false in local dialog state).
- `ScaffoldSelectionIndicatorToggle(value:, onChanged:)` — autoJoinRooms (space modes only).
- `showToast(context, message, {title, type: ToastType.error, ...})` — failure feedback.
- Dialog holds local mutable form state in a `StatefulWidget`, publishes `GcsCommand()` (`createSpace` / `createRoom` / `updateSpace`) via an injected `GcsCommandTransport` + optional `showToast` on `publishCommand == false`, closes on success. No local optimism: the rail updates when `SpaceTree` arrives.
- Edit mode: same widget, prefilled fields + `space_id` carried; entry affordance on the space row (e.g. trailing edit icon within `ScaffoldPressable` — interaction detail is discretion).

### Anti-Patterns to Avoid

- **Renumbering/reusing proto tags or restructuring `RoomList`** — append-only discipline (D-24/D-26); RoomList's shape is locked by D-02.
- **Partial-field update semantics** — per-key LWW replaces the whole value; patches cannot merge. Update commands carry full desired state.
- **Physical deletion or manifest pruning on tombstone** — D-03: readers skip; manifest retains ids.
- **Deleting/overwriting `g_roomTopics` semantics silently** — the joined set gains a third source (derived); keep the union explicit.
- **Hand-rolling a second .proto for Dart** — D-24 single source of truth; regenerate from the same file.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|------|
| Unique ids | UUID library / custom algorithm | NextMessageId seed+counter idiom (generalized with prefixes) | Already regression-tested (CR-01 salt); no new dependency; collision-safe across sessions |
| Catalog enumeration | Prefix-scan of `gcs/entities/*` | Manifest key (D-02) | GcsGlobalDb has NO enumeration/prefix API (verified surface: Put/Get/Add*Topic only) |
| Incremental tree patches/diffing | Per-entity add/remove events | Full SpaceTree push + full-replacement RailState | Matches RailCubit.setRooms; tiny catalogs; zero reconciliation bugs |
| Conflict-free membership set | CRDT set/orphan semantics on one key | Per-entity keys + per-key LWW | Verified: crdt_datastore has no union semantics; per-entity keys confine LWW to exactly-correct scope |
| Dialog chrome (drawer/sheet, fields, toasts) | Custom modal + text fields | ResponsiveDrawer/TextEntryFieldWidget/showToast | D-05 locked; atoms verified present |

**Key insight:** every deceptively hard problem here (CRDT convergence, id uniqueness, catalog discovery) already has a verified in-repo answer; the phase's real work is composition and wiring, not invention.

## Common Pitfalls

### Pitfall 1: Dart proto regen with an unpinned protoc_plugin
**What goes wrong:** Generated `gcs_chat.pb.dart` references `BuilderInfo.aE`, absent from protobuf 4.2.0; Dart build fails.
**Why:** Latest protoc_plugin (25.x) targets newer protobuf runtimes.
**How to avoid:** Regenerate ONLY with protoc_plugin 22.5.0 (`dart pub global activate protoc_plugin 22.5.0`). Verified currently activated at 22.5.0 on this machine. Command (from 01-05-PLAN.md, verbatim workflow):
```bash
cd src/app && /Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/thirdparty/build/OSX/Debug/protobuf/bin/protoc \
  -I ../proto --dart_out=lib/generated/proto \
  --plugin=protoc-gen-dart=$HOME/.pub-cache/bin/protoc-gen-dart \
  ../proto/gcs_chat.proto
```
Commit the regenerated .pb.dart/.pbenum/.pbjson (tracked by design). C++ halves regenerate automatically via ninja (`add_proto_library`).
**Warning signs:** analyzer errors on generated files mentioning `aE`.

### Pitfall 2: Treating manifest Get failure as a fatal error
**What goes wrong:** First launch (no manifest key yet) fails or blocks readiness.
**Why:** `GcsGlobalDb::Get` maps EVERY underlying error — including missing-key from `CrdtSet::GetElement` (verified crdt_set.cpp:185-198) — to `Error::GcsDbError`. Missing vs broken is indistinguishable at this surface.
**How to avoid:** `LoadFromStore` treats Get(manifest) failure as an empty catalog (`spdlog::debug` it). Same for per-entity Gets (skip + debug). The Error enum has no NotFound code — do NOT add one this phase (minimal change).
**Warning signs:** any `ASSERT` on manifest Get result in the startup path.

### Pitfall 3: Assuming entity writes sync across nodes
**What goes wrong:** Expectation that a second peer sees created spaces.
**Why:** `GcsGlobalDb::Put` hard-codes `kNoTopics` — "plain local store, not a broadcast write" (verified gcs_global_db.cpp:255-265). Records persist locally (restart criterion passes) but never gossip.
**How to avoid:** State it in the plan: Phase 2 persistence is local-restart only; cross-node catalog sync requires a topics-bearing Put overload (Phase 4+, out of scope). Do not "fix" by passing topics through the current API — it takes none.
**Warning signs:** any plan step claiming multi-device visibility.

### Pitfall 4: Trying to un-join pubsub on autoJoin false
**What goes wrong:** Search for RemoveTopic API, or hacks around its absence.
**Why:** GcsGlobalDb exposes only Add{Listen,Broadcast}Topic. Registrations are sticky for the store's lifetime.
**How to avoid:** The false toggle changes ONLY the derived set feeding RoomList. Registrations remain — harmless pre-messaging (no traffic in Phase 2). Document; revisit Phase 3/4.
**Warning signs:** any new remove/unsubscribe code in the FFI or storage layer.

### Pitfall 5: Bare-counter entity ids (CR-01 replay at the entity layer)
**What goes wrong:** A fresh process mints `room-0`, overwriting a prior session's `gcs/entities/rooms/room-0` under LWW.
**Why:** Same root cause as CR-01; the regression test exists precisely for this.
**How to avoid:** Seed+counter idiom for entity ids (Pattern 3). Consider asserting the salt-separator shape in entity tests, mirroring test_gcs_ffi_sdk's `kMinIdSaltSeparators` check.
**Warning signs:** ids like `space-3` with no seed component.

### Pitfall 6: Two-session tests without shutdown between cycles
**What goes wrong:** Second `gcs_init` in the same process returns the FIRST session (idempotency guard, gcs_core_ffi.cpp:187-190) — the test "passes" without exercising restart.
**How to avoid:** Follow the CR-01 shape: init → act → `gcs_shutdown(handle)` → init again on the same db_path. For pure EntityStore restart tests, two CoreSession/EntityStore instances (sequential, injected pubsub) on one db_path are lighter than the SDK-node path (test_gcs_core_smoke fixture).
**Warning signs:** two live handles; missing gcs_shutdown in the test.

### Pitfall 7: Rail selection semantics for catalog-but-not-joined rooms
**What goes wrong:** `RailCubit.selectRoom` ignores topics not in the joined list (verified rail_cubit.dart:60-65) — taps on standalone rooms (never joined in Phase 2) silently do nothing.
**Why:** Selection was joined-only by construction in Phase 1; the tree now renders unjoined rooms.
**How to avoid:** Decide explicitly in the plan. Recommended minimal: render unjoined rooms with `ScaffoldPressable(disabled: true)` (dimmed, non-interactive) — honest state, zero composer changes; revisit when joining gets a UI. Alternative: allow selection and gate the composer on joined membership (touches ComposerCubit).
**Warning signs:** tap handlers on rows that can never select.

### Pitfall 8: Empty-name / unknown-parent commands accepted
**What goes wrong:** Records with empty names or rooms orphaned by bad parent ids poison the tree.
**How to avoid:** FFI validation mirroring existing arms: empty name → `PostErrorNotice` + `GCS_ERROR_INVALID_ARGUMENT` (join_topic/send_text precedent); `create_room` with non-empty parent that fails `IsValidParentSpace` → same. Empty parent = standalone (valid by D-01).

### Pitfall 9: Pushing SpaceTree only from gcs_init
**What goes wrong:** Dart never sees the tree (port not yet registered at init; posts are no-ops — verified PostToDart guard).
**How to avoid:** Push SpaceTree from `gcs_subscribe` (before RoomList/Readiness) and after every mutation. Init only loads state.

### Pitfall 10: Scope creep into delete commands/UI
**What goes wrong:** Phase 2 grows a delete flow no success criterion asks for.
**How to avoid:** D-03 lands the tombstone FIELDS + reader-skip semantics (unit-testable by direct Put of a tombstoned record). Delete commands/UI arrive with moderation/membership phases (MODR-01 is Phase 5; message-tombstone UI is Phase 3 per deferred list). See Gap Analysis.

## Code Examples

### FFI command arm (create_room) — concrete shape

```cpp
// Source: grounded in gcs_publish's kJoinTopic arm (gcs_core_ffi.cpp:290-322, read this session)
case gcs::chat::GcsCommand::kCreateRoom:
{
    const gcs::chat::CreateRoomCommand& cmd = command.create_room();
    if ( cmd.name().empty() )
    {
        PostErrorNotice( "create_room rejected: name is empty" );
        return GCS_ERROR_INVALID_ARGUMENT;
    }
    if ( !cmd.parent_space_id().empty() && !g_entities->IsValidParentSpace( cmd.parent_space_id() ) )
    {
        PostErrorNotice( "create_room rejected: parent space '" + cmd.parent_space_id() + "' not found" );
        return GCS_ERROR_INVALID_ARGUMENT;
    }
    auto created = g_entities->CreateRoom( cmd.name(), cmd.parent_space_id() );
    if ( !created.has_value() )
    {
        PostErrorNotice( "create_room store write failed for '" + cmd.name() + "'" );
        return GCS_ERROR_GENERIC;
    }
    // Derived joins may change (parent autoJoin) — recompute + register topics.
    PostToDart( BuildSpaceTreeEvent() );
    PostToDart( BuildRoomListEvent() );
    return GCS_OK;
}
```

### Restart persistence test skeleton (criterion 4)

```cpp
// Source: composed from test_gcs_core_smoke.cpp fixture (injected pubsub, port 0)
// + test_gcs_ffi_sdk.cpp two-cycle pattern (read this session). Wait-condition only.
TEST_F( GcsEntities, EntitiesSurviveSessionRestartOnSharedDb )
{
    auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
    auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );
    {
        gcs::CoreSession::Config cfg{};  cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession sessionA( cfg );
        ASSERT_TRUE( sessionA.Initialize( pubsub, graphsync.network ).has_value() );
        gcs::EntityStore storeA( sessionA );
        ASSERT_TRUE( storeA.CreateSpace( "ops", true, true ).has_value() );
        sessionA.Shutdown();
    }
    {
        gcs::CoreSession::Config cfg{};  cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession sessionB( cfg );
        ASSERT_TRUE( sessionB.Initialize( pubsub, graphsync.network ).has_value() );
        gcs::EntityStore storeB( sessionB );
        ASSERT_TRUE( storeB.LoadFromStore().has_value() );       // manifest -> N+1 Gets
        EXPECT_EQ( storeB.Spaces().front().name(), "ops" );      // survived restart
        sessionB.Shutdown();
    }
    pubsub->Stop();
}
```

### Dart dispatch arm

```dart
// Source: _dispatchEvent pattern, session_cubit.dart:353-372 (read this session)
if (event.hasSpaceTree()) {
  _railCubit?.setTree(event.spaceTree.space, event.spaceTree.room);
  return;
}
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Bare `<room_name>` topics (architecture note sketch) | Opaque ids + name-as-metadata (D-01) | Phase 2 context, 2026-09-18 | Matches Matrix/Discord/Slack id-vs-alias precedent (advisor-verified per CONTEXT); rename/move never splits history |
| In-memory catalog (`g_roomTopics`) | CRDT-persisted entity records + manifest | This phase | Catalog survives restart (criterion 4); records reuse the per-record key precedent from Phase 1 messages |

**Deprecated/outdated:** nothing in-repo is deprecated by this phase; `g_roomTopics` remains (as the joined-session view's backing set, now including derived joins).

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | `gcs_chat.pb.dart` regeneration remains a manual, committed step (no CMake automation was added since 01-05) | Pitfall 1, Environment | If automation exists elsewhere, the regen task is a no-op (harmless) |
| A2 | `ScaffoldPressable.disabled` + existing row composition suffice for unjoined-room rows (dimmed non-interactive) — exact visual per scaffold capabilities | Pitfall 7, Pattern 6 | Minor UI adjustment; atoms verified present, styling is planner's call |
| A3 | `SpaceTree` flat-records shape (vs nested tree proto) is acceptable to the planner as the D-02 "SpaceTree message" realization | Pattern 1 | Shape change only — both are append-only-safe; flat recommended for type reuse |
| A4 | Expand/collapse of space nodes can be a simple always-expanded list (or local-state expansion) without a new scaffold atom | Pattern 6 | If a disclosure atom is required, `scaffold_disclosure.dart` exists in the component list (unverified API) |
| A5 | Entity records need no broadcast topics in Phase 2 (local persistence satisfies criterion 4) | Pattern 2, Pitfall 3 | If the user expects multi-device catalog sync this phase, GcsGlobalDb needs a topics-bearing Put overload — scope conversation required |

All other claims were verified by reading this repo's sources this session (file:line cited inline).

## Open Questions (RESOLVED)

1. **Selection/join semantics for unjoined rooms (Pitfall 7)** — **RESOLVED: disabled rows.** Catalog-but-not-joined rooms render with `ScaffoldPressable(disabled: true)` + `Semantics(enabled: false)` (dimmed, non-interactive); joined rooms stay tappable. Adopted in 02-05 Task 1 (room_rail.dart render) and test-proven in 02-05 Task 2 (`UnjoinedRoomRowIsDisabled`). Join UI deferred to a later phase.
2. **Delete command inclusion (Gap: none of the 4 criteria require it)** — **RESOLVED: field-only tombstones, no delete command/UI this phase.** D-03 lands the tombstone fields (`deleted`/`deleted_at_ms`, present from creation, set by NO Phase 2 command) and reader-skip semantics. Adopted in 02-01 Task 1 (proto fields) and 02-01 Task 3 (`TombstonedRecordsAreSkipped`). Delete commands/UI land with a later moderation/membership phase.
3. **Space-node "+" affordance modality (CONTEXT discretion)** — **RESOLVED: always-visible trailing icon.** Each space node row ends with an edit icon + a "+" icon (both `textSecondary`, always visible, no hover/long-press dependence). Adopted in 02-05 Task 1 (space node row edit + "+" affordances).
4. **Manifest N+1 read scaling** — **RESOLVED: no action.** Fine at Phase 2 scale (tens of entities); accepted as the v1 tradeoff (recorded as T-02-03 DoS `accept` disposition in the 02-01 threat model). A future prefix-query/batched design is a later optimization if GlobalDB ever gains enumeration.

## Gap Analysis (success criteria vs CONTEXT decisions)

| Criterion | Covered by decisions? | Gap / Resolution |
|-----------|----------------------|------------------|
| 1. Create public/private space, see it in local space list | D-01/D-02/D-05 fully | None — "local space list" = rail Spaces section from SpaceTree |
| 2. Create room in space or standalone | D-01 (unified record) fully | None — empty parent = standalone |
| 3. Toggle autoJoinRooms, observe join behavior change | D-04 defines semantics; D-05 provides the edit surface | Requires the UPDATE command (UpdateSpaceCommand) — implied by "same dialog serves create and edit"; resolved in Pattern 1. Observation = RoomList change in rail (derived joins evaporate/return) |
| 4. Metadata survives restart | D-02 defines persistence layout | None — two-session test pattern exists (CR-01); restart RoomList = smoke ∪ derived (explicit joins from prior sessions remain ephemeral — carried forward from Phase 1, unchanged) |

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| dart | Dart pb regen, analyze, tests | ✓ | 3.11.5 | — |
| flutter | app tests / builds | ✓ | 3.41.9 | — |
| protoc (thirdparty) | C++ + Dart proto gen | ✓ | libprotoc 34.0 (`thirdparty/build/OSX/Debug/protobuf/bin/protoc`) | — |
| protoc-gen-dart (global) | Dart pb regen | ✓ | 22.5.0 (pinned — correct) | reactivate pinned version if changed |
| ninja + cmake | builds | ✓ | 1.13.2 / 3.29.2 | — |
| GTest | C++ tests | ✓ | thirdparty (26/26 passing per 01-VERIFICATION) | — |
| GeniusSDK node | FFI two-session test | ✓ (test_gcs_ffi_sdk passes; option-C skip contract if boot fails) | — | injected-pubsub EntityStore tests cover restart without node |

**Missing dependencies with no fallback:** none.
**Missing dependencies with fallback:** none.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | C++: GTest via `gcs_test` macro (test/CMakeLists.txt). Dart: flutter_test (`flutter test`), plain fakes — no bloc_test package (verified dev_dependencies) |
| Config file | test/CMakeLists.txt (C++); src/app/CMakeLists.txt app_test/app_analyze targets (Dart) |
| Quick run command | `ninja -C build/OSX/Debug && ctest --test-dir build/OSX/Debug -R "gcs" --output-on-failure` |
| Full suite command | `ctest --test-dir build/OSX/Debug --output-on-failure` + `cd src/app && flutter test` (via `ninja -C build/OSX/Debug app_test`) |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| CORE-01 | CreateSpace writes record + manifest keys; SpaceTree contains the space | unit | `ctest --test-dir build/OSX/Debug -R test_gcs_entities --output-on-failure` | ❌ Wave 0 |
| CORE-02 | CreateRoom in-space + standalone (empty parent); parent validation | unit | same | ❌ Wave 0 |
| CORE-03 | Derived joins: true→rooms joined, false→evaporate, true→return (retroactive) | unit | same | ❌ Wave 0 |
| CORE-04 (restart, criterion 4) | Entities survive session restart on shared db_path | unit (injected pubsub) + FFI integration | same + `ctest -R test_gcs_ffi_sdk` | Partially (extend test_gcs_ffi_sdk.cpp or new binary) |
| proto append-only | New arms parse; existing tags untouched | unit (C++ parse round-trips) | same | ❌ Wave 0 |
| Dart dispatch | Pushed SpaceTree bytes → RailState tree | unit (handlePushedBytes, shell_cubits_test pattern) | `cd src/app && flutter test test/cubits/shell_cubits_test.dart` | ✅ extend |
| Dart rail | Tree render: sections, badge, empty state, disabled unjoined rows | widget | `flutter test test/chat_shell_test.dart` (+ new rail test) | Partially — extend |
| Dialog | Create + edit publish correct GcsCommand via fake transport; toast on failure | widget | `flutter test test/shell/space_room_dialog_test.dart` | ❌ Wave 0 |

### Sampling Rate
- **Per task commit:** `ninja -C build/OSX/Debug && ctest --test-dir build/OSX/Debug -R "gcs" --output-on-failure` and/or targeted `flutter test <file>`
- **Per wave merge:** full ctest + `ninja -C build/OSX/Debug app_analyze app_test`
- **Phase gate:** full suite green before `/gsd:verify-work`

### Wave 0 Gaps
- [ ] `test/test_gcs_entities.cpp` — EntityStore unit tests (CORE-01/02/03 + restart); register via `gcs_test(test_gcs_entities test_gcs_entities.cpp "gcs_core;gcs_storage;neoswarm_common")`
- [ ] FFI-level create/update two-session coverage — extend `test/test_gcs_ffi_sdk.cpp` (preferred; its fixture already owns the node + fake port) or a sibling binary
- [ ] `src/app/test/shell/space_room_dialog_test.dart` — dialog widget test (fake GcsCommandTransport recorder, per shell_cubits_test fake pattern)
- [ ] Dart pb regeneration task early in the wave order (all Dart work depends on the new arms)

## Security Domain

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | No identity until membership phase (libp2p peer id later) |
| V3 Session Management | no | Single local session; FFI handle lifetime rules already locked (Phase 1) |
| V4 Access Control | no (see note) | `is_public` is display metadata ONLY in Phase 2 — no access enforcement exists until membership/encryption phases. Plan/docs must state this; do not imply privacy enforcement |
| V5 Input Validation | yes | FFI-arm validation: empty-name rejection, parent-existence check, protobuf parse failure → ErrorNotice + INVALID_ARGUMENT (existing pattern); untrusted bytes never reach partially-parsed state (gcs_init precedent) |
| V6 Cryptography | no | Encryption deferred v1.1 (locked) |

### Known Threat Patterns for C++ FFI + CRDT KV

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Malformed command bytes at ABI | Tampering | ParseFromArray failure → reject before any state change (existing gcs_publish pattern) |
| Payload size narrowing (size_t→int) | Tampering | `payloadLength > INT_MAX` guard (existing IN-01 pattern) — copy for any new byte path (none new expected; commands reuse gcs_publish) |
| Hostile/corrupt record bytes at startup replay | Tampering/DoS | Per-record ParseFromString failure ⇒ skip record + spdlog::warn, never abort startup |
| Key-space collision from id reuse | Tampering | Seed+counter id minting (CR-01) |

## Project Constraints (from CLAUDE.md / global instructions)

Extracted from the user's global CLAUDE.md (no repo-local CLAUDE.md exists — verified):

- **Project-grounded analysis only** — this research cites file:line for every load-bearing claim; planner tasks must read the named files first.
- **Minimal change philosophy** — smallest diff; no refactors. Phase 2 adds files/arms; it must NOT restructure existing proto messages, CoreSession, or the storage surface.
- **C++17 ceiling** — no C++20 features (e.g., no designated initializers in C++-side calls beyond what's already used, no std::format).
- **No OS `#ifdef` in source** — platform variance resolves via CMake only.
- **All variables initialized; braces on all if/while/for; Allman/Ullman bracing; Doxygen headers on every function/public interface** (matches existing gcs_core.hpp style).
- **No magic numbers** — `constexpr` named constants (kCamelCase) for all numerics except trivial 0/1/-1.
- **Tests: Google Test + wait-condition templates (`test/test_wait_condition.hpp`), NEVER `std::this_thread::sleep_for`; ≥80% coverage on new code.**
- **spdlog for all diagnostics; never printf/cout.**
- **Never use system-installed libraries** — thirdparty/ only (protoc, GTest, protobuf already there).
- **Do not commit without explicit user permission; run build + tests + linter + formatter first.**
- **Submodule rules** — `src/app/scaffold` is a submodule: `lib/` is read-only, never edit generated families (scaffold CLAUDE.md contract). Dart work goes in `src/app/lib/` + `src/app/test/` only.
- **PRs → develop branch (never main); `/gsd:code-review` required before ship; draft-first PRs.**

## Sources

### Primary (HIGH confidence — read this session)
- `src/proto/gcs_chat.proto` — full current contract; oneof tags 1-2 (command) and 1-4 (event)
- `src/ffi/gcs_core_ffi.cpp` — g_roomTopics, NextMessageId (149-158), message-key precedent (355), join/send arms, subscribe push ordering, init idempotency
- `src/lib/gcs_storage/gcs_global_db.hpp` + `.cpp` — complete surface; Put kNoTopics local-only (250-265); Get error conflation (267-280)
- `src/lib/gcs_core.hpp` — CoreSession pass-through design + Phase 2-3 staging note
- `../SuperGenius/src/crdt/impl/crdt_datastore.cpp` — priority = head_height + 1 stamping (1541-1547); PutKey path (1374-1386)
- `../SuperGenius/src/crdt/impl/crdt_set.cpp` — GetElement missing-key failure path (185-198)
- `../SuperGenius/src/crdt/globaldb/globaldb.cpp` — GetKey delegation (587-595); Remove exists upstream but unexposed (597)
- `src/app/lib/cubits/{session,rail,composer}_cubit.dart`, `src/app/lib/shell/{gcs_shell,room_rail}.dart` — dispatch pipeline, RailState, selectRoom joined-only check, transport seam
- `src/app/scaffold/lib/components/` — ResponsiveDrawer.show, TextEntryFieldWidget/TextFormFieldLogic, ScaffoldSelectionIndicatorRadio/Toggle (bool-based APIs), ScaffoldBadge, showToast — APIs read directly
- `test/test_gcs_ffi_sdk.cpp` — two-session restart pattern, fake API_DL port capture, salt assertions
- `test/test_gcs_core_smoke.cpp` — injected-pubsub CoreSession test fixture
- `test/test_wait_condition.hpp` — wait-condition template
- `test/CMakeLists.txt`, `src/CMakeLists.txt`, `src/proto/CMakeLists.txt`, `src/ffi/CMakeLists.txt`, `src/app/CMakeLists.txt` — target/test wiring, app_analyze/app_test gates
- `.planning/workstreams/app/phases/01-foundation/01-05-PLAN.md` (168-172, 303-317) — protoc-gen-dart regen command + 22.5.0 pin
- `.planning/workstreams/app/phases/01-foundation/01-VERIFICATION.md` — CI state, 26/26 tests, deviations
- `.planning/workstreams/app/phases/01-foundation/01-UI-SPEC.md` — rail spacing/typography/color contracts (space12 section breaks, Title space headers, Body item labels, borderRadiusCard 15)
- `.planning/notes/gcs-chat-architecture.md` — autoJoinRooms contract, unified room model (retained); bare-name topics superseded by D-01
- Local toolchain — dart 3.11.5 / flutter 3.41.9 / protoc 34.0 / protoc-gen-dart 22.5.0 verified via command

### Secondary (MEDIUM confidence)
- Matrix/Discord/Slack id-vs-alias precedent — cited in 02-CONTEXT.md as advisor-verified (2026-09-18); not re-verified this session

### Tertiary (LOW confidence)
- None used.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — no new packages; all versions verified on this machine
- Architecture: HIGH — every pattern grounded in an existing verified in-repo analogue
- Pitfalls: HIGH — Pitfalls 1-6 verified against source this session; 7-10 derived from verified code behavior
- CRDT semantics: HIGH — read directly from crdt_datastore.cpp/crdt_set.cpp (not assumed from training)

**Research date:** 2026-09-19
**Valid until:** 2026-10-19 (stable — in-repo facts; re-verify only if Phase 1 PR #12 merge or thirdparty refresh lands first)
