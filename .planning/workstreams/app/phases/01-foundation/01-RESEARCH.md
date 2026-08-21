# Phase 1: Foundation - Research

**Researched:** 2026-08-15; **UI strand refreshed:** 2026-08-21
**Domain:** C++ build system integration (cmaketemplate parent build), SuperGenius GlobalDB/CRDT, Dart FFI (ffigen + native→Dart callbacks), GitHub Actions CI with prebuilt release dependencies; **Flutter UI strand:** scaffold-based chat shell, Jinja2 codegen of message composites, CMake-gated Flutter wiring, scaffold theming
**Confidence:** HIGH (all build/link/test claims verified against live files in this workspace; release tags verified via `gh release list`; scaffold APIs verified against live submodule pin `ef16a0c`)

## Summary

Phase 1 is mostly a **wiring phase, not a design phase**: every hard problem (GlobalDB component, GeniusSDK linkage, ffigen pattern, CI release-download pattern) already has a working in-repo or sibling-repo implementation to copy. The `gcs_core` static library target already builds today (`build/OSX/Debug/gcs_src/libgcs_core.a` exists, configured via `cmake -S build/OSX -B build/OSX/Debug`). Per D-25 (2026-08-21) the `GcsGlobalDb` component MOVES out of the neoswarm submodule into the root repo `src/lib/` as its own `gcs_storage` target (it was misplaced there by neoswarm ws 03-01 before `feature/app-restructure` established `src/lib/`). Phase 1's real work is: (0) move + extend `GcsGlobalDb` into `src/lib/` with the four data-plane accessors (01-01), (1) link `gcs_core` to the moved `gcs_storage` target, (2) flesh out `gcs_core.cpp` around `GcsGlobalDb` lifecycle, (3) add a new `src/ffi/` `gcs_ffi` SHARED library with a single opaque-handle C API, (4) add a GTest smoke suite under a new GCS-root `test/` directory, and (5) author `.github/workflows/cmake.yml` copying the GeniusSDK consumer-repo workflow (which already downloads thirdparty + SuperGenius releases and builds 5 platforms).

**UI strand (added 2026-08-21):** The Flutter half is also mostly wiring. Every UI mechanism D-10..D-22 requires already exists in the pinned scaffold submodule (`ef16a0c` — verified live): the `AppScreenView` shell, `ScaffoldComposer`, `ScaffoldStateView` family, theme extensions (`ScaffoldPalette.lightPalette`/`defaultPalette`, `ScaffoldDimens.defaultDimens`), the Jinja2 `engine.py` with multi-`--template-dir` + StrictUndefined, and the per-component stamp/target CMake pattern (scaffold `CMakeLists.txt` §5–6) that our composite codegen driver copies. The genuinely new artifacts are: (1) `src/app/CMakeLists.txt` gating Flutter behind dart/flutter detection (ai-boss `FRONTEND_BUILD_ENABLED` pattern, relocated per D-16), (2) `src/app/templates/components/chat_*.jinja2` + `_vars.json` driving the bubble/code/media/flow composite triples, (3) a CMake `add_custom_command` loop mirroring scaffold §6 that renders each composite into `src/app/lib/generated/`, and (4) the rewritten `src/app/lib/main.dart` registering scaffold theme extensions and deleting the hardcoded purple `ColorScheme.fromSeed`. The FFI→Cubit wiring reuses the same Dart API_DL NativePort pattern identified in the C++ strand (Open Question 1, resolved).

One notable gap discovered: **the Dart NativePort/ReceivePort callback pattern referenced in D-05 does NOT currently exist in `neoswarm_ffi`** — that plugin is the Flutter plugin template's stock `sum()`/`sum_long_running()` example with no `Dart_PostCObject`, no `Dart_Port`, and no `Pointer<NativeFunction>` callback anywhere in GNUS-NEO-SWARM, SuperGenius, or GeniusSDK. The working callback mechanism to mirror is therefore the well-known Dart `Dart_PostCObjectType` + `NativePort` API pattern (documented below with the exact ffigen idioms), not an existing in-repo implementation. This is flagged in Open Questions — the planner should treat D-05's "reuse that mechanism" as "implement the standard mechanism in the same style as neoswarm_ffi's ffigen setup", since only the ffigen scaffolding exists to reuse.

**Primary recommendation (C++ strand):** Move + extend `GcsGlobalDb` into `src/lib/` as `gcs_storage` (01-01, D-25); extend `src/CMakeLists.txt` to link `gcs_core PUBLIC gcs_storage`; create `src/ffi/CMakeLists.txt` with a `gcs_ffi` SHARED target (pattern from `GNUS-NEO-SWARM/neoswarm_ffi/src/CMakeLists.txt` + export-macro style from `genius_elm_chat_completions.h`); create GCS-root `test/` with a wait-condition smoke test modeled line-for-line on the moved `test_gcs_global_db.cpp` (injected-pubsub seam, port 0, no GeniusSDK node needed); write CI as a near-verbatim copy of `../GeniusSDK/.github/workflows/cmake.yml` minus zkLLVM.

**Primary recommendation (UI strand):** Add `add_subdirectory(app)` to `src/CMakeLists.txt`; author `src/app/CMakeLists.txt` with a `FRONTEND_BUILD_ENABLED`-style gate (default OFF, dart/flutter `find_program` detection, WARNING-and-skip on absence) plus `app_pub_get` / `app_analyze` / `app_test` / `app_generate_chat_components` custom targets; `add_subdirectory(scaffold)` from `src/app/CMakeLists.txt` with `TEMPLATES_DIR=${CMAKE_CURRENT_SOURCE_DIR}/templates` and `GENERATED_DIR=${CMAKE_CURRENT_SOURCE_DIR}/lib/generated` set before the call; author `src/app/templates/components/chat_message_{bubble,code_block,media,flow}{,_cubit,_state}.dart.jinja2` + `chat_message_*_vars.json`; rewrite `main.dart` to register `[ScaffoldPalette.lightPalette, ScaffoldDimens.defaultDimens]` / `[ScaffoldPalette.defaultPalette, ScaffoldDimens.defaultDimens]` on light/dark `ThemeData` and compose the shell from `AppScreenView` + a left rail + `ScaffoldComposer`.

## User Constraints (from CONTEXT.md)

### Locked Decisions (C++ strand)
- **D-01:** Single opaque-handle "core session" API in `src/ffi/` — one `extern "C"` header where everything funnels through a session handle (`gcs_init` → handle; subsequent calls take the handle). NOT per-module headers.
- **D-02:** Separate `gcs_ffi` **shared library** target (not folded into `gcs_core` static lib). `gcs_ffi` links `gcs_core` (and transitively `neoswarm_storage` → `sgns::crdt_globaldb` / `sgns::GeniusSDK_shared`). Flutter loads the shared lib directly.
- **D-03:** `neoswarm_ffi` may no longer be needed by the app once `gcs_ffi` exists. Do NOT remove it in this phase; leave it working. Re-evaluate at Phase 6.
- **D-04:** C++ owns essentially all state and logic. Dart is a thin wrapper.
- **D-05:** Network data pushed to Dart via **native→Dart callbacks** — ffigen `Pointer<NativeFunction>` + `ReceivePort`/NativePort pattern. Dart does NOT poll. Listener registration on the session handle (e.g. `gcs_on_message(handle, dartPort)`-style).
- **D-06:** Follow SuperGenius CI pattern: `resolve-runners` job picks self-hosted runners by label (`sg-ubuntu-linux`, `sg-arm-linux`, `SG-WIN11`, `gv-OSX-Large`) with GitHub-hosted fallbacks, then a matrix job builds **Android (arm64-v8a, armeabi-v7a), iOS, OSX, Linux (x86_64, aarch64), Windows × Debug/Release**.
- **D-07:** Dependencies downloaded as **prebuilt release tarballs** (thirdparty from `GeniusVentures/thirdparty`; SuperGenius and GeniusSDK releases, tagged per `Target-ABI-branch-buildtype`) — NOT built per-run. GeniusSDK's workflow is the template.
- **D-08:** CI builds and ctests the **C++ core only** in Phase 1 (no Flutter app build in CI). The Flutter gate in D-16 must default OFF / skip cleanly when dart+flutter are absent so this CI path is unaffected.
- **D-09:** Self-hosted runner cleanup + container usage (`ghcr.io/geniusventures/debian-bullseye` for Linux/Android) copied from SuperGenius workflow; skip zkLLVM.

### Locked Decisions (UI strand, D-10..D-25)
- **D-10:** **Pure-scaffold chat UI.** Drop `flutter_chat_ui` + `flutter_chat_core` from `src/app/pubspec.yaml`. Chat UI composed from scaffold primitives: `ScaffoldComposer` (message input), surfaces/cards (message list), state views/toasts (chrome). Multi-variant chat widgets are data-driven composites per D-17. Adopt `ScaffoldStreamingRichText` / `ScaffoldCodeBlock` when scaffold Phases 9–11 land on develop (additive, no blocker).
- **D-11:** **Scaffold app-shell structure.** Screens follow the scaffold `app_screen_view` pattern with per-screen Cubits; a session Cubit owns the FFI session handle (D-01/D-04). One-off screens live in `src/app/lib/` as plain Dart (ai-boss consumer pattern); multi-variant composites are generated per D-17.
- **D-12:** **Scaffold theming is the source of truth.** `src/app/scaffold/design_tokens.json` + scaffold `theme/` package. Delete the hardcoded purple `ColorScheme.fromSeed` in `src/app/lib/main.dart`.
- **D-13:** **Track scaffold `develop`.** Pin bumped 2026-08-20 to `ef16a0c` (gains Phase 8 atoms — `ScaffoldComposer`, `ScaffoldChip/ChipGroup`, `ScaffoldDisclosure`, `ScaffoldTraceList`, light palette — plus fixes `1762a45` template-dir flag and `ef16a0c` `--api-specs-dir` override). **Verified live: the submodule is at `ef16a0c83ba1...` today.**
- **D-14:** **Never edit scaffold generated families.** Scaffold `lib/` is a read-only public contract; app-side composites import `package:frontend_scaffold` atoms.
- **D-15:** **Build from repo root** via the cmaketemplate chain: `build/OSX/CMakeLists.txt` → `build/CommonBuildParameters.cmake` → `cmake/CommonBuildParameters.cmake` (the add_subdirectory hub). Template-only regeneration: scaffold's `scaffold_generate_templates` / `generate_all_components` targets (no `frontend_template` target exists — verified).
- **D-16:** **`src/CMakeLists.txt` owns the whole src tree** — `gcs_core`, `src/ffi/` `gcs_ffi`, AND the Flutter app under `src/app/` via `add_subdirectory(app)`. No frontend wiring in `cmake/CommonBuildParameters.cmake`. Dart/flutter detection + gating lives inside `src/CMakeLists.txt` / `src/app/CMakeLists.txt`.
- **D-17:** **Data-driven chat composites.** Multi-variant chat widgets (message bubble with roles × states; code block; media; the interleave-capable message flow) are **generated from our own `.jinja2` templates + `_vars.json` via scaffold `engine.py`** — templates + vars in `src/app/templates/`, custom target drives `engine.py` with `--template-dir` (ours + scaffold's), scaffold `design_tokens.json` (D-12), our vars. Never hand-edit generated output.
- **D-18:** **Message roles:** `user` (with `self`/`peer` variant flag), `assistant`, `system`. `error` is NOT a role.
- **D-19:** **Message states:** `pending`, `streaming`, `thinking`, `complete`, `error`. `thinking` = typing-dots indicator, distinct from `streaming`.
- **D-20:** **Text-only bubbles; three composites sharing an interleave-capable flow envelope.** Code and media are their own composites rendered inline (iMessage model). The message-flow list interleaves `[text bubble][code block][text bubble][media]…` via per-item variant data.
- **D-21:** **App shell = left rail with space→room tree** (Slack/Discord-style); rail switching works in Phase 1. Main pane = active room's message flow + composer.
- **D-22:** **Theming via scaffold's existing light/dark toggle** — `ScaffoldPalette.lightPalette` (light ThemeData) + `defaultPalette` (dark ThemeData); D-12 design_tokens.json drives both.
- **D-23:** **App class is `GCSChat`** — the stock `src/app/lib/main.dart` `MyApp` is renamed/replaced by `GCSChat`; `src/app/test/widget_test.dart` (references the nonexistent `MyApp`, fails analyze today) is rewritten to pump `GCSChat`, not patched.
- **D-24:** **Generate both halves per message composite (Dart widget + C++ Cubit-state), contract-agnostic.** Adopts ai-boss frontend-templates 07 ownership model (`07-CONTEXT.md` D-01..D-08): always generate both halves (their D-02); the C++ store is contract-agnostic with a pluggable importer seam (their D-04 — FFI/JSON is Phase 1's only importer, but the wire format never leaks into the store's public surface); reuse the contract types directly as state fields (their D-05 — no independent canonical struct + translate); the message contract (`_vars.json` → FFI payload) is append-only from day one (their D-06/D-07 — only add fields). C++ state template lives in `src/app/templates/cpp/` (scaffold's `templates/cpp/` is an empty consumer-owned placeholder). Ownership stays per-instance at composition (their D-01 — consistent with D-04/D-05/D-11).
- **D-25:** **`GcsGlobalDb` moves into the root repo.** Relocate `GNUS-NEO-SWARM/src/storage/gcs_global_db.{hpp,cpp}` + `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp` into the app structure under `src/lib/` as its own CMake target (e.g. `gcs_storage`) linking `sgns::crdt_globaldb` / pubsub directly. `gcs_core` links the moved target in `src/lib/` — NOT `neoswarm_storage`. It was misplaced in the submodule by neoswarm ws 03-01 (`bc6ab3c`) before `feature/app-restructure` established `src/lib/` as the app C++ home. 01-01 becomes "move + extend" (git mv preserves history, then add the data-plane accessors); the neoswarm `03-gcs-globaldb-integration` phase (03-02/03-03 pending) gets a pointer-update note.

### Claude's Discretion
- Exact C API function names/signatures in the session header (C-only, opaque handle, C++17).
- GTest smoke-test specifics (GlobalDB local pub/sub round-trip, FFI init/echo call) — wait-condition templates, never sleep_for.
- Whether `gcs_ffi` lives under `src/ffi/` (expected home per user) vs `src/lib/`.
- FFI transport mechanism for the generated C++ Cubit-state halves (per ai-boss frontend-templates phase 07 discussion: Claude's discretion; must be C++17-clean — see "Composite C++ Half" below). **Resolution direction locked by D-24: FFI/JSON is Phase 1's only importer; the store surface is contract-agnostic (their D-04) so the FFI JSON types never leak into it.**

### Deferred Ideas (OUT OF SCOPE)
- Removing/retiring `neoswarm_ffi` (Phase 6).
- Flutter app builds in CI (later phase).
- zkLLVM in CI.
- Message CRDT schema design (Phase 3).
- GCS bot identity mapping (Phase 6).
- Adopting scaffold Phase 9–10 atoms beyond chat (`ScaffoldChart`/`ScaffoldChartScrubber`, `ScaffoldSelectionActions`) — not Phase 1 scope.
- Code-block and media composite *content* — Phase 1 generates the template skeletons only; the flow exercises text-only items.
- Destructive chat actions (delete/leave) — no destructive actions ship in Phase 1.

## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| CORE-05 | Messages sync via CRDT across all room participants | Phase 1 proves the substrate: `GcsGlobalDb` wraps `sgns::crdt::GlobalDB` (Put/Get/AddBroadcastTopic/AddListenTopic verified in SuperGenius `globaldb.hpp`); smoke test exercises init → `AddBroadcastTopic` + `AddListenTopic` on a test topic → `Put` → local CRDT store contains the value (Get round-trip). Real multi-participant sync semantics land in Phase 3 with the message schema; Phase 1 establishes the CRDT is initialized, started, and round-trips locally over a real GossipPubSub. |
| (UI strand — D-10..D-22) | Chat app shell renders from scaffold primitives; message composites generated from data; theming from scaffold tokens; FFI-pushed state drives Cubit projections | UI-SPEC.md locks every token value against scaffold pin `ef16a0c`; the codegen path (engine.py multi-template-dir + stamp targets) is verified in scaffold `CMakeLists.txt` §5–6; the CMake gating pattern is verified in ai-boss `cmake/CommonBuildParameters.cmake:136-157`; shell/composer/state-view APIs verified against live scaffold `lib/components/`. |

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| CRDT store lifecycle (init/start/shutdown) | C++ core (`gcs_core` via `GcsGlobalDb`) | — | `GcsGlobalDb` already owns this; gcs_core wraps it (D-04) |
| Pub/sub transport (GossipSub topics) | C++ core (SuperGenius `GossipPubSub` via GeniusSDK node or injected) | — | libp2p stack lives in C++; Dart never touches sockets |
| FFI symbol export / C ABI | `gcs_ffi` shared lib | — | D-01/D-02: thin translation layer over gcs_core, owns no state |
| Async event delivery to UI | `gcs_ffi` (Dart_PostCObject) | Dart `ReceivePort` | D-05: push not poll; C++ posts, Dart receives |
| State ownership (session, rooms later) | C++ core (opaque handle table in `gcs_ffi`) | — | D-01 opaque handle = pointer into C++-owned session map |
| Message variant axes (role/state/payload) | Generated Dart composites (`lib/generated/chat_*`) | FFI event payload (C++ sets the axis values) | D-17..D-20: rendering variants are data-driven in Dart; the *values* come from C++-pushed events (D-04: Cubits never synthesize state locally) |
| UI rendering & interaction (rail, flow, composer) | Flutter app (`src/app/lib/`) | — | D-11/D-21: thin UI wrapper; composer forwards raw strings, Cubits project FFI state |
| UI theming | Scaffold `theme/` extensions registered by host `main.dart` | — | D-12/D-22: `ThemeData(extensions: ...)` pattern; scaffold `lib/` untouched (D-14) |
| Composite codegen | CMake custom targets in `src/app/CMakeLists.txt` driving scaffold `engine.py` | scaffold `CMakeLists.txt` §5–6 (pattern source) | D-17: our templates + vars, scaffold's engine + tokens |
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

### Core (UI strand — all in-tree via scaffold submodule)
| Component | Version/Pin | Purpose | Why Standard |
|-----------|-------------|---------|--------------|
| `frontend_scaffold` (scaffold submodule) | pin `ef16a0c` [VERIFIED: `git rev-parse HEAD` in `src/app/scaffold` 2026-08-21] | M3 atom library + theme extensions + Jinja2 codegen | D-10/D-12/D-14: sole UI kit; locked by CONTEXT |
| `scaffold_codegen.engine` (`src/app/scaffold/tools/scaffold_codegen/engine.py`) | in submodule | Jinja2 render pipeline: multi-`--template-dir` (FileSystemLoader, first match wins), `StrictUndefined`, `--tokens`/`--vars` merge (vars override tokens) [CITED: engine.py:57-104, 184-288] | D-17 mandates it; identical CLI used by scaffold's own CMake targets |
| `flutter_bloc` `^9.0.0` (scaffold transitive dep) | scaffold pubspec [CITED: scaffold pubspec.yaml dependencies] | Cubit base classes for per-screen Cubits and generated cubit halves | D-11 per-screen Cubits; scaffold's own cubit atoms (`scaffold_card_cubit` etc.) build on it |
| `AppScreenView` | scaffold `lib/components/app_screen_view.dart` | Top-level shell: `SafeArea` + `CustomScrollView` with `body` + bottom-anchored `footer` [CITED: read of file] | D-11 app-shell pattern |
| `ScaffoldComposer` | scaffold `lib/components/scaffold_composer.dart` | Message input: `hintText`, `badgeRow`, `actionRow`, `onSubmit(String)`, `disabled`, `maxLines`, `focusNode` [CITED: read of API surface lines 31-68] | D-10/D-21 composer; consumes `context.palette`/`context.dimens` internally |
| `ScaffoldStateView` / `ScaffoldStateViewCubit` | scaffold `lib/components/scaffold_state_view*.dart` | Empty/error/loading/unavailable/success states with instanceId-driven Cubit family [CITED: state_view API lines 48-133] | UI-SPEC empty/error copy contract; rail + pane states |
| `ScaffoldPalette` / `ScaffoldDimens` | scaffold `lib/theme/` | ThemeExtension pair; `defaultPalette` (dark) + `lightPalette` (light); `defaultDimens` [CITED: palette lines 103/132; dimens lines 115-137] | D-12/D-22 theming source of truth |
| Jinja2 | 3.1.6 installed locally [VERIFIED: `python3 -c "import jinja2"` 2026-08-21] | Template rendering under engine.py | engine.py hard-requires it (exit 1 if absent) |

### Supporting
| Component | Purpose | When to Use |
|-----------|---------|-------------|
| GTest (thirdparty) | Test framework | All C++ smoke tests; `find_package(GTest CONFIG REQUIRED)` already in root CMakeLists (guarded by `BUILD_TESTING`) |
| ffigen (Dart dev-dependency) | Generate Dart bindings from the C header | Local dev loop when the `gcs_ffi` header changes; config modeled on `neoswarm_ffi/ffigen.yaml` |
| Dart `ffi` package `^2.2.0` | `Pointer<NativeFunction>`, `NativePort` types | Already in `src/app/pubspec.yaml` [VERIFIED: read of pubspec] |
| `Dart_PostCObject` (Dart VM C API `dart_native_api.h`) | Native→Dart async message post | The D-05 callback mechanism; header ships with the Dart SDK / Flutter engine |
| `dart` / `flutter` CLIs | Pub get, analyze, test, app build | Local dev loop only (D-08 keeps Flutter out of CI); detected via `find_program` in `src/app/CMakeLists.txt` [VERIFIED present: dart 3.11.5, Flutter 3.41.9 via `thirdparty/flutter/bin`] |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| Injected-pubsub smoke test (`GcsGlobalDb::Initialize(pubsub)`) | Full GeniusSDK init in tests (`GeniusSDKInit` → node online) | Full SDK init brings up the whole GeniusNode (wallet, network) — slow, needs ports/config, flaky in CI. The injected seam exists precisely for this (Tier 2 fixture pattern; CITED: gcs_global_db.hpp doc comment). Use full-SDK init only in the FFI-level test if planner wants an end-to-end init path — recommend deferring that to a manual/integration test, not the CI smoke suite. |
| `Dart_PostCObject` + ReceivePort | `NativeCallable.listener` (Dart 3.x isolate-callback style) | NativeCallable avoids dart_native_api.h but requires Dart-side isolate management and is newer/less documented in ffigen-generated code. D-05 names the ReceivePort/NativePort pattern explicitly — follow the decision. [ASSUMED — Dart version nuance; both work with `ffi: ^2.2.0`] |
| Generated message composites (D-17) | Hand-written Dart with a runtime `switch` on role/state enums | D-17 explicitly forbids hand-switched Dart. The generated-variant approach keeps 4 roles × 5 states × 3 payloads (the axes from D-18/D-19/D-20) from exploding into hand-maintained widget code, and matches scaffold's proven card/state/search_bar pattern. |
| `add_subdirectory(scaffold)` from `src/app/CMakeLists.txt` | Consumer-side copy of the engine-driver CMake (standalone `add_custom_command` loop in our file, no scaffold add_subdirectory) | Standalone driver avoids depending on scaffold's CMake cache-var contract, but re-implements the pattern and drifts when scaffold evolves. The ai-boss pattern (add_subdirectory + cache vars) is the canonical consumer path (D-14/D-15) — use it. |
| `GENERATED_DIR=src/app/lib/generated` (committed) | `GENERATED_DIR=${CMAKE_BINARY_DIR}/generated` (build-tree, gitignored) | Build-tree output means `dart analyze` can't resolve generated imports without a build having run, and diverges from scaffold's own committed-families convention. Committed `lib/generated/` matches the "never hand-edit, regenerate + drift-check" rule (D-14) and keeps the pure-Dart toolchain self-contained. See Pitfall U5. |

**Installation:** None. No new external packages are introduced this phase. All C++ deps come from prebuilt thirdparty/SuperGenius/GeniusSDK trees; Dart deps (`ffi`, `ffigen`) are already in `src/app/pubspec.yaml` / dev-dependencies pattern of `neoswarm_ffi`. The UI strand **removes** two pub packages (`flutter_chat_ui`, `flutter_chat_core` per D-10) and adds none — `flutter_bloc` arrives transitively via `frontend_scaffold` (path dependency already in pubspec).

## Package Legitimacy Audit

| Package | Registry | Age | Downloads | Source Repo | slopcheck | Disposition |
|---------|----------|-----|-----------|-------------|-----------|-------------|
| *(none — this phase adds no external packages)* | — | — | — | — | — | — |
| `flutter_chat_ui`, `flutter_chat_core` | pub.dev | — | — | — | — | **REMOVED per D-10** (dependency removal, not install) |

**Packages removed due to slopcheck [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

Note: slopcheck was run and is available. The only Dart-side package this phase touches is the already-present `ffi: ^2.2.0` (Dart pub, not PyPI — slopcheck's pypi check is inapplicable). All C++ dependencies are prebuilt release artifacts from the three sibling GeniusVentures repos, not registry packages. `flutter_bloc` is consumed transitively through the in-tree scaffold submodule (path dependency), not resolved from pub.dev as a new direct dependency.

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

### How gcs_core links GlobalDB (Q1) — SUPERSEDED by D-25 (2026-08-21)

Pre-D-25 this section linked `gcs_core PUBLIC neoswarm_storage`. D-25 relocates `GcsGlobalDb` into the root repo (`src/lib/`) as its own target, so the link changes: `gcs_core` links the **moved `gcs_storage` target in `src/lib/`**, NOT `neoswarm_storage`. The moved target links the CRDT deps directly:

Minimal change to `src/CMakeLists.txt` (after 01-01's move):
```cmake
# gcs_storage target created by 01-01 in src/lib/CMakeLists.txt (or src/CMakeLists.txt):
add_library(gcs_storage STATIC lib/gcs_storage/gcs_global_db.cpp)   # moved from GNUS-NEO-SWARM/src/storage/
target_link_libraries(gcs_storage PUBLIC
    sgns::crdt_globaldb     # direct — was previously inherited via neoswarm_storage
    sgns::GeniusSDK_shared  # direct
)
target_include_directories(gcs_storage PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lib)

target_link_libraries(gcs_core PUBLIC
    gcs_storage        # the moved component (D-25) — was neoswarm_storage pre-D-25
    spdlog::spdlog
    fmt::fmt
)
```
The thirdparty/GeniusSDK include dirs that were `PUBLIC` on `neoswarm_storage` must be re-declared on `gcs_storage` (or inherited by linking the same `sgns::*` targets, which carry their own interface includes). The pre-D-25 note about `neoswarm_storage` providing transitive includes is preserved below for reference, but the moved target owns its own include surface now.

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

## UI Strand — D-16 Build Wiring

**Pattern source:** `../apps/genius-ai-boss/cmake/CommonBuildParameters.cmake:136-157` (the `FRONTEND_BUILD_ENABLED` gate) + `../apps/genius-ai-boss/frontend/CMakeLists.txt` (the per-tool custom targets). Per D-16, GCS relocates this gating from the root-level CommonBuildParameters into `src/CMakeLists.txt` / `src/app/CMakeLists.txt`.

### Wiring shape

`src/CMakeLists.txt` (one-line addition at the bottom — the existing `gcs_core` target block is untouched):
```cmake
add_subdirectory(app)
```

`src/app/CMakeLists.txt` (new file; modeled on ai-boss, all targets gated):
```cmake
# GCS Flutter app — gated behind dart/flutter detection (D-16, D-08).
# Default OFF so the C++-only CI path (D-08) configures cleanly.
option(FRONTEND_BUILD_ENABLED "Enable Flutter/Dart app targets" OFF)

if(NOT FRONTEND_BUILD_ENABLED)
    return()
endif()

find_program(DART_EXECUTABLE dart)
find_program(FLUTTER_EXECUTABLE flutter)
find_program(PYTHON3_EXECUTABLE python3)

if(NOT DART_EXECUTABLE OR NOT FLUTTER_EXECUTABLE)
    message(WARNING "FRONTEND_BUILD_ENABLED=ON but dart/flutter not found — skipping app targets.")
    return()
endif()

# Scaffold codegen (D-17): cache vars MUST be set before add_subdirectory.
# TEMPLATES_DIR points engine.py at OUR templates; the scaffold's own
# templates are always included by its CMakeLists (Source A).
set(TEMPLATES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/templates" CACHE STRING
    "GCS chat composite templates (consumed by scaffold engine.py)")
set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/generated" CACHE STRING
    "GCS app generated composite output (committed; never hand-edit)")
set(FRONTEND_TARGET "flutter" CACHE STRING "Scaffold codegen target")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/scaffold/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/scaffold scaffold)
endif()

# Pub / analyze / test targets — ai-boss frontend/CMakeLists.txt §1, §2, §7 pattern.
add_custom_target(app_pub_get
    COMMAND ${DART_EXECUTABLE} pub get
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Resolving app Dart dependencies"
)
add_custom_target(app_analyze
    COMMAND ${DART_EXECUTABLE} analyze --fatal-infos
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS app_pub_get app_generate_chat_components
)
add_custom_target(app_test
    COMMAND ${FLUTTER_EXECUTABLE} test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS app_pub_get app_generate_chat_components
)
```

Key wiring facts (all verified):
- `cmake/CommonBuildParameters.cmake:503` already does `add_subdirectory(${PROJECT_ROOT}/src ...)` — so `src/CMakeLists.txt` → `add_subdirectory(app)` → `add_subdirectory(scaffold)` chains through the existing hub with **zero changes to cmake/ or build/** (D-16 as locked).
- Scaffold's CMakeLists is standalone-safe (`if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR) project(...)` guard, `CMakeLists.txt:12-14`) and additive under a parent.
- Scaffold cache vars consumed before `add_subdirectory(scaffold)`: `ENGINE_SCRIPT`, `DESIGN_TOKENS`, `GENERATED_DIR`, `FRONTEND_TARGET`, `API_SPECS_DIR`, `TEMPLATES_DIR` (all `CACHE STRING` with scaffold-local defaults, `CMakeLists.txt:84-93`). GCS sets only `TEMPLATES_DIR`, `GENERATED_DIR`, `FRONTEND_TARGET`.
- **`API_SPECS_DIR` caveat:** scaffold `CMakeLists.txt:32-41` defaults API spec resolution to `<scaffold-parent>/api-specs` — which for GCS is `src/app/api-specs/` (does not exist). The `frontend_generate_api` target would fail if invoked. Mitigation: GCS never invokes `frontend_generate_api` / `frontend_all` / `frontend_build_*` (no OpenAPI specs this phase); only `scaffold_generate_templates` and our own composite targets are referenced. Optionally set `-DAPI_SPECS_DIR=` explicitly to a dummy to silence; planner's call. The `ef16a0c` pin added the `--api-specs-dir` override flag precisely for this class of problem.
- **List-form flag bug (`1762a45` fix, present at pin):** when passing optional dir flags in scaffold-style custom commands, use CMake list form (`set(_flag --template-dir "${DIR}")`) so the flag expands to TWO argv tokens. A single space-embedded string reaches argparse as one token and fails. This bug class is documented in scaffold's own CMakeLists comments — copy the idiom exactly.

## UI Strand — D-17 Engine Driver (composite codegen targets)

**Pattern source:** `src/app/scaffold/CMakeLists.txt` §6 (lines 204-319, the `_component_names` loop) — read in full. Each component renders THREE files from three parallel `add_custom_command` blocks sharing one `_vars.json`: widget (`${_comp}.dart.jinja2` → `${_file_stem}.dart`), cubit (`${_comp}_cubit.dart.jinja2` → `${_file_stem}_cubit.dart`), state (`${_comp}_state.dart.jinja2` → `${_file_stem}_state.dart`; the `state` component special-cases to `state_view_state`). Each block: stamp file under `${CMAKE_CURRENT_BINARY_DIR}/template_gen/`, `DEPENDS` on template + tokens + engine + vars, per-component named target `generate_component_${_comp}`, aggregate `generate_all_components`.

GCS's driver (in `src/app/CMakeLists.txt`, after the scaffold add_subdirectory) copies §6 with our component set. Note: scaffold's §6 loop only covers scaffold's OWN six components (its `_component_names` is hardcoded, `CMakeLists.txt:208-215`) — **scaffold does NOT auto-render consumer `components/` templates from `TEMPLATES_DIR`** (Source B is restricted to `base/*.jinja2` standalone templates, `CMakeLists.txt:143-179`). Therefore GCS must declare its own composite loop — this is expected and matches the ai-boss "consumer owns parameterized rendering" contract documented in scaffold's §5/§6 comments.

```cmake
# --- GCS chat composites (D-17/D-20): bubble, code_block, media, flow ----
# Mirrors scaffold CMakeLists.txt §6: one stamp triple per composite.
set(_chat_templates_dir "${CMAKE_CURRENT_SOURCE_DIR}/templates/components")
set(_chat_composites
    chat_message_bubble
    chat_message_code_block
    chat_message_media
    chat_message_flow
)
set(_chat_stamps "")
set(_chat_outputs "")

foreach(_comp ${_chat_composites})
    set(_vars "${_chat_templates_dir}/${_comp}_vars.json")

    foreach(_kind "" "_cubit" "_state")
        set(_template "${_chat_templates_dir}/${_comp}${_kind}.dart.jinja2")
        set(_stamp "${CMAKE_CURRENT_BINARY_DIR}/template_gen/${_comp}${_kind}.stamp")
        set(_output "${GENERATED_DIR}/chat/${_comp}${_kind}.dart")

        add_custom_command(
            OUTPUT ${_stamp} ${_output}
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_CURRENT_BINARY_DIR}/template_gen"
            COMMAND ${PYTHON3_EXECUTABLE} ${ENGINE_SCRIPT}
                --template "components/${_comp}${_kind}.dart.jinja2"
                --tokens ${DESIGN_TOKENS}
                --output ${_output}
                --template-dir "${_chat_templates_dir}/.."          # ours FIRST (first match wins)
                --template-dir "${CMAKE_CURRENT_SOURCE_DIR}/scaffold/templates"
                --vars ${_vars}
            COMMAND ${CMAKE_COMMAND} -E touch ${_stamp}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Rendering chat composite: ${_comp}${_kind}"
            DEPENDS ${_template} ${DESIGN_TOKENS} ${ENGINE_SCRIPT} ${_vars}
        )
        list(APPEND _chat_stamps ${_stamp})
        list(APPEND _chat_outputs ${_output})
    endforeach()

    add_custom_target(generate_composite_${_comp}
        DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/template_gen/${_comp}.stamp
                ${CMAKE_CURRENT_BINARY_DIR}/template_gen/${_comp}_cubit.stamp
                ${CMAKE_CURRENT_BINARY_DIR}/template_gen/${_comp}_state.stamp
    )
endforeach()

add_custom_target(app_generate_chat_components
    COMMENT "Generate all chat composite widget/cubit/state triples"
    DEPENDS ${_chat_stamps} ${_chat_outputs}
)
```

Engine invocation facts (verified against `engine.py:184-288`):
- `--template` is **relative to a template dir** (e.g. `components/chat_message_bubble.dart.jinja2`) and path-traversal-guarded (`_validate_template_name` rejects `..` and absolute paths, engine.py:154-175) — so our templates MUST live under `src/app/templates/components/` to keep the `components/...` prefix parallel with scaffold's layout.
- Multi-`--template-dir` is repeatable (`action="append"`, engine.py:218-224); `FileSystemLoader` searches **in order, first match wins** (engine.py:57-104 docstring) — pass ours first so a name collision resolves to GCS's template.
- `--tokens` loads `design_tokens.json` into the `tokens` var; `--vars` JSON merges **on top of** tokens (engine.py:229-269) — composite vars can reference but not accidentally shadow `tokens` unless deliberately named so.
- `StrictUndefined` is non-negotiable (engine.py:100): any template variable missing from the merged vars fails the build with `jinja2.UndefinedError` naming the variable. This is the drift-guard — a vars/template mismatch is a build error, not silent wrong output.
- Incremental invariant: touching only the cubit template re-renders only that stamp (per-stamp DEPENDS). Stamp files live in the build tree (`template_gen/`), so a fresh configure re-renders everything once.
- Python env: scaffold sets `PYTHONPATH=<scaffold>/tools` per invocation via `${CMAKE_COMMAND} -E env` (`CMakeLists.txt:27-28`) — no pip install needed for the `scaffold_codegen` package itself; only `jinja2` must be importable by the system python3 (3.1.6 present locally, verified).

### Composite `_vars.json` shape

Modeled on scaffold's `card_vars.json` (the canonical minimal fixture — verified):
```json
{
  "_comment": "Fixture variables for chat_message_bubble composite...",
  "widget_class_name": "ChatMessageBubble",
  "file_stem": "chat_message_bubble",
  "...variant axes...": "..."
}
```
`widget_class_name` + `file_stem` are the scaffold-established keys (`file_stem` derives generated import filenames; scaffold's CMake reads it from vars.json via regex, `CMakeLists.txt:226-231` — our loop uses fixed output names instead, simpler). The **variant axes** (D-18/D-19/D-20) belong in vars as data the template iterates, e.g.:
```json
{
  "widget_class_name": "ChatMessageBubble",
  "file_stem": "chat_message_bubble",
  "roles": ["user_self", "user_peer", "assistant", "system"],
  "states": ["pending", "streaming", "thinking", "complete", "error"],
  "payload": "text"
}
```
The template then generates **one variant widget class per role** (or per meaningful role×state combination — see "variant mapping" below), never a runtime enum/switch. Append-only rule (architectural context, ai-boss D-06/D-07): new roles/states are ADDED to these arrays; existing entries are never renamed or removed — the generated file's public class names become an append-only contract. Recommend a trivial CI drift-check later (regenerate + `git diff --exit-code`), matching scaffold's documented manual drift-check convention; not Phase 1 scope.

### Variant mapping (roles × states → generated widgets, not runtime switch)

Per D-17 and the UI-SPEC contract: the role axis (4 values: `user_self`, `user_peer`, `assistant`, `system`) drives **layout/alignment/color structure** — generate one widget class per role: `ChatMessageBubbleUserSelf`, `ChatMessageBubbleUserPeer`, `ChatMessageBubbleAssistant`, `ChatMessageBubbleSystem`. The state axis (5 values) drives **within-bubble chrome** (opacity overlay, typing-dots, error tint, streaming text) — state arrives at runtime from FFI events, but each state's *visual treatment* is generated as a typed variant parameter or small generated sub-widget per state (e.g. `_ThinkingDots`, `_ErrorOverlay`), selected by the generated code's compile-time-known structure, not a hand-written switch. The flow composite (`chat_message_flow`) generates the item-type dispatch (text bubble / code block / media) from per-item variant data — a generated `sealed class ChatFlowItem` with one subclass per payload type is the idiomatic Dart 3 shape and keeps dispatch exhaustive at compile time. [ASSUMED — exact generated-class granularity is template-authoring discretion; the planner should task "author bubble template + vars, render, review output" as one plan before scaling to all four composites.]

### Composite C++ half (architectural-context D-02: always generate both halves)

The ai-boss frontend-templates phase-07 discussion locks: every atom/composite generates BOTH the Dart widget half AND the C++ Cubit-state half; the C++ store is authoritative and contract-agnostic with pluggable importers (FFI, protobuf, JSON, OpenAPI); reuse generated contract types directly as store state (no independent canonical struct + translate layer). For GCS Phase 1:

- Scaffold's `templates/cpp/` is an **empty placeholder** — its README explicitly says C++ template content is owned by the consuming repo (verified: `src/app/scaffold/templates/cpp/README.md`). So the C++ half of our composites is OURS to author: `src/app/templates/cpp/` (or `src/templates/cpp/`) holding Jinja2 templates that emit C++ state structs matching each composite's `_vars.json` axes.
- The FFI transport mechanism is **Claude's discretion** (ai-boss discussion, "FFI transport mechanism — no existing FFI layer — researcher to identify C++17-clean approach"). For Phase 1 the transport is already fixed by D-05: the message-event payload crossing `Dart_PostCObject` is a JSON string (the `gcs_on_message(port)` posts `const char* json` per the FFI Callback Mechanism section). The generated C++ Cubit-state half should therefore be expressed as **plain C++17 structs with append-only fields**, serialized to/from JSON via `nlohmann_json` (already linked in the parent build — root CMakeLists `find_package(nlohmann_json CONFIG REQUIRED)`), serving as the authoritative store-side message record. The Dart side decodes the same JSON into the generated Dart state class. One JSON schema, two generated endpoints — no translate layer (their D-05).
- **Phase 1 scoping (LOCKED by D-24, 2026-08-21):** the C++ half is generated this phase, minimal — the message record struct (`id`, `room_topic`, `role`, `state`, `text`, `timestamp` — append-only optional adds later) emitted from one `templates/cpp/chat_message_state.hpp.jinja2` driven by the same `_vars.json` as the bubble. This satisfies "always generate both halves" (their D-02) without building the full pluggable-importer framework (ai-boss phase 07's deliverable, not ours) — but the generated store is contract-agnostic with the importer seam from day one, so the FFI JSON transport (Phase 1's only importer) never leaks into the store's public surface and later importers (protobuf, OpenAPI) slot in without touching it.

## UI Strand — D-11 App Shell Structure

**Pattern sources (all verified live):**
- `src/app/scaffold/lib/components/app_screen_view.dart` — `AppScreenView({required body, footer})`: `SafeArea` + `CustomScrollView`, `body` in a `SliverToBoxAdapter`, `footer` bottom-anchored via `SliverFillRemaining`. The chat shell maps: `body` = Row(rail, message-flow pane), `footer` = `ScaffoldComposer`. Note `AppScreenView` wraps everything in a scroll view — for a chat shell with its own internal scroll (the message flow `CustomScrollView` anchored bottom per UI-SPEC), the planner should verify nesting behavior; if the nested scroll views conflict, the shell can use scaffold atoms directly (`ScaffoldSurface` rail + flow + composer in a plain `Column`) while keeping the AppScreenView *pattern* (body/footer split, per-screen Cubit). [ASSUMED — one-line layout spike in the shell task.]
- Per-screen Cubits on `flutter_bloc` `^9.0.0` (scaffold's transitive dep): `SessionCubit` at app root owns the FFI handle (init on start → shutdown on dispose; exposes `isReady`, rooms stream, active-room stream, `sendMessage(String)`, `selectRoom(roomId)` — per UI-SPEC Cubit Architecture section). `RailCubit`, `MessageFlowCubit`, `ComposerCubit` subscribe to SessionCubit projections. Cubits NEVER synthesize message state locally (D-04) — pending/streaming/error transitions map FFI payloads to variant axes.
- ai-boss consumer pattern (`../apps/genius-ai-boss/frontend`): consumer-space Dart lives in the app's own `lib/` importing `package:frontend_scaffold/...` via path dependency; scaffold `lib/` is never edited (D-14). ai-boss apps use Riverpod for DI (`apps/admin/lib/main.dart`) — **GCS should NOT copy that**: scaffold atoms are pure `Theme.of(context)` consumers and our state pattern is Cubit (D-11), so `flutter_bloc`'s `BlocProvider` is the DI mechanism, no Riverpod.
- Rail (D-21): left rail = space→room tree from `ScaffoldSurface` cards + `ScaffoldDisclosure` (expand/collapse — present at pin `ef16a0c`, verified in `lib/components/`); selected-room indicator per UI-SPEC color contract. Phone layout replaces rail with a drawer (UI-SPEC rail contract lines).
- Composition is recursive (ai-boss D-03): atoms (scaffold) → composites (our generated `chat_*`) → the shell screen (plain Dart). Wiring at each boundary: composer `onSubmit` → `SessionCubit.sendMessage`; FFI port events → `SessionCubit` → `MessageFlowCubit` → generated flow widget.

## UI Strand — D-12/D-22 Theming

**Verified mechanics** (against `src/app/scaffold/lib/theme/`):
- `scaffold_theme.dart:21-24` exports `scaffoldThemeExtensions = [ScaffoldPalette.defaultPalette, ScaffoldDimens.defaultDimens]` — but that's the DARK palette only. D-22 needs per-brightness registration:
```dart
// src/app/lib/main.dart (rewritten)
MaterialApp(
  theme: ThemeData(
    brightness: Brightness.light,
    useMaterial3: true,
    extensions: const [ScaffoldPalette.lightPalette, ScaffoldDimens.defaultDimens],
  ),
  darkTheme: ThemeData(
    brightness: Brightness.dark,
    useMaterial3: true,
    extensions: const [ScaffoldPalette.defaultPalette, ScaffoldDimens.defaultDimens],
  ),
  themeMode: ThemeMode.system,   // or a persisted toggle — UI-SPEC leaves toggle UI to scaffold's existing pattern
  ...
)
```
- `ScaffoldPalette.lightPalette` (`scaffold_palette.dart:132-155`) and `defaultPalette` (`:103-126`) are `static const` — both can go in `const` extension lists. All 16 color fields + focusRingColor + skeleton colors verified present.
- `ScaffoldDimens.defaultDimens` (`scaffold_dimens.dart:115-137`) — every UI-SPEC spacing/radius value verified verbatim (space2=4 … space12=24, radiusMd=12, radiusPill=48, borderRadiusCard=15, disabledOverlayOpacity=0.40, minTouchTarget=48).
- Widgets resolve via `context.palette` / `context.dimens` extensions with default fallbacks (`scaffold_theme.dart:6-17`) — so even before host registration, atoms render with dark defaults; registration makes light mode correct.
- **Delete** the hardcoded `ColorScheme.fromSeed(seedColor: Color(0xFF6C3CE1))` in `main.dart:27-31` (D-12). The current `main.dart` also hardcodes a Mistral model path + `geniusSlmInit` call (lines 9-16) — the whole file is rewritten per D-10/D-11; the SLM init path is NOT carried over (that's the neoswarm_ffi legacy surface, D-03 leaves the lib working but the new app shell boots the GCS session instead; if a session isn't ready, the composer renders disabled per UI-SPEC).
- `design_tokens.json` drives the codegen side (D-17 `--tokens`), the Dart theme extensions drive runtime — both from the same scaffold source of truth (D-12).

## UI Strand — FFI→Cubit Wiring (D-04/D-05)

The full chain, each link verified or patterned above:

1. **C++ → port:** network data arrives in `GcsGlobalDb`/GossipSub listener → `gcs_ffi` session → `Dart_PostCObject(registered_port, json_string)` (FFI Callback Mechanism section; spike task de-risks the `dart_native_api.h` include strategy).
2. **Port → Dart:** `ReceivePort` in `SessionCubit` (created at `init()`, `sendPort.nativePort` passed to `gcs_on_message(handle, port)`; port closed in `shutdown()` BEFORE the native session is torn down — Pitfall 6 ordering).
3. **JSON → state:** the posted JSON decodes into the generated Dart state class (mirror of the C++ record — see Composite C++ Half). `SessionCubit` routes by payload type: room-list events → `RailCubit` projection; message events → `MessageFlowCubit`; readiness → `ComposerCubit` (UI-SPEC Cubit Architecture).
4. **State → variant axes:** the FFI payload's `role`/`state`/`payload_type` fields ARE the D-18/D-19/D-20 axes. The Cubit maps payload → generated flow-item type (`ChatFlowItemTextBubble(role, state, ...)` etc.); the generated flow widget renders the matching composite. No local synthesis — `pending` enters when C++ acknowledges the send intent (echo through the FFI boundary), not when the user taps send. [ASSUMED — the precise acknowledgment protocol (optimistic-local vs echo-confirmed) is a session-API design detail within Claude's C-API discretion; UI-SPEC's "pending = user send in-flight" is satisfied either way, but echo-confirmed keeps D-04 pure.]

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
├── app/
│   ├── CMakeLists.txt        # NEW — FRONTEND_BUILD_ENABLED gate + pub/analyze/test/codegen targets (D-16/D-17)
│   ├── templates/
│   │   └── components/       # NEW — chat_message_{bubble,code_block,media,flow}{,_cubit,_state}.dart.jinja2 + *_vars.json (D-17)
│   ├── lib/
│   │   ├── main.dart         # REWRITTEN — scaffold theme registration + app shell (D-10..D-12, D-22)
│   │   ├── shell/            # NEW — chat shell screen, left rail (space→room tree), message pane (D-11/D-21)
│   │   ├── cubits/           # NEW — session_cubit, rail_cubit, message_flow_cubit, composer_cubit (D-11)
│   │   └── generated/chat/   # NEW (generated, committed, never hand-edit) — composite triples (D-17)
│   └── pubspec.yaml          # EDITED — drop flutter_chat_ui/flutter_chat_core (D-10)
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

### Pattern 4: Scaffold composite codegen (widget + cubit + state triple)
**What:** One `_vars.json` drives three Jinja2 templates producing a widget, its Cubit, and its state class; per-artifact stamp files give incremental rebuilds; `StrictUndefined` makes vars drift a build error.
**Source:** `src/app/scaffold/CMakeLists.txt:204-319` + `src/app/scaffold/templates/components/card*.jinja2` + `card_vars.json`. GCS copies this loop with its own component set (see D-17 Engine Driver).

### Pattern 5: Host-app theme extension registration
**What:** Scaffold atoms resolve `Theme.of(context).extension<ScaffoldPalette>()` / `<ScaffoldDimens>()` with dark defaults as fallback; the host registers light/dark palettes per `ThemeData` brightness.
**Source:** `src/app/scaffold/lib/theme/scaffold_theme.dart:1-24` (read in full). See D-12/D-22 Theming for the exact `main.dart` shape.

### Anti-Patterns to Avoid
- **Polling from Dart** for incoming messages — D-05 forbids it; use the port-post callback.
- **Folding the FFI symbols into `gcs_core`** — D-02 requires a separate SHARED lib; static lib + Flutter dynamic loading don't mix on iOS/Android the way the plugin flow expects.
- **`sleep_for` in tests** — banned project-wide; copy the WaitForCondition template.
- **Building deps in CI** — D-07; any CI job compiling thirdparty/SuperGenius/GeniusSDK from source is a regression.
- **Per-module C headers** — D-01; one session header.
- **OS `#ifdef` in C++ logic** — banned (exception: the `_WIN32` dllexport macro block in FFI headers is the established idiom).
- **Hand-editing generated composites** (`src/app/lib/generated/chat/*.dart`) — D-14/D-17; edit `src/app/templates/components/*.jinja2` + `_vars.json` and re-run `app_generate_chat_components`. Same rule as scaffold's committed families.
- **Editing scaffold `lib/` or scaffold `templates/`** from the GCS repo — D-14; scaffold changes go through the scaffold repo's own develop branch and arrive via pin bump (D-13).
- **Runtime role/state enum + switch in Dart** — D-17; variant structure is generated, per-item dispatch is via the generated sealed-class shape.
- **Riverpod or other DI frameworks** — scaffold atoms are `Theme.of`-only; D-11's Cubit pattern uses `flutter_bloc`'s `BlocProvider`. ai-boss's Riverpod usage is their app-specific choice, not the consumer contract.
- **Cubits synthesizing message state locally** (e.g. local `pending` flag set on tap) — D-04/UI-SPEC; all transitions are FFI-event-driven projections.
- **Wrapping code/media in bubble chrome** — D-20; they are top-level flow items (iMessage model).

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| CRDT store | Custom op log / vector-clock sync | `sgns::crdt::GlobalDB` via `GcsGlobalDb` | CRDT correctness (convergence, tombstones, DAG sync) is a research-grade problem; the component exists and is tested |
| Pub/sub transport | Raw libp2p gossipsub wiring | `GossipPubSub` (thirdparty/ipfs-pubsub) | Already integrated with SuperGenius keypair/identity |
| Wait-for-async in tests | `sleep_for` loops | `WaitForCondition` cv-polling template (copy from `test_gcs_global_db.cpp`) | Project rule; deterministic, fast-failing |
| Dart↔native string memory | `malloc` without paired free | `...StringFree` pattern (mirrors `GeniusElmStringFree`) | Cross-allocator heap mismatch crashes on Windows |
| Dependency acquisition in CI | `git clone` + build | `gh release download` of prebuilt tarballs | D-07; SuperGenius build is ~hours, download is ~minutes |
| Runner selection | Hardcoded `runs-on` | `resolve-runners` job (copy verbatim) | Self-hosted with hosted fallback, org+repo runner merge |
| Chat UI kit | Hand-rolled bubbles/composer or `flutter_chat_ui` | `frontend_scaffold` atoms + generated composites (D-10/D-17) | D-10 dropped the third-party kit; scaffold is the design system (tokens, a11y, M3 conformance already tested — 214 scaffold tests) |
| Message input | Custom TextField row | `ScaffoldComposer` (hint/badge/action rows, disabled overlay, focus outline built in) | Verified API surface covers every UI-SPEC composer contract line |
| Empty/error/loading states | Ad-hoc `if (empty) Text(...)` | `ScaffoldStateView` / `ScaffoldStateViewCubit` family | InstanceId-driven state machine already handles loading/empty/error/unavailable/success |
| Theme tokens | Hardcoded colors/dimens in widgets | `context.palette` / `context.dimens` + host-registered extensions (D-12) | Single source of truth; light/dark both driven by scaffold palettes (D-22) |
| Variant widget multiplication | Runtime switch over role×state | Generated variant classes from `_vars.json` axes (D-17) | StrictUndefined + codegen make drift a build error; 4×5 axis growth stays linear in template, not widget code |
| CMake codegen driver | A new Python/shell driver | `engine.py` CLI via `add_custom_command` stamp pattern (scaffold §6) | Proven incremental semantics; GCS copies the loop verbatim with different names |

**Key insight:** every deceptively complex subsystem in this phase (CRDT, gossip, FFI codegen, CI runner orchestration, theme token propagation, variant codegen) already has an in-repo or sibling-repo implementation. The phase's risk is **wiring mistakes** (wrong `-D` vars, wrong include paths, wrong link order, wrong cache-var timing before `add_subdirectory`), not algorithmic risk.

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
**How to avoid:** Session handle owns the registered port; `gcs_shutdown` clears it under the session mutex before tearing down `GcsGlobalDb`; the io thread (owned by `GcsGlobalDb`) must be joined (it is — `Shutdown()` joins) before the handle is freed. SessionCubit closes its ReceivePort before calling `gcs_shutdown`.
**Warning signs:** Crash in `Dart_PostCObject` on shutdown path. [ASSUMED — standard Dart API_DL behavior; will be validated by the FFI smoke test's shutdown ordering]

### Pitfall U1: Scaffold cache vars set AFTER `add_subdirectory(scaffold)`
**What goes wrong:** `TEMPLATES_DIR` / `GENERATED_DIR` changes silently ignored — scaffold renders to its own defaults (`${CMAKE_BINARY_DIR}/generated`, no consumer templates).
**Why:** Scaffold declares them `CACHE STRING ... <default>` at `CMakeLists.txt:84-93`; a `set(... CACHE STRING)` after the subdirectory doesn't override the already-cached value, and a normal `set()` after is out of scope.
**How to avoid:** All three `set(... CACHE STRING)` calls immediately BEFORE `add_subdirectory(scaffold)` in `src/app/CMakeLists.txt` (shape shown in D-16 Build Wiring). Verify once by checking the generated output lands in `src/app/lib/generated/`.
**Warning signs:** Generated files appear under `build/OSX/Debug/gcs_src/app/generated/` instead of the source tree; our `components/chat_*` templates "not found" errors from engine.py.

### Pitfall U2: Bare `--template-dir` flag with empty value breaks argparse
**What goes wrong:** engine.py argument parsing fails when an optional dir flag is passed with no value.
**Why:** Documented in scaffold's own CMakeLists (§5 comments, fix `1762a45`): a space-embedded string reaches argparse as ONE token.
**How to avoid:** List-form flags only: `set(_flag --template-dir "${DIR}")` when set, `set(_flag "")` otherwise; never `"--template-dir ${DIR}"` as a single string.
**Warning signs:** `engine.py: error: argument --template-dir: expected one argument`.

### Pitfall U3: StrictUndefined turns vars drift into build failures (by design)
**What goes wrong:** Adding a `{{ new_var }}` to a template without adding it to `_vars.json` fails the whole codegen target with `jinja2.UndefinedError`.
**Why:** StrictUndefined is intentional (scaffold PROJECT.md: "missing variables must fail loudly").
**How to avoid:** Template and `_vars.json` are edited together in the same commit; the composite task's verification step is the codegen target going green. Treat failures as the guard working, not as engine bugs.
**Warning signs:** `ERROR: undefined variable: 'foo'` from a codegen stamp command.

### Pitfall U4: Scaffold `frontend_generate_api` expects an api-specs/ dir GCS doesn't have
**What goes wrong:** If any GCS target depends on scaffold's `frontend_generate_api` / `frontend_all`, the build fails — `<scaffold-parent>/api-specs` resolves to `src/app/api-specs/` which doesn't exist.
**Why:** `API_SPECS_DIR` default is parent-relative (scaffold `CMakeLists.txt:32-41, 92-93`); ai-boss has `api-specs/` at its repo root, GCS does not.
**How to avoid:** GCS targets depend ONLY on our `app_generate_chat_components` (and transitively nothing from scaffold's API targets). Do not wire `DEPENDS frontend_all`. If the WARNING about missing api-specs is noisy, set `-DAPI_SPECS_DIR=` to an inert path.
**Warning signs:** "api-specs/ not found" warnings; `frontend_generate_api` errors if invoked.

### Pitfall U5: Generated composites not visible to `dart analyze` on a fresh checkout
**What goes wrong:** `dart analyze --fatal-infos` fails with unresolved imports of `generated/chat/*.dart` before any CMake codegen has run.
**Why:** `GENERATED_DIR` points into the source tree but the files only exist after `app_generate_chat_components` runs.
**How to avoid:** Commit the generated output (scaffold's committed-families convention, D-14) AND make `app_analyze` / `app_test` DEPEND on `app_generate_chat_components` (shown in the D-16 snippet). Fresh-checkout analysis works because generated files are in git; build-tree runs regenerate incrementally.
**Warning signs:** `Target of URI doesn't exist: 'package:flutter_app/generated/chat/...'`.

### Pitfall U6: `flutter_chat_ui` remnant imports after D-10 removal
**What goes wrong:** `dart pub get` succeeds but analyze fails on `package:flutter_chat_ui` imports — or pub get fails because pubspec dropped the deps while `main.dart` still imports them.
**Why:** The current `main.dart` imports both chat packages plus `neoswarm_ffi` (verified: `main.dart:5-7`); the rewrite must land in the same change as the pubspec edit.
**How to avoid:** One task owns pubspec + main.dart + widget_test.dart together (the existing `test/widget_test.dart` references a `MyApp` counter test that no longer exists — it must be replaced, not patched).
**Warning signs:** Pubspec/imports mismatch errors at `app_pub_get` or `app_analyze`.

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

### Scaffold theme registration (D-12/D-22)
```dart
// Source: src/app/scaffold/lib/theme/scaffold_theme.dart:19-24 (extension list pattern)
// + scaffold_palette.dart:103-155 (defaultPalette/lightPalette), applied to host ThemeData.
MaterialApp(
  theme: ThemeData(
    brightness: Brightness.light,
    useMaterial3: true,
    extensions: const <ThemeExtension<dynamic>>[
      ScaffoldPalette.lightPalette,
      ScaffoldDimens.defaultDimens,
    ],
  ),
  darkTheme: ThemeData(
    brightness: Brightness.dark,
    useMaterial3: true,
    extensions: const <ThemeExtension<dynamic>>[
      ScaffoldPalette.defaultPalette,
      ScaffoldDimens.defaultDimens,
    ],
  ),
  themeMode: ThemeMode.system,
  home: /* ChatShellScreen wired to SessionCubit */,
)
```

### Composer wiring (D-10/D-11; UI-SPEC composer contract)
```dart
// Source: src/app/scaffold/lib/components/scaffold_composer.dart:31-68 (API surface)
ScaffoldComposer(
  hintText: roomName == null
      ? 'Select a room to start messaging'
      : 'Message #$roomName',
  disabled: !sessionReady,          // bound to SessionCubit.isReady
  actionRow: <Widget>[/* single send button: radiusPill, lightGreenPrimary fill, Icons.send */],
  onSubmit: (text) => context.read<SessionCubit>().sendMessage(text),
)
```

### Per-composite codegen custom command (D-17; the loop body)
```cmake
# Source: src/app/scaffold/CMakeLists.txt:234-248 (widget stamp block), adapted to GCS paths.
add_custom_command(
    OUTPUT ${_stamp} ${_output}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/template_gen"
    COMMAND ${PYTHON3_EXECUTABLE} ${ENGINE_SCRIPT}
        --template "components/chat_message_bubble.dart.jinja2"
        --tokens ${DESIGN_TOKENS}
        --output ${_output}
        --template-dir "${CMAKE_CURRENT_SOURCE_DIR}/templates"
        --template-dir "${CMAKE_CURRENT_SOURCE_DIR}/scaffold/templates"
        --vars "${CMAKE_CURRENT_SOURCE_DIR}/templates/components/chat_message_bubble_vars.json"
    COMMAND ${CMAKE_COMMAND} -E touch ${_stamp}
    DEPENDS ${_template} ${DESIGN_TOKENS} ${ENGINE_SCRIPT} ${_vars}
)
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Build deps from source in CI | Prebuilt release tarballs (`gh release download`, tag `Target-ABI-branch-buildtype`) | Established across SuperGenius/GeniusSDK CI (current) | CI time drops from hours to minutes; GCS copies it |
| Flutter plugin template (`flutter create --template=plugin`) for FFI | Plain SHARED lib target loaded via `DynamicLibrary.open` + ffigen | neoswarm_ffi still uses plugin template; gcs_ffi per D-02 is a plain shared lib | Simpler CMake (no Flutter toolchain coupling in C++ build) |
| Dart polling loop for native events | NativePort push (`Dart_PostCObject`) | D-05 (this phase) | No in-repo precedent — see Open Questions |
| `flutter_chat_ui`/`flutter_chat_core` third-party chat kit | Pure-scaffold UI + generated composites | D-10 (2026-08-19) | Sole UI kit = design system; tokens/a11y tested upstream (214 scaffold tests) |
| Hand-written variant widgets / runtime enum-switch | Jinja2-generated variant triples from `_vars.json` axes | D-17 (2026-08-20) | Variant growth is template-linear; StrictUndefined makes drift a build error |
| Host-app hardcoded `ColorScheme.fromSeed` | Scaffold `ThemeExtension` registration (palette + dimens per brightness) | D-12/D-22 (2026-08-20) | Light/dark both token-driven; scaffold pin bumps propagate design changes |

**Deprecated/outdated:**
- `neoswarm_ffi/ffigen.yaml` entry point (`genius_slm_chat_c.h`) — stale; file doesn't exist. Do not copy that path.
- `src/app/lib/main.dart` (legacy single-screen SLM chat, hardcoded Mistral path, stock `Chat` widget, purple seed) — fully rewritten this phase (D-10/D-11/D-12).
- `src/app/test/widget_test.dart` (stock counter test referencing `MyApp`) — replaced with shell smoke tests.
- `FRONTEND_TARGET=html` scaffold CMake targets — known broken upstream under StrictUndefined (documented in scaffold CLAUDE.md "Known broken"); GCS uses `flutter` target only.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | Dart NativePort mechanism details (`Dart_PostCObject`, `sendPort.nativePort`, ffigen `int64_t` mapping) follow the standard documented Dart API_DL pattern | FFI Callback Mechanism | Medium — no in-repo precedent exists to copy; a small spike task in the plan de-risks it. If the include/link strategy for `dart_native_api.h` differs, only `src/ffi/` files are affected. |
| A2 | `NativeCallable.listener` is a viable alternative but ReceivePort is preferred per D-05 | Alternatives Considered | Low — D-05 already locked the pattern. |
| A3 | GCS CI can rely on GeniusSDK develop releases (2026-08-04) containing `GeniusSDKGetNode` (added Aug 3) | Build System Wiring | Low-Medium — dates line up, but the first CI run is the real verification; failure mode is a clear link error with a documented fix (fresher GeniusSDK release). |
| A4 | Plain `ctest` suffices for GCS smoke tests on Linux (no gnome-keyring wrapper needed) because tests use the injected-pubsub seam and never init GeniusSDK | CI/CD item 5 | Low — copying the dbus-run-session wrapper verbatim is zero-cost insurance if wrong. |
| A5 | Letting cmaketemplate's zkLLVM auto-download run in CI (headers only) satisfies D-09's "skip zkLLVM" intent | CI/CD item 3 | Low — worst case CI downloads a tarball it doesn't compile against; confirm with user if "skip" meant "zero zkLLVM artifacts". |
| A6 | `AppScreenView`'s `CustomScrollView` wrapper composes cleanly with the message flow's own bottom-anchored `CustomScrollView`; if not, the shell keeps the AppScreenView *pattern* (body/footer + per-screen Cubit) using a plain `Column` + `ScaffoldSurface` | D-11 App Shell | Low — one-line layout spike in the shell task; either way the Cubit structure and composer placement are unchanged. |
| A7 | Generated composite granularity = one widget class per role, state as generated typed variant parameter, flow dispatch via generated sealed class | Variant mapping | Low-Medium — template-authoring discretion; the "author bubble template first, review output, then scale" task sequence de-risks before 4 composites exist. |
| A8 | The message event JSON posted across FFI can be decoded by generated Dart state classes that mirror a generated C++ struct (one schema, two endpoints, nlohmann_json on the C++ side) | Composite C++ Half | Medium — the C++ half's template shape is Phase 1-minimal and contract-agnostic per D-24 (FFI/JSON is the only importer; the full pluggable-importer model belongs to ai-boss phase 07). Both-halves generation is LOCKED by D-24 — not deferrable. |
| A9 | Pending-state entry is echo-confirmed through the FFI boundary (C++ acknowledges), not optimistic-local in Dart | FFI→Cubit Wiring | Low — session-API design detail within C-API discretion; UI-SPEC satisfied either way. |
| A10 | Committing `src/app/lib/generated/` output (vs build-tree-only) is the right call for analyzer/test self-containment | Alternatives / Pitfall U5 | Low — matches scaffold's committed-families convention; reversible by changing GENERATED_DIR + gitignore. |

## Open Questions

1. **D-05 references a neoswarm_ffi callback mechanism that doesn't exist.** — **RESOLVED (2026-08-15):** Plan 01-05 spike implements the standard Dart API_DL NativePort pattern (`Dart_InitializeApiDL` + `Dart_PostCObject` + `ReceivePort`). No alternate in-repo reference surfaced; the standard mechanism is the design. The UI strand's FFI→Cubit wiring consumes this same mechanism (SessionCubit owns the ReceivePort).

2. **CRDT round-trip needs GcsGlobalDb accessors (Put/Get/AddBroadcastTopic) that don't exist yet.** — **RESOLVED (2026-08-15), LOCATION SUPERSEDED BY D-25 (2026-08-21):** Plan 01-01 adds the four pass-through accessors — but the class MOVES out of the GNUS-NEO-SWARM submodule into the root repo `src/lib/` first (D-25). 01-01 becomes "move + extend": `git mv` `gcs_global_db.{hpp,cpp}` + `test_gcs_global_db.cpp` into `src/lib/` + the root test tree (history preserved), new `gcs_storage` CMake target linking `sgns::crdt_globaldb`/pubsub directly, then add the accessors. No neoswarm feature branch; `gcs_core` (01-02) links the moved target, not `neoswarm_storage`. The neoswarm `03-gcs-globaldb-integration` phase (03-02/03-03 pending) gets a pointer-update note.

3. **zkLLVM in CI: header-only consumption vs. full skip.** — **RESOLVED (2026-08-15):** Plan 01-06 keeps cmaketemplate's auto-download (headers-only) and adds no explicit zkLLVM CI step.

4. **Does the FFI session API carry room-list/readiness events in Phase 1, or only message events?**
   - What we know: UI-SPEC's SessionCubit exposes room list stream, active-room stream, and `isReady`; D-21 requires the rail to switch spaces/rooms in Phase 1. The C++ smoke suite (01-01..01-06 lineage) only proves topic join + publish + message callback.
   - What's unclear: Whether Phase 1's session API needs `gcs_list_rooms`-style calls + room-list push events to feed `RailCubit`, or whether the rail renders from a hardcoded smoke-topic set (`gcs/chat/smoke-test`) as a shell demonstrator until Phase 2/3 land real spaces/rooms.
   - Recommendation: Minimal — the rail in Phase 1 can be driven by a small FFI call surface (`gcs_joined_topics(handle)` returning the topics joined this session) with the smoke topic pre-joined at init. Do NOT design the spaces/rooms data model in this phase (that's Phase 2 + the deferred CRDT schema). Planner should keep the rail's data source behind the SessionCubit projection so Phase 2 swaps the source without touching the rail widget.

5. **Where do the generated C++ Cubit-state halves live and how minimal are they in Phase 1?** — **RESOLVED (2026-08-21) by D-24:** Generate it — one `chat_message_state.hpp.jinja2` driven by the bubble's `_vars.json`, in `src/app/templates/cpp/` (scaffold's `templates/cpp/` is an empty consumer-owned placeholder). Establishes the both-halves invariant cheaply and keeps the JSON schema single-sourced. Merged ai-boss 07 constraints: the store is contract-agnostic with a pluggable importer seam (their D-04 — FFI/JSON is Phase 1's only importer, never leaked into the store surface), reuses contract types directly (their D-05), append-only from day one (their D-06/D-07).

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| cmake | all builds | ✓ | 3.29.2 | — |
| ninja | all builds | ✓ | 1.13.2 | — |
| clang (Apple) | OSX build | ✓ | 17.0.0 | — |
| gh CLI | CI release downloads + local release inspection | ✓ | 2.93.0 | curl fallback in cmaketemplate auto-download |
| git | submodule ops | ✓ | 2.50.1 | — |
| python3 | scaffold engine.py codegen (D-17) | ✓ | 3.x at `/opt/homebrew/bin/python3` | — |
| jinja2 | engine.py hard requirement | ✓ | 3.1.6 [VERIFIED 2026-08-21] | `pip install jinja2` (documented in engine.py error path) |
| dart | app pub get / analyze / ffigen (D-16 targets) | ✓ | 3.11.5 via `thirdparty/flutter/bin/dart` [VERIFIED 2026-08-21] | gate skips app targets cleanly when absent (D-16) |
| flutter | app test / dev-loop run | ✓ | 3.41.9 via `thirdparty/flutter/bin/flutter` [VERIFIED 2026-08-21] | gate skips (D-08 keeps Flutter out of CI anyway) |
| thirdparty build tree | local builds | ✓ | `../thirdparty` present with build outputs | CI downloads release tarball |
| SuperGenius build tree | local builds | ✓ | `../SuperGenius/build/OSX/Debug` (resolved in existing cache) | CI downloads release |
| GeniusSDK build tree | local builds | ✓ | `../GeniusSDK/build/OSX/Debug` (resolved in existing cache) | CI downloads release |
| scaffold submodule pin | all UI work (D-13) | ✓ | `ef16a0c` checked out [VERIFIED `git rev-parse HEAD` 2026-08-21] | — |
| Android NDK | Android builds (CI) | ✗ (not checked locally; CI installs r27b per workflow) | r27b in CI | CI step downloads it |
| Existing local build proof | — | ✓ | `build/OSX/Debug/gcs_src/libgcs_core.a` exists | — |

**Missing dependencies with no fallback:** none for the planned work.
**Missing dependencies with fallback:** Android NDK locally (CI installs it; local Android builds need `ANDROID_NDK_HOME` set).

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework (C++) | Google Test (thirdparty-provided; `find_package(GTest CONFIG REQUIRED)` in root CMakeLists under `BUILD_TESTING`) |
| Framework (Dart) | `flutter_test` (SDK-provided, already in `src/app/pubspec.yaml` dev_dependencies) + `flutter analyze --fatal-infos` as the static gate |
| Config file (C++) | none for GCS yet — GCS-root `test/CMakeLists.txt` is a Wave 0 deliverable; `build/CommonBuildParameters.cmake:146-151` (hook at `cmake/CommonBuildParameters.cmake:505-508`) already has the `add_subdirectory(${PROJECT_ROOT}/test ...)` gated on `BUILD_TESTS`/`BUILD_TESTING` |
| Config file (Dart) | `src/app/analysis_options.yaml` (exists — `flutter_lints`); no test config file needed |
| Quick run command (C++) | `ctest --test-dir build/OSX/Debug -R gcs --output-on-failure` |
| Quick run command (Dart) | `cd src/app && flutter test` (or `cmake --build build/OSX/Debug --target app_test` when `FRONTEND_BUILD_ENABLED=ON`) |
| Full suite command | `ctest --test-dir build/OSX/Debug --output-on-failure` (includes neoswarm tests) + `cd src/app && dart analyze --fatal-infos && flutter test` |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| CORE-05 | GlobalDB CRDT initializes; broadcast+listen topic wired; Put→Get round-trips locally over real GossipPubSub (port 0) | unit (integration-style fixture) | `ctest --test-dir build/OSX/Debug -R test_gcs_core_smoke --output-on-failure` | ❌ Wave 0 |
| (success criterion 3) | FFI shared lib loads; `gcs_init` returns handle; echo/round-trip call works; shutdown clean | unit (links gcs_ffi) | `ctest --test-dir build/OSX/Debug -R test_gcs_ffi --output-on-failure` | ❌ Wave 0 |
| (success criterion 4) | `gcs_join_topic` / GlobalDB `AddBroadcastTopic`+`AddListenTopic` succeed on a `gcs/chat/*` topic | covered by CORE-05 smoke test | same as CORE-05 | ❌ Wave 0 |
| (success criteria 1+5) | All 5 platforms build + ctest green | CI | workflow matrix (OSX/Linux/Windows run ctest; Android/iOS build-only) | ❌ Wave 0 (`.github/workflows/cmake.yml`) |
| (UI: D-12/D-22) | Host registers light+dark scaffold palettes; no `ColorScheme.fromSeed` remains; atoms resolve `context.palette`/`context.dimens` | widget test + analyze | `cd src/app && flutter test test/theme_registration_test.dart` + `dart analyze --fatal-infos` | ❌ Wave 0 |
| (UI: D-11/D-21) | Shell renders rail + pane + composer; rail switch changes active room projection; composer disabled until session ready | widget test (fake SessionCubit — no FFI) | `cd src/app && flutter test test/chat_shell_test.dart` | ❌ Wave 0 |
| (UI: D-17/D-18/D-19/D-20) | Generated bubble composite renders each role × key states from fixture data; flow interleaves text/code/media items from per-item variant data | widget test (golden-optional; render + find) | `cd src/app && flutter test test/generated/chat_composites_test.dart` | ❌ Wave 0 (test + the generated files it covers) |
| (UI: D-17 build) | `app_generate_chat_components` re-renders on template/vars touch; StrictUndefined fails loudly on vars drift | build-level (cmake target) | `cmake --build build/OSX/Debug --target app_generate_chat_components` (with `-DFRONTEND_BUILD_ENABLED=ON` at configure) | ❌ Wave 0 (`src/app/CMakeLists.txt`) |

### Sampling Rate
- **Per task commit (C++):** `cmake --build build/OSX/Debug -j && ctest --test-dir build/OSX/Debug -R gcs --output-on-failure`
- **Per task commit (Dart):** `cd src/app && dart analyze --fatal-infos && flutter test`
- **Per wave merge:** `ctest --test-dir build/OSX/Debug --output-on-failure` (full parent-build suite) + full Dart gate
- **Phase gate:** Full C++ suite green locally on OSX + CI green on all 5 platforms + Dart analyze/test green locally before `/gsd:verify-work`

### Wave 0 Gaps
- [ ] `test/CMakeLists.txt` (GCS root) — registers smoke tests; gate on GTest found
- [ ] `test/test_wait_condition.hpp` — WaitForCondition template (copy from neoswarm)
- [ ] `test/test_gcs_core_smoke.cpp` — covers CORE-05 substrate (lifecycle + topic + Put/Get round-trip)
- [ ] `test/test_gcs_ffi.cpp` — covers FFI init/echo/shutdown
- [ ] `src/ffi/CMakeLists.txt` + `gcs_ffi` SHARED target
- [ ] `.github/workflows/cmake.yml` — CI (Wave 0 or first wave; without it criterion 5 is unverifiable)
- [ ] `src/app/CMakeLists.txt` — FRONTEND_BUILD_ENABLED gate + app targets + composite codegen loop
- [ ] `src/app/templates/components/chat_message_{bubble,code_block,media,flow}{,_cubit,_state}.dart.jinja2` + `*_vars.json` — D-17 authoring (bubble first per A7)
- [ ] `src/app/lib/generated/chat/` — produced by the codegen target, committed (D-14 convention)
- [ ] `src/app/test/theme_registration_test.dart`, `chat_shell_test.dart`, `generated/chat_composites_test.dart` — replace the stock `widget_test.dart` (which references a nonexistent `MyApp` and would fail analyze today)
- [ ] Dart fake/mock harness for SessionCubit in widget tests (Cubit test doubles; no FFI in widget tests — `bloc_test` is NOT in pubspec and not required; hand-rolled fake Cubits suffice for Phase 1)

## Security Domain

### Applicable ASVS Categories

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | no | P2P identity via libp2p keypair (`KeyPairFileStorage`) — no user auth this phase |
| V3 Session Management | partial | FFI session handle: opaque pointer, no serialization, shutdown unregisters callbacks (Pitfall 6) |
| V4 Access Control | no | Membership/roles land in Phase 4 |
| V5 Input Validation | yes | FFI boundary: null-check all `const char*` from Dart, treat as untrusted UTF-8; length-bound copies; mirror the null-tolerant style of `genius_elm_chat_completions.cpp`. Dart side: JSON from the FFI port is parsed defensively (unknown fields ignored — pairs with the append-only contract rule). Jinja2 codegen: engine.py already guards template-name path traversal (T-01-01 mitigation, engine.py:154-175). |
| V6 Cryptography | yes (inherited) | Never hand-roll: libp2p keypair via SuperGenius `KeyPairFileStorage`; app-layer encryption deferred to v1.1 |

### Known Threat Patterns for C++/FFI/libp2p + Flutter stack

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Use-after-free across FFI (Dart disposes while C++ posts) | Tampering/DoS | Session mutex + unregister-on-shutdown ordering; SessionCubit closes ReceivePort before `gcs_shutdown` (Pitfall 6) |
| Heap mismatch (Dart-side free of C++ malloc) | DoS | Paired `gcs_string_free` export; never let Dart free native memory directly |
| Null/malformed strings from Dart | Tampering | Null-check + validate at FFI boundary before touching C++ state |
| Malformed JSON from native port | Tampering | Dart decode in try/catch with schema-tolerant parsing; StrictUndefined-style loud failure at codegen time, tolerant skip at runtime |
| Double-shutdown of GcsGlobalDb | DoS | Already handled: `Shutdown()` is idempotent (CITED: gcs_global_db.hpp) |
| Double `gcs_init` | DoS | Mirror ELM pattern: thread-safe, subsequent calls no-op or return existing handle |
| Template path traversal via build config | Tampering | engine.py `_validate_template_name` rejects `..`/absolute paths (in place upstream) |
| Supply chain via pub deps | Tampering | Phase 1 adds zero new pub packages; removes two; scaffold arrives via pinned git submodule (D-13), not a registry |

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
- `src/app/scaffold/` submodule at pin `ef16a0c` (verified `git rev-parse HEAD` 2026-08-21):
  - `CMakeLists.txt` (read in full, 417 lines) — cache-var contract, Source A/B template rendering, §6 per-component stamp loop, `1762a45` list-form flag idiom
  - `tools/scaffold_codegen/engine.py` (read in full) — multi-template-dir FileSystemLoader, StrictUndefined, tokens/vars merge, path-traversal guard, CLI
  - `templates/components/card_vars.json` + full `templates/` listing — vars fixture shape, cubit/state triple naming, `templates/cpp/README.md` (consumer-owned placeholder)
  - `lib/theme/scaffold_theme.dart`, `scaffold_palette.dart`, `scaffold_dimens.dart` — extension registration pattern, both palettes, all dimens values
  - `lib/components/app_screen_view.dart`, `scaffold_composer.dart`, `scaffold_state_view.dart` — API surfaces (read/grepped)
  - `pubspec.yaml` — `flutter_bloc: ^9.0.0` transitive dep
  - `CLAUDE.md` — generated-families rule, pre-finish gates, FRONTEND_TARGET=html known-broken note
- `../apps/genius-ai-boss/cmake/CommonBuildParameters.cmake:136-157` + `frontend/CMakeLists.txt` (read in full) — FRONTEND_BUILD_ENABLED gate, pub/analyze/test custom targets, TEMPLATES_DIR/GENERATED_DIR cache-var pattern, scaffold add_subdirectory bridge
- `../apps/genius-ai-boss/.planning/workstreams/frontend-templates/phases/07-c-interface-templates/07-DISCUSSION-LOG.md` — C++/Dart ownership model (per-instance ownership, always-both-halves, recursive composition, append-only contracts, FFI transport = Claude's discretion)
- `src/app/lib/main.dart`, `src/app/pubspec.yaml`, `src/app/test/widget_test.dart` (read) — current app state: chat-kit imports, hardcoded purple seed, stock counter test
- Tool availability probes: `python3` + jinja2 3.1.6, dart 3.11.5, Flutter 3.41.9 (verified 2026-08-21)

### Secondary (MEDIUM confidence)
- None — all load-bearing claims verified against files in this workspace.

### Tertiary (LOW confidence)
- Dart API_DL include/link strategy for a plain shared lib (A1) — standard pattern from Dart documentation knowledge; spike task planned.
- Nested-CustomScrollView behavior of AppScreenView + message flow (A6) — layout spike in shell task.

## Metadata

**Confidence breakdown:**
- Standard stack: HIGH — no new packages; all components read in-tree
- Architecture: HIGH — build/link/test wiring verified against live files and an existing successful local build
- Pitfalls: HIGH — each pitfall derived from an in-repo comment, CMake guard, or verified file state (including the ffigen.yaml staleness and the missing D-05 precedent)
- FFI callback mechanism: MEDIUM — the pattern is standard, but no in-repo precedent exists (see Open Question 1)
- UI strand (build wiring, codegen driver, theming, shell): HIGH — every mechanism verified against the live scaffold pin `ef16a0c` and the ai-boss consumer repo; the only soft spots are template-authoring granularity (A7) and C++-half minimality (A8), both sequenced as spike-first tasks

**Research date:** 2026-08-15; **UI strand:** 2026-08-21
**Valid until:** 2026-09-20 (30 days; CI release tags roll forward continuously — re-check `gh release list` if planning happens after GeniusSDK publishes newer develop releases; scaffold pin moves on its own develop branch — re-verify against the checked-out pin if bumped again)
