---
phase: 03-messaging
fixed_at: 2026-09-24T01:15:06Z
review_path: .planning/workstreams/app/phases/03-messaging/03-REVIEW.md
iteration: 1
findings_in_scope: 5
fixed: 5
skipped: 0
status: all_fixed
---

# Phase 3: Code Review Fix Report

**Fixed at:** 2026-09-24T01:15:06Z
**Source review:** .planning/workstreams/app/phases/03-messaging/03-REVIEW.md
**Iteration:** 1
**Scope:** Critical + Warning (CR-01, CR-02, WR-01, WR-02, WR-03). Info findings out of scope.

**Summary:**
- Findings in scope: 5
- Fixed: 5
- Skipped: 0

All fixes are confined to `src/ffi/gcs_core_ffi.cpp` (the FFI thunk — the layer
where the review located every in-scope defect). No changes under `thirdparty/`.
Fixes were applied and committed in dependency order rather than severity order:
the WR-02 in-flight mechanism is the safety net the CR-01 unlocked window
requires, and the CR-02 teardown restructure is what carries the WR-01 reorder.

The threading invariant now enforced throughout: no operation that waits on the
pubsub strand or a DagWorker runs under `g_mutex`, and every unlocked
raw-pointer window against the session/messaging globals is counted in
`g_inFlight` so `gcs_shutdown` drains it to zero before destroying anything.

## Fixed Issues

### CR-01: Join/derived-join blocks on the GossipSub strand while holding g_mutex — deadlock under concurrent live traffic

**Files modified:** `src/ffi/gcs_core_ffi.cpp`
**Commit:** 09c8cd4
**Applied fix:** `SubscribeLive` now takes the caller's `std::unique_lock`,
copies the raw `CoreSession*` under the lock, counts itself in `g_inFlight`,
releases `g_mutex` across the blocking `session->Subscribe(...)` (whose future
only the GossipSub strand fulfills), then re-locks and decrements. All call
sites updated: `kJoinTopic` arm, `RefreshDerivedJoins` (now takes the lock and
releases it only around the blocking subscribes), and `gcs_init` (lock_guard →
unique_lock). `PushMessageHistory` stays under the lock (local RocksDB scan,
per the review). Took the review's primary FFI-side fix; `GcsGlobalDb::Subscribe`
left untouched.

### CR-02: Teardown under g_mutex can deadlock when a receive callback is in flight (shutdown hang)

**Files modified:** `src/ffi/gcs_core_ffi.cpp`
**Commit:** 560b981
**Applied fix:** Three-part fix per the review sketch. (1) Atomic intake gate
`g_receivingDisabled`, checked FIRST (before taking g_mutex) by both receive
bridges (pubsub live + CRDT DagWorker); `gcs_shutdown` latches it before
draining, `gcs_init` re-arms it for a fresh session. (2) The receive lambdas
count their unlocked `OnLiveMessage`/`OnMessageArrived` windows in `g_inFlight`.
(3) `TeardownSessionAndNode(std::unique_lock*)` now evicts the guarded globals
(session/entities/messaging/topic sets) under the lock, then RELEASES the lock
across every blocking join — `GeniusSDKShutdown()` (node/pubsub threads),
`Messaging` destruction (archive worker; its Puts can trigger DagWorker
bridges), and `session->Shutdown()` (ShutdownNow → WaitForWorkersToExit) — and
re-acquires it before returning. The atexit hook passes nullptr (process-exit
path, unchanged unlocked semantics). A `g_tearingDown` atomic makes `gcs_init`
refuse to build a session during the unlocked destruction phase, since the
mutex no longer excludes init across the teardown joins (a window this fix
itself opened; closed rather than documented).

### WR-01: Teardown ordering guarantees the archive-queue drain drops every pending write (local message loss)

**Files modified:** `src/ffi/gcs_core_ffi.cpp`
**Commit:** 3441b75
**Applied fix:** Inside the CR-02-restructured teardown, `messaging.reset()`
(joins the archive worker, drains pending Puts) now runs BEFORE
`session->Shutdown()` so the drain's Puts hit a still-running store instead of
`GcsGlobalDb::Put`'s `m_running` failure gate. Ordering verified consistent in
the single shared teardown body (`TeardownSessionAndNode`), which is the only
teardown path that drains (`gcs_shutdown` and the exit hook both funnel through
it). Paired with the CR-02 intake gate so nothing re-enqueues mid-drain, per
the review's fix note.

### WR-02: kSendText arm dereferences the Messaging raw pointer outside the lock — use-after-free if gcs_shutdown runs concurrently

**Files modified:** `src/ffi/gcs_core_ffi.cpp`
**Commit:** 3dca798
**Applied fix:** Took the review's first option (enforce, don't document): an
in-flight counter (`g_inFlight`, guarded by `g_mutex`) plus condition variable
`g_idleCond`. The kSendText arm increments before unlocking, decrements and
notifies after relocking; `gcs_shutdown` (now on a `unique_lock`) waits for
`g_inFlight == 0` before calling teardown, so the `Messaging` object cannot be
destroyed under an in-flight `SendMessage` — including its post-Put `m_sink`
call. The comment documenting the Dart-serialization assumption was replaced
by the enforced invariant. The same accounting was extended to the CR-01
subscribe window and both receive funnels (commits 560b981 / 09c8cd4), so
every unlocked raw-pointer window in the file is covered.

### WR-03: Live-subscribe failure is swallowed — room appears joined but never receives live messages

**Files modified:** `src/ffi/gcs_core_ffi.cpp`
**Commit:** 8d28ee9
**Applied fix:** `SubscribeLive` returns bool and, on failure, pushes
`PostErrorNotice("live subscribe failed for room '<topic>'")` — the
established D-29 path — next to the existing `spdlog::error`, so both the
kJoinTopic arm and derived joins surface the failure. The kJoinTopic arm
returns `GCS_ERROR_GENERIC` when the live subscribe fails, matching the arm's
convention for surfaced partial failures (the join registration itself
succeeded; the RoomList with the topic was already pushed, so the client sees
the degraded state rather than a silent success).

## Verification

Per fix (before its commit): `ninja` in `build/OSX/Debug` + `ctest -R
"test_gcs_messaging|test_gcs_crypto|test_gcs_core" --output-on-failure` — all
passed after every commit (test_gcs_core_smoke, test_gcs_crypto,
test_gcs_messaging, test_gcs_messaging_multinode; 4/4 each run).

After the final fix: `ctest -R "test_gcs_ffi" --output-on-failure` — 4/4 passed
(test_gcs_ffi 9.4s, test_gcs_ffi_sdk 43.4s, test_gcs_ffi_coldboot 18.1s), no
shutdown hangs (the deadlock class these fixes closes).

Note for human review: these are concurrency-invariant fixes; the suites
exercise the paths but cannot deterministically reproduce the original races.
The lock-drain-gate reasoning above is the actual fix and deserves a read.

## Process notes

- Fixes applied directly on `feature/03-messaging` (not an isolated worktree):
  per-fix verification requires the shared incremental build dir
  `build/OSX/Debug`, whose CMakeCache resolves submodules and prebuilt
  thirdparty trees not reproducible in a fresh worktree. Each commit stages
  only `src/ffi/gcs_core_ffi.cpp`; the pre-existing dirty submodule pointer
  (`GNUS-NEO-SWARM`) was never staged.
- Commits (dependency order): 3dca798 (WR-02) → 560b981 (CR-02) → 3441b75
  (WR-01) → 09c8cd4 (CR-01) → 8d28ee9 (WR-03).

---

_Fixed: 2026-09-24T01:15:06Z_
_Fixer: Claude (gsd-code-fixer)_
_Iteration: 1_
