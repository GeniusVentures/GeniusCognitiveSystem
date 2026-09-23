# Phase 3: Messaging - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-09-22
**Phase:** 3-Messaging
**Areas discussed:** CRDT schema & ordering, Delivery & sender identity, History loading, Send lifecycle UX

---

## CRDT Schema & Ordering

| Option | Description | Selected |
|--------|-------------|----------|
| Prefix scan (via `QueryKeyValues`) | Per-message keys `room_topic + "/" + message-id`; GlobalDB's converged prefix scan enumerates; sort by `(timestamp, id)` | ✓ |
| Per-room index manifest | Phase 2's entity-manifest pattern applied to message ids under one key | |
| Space-blob / embedded history | Messages embedded in a room record | |

**User's choice:** Prefix scan (Recommended)
**Notes:** Advisor research verified `QueryKeyValues` exists in SuperGenius `globaldb.hpp` (~line 144). The manifest alternative has a read-union-write LWW lost-update convergence failure under concurrent sends (per-key LWW, no union semantics) — dropped. `GcsGlobalDb`/`CoreSession` must expose the scan. Record shape: append `sender` + tombstone fields to `ChatMessageState`; tombstones present from creation per Phase 2 D-03.

## Delivery & Sender Identity

| Option | Description | Selected |
|--------|-------------|----------|
| GossipSub fast-path + CRDT archive | Publish serialized message on room topic for live delivery AND Put with `{room_topic}` for archive replication; receivers apply-once by id (dedupe), render, `PutLocal` without re-broadcast | ✓ |
| CRDT write-through only | Messages render only after CRDT/graphsync convergence | |

**User's choice:** GossipSub fast-path + CRDT archive
**Notes:** User's verbatim rationale: "GossipSub fast-path seems better as then the CRDT is just the archive of that message." Enables the `kNoTopics` → `{room_topic}` fix in `GcsGlobalDb::Put` (`gcs_global_db.cpp` ~264-266). Sender identity = wallet address via `GeniusNode::GetAddress()`, stamped by C++, unsigned MVP (signing deferred to v1.1 with encryption). Dedupe set absorbs the double-arrival (pub/sub + CRDT sync) of the same message id.

## History Loading

| Option | Description | Selected |
|--------|-------------|----------|
| Join-time replay | Prefix scan at join → one `MessageHistory` batch event → Dart `replaceAll` | ✓ |
| Lazy on-scroll | Load windowed pages as the user scrolls up | |
| Startup full load | Load all rooms' history at app start | |

**User's choice:** Join-time replay (Recommended)
**Notes:** Scan runs under the session mutex on `join_topic` and Phase 2 derived joins. Live messages append after the batch; apply-once dedupe absorbs scan/subscribe races. No pagination API for MVP.

## Send Lifecycle UX

| Option | Description | Selected |
|--------|-------------|----------|
| Pending echo + upsert | C++ pushes pending at send-accept then complete/error with the same id; Cubit upserts by id; error is terminal tint + manual re-send | ✓ |
| Optimistic Dart insert | Dart inserts locally before C++ acknowledges | |
| Send-then-render | No local echo; message appears only after round-trip | |

**User's choice:** Pending echo + upsert (Recommended)
**Notes:** Preserves C++-owns-state (no Dart optimism). ~10-line `MessageFlowCubit` change from append-only to by-id upsert. Transport errors additionally `showToast`; no auto-retry. Composer unchanged — Enter/button already converge on one submit path.

---

## Claude's Discretion

- Proto field names/numbering (`sender`, tombstone fields, `MessageHistory` shape).
- Dedupe-set implementation and retention policy.
- Exact prefix string layout details beyond one stable per-room prefix.
- `QueryKeyValues` exposure shape on `GcsGlobalDb` (raw passthrough vs decoded helper).
- Pending/complete/error representation in `ChatMessageState`.
- Dart `replaceAll` wiring and client-side render cap constant.

## Deferred Ideas

- Message signing + sender verification (v1.1, with encryption).
- Reply threading / Lamport-HLC / vector-clock causal ordering (bot phase).
- Message deletion UI + moderation flows (Phase 5; tombstone fields land now).
- Pagination / windowed history (post-MVP).
- Auto-retry with backoff (rejected for MVP; manual re-send only).
- @mentions / GCS bot auto-answer (Phase 5+).
