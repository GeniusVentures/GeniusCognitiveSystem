---
phase: 03-messaging
plan: 02
subsystem: storage
tags: [crdt, gossipsub, globaldb, querykeyvalues, putlocal, pubsub, topics-aware-put, cpp17]

# Dependency graph
requires:
  - phase: 03-01
    provides: resolved SuperGenius/libp2p/GossipSub/OpenSSL signatures + proto contract (03-API-SIGNATURES.md)
provides:
  - Widened GcsGlobalDb storage surface (topics-aware Put, PutLocal, QueryKeyValues, RegisterNewElementCallback, raw Publish/Subscribe) behind std::string signatures
  - CoreSession pass-throughs for the six widened operations
  - Storage-layer tests proving prefix scan, topic-write, PutLocal overwrite, and raw pubsub surface
affects: [03-messaging (03-04 messaging + crypto, 03-05 FFI wiring, 03-06 integration)]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "std::string wrapper hides SuperGenius/libp2p types (Buffer/CID/Gossip::Message/callback typedefs) from messaging/FFI layers"
    - "QueryKeyValues is a raw passthrough — returned keys are datastore-internal (/crdt/k/<key>/v), NOT logical keys; 03-04 must parse the message id from the raw key"
    - "Shared GossipPubSub handle retained (m_pubsub) backs both CRDT broadcast and raw live path (D-03)"

key-files:
  created: []
  modified:
    - src/lib/gcs_storage/gcs_global_db.hpp
    - src/lib/gcs_storage/gcs_global_db.cpp
    - src/lib/gcs_core.hpp
    - src/lib/gcs_core.cpp
    - test/test_gcs_global_db.cpp

key-decisions:
  - "QueryKeyValues returns raw datastore keys (/crdt/k/<key>/v); wrapper is a blind std::string passthrough per threat T-03-03 — 03-04 parses the message id from the raw key"
  - "PutLocal (SuperGenius PutKeyLocal) is overwrite-only: it requires the key's priority record to already exist, so a fresh-key PutLocal is rejected — 03-04 receive-path archive write must account for this"
  - "Retain m_pubsub before std::move into GlobalDB::New, and reset it in Shutdown() plus both Initialize failure paths"

patterns-established:
  - "Storage wrapper converts Buffer->std::string via std::string{buf.toString()} (size-explicit, binary-safe for D-08 envelope NUL bytes)"
  - "Subscribe adapter drops the empty EOS optional, forwards (msg.topic, std::string(msg.data)), and blocks on the returned future"

requirements-completed: [CORE-04]

# Metrics
duration: 12min
completed: 2026-09-23
---

# Phase 3 Plan 2: Storage Widening Summary

**Widened the GCS storage layer to six messaging operations — topics-aware Put (D-02), PutLocal, QueryKeyValues prefix scan (D-01), RegisterNewElementCallback, and raw GossipSub Publish/Subscribe (D-03) — all behind std::string signatures, with green storage tests.**

## Performance

- **Duration:** 12 min
- **Started:** 2026-09-23T21:40:00Z
- **Completed:** 2026-09-23T21:52:14Z
- **Tasks:** 2
- **Files modified:** 5

## Accomplishments

- `GcsGlobalDb` now exposes the six Phase 3 storage operations with `std::string`-based signatures, keeping all SuperGenius/libp2p types (`Buffer`, `CID`, `Gossip::Message`, callback typedefs) hidden behind the wrapper.
- The shared `GossipPubSub` is retained (`m_pubsub`) before the `std::move` into `GlobalDB::New`, so the raw live `Publish`/`Subscribe` path and the CRDT broadcast share one handle (D-03).
- `CoreSession` exposes all six operations as one-line pass-throughs for the FFI/messaging layers (03-04/03-05).
- Storage-layer tests cover topics-aware Put, the 2-arg Put, PutLocal, prefix-scan isolation (roomA vs roomB), and raw Publish/Subscribe success — all green, no `sleep_for`.

## Task Commits

1. **Task 1: Widen GcsGlobalDb + CoreSession** - `9052669` (feat)
2. **Task 2: Extend test_gcs_global_db.cpp** - `c3e7235` (test)

## Files Created/Modified

- `src/lib/gcs_storage/gcs_global_db.hpp` - Added six method declarations + `m_pubsub` member; widened includes
- `src/lib/gcs_storage/gcs_global_db.cpp` - Retained pubsub, implemented the six operations, delegated 2-arg Put
- `src/lib/gcs_core.hpp` - Added six pass-through declarations
- `src/lib/gcs_core.cpp` - Added six pass-through implementations
- `test/test_gcs_global_db.cpp` - Added `WidenedStorageSurfaceRoundTrips` test

## Decisions Made

- `QueryKeyValues` is exposed as a raw passthrough returning datastore-internal keys (per threat T-03-03 "raw key/value bytes only"); the logical message id lives inside the returned key and is 03-04's job to parse.
- `PutLocal` is tested as an overwrite of an existing key — its actual SuperGenius contract — rather than a fresh-key write.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Corrected QueryKeyValues key-format expectation in the test**
- **Found during:** Task 2 (test execution)
- **Issue:** The plan's Test 2 assumed `QueryKeyValues("gcs/messages/roomA/")` returns the logical keys (`gcs/messages/roomA/msg-1`). In reality SuperGenius `QueryKeyValues` returns raw datastore-internal keys (`/crdt/k/gcs/messages/roomA/msg-1/v`), so the planned assertion threw `std::out_of_range`.
- **Fix:** Rewrote the test to assert on entry count, values, and key-fragment matching (embedded message id) instead of logical keys. The wrapper itself is unchanged — it is a blind `std::string` passthrough as the plan specified.
- **Files modified:** test/test_gcs_global_db.cpp
- **Verification:** `ctest -R test_gcs_storage` green (6/6).
- **Committed in:** c3e7235

**2. [Rule 1 - Bug] Corrected PutLocal fresh-key assumption in the test**
- **Found during:** Task 2 (test execution)
- **Issue:** The plan's Test 3 wrote `PutLocal(".../msg-3", payload, "msg-3")` on a fresh key. SuperGenius `PutKeyLocal` is overwrite-only: it sets priority to the existing key's priority, and the equal-priority branch reads the existing value — for a fresh key that read is `NOT_FOUND` and the write fails.
- **Fix:** Changed the test to exercise `PutLocal` as an overwrite of an existing key (msg-1), which is its actual contract. Flagged below for 03-04: the D-03 receive-path "PutLocal into the archive with no re-broadcast" on a fresh message key needs re-examination (the wrapper is correct; the design assumption about fresh-key local writes was not).
- **Files modified:** test/test_gcs_global_db.cpp
- **Verification:** `ctest -R test_gcs_storage` green; overwrite reflected in a subsequent scan.
- **Committed in:** c3e7235

**3. [Rule 2 - Missing Critical] Added `m_pubsub.reset()` on Initialize failure paths**
- **Found during:** Task 1
- **Issue:** The plan specified `m_pubsub.reset()` only in `Shutdown()`. If `GlobalDB::New` or `AddBroadcastTopic` failed after `m_pubsub = pubsub`, the member would retain a stale handle while `m_running` stayed false.
- **Fix:** Added `m_pubsub.reset()` to both Initialize cleanup blocks (plus the planned Shutdown reset).
- **Files modified:** src/lib/gcs_storage/gcs_global_db.cpp
- **Verification:** `ninja gcs_core` green.
- **Committed in:** 9052669

---

**Total deviations:** 3 auto-fixed (2 bug, 1 missing critical)
**Impact on plan:** All fixes were necessary for correctness. The two test corrections surface a real contract gap (raw datastore keys + overwrite-only PutLocal) that 03-04 must consume, but no production-code behavior diverged from the plan.

## Issues Encountered

- `QueryKeyValues` and `PutLocal` behaved differently from the plan's assumptions (raw datastore keys; overwrite-only). Both are SuperGenius API contracts, not GCS bugs — resolved by correcting the tests and documenting the contracts for 03-04.

## Known Stubs

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Storage layer is ready for 03-04 (Messaging + D-08 crypto) and 03-05 (FFI wiring).
- **Flag for 03-04:** `QueryKeyValues` returns raw datastore keys (`/crdt/k/<key>/v`) — the message id must be extracted from the returned key, not expected as the logical key.
- **Flag for 03-04:** `PutLocal` is overwrite-only (requires an existing key). The D-03 receive-path archive write on a *fresh* message key will not succeed as written — 03-04 must either Put the message before PutLocal, use the 2-arg Put (empty topics, no broadcast) for the archive write, or otherwise resolve the fresh-key local-write path.

---
*Phase: 03-messaging*
*Completed: 2026-09-23*
