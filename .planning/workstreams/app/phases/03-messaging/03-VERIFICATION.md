---
phase: 03-messaging
verified: 2026-09-24T01:26:03Z
status: passed
score: 4/4 must-haves verified
overrides_applied: 0
resolution: "Sole human item (optional visual smoke) resolved by prior user decision 2026-09-23: 'Auto test + optional smoke' — automated multinode suite is the gate, smoke optional/non-gating per 03-06-PLAN.md gate=\"optional\". Phase proceeds complete; smoke remains available to the developer at will."
human_verification:
  - test: "Two-instance live visual smoke (the 03-06 optional non-gating Task 2, skipped as planned)"
    expected: "Two macOS Flutter app instances (separate TMPDIRs) exchange messages live: pending bubble at reduced opacity resolves to solid, peer bubbles lead with truncated 0x… sender label, self bubbles trail with none, both apps show identical history order after restart, 'Send failed' toast appears on a disconnected send."
    why_human: "No single automated test drives real-FFI data through the real widget tree: the multinode suite proves cross-node delivery in C++, the real-dylib smoke test proves the FFI push plane without the full shell, and the flutter suite pumps the shell with fake cubits. Visual chrome and two-real-app interaction are observable only in a running app. Note: 03-06-PLAN.md explicitly declared this smoke optional/non-gating (gate=\"optional\") and its skip was recorded per the plan's own acceptance criteria — this item surfaces it for a human accept/decline decision, it is not an execution gap."
---

# Phase 3: Messaging Verification Report

**Phase Goal:** Users can send and receive text messages in real-time, with all room participants converging on the same message history via CRDT.
**Verified:** 2026-09-24T01:26:03Z
**Status:** passed (4/4 truths verified; the 1 optional visual smoke resolved by the developer's prior non-gating decision — see Orchestrator resolution below)
**Re-verification:** No — initial verification

## Goal Achievement

### Observable Truths

| # | Truth | Status | Evidence |
|---|-------|--------|----------|
| 1 | User can send a text message to a room and see it appear locally (SC1) | ✓ VERIFIED | FFI `kSendText` arm validates and delegates to `gcs::Messaging::SendMessage` without holding `g_mutex` (`src/ffi/gcs_core_ffi.cpp:906-951`); SendMessage mints id, stamps sender, pushes PENDING then COMPLETE echoes with the same id (`src/lib/gcs_messaging.cpp:173-240`, pending at :191, complete at :238). End-to-end through the REAL dylib: `src/app/test/gcs_native_port_smoke_test.dart:186-202` asserts pending→complete echo with C++-stamped id and USER_SELF role. Unit: `GcsMessagingTest.SendLifecyclePushesPendingThenCompleteAndArchives` (green). Dart renders via `MessageFlowCubit.upsert` by-instanceId replace (`src/app/lib/cubits/message_flow_cubit.dart:74-81`). |
| 2 | A second user in the same room receives the message without manual refresh (SC2) | ✓ VERIFIED | Live path armed at join: `kJoinTopic` replays history THEN `SubscribeLive` (`src/ffi/gcs_core_ffi.cpp:892-903`); derived joins identical (`:578-581`); live callback funnels `OnLiveMessage` outside `g_mutex` with intake gate (`:488-514`). Genuine cross-node proof over real GossipSub (echo_forward_mode off, `AddPeers` cross-connect): `MessagingMultinodeTest.LiveDeliveryAndHistoryConvergeAcrossNodes` asserts A→B and B→A delivery via `WaitForCondition` on the peer's sink (`test/test_gcs_messaging_multinode.cpp:414-476`) — green in 2.0s. |
| 3 | Message history is identical across all participants after sync (SC3) | ✓ VERIFIED | Archive replication: `Put(key, payload, {roomTopic})` at `src/lib/gcs_messaging.cpp:232`; heal route via `RegisterNewElementCallback(kMessagesKeyCallbackPattern="/gcs/messages/.*")` (`src/ffi/gcs_core_ffi.cpp:753-781`); two-route id-keyed dedupe in `ApplyMessage` (`gcs_messaging.cpp:285-297`). Multinode test asserts per-index equality of id, timestamp, and text across both nodes after both send directions (`test_gcs_messaging_multinode.cpp:459-470`); `MessageSurvivesSessionRestartOnSharedDb` and `HistoryReplaysAfterRestart` prove persistence. FFI SDK test re-reads the encrypted `gcs/messages/<topic>/<id>` archive across session cycles (`test/test_gcs_ffi_sdk.cpp:448-502`). |
| 4 | Messages display in chronological order with sender identification (SC4) | ✓ VERIFIED | Sort by `(timestamp, id)` with id tiebreak (`src/lib/gcs_messaging.cpp:354-361`); sender stamped C++-side from `GeniusSDKGetAddress().address` (`gcs_core_ffi.cpp:736`, `gcs_messaging.cpp:184`); role flip self/peer on both push and history paths (`gcs_messaging.cpp:350, 395-400`). Tests: `QueryHistorySortsByTimestampThenId`, `CompleteEventCarriesStampedSenderAndSelfRole`, `HistoryFlipsPeerRoleButKeepsSelfRole`, and restart replay asserts non-decreasing timestamps (`test_gcs_messaging_multinode.cpp:549-553`). Dart: `MessageHistory` batch → `replaceAll` (`src/app/lib/cubits/session_cubit.dart:385-393`), truncated sender label with named constants (`message_flow_cubit.dart:42-56,101`); shell test asserts `0x…` truncation and null-for-empty (`src/app/test/cubits/shell_cubits_test.dart:287-307`). |

**Score:** 4/4 truths verified

### Decision D-08 Spot-Check (At-Rest/Wire Encryption)

| Claim | Status | Evidence |
|-------|--------|----------|
| gcs_crypto is the ONLY src/ file including OpenSSL headers | ✓ VERIFIED | `grep -rn "#include <openssl" src/` → matches only `src/lib/gcs_crypto.cpp:19-21` |
| DeriveRoomKey = HKDF-SHA256, AES-256-GCM envelope nonce(12)\|\|ct\|\|tag(16) | ✓ VERIFIED | `src/lib/gcs_crypto.hpp:44-52` (kGcmNonceLength=12, kGcmTagLength=16, kRoomKeyLengthBytes=32, salt/info domain separators); EVP implementation checks every return, frees ctx on all paths (`gcs_crypto.cpp:28-177`) |
| Encrypted by default, injected seam, plaintext path never calls it | ✓ VERIFIED | `gcs_init` builds `CryptoSeam` with real adapter, `enabled = true` (`gcs_core_ffi.cpp:730-743`); `Messaging` never includes OpenSSL; `PlaintextPathNeverCallsTheInjectedSeam` green |
| Disk carries ciphertext only | ✓ VERIFIED | `AtRestOpacityBothSides` binary-scans BOTH nodes' db dirs for the plaintext marker: 0 hits (`test_gcs_messaging_multinode.cpp:480-505`); `EncryptedArchiveIsOpaqueAtRest` green |

### Required Artifacts

| Artifact | Expected | Status | Details |
|----------|----------|--------|---------|
| `src/proto/gcs_chat.proto` | sender=7/deleted=8/deleted_at_ms=9, `message_history=6`, MessageHistory | ✓ VERIFIED | :74-76, :102, :178-181; append-only (existing tags untouched); generated Dart pb matches (:430-432) |
| `src/lib/gcs_messaging.{hpp,cpp}` | Messaging component, CryptoSeam, dedupe, async archive worker, key prefix/pattern | ✓ VERIFIED | 254/402 lines; `kMessagesKeyPrefix`/`kMessagesKeyCallbackPattern` (`hpp:82-87`); archive worker thread + queue + condvar (`cpp:95-119`) |
| `src/lib/gcs_crypto.{hpp,cpp}` | HKDF + AES-256-GCM stateless adapter | ✓ VERIFIED | 113/179 lines; fresh EVP ctx per call |
| `src/ffi/gcs_core_ffi.cpp` | send_text/join_topic arms, EventSink, receive bridges, threading contract | ✓ VERIFIED | 1122 lines; 4-function ABI unchanged (`gcs_core.h:92-149`) |
| `src/app/lib/cubits/*`, `src/app/lib/shell/gcs_shell.dart` | upsert/replaceAll, messageHistory dispatch, send→bool, toast | ✓ VERIFIED | All present and substantive (see Truth 1/4 evidence) |
| `test/test_gcs_messaging.cpp` (11 tests), `test_gcs_messaging_multinode.cpp` (3), `test/test_gcs_crypto.cpp` (7) | Substantive suites | ✓ VERIFIED | All registered in `test/CMakeLists.txt:80-87`; 39 assertions in multinode alone; zero `sleep_for` |

### Key Link Verification

| From | To | Via | Status | Details |
|------|----|----|--------|---------|
| Dart composer | C++ send_text | `ComposerCubit.send()` → `publishCommand` → `gcs_publish` | ✓ WIRED | `composer_cubit.dart:84-99`; real-dylib smoke asserts the round trip |
| FFI send_text | Messaging | unlocked `SendMessage` delegation, `g_inFlight`-counted | ✓ WIRED | `gcs_core_ffi.cpp:939-945` |
| join_topic / derived joins | History + live subscribe | `PushMessageHistory` then `SubscribeLive` | ✓ WIRED | `:892-903`, `:578-581` |
| GossipSub live callback | Messaging | gate → copy pointer under lock → `OnLiveMessage` outside lock | ✓ WIRED | `:488-514` |
| CRDT DagWorker callback | Messaging | `kMessagesKeyCallbackPattern` → `OnMessageArrived` outside lock | ✓ WIRED | `:753-781` |
| Messaging sink | Dart | EventSink self-locks → `PostToDart` → NativePort | ✓ WIRED | `:737-741`; smoke test consumes pushed events |
| GcsEvent message/messageHistory | Cubits | `upsert` / `replaceAll` | ✓ WIRED | `session_cubit.dart:378-394` |

### Data-Flow Trace (Level 4)

Real data flows through every layer — verified by tests against real transports/storage, not mocks: real GossipSub cross-node delivery (multinode `AddPeers`), real CRDT/graphsync heal (multinode convergence + `WaitForCondition`), real RocksDB persistence (restart replay on same db dir; binary disk scan for opacity), real dylib FFI push (smoke test `DynamicLibrary.open`). No static/hollow data sources found.

### Behavioral Spot-Checks

| Behavior | Command | Result | Status |
|----------|---------|--------|--------|
| Messaging + crypto + multinode suites | `ctest -R "test_gcs_messaging\|test_gcs_crypto"` | 3/3 passed (0.11s / 1.43s / 2.03s) | ✓ PASS |
| FFI suites (incl. post-review-fix threading paths) | `ctest -R "test_gcs_ffi"` | 4/4 passed (ffi 17.3s, sdk 42.7s, coldboot 17.1s) — no shutdown hangs | ✓ PASS |
| Full C++ suite | `ctest` | 10/11 first pass; `test_gcs_global_db_sdk` hit the documented pre-existing Phase 2 teardown flake (RESEARCH Q-01, ~25-35% rate) and PASSED on single re-run (8.87s) — effectively 11/11 | ✓ PASS |
| Flutter suite | `flutter test` (src/app) | 54 passed +1 skipped, "All tests passed!" (exceeds the 26 claimed at 03-03 time; suite includes earlier-phase tests) | ✓ PASS |

### Review Fix Verification (03-REVIEW → 03-REVIEW-FIX)

All 5 in-scope findings (2 Critical + 3 Warning) verified present in current `src/ffi/gcs_core_ffi.cpp`, not just committed:

| Finding | Commit | Code Evidence | Status |
|---------|--------|---------------|--------|
| CR-01 subscribe-live ABBA | 09c8cd4 | `SubscribeLive(std::unique_lock&, ...)` unlocks across `session->Subscribe` (:483-516); `RefreshDerivedJoins` + `gcs_init` pass unique_lock (:551, :613) | ✓ VERIFIED |
| CR-02 teardown deadlock | 560b981 | `g_receivingDisabled` checked FIRST in both bridges (:491, :757); `g_tearingDown` init guard (:615); `TeardownSessionAndNode` evicts under lock, releases across all blocking joins (:221-261) | ✓ VERIFIED |
| WR-01 archive-queue drain loss | 3441b75 | `messaging.reset()` BEFORE `session->Shutdown()` (:253-254) | ✓ VERIFIED |
| WR-02 unlocked raw-pointer window | 3dca798 | `g_inFlight`/`g_idleCond`; counted in send (:940-945), subscribe (:486, :517), both funnels (:501/:511, :767/:777); `gcs_shutdown` waits `g_inFlight == 0` (:1115) | ✓ VERIFIED |
| WR-03 swallowed live-subscribe failure | 8d28ee9 | `SubscribeLive` returns bool + `PostErrorNotice` (:519-527); kJoinTopic returns `GCS_ERROR_GENERIC` (:896-903) | ✓ VERIFIED |

Known deliberate exception (review-blessed): `PushMessageHistory`/`QueryHistory` runs under `g_mutex` — the review confirmed `QueryKeyValues` is a local RocksDB scan that cannot block on the strand/DagWorkers.

### Requirements Coverage

| Requirement | Source Plan | Description | Status | Evidence |
|-------------|------------|-------------|--------|----------|
| CORE-04 | 03-01..03-06 (all claim it) | "User can send and receive text messages in real-time" | ✓ SATISFIED | Truths 1-4; REQUIREMENTS.md maps CORE-04 → Phase 3, marked Complete |

No orphaned requirements: REQUIREMENTS.md maps only CORE-04 to Phase 3; all six plans declare it.

### Anti-Patterns Found

| File | Line | Pattern | Severity | Impact |
|------|------|---------|----------|--------|
| `src/ffi/gcs_core_ffi.cpp` | 73 | "placeholder" wording in comment describing the offline-safe dev boot config (review IN-02, pre-existing Phase 1 pattern) | ℹ️ Info | None — comment only; documented interim contract |

Zero TBD/FIXME/XXX/TODO/HACK markers in any phase file. Zero `sleep_for` (tests use wait-condition templates). Review Info findings IN-01..IN-05 were declared out of scope by 03-REVIEW-FIX (fix scope = Critical+Warning) and remain open informational items; none block the goal.

### Human Verification Required

### 1. Two-instance live visual smoke (optional per plan — awaiting accept/decline)

**Test:** Launch two macOS Flutter app instances with separate TMPDIRs (`TMPDIR=/tmp/gcsA .../flutter_app.app/Contents/MacOS/flutter_app &`, likewise `/tmp/gcsB`), join a common room, exchange messages.
**Expected:** Pending bubble at reduced opacity resolves to solid; peer bubbles lead with truncated `0x…` sender label, self bubbles trail with none; both apps converge on identical history order; history replays after app restart; "Send failed" toast appears when sending while disconnected.
**Why human:** Visual chrome and two-real-app interaction are observable only in a running app — no automated test drives real-FFI data through the real widget tree (see frontmatter note for the layer-coverage breakdown). 03-06-PLAN.md declared this Task 2 `gate="optional"`/non-gating and recorded the skip per its own acceptance criteria; surfacing it here per the verification contract so the developer decides whether to run it or accept the automated coverage.

**Orchestrator resolution (2026-09-24):** ACCEPTED automated coverage. The developer pre-decided this item's disposition on 2026-09-23 when directing the 03-06 replan (choice: "Auto test + optional smoke (Recommended)" — the automated multinode suite is the gating verification; the human smoke is optional and non-gating). Every underlying layer has green automated coverage (multinode cross-node C++, real-dylib FFI smoke, flutter widget suite). Status advanced to `passed` on that basis. The smoke steps above remain available to the developer at any time and require no phase action.

### Gaps Summary

No gaps. All four roadmap success criteria are verified in code and by green tests, all D-01..D-08 decisions are implemented as decided (including the encrypted-by-default full-record envelope with the injected seam), all five code-review Critical/Warning fixes are present in the current tree and their suites pass, and the requirement CORE-04 is satisfied. One informational pre-existing comment and the five review Info findings remain open (non-gating). The single human item is the plan-declared-optional visual smoke, not an execution failure. Observation (not a gap): the verification brief mentioned a `leave_topic` command arm — no such command exists in `gcs_chat.proto` or any Phase 3 plan (the oneof is join_topic/send_text/create_space/create_room/update_space); leaving rooms was never in the Phase 3 contract.

---

_Verified: 2026-09-24T01:26:03Z_
_Verifier: Claude (gsd-verifier)_
