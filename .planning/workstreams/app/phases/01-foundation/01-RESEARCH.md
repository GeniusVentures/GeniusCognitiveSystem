# Phase 1: Foundation - Research

**Researched:** 2026-08-15
**Domain:** C++ build system integration (cmaketemplate parent build), SuperGenius GlobalDB/CRDT, Dart FFI (ffigen + native→Dart callbacks), GitHub Actions CI with prebuilt release dependencies
**Confidence:** HIGH (all build/link/test claims verified against live files in this workspace; release tags verified via `gh release list`)

## Summary

Phase 1 is mostly a **wiring phase, not a design phase**: every hard problem (GlobalDB component, GeniusSDK linkage, ffigen pattern, CI release-download pattern) already has a working in-repo or sibling-repo implementation to copy. The `gcs_core` static library target already builds today (`build/OSX/Debug/gcs_src/libgcs_core.a` exists, configured via `cmake -S build/OSX -B build/OSX/Debug`), and `neoswarm_storage` already links `sgns::crdt_globaldb` + `sgns::GeniusSDK_shared` with a hard `FATAL_ERROR` contract. Phase 1's real work is: (1) link `gcs_core` to `neoswarm_storage`, (2) flesh out `gcs_core.cpp` around `GcsGlobalDb` lifecycle, (3) add a new `src/ffi/` `gcs_ffi` SHARED library with a single opaque-handle C API, (4) add a GTest smoke suite under a new GCS-root `test/` directory, and (5) author `.github/workflows/cmake.yml` copying the GeniusSDK consumer-repo workflow (which already downloads thirdparty + SuperGenius releases and builds 5 platforms).

One notable gap discovered: **the Dart NativePort/ReceivePort callback pattern referenced in D-05 does NOT currently exist in `neoswarm_ffi`** — that plugin is the Flutter plugin template's stock `sum()`/`sum_long_running()` example with no `Dart_PostCObject`, no `Dart_Port`, and no `Pointer<NativeFunction>` callback anywhere in GNUS-NEO-SWARM, SuperGenius, or GeniusSDK. The working callback mechanism to mirror is therefore the well-known Dart `Dart_PostCObjectType` + `NativePort` API pattern (documented below with the exact ffigen idioms), not an existing in-repo implementation. This is flagged in Open Questions — the planner should treat D-05's "reuse that mechanism" as "implement the standard mechanism in the same style as neoswarm_ffi's ffigen setup", since only the ffigen scaffolding exists to reuse.

**Primary recommendation:** Extend `src/CMakeLists.txt` to link `gcs_core PUBLIC neoswarm_storage`; create `src/ffi/CMakeLists.txt` with a `gcs_ffi` SHARED target (pattern from `GNUS-NEO-SWARM/neoswarm_ffi/src/CMakeLists.txt` + export-macro style from `genius_elm_chat_completions.h`); create GCS-root `test/` with a wait-condition smoke test modeled line-for-line on `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp` (injected-pubsub seam, port 0, no GeniusSDK node needed); write CI as a near-verbatim copy of `../GeniusSDK/.github/workflows/cmake.yml` minus zkLLVM.

## User Constraints (from CONTEXT.md)

### Locked Decisions
- **D-01:** Single opaque-handle "core session" API in `src/ffi/` — one `extern "C"` header where everything funnels through a session handle (`gcs_init` → handle; subsequent calls take the handle). NOT per-module headers.
- **D-02:** Separate `gcs_ffi` **shared library** target (not folded into `gcs_core` static lib). `gcs_ffi` links `gcs_core` (and transitively `neoswarm_storage` → `sgns::crdt_globaldb` / `sgns::GeniusSDK_shared`). Flutter loads the shared lib directly.
- **D-03:** `neoswarm_ffi` may no longer be needed by the app once `gcs_ffi` exists. Do NOT remove it in this phase; leave it working. Re-evaluate at Phase 6.
- **D-04:** C++ owns essentially all state and logic. Dart is a thin wrapper.
- **D-05:** Network data pushed to Dart via **native→Dart callbacks** — ffigen `Pointer<NativeFunction>` + `ReceivePort`/NativePort pattern. Dart does NOT poll. Listener registration on the session handle (e.g. `gcs_on_message(handle, dartPort)`-style).
- **D-06:** Follow SuperGenius CI pattern: `resolve-runners` job picks self-hosted runners by label (`sg-ubuntu-linux`, `sg-arm-linux`, `SG-WIN11`, `gv-OSX-Large`) with GitHub-hosted fallbacks, then a matrix job builds **Android (arm64-v8a, armeabi-v7a), iOS, OSX, Linux (x86_64, aarch64), Windows × Debug/Release**.
- **D-07:** Dependencies downloaded as **prebuilt release tarballs** (thirdparty from `GeniusVentures/thirdparty`; SuperGenius and GeniusSDK releases, tagged per `Target-ABI-branch-buildtype`) — NOT built per-run. GeniusSDK's workflow is the template.
- **D-08:** CI builds and ctests the **C++ core only** in Phase 1 (no Flutter app build in CI).
- **D-09:** Self-hosted runner cleanup + container usage (`ghcr.io/geniusventures/debian-bullseye` for Linux/Android) copied from SuperGenius workflow; skip zkLLVM.

### Claude's Discretion
- Exact C API function names/signatures in the session header (C-only, opaque handle, C++17).
- GTest smoke-test specifics (GlobalDB local pub/sub round-trip, FFI init/echo call) — wait-condition templates, never sleep_for.
- Whether `gcs_ffi` lives under `src/ffi/` (expected home per user) vs `src/lib/`.

### Deferred Ideas (OUT OF SCOPE)
- Removing/retiring `neoswarm_ffi` (Phase 6).
- Flutter app builds in CI (later phase).
- zkLLVM in CI.
- Message CRDT schema design (Phase 3).
- GCS bot identity mapping (Phase 6).

## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| CORE-05 | Messages sync via CRDT across all room participants | Phase 1 proves the substrate: `GcsGlobalDb` wraps `sgns::crdt::GlobalDB` (Put/Get/AddBroadcastTopic/AddListenTopic verified in SuperGenius `globaldb.hpp`); smoke test exercises init → `AddBroadcastTopic` + `AddListenTopic` on a test topic → `Put` → local CRDT store contains the value (Get round-trip). Real multi-participant sync semantics land in Phase 3 with the message schema; Phase 1 establishes the CRDT is initialized, started, and round-trips locally over a real GossipPubSub. |

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| CRDT store lifecycle (init/start/shutdown) | C++ core (`gcs_core` via `GcsGlobalDb`) | — | `GcsGlobalDb` already owns this; gcs_core wraps it (D-04) |
| Pub/sub transport (GossipSub topics) | C++ core (SuperGenius `GossipPubSub` via GeniusSDK node or injected) | — | libp2p stack lives in C++; Dart never touches sockets |
| FFI symbol export / C ABI | `gcs_ffi` shared lib | — | D-01/D-02: thin translation layer over gcs_core, owns no state |
| Async event delivery to UI | `gcs_ffi` (Dart_PostCObject) | Dart `ReceivePort` | D-05: push not poll; C++ posts, Dart receives |
| State ownership (session, rooms later) | C++ core (opaque handle table in `gcs_ffi`) | — | D-01 opaque handle = pointer into C++-owned session map |
| Build orchestration per platform | `build/<Platform>/CMakeLists.txt` (cmaketemplate) | CI workflow | Existing skeleton; CI just drives `cmake -S build/<Target> -B ...` |
| Dependency provisioning | Prebuilt release tarballs (CI) / sibling-repo build trees (local) | CMake auto-download fallback in cmaketemplate | D-07; cmaketemplate already auto-downloads when sibling dirs are absent |

## Standard Stack

### Core (all already in-tree — no new external packages this phase)
| Component | Version/Commit | Purpose | Why Standard |
|-----------|---------------|---------|--------------|
| `sgns::crdt_globaldb` (SuperGenius) | develop (release tags `OSX-develop-Debug` etc., latest 2026-08-14) [VERIFIED: `gh release list --repo GeniusVentures/SuperGenius`] | CRDT store: `Put`/`Get`/`AddBroadcastTopic`/`AddListenTopic`/`Start` [CITED: SuperGenius `src/crdt/globaldb/globaldb.hpp`] | The mandated GlobalDB; CONTEXT D-01..D-04 of neoswarm Phase 3 |
| `sgns::GeniusSDK_shared` (GeniusSDK) | develop (releases 2026-08-04) [VERIFIED: `gh release list --repo GeniusVentures/GeniusSDK`] | `GeniusSDKInit`/`GeniusSDKGetNode()` — in-process GeniusNode + shared GossipPubSub [CITED: GeniusSDK `src/GeniusSDK.h:200`, `src/GeniusSDK.hpp:22`] | D-15/D-16a: GcsGlobalDb pulls pubsub from the in-process SDK |
| `neoswarm_storage` (GNUS-NEO-SWARM submodule) | in-tree | `GcsGlobalDb` component (init/shutdown/IsRunning, injected-pubsub test seam) [CITED: `GNUS-NEO-SWARM/src/storage/gcs_global_db.hpp`] | Existing component Phase 1 initializes and smoke-tests |
| `sgns::ipfs_pubsub::GossipPubSub` (thirdparty/ipfs-pubsub) | in-tree thirdparty | `Start(port, ...)` → future, `Publish(topic, bytes)`, `Stop()` [CITED: `thirdparty/ipfs-pubsub/src/ipfs_pubsub/gossip_pubsub.hpp:110-146`] | Transport under GlobalDB; smoke test stands one up on port 0 |

### Supporting
| Component | Purpose | When to Use |
|-----------|---------|-------------|
| GTest (thirdparty) | Test framework | All smoke tests; `find_package(GTest CONFIG REQUIRED)` already in root CMakeLists (guarded by `BUILD_TESTING`) |
| ffigen (Dart dev-dependency) | Generate Dart bindings from the C header | Local dev loop when the `gcs_ffi` header changes; config modeled on `neoswarm_ffi/ffigen.yaml` |
| Dart `ffi` package `^2.2.0` | `Pointer<NativeFunction>`, `NativePort` types | Already in `src/app/pubspec.yaml` [VERIFIED: read of pubspec] |
| `Dart_PostCObject` (Dart VM C API `dart_native_api.h`) | Native→Dart async message post | The D-05 callback mechanism; header ships with the Dart SDK / Flutter engine |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Injected-pubsub smoke test (`GcsGlobalDb::Initialize(pubsub)`) | Full GeniusSDK init in tests (`GeniusSDKInit` → node online) | Full SDK init brings up the whole GeniusNode (wallet, network) — slow, needs ports/config, flaky in CI. The injected seam exists precisely for this (Tier 2 fixture pattern; CITED: gcs_global_db.hpp doc comment). Use full-SDK init only in the FFI-level test if planner wants an end-to-end init path — recommend deferring that to a manual/integration test, not the CI smoke suite. |
| `Dart_PostCObject` + ReceivePort | `NativeCallable.listener` (Dart 3.x isolate-callback style) | NativeCallable avoids dart_native_api.h but requires Dart-side isolate management and is newer/less documented in ffigen-generated code. D-05 names the ReceivePort/NativePort pattern explicitly — follow the decision. [ASSUMED — Dart version nuance; both work with `ffi: ^2.2.0`] |

**Installation:** None. No new external packages are introduced this phase. All C++ deps come from prebuilt thirdparty/SuperGenius/GeniusSDK trees; Dart deps (`ffi`, `ffigen`) are already in `src/app/pubspec.yaml` / dev-dependencies pattern of `neoswarm_ffi`.

## Package Legitimacy Audit

| Package | Registry | Age | Downloads | Source Repo | slopcheck | Disposition |
|---------|----------|-----|-----------|-------------|-----------|-------------|
| *(none — this phase adds no external packages)* | — | — | — | — | — | — |

**Packages removed due to slopcheck [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

Note: slopcheck was run and is available. The only Dart-side package this phase touches is the already-present `ffi: ^2.2.0` (Dart pub, not PyPI — slopcheck's pypi check is inapplicable). All C++ dependencies are prebuilt release artifacts from the three sibling GeniusVentures repos, not registry packages.

## Build System Wiring (Q1, Q5, Q6 — verified against live files)

### Entry points (Q6)

The GCS repo has **two** CMake entry paths:

1. **`build/<Platform>/CMakeLists.txt`** (the real one — cmaketemplate submodule). Chain: `build/OSX/CMakeLists.txt` → `build/CommonCompilerOptions.cmake` (platform/toolchain/thirdparty discovery, `get_default_root()` sets `PROJECT_ROOT` to the GCS repo root, `get_third_party_dir()` walks up to find `../thirdparty` with the `GeniusVentures/thirdparty` git remote) → `build/CommonBuildParameters.cmake` (find_package for all thirdparty deps, then `add_subdirectory(${PROJECT_ROOT}/GNUS-NEO-SWARM ...)` and `add_subdirectory(${PROJECT_ROOT}/src ${CMAKE_BINARY_DIR}/gcs_src)`). Platform dirs present: `build/{Android,iOS,Linux,OSX,Windows}/CMakeLists.txt` — all exist. [VERIFIED: directory listing + file reads]

2. **Repo-root `CMakeLists.txt`** — a lighter top-level that also does `add_subdirectory(GNUS-NEO-SWARM)` + `add_subdirectory(src)`. Per `GNUS-NEO-SWARM/CMakeLists.txt` header comment, the standalone/canonical path is the `build/<Platform>` chain; the root file exists for parent-style embedding. **The existing successful local build used path 1**: `build/OSX/Debug/CMakeCache.txt` shows `CMAKE_HOME_DIRECTORY=.../build/OSX`, generator Ninja, `CMAKE_BUILD_TYPE=Debug`, and `libgcs_core.a` present under `build/OSX/Debug/gcs_src/`. [VERIFIED: CMakeCache.txt]

**Canonical local configure command (Q5):**
```bash
cmake -S build/OSX -B build/OSX/Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build/OSX/Debug -j
```
(Linux: `build/Linux` + `ABI_SUBFOLDER_NAME` is auto-set from `CMAKE_SYSTEM_PROCESSOR` → `/x86_64` or `/aarch64`; Android: `build/Android` + `-DANDROID_ABI=arm64-v8a` + `-DCMAKE_ANDROID_NDK=...`; Windows: `build/Windows`; iOS: `build/iOS` + `-DPLATFORM=OS64`.)

### Dependency env-var flow (Q1)

Discovery order in `GNUS-NEO-SWARM/cmake/CommonBuildParameters.cmake` (lines 310–456) — all verified:

| Variable | How set | Consumed by |
|----------|---------|-------------|
| `_THIRDPARTY_BUILD_DIR` | `build/CommonCompilerOptions.cmake` → `${THIRDPARTY_DIR}/build/${BUILD_PLATFORM_NAME}/${CMAKE_BUILD_TYPE}${ABI_SUBFOLDER_NAME}`; auto-downloads `GeniusVentures/thirdparty` release tarball if the sibling tree is missing | `cmake/CommonBuildParameters.cmake` (root) sets `THIRDPARTY_BUILD_DIR` from it; all `find_package` dirs derive from it |
| `GENIUSSDK_BUILD_DIR` (input) / `GENIUS_SDK_BUILD_DIR` (derived) | Pass `-DGENIUSSDK_BUILD_DIR=...` to override; else auto-detected as `${PROJECT_SUPER_ROOT}/GeniusSDK/build/<Platform>/<BuildType>[/<ABI>]`; auto-downloads GeniusSDK release tarball via curl if `${PROJECT_SUPER_ROOT}/GeniusSDK` is absent | `find_package(GeniusSDK CONFIG)` at `${GENIUS_SDK_BUILD_DIR}/GeniusSDK/lib/cmake/GeniusSDK/` (fallback: the dir itself); `neoswarm_storage` include dirs |
| `SUPERGENIUS_BUILD_DIR` | Auto-detected as `${PROJECT_SUPER_ROOT}/SuperGenius/build/<Platform>/<BuildType matching GeniusSDK>[/<ABI>]` | `find_package(SuperGenius CONFIG)` at `${SUPERGENIUS_BUILD_DIR}/SuperGenius/lib/cmake/SuperGenius/`; include dirs `${SUPERGENIUS_BUILD_DIR}/SuperGenius/include[/evmrelay]` |
| `GENIUS_SDK_DIR` | Source tree path (`${PROJECT_SUPER_ROOT}/GeniusSDK`) — used for `GeniusSDK.hpp` include (not yet installed) | `neoswarm_storage` include dirs |
| `ZKLLVM_BUILD_DIR` | Auto-detected/auto-downloaded by CommonCompilerOptions; include dir added to `neoswarm_storage` only `if(DEFINED ...)` | Optional include path (GeniusNode.hpp transitively pulls ProofSystem headers) |

Locally (this machine) the cache resolved to sibling build trees: `GENIUS_SDK_BUILD_DIR=../GeniusSDK/build/OSX/Debug`, `SUPERGENIUS_BUILD_DIR=../SuperGenius/build/OSX/Debug`. [VERIFIED: CMakeCache.txt]

**CI flow (D-07):** the workflow downloads tarballs into sibling dirs *before* configure, so the auto-detect paths hit and no `-D` overrides are needed:
- `gh release download <Target>[-<ABI>]-<branch>-<BuildType> --repo GeniusVentures/thirdparty -p <FILE>.tar.gz` → extract to `thirdparty/` → `THIRDPARTY_BUILD_DIR=<ws>/thirdparty/build/<Target>/<BuildType>[/<ABI>]`
- Same pattern for `GeniusVentures/SuperGenius` (→ `SUPERGENIUS_DIR=<ws>/SuperGenius`) and — **new for GCS vs the GeniusSDK workflow** — `GeniusVentures/GeniusSDK` (→ `<ws>/GeniusSDK`), because GCS consumes GeniusSDK prebuilt rather than building it.
- GCS CI must download **three** dependency releases (thirdparty, SuperGenius, GeniusSDK), one more than the GeniusSDK workflow (which builds GeniusSDK itself). The GeniusSDK CMake auto-download fallback (curl in CommonBuildParameters.cmake lines 339–363) also works, but explicit `gh release download` steps matching D-07 are the intended path — deterministic and fail-fast.

**Release tag naming convention (Q4):** `<Target>[-<ABI>]-<branch>-<BuildType>`, where ABI segment appears only for Android (`arm64-v8a`/`armeabi-v7a`) and Linux (`x86_64`/`aarch64`). Asset file name: `<Target>[-<ABI>]-<BuildType>.tar.gz`. Verified live tags (2026-08-15):
- **SuperGenius**: all 12 matrix combos on develop (latest 2026-08-14), e.g. `OSX-develop-Debug`, `Linux-x86_64-develop-Release`, `Android-arm64-v8a-develop-Debug`, `Windows-develop-Release`, `iOS-develop-Debug`. [VERIFIED: `gh release list --repo GeniusVentures/SuperGenius`]
- **GeniusSDK**: all 12 combos on develop (2026-08-04). [VERIFIED: `gh release list --repo GeniusVentures/GeniusSDK`]
- **thirdparty**: all combos on develop (2026-07-24). [VERIFIED: `gh release list --repo GeniusVentures/thirdparty`]

So every matrix cell GCS CI needs has a published prebuilt for all three deps today. Note: GeniusSDK develop releases are from **2026-08-04**, but `GeniusSDKGetNode()` was added in commit `d550800` (Aug 3) — the prebuilt should contain it; the `neoswarm_storage` build-tree-dylib redirect (CMakeLists lines 30–39) handles the case where the *install-prefix* copy lags. **Pitfall:** on CI there is no GeniusSDK build tree (only the extracted release), so the redirect is a no-op and the release's `libGeniusSDK_shared` must export `GeniusSDKGetNode` — worth a link-time sanity check on the first CI run (a failure surfaces as "Undefined symbols: GeniusSDKGetNode", which the neoswarm CMakeLists comment explicitly documents as the signal to re-run `ninja install` in GeniusSDK / refresh the release).

### How gcs_core links GlobalDB (Q1)

Minimal change to `src/CMakeLists.txt`:
```cmake
target_link_libraries(gcs_core PUBLIC
    neoswarm_storage   # brings sgns::crdt_globaldb + sgns::GeniusSDK_shared transitively (both PUBLIC on neoswarm_storage)
    spdlog::spdlog
    fmt::fmt
)
```
`sgns::crdt_globaldb` and `sgns::GeniusSDK_shared` are `PUBLIC` links on `neoswarm_storage` [CITED: `GNUS-NEO-SWARM/src/storage/CMakeLists.txt:14-47`], so linking `neoswarm_storage` is sufficient. The include dirs for SuperGenius/ipfs-pubsub/libp2p/boost/wallet-core/GeniusSDK-src are also `PUBLIC` on `neoswarm_storage`, so `gcs_core` sources can `#include "storage/gcs_global_db.hpp"` and call its API without adding include paths. If `gcs_core.cpp` includes `gcs_global_db.hpp` (which forward-declares the heavy types — clean), no additional includes are needed; only if gcs_core touches `GossipPubSub`/`GlobalDB` headers directly would it need the thirdparty include dirs, which it inherits transitively anyway.

`gcs_ffi` (new, `src/ffi/CMakeLists.txt`):
```cmake
add_library(gcs_ffi SHARED gcs_core_ffi.cpp)   # name TBD — planner's discretion
target_link_libraries(gcs_ffi PRIVATE gcs_core)
target_compile_definitions(gcs_ffi PRIVATE GCS_FFI_EXPORTS)   # dllexport macro, mirrors NEOSWARM_CHAT_C_EXPORTS
```
Export-macro header pattern to copy verbatim: `GNUS-NEO-SWARM/src/genius_elm_chat_completions.h` lines 6–14 (`__declspec(dllexport/dllimport)` on `_WIN32`, empty elsewhere, `extern "C"` guard). Note this file uses `#if defined(_WIN32)` **in a header for export macros only** — that is the established in-repo idiom for FFI export headers and is distinct from the banned OS-ifdef-in-logic pattern.

## GcsGlobalDb Smoke Test (Q2)

`GcsGlobalDb::Initialize()` (no-arg) requires the full GeniusSDK init chain to have run (`GeniusSDKGetNode() != nullptr`), else `Error::SdkNotInitialized`. For CI smoke tests, use the **injected-pubsub overload** `Initialize(std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub>)` — the deliberate test seam. The complete, working recipe is `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp` (249 lines, in-tree, passing): [CITED: read of full file]

1. `SetUpTestSuite`: configure soralog via `ConfiguratorFromYAML` (minimal console-sink YAML) + `libp2p::log::setLoggingSystem()` — **required**, otherwise `GossipPubSub` construction crashes with "Logging system is not ready".
2. Per-test temp dir under `std::filesystem::temp_directory_path()` with a unique salt; removed in `TearDown`.
3. Stand up pubsub: `sgns::crdt::KeyPairFileStorage keyStore(keyDir)` → `GetKeyPair()` → `std::make_shared<GossipPubSub>(keyPair)` → `pubsub->Start(0, {}, "0.0.0.0", {}).get()` (port 0 = random free port).
4. `GcsGlobalDb::Config{ .m_dbPath = temp + "/db" }` → `Initialize(pubsub)` → expect success, `IsRunning() == true`.
5. Wait-condition template: `WaitForCondition(predicate, kWaitTimeout{25000ms})` — `condition_variable::wait_for(lock, slice)` polling loop with `kPollInterval{10ms}`, no `sleep_for`. Copy this template verbatim into the GCS test suite.
6. `Shutdown()` → `IsRunning() == false`; `pubsub->Stop()`.

For the CORE-05 CRDT round-trip the smoke test adds (beyond the lifecycle test above): after init, use the underlying GlobalDB APIs — `AddBroadcastTopic("gcs-chat-smoke")` + `AddListenTopic("gcs-chat-smoke")` + `Put(HierarchicalKey, buffer)` → `Get(key)` returns the buffer — topic names follow the architecture convention `gcs/chat/<roomname>`; for the smoke test a dedicated `gcs/chat/smoke-test` topic is appropriate. GlobalDB API verified: `Put` (with optional broadcast topic), `Get`, `AddBroadcastTopic`, `AddListenTopic`, `Start` [CITED: SuperGenius `src/crdt/globaldb/globaldb.hpp:89-225`]. `GcsGlobalDb` does not currently expose `Put/Get/AddBroadcastTopic` publicly (only lifecycle + IsRunning) — **the planner must decide**: either (a) add minimal pass-through accessors to `GcsGlobalDb` (small neoswarm change — requires care re: submodule PR scope), or (b) Phase 1's smoke test exercises CRDT through a `GossipPubSub::Publish` + topic membership only, deferring Put/Get to Phase 3. Option (a) is recommended — it makes CORE-05's substrate claim testable — but it touches the submodule, so the plan should sequence it as its own task with its own neoswarm-branch commit.

Topic create/join (success criterion 4): GossipSub topics are implicit — "create/join" = `GlobalDB::AddBroadcastTopic` + `AddListenTopic`, or raw `GossipPubSub::Publish(topic, ...)`/subscription. No explicit join API exists. [CITED: gossip_pubsub.hpp / globaldb.hpp]

## FFI Callback Mechanism (Q3)

**What exists in `neoswarm_ffi` today** [VERIFIED: file reads + repo-wide grep]:
- `neoswarm_ffi/ffigen.yaml` — ffigen config: `dart run ffigen --config ffigen.yaml`, entry-point header `../../src/genius_slm_chat_c.h` (**note: that header does not exist in the tree** — `src/` has `genius_elm_chat_completions.h` instead; the yaml is stale), output `lib/genius_slm_bindings_generated.dart`, `comments: style: any, length: full`.
- `neoswarm_ffi/src/flutter_slm_bridge.{c,h}` — stock Flutter plugin template (`sum`, `sum_long_running`), `FFI_PLUGIN_EXPORT` from `os_defines.h`, `dart_shared_lib` define, Android 16k-page-size link option (`-Wl,-z,max-page-size=16384`).
- `neoswarm_ffi/src/CMakeLists.txt` — minimal Flutter-plugin CMake (3.10 floor, SHARED lib, PUBLIC_HEADER).
- C++ C-ABI example: `GNUS-NEO-SWARM/src/genius_elm_chat_completions.{h,cpp}` — the real pattern: `NEOSWARM_ELM_CHAT_C_API` export macro, `extern "C"`, `noexcept`, heap-string returns freed by a paired `...StringFree`, global-mutex thread safety, int status codes. **This is the header style `gcs_ffi`'s session header should mirror.**
- Test-side: `GNUS-NEO-SWARM/test/ffi/test_genius_elm_ffi.cpp` exercises the C API from GTest — the model for the gcs_ffi echo test.

**What does NOT exist:** any use of `Dart_PostCObject`, `Dart_Port`, `ReceivePort`, `NativePort`, or `Pointer<NativeFunction>` callback registration — grep across GNUS-NEO-SWARM, SuperGenius, GeniusSDK, and `neoswarm_ffi/lib/*.dart` found zero hits. D-05's premise ("the same pattern neoswarm_ffi already uses") is inaccurate; only the ffigen scaffolding exists.

**The standard mechanism to implement** (well-established Dart VM C API; [ASSUMED] from Dart documentation knowledge — not verifiable against in-repo code because no in-repo usage exists):

C header (ffigen-compatible):
```c
#include <stdint.h>
// Dart_PostCObjectType is provided by dart_native_api.h (ships with Dart SDK);
// declare the function-pointer type explicitly so ffigen sees it:
typedef void (*GcsMessageCallback)(int64_t port, const char* json);

// Register the port: C++ stores it; when a message arrives, it posts a
// Dart_CObject (string) to that port via Dart_PostCObjectType.
GCS_FFI_API int gcs_on_message(GcsSession* handle, int64_t dart_port);
```
Dart side:
```dart
final receivePort = ReceivePort();
bindings.gcs_on_message(handle, receivePort.sendPort.nativePort);
receivePort.listen((msg) { /* msg is the posted string */ });
```
ffigen maps `int64_t` ↔ Dart `int`, function pointers ↔ `Pointer<NativeFunction<...>>`. The native library must link/include `dart_native_api.h` (`Dart_PostCObjectType Dart_PostCObject`); in a plain shared lib loaded by Flutter, the symbol resolves from the Flutter engine at load time — declare it via `#include "dart_native_api.h"` with the header copied or referenced from the Dart SDK, a common practice. [ASSUMED — needs one spike to confirm include strategy: copy `dart_native_api.h` + `dart_api_dl.c` into `src/ffi/` (the `package:ffigen`-documented approach) vs. weak-linkage declare. This is the one genuine implementation unknown in the phase; budget a small spike task.]

## CI/CD (D-06..D-09)

Copy `../GeniusSDK/.github/workflows/cmake.yml` (594 lines, read in full) with these deltas:

1. **Checkout**: `actions/checkout` with `submodules: "recursive"` at workspace root (GCS has submodules: `GNUS-NEO-SWARM`, `build` (cmaketemplate), `gendoc-template`, `src/app/scaffold`). The GeniusSDK workflow checks out into a `GeniusSDK/` subdir; GCS should checkout to the default workspace root.
2. **Add a third download step**: GeniusSDK release (tag `<Target>[-<ABI>]-<branch>-<BuildType>`, asset `<Target>[-<ABI>]-<BuildType>.tar.gz`) into `${{github.workspace}}/GeniusSDK`. thirdparty + SuperGenius steps copy verbatim.
3. **Delete the zkLLVM download step** (D-09). Caveat: `CommonCompilerOptions.cmake` auto-downloads zkLLVM if absent (curl from `GeniusVentures/zkLLVM` releases) — it is not skippable via a flag today because `GeniusNode.hpp` transitively includes ProofSystem headers. GCS CI will still pull the zkLLVM *headers* release implicitly at configure time unless the planner adds a `-DZKLLVM_BUILD_DIR=` pointing somewhere inert or a skip flag. Cheapest correct move: let the existing auto-download run (it's header-only consumption for GCS) and simply don't add the explicit step; D-09's "skip zkLLVM" then means "no explicit step, no assigner binaries" rather than "zero zkLLVM bytes". Flag for planner/user confirmation — see Open Questions.
4. **Layout expectation**: the auto-detect requires the three dep trees as *siblings of the GCS checkout* (`PROJECT_SUPER_ROOT` = parent of the found `thirdparty` = `get_third_party_dir` walks up from `build/cmake` looking for a dir containing `thirdparty` with the right git remote). The GeniusSDK workflow extracts deps *inside* the workspace (`${{github.workspace}}/thirdparty`, `.../SuperGenius`, `.../GeniusSDK`) and passes `-DTHIRDPARTY_BUILD_DIR` / `-DSUPERGENIUS_DIR` explicitly. For GCS, the workspace IS the repo root, so extracted `thirdparty/` at workspace root is found by `get_third_party_dir` **only if it has a `.git` with the GeniusVentures/thirdparty remote** — a tarball extraction has no `.git`. **Therefore GCS CI must pass `-DTHIRDPARTY_DIR=<ws>/thirdparty` explicitly** (input to `get_third_party_dir`'s `THIRDPARTY_DIR` early-out at CommonCompilerOptions.cmake:91) — then `PROJECT_SUPER_ROOT` = workspace and SuperGenius/GeniusSDK auto-detect from sibling dirs `<ws>/SuperGenius`, `<ws>/GeniusSDK` resolves cleanly. This is the one non-copy-paste wiring detail; the GeniusSDK workflow's explicit `-DTHIRDPARTY_BUILD_DIR=...` achieves the same end (bypasses discovery). Either works; passing `-DTHIRDPARTY_BUILD_DIR` (full build path, as GeniusSDK CI does) is the more proven option.
5. **ctest**: copy the SuperGenius test steps (lines 703–740): ctest on Linux (inside `dbus-run-session` + gnome-keyring stub — needed because GeniusSDK wallet touches the keyring; GCS's smoke tests don't init the SDK, so plain `ctest` may suffice, but copying the wrapper is harmless), OSX, Windows Release. Android/iOS are cross-compiles — **no ctest there** (matches SuperGenius: tests only run on Linux/OSX/Windows).
6. **Runner labels** (D-06): self-hosted `sg-ubuntu-linux` (Linux x86_64 + Android builds, container `ghcr.io/geniusventures/debian-bullseye:latest`), `sg-arm-linux` (Linux aarch64), `SG-WIN11`, `gv-OSX-Large` (OSX + iOS); fallbacks `ubuntu-latest`, `ubuntu-24.04-arm`, `windows-2022`, `macos-latest`. Copy the `resolve-runners` job verbatim (uses `secrets.GNUS_TOKEN_1`, `gh api` repo+org runner merge).
7. **Self-hosted cleanup** (D-09): copy the "Clean workspace" step verbatim (git clean/reset + `rm -rf thirdparty zkLLVM SuperGenius GeniusSDK`).
8. **Release upload**: optional this phase. The GeniusSDK workflow creates `<Target>[-<ABI>]-<branch>-<BuildType>` releases of its own build output — GCS will eventually need this (so the Flutter app and downstream repos can pull `gcs_ffi` binaries), and adding it now is ~30 lines copied verbatim. Recommend including (it also smoke-proves artifact packaging), but it's planner's discretion.
9. Trigger branches: `develop` + `main`, paths-ignore `.github/**`, plus `workflow_dispatch`.

## Architecture Patterns

### Recommended Project Structure (additions this phase)
```
src/
├── lib/
│   └── gcs_core.cpp          # fleshed out: session state owning GcsGlobalDb
├── ffi/
│   ├── CMakeLists.txt        # NEW — gcs_ffi SHARED target
│   ├── gcs_core.h            # NEW — single opaque-handle C API (D-01)
│   └── gcs_core_ffi.cpp      # NEW — handle table + thunk to gcs_core
├── app/                       # Flutter app (untouched this phase except eventual binding regen)
test/
├── CMakeLists.txt            # NEW — GCS-root test tree (BUILD_TESTING-gated; root CMakeLists already has the add_subdirectory hook via build/CommonBuildParameters.cmake:146-151)
├── test_wait_condition.hpp   # NEW — WaitForCondition template copied from neoswarm test
└── test_gcs_core_smoke.cpp   # NEW — GlobalDB lifecycle + CRDT round-trip + FFI init/echo
```

### Pattern 1: Opaque-handle C session API
**What:** All FFI calls take a `GcsSession*` handle from `gcs_init()`; C++ keeps a mutex-guarded handle→object map (or returns a raw pointer directly — simpler, standard for this shape; neoswarm's ELM API uses a global instance + mutex instead).
**When to use:** D-01 mandates it.
**Example (style copied from `genius_elm_chat_completions.h`):**
```c
// Source: GNUS-NEO-SWARM/src/genius_elm_chat_completions.h (export-macro + extern "C" idiom)
#if defined( _WIN32 )
#if defined( GCS_FFI_EXPORTS )
#define GCS_FFI_API __declspec( dllexport )
#else
#define GCS_FFI_API __declspec( dllimport )
#endif
#else
#define GCS_FFI_API
#endif

#if defined( __cplusplus )
extern "C"
{
#endif
    typedef struct GcsSession GcsSession;   // opaque
    GCS_FFI_API GcsSession* gcs_init( const char* db_path ) /* noexcept in C++ */;
    GCS_FFI_API int         gcs_join_topic( GcsSession* session, const char* topic );
    GCS_FFI_API int         gcs_publish( GcsSession* session, const char* topic, const char* utf8_payload );
    GCS_FFI_API int         gcs_on_message( GcsSession* session, int64_t dart_port );
    GCS_FFI_API void        gcs_shutdown( GcsSession* session );
#if defined( __cplusplus )
}
#endif
```

### Pattern 2: Injected-pubsub test seam (Tier 2 fixture)
**What:** `GcsGlobalDb::Initialize(pubsub)` overload lets tests run the full init chain against a real `GossipPubSub` on port 0 without bringing GeniusNode online.
**Source:** `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp` (verbatim recipe in "GcsGlobalDb Smoke Test" above).

### Pattern 3: Hard-required dependency linkage (no stubs)
**What:** `if(TARGET sgns::crdt_globaldb) ... else() message(FATAL_ERROR ...)`.
**Source:** `GNUS-NEO-SWARM/src/storage/CMakeLists.txt:14-18`. Apply the same to `gcs_core` → `neoswarm_storage` linkage: if the target is missing, fail configure.

### Anti-Patterns to Avoid
- **Polling from Dart** for incoming messages — D-05 forbids it; use the port-post callback.
- **Folding the FFI symbols into `gcs_core`** — D-02 requires a separate SHARED lib; static lib + Flutter dynamic loading don't mix on iOS/Android the way the plugin flow expects.
- **`sleep_for` in tests** — banned project-wide; copy the WaitForCondition template.
- **Building deps in CI** — D-07; any CI job compiling thirdparty/SuperGenius/GeniusSDK from source is a regression.
- **Per-module C headers** — D-01; one session header.
- **OS `#ifdef` in C++ logic** — banned (exception: the `_WIN32` dllexport macro block in FFI headers is the established idiom).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| CRDT store | Custom op log / vector-clock sync | `sgns::crdt::GlobalDB` via `GcsGlobalDb` | CRDT correctness (convergence, tombstones, DAG sync) is a research-grade problem; the component exists and is tested |
| Pub/sub transport | Raw libp2p gossipsub wiring | `GossipPubSub` (thirdparty/ipfs-pubsub) | Already integrated with SuperGenius keypair/identity |
| Wait-for-async in tests | `sleep_for` loops | `WaitForCondition` cv-polling template (copy from `test_gcs_global_db.cpp`) | Project rule; deterministic, fast-failing |
| Dart↔native string memory | `malloc` without paired free | `...StringFree` pattern (mirrors `GeniusElmStringFree`) | Cross-allocator heap mismatch crashes on Windows |
| Dependency acquisition in CI | `git clone` + build | `gh release download` of prebuilt tarballs | D-07; SuperGenius build is ~hours, download is ~minutes |
| Runner selection | Hardcoded `runs-on` | `resolve-runners` job (copy verbatim) | Self-hosted with hosted fallback, org+repo runner merge |

**Key insight:** every deceptively complex subsystem in this phase (CRDT, gossip, FFI codegen, CI runner orchestration) already has an in-repo or sibling-repo implementation. The phase's risk is **wiring mistakes** (wrong `-D` vars, wrong include paths, wrong link order), not algorithmic risk.

## Common Pitfalls

### Pitfall 1: Stale GeniusSDK install-prefix dylib
**What goes wrong:** Link fails with `Undefined symbols: GeniusSDKGetNode` because the imported `sgns::GeniusSDK_shared` points at the install prefix, which lags the build tree.
**Why it happens:** GeniusSDK's install step exports the C header + lib but `GeniusSDK.hpp` (C++ API) isn't installed; the dylib in the prefix can be older than the build tree.
**How to avoid:** `neoswarm_storage` CMakeLists already redirects `IMPORTED_LOCATION` to the build-tree dylib when present (lines 30–39). Locally: re-run `ninja install` in GeniusSDK if you see the undefined-symbol error. In CI: the extracted release has no build tree, so the release itself must be fresh enough (develop releases from 2026-08-04 postdate the Aug 3 commit — OK today, but re-verify if releases go stale).
**Warning signs:** "Undefined symbols: GeniusSDKGetNode" at link time.

### Pitfall 2: soralog logging system not configured in tests
**What goes wrong:** `GossipPubSub` construction crashes with "Logging system is not ready".
**Why:** SuperGenius `base::createLogger` asserts a configured soralog `LoggingSystem`.
**How to avoid:** Copy the `SetUpTestSuite` soralog YAML block from `test_gcs_global_db.cpp` verbatim.
**Warning signs:** Segfault/assert on first pubsub construction in a new test binary.

### Pitfall 3: `get_third_party_dir` remote check fails on CI tarballs
**What goes wrong:** CMake configure can't find thirdparty on CI even though it was downloaded and extracted to the workspace.
**Why:** `get_third_party_dir()` requires `<candidate>/.git` with a `GeniusVentures/thirdparty` remote; `gh release download` + `tar xzf` produces no `.git`.
**How to avoid:** Pass `-DTHIRDPARTY_BUILD_DIR=<ws>/thirdparty/build/<Target>/<BuildType>[/<ABI>]` explicitly (GeniusSDK CI pattern), which bypasses discovery via the `_THIRDPARTY_BUILD_DIR` cache path.
**Warning signs:** "Cannot find thirdparty directory required to build" then an attempted (failing or slow) auto-download in CI logs.

### Pitfall 4: ffigen entry-point header drift
**What goes wrong:** `dart run ffigen --config ffigen.yaml` fails or generates stale bindings.
**Why:** `neoswarm_ffi/ffigen.yaml` references `../../src/genius_slm_chat_c.h`, which does not exist (renamed/replaced by `genius_elm_chat_completions.h`). The yaml was never updated.
**How to avoid:** GCS's `ffigen.yaml` must point at the real `src/ffi/gcs_core.h`; add binding regeneration as an explicit dev-loop step in the plan (not CI this phase).
**Warning signs:** ffigen "file not found" or Dart bindings missing new functions.

### Pitfall 5: Dual entry-point confusion (root CMakeLists vs build/<Platform>)
**What goes wrong:** Configuring from the repo root (`cmake -S . -B out`) takes the root CMakeLists path, which skips `CommonCompilerOptions.cmake` — no thirdparty discovery, no `PROJECT_SUPER_ROOT`, and the GNUS-NEO-SWARM subdirectory then can't resolve `_THIRDPARTY_BUILD_DIR`.
**Why:** Two entry points exist; the root one is for parent-embedding scenarios where the parent already resolved deps.
**How to avoid:** Always configure via `cmake -S build/<Platform> -B build/<Platform>/<BuildType>`. CI and docs should only ever show this form.
**Warning signs:** `find_package` failures for OpenSSL/Boost/etc. immediately after configure starts.

### Pitfall 6: Dart callback posted after isolate/handle death
**What goes wrong:** Crash or leaked posts when the Dart side disposes the ReceivePort while C++ still holds the port.
**Why:** `Dart_PostCObject` on a closed port is safe (returns false), but C++ must also stop posting when the session shuts down, and `gcs_shutdown` must unregister before destroying state.
**How to avoid:** Session handle owns the registered port; `gcs_shutdown` clears it under the session mutex before tearing down `GcsGlobalDb`; the io thread (owned by `GcsGlobalDb`) must be joined (it is — `Shutdown()` joins) before the handle is freed.
**Warning signs:** Crash in `Dart_PostCObject` on shutdown path. [ASSUMED — standard Dart API_DL behavior; will be validated by the FFI smoke test's shutdown ordering]

## Code Examples

### Wait-condition template (copy verbatim)
```cpp
// Source: GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp:72-90
bool WaitForCondition( const std::function<bool()> &predicate, std::chrono::milliseconds timeout )
{
    std::mutex              mtx;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock( mtx );
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while ( std::chrono::steady_clock::now() < deadline )
    {
        if ( predicate() )
        {
            return true;
        }
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now() );
        const auto slice     = std::min( kPollInterval, remaining );
        cv.wait_for( lock, slice );
    }
    return predicate();
}
```

### GossipPubSub bring-up (test fixture)
```cpp
// Source: GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp:140-158
sgns::crdt::KeyPairFileStorage keyStore( keyDir );
auto keyPairResult = keyStore.GetKeyPair();
auto pubsub = std::make_shared<sgns::ipfs_pubsub::GossipPubSub>( keyPairResult.value() );
auto startError = pubsub->Start( 0, {}, "0.0.0.0", {} ).get();   // port 0 = ephemeral
```

### gcs_core link change
```cmake
# Source: pattern from GNUS-NEO-SWARM/src/storage/CMakeLists.txt
if(TARGET neoswarm_storage)
    target_link_libraries(gcs_core PUBLIC neoswarm_storage)
else()
    message(FATAL_ERROR "neoswarm_storage target not found — GNUS-NEO-SWARM must be configured before gcs_core")
endif()
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Build deps from source in CI | Prebuilt release tarballs (`gh release download`, tag `Target-ABI-branch-buildtype`) | Established across SuperGenius/GeniusSDK CI (current) | CI time drops from hours to minutes; GCS copies it |
| Flutter plugin template (`flutter create --template=plugin`) for FFI | Plain SHARED lib target loaded via `DynamicLibrary.open` + ffigen | neoswarm_ffi still uses plugin template; gcs_ffi per D-02 is a plain shared lib | Simpler CMake (no Flutter toolchain coupling in C++ build) |
| Dart polling loop for native events | NativePort push (`Dart_PostCObject`) | D-05 (this phase) | No in-repo precedent — see Open Questions |

**Deprecated/outdated:**
- `neoswarm_ffi/ffigen.yaml` entry point (`genius_slm_chat_c.h`) — stale; file doesn't exist. Do not copy that path.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | Dart NativePort mechanism details (`Dart_PostCObject`, `sendPort.nativePort`, ffigen `int64_t` mapping) follow the standard documented Dart API_DL pattern | FFI Callback Mechanism | Medium — no in-repo precedent exists to copy; a small spike task in the plan de-risks it. If the include/link strategy for `dart_native_api.h` differs, only `src/ffi/` files are affected. |
| A2 | `NativeCallable.listener` is a viable alternative but ReceivePort is preferred per D-05 | Alternatives Considered | Low — D-05 already locked the pattern. |
| A3 | GCS CI can rely on GeniusSDK develop releases (2026-08-04) containing `GeniusSDKGetNode` (added Aug 3) | Build System Wiring | Low-Medium — dates line up, but the first CI run is the real verification; failure mode is a clear link error with a documented fix (fresher GeniusSDK release). |
| A4 | Plain `ctest` suffices for GCS smoke tests on Linux (no gnome-keyring wrapper needed) because tests use the injected-pubsub seam and never init GeniusSDK | CI/CD item 5 | Low — copying the dbus-run-session wrapper verbatim is zero-cost insurance if wrong. |
| A5 | Letting cmaketemplate's zkLLVM auto-download run in CI (headers only) satisfies D-09's "skip zkLLVM" intent | CI/CD item 3 | Low — worst case CI downloads a tarball it doesn't compile against; confirm with user if "skip" meant "zero zkLLVM artifacts". |

## Open Questions

1. **D-05 references a neoswarm_ffi callback mechanism that doesn't exist.**
   - What we know: neoswarm_ffi is the stock Flutter plugin template; zero `Dart_PostCObject`/`NativePort` usage anywhere in the workspace's C++/Dart code.
   - What's unclear: Whether the user has seen this pattern in some other repo (GeniusWallet? a branch not fetched?) that should be the reference instead of the standard Dart API_DL idiom.
   - Recommendation: Plan a small spike task ("stand up minimal NativePort echo through gcs_ffi locally") before the full FFI task; implement the standard mechanism; flag to user at plan review.

2. **CRDT round-trip needs GcsGlobalDb accessors (Put/Get/AddBroadcastTopic) that don't exist yet.**
   - What we know: `GcsGlobalDb` public API is lifecycle-only (Initialize/Shutdown/IsRunning); `m_db` is private.
   - What's unclear: Whether Phase 1 may extend `GcsGlobalDb` in the GNUS-NEO-SWARM submodule (separate branch/PR there) or whether CORE-05's smoke proof should use raw `GossipPubSub::Publish` + GlobalDB accessed some other way.
   - Recommendation: Planner sequences a submodule task adding minimal read-only accessors (or pass-throughs) to `GcsGlobalDb`, committed on a neoswarm branch, before the GCS smoke test task. Confirm scope with user since STATE.md lists "Phase 1 depends on GlobalDB CRDT integration from GNUS-NEO-SWARM Phase 3" as a concern.

3. **zkLLVM in CI: header-only consumption vs. full skip.**
   - What we know: `CommonCompilerOptions.cmake` auto-downloads zkLLVM when absent; `neoswarm_storage` adds its include dir conditionally; D-09 says skip zkLLVM.
   - What's unclear: Whether "skip" means "no explicit CI step" (auto-download still runs, headers only) or "zero zkLLVM presence" (requires a CMake skip flag that doesn't exist yet).
   - Recommendation: Default to no-explicit-step (option 1); if the user wants zero-presence, planner adds a `GCS_SKIP_ZKLLVM`-style guard to CommonCompilerOptions — small but touches the cmaketemplate submodule.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| cmake | all builds | ✓ | 3.29.2 | — |
| ninja | all builds | ✓ | 1.13.2 | — |
| clang (Apple) | OSX build | ✓ | 17.0.0 | — |
| gh CLI | CI release downloads + local release inspection | ✓ | 2.93.0 | curl fallback in cmaketemplate auto-download |
| git | submodule ops | ✓ | 2.50.1 | — |
| thirdparty build tree | local builds | ✓ | `../thirdparty` present with build outputs | CI downloads release tarball |
| SuperGenius build tree | local builds | ✓ | `../SuperGenius/build/OSX/Debug` (resolved in existing cache) | CI downloads release |
| GeniusSDK build tree | local builds | ✓ | `../GeniusSDK/build/OSX/Debug` (resolved in existing cache) | CI downloads release |
| Android NDK | Android builds (CI) | ✗ (not checked locally; CI installs r27b per workflow) | r27b in CI | CI step downloads it |
| Existing local build proof | — | ✓ | `build/OSX/Debug/gcs_src/libgcs_core.a` exists | — |

**Missing dependencies with no fallback:** none for the planned work.
**Missing dependencies with fallback:** Android NDK locally (CI installs it; local Android builds need `ANDROID_NDK_HOME` set).

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | Google Test (thirdparty-provided; `find_package(GTest CONFIG REQUIRED)` in root CMakeLists under `BUILD_TESTING`) |
| Config file | none for GCS yet — GCS-root `test/CMakeLists.txt` is a Wave 0 deliverable; `build/CommonBuildParameters.cmake:146-151` already has the `add_subdirectory(${PROJECT_ROOT}/test ...)` hook gated on `BUILD_TESTS`/`BUILD_TESTING` |
| Quick run command | `ctest --test-dir build/OSX/Debug -R gcs --output-on-failure` |
| Full suite command | `ctest --test-dir build/OSX/Debug --output-on-failure` (includes neoswarm tests, which also build in the parent build) |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| CORE-05 | GlobalDB CRDT initializes; broadcast+listen topic wired; Put→Get round-trips locally over real GossipPubSub (port 0) | unit (integration-style fixture) | `ctest --test-dir build/OSX/Debug -R test_gcs_core_smoke --output-on-failure` | ❌ Wave 0 |
| (success criterion 3) | FFI shared lib loads; `gcs_init` returns handle; echo/round-trip call works; shutdown clean | unit (links gcs_ffi) | `ctest --test-dir build/OSX/Debug -R test_gcs_ffi --output-on-failure` | ❌ Wave 0 |
| (success criterion 4) | `gcs_join_topic` / GlobalDB `AddBroadcastTopic`+`AddListenTopic` succeed on a `gcs/chat/*` topic | covered by CORE-05 smoke test | same as CORE-05 | ❌ Wave 0 |
| (success criteria 1+5) | All 5 platforms build + ctest green | CI | workflow matrix (OSX/Linux/Windows run ctest; Android/iOS build-only) | ❌ Wave 0 (`.github/workflows/cmake.yml`) |

### Sampling Rate
- **Per task commit:** `cmake --build build/OSX/Debug -j && ctest --test-dir build/OSX/Debug -R gcs --output-on-failure`
- **Per wave merge:** `ctest --test-dir build/OSX/Debug --output-on-failure` (full parent-build suite)
- **Phase gate:** Full suite green locally on OSX + CI green on all 5 platforms before `/gsd:verify-work`

### Wave 0 Gaps
- [ ] `test/CMakeLists.txt` (GCS root) — registers smoke tests; gate on GTest found
- [ ] `test/test_wait_condition.hpp` — WaitForCondition template (copy from neoswarm)
- [ ] `test/test_gcs_core_smoke.cpp` — covers CORE-05 substrate (lifecycle + topic + Put/Get round-trip)
- [ ] `test/test_gcs_ffi.cpp` — covers FFI init/echo/shutdown
- [ ] `src/ffi/CMakeLists.txt` + `gcs_ffi` SHARED target
- [ ] `.github/workflows/cmake.yml` — CI (Wave 0 or first wave; without it criterion 5 is unverifiable)

## Security Domain

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | P2P identity via libp2p keypair (`KeyPairFileStorage`) — no user auth this phase |
| V3 Session Management | partial | FFI session handle: opaque pointer, no serialization, shutdown unregisters callbacks (Pitfall 6) |
| V4 Access Control | no | Membership/roles land in Phase 4 |
| V5 Input Validation | yes | FFI boundary: null-check all `const char*` from Dart, treat as untrusted UTF-8; length-bound copies; mirror the null-tolerant style of `genius_elm_chat_completions.cpp` |
| V6 Cryptography | yes (inherited) | Never hand-roll: libp2p keypair via SuperGenius `KeyPairFileStorage`; app-layer encryption deferred to v1.1 |

### Known Threat Patterns for C++/FFI/libp2p stack

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Use-after-free across FFI (Dart disposes while C++ posts) | Tampering/DoS | Session mutex + unregister-on-shutdown ordering (Pitfall 6) |
| Heap mismatch (Dart-side free of C++ malloc) | DoS | Paired `gcs_string_free` export; never let Dart free native memory directly |
| Null/malformed strings from Dart | Tampering | Null-check + validate at FFI boundary before touching C++ state |
| Double-shutdown of GcsGlobalDb | DoS | Already handled: `Shutdown()` is idempotent (CITED: gcs_global_db.hpp) |
| Double `gcs_init` | DoS | Mirror ELM pattern: thread-safe, subsequent calls no-op or return existing handle |

## Sources

### Primary (HIGH confidence)
- `GNUS-NEO-SWARM/src/storage/gcs_global_db.hpp` + `src/storage/CMakeLists.txt` — component API, linkage contract, dylib redirect
- `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp` — complete working smoke-test recipe (read in full)
- `GNUS-NEO-SWARM/cmake/CommonBuildParameters.cmake:290-479` — GeniusSDK/SuperGenius discovery, auto-download, find_package
- `cmake/CommonBuildParameters.cmake` + `build/CommonCompilerOptions.cmake` + `build/cmake/functions.cmake` (GCS root) — thirdparty discovery, entry-point chain
- `build/OSX/Debug/CMakeCache.txt` — proof the local build configures/builds with sibling-tree deps (libgcs_core.a exists)
- `../GeniusSDK/.github/workflows/cmake.yml` — consumer-repo CI template (read in full)
- `../SuperGenius/.github/workflows/cmake.yml:700-740` — ctest steps with keyring wrapper
- `gh release list` for SuperGenius / GeniusSDK / thirdparty — all 12 matrix combos exist on develop (verified 2026-08-15)
- `../GeniusSDK/src/GeniusSDK.h:200-240`, `GeniusSDK.hpp:22` — `GeniusSDKInit`/`GeniusSDKGetNode` signatures
- `thirdparty/ipfs-pubsub/src/ipfs_pubsub/gossip_pubsub.hpp:110-146`, `../SuperGenius/src/crdt/globaldb/globaldb.hpp:89-225` — pub/sub + CRDT API surface
- Repo-wide greps confirming absence of `Dart_PostCObject`/`NativePort` usage (the D-05 gap)

### Secondary (MEDIUM confidence)
- None — all load-bearing claims verified against files in this workspace.

### Tertiary (LOW confidence)
- Dart API_DL include/link strategy for a plain shared lib (A1) — standard pattern from Dart documentation knowledge; spike task planned.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — no new packages; all components read in-tree
- Architecture: HIGH — build/link/test wiring verified against live files and an existing successful local build
- Pitfalls: HIGH — each pitfall derived from an in-repo comment, CMake guard, or verified file state (including the ffigen.yaml staleness and the missing D-05 precedent)
- FFI callback mechanism: MEDIUM — the pattern is standard, but no in-repo precedent exists (see Open Question 1)

**Research date:** 2026-08-15
**Valid until:** 2026-09-14 (30 days; CI release tags roll forward continuously — re-check `gh release list` if planning happens after GeniusSDK publishes newer develop releases)
