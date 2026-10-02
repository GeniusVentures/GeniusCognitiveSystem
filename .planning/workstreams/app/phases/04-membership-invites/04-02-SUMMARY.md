---
phase: 04-membership-invites
plan: 02
subsystem: crypto

tags: [crypto, aes-256-gcm, hkdf, openssl, key-wrap, key-store, thread-safety, cpp17, wr-02, d-06]

# Dependency graph
requires:
  - phase: 03-04
    provides: "gcs::crypto stateless OpenSSL adapter (DeriveRoomKey/EncryptPayload/DecryptPayload) + the D-08 nonce(12)||ct||tag(16) envelope"
  - phase: 03-02
    provides: "resolved OpenSSL/vendored crypto linkage (OpenSSL::Crypto 3.3.3)"
provides:
  - "gcs::crypto::WrapRoomKey/UnwrapRoomKey — AES-256-GCM key wrap of a 32-byte room key under a caller-supplied member key (reuses the D-08 envelope)"
  - "gcs::crypto per-room key store: MintRoomKey/StoreRoomKey/RoomKeyFor/ClearRoomKey — mutex-guarded map of room topic -> 32-byte key bytes (the documented statelessness exception)"
  - "DeriveRoomKey ikm swap — the stored 32-byte room key replaces roomTopic (WR-02 closure), falling back to topic-derivation for pre-membership rooms"
affects: [04-03 (membership component), 04-04 (creator stamping), 04-05 (invite mint/redeem), gcs_messaging decrypt path (unchanged seam)]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "gcs_crypto statelessness now has exactly one documented exception: a std::mutex-guarded unordered_map of room topic -> 32-byte key BYTES; no EVP context is ever cached (P7)"
    - "Key wrap composes the existing EVP_aes_256_gcm() envelope, differing from EncryptPayload only in the key source (caller memberKey, NOT HKDF(roomTopic))"
    - "DeriveRoomKey resolves the store under the mutex then releases it before EVP; ikm pointer targets a local copy so no lock is held during crypto"

key-files:
  created: []
  modified:
    - src/lib/gcs_crypto.hpp
    - src/lib/gcs_crypto.cpp
    - test/test_gcs_crypto.cpp

key-decisions:
  - "Key store lives in a .cpp anonymous namespace exposed via function-local static accessors (thread-safe, order-independent init); only <optional> was added to the public header — <mutex>/<unordered_map> stay in the .cpp per CLAUDE.md 'include only what a header directly needs'"
  - "MintRoomKey returns an empty vector on RAND_bytes failure (documented empty-on-failure contract) rather than an outcome, keeping this accessor outcome-free as the plan specified"
  - "WrapRoomKey/UnwrapRoomKey reject non-32-byte key inputs up front with GcsDbError before any EVP call (T-04-02-DoS)"

patterns-established:
  - "Per-room key store: mutex-guarded bytes only, never EVP state (the D-06 exception to gcs_crypto's statelessness)"
  - "D-06 key wrap = the D-08 GCM envelope with the member/invite key as the AES key and the 32-byte room key as the plaintext"

requirements-completed: [MEMB-01, MEMB-02]

# Metrics
duration: ~40min
completed: 2026-10-02
---

# Phase 4 Plan 2: Crypto Substrate (Key Store + Key Wrap + WR-02 ikm Swap) Summary

**Mutex-guarded per-room key store, AES-256-GCM room-key wrap/unwrap under a member key, and the DeriveRoomKey ikm swap to member-distributed keys (WR-02 closure) — 19 green unit tests, D-08 envelope and HKDF salt/info unchanged.**

## Performance

- **Duration:** ~40 min (includes one-time worktree build/verify setup: submodule init + CMake configure + baseline build)
- **Started:** 2026-10-02T12:57:22Z (first RED commit); setup began earlier
- **Completed:** 2026-10-02T20:00:00Z
- **Tasks:** 2 (both TDD: RED → GREEN)
- **Files modified:** 3

## Accomplishments

- `gcs::crypto` gained the D-06 surface: `MintRoomKey` (checked `RAND_bytes`), `StoreRoomKey`/`RoomKeyFor`/`ClearRoomKey` over a `std::mutex`-guarded `unordered_map<std::string, std::vector<unsigned char>>` holding **only** 32-byte key bytes — never EVP contexts (P7).
- `DeriveRoomKey` now resolves its HKDF ikm from the store when a room key is present, falling back to `roomTopic` for pre-membership rooms; `kMessagesHkdfSalt`/`kMessagesHkdfInfo` and the D-08 envelope are byte-for-byte unchanged (WR-02 closed).
- `WrapRoomKey`/`UnwrapRoomKey` wrap a 32-byte room key under a caller-supplied member key by reusing the existing AES-256-GCM envelope; wrong-key, tampered, and truncated envelopes fail closed, and non-32-byte keys are rejected before any EVP call.
- The `Messaging` `(roomTopic, envelope)` seam is untouched — key resolution happens inside `gcs_crypto`, so `gcs_messaging.*` needed no change.

## Task Commits

Each task was committed atomically (TDD RED → GREEN):

1. **Task 1 (RED): failing key-store/DeriveRoomKey tests** - `afca506` (test)
2. **Task 1 (GREEN): per-room key store + DeriveRoomKey ikm swap** - `3fdd39e` (feat)
3. **Task 2 (RED): failing WrapRoomKey/UnwrapRoomKey tests** - `003818b` (test)
4. **Task 2 (GREEN): WrapRoomKey/UnwrapRoomKey** - `bf1309f` (feat)

**Plan metadata:** (this SUMMARY commit) (docs: complete plan)

## Files Created/Modified

- `src/lib/gcs_crypto.hpp` - Key-store + wrap/unwrap declarations with Doxygen headers; `#include <optional>`; `@details` documents the single statelessness exception; `DeriveRoomKey` doc updated for the ikm swap.
- `src/lib/gcs_crypto.cpp` - Anonymous-namespace mutex + map accessors; `MintRoomKey`/`StoreRoomKey`/`RoomKeyFor`/`ClearRoomKey`; ikm resolution + swap in `DeriveRoomKey`; `WrapRoomKey`/`UnwrapRoomKey`.
- `test/test_gcs_crypto.cpp` - 12 new cases (6 key-store/DeriveRoomKey, 6 wrap/unwrap incl. non-32-byte rejection); added `MakeKey` helper and `<optional>`/`<thread>` includes.

## Decisions Made

- **Store placement.** The store is a `.cpp` anonymous-namespace `unordered_map` returned by a function-local static accessor, with a sibling mutex accessor. Function-local statics avoid static-init-order issues. Only `<optional>` was added to the public header (needed for the `RoomKeyFor` return type); `<mutex>`/`<unordered_map>` live in the `.cpp` where the store is defined.
- **`MintRoomKey` failure mode.** Returns an empty vector on `RAND_bytes` failure (documented empty-on-failure contract) — the plan asked to keep this accessor outcome-free.
- **ikm resolution ordering.** `RoomKeyFor` copies the key under the mutex and returns; the lock is released before any EVP call, and the `ikm` pointer targets that local copy so its lifetime covers the whole HKDF derivation.
- **Fail-closed key-size validation.** `WrapRoomKey`/`UnwrapRoomKey` validate `kRoomKeyLengthBytes` up front (T-04-02-DoS) rather than relying on OpenSSL's length errors.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical] Added a non-32-byte-key rejection test (T-04-02-DoS)**
- **Found during:** Task 2 (wrap/unwrap test authoring)
- **Issue:** The plan's `<threat_model>` marks T-04-02-DoS (`WrapRoomKey`/`UnwrapRoomKey` key-size input) as `mitigate`, but the Task 2 behavior block listed only five cases and none exercised the up-front size rejection.
- **Fix:** Implemented the up-front `memberKey.size()/roomKey.size()/cipherLen == kRoomKeyLengthBytes` guards (they were part of the Task 2 action) and added `Non32ByteKeysAreRejected` to prove the mitigation.
- **Files modified:** src/lib/gcs_crypto.hpp, src/lib/gcs_crypto.cpp, test/test_gcs_crypto.cpp
- **Verification:** `test_gcs_crypto` `Non32ByteKeysAreRejected` passes.
- **Committed in:** 003818b (test) + bf1309f (feat)

**2. [Intentional deviation - plan instruction vs CLAUDE.md] Header include minimization**
- **Found during:** Task 1 (header edit)
- **Issue:** The plan's action step 1 said to add `#include <mutex>` and `<unordered_map>` to `gcs_crypto.hpp`. Those types are only used by the `.cpp`-local store; the public header declares none of them.
- **Fix:** Added only `#include <optional>` to the header (required by the `RoomKeyFor` return type) and placed `<mutex>`/`<unordered_map>` in `gcs_crypto.cpp`. Per CLAUDE.md "Include only what a header directly needs". No functional difference; avoids leaking implementation-only types into a public contract.
- **Files modified:** src/lib/gcs_crypto.hpp, src/lib/gcs_crypto.cpp
- **Verification:** Clean build with the project's strict warning flags; all tests pass.
- **Committed in:** 3fdd39e

---

**Total deviations:** 2 (1 Rule-2 auto-fix, 1 intentional plan/CLAUDE.md alignment)
**Impact on plan:** No scope creep. The extra test directly covers the plan's own threat register; the header change is a style/contract refinement with identical behavior.

## Issues Encountered

- **Worktree had no build tree.** The plan's verify command (`ctest --test-dir build`) assumes a configured build. The isolated worktree started with empty submodules and no build directory. Resolved by initializing the `build` (cmaketemplate) and `GNUS-NEO-SWARM` submodules and configuring `build/OSX/Debug` against the prebuilt thirdparty/SuperGenius/GeniusSDK trees. The full project `test_gcs_crypto` target then built and ran green.
- **`clang-format` version mismatch.** The installed `clang-format` 22.1.7 reformats *untouched* committed lines (Doxygen comment reflow, signature wrapping) in these files, i.e. it is a newer version than the one the repo was formatted with. Running it would have produced a large unrelated diff. New/changed lines were hand-matched to each file's existing local style instead. There is no root `.clang-tidy` config and no clang-format/clang-tidy step in this repo's CI.
- **TDD RED is a compile failure.** Because the new API did not exist yet, both RED commits fail at compile time (undeclared identifier) rather than at assertion time — the standard C++ TDD RED for a new interface. Each RED commit was verified to fail before its GREEN commit.

## Verification

- `ctest --test-dir build/OSX/Debug -R test_gcs_crypto --output-on-failure` → **Passed** (19 tests: 7 pre-existing + 12 new).
- Broader regression on targets linking `gcs_core` (which carries `gcs_crypto`): `test_gcs_core_smoke`, `test_gcs_entities`, `test_gcs_crypto`, `test_gcs_messaging` → **4/4 Passed**. `test_gcs_messaging` exercises the encrypted send/receive/history path through the unchanged `(roomTopic, envelope)` seam with an empty store (fallback path), confirming no behavior change for pre-membership rooms.
- Acceptance greps: all four key-store declarations present with Doxygen headers; `std::mutex` guards the map; `RoomKeyFor` is called from `DeriveRoomKey`; `WrapRoomKey`/`UnwrapRoomKey` declared + defined with no `DeriveRoomKey` call in their bodies; `kMessagesHkdfSalt`/`kMessagesHkdfInfo` values unchanged; no `sleep_for` in the test file.

## Test Gate Compliance

TDD gate sequence present in git history: `test(...)` RED commits (`afca506`, `003818b`) each followed by a `feat(...)` GREEN commit (`3fdd39e`, `bf1309f`).

## Known Stubs

None. `RoomKeyFor`/`StoreRoomKey`/`ClearRoomKey` are wired to a real store; `DeriveRoomKey`'s store lookup is live and the topic fallback is the intended pre-membership behavior. `ClearRoomKey` is declared for later removal paths (per plan) and is exercised by tests now.

## Threat Flags

None. No new network endpoints, auth paths, file access, or schema changes at a trust boundary. The new surface is in-process key bytes only, already covered by the plan's `<threat_model>` (T-04-02-T/-EoP/-DoS/-InfoDisc).

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- The D-06 crypto substrate is ready for 04-03/04-04/04-05: call `MintRoomKey()` at room creation, distribute the room key wrapped with `WrapRoomKey`, `StoreRoomKey(topic, key)` at redeem, and Messaging's existing `DecryptPayload(roomTopic, envelope)` transparently resolves the stored key.
- The D-08 envelope and HKDF salt/info are wire/at-rest stable; no member record or key store entry is written before redeem (P4 resolution).
- Removal paths (later plan) should call `ClearRoomKey(topic)` when a room's key is retired.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-02*

## Self-Check: PASSED

- All 3 modified source/test files exist on disk.
- All 4 task commits verified in git history (afca506, 3fdd39e, 003818b, bf1309f).
