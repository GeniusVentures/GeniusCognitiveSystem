# Phase 1: Foundation - Context

**Gathered:** 2026-08-15
**Status:** Ready for planning

<domain>
## Phase Boundary

The C++ chat core compiles, links against GlobalDB (via GNUS-NEO-SWARM's `neoswarm_storage` / `sgns::crdt_globaldb`), exposes a working FFI surface that Flutter can call, builds on all 5 platforms (macOS, Linux, Windows, iOS, Android), and has CI/CD on self-hosted runners proving it. No chat features yet (spaces/rooms/messages are Phases 2–3) — this phase is the skeleton: build system, GlobalDB init + pub/sub smoke test, FFI bridge, CI.

</domain>

<decisions>
## Implementation Decisions

### FFI Surface Shape
- **D-01:** Single opaque-handle "core session" API in `src/ffi/` — one `extern "C"` header (`gcs_core.h` or similar) where everything funnels through a session handle (`gcs_init` → handle; subsequent calls take the handle). NOT per-module headers. Rationale: Cubit-style C++ state storage with a thin Flutter UI wrapper; one entry point keeps the C API small and the state ownership clearly in C++.

### FFI Boundary / Library Layout
- **D-02:** Separate `gcs_ffi` **shared library** target (not folded into `gcs_core` static lib). `gcs_ffi` links `gcs_core` (and transitively `neoswarm_storage` → `sgns::crdt_globaldb` / `sgns::GeniusSDK_shared`). Flutter loads the shared lib (dylib/so/dll) directly. Mirrors the `neoswarm_ffi` plugin pattern but as a plain shared library the app links.
- **D-03:** [informational] `neoswarm_ffi` (GNUS-NEO-SWARM) may no longer be needed by the app once `gcs_ffi` exists — its SLM-facing functions get absorbed behind the GCS session API over time. Do NOT remove it in this phase; leave it working. Re-evaluate at Phase 6 (GCS Bot) when the bot path is wired through gcs_core.

### Async / Event Model Across FFI
- **D-04:** C++ owns essentially all state and logic. Dart is a thin wrapper: input (send text, select channel, change config) and output (render what C++ gives it).
- **D-05:** Data arriving from the network (incoming messages, CRDT sync updates) is pushed to Dart via **native→Dart callbacks** — ffigen `Pointer<NativeFunction>` + `ReceivePort`/NativePort pattern. Dart does NOT poll. This is the same pattern `GNUS-NEO-SWARM/neoswarm_ffi` already uses — reuse that mechanism (listener registration on the session handle, e.g. `gcs_on_message(handle, dartPort)`-style).

### CI/CD
- **D-06:** Follow the SuperGenius CI pattern (`.github/workflows/cmake.yml` in `../SuperGenius`): a `resolve-runners` job picks self-hosted runners by label (`sg-ubuntu-linux`, `sg-arm-linux`, `SG-WIN11`, `gv-OSX-Large`) with GitHub-hosted fallbacks, then a matrix job builds **Android (arm64-v8a, armeabi-v7a), iOS, OSX, Linux (x86_64, aarch64), Windows × Debug/Release**.
- **D-07:** Dependencies are **downloaded as prebuilt release tarballs** (thirdparty from `GeniusVentures/thirdparty` releases; additionally SuperGenius and GeniusSDK releases, tagged per `Target-ABI-branch-buildtype`) — NOT built per-run. GeniusSDK's workflow is the template for a consumer repo that downloads its deps this way.
- **D-08:** CI builds and ctests the **C++ core only** in Phase 1 (no Flutter app build in CI yet — desktop-first; Flutter smoke test of FFI is a local/dev-loop concern this phase).
- **D-09:** Self-hosted runner cleanup + container usage (`ghcr.io/geniusventures/debian-bullseye` for Linux/Android) copied from SuperGenius workflow; skip zkLLVM (GCS doesn't need it).

### Claude's Discretion
- Exact C API function names/signatures in the session header (planner/researcher design; must stay C-only, opaque handle, C++17).
- GTest smoke-test specifics (GlobalDB local pub/sub round-trip, FFI init/echo call) — must use wait-condition templates, never sleep_for.
- Whether `gcs_ffi` lives under `src/ffi/` (CMake target there) vs `src/lib/` — user moved structure to `src/ffi` for "Dart FFI -> C usage", so `src/ffi/` is the expected home.

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Architecture
- `.planning/notes/gcs-chat-architecture.md` — full chat architecture (spaces/rooms, topics `ipfs-pubsub://gcs/chat/<roomname>`, CRDT ops, membership, GCS bot message flow Flutter Cubit → FFI → C++ GCS core → GossipSub → GlobalDB). Phase 1 builds the bottom of this stack.

### CI/CD pattern to copy
- `../SuperGenius/.github/workflows/cmake.yml` — runner resolution, 5-platform matrix, prebuilt thirdparty release downloads, build/install/ctest, release upload.
- `../GeniusSDK/.github/workflows/cmake.yml` — consumer-repo variant (downloads SuperGenius + thirdparty releases); closest analog to what GCS CI must do.

### Build system
- `CMakeLists.txt` (repo root) — parent build: cxx17 toolchain, find_package deps, `add_subdirectory(GNUS-NEO-SWARM)` + `add_subdirectory(src)`.
- `GNUS-NEO-SWARM/src/storage/CMakeLists.txt` — `neoswarm_storage` target, `sgns::crdt_globaldb` / `sgns::GeniusSDK_shared` linkage (hard-required pattern; FATAL_ERROR if missing), build-tree dylib redirect for GeniusSDK.

### FFI pattern to reuse
- `GNUS-NEO-SWARM/neoswarm_ffi/ffigen.yaml` + `GNUS-NEO-SWARM/neoswarm_ffi/src/flutter_slm_bridge.c/.h` — working C-header → ffigen Dart bindings → callback pattern.

</canonical_refs>

<code_context>
## Existing Code Insights

### Reusable Assets
- `src/CMakeLists.txt` — `gcs_core` static library target already wired into the parent build (spdlog/fmt); Phase 1 extends this file (GlobalDB link via neoswarm_storage) and adds `src/ffi/` target.
- `src/lib/gcs_core.cpp` — placeholder anchor (`gcs::kGcsCoreVersion`); Phase 1 fleshes out init/shutdown around `GcsGlobalDb`.
- `GNUS-NEO-SWARM/src/storage/gcs_global_db.{hpp,cpp}` (`neoswarm_storage`) — GCS GlobalDB component from neoswarm Phase 3 Wave 1; the CRDT instance Phase 1 initializes and smoke-tests.
- `GNUS-NEO-SWARM/neoswarm_ffi/` — ffigen + NativePort callback machinery to mirror for `gcs_ffi`.
- `.github/workflows/deploy.yaml` — docs deploy only; build CI is net-new alongside it.

### Established Patterns
- Conditional-compilation rule: missing required libraries fail at CMake configure time (`FATAL_ERROR`), never degrade to stub — apply to GlobalDB/GeniusSDK linkage in gcs targets.
- No OS `#ifdef` in C++ source; platform specifics live in `os/<platform>/Platform.hpp` and CMake resolves platform via `CMAKE_SYSTEM_NAME`.
- C++17 ceiling; tests use wait-condition templates (no sleep_for).
- CMake layout convention from SuperGenius: `cmake -S build/<Target> -B build/<Target>/<BuildType>[/<ABI>]` with `-DTHIRDPARTY_BUILD_DIR=...` — GCS already has `build/` directory skeleton (OSX/Linux/Windows/Android/iOS dirs present).

### Integration Points
- Root `CMakeLists.txt` already does `add_subdirectory(GNUS-NEO-SWARM)` and `add_subdirectory(src)` — `gcs_ffi` slots into `src/` without touching the root file (only `src/CMakeLists.txt` + new `src/ffi/CMakeLists.txt`).
- Flutter app at `src/app/` (pubspec already has `ffi: ^2.2.0`); `src/app/pubspec.yaml` currently depends on `neoswarm_ffi` via path — will migrate to gcs bindings as they appear (not this phase).

</code_context>

<specifics>
## Specific Ideas

- "Cubit style C++ state storage with a thin flutter UI wrapper" — the Bloc/Cubit state objects live in C++; Flutter only renders and forwards user input.
- "probably only send/receive FFI calls to dart... C++ handles most everything" + callbacks instead of polling for arriving data.
- CI/CD: "kick-off those builds on our self-hosted runners and they will pick-up thirdparty & SuperGenius & GeniusSDK and download dependencies" — realized as the SuperGenius/GeniusSDK release-download pattern (D-07).

</specifics>

<deferred>
## Deferred Ideas

- Removing/retiring `neoswarm_ffi` in favor of `gcs_ffi` — re-evaluate at Phase 6 (GCS Bot) when inference routes through gcs_core.
- Flutter app builds in CI (iOS/Android app bundles) — later phase; Phase 1 CI covers C++ core only.
- zkLLVM in CI — not needed by GCS; skip.
- Message CRDT schema design — tracked in `.planning/todos/pending/design-message-crdt-schema.md`, resolves Phase 3.
- GCS bot identity mapping — tracked in `.planning/todos/pending/define-gcs-bot-identity-mapping.md`, resolves Phase 6.

</deferred>

---

*Phase: 1-Foundation*
*Context gathered: 2026-08-15*
