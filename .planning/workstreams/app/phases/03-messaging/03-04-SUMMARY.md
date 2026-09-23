---
phase: 03-messaging
plan: 04
subsystem: messaging
tags: [messaging, aes-256-gcm, hkdf, openssl, gossipsub, crdt, crypto, cpp17]

# Dependency graph
requires:
  - phase: 03-02
    provides: widened GcsGlobalDb/CoreSession surface (topics-aware Put, PutLocal, QueryKeyValues, RegisterNewElementCallback, raw Publish/Subscribe)
  - phase: 03-01
    provides: resolved SuperGenius/libp2p/GossipSub/OpenSSL signatures + proto contract (sender/deleted/deleted_at_ms + MessageHistory)
provides:
  - "gcs::crypto stateless OpenSSL adapter (DeriveRoomKey/EncryptPayload/DecryptPayload) — the only OpenSSL-including file"
  - "gcs::Messaging component (SendMessage/OnLiveMessage/OnMessageArrived/QueryHistory) with the injected D-08 CryptoSeam"
  - "test_gcs_crypto (7 tests) + test_gcs_messaging (11 tests) covering encrypted + plaintext paths"
affects: [03-messaging (03-05 FFI wiring, 03-06 integration), Phase 4 membership (key distribution swap only)]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "D-08 crypto seam: Messaging never includes OpenSSL; injected encrypt/decrypt callables + enabled flag (plaintext path when disabled)"
    - "nonce(12)||ciphertext||tag(16) envelope; room key = HKDF-SHA256(room_topic, gcs-messages-hkdf-v1, gcs-messages-v1, L=32); fresh EVP ctx per call"
    - "apply-once id-keyed dedupe funnel (ApplyMessage) absorbs the live + CRDT double-delivery; bounded by kMaxSeenIds=4096"
    - "receive-path archive write uses 2-arg Put (empty topics = local-only, no rebroadcast) — PutLocal is overwrite-only on fresh keys"

key-files:
  created:
    - src/lib/gcs_crypto.hpp
    - src/lib/gcs_crypto.cpp
    - src/lib/gcs_messaging.hpp
    - src/lib/gcs_messaging.cpp
    - test/test_gcs_crypto.cpp
    - test/test_gcs_messaging.cpp
  modified:
    - src/CMakeLists.txt
    - test/CMakeLists.txt

key-decisions:
  - "Receive-path archive write uses the 2-arg Put (empty topics = local-only, no rebroadcast) instead of PutLocal — SuperGenius PutLocal/PutKeyLocal is overwrite-only and rejects fresh keys (03-02 verified contract)"
  - "RoomTopicFromKey strips raw datastore framing (/<ns>/k/.../v) AND the leading '/' that HierarchicalKey adds to callback keys; QueryHistory ignores the raw QueryKeyValues key (the value carries room/id)"
  - "CryptoSeam uses a user-provided default constructor (not = nullptr member initializers) because a nested struct's default member initializer referencing the enclosing class's typedefs is rejected by Clang"

patterns-established:
  - "gcs_crypto is the ONLY src/ file including OpenSSL headers; stateless, fresh EVP_CIPHER_CTX/EVP_PKEY_CTX per call across the three calling thread types"
  - "GossipPubSub constructed with echo_forward_mode=true gives a single-node loopback for observing the live full-value publish (not a CID)"

requirements-completed: [CORE-04]

# Metrics
duration: 13min
completed: 2026-09-23
---

# Phase 3 Plan 4: Messaging + Crypto Summary

**gcs::Messaging component (live full-value publish + CRDT archive send, two-route id-keyed dedupe, decrypt-before-parse/sort history) with a stateless gcs::crypto AES-256-GCM adapter behind the injected D-08 seam — 18 green tests covering both encrypted and plaintext paths.**

## Performance

- **Duration:** 13 min
- **Started:** 2026-09-23T22:24:34Z
- **Completed:** 2026-09-23T22:37:23Z
- **Tasks:** 3
- **Files modified:** 8

## Accomplishments

- `gcs::crypto` stateless adapter: HKDF-SHA256 room-key derivation + AES-256-GCM encrypt/decrypt with the pinned `nonce(12)||ciphertext||tag(16)` envelope; the ONLY file including OpenSSL headers, fresh EVP context per call.
- `gcs::Messaging` component: `SendMessage` pushes pending then complete with the same id, publishes the full serialized record (or envelope) on the room topic, and archives under `gcs/messages/<room_topic>/<id>` with `{room_topic}`.
- A single `ApplyMessage` funnel decrypts first, parses, dedupes by message id (every message arrives twice — live + CRDT heal), flips self/peer role, and archives the received envelope locally without rebroadcast.
- `QueryHistory` decrypts each record before parse/sort, skips bad records with a warning, sorts by `(timestamp, id)`, and flips per-message self/peer role.
- `gcs_core` links the vendored OpenSSL via `OpenSSL::Crypto` (never pkg-config/homebrew).

## Task Commits

Each task was committed atomically (TDD RED → GREEN → GREEN):

1. **Task 1 (RED): failing crypto + messaging tests + target registration** - `dcc5de5` (test)
2. **Task 2 (GREEN): gcs_crypto adapter + OpenSSL::Crypto link** - `7f7be06` (feat)
3. **Task 3 (GREEN): Messaging component with CryptoSeam** - `4242f6a` (feat)

## Files Created/Modified

- `src/lib/gcs_crypto.hpp` - Stateless crypto adapter contract (constants + DeriveRoomKey/EncryptPayload/DecryptPayload)
- `src/lib/gcs_crypto.cpp` - HKDF-SHA256 + AES-256-GCM EVP implementation (fresh ctx per call, every return checked)
- `src/lib/gcs_messaging.hpp` - Messaging component API + injected CryptoSeam
- `src/lib/gcs_messaging.cpp` - SendMessage/OnLiveMessage/OnMessageArrived/ApplyMessage/QueryHistory
- `test/test_gcs_crypto.cpp` - 7 crypto tests (HKDF determinism, binary round-trip, envelope shape, nonce uniqueness, wrong key, tamper, truncation)
- `test/test_gcs_messaging.cpp` - 11 messaging tests (send lifecycle, sender stamp, dedupe, role flip, history sort/persistence/role flip, encrypted round-trip + wire/at-rest opacity, wrong-key skip, plaintext seam-never-called)
- `src/CMakeLists.txt` - Added gcs_crypto.cpp + gcs_messaging.cpp to gcs_core; `target_link_libraries(gcs_core PUBLIC OpenSSL::Crypto)`
- `test/CMakeLists.txt` - Registered test_gcs_crypto + test_gcs_messaging

## Decisions Made

- **Receive-path archive write = 2-arg `Put` (empty topics).** The plan's `PutLocal` is overwrite-only in SuperGenius (`PutKeyLocal` re-reads the existing priority record and fails on a fresh key — verified in 03-02). The 2-arg `Put` with an empty topic set is local-only (no DAG broadcast, no destination topics) and works on fresh keys, satisfying D-03 "no rebroadcast" and D-08 "ciphertext at rest" with the minimal change.
- **`RoomTopicFromKey` handles raw datastore keys + leading `/`.** SuperGenius `QueryKeyValues` returns `/crdt/k/<key>/v`, and the CRDT heal callback key gets a leading `/` from `HierarchicalKey` normalization. `RoomTopicFromKey` locates the `gcs/messages/` prefix and strips raw framing + the trailing `/<id>` so it is correct for logical keys (tests), normalized keys (heal), and raw keys (defensive).
- **`CryptoSeam` default constructor instead of `= nullptr` member initializers.** A nested struct's default member initializer referencing the enclosing class's `EncryptFn`/`DecryptFn` typedefs is rejected by Clang ("needed within definition of enclosing class ... outside of member functions"). A user-provided default constructor is behaviorally identical (`std::function` default-constructs empty).

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical] Receive-path archive write switched from PutLocal to empty-topics Put**
- **Found during:** Task 3 (ApplyMessage implementation)
- **Issue:** The plan (and 03-02's flag) called for `PutLocal` on the received message key, but SuperGenius `PutKeyLocal` is overwrite-only — it reads the existing key's priority record and fails on a fresh key. The live GossipSub copy of a message arrives before the CRDT archive converges, so the archive key is fresh and `PutLocal` would fail, dropping the local archive write.
- **Fix:** `ApplyMessage` archives via `m_session.Put(key, valueBytes)` (2-arg, empty topics = local-only, no rebroadcast). Verified against SuperGenius source: empty destinations ⇒ no `pendingBroadcastTopics_` ⇒ no CID broadcast.
- **Files modified:** src/lib/gcs_messaging.cpp
- **Verification:** Test 3 (two-route dedupe archives exactly once) and Test 11 (plaintext at-rest) pass.
- **Committed in:** 4242f6a

**2. [Rule 1 - Bug] RoomTopicFromKey made robust to raw + leading-slash key forms**
- **Found during:** Task 3 (OnMessageArrived implementation)
- **Issue:** Upstream finding #1: `QueryKeyValues` returns raw datastore keys (`/crdt/k/<key>/v`), and the CRDT heal callback key carries a leading `/` from `HierarchicalKey` normalization — neither is the bare logical key the plan assumed.
- **Fix:** `RoomTopicFromKey` locates `gcs/messages/`, strips raw `/v`/`/p` suffixes, strips the prefix, then drops the trailing `/<id>` segment (room topics may contain `/`). `QueryHistory` ignores the raw key entirely (the value carries room/id).
- **Files modified:** src/lib/gcs_messaging.cpp
- **Verification:** Tests 3/4 pass with logical keys; production heal keys (leading slash) handled by the same path.
- **Committed in:** 4242f6a

**3. [Rule 3 - Blocking] CryptoSeam default member initializer rejected by Clang**
- **Found during:** Task 3 (first build)
- **Issue:** `struct CryptoSeam { EncryptFn encrypt = nullptr; ... }` nested inside `Messaging` failed to compile: a nested struct's default member initializer referencing the enclosing class's typedefs is rejected by Clang when used by the constructor's default argument `crypto = {}`.
- **Fix:** Replaced the `= nullptr` initializers with a user-provided default constructor (`CryptoSeam() : encrypt(), decrypt(), enabled(false) {}`). Behaviorally identical.
- **Files modified:** src/lib/gcs_messaging.hpp
- **Verification:** gcs_core + both test targets build clean; all 18 tests green.
- **Committed in:** 4242f6a

**4. [Rule 1 - Bug] `-Wcomment` warnings from `/*` in Doxygen block comments**
- **Found during:** Task 2 build
- **Issue:** Doxygen `@details` lines written as `<openssl/*>` contained a literal `/*` inside a `*` block comment, tripping `-Wcomment` (and the literal `openssl/` string would also trip the "no OpenSSL include" grep).
- **Fix:** Reworded to "OpenSSL headers (the `openssl/...` family)" in gcs_crypto.hpp/.cpp and gcs_messaging.cpp.
- **Files modified:** src/lib/gcs_crypto.hpp, src/lib/gcs_crypto.cpp, src/lib/gcs_messaging.cpp
- **Verification:** Clean build (no warnings).
- **Committed in:** 7f7be06 (crypto) + 4242f6a (messaging)

---

**Total deviations:** 4 auto-fixed (2 bug, 1 missing-critical, 1 blocking)
**Impact on plan:** All fixes were necessary for correctness and consumed the two verified 03-02 upstream findings. The only intentional deviation from a plan acceptance grep is `PutLocal` (replaced by the empty-topics 2-arg `Put`); no scope creep.

## Issues Encountered

- `QueryKeyValues` raw keys and `PutLocal` overwrite-only semantics were consumed per the 03-02 flags rather than rediscovered — both are SuperGenius contracts, not GCS bugs.
- The nested `CryptoSeam` aggregate-initializer limitation is a Clang/C++ language detail; the user-provided default constructor preserves the plan's `CryptoSeam{}` default-argument semantics exactly.

## Known Stubs

None.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- `gcs::Messaging` and `gcs::crypto` are ready for 03-05 (FFI wiring: `send_text` delegate, join-time history replay, live subscribe + CRDT `RegisterNewElementCallback` bridge). 03-05 must take the session mutex in both receive lambdas (GossipSub strand + CRDT DagWorker threads are not the FFI command thread).
- The D-08 envelope is wire/at-rest stable; Phase 4 membership swaps only the key distribution, not the record format.

## Self-Check: PASSED

- All 6 created source/test files + SUMMARY.md exist on disk.
- All 3 task commits verified in git history (dcc5de5, 7f7be06, 4242f6a).

---
*Phase: 03-messaging*
*Completed: 2026-09-23*
