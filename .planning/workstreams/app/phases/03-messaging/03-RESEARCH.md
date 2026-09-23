# Phase 3: Messaging - Research

**Researched:** 2026-09-23
**Domain:** P2P group messaging over a CRDT (SuperGenius GlobalDB) + GossipSub, with a Dart FFI-push render layer
**Confidence:** HIGH (architecture/storage surface verified in-repo; SuperGenius API signatures MEDIUM — prebuilt SDK not in local tree)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

- **D-01:** **Per-message keys + converged prefix scan.** Each message is one serialized `ChatMessageState` record under `gcs/messages/<room_topic>/<message-id>` (generalizes Phase 2's per-entity key layout and the existing `room_topic + "/" + message-id` precedent in `gcs_core_ffi.cpp` ~line 709). Read-back enumerates via GlobalDB's **`QueryKeyValues`** converged prefix scan (verified exposed in SuperGenius `globaldb.hpp` ~line 144) — surfaced through `GcsGlobalDb`/`CoreSession` as a new API — then deserializes, sorts by `(timestamp, id)` (id tiebreak makes ordering total), and ships as one batch. Rejected: a per-room index manifest of message ids (the Phase 2 entity-manifest pattern) — a manifest held under one key is a read-union-write LWW lost-update hazard at message send rates, and it is unnecessary given the prefix scan exists. Single-writer-per-key LWW is exactly correct for immutable messages: no concurrent writes to the same key, so no lost-update path.
- **D-02:** **`GcsGlobalDb::Put` gets a topics parameter.** Today `Put` hardcodes `kNoTopics` (`gcs_global_db.cpp` ~lines 264-266) — every write is local-only, nothing replicates. Message Puts must pass `{room_topic}` so the CRDT broadcast replicates the archive. Fix is at the wrapper (add a topics-aware overload; keep the existing signature delegating to it with `kNoTopics`), not a new storage class.
- **D-03:** **GossipSub fast path for live delivery; CRDT as the archive.** Send (C++ side): mint id (`NextMessageId()` idiom) → stamp sender + timestamp → push local `pending` echo (D-06) → **publish the serialized `ChatMessageState` on the room's pub/sub topic** (live path, reuses Phase 1's topic infra) **and `Put` it with `{room_topic}`** (archive replication per D-02) → push `complete`. Receive: a pub/sub message arrives → decode → **apply-once keyed by message id** (dedupe — the same message will also arrive via CRDT sync) → render + `PutLocal` into the archive with no re-broadcast (no echo loops; graphsync heals any pub/sub delivery that was missed). Rejected: CRDT write-through as the only delivery path — render latency ties to graphsync convergence rather than the live topic. *(User choice, 2026-09-22: "GossipSub fast-path seems better as then the CRDT is just the archive of that message.")*
- **D-04:** **Sender identity = wallet address, stamped by C++, unsigned MVP.** The sender field is the local node's `GeniusNode::GetAddress()` (the embedded child wallet from `gcs_init`), set in the FFI send path — Dart never supplies or trusts a sender. Not cryptographically signed in this phase (any syncing node could forge a sender string); signing is v1.1 with encryption. `ChatMessageState` gains a `sender` field (append-only, next free tag); UI renders it truncated/short-form.
- **D-05:** **Append-only field additions to `ChatMessageState`:** `sender` (D-04) and tombstone fields (`deleted` + `deleted_at_ms`) present from creation — greenfield now per Phase 2 D-03; Phase 5 moderation flips flags rather than migrating. Ordering fields (timestamp, id) already exist.
- **D-06:** **Join-time replay.** On `join_topic` (and derived joins from Phase 2's evaluator), C++ runs the D-01 prefix scan under the existing session mutex and pushes one new **`MessageHistory`** batch event (new `GcsEvent` oneof arm, append-only) containing the full sorted room history; Dart replaces the room's message list (`replaceAll` semantics, mirroring `RailCubit.setRooms`). Live messages append after the batch — the apply-once dedupe set (D-03) absorbs any pub/sub message that lands between scan and subscribe. No incremental pagination, no scroll-windowing API for MVP (the client-side cap bounds rendering).
- **D-07:** **Pending echo + by-id upsert.** C++ pushes the sender's own message twice — once as `pending` (optimistic echo at send-accept) and once as `complete` with the same id; failures push `error` for that id. `MessageFlowCubit` upserts by message id (~10-line change to the current append-only mapping) so pending → complete/error replaces in place rather than duplicating. Error is a **terminal** tint; re-send is a **manual user action** (re-submit via the composer path with a new id) — no auto-retry loop. Transport-level failures (topic publish throws) additionally surface a `showToast`; send-path validation failures stay inline per the Phase 2 dialog pattern. The composer itself is unchanged — Enter and the button already converge on one submit path in `composer_cubit.dart`.

### Claude's Discretion
- Proto field names/numbering within the append-only discipline (`sender`, tombstone fields, `MessageHistory` event shape).
- Dedupe-set implementation in C++ (fixed-size LRU per room vs global keyed map) and its retention policy.
- Exact prefix string layout (`gcs/messages/<room_topic>/<id>` — slash-vs-colon details) as long as it is one stable prefix per room and consistent with the scan.
- Whether `QueryKeyValues` is exposed on `GcsGlobalDb` as a raw passthrough returning key/value byte pairs or as a decoded-message helper.
- Pending/complete/error state representation in the pushed `ChatMessageState` (reuse `MessageState` enum values vs new values — append-only either way).
- Dart-side history `replaceAll` wiring details and any client-side render cap constant.

### Deferred Ideas (OUT OF SCOPE)
- Cryptographic message signing + sender verification — v1.1 with encryption (D-04 keeps the field unsigned).
- Reply threading / `reply_to_id` causal ordering (Lamport/HLC stamps, vector clocks) — bot phase; revisit only if bot turns need causal ordering. Simple `(timestamp, id)` sort is locked for Phase 3.
- Message deletion UI + moderator flows — Phase 5 Moderation; tombstone fields land now (D-05).
- Pagination / scroll-windowed history loading — post-MVP; batch `replaceAll` is locked for Phase 3.
- Auto-retry of failed sends — rejected for MVP (manual re-send only, D-07); revisit with a backoff policy if UAT shows transport flakiness.
- @mentions and GCS bot auto-answer — Phase 5+.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| CORE-04 | User can send and receive text messages in real-time | D-02 (topics-aware Put), D-03 (GossipSub fast path + CRDT archive), D-06 (join-time replay), D-07 (pending echo + by-id upsert) |

Success criteria trace (roadmap SC 1-4):
1. "Send and see it locally" → D-07 pending echo at send-accept (C++ pushes before any network round-trip).
2. "Second user receives without manual refresh" → D-03 live GossipSub publish on the room topic; receive path decodes and pushes to Dart.
3. "History identical across participants after sync" → D-01 converged `QueryKeyValues` prefix scan over per-message keys; D-02 topics-aware Put replicates the archive.
4. "Chronological order with sender identification" → D-01 `(timestamp, id)` total sort; D-04 sender = wallet address rendered short-form.
</phase_requirements>

## Project Constraints (from CLAUDE.md / submodule contracts)

No root `./CLAUDE.md` exists in this repo (`config.json` points to `./CLAUDE.md`; the file is absent). Binding constraints come from three sources and are treated as locked:

**C++ coding standards (user global instructions, apply to all `.cpp/.hpp` in `src/`):**
- C++17 ceiling — no C++20 features (no `boost::coroutines`, no concepts/ranges beyond C++17).
- No OS preprocessor guards in source (`#ifdef __APPLE__` etc.); platform code lives in `os/{platform}/Platform.hpp`.
- No magic numbers — `constexpr` `kCamelCase` constants (exceptions: 0, 1, -1 in trivial contexts).
- Allman/Ullman bracing, Doxygen headers on every function/public interface, all variables initialized.
- Diagnostics via `spdlog::debug/error/warn` only — never `fprintf`/`cout`/`cerr`/`printf`.
- Tests use wait-condition templates (`test/test_wait_condition.hpp`) — **never** `std::this_thread::sleep_for`. Target ≥80% coverage on new code.
- Only thirdparty libraries (no system-installed); a new library goes through the thirdparty build system.
- Program to interfaces, service-locator over singletons, composition over inheritance.

**Scaffold submodule (`src/app/scaffold/CLAUDE.md` — public contract, read-only):**
- `lib/` is read-only; never edit generated widget families (`scaffold_{card,state_view,search_bar}*`, `scaffold_{animated_display,formatted_value,image_placeholder,selection_indicator}_*`). They are committed Jinja2 output — edit `templates/components/*.jinja2` + regenerate.
- `dart analyze --fatal-infos` must be clean; `flutter test` must pass.
- App-side message-list/sender/pending/error rendering belongs in the app's own consumer space (`src/app/lib/`, `src/app/templates/`), NOT in scaffold `lib/`.

**Inherited phase contracts (already enforced in code):**
- Append-only proto evolution (`gcs_chat.proto` — only add fields/messages/enums; never retype/remove/reorder). Phase 1 D-24/D-26, Phase 2 D-01..D-04.
- C++ owns state; Dart Cubits are thin subscribers rendering pushed events (Phase 1 D-04/D-27).
- Tombstones at record creation, never physical key removal (Phase 2 D-03).
- Opaque C++-minted ids; data-only Dart commands (Phase 2 D-01, Phase 1 D-27).

## Summary

Phase 3 turns the existing Phase 1 "local echo" into a real P2P messaging path. Today the `send_text` FFI arm (`src/ffi/gcs_core_ffi.cpp` ~lines 667-720) mints an id, stamps role/state/text/timestamp, writes the message locally via `GcsGlobalDb::Put` (which hardcodes `kNoTopics`, so nothing replicates), and pushes the single `ChatMessageState` back to Dart. There is no pub/sub publish, no receive-side callback, and no history read-back — two peers in the same room currently cannot exchange messages at all.

Phase 3 closes that gap with three coordinated changes, all grounded in the code I verified in-repo:
1. **Storage widening** — `GcsGlobalDb::Put` gains a topics-aware overload (the 3-arg `GlobalDB::Put(key, value, topicSet)` already exists in the SuperGenius API — the local `gcs_global_db.cpp:266` already calls it with an empty set). A new `QueryKeyValues`-backed prefix scan is surfaced through `GcsGlobalDb`/`CoreSession` for history read-back.
2. **FFI send/receive rewrite** — the send path publishes the serialized `ChatMessageState` on the room topic (live GossipSub) AND `Put`s it with `{room_topic}` (archive), pushing `pending` then `complete`; the receive path (via SuperGenius `RegisterNewElementCallback` + pub/sub decode) applies-once by message id and `Put`s locally without re-broadcast.
3. **Proto + Dart** — `ChatMessageState` gains `sender` + tombstone fields; `GcsEvent` gains a `MessageHistory` batch arm; `MessageFlowCubit` gains by-id upsert (its generated base `ChatMessageFlowCubit` already has `replaceAll`); `SessionCubit` gains a `messageHistory` dispatch arm.

**Primary recommendation:** Implement in the C++ layer first (storage widening → send/receive path → join-time replay), then the append-only proto additions, then the thin Dart dispatch/upsert changes. The SuperGenius CRDT API names (`QueryKeyValues`, `RegisterNewElementCallback`, topics-parameter `Put`) are confirmed by the NEO-SWARM submodule's own `03-gcs-globaldb-integration` planning artifacts and the local `gcs_global_db.cpp`, but the **exact signatures** must be re-verified against the prebuilt SDK headers at plan time (the SDK is not in the local tree — see Environment Availability).

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Message id minting (`NextMessageId`) | API/Backend (C++ FFI) | — | C++ owns authority (D-04); id is the CRDT key suffix and must be process-unique (CR-01). |
| Sender identity stamping | API/Backend (C++ FFI) | — | `GeniusNode::GetAddress()` only exists C++-side; Dart never supplies/trusts a sender (D-04). |
| Live message delivery (send + receive push) | API/Backend (C++ GossipSub) | — | GossipSub fast path (D-03); topic infra already wired via `AddBroadcastTopic`/`AddListenTopic`. |
| CRDT archive replication | Database/Storage (GlobalDB) | API/Backend (C++ wrapper) | `Put` with `{room_topic}` replicates via the room's CRDT broadcast topic (D-02). |
| History read-back (prefix scan) | Database/Storage (GlobalDB `QueryKeyValues`) | API/Backend (C++ sort + batch) | Converged prefix scan over `gcs/messages/<room_topic>/` (D-01); C++ deserializes + sorts `(timestamp, id)`. |
| Receive-side dedupe + local archive write | API/Backend (C++) | Database/Storage | Apply-once keyed by id absorbs the pub/sub↔CRDT double-delivery; `PutLocal` = empty-topic Put (no echo loops). |
| Message rendering (sender, pending/error/complete, chronological list) | Browser/Client (Flutter) | — | Thin subscriber; all state arrives pushed; no local optimism beyond the C++-pushed pending echo. |
| Composer send path | Browser/Client (Flutter) | — | Already converges on one submit path (`composer_cubit.dart`); unchanged this phase. |
| Join-triggered history replay | API/Backend (C++) | Browser/Client (Flutter replaceAll) | Hooked on `join_topic` + `RefreshDerivedJoins` (D-06); Dart mirrors `RailCubit.setRooms` full-replacement. |

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| SuperGenius GlobalDB | prebuilt (thirdparty release) | CRDT store: per-key LWW, `QueryKeyValues` prefix scan, `RegisterNewElementCallback`, topics-parameter `Put` | Already owned by `GcsGlobalDb`; the CRDT substrate for CORE-05 (Phase 1). No alternative. |
| libp2p GossipPubSub | prebuilt (via SuperGenius/GeniusSDK) | Live room-topic message delivery (D-03 fast path) | Already wired (`AddBroadcastTopic`/`AddListenTopic`); reuses Phase 1 topic infra. |
| protobuf (C++) | thirdparty (add_proto_library) | `gcs_chat.proto` wire contract, both halves | Append-only schema source (D-29); `protoc` is the both-sides generator. |
| protobuf (Dart) | 4.2.0 (locked) | Generated `gcs_chat.pb.dart` decode/encode | Matches the C++ wire; `protoc_plugin` 22.5.0 dev-dep. |
| flutter_bloc | 9.1.1 (locked) | Cubit state holders (`MessageFlowCubit`, `SessionCubit`, `ComposerCubit`, `RailCubit`) | Established in Phases 1-2. |
| frontend_scaffold | path `scaffold` (submodule pin `8ada743`) | Message-list atoms, `showToast`, composer | Read-only public contract (scaffold CLAUDE.md); app composites live in `src/app/`. |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| spdlog | thirdparty | C++ diagnostics | Mandatory for all new logging (no stdio). |
| GTest + `test_wait_condition.hpp` | thirdparty | C++ unit/integration tests | Wait-condition templates only; no `sleep_for`. |
| ffi / fixnum (Dart) | 2.2.0 / 1.1.1 (locked) | FFI bindings; int64 proto fields | Already wired; unchanged. |
| Boost.Asio | prebuilt (via SuperGenius) | io_context/scheduler for GlobalDB | Already owned by `GcsGlobalDb`; unchanged. |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| `QueryKeyValues` prefix scan (D-01) | Per-room id-manifest key (Phase 2 pattern) | Manifest is a read-union-write LWW lost-update hazard at send rates; rejected (locked). |
| Topics-aware `Put` (D-02) | CRDT write-through as the only delivery path | Render latency ties to graphsync convergence; rejected in favor of GossipSub fast path (locked). |
| `PutConvergentImmutable` (exists in GlobalDB ~line 104) | Regular `Put` with topics (locked D-01/D-02) | Immutable-put may skip LWW merge, but single-writer-per-key makes LWW already correct; regular Put is locked. |

**Installation:** No new external packages are installed this phase. The proto is extended in-repo; the SuperGenius SDK and all Dart deps are already vendored/pinned from Phases 1-2.

**Version verification:** Locked Dart versions confirmed from `src/app/pubspec.lock` (flutter_bloc 9.1.1, protobuf 4.2.0, ffi 2.2.0, fixnum 1.1.1). SuperGenius/GeniusSDK versions are thirdparty release tarballs (not versionable here — see Environment Availability). Toolchain probed on this machine: cmake 3.29.2, ninja 1.13.2, Flutter 3.41.9, Dart 3.11.5, Apple clang 17.

## Package Legitimacy Audit

> This phase installs **zero** new external packages. No npm/PyPI/crates/pub.dev additions; therefore no slopcheck gate applies. The only dependency surface changes are (a) appending fields/messages to the in-repo `gcs_chat.proto`, and (b) calling already-vendored SuperGenius GlobalDB APIs.

| Package | Registry | Notes | Disposition |
|---------|----------|-------|-------------|
| SuperGenius GlobalDB / GeniusSDK | thirdparty (prebuilt release, already a dependency) | Already linked by `gcs_storage`; Phase 3 only calls more of its API. | Approved (existing dep) |
| gcs_chat.proto | in-repo | Extended, not installed. | N/A (source) |
| flutter_bloc / protobuf / ffi / fixnum / frontend_scaffold | pub.dev / path (already locked) | Already in `pubspec.lock`. | Approved (existing deps) |

**Packages removed due to slopcheck [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

## Architecture Patterns

### System Architecture Diagram

```
Dart (Flutter)                      C++ FFI (gcs_core_ffi.cpp)              Storage / Network
───────────────                     ──────────────────────────────          ────────────────────────
ComposerCubit ── publishCommand ──► gcs_publish (command topic)
   (send_text)                         │ parse GcsCommand
                                       ▼
                              ┌── send_text arm ───────────────────────┐
                              │ mint id (NextMessageId)                │
                              │ stamp sender=GetAddress(), role, ts    │
                              │ push pending ChatMessageState ──► Dart │
                              │ publish ChatMessageState on room topic ┼──► GossipSub ──► other peers
                              │ Put(key, val, {room_topic}) ───────────┼──► GlobalDB (CRDT archive)
                              │ push complete ChatMessageState ──► Dart│
                              └────────────────────────────────────────┘

                              ┌── receive path ────────────────────────┐
GossipSub (peer) ────────────►│ decode ChatMessageState                │
GlobalDB RegisterNewElement   │ apply-once keyed by message id         │
  callback (CRDT sync) ──────►│   (dedupe absorbs pub/sub + CRDT dup)  │
                              │ render → push ChatMessageState ──► Dart│
                              │ Put(key, val, {})  // local, no rebroadcast
                              └────────────────────────────────────────┘

                              ┌── join_topic arm ─────────────────────┐
join_topic / derived join ───►│ QueryKeyValues("gcs/messages/<room>/") │──► GlobalDB prefix scan
                              │ deserialize → sort (timestamp, id)     │
                              │ push MessageHistory batch ──────► Dart │
                              └────────────────────────────────────────┘

Dart receive port ─► SessionCubit._dispatchEvent:
   message       → MessageFlowCubit.upsert (by id, pending→complete)
   messageHistory→ MessageFlowCubit.replaceAll
   (SpaceTree/RoomList/readiness/error unchanged)
```

### Recommended Project Structure (delta, not full tree)

```
src/lib/gcs_storage/gcs_global_db.{hpp,cpp}   # + topics-aware Put overload; + QueryKeyValues prefix scan
src/lib/gcs_core.{hpp,cpp}                    # + pass-through widening (Put(topics), QueryMessages)
src/lib/gcs_messaging.* (new, if extracted)   # OR: inline in gcs_core_ffi.cpp — dedupe set + receive bridge
src/ffi/gcs_core_ffi.cpp                      # send_text rewrite (publish+Put+pending/complete); join_topic replay; receive callback
src/proto/gcs_chat.proto                      # + sender/deleted/deleted_at_ms on ChatMessageState; + MessageHistory in GcsEvent
src/app/lib/cubits/message_flow_cubit.dart    # + upsert (by-id); keep append for compat
src/app/lib/cubits/session_cubit.dart         # + messageHistory dispatch arm
src/app/templates/ + src/app/lib/generated/   # regenerate if sender/pending/error chrome changes (never hand-edit)
test/                                          # + test_gcs_messaging.cpp (send/receive/history/dedupe)
src/app/test/                                  # + cubit upsert + messageHistory dispatch tests
```

### Pattern 1: Topics-aware Put overload (D-02)
**What:** Add an overload that accepts a topic set; the existing 2-arg `Put` delegates with `kNoTopics`. This is the fix at the wrapper, not a new storage class.
**When to use:** Every archive-replicating message write (send path). Receive path uses the 2-arg form (empty topics) as the "PutLocal" primitive.

### Pattern 2: Apply-once dedupe keyed by message id (D-03)
**What:** A bounded set of seen message ids. Both the pub/sub fast path and the CRDT sync callback funnel into one `OnMessageArrived(id, bytes)` that returns early if the id is seen, else records it, pushes to Dart, and `Put`s locally. The same message arrives twice by design (once via GossipSub, once via CRDT graphsync), so dedupe is load-bearing, not optional.
**When to use:** Every receive path. Retention is Claude's discretion (fixed-size LRU per room vs global keyed map).

### Pattern 3: Join-time batch replay + replaceAll (D-06)
**What:** Under the existing `g_mutex`, run the prefix scan, build a full sorted `MessageHistory`, push it as one `GcsEvent`; Dart does a full-list replace (mirrors `RailCubit.setRooms` and the generated `ChatMessageFlowCubit.replaceAll`). Live messages append after via the dedupe set.
**When to use:** `join_topic` arm AND `RefreshDerivedJoins` (derived joins from Phase 2). Push ordering matters: history batch first, then any live appends.

### Pattern 4: Append-only proto evolution (D-05)
**What:** `ChatMessageState` gains `sender` (next free tag) + `deleted`/`deleted_at_ms`; `GcsEvent` oneof gains `message_history = 6`. Never retype/reorder/remove. The generated Dart `gcs_chat.pb.dart` is regenerated (gitignored `generated/`), never hand-edited.

### Anti-Patterns to Avoid
- **Per-room id-manifest for message history:** read-union-write LWW lost-update under concurrency — rejected in D-01; use the prefix scan.
- **CRDT write-through as the only delivery path:** render latency ties to graphsync convergence — rejected in D-03.
- **Echo loops:** re-broadcasting a received message; receivers must `Put` with empty topics.
- **Dart-side sender/authority:** Dart supplying `sender`, `id`, or `timestamp` — C++ stamps all authority fields (D-04/D-27).
- **Hand-editing generated Dart:** `lib/generated/chat/*` and scaffold families are generated output — regenerate, never edit.
- **`sleep_for` in tests:** use `WaitForCondition` (`test/test_wait_condition.hpp`).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Message history enumeration | A per-room id manifest key | GlobalDB `QueryKeyValues` prefix scan | Manifest is a LWW lost-update hazard; the prefix scan already exists in SuperGenius (verified in NEO-SWARM planning + CONTEXT.md). |
| Live message delivery | A custom multicast/relay | GossipSub room topic (`AddBroadcastTopic`/`AddListenTopic` + publish) | Already wired in Phase 1; libp2p handles fan-out, dedupe-at-transport, and missed-delivery heuristics. |
| Archive replication | Hand-rolled merge protocol | GlobalDB `Put` with `{room_topic}` | CRDT per-key LWW already correct for immutable single-writer-per-key messages. |
| Message id uniqueness | Timestamp-only ids | `NextMessageId()` (seed + random + counter) | Bare counters collide across restarts (CR-01, documented in-repo). |
| Echo/duplicate suppression | Custom delivery tracking | Apply-once dedupe set keyed by message id | The pub/sub + CRDT double-delivery is by design (D-03); dedupe is the sanctioned convergence guard. |
| Pending/complete/error lifecycle | Auto-retry, queues, offline buffers | C++-pushed pending→complete/error + terminal error tint; manual re-send | Locked D-07 — no retry loop in MVP. |

**Key insight:** Every hard problem in this phase (convergent enumeration, live fan-out, replication, dedupe) already has a primitive in the vendored SuperGenius/libp2p stack. The phase is a wiring + contract-extension exercise, not a new-distributed-systems build.

## Common Pitfalls

### Pitfall 1: SuperGenius API signature drift (QueryKeyValues / RegisterNewElementCallback)
**What goes wrong:** Planning against a guessed callback typedef (`CRDTNewElementCallback` vs `GlobalDBNewElementCallback`) or a guessed `QueryKeyValues` return shape that does not match the prebuilt SDK.
**Why it happens:** The SuperGenius/GeniusSDK headers are prebuilt release tarballs pulled at CMake configure — not present in the local tree or git. The NEO-SWARM submodule's own plan explicitly warns "VERIFY the exact callback signatures in crdt_datastore.hpp … do not guess."
**How to avoid:** Add a Wave-0 verification task that locates `globaldb.hpp`/`crdt_datastore.hpp` in the resolved thirdparty build dir and confirms: `Put(HierarchicalKey, Buffer, topicSet)`, `QueryKeyValues(prefix)` return type, and the exact `RegisterNewElementCallback` typedef + bool return. Adapt signatures exactly.
**Warning signs:** Compile errors naming the callback typedef; a callback that never fires because the lambda signature mismatches the stored `std::function`.

### Pitfall 2: Echo loop (re-broadcasting received messages)
**What goes wrong:** A receiver re-publishes a received message, which re-arrives, gets re-published … unbounded amplification.
**Why it happens:** The send path publishes; if the receive path also publishes, every message fans out forever.
**How to avoid:** Receive path does `Put(key, value, /*empty topics*/)` only (the existing 2-arg `Put` with `kNoTopics` is exactly this primitive). Only the originating send path publishes + Puts-with-topics.
**Warning signs:** Duplicate messages multiplying in the archive; topic traffic growing without new sends.

### Pitfall 3: History/live ordering race (missing the join scan window)
**What goes wrong:** A message arrives via pub/sub between the prefix scan and the subscribe/apply-once activation, so it is neither in the batch nor deduped, and either duplicates or drops.
**Why it happens:** D-06 pushes history as a batch and then live messages append; the window between "scan" and "dedupe armed" is the race.
**How to avoid:** Run the scan under the session mutex and activate the apply-once set (or run the scan and register the callback atomically under `g_mutex`) so any pub/sub message that lands mid-join is either in the batch or absorbed by dedupe (D-03/D-06 design intent). Push the `MessageHistory` batch before any subsequent live message for that room.
**Warning signs:** Missing/duplicated messages right after a join; flaky history-equality tests at the join boundary.

### Pitfall 4: Ordering instability without a total sort
**What goes wrong:** Sorting by timestamp alone leaves ties unordered, so two peers disagree on message order (violates SC 3/4).
**Why it happens:** Wall-clock timestamps collide (same-ms sends).
**How to avoid:** Sort by `(timestamp, id)` — the id tiebreak makes the order total (D-01). Do not sort by id alone (ids are process-unique but not temporally ordered across peers).
**Warning signs:** Same-millisecond messages rendering in different orders across peers.

### Pitfall 5: Sender trust boundary (unsigned field)
**What goes wrong:** Treating `sender` as authenticated — any syncing node can forge a sender string on the wire.
**Why it happens:** D-04 locks unsigned MVP; signing is v1.1.
**How to avoid:** Render `sender` as display-only short-form (truncated wallet address). Do not gate any authorization on it this phase. Document the spoofing surface in the code Doxygen (security note).
**Warning signs:** Building access-control or moderation logic on the sender field (that's Phase 4/5 territory and needs signed identity).

### Pitfall 6: Generated-Dart drift / scaffold read-only violations
**What goes wrong:** Hand-editing `lib/generated/chat/*` or scaffold `lib/components/*`, causing `dart analyze --fatal-infos` failures or silent overwrite.
**Why it happens:** The generated flow cubit (`chat_message_flow_cubit.dart`) and scaffold families are Jinja2 output; the scaffold is a read-only public contract.
**How to avoid:** App-side changes (by-id upsert, sender/pending/error rendering) live in `src/app/lib/cubits/` + app-owned templates under `src/app/templates/`; regenerate and never hand-edit generated output.
**Warning signs:** Edits inside `generated/`; `dart analyze` fatal-infos on a modified generated file.

## Code Examples

Patterns anchored in verified in-repo code (file:line references checked this session). SuperGenius API snippets are marked `[ASSUMED signature]` where the prebuilt header is not local.

### Topics-aware Put overload (D-02 — fix site is `gcs_global_db.cpp:256-271`)
```cpp
// Existing 2-arg form becomes a delegate (verified: current body hardcodes kNoTopics).
outcome::result<void> GcsGlobalDb::Put(const std::string &key, const std::string &value) {
  return Put(key, value, /*topics=*/{}); // empty set = local-only (the "PutLocal" primitive)
}

// New topics-aware overload (D-02): passes the room topic through to the
// 3-arg GlobalDB::Put already in use (verified call at gcs_global_db.cpp:266).
outcome::result<void> GcsGlobalDb::Put(const std::string &key, const std::string &value,
                                       const std::unordered_set<std::string> &topics) {
  if (!m_running.load()) {
    return outcome::failure(Error::GcsDbError);
  }
  crdt::HierarchicalKey keyTyped{key};
  crdt::GlobalDB::Buffer valueTyped;
  valueTyped.put(value);
  auto result = m_db->Put(keyTyped, valueTyped, topics);
  if (result.has_error()) {
    return outcome::failure(Error::GcsDbError);
  }
  return outcome::success();
}
```

### Send path rewrite (D-03/D-07 — current arm at `gcs_core_ffi.cpp:667-720`)
```cpp
// After the existing validation + id mint + authority stamping:
// 1. push pending echo (state = MESSAGE_STATE_PENDING) to Dart.
// 2. publish the serialized ChatMessageState on sendText.room_topic() (live path).
// 3. Put under "gcs/messages/<room_topic>/<id>" with {room_topic} (archive, D-02).
// 4. push complete echo (state = MESSAGE_STATE_COMPLETE, same id) to Dart.
// Failure on publish or Put pushes an error-state ChatMessageState for that id (D-07).
// Key generalizes the Phase 1 precedent at gcs_core_ffi.cpp:709:
//   room_topic + "/" + message->id()  →  "gcs/messages/" + room_topic + "/" + message->id()
```

### Receive bridge (D-03) — `[ASSUMED signature]`
```cpp
// SuperGenius receive-side push hook (verified name in NEO-SWARM 03-02-PLAN.md:120;
// exact typedef to re-verify against crdt_datastore.hpp). Fires on the io thread —
// lock g_mutex before touching session state.
//   m_db->RegisterNewElementCallback(prefix, [](key, value) { ... });
// Inside the callback: decode ChatMessageState, if !seen(id) { seen.insert(id);
//   PostToDart(message); g_session->Put(key, value, {}); }  // empty topics = no echo
```

### Append-only proto additions (D-04/D-05/D-06)
```proto
// ChatMessageState (gcs_chat.proto:67) gains (next free field tags):
//   string sender = 7;       // wallet address, stamped by C++ (D-04)
//   bool   deleted = 8;      // tombstone fields, present from creation (D-05)
//   int64  deleted_at_ms = 9;
// GcsEvent oneof (gcs_chat.proto:92) gains:
//   MessageHistory message_history = 6;  // batch history replay (D-06)
//   message MessageHistory { string room_topic = 1; repeated ChatMessageState message = 2; }
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Phase 1: send = local Put (kNoTopics) + local echo; no pub/sub, no receive | GossipSub fast path + topics-aware Put archive + apply-once receive (D-03) | Phase 3 (this phase) | Messages now reach peers live and converge via CRDT. |
| Phase 1: no history read-back | `QueryKeyValues` converged prefix scan + `MessageHistory` batch (D-01/D-06) | Phase 3 | Join-time replay of full room history. |
| Phase 2: per-entity manifest as the only enumeration | Message history uses prefix scan (manifest pattern rejected for messages) | Phase 3 | No LWW lost-update hazard at message rates. |
| Append-only message records (id/role/state/text/timestamp) | Add `sender` + tombstone fields (D-04/D-05) | Phase 3 | Sender identity + forward-compatible moderation (Phase 5). |

**Deprecated/outdated:**
- The Phase 1 per-record key `room_topic + "/" + message_id` (`gcs_core_ffi.cpp:709`) is generalized to `gcs/messages/<room_topic>/<id>` for a stable scan prefix (D-01).
- The Phase 1 "local echo only" send path is replaced, not extended — the store-write semantics change from local-only to replicating.
- Phase 2's manifest enumeration remains valid for entities (spaces/rooms) — it is NOT deprecated; only its application to messages is rejected.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | `QueryKeyValues(prefix)` exact signature (return type `outcome::result<QueryResult>` per NNS 03-02-PLAN.md:117) and `RegisterNewElementCallback` typedef match the prebuilt SuperGenius SDK | Standard Stack / Code Examples | Compile-time mismatch; callback never fires. Mitigated by Wave-0 header-verification task. |
| A2 | The 3-arg `GlobalDB::Put(key, value, topicSet)` is the same call the topics-aware overload will use (already proven locally at `gcs_global_db.cpp:266`, so HIGH, not assumed) | Standard Stack | n/a — verified in-repo. |
| A3 | `GeniusNode::GetAddress()` is the correct API for the local wallet address (D-04) | Summary / Security | Wrong sender string. Verify against `GeniusSDK.hpp` at plan time (header not local). |
| A4 | Prefix `gcs/messages/` matches `QueryKeyValues` prefix-matching semantics (trailing-slash vs not) | Standard Stack | Wrong prefix returns empty/over-broad scans. NNS 03-02-PLAN.md:171 flags the same verification need. |
| A5 | `RegisterNewElementCallback` fires on the io thread (CONTEXT.md claim) | Architecture | Mutex discipline wrong; data race. Verify in `crdt_datastore.hpp`. |

**Assumptions A1/A3/A4/A5 all reduce to one environment fact:** the SuperGenius/GeniusSDK headers are prebuilt tarballs not present locally, so exact signatures must be re-verified against the resolved thirdparty build dir at plan/implement time. All are flagged MEDIUM confidence; none are presented as verified fact.

## Open Questions

1. **SuperGenius header resolution at build time**
   - What we know: `gcs_global_db.cpp` includes `crdt/globaldb/globaldb.hpp`, `GeniusSDK.hpp`, `ipfs_pubsub/gossip_pubsub.hpp`; these resolve from the thirdparty prebuilt (not in local tree/git).
   - What's unclear: the exact resolved path after CMake configure on the dev machine and the exact API signatures.
   - Recommendation: Wave-0 task greps the resolved headers for `QueryKeyValues`, `RegisterNewElementCallback`, `GetAddress`, `Put(` and records exact signatures before any implementation.

2. **Receive-side callback threading model**
   - What we know: `RegisterNewElementCallback` exists; CONTEXT.md states it fires on the io thread (requires `g_mutex` when bridging into FFI session state).
   - What's unclear: whether the pub/sub fast path also delivers on the io thread or a separate pubsub reactor thread, and how the two receive paths serialize.
   - Recommendation: single funnel function `OnMessageArrived` that takes `g_mutex`; both callback paths call it. Verify thread ownership in the Wave-0 header read.

3. **Dedupe-set scope and retention (Claude's discretion)**
   - What we know: apply-once keyed by message id; fixed-size LRU per room vs global keyed map, retention unspecified.
   - What's unclear: the eviction bound and whether it is per-room or global.
   - Recommendation: a global `std::unordered_set<std::string>` keyed by id with a per-room clear on join is the simplest correct MVP; planner may pick an LRU cap constant (`constexpr`).

4. **Sender truncation format**
   - What we know: UI renders sender short-form (D-04); scaffold has no built-in address formatter.
   - What's unclear: exact truncation (e.g. `0x1234…abcd` — first/last N chars).
   - Recommendation: app-side helper with a `constexpr` char budget; render the raw string as-is if under budget.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| SuperGenius GlobalDB + GeniusSDK headers | QueryKeyValues / RegisterNewElementCallback / GetAddress verification, C++ build | ✗ (not in local tree — prebuilt, pulled at configure) | thirdparty release | Verify signatures from NEO-SWARM planning artifacts now; add Wave-0 header-verification task at implement time |
| cmake / ninja | C++ build | ✓ | 3.29.2 / 1.13.2 | — |
| C++ compiler | C++ build | ✓ | Apple clang 17 (g++/clang++) | — |
| protoc | proto regeneration | ✗ (system) | — | thirdparty protoc via `add_proto_library` (already wired — Phase 1 D-07); no system protoc needed |
| Flutter / Dart | app build + Dart tests | ✓ | 3.41.9 / 3.11.5 | — |
| GTest | C++ tests | ✓ (thirdparty, `find_package`/manual path in `test/CMakeLists.txt`) | — | — |
| GeniusSDK node (runtime) | end-to-end send/receive tests | ✓ (boots embedded via `gcs_init`; or injected pubsub seam in unit tests) | — | injected port-0 GossipPubSub fixture (Tier 2 seam, already used) |

**Missing dependencies with no fallback:**
- SuperGenius/GeniusSDK headers (local). This does not block planning (the API names are confirmed in-repo), but the exact signatures are unverifiable until the thirdparty build resolves them. The planner must include a Wave-0 signature-verification task.

**Missing dependencies with fallback:**
- System `protoc` — not needed; the thirdparty protobuf toolchain is already wired.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | GTest (C++, `test/` tree) + flutter_test (Dart, `src/app/test/`) |
| Config file | `test/CMakeLists.txt` (gcs_test macro + ctest registration); Dart has none (flutter_test default) |
| Quick run command | C++: `cd build/OSX/Debug && ctest -R test_gcs_messaging --output-on-failure` |
| Full suite command | C++: `cd build/OSX/Debug && ctest --output-on-failure`; Dart: `cd src/app && flutter test` |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| CORE-04 SC1 | send_text pushes pending then complete with same id; local archive write | unit (FFI + injected pubsub) | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 SC2 | peer session receives message without refresh (pub/sub fast path) | integration (two sessions, shared GossipSub) | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 SC3 | two sessions' history converges (QueryKeyValues prefix scan equal after sync) | integration | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 SC4 | history batch sorted `(timestamp, id)`; sender field stamped | unit | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 (D-07) | MessageFlowCubit upsert replaces pending→complete by id, no duplicate | unit (Dart) | `cd src/app && flutter test test/cubits/shell_cubits_test.dart` | ❌ extend existing |
| CORE-04 (D-06) | SessionCubit dispatches messageHistory → replaceAll | unit (Dart) | `cd src/app && flutter test test/cubits/shell_cubits_test.dart` | ❌ extend existing |
| CORE-04 (D-02) | topics-aware Put passes `{room_topic}`; 2-arg Put delegates empty | unit (C++) | `ctest -R test_gcs_storage` | ❌ extend existing |

### Sampling Rate
- **Per task commit:** `cd build/OSX/Debug && ninja gcs_core gcs_ffi && ctest -R test_gcs_messaging --output-on-failure` (C++), `cd src/app && flutter test test/cubits/` (Dart)
- **Per wave merge:** full `ctest` + `flutter test`
- **Phase gate:** full C++ + Dart suites green, plus `dart analyze --fatal-infos` clean, before `/gsd:verify-work`

### Wave 0 Gaps
- [ ] `test/test_gcs_messaging.cpp` — CORE-04 send/receive/history/dedupe (new test target `test_gcs_messaging`, links `gcs_core;gcs_storage;neoswarm_common`)
- [ ] `test_gcs_storage.cpp` — extend for topics-aware Put + QueryKeyValues prefix scan
- [ ] `src/app/test/cubits/shell_cubits_test.dart` — extend for upsert + messageHistory dispatch
- [ ] SuperGenius header signature verification task (Environment Availability A1/A3/A4/A5)

*(Existing infra: `test_wait_condition.hpp` wait-condition template; Tier-2 injected pubsub fixture in `test_gcs_core_smoke.cpp`/`test_gcs_entities.cpp`; Dart cubit tests in `src/app/test/cubits/shell_cubits_test.dart`.)*

## Security Domain

### Applicable ASVS Categories (ASVS Level 1)

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | No (deferred) | Sender identity = wallet address is display-only; signing deferred to v1.1 (D-04) |
| V3 Session Management | No | P2P — no server-side sessions; local FFI handle lifecycle already guarded |
| V4 Access Control | No (Phase 4) | No membership/roles this phase; room topic is the only gate |
| V5 Input Validation | **Yes** | FFI validates: room_topic non-empty + ≤`kMaxTopicLength`(128), text ≤`kMaxMessageTextLength`(4096), UTF-8 code-point caps on names; protobuf parse from untrusted bytes never reaches partial state |
| V6 Cryptography | No (v1.1) | No encryption this phase; content is opaque text field, schema already accommodates bytes-ish payloads |

### Known Threat Patterns for this stack

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Forged sender on the wire (unsigned `sender` field) | Spoofing | Accepted MVP limitation (D-04); render sender as display-only; do NOT authorize on it; document in Doxygen. v1.1 signing. |
| Unbounded message growth / oversized publishes | DoS | `kMaxMessageTextLength` 4096, `kMaxTopicLength` 128, client-side `kMaxFlowItems` cap (drop oldest). |
| Malformed/corrupt CRDT records (hostile or partial writes) | Tampering | Per-entry skip-and-log during scan (EntityStore `LoadFromStore` precedent + NNS QueryKeyValues fault-tolerance pattern); one bad record never hides the rest or aborts. |
| Echo loops / message amplification | DoS | Apply-once dedupe by id + receive path `Put` with empty topics (no re-broadcast, D-03). |
| Spoofed message injection into a room topic | Spoofing | C++ stamps id/timestamp/sender on SEND; receivers decode-and-dedupe. Full sender authentication is deferred (unsigned MVP). |

## Sources

### Primary (HIGH confidence — verified in-repo this session)
- `src/lib/gcs_storage/gcs_global_db.cpp` — `Put` hardcodes `kNoTopics` at lines 264-266 (D-02 fix site); 3-arg `GlobalDB::Put(key, value, topicSet)` call proven local; `AddListenTopic`/`AddBroadcastTopic` wrap GlobalDB (lines 234-254).
- `src/ffi/gcs_core_ffi.cpp` — `send_text` arm (667-720, Phase 1 local-echo only), `NextMessageId()` (449-458), per-record key `room_topic + "/" + id` (709), `join_topic` arm (621-666), `RefreshDerivedJoins` (374-421), globals + caps (44-100), `PostToDart` (294-312).
- `src/lib/gcs_core.{hpp,cpp}` — `CoreSession` thin pass-through of Put/Get/AddListenTopic/AddBroadcastTopic (needs the same widening).
- `src/lib/gcs_entity_store.hpp` — Phase 2 manifest pattern (why it is NOT reused for messages).
- `src/proto/gcs_chat.proto` — current `ChatMessageState` (67-74), `GcsEvent` oneof (92-100), append-only Phase 2 additions.
- `src/app/lib/cubits/message_flow_cubit.dart` — append-only (47-49); `composer_cubit.dart` — converged submit path (unchanged); `session_cubit.dart` — `_dispatchEvent` (364-387, needs messageHistory arm); `rail_cubit.dart` — `setRooms` full-replacement precedent.
- `src/app/lib/generated/chat/chat_message_flow_cubit.dart` — `replaceAll` + `cappedItems` already exist (generated).
- `src/app/pubspec.yaml` + `pubspec.lock` — flutter_bloc 9.1.1, protobuf 4.2.0, ffi 2.2.0, fixnum 1.1.1, frontend_scaffold (path).
- `test/CMakeLists.txt` + `test/test_wait_condition.hpp` — test targets + wait-condition template.
- `src/app/scaffold/CLAUDE.md` — read-only lib/ contract.
- `.planning/workstreams/app/phases/03-messaging/03-CONTEXT.md` — locked decisions D-01..D-07 (the authoritative user decisions).

### Secondary (MEDIUM confidence — cross-referenced in-repo, source not local)
- `GNUS-NEO-SWARM/.planning/workstreams/neoswarm/phases/03-gcs-globaldb-integration/03-02-PLAN.md` — documents `QueryKeyValues` signature (line 117), `RegisterNewElementCallback` (line 120), const-ness of Get/QueryKeyValues (line 174), prefix-matching caveat (line 171), and the "verify exact callback typedefs — do not guess" warning (line 236).
- `GNUS-NEO-SWARM/.planning/workstreams/neoswarm/phases/03-gcs-globaldb-integration/03-CONTEXT.md` + `ARCHIVED.md` — SuperGenius `globaldb.hpp` API surface (Put/Get/Remove/QueryKeyValues/BeginTransaction/topics/callbacks).
- `.planning/notes/gcs-chat-architecture.md` — Message Flow section, room topic model.

### Tertiary (LOW confidence — not locally verifiable, flagged in Assumptions Log)
- Exact SuperGenius signatures (`QueryKeyValues` return type, `RegisterNewElementCallback`/`CRDTNewElementCallback` typedefs, `GeniusNode::GetAddress`, prefix-match semantics) — prebuilt SDK headers absent from local tree; all marked `[ASSUMED]` (A1/A3/A4/A5).

## Metadata

**Confidence breakdown:**
- Standard stack: **HIGH** — no new packages; all deps verified in-repo (pubspec.lock, CMakeLists, moved gcs_global_db.cpp proving the 3-arg Put).
- Architecture: **HIGH** — decisions map directly onto verified in-repo code (send arm, Put site, manifest pattern, generated replaceAll, dispatch switch).
- Pitfalls: **MEDIUM** — six pitfalls grounded in code + NNS artifacts; the SuperGenius signature-drift pitfall depends on a non-local header (flagged).
- SuperGenius API signatures: **MEDIUM** — confirmed by name in two independent in-repo sources, but exact typedefs/signatures unverifiable until the prebuilt SDK resolves.

**Research date:** 2026-09-23
**Valid until:** 2026-10-07 (stable stack; the only fast-moving element is the thirdparty SuperGenius release, which is pinned per CI)
