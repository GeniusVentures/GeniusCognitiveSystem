---
phase: 03-messaging
plan: 06
subsystem: messaging
tags: [integration-test, two-node, gossip-sub, crdt, at-rest-encryption, restart-replay]
requires: [03-05, 03-03]
provides: [two-node-messaging-integration-coverage]
affects: [src/lib/gcs_messaging, src/lib/gcs_storage/gcs_global_db, src/ffi/gcs_core_ffi]
tech-stack:
  added: []
  patterns: [supergenius-multinode-fixture, injected-pubsub-seam, wait-condition-template, real-crypto-seam]
key-files:
  created: [test/test_gcs_messaging_multinode.cpp]
  modified: [test/CMakeLists.txt]
decisions: []
metrics:
  duration: "28min"
  completed_date: "2026-09-23"
---

# Phase 3 Plan 6: Two-node messaging integration test Summary

Two-node in-process integration test (`MessagingMultinodeTest`) proving live cross-node
delivery, history convergence, at-rest ciphertext opacity, and restart replay over the real
GossipSub transport with the real `gcs::crypto` seam injected — written, compiling, and
registered, but **currently RED** pending a production threading fix (see "Blocking Issue").

## What Was Built

`test/test_gcs_messaging_multinode.cpp` — a `MessagingMultinodeTest` fixture following the
`../SuperGenius` `globaldb_integration.cpp` multinode pattern:

- Per-node bring-up: `KeyPairFileStorage` + `GossipPubSub` (single-arg ctor, DEFAULT config so
  `echo_forward_mode` is FALSE — delivery is genuinely cross-node), `MakeGraphsyncContext`,
  `CoreSession::Initialize(pubsub, graphsync.network)` (injected seam, no GeniusSDK).
- FFI join wiring mirrored per node: `AddListenTopic`/`AddBroadcastTopic` (CRDT archive),
  `Subscribe(room, -> OnLiveMessage)` (live path), `RegisterNewElementCallback(kMessagesKeyPrefix,
  -> OnMessageArrived)` (heal path), and a `Messaging` with the REAL `CryptoSeam`
  (`&gcs::crypto::EncryptPayload` / `&gcs::crypto::DecryptPayload`, `enabled = true`).
- Cross-connect via `AddPeers({ other->GetInterfaceAddress() })` + `WaitForCondition` on both
  hosts' connection-manager size.
- Three tests: `LiveDeliveryAndHistoryConvergeAcrossNodes`, `AtRestOpacityBothSides`,
  `HistoryReplaysAfterRestart`. All waits use `WaitForCondition` (no `sleep_for`).

`test/CMakeLists.txt` — registered the `test_gcs_messaging_multinode` target (own binary,
`gcs_core;gcs_storage;neoswarm_common`), mirroring `test_gcs_messaging`.

## Acceptance Criteria Status

| Criterion | Result |
|-----------|--------|
| `ctest -R test_gcs_messaging_multinode` green | **FAIL** — 3/3 tests hang on the graphsync CRDT heal (see below) |
| `grep -c sleep_for` == 0 | PASS (0) |
| `grep -c EncryptPayload` >= 1 | PASS (2) |
| `grep -c AddPeers` >= 1 | PASS (3) |
| `grep -c echo_forward_mode` == 0 | Code-review check — 2 comment-only mentions; the single-arg default ctor is used (echo OFF) |
| `grep -c "Initialize( "` >= 2 | Code-review check — 1 (single shared `StartSession` helper initializes BOTH nodes via the injected seam; no GeniusSDK `Initialize()`) |
| Existing suites unregressed | PASS — `test_gcs_messaging`, `test_gcs_crypto`, `test_gcs_ffi` all green |

## Blocking Issue (Task 1 not complete)

The test exposes a **production threading deadlock** in the messaging receive path that was
never reachable from the single-node unit suites:

1. `Messaging::ApplyMessage` runs on the GossipSub **strand thread** (live `Subscribe`
   callback) and performs a **synchronous** archive write (`m_session.Put(key, value)` ->
   `CrdtDatastore::PutKey` -> `AddDAGNode` + `WaitForJob`), blocking the strand.
2. The CRDT archive broadcast (`Put` with topics, from `SendMessage`) makes the peer fetch the
   message CID via **graphsync**, whose response handling also runs on the **same strand**.
3. The strand blocks on the archive write, the datastore blocks on the graphsync response, the
   graphsync response needs the strand — circular deadlock. The archive write then fails only at
   session shutdown ("failed to archive received message"), so `QueryHistory` never converges and
   the reverse send's raw `Publish` times out ("Timed out waiting for publish ... stays queued on
   the strand").

Evidence: `GraphsyncDAGSyncer: We exited while trying to sync ... still in progress` and
`CrdtDatastore WaitForJob: Aborting wait for CID ... due to datastore shutdown` at teardown, and
a connection-only variant (no messaging) that passes in 238 ms — confirming the hang is triggered
by the message archive broadcast, not by the connection itself.

## Deviations from Plan

### Auto-fixed Issues

None — three candidate fixes were attempted and reverted because they did not resolve the
deadlock (see "Attempted fixes" below). The blocking defect is pre-existing production code, not
caused by this plan's changes.

### Attempted fixes (reverted — out of scope / ineffective)

1. **[Rule 3] Test graphsync Network scheduler on `pubsub->GetAsioContext()`** — aligned the
   test's `Network` scheduler with the SuperGenius pattern. Did not unblock; reverted to the
   stock `MakeGraphsyncContext` per the plan.
2. **[Rule 1] `GcsGlobalDb::Initialize` `m_scheduler` on `pubsub->GetAsioContext()`** — a
   genuine latent fix (the SuperGenius comment mandates the graphsync scheduler share the pubsub
   host's io context, not a private io), but it is production code outside this plan's
   `files_modified` and did not resolve the strand/deadlock. Reverted.
3. **[Rule 1] Disable `AddListenTopic` on both nodes** — expected to avoid the CID fetch, but the
   graphsync fetch still fired (the `Put`-with-topics broadcast is received independently of the
   listen registration). Reverted.

## Deferred Items

1. **Production threading deadlock (blocker).** The receive-path archive write must not block the
   pubsub strand (make it async, or move the graphsync response handling off the strand). This is
   an architectural change to `src/lib/gcs_messaging.cpp` (and possibly `gcs_global_db.cpp`) and
   needs a design decision before the multinode test can go green.
2. **CRDT receive callback pattern does not match.** `RegisterNewElementCallback(kMessagesKeyPrefix
   = "gcs/messages/", ...)` is matched via `std::regex_match` (full match) against keys like
   `/gcs/messages/<topic>/<id>`, so the callback never fires ("No callbacks were triggered for key
   ... no pattern matches found"). The heal path is therefore inert in both this test and the FFI
   production wiring (mirrored faithfully). Live delivery masks it; a `.*gcs/messages/.*`-style
   pattern would be needed for the heal path.
3. **`GcsGlobalDb::Initialize` graphsync scheduler on private `m_io`.** Per SuperGenius, the
   graphsync scheduler backend should share the pubsub host's io context to avoid the cross-thread
   `WriteQueue` race. Worth applying alongside the deadlock fix.

## Task 2 (Optional human visual smoke)

**skipped-non-gating** — per the plan, recorded as skipped with the automated-coverage pointer.
The SC4 visual chrome (pending 40% opacity -> solid, peer bubbles, sender labels, error toast,
restart history order) is covered by the flutter suite (`flutter_test` 26/26) and the (blocked)
Task 1 multinode suite; no human smoke was performed this session.

## Threat Flags

| Flag | File | Description |
|------|------|-------------|
| threat_flag: thread-deadlock | src/lib/gcs_messaging.cpp (ApplyMessage) | Synchronous archive write on the GossipSub strand deadlocks with the graphsync CRDT heal response (also on the strand) — DoS-by-deadlock on any two-node messaging exchange. |

## Self-Check

- `test/test_gcs_messaging_multinode.cpp` — committed (2515af3).
- `test/CMakeLists.txt` — committed (2515af3).
- `test_gcs_messaging_multinode` binary builds via `ninja` — PASS.
- Existing suites `test_gcs_messaging`, `test_gcs_crypto`, `test_gcs_ffi` — PASS (no regression).
- `test_gcs_messaging_multinode` — **RED** (documented blocker above).
