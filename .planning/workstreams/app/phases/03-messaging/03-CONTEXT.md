# Phase 3: Messaging - Context

**Gathered:** 2026-09-22
**Status:** Ready for planning

<domain>
## Phase Boundary

Users can send and receive text messages in real-time within joined rooms; all participants converge on the same CRDT-backed message history, loaded on room join and rendered chronologically with sender identification (CORE-04, roadmap success criteria 1-4). Messages persist in GlobalDB under per-message keys, replicate via the room's CRDT broadcast topic, and survive restart. No @mentions or GCS bot responses (Phase 5+), no message deletion/moderation UI (Phase 5 — but the tombstone fields land on the record now per Phase 2 D-03), no membership/invites (Phase 4), no reply threading (bot phase), no editing (out of scope — append-only per PROJECT.md), no encryption (v1.1; schema accommodates opaque payloads since text is a bytes-ish field either way).

</domain>

<decisions>
## Implementation Decisions

### Message Storage & Read-Back
- **D-01:** **Per-message keys + converged prefix scan.** Each message is one serialized `ChatMessageState` record under `gcs/messages/<room_topic>/<message-id>` (generalizes Phase 2's per-entity key layout and the existing `room_topic + "/" + message-id` precedent in `gcs_core_ffi.cpp` ~line 709). Read-back enumerates via GlobalDB's **`QueryKeyValues`** converged prefix scan (verified exposed in SuperGenius `globaldb.hpp` ~line 144) — surfaced through `GcsGlobalDb`/`CoreSession` as a new API — then deserializes, sorts by `(timestamp, id)` (id tiebreak makes ordering total), and ships as one batch. Rejected: a per-room index manifest of message ids (the Phase 2 entity-manifest pattern) — a manifest held under one key is a read-union-write LWW lost-update hazard at message send rates, and it is unnecessary given the prefix scan exists. Single-writer-per-key LWW is exactly correct for immutable messages: no concurrent writes to the same key, so no lost-update path.
- **D-02:** **`GcsGlobalDb::Put` gets a topics parameter.** Today `Put` hardcodes `kNoTopics` (`gcs_global_db.cpp` ~lines 264-266) — every write is local-only, nothing replicates. Message Puts must pass `{room_topic}` so the CRDT broadcast replicates the archive. Fix is at the wrapper (add a topics-aware overload; keep the existing signature delegating to it with `kNoTopics`), not a new storage class.

### Delivery & Sender Identity
- **D-03:** **GossipSub fast path for live delivery; CRDT as the archive.** Send (C++ side): mint id (`NextMessageId()` idiom) → stamp sender + timestamp → push local `pending` echo (D-06) → **publish the serialized `ChatMessageState` on the room's pub/sub topic** (live path, reuses Phase 1's topic infra) **and `Put` it with `{room_topic}`** (archive replication per D-02) → push `complete`. Receive: a pub/sub message arrives → decode → **apply-once keyed by message id** (dedupe — the same message will also arrive via CRDT sync) → render + `PutLocal` into the archive with no re-broadcast (no echo loops; graphsync heals any pub/sub delivery that was missed). Rejected: CRDT write-through as the only delivery path — render latency ties to graphsync convergence rather than the live topic. *(User choice, 2026-09-22: "GossipSub fast-path seems better as then the CRDT is just the archive of that message.")*
- **D-04:** **Sender identity = wallet address, stamped by C++, unsigned MVP.** The sender field is the local node's `GeniusNode::GetAddress()` (the embedded child wallet from `gcs_init`), set in the FFI send path — Dart never supplies or trusts a sender. Not cryptographically signed in this phase (any syncing node could forge a sender string); signing is v1.1 with encryption. `ChatMessageState` gains a `sender` field (append-only, next free tag); UI renders it truncated/short-form.

### Message Record Shape
- **D-05:** **Append-only field additions to `ChatMessageState`:** `sender` (D-04) and tombstone fields (`deleted` + `deleted_at_ms`) present from creation — greenfield now per Phase 2 D-03; Phase 5 moderation flips flags rather than migrating. Ordering fields (timestamp, id) already exist.

### History Loading
- **D-06:** **Join-time replay.** On `join_topic` (and derived joins from Phase 2's evaluator), C++ runs the D-01 prefix scan under the existing session mutex and pushes one new **`MessageHistory`** batch event (new `GcsEvent` oneof arm, append-only) containing the full sorted room history; Dart replaces the room's message list (`replaceAll` semantics, mirroring `RailCubit.setRooms`). Live messages append after the batch — the apply-once dedupe set (D-03) absorbs any pub/sub message that lands between scan and subscribe. No incremental pagination, no scroll-windowing API for MVP (the client-side cap bounds rendering).

### Send Lifecycle UX
- **D-07:** **Pending echo + by-id upsert.** C++ pushes the sender's own message twice — once as `pending` (optimistic echo at send-accept) and once as `complete` with the same id; failures push `error` for that id. `MessageFlowCubit` upserts by message id (~10-line change to the current append-only mapping) so pending → complete/error replaces in place rather than duplicating. Error is a **terminal** tint; re-send is a **manual user action** (re-submit via the composer path with a new id) — no auto-retry loop. Transport-level failures (topic publish throws) additionally surface a `showToast`; send-path validation failures stay inline per the Phase 2 dialog pattern. The composer itself is unchanged — Enter and the button already converge on one submit path in `composer_cubit.dart`.

### Claude's Discretion
- Proto field names/numbering within the append-only discipline (`sender`, tombstone fields, `MessageHistory` event shape).
- Dedupe-set implementation in C++ (fixed-size LRU per room vs global keyed map) and its retention policy.
- Exact prefix string layout (`gcs/messages/<room_topic>/<id>` — slash-vs-colon details) as long as it is one stable prefix per room and consistent with the scan.
- Whether `QueryKeyValues` is exposed on `GcsGlobalDb` as a raw passthrough returning key/value byte pairs or as a decoded-message helper.
- Pending/complete/error state representation in the pushed `ChatMessageState` (reuse `MessageState` enum values vs new values — append-only either way).
- Dart-side history `replaceAll` wiring details and any client-side render cap constant.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Locked prior-phase constraints this phase inherits
- `.planning/workstreams/app/phases/01-foundation/01-CONTEXT.md` — D-04 (C++ owns state; Dart is a thin subscriber), D-24/D-26 (append-only proto discipline), D-27/D-29 (topic pub/sub FFI plane: data-only Dart commands, protobuf `GcsEvent` envelopes, codec-tagged bytes, raw error strings).
- `.planning/workstreams/app/phases/02-spaces-rooms/02-CONTEXT.md` — D-01 (opaque ids; C++ mints), D-02 (per-entity KV records; GlobalDB per-key LWW, priority = head_height + 1, **no union semantics** — verified in `crdt_datastore.cpp` ~1546-1556), D-03 (tombstones at creation — message records follow via D-05), D-04 (derived joins — history replay hangs off this join evaluation).

### Architecture
- `.planning/notes/gcs-chat-architecture.md` — Message Flow section, room topic model, CRDT sync contract.

### Verified SuperGenius CRDT surface (load-bearing for D-01/D-02/D-03)
- `../SuperGenius/src/crdt/globaldb/globaldb.hpp` — `QueryKeyValues` converged prefix scan (~line 144), `PutConvergentImmutable` (~line 104), `Remove` (~line 138). `GcsGlobalDb` currently exposes only string `Put`/`Get` — Phase 3 widens it.
- `../SuperGenius/src/crdt/impl/crdt_datastore.cpp` — per-key LWW merge semantics; Put delta stamping.
- `../SuperGenius/src/crdt/crdt_datastore.hpp` — `RegisterNewElementCallback`: receive-side push hook for CRDT-synced writes; fires on the io thread (mutex discipline required when bridging into the FFI session state).

### Wire contract and existing session code
- `src/proto/gcs_chat.proto` — the append-only contract being extended: `SendTextCommand`, `ChatMessageState`, `GcsCommand`/`GcsEvent` oneofs, `MessageRole`/`MessageState` enums.
- `src/ffi/gcs_core_ffi.cpp` — `send_text` arm; `NextMessageId()` seed+counter idiom (~lines 449-458); per-record message key precedent (~line 709); `g_roomTopics` + `join_topic` handling where history replay hooks in.
- `src/lib/gcs_storage/gcs_global_db.{hpp,cpp}` — the wrapper being widened: `Put` hardcodes `kNoTopics` (~lines 264-266 — the D-02 fix site); `AddBroadcastTopic`/`AddListenTopic` already exist.

### Flutter consumption surface
- `src/app/lib/cubits/message_flow_cubit.dart` — current append-only mapping; gains by-id upsert (D-07).
- `src/app/lib/cubits/composer_cubit.dart` — publishes `SendTextCommand` via `GcsCommandTransport.publishCommand`; submit path already converged (no changes needed).
- `src/app/scaffold/CLAUDE.md` — scaffold submodule contract (lib/ read-only, never edit generated families).
- `src/app/scaffold/lib/components/` — `showToast` (`toast_manager.dart`) for transport errors; existing message-list atoms for pending/error tints.

### Resolved planning input
- `.planning/todos/pending/design-message-crdt-schema.md` — its four open questions are answered by this context (fields → D-04/D-05; ordering → D-01 `(timestamp, id)` sort, wall-clock + id tiebreak, no vector clocks/Lamport; delete → tombstone fields now, moderation UI Phase 5, no edit; encrypted payloads → v1.1, schema unchanged since content is an opaque text field).

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- Phase 1 topic pub/sub (`AddBroadcastTopic`/`AddListenTopic` + `GcsEvent` push plane) — the live GossipSub path in D-03 already works; messaging adds payload + dedupe on top.
- `NextMessageId()` — deterministic-ish C++ id minting for messages (D-03 send path).
- Per-record key precedent (`room_topic + "/" + message-id`, `gcs_core_ffi.cpp` ~709) — D-01 generalizes it into a stable scan prefix.
- `RailCubit.setRooms` full-replacement pattern — the model for the Dart-side `MessageHistory` → `replaceAll` (D-06).
- `GcsCommandTransport.publishCommand` — composer seam, unchanged.

### Established Patterns
- Append-only proto evolution (only add fields/messages/events; never retype/remove/reorder).
- C++ owns state, Dart Cubits are thin subscribers rendering pushed events — no local Dart optimism beyond the C++-pushed pending echo.
- GlobalDB per-key LWW with no union — correct for immutable per-message keys, wrong for any shared-key index (why D-01 rejects a manifest).
- Tests use wait-condition templates (no `sleep_for`); C++17 ceiling; no OS `#ifdef` in source; spdlog for diagnostics.

### Integration Points
- `GcsEvent` oneof gains `MessageHistory` (D-06); `ChatMessageState` gains `sender` + tombstone fields (D-04/D-05).
- `GcsGlobalDb` gains a topics-aware `Put` overload (or parameter) and a `QueryKeyValues`-backed prefix scan API (D-01/D-02).
- `RegisterNewElementCallback` (SuperGenius) bridges CRDT-synced message arrivals into the apply-once/dedupe + `PutLocal` receive path (D-03).
- Join path (explicit `join_topic` + Phase 2 derived joins) triggers the history scan + batch push (D-06).
- `MessageFlowCubit` gains by-id upsert; message-list widget renders sender + pending/error states (D-04/D-07).

</code_context>

<specifics>
## Specific Ideas

- "GossipSub fast-path seems better as then the CRDT is just the archive of that message" (2026-09-22) — D-03: publish on the room topic for live delivery, Put-with-topics for archive replication; receivers apply-once and `PutLocal` without re-broadcast.
- Advisor research (4 parallel gsd-advisor-researcher runs, 2026-09-22) verified in-repo: `QueryKeyValues` prefix scan exists in SuperGenius GlobalDB (supersedes an earlier per-room index-manifest sketch — that design's LWW lost-update convergence failure is why it was dropped); `GcsGlobalDb::Put` hardcoding `kNoTopics` confirmed at `gcs_global_db.cpp` ~264-266; `RegisterNewElementCallback` confirmed as the receive-side push hook.

</specifics>

<deferred>
## Deferred Ideas

- Cryptographic message signing + sender verification — v1.1 with encryption (D-04 keeps the field unsigned).
- Reply threading / `reply_to_id` causal ordering (Lamport/HLC stamps, vector clocks) — bot phase; revisit only if bot turns need causal ordering. Simple `(timestamp, id)` sort is locked for Phase 3.
- Message deletion UI + moderator flows — Phase 5 Moderation; tombstone fields land now (D-05).
- Pagination / scroll-windowed history loading — post-MVP; batch `replaceAll` is locked for Phase 3.
- Auto-retry of failed sends — rejected for MVP (manual re-send only, D-07); revisit with a backoff policy if UAT shows transport flakiness.
- @mentions and GCS bot auto-answer — Phase 5+.

</deferred>

---

*Phase: 3-Messaging*
*Context gathered: 2026-09-22*
