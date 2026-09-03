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
- **D-05:** Data arriving from the network (incoming messages, CRDT sync updates) is pushed to Dart via **native→Dart callbacks** — ffigen `Pointer<NativeFunction>` + `ReceivePort`/NativePort pattern. Dart does NOT poll. This is the same pattern `GNUS-NEO-SWARM/neoswarm_ffi` already uses — reuse that mechanism (listener registration on the session handle; amended by D-27/D-29 to `gcs_subscribe(handle, topic, dartPort)`).

### CI/CD
- **D-06:** Follow the SuperGenius CI pattern (`.github/workflows/cmake.yml` in `../SuperGenius`): a `resolve-runners` job picks self-hosted runners by label (`sg-ubuntu-linux`, `sg-arm-linux`, `SG-WIN11`, `gv-OSX-Large`) with GitHub-hosted fallbacks, then a matrix job builds **Android (arm64-v8a, armeabi-v7a), iOS, OSX, Linux (x86_64, aarch64), Windows × Debug/Release**.
- **D-07:** Dependencies are **downloaded as prebuilt release tarballs** (thirdparty from `GeniusVentures/thirdparty` releases; additionally SuperGenius and GeniusSDK releases, tagged per `Target-ABI-branch-buildtype`) — NOT built per-run. GeniusSDK's workflow is the template for a consumer repo that downloads its deps this way.
- **D-08:** CI builds and ctests the **C++ core only** in Phase 1 (no Flutter app build in CI yet — desktop-first; Flutter smoke test of FFI is a local/dev-loop concern this phase). The Flutter gate in D-16 must default OFF / skip cleanly when dart+flutter are absent so this CI path is unaffected.
- **D-09:** Self-hosted runner cleanup + container usage (`ghcr.io/geniusventures/debian-bullseye` for Linux/Android) copied from SuperGenius workflow; skip zkLLVM (GCS doesn't need it).

### Storage Component Location
- **D-25:** **`GcsGlobalDb` moves into the root repo.** Relocate `GNUS-NEO-SWARM/src/storage/gcs_global_db.{hpp,cpp}` + `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp` into the app structure under `src/lib/` (and the root test tree), as its own CMake target (e.g. `gcs_storage`) linking `sgns::crdt_globaldb` / pubsub directly. `gcs_core` (01-02) links the moved target in `src/lib/` — NOT `neoswarm_storage`. Rationale: it is GCS's storage component, misplaced in the neoswarm submodule by neoswarm workstream Phase 03-01 (`bc6ab3c`) before `feature/app-restructure` established `src/lib/` as the app C++ home. The neoswarm `03-gcs-globaldb-integration` phase (03-02/03-03 pending) gets a pointer-update note; 01-01 becomes "move + extend" (git mv preserves history, then add the data-plane accessors).

### FFI Event Model & Wire Format (amended 2026-08-22)
- **D-26:** **Publish/subscribe FFI; protobuf wire format.** Two reversals over the earlier FFI shape:
  - **Push, not pull.** All data flows native→Dart over the D-05 callback stream — Dart subscribes a listener on the session handle and C++ **pushes** events. This covers incoming messages AND **room-list / membership events** (the `gcs/chat/<roomname>` topics joined this session) AND readiness. There is **no pull call** — `gcs_joined_topics(handle)` is rejected; the rail renders from pushed room events. This generalizes D-05 (messages-only) to all event types and supersedes RESEARCH Q4's "minimal pull" recommendation.
  - **Protobuf is the FFI wire format** — reverses D-24's "FFI/JSON is Phase 1's only importer." The message/event contract is a `.proto` (e.g. `gcs_chat.proto`) compiled via the existing `add_proto_library` helper (`build/cmake/functions.cmake:104`) + `find_package(Protobuf CONFIG REQUIRED)` + `protoc` from thirdparty (already wired in `cmake/CommonBuildParameters.cmake:36-61`; three `.proto` files already exist in `GNUS-NEO-SWARM/src/proto/` as the pattern). Aligns with ADR-005 (protobuf = source-of-truth for FFI commands/events). The store stays **contract-agnostic** (D-24's merged D-04 seam holds — protobuf is now Phase 1's importer instead of JSON, and the wire format never leaks into the store's public surface). The generated C++ half (D-24) is the **protobuf-generated message type reused directly** (ai-boss D-05), not a hand-authored `chat_message_state.hpp.jinja2` + nlohmann_json struct. Append-only discipline (ai-boss D-06/D-07) applies to the `.proto` from day one. No temporary JSON-then-protobuf hack. *(Wire format RESTORED 2026-08-26 — after a same-day JSON-only flip (D-29's initial draft), the final call is protobuf; of this bullet only the dangling ADR-005 citation is dropped — no such ADR exists in `.planning`.)*

### FFI Data Plane & Generated Contract (locked 2026-08-26, synced with GeniusTube)
- **D-27:** **Symmetric in-process topic pub/sub across the FFI boundary.** Imported from GeniusTube's frozen D-21 (`../apps/genius-tube/.planning/phases/00-architecture-schema-freeze/00-CONTEXT.md`), adapted to GCS:
  - **NOT libp2p, NOT the GossipSub wire, NOT an RPC shell.** The network transport (CoreSession → GossipPubSub, topics `ipfs-pubsub://gcs/chat/<roomname>`) stays entirely inside C++; raw pub/sub topics never cross the FFI boundary.
  - **Dart→C++ is publish, not call.** Dart publishes chat-shaped protobuf `Command` envelopes to command topics (join/send/leave/config); C++ subscribes and validates. This replaces 01-03's call-style "gcs_publish wraps raw text" shape — the envelope is built Dart-side (thin, data-only). The ffigen-bound C ABI is exactly: init / subscribe / publish (+ error surface), nothing per-feature.
  - **C++→Dart stays push** (D-05/D-26 unchanged): C++ publishes event envelopes; Dart Cubits are thin topic subscribers that render typed data. "C++ Cubits" (state-holder services) own application state per D-04.
  - **ELM orchestration is invisible to Dart.** Per `../SuperGenius/.planning/notes/ELM-bridging-gaps.md` (v1.1): C++ alone creates/funds ONE SuperGenius job containing N ELM work items, listens for results, and folds them into chat events. Dart never sees job/funding/token accounting; ELM streaming (`ElmTokenDelta`/`ElmProgress`) surfaces later as new event-envelope variants (oneof — non-breaking, append-only per D-24).
  - **Errors cross FFI as raw strings** (GeniusTube D-14): no typed Dart exception mapping layer.
  - *(Payload encoding amended 2026-08-26 — see D-29: envelopes are protobuf messages carried as codec-tagged bytes; the C ABI takes `uint8_t* + length`, never `char*`.)*
- **D-28:** **`gcs_chat.proto` is generated from a Jinja2 template source — never hand-authored; no OpenAPI doc.** GeniusTube D-25/D-03a adapted to GCS's Jinja2-only reality (GCS has no api-specs — Pitfall U4 holds; user confirmed no OpenAPI doc 2026-08-26):
  - The authored source is a template family in `src/app/templates/` (alongside the D-17 composites), rendered by the existing scaffold stamp chain (`engine.py` via the `scaffold_generate_templates`/component targets, D-15) into a build-dir `gcs_chat.proto`. Supersedes D-26's hand-authored `src/proto/gcs_chat.proto`; `add_proto_library` may still compile the generated file — the *authoring* moves.
  - protoc dual-targets C++ + Dart from the generated `.proto`; ffigen covers only the D-27 C ABI.
  - Generated output never committed — clean regeneration is the drift check (GeniusTube D-26 rule; extends D-14's never-edit-generated contract to the wire schema).
  - **Messages are broken up** (user 2026-08-26): the contract factors into small messages — a `Command` envelope with a oneof of command variants, an `Event` envelope with a oneof of event variants, and the chat-shaped structs they carry — never one monolithic blob. New ELM event variants extend the oneof without rework. **(SUPERSEDED 2026-08-26, same day, by D-29 — no Jinja2→`.proto` generation: the `.proto` itself is the authored schema source and protoc is the both-sides generator. The broken-up-messages and append-only requirements carry over to it.)**

- **D-29:** **Org-wide FFI data plane adopted: codec-tagged bytes, per-store codec binding — GCS binds protobuf.** (2026-08-26, per user; amended same-day from an initial JSON-only draft after the wallet-widget consideration. Org-wide pattern — sibling copies in genius-tube (their D-27/D-28) and genius-ai-boss (their D-20/D-20a) were NOT yet written in those repos' planning trees when this was locked, so this decision is the canonical statement; sync tracked in `.planning/todos/pending/sync-org-ffi-data-plane-decision.md`.)
  - **Data plane (unchanged from D-27):** in-process topic pub/sub; minimal C ABI = init / publish / subscribe; the C++ core owns application state, Dart Cubits are thin subscribers; commands are topic publishes; state and raw error strings arrive on the push port.
  - **ABI payloads are codec-tagged bytes** — `uint8_t* + length`, never `char*`; the codec is bound **per store at creation — per-store, not per-project; JSON is not exclusive, it is used when the data is JSON-native** (user 2026-08-26). A codec swap or a second store with a different codec is not an ABI break. genius-ai-boss binds JSON on its OpenAPI-spec'd backend channels and can bind protobuf per channel when needed; GCS's stores bind protobuf.
  - **GCS stores bind protobuf.** Chat and the eventual wallet widget are protobuf-native: the wallet's C++ Cubits will hold GeniusSDK proto types (binary keys, 64-bit amounts, decimal strings) — `SerializeToString` across FFI with zero adaptation, no proto↔JSON transcoding conventions on the Dart side. ELM `ElmExecutionEvent` protos stay C++-internal (D-27); the JSON that does exist (OpenAI-v1 HTTP) lives server-side.
  - **Schema source: the `.proto` IS the source** (org rule: "you can use OpenAPI or .protobuf"). A small hand-authored contract with **broken-up messages** — a `Command` envelope with a oneof of command variants, an `Event` envelope with a oneof of event variants, and the chat-shaped structs they carry — compiled via `add_proto_library` for C++ + the Dart protobuf plugin: **protoc is the both-sides generator; Jinja2 is out of the wire path entirely** (it stays for widget composites per D-17). Direct authoring is safe because GCS has exactly one schema source (GeniusTube's never-hand-author rule guards against two-source drift, which GCS doesn't have). Schema-source canonicality is a **per-project choice** org-wide (user 2026-08-26); if an OpenAPI artifact is ever wanted for GCS, it is a GENERATED OUTPUT (`protoc-gen-openapiv2`/buf — the standard proto→OpenAPI direction), never an authored source. Net shape = D-26's original protobuf wire + D-27's topic pub/sub + the codec-tagged ABI. Append-only discipline (D-24/ai-boss D-06/D-07) anchors on the `.proto` from day one.
  - **OpenAI-v1 endpoint is server-facing and separate:** the OpenAI-compatible API is exposed by the running app in background mode on desktop only (Windows/Linux/macOS) — a proxy to decentralized AI/LLM simulated models via ELMs (the `../SuperGenius/.planning/notes/ELM-bridging-gaps.md` flow). It speaks JSON externally and translates C++-side (`Task.json_data` is a JSON `bytes` field by design) — that translation exists regardless of the FFI codec and does not flow through this in-process plane. Not Phase 1 scope.

### Flutter UI & Scaffold (added 2026-08-19)
- **D-10:** **Pure-scaffold chat UI.** Drop `flutter_chat_ui` + `flutter_chat_core` from `src/app/pubspec.yaml`. Chat UI is composed from scaffold primitives: `ScaffoldComposer` (message input), surfaces/cards for the message list, state views/toasts for chrome. Multi-variant chat widgets (message bubble — roles × states) are data-driven composites per D-17, not hand-switched Dart. Adopt `ScaffoldStreamingRichText` / `ScaffoldCodeBlock` for bot responses when scaffold Phases 9–11 land on develop (in progress — additive atoms, no blocker).
- **D-11:** **Scaffold app-shell structure.** Screens follow the scaffold `app_screen_view` pattern with per-screen Cubits; a session Cubit owns the FFI session handle (D-01/D-04). App-specific one-off screens live in `src/app/lib/` as plain Dart — the ai-boss consumer pattern (`../apps/genius-ai-boss/frontend`); multi-variant composites are generated per D-17 (templates + vars in our repo, e.g. `src/app/templates/`).
- **D-12:** **Scaffold theming is the source of truth.** `src/app/scaffold/design_tokens.json` + scaffold `theme/` package. Delete the hardcoded purple `ColorScheme.fromSeed` in `src/app/lib/main.dart`.
- **D-13:** **Track scaffold `develop`.** Pin bumped 2026-08-20: `1cd3759` → `ef16a0c` (53 commits behind; gains Phase 8 atoms — `ScaffoldComposer`, `ScaffoldChip/ChipGroup`, `ScaffoldDisclosure`, `ScaffoldTraceList`, light palette — all absent from the old pin, plus bug fixes `1762a45` Source A template-dir flag and `ef16a0c` `--api-specs-dir` override). Scaffold Phases 9–11 land automatically thereafter.
- **D-14:** **Never edit scaffold generated families** (`scaffold_{animated_display,formatted_value,image_placeholder,selection_indicator}_*`, `scaffold_{card,state_view,search_bar}*` — committed Jinja2 output; edit `templates/components/*.jinja2` + regenerate instead, per `src/app/scaffold/CLAUDE.md`). App-side composites/wrappers import `package:frontend_scaffold` atoms; scaffold `lib/` is a read-only public contract (ai-boss pattern — consumer-space widgets in `../apps/genius-ai-boss/frontend`, backend CMake consumption in `../apps/genius-ai-boss/backend`).
- **D-15:** **Build from repo root:** `mkdir -p build/OSX/Debug && cd build/OSX/Debug && cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug && ninja`. Entry chain: `build/OSX/CMakeLists.txt` → `build/CommonBuildParameters.cmake` → `cmake/CommonBuildParameters.cmake` (the add_subdirectory hub, modeled on `../apps/genius-ai-boss`). Template-only regeneration: scaffold's `scaffold_generate_templates` / `generate_all_components` targets (note: no `frontend_template` target exists anywhere — verified against scaffold pin, scaffold origin/develop, ai-boss, genius-tube).
- **D-16:** **`src/CMakeLists.txt` owns the whole src tree** — `gcs_core` lib, `src/ffi/` `gcs_ffi` shared lib (D-02), AND the Flutter app under `src/app/` (via `add_subdirectory(app)`). No frontend wiring in `cmake/CommonBuildParameters.cmake` — its existing `add_subdirectory(src)` pulls everything through the one hub. Dart/flutter detection + gating (ai-boss `FRONTEND_BUILD_ENABLED` style) lives inside `src/CMakeLists.txt` / `src/app/CMakeLists.txt`, keeping the C++-only CI path clean (D-08).
- **D-17:** **Data-driven chat composites.** Multi-variant chat widgets — the message bubble (roles × states locked in D-18/D-19; text-only payload per D-20; three variants — text bubble, code block, media — sharing an interleave-capable message-flow envelope) and any other multi-variant chat widget — are **generated from our own `.jinja2` templates + `_vars.json` via the scaffold `engine.py`**, not hand-switched Dart. Templates + vars live in our repo (e.g. `src/app/templates/`); a `src/app/CMakeLists.txt` custom target drives `engine.py` with `--template-dir` (our templates + scaffold's), scaffold `design_tokens.json` (D-12), and our vars. One-off screens (chat shell / `app_screen_view`) remain plain Dart — no variant data to drive. Never hand-edit generated output; regenerate + drift-check per `src/app/scaffold/CLAUDE.md`. (Root cause: the hand-written-Dart assumption was a remnant of `flutter_chat_ui`'s LLM chat widgets, dropped in D-10.)

### Chat Variant Taxonomy & App Shell (locked 2026-08-20)
- **D-18:** **Message roles.** Three roles: `user`, `assistant` (GCS bot), `system` (join/leave/moderation notices). `error` is NOT a role — it's a state (D-19). The `user` role carries a `self`/`peer` distinction (alignment + color for "mine" vs "theirs"); the bubble composite exposes it as a variant flag, not a separate role.
- **D-19:** **Message states.** Five states: `pending` (user send in-flight), `streaming`, `thinking`, `complete`, `error`. `thinking` is a distinct visual state — the "…" animating indicator shown pre-generation, styled like the typing animation; it is NOT `streaming` (no tokens arriving yet).
- **D-20:** **Text-only bubbles (iMessage model); three message composites sharing an interleave-capable flow envelope.** The message bubble's payload is text only. Code and media are NOT wrapped in bubble chrome — they are their own composites (a `code_block` composite wrapping `ScaffoldCodeBlock` when it lands, a `media` composite) rendered inline in the conversation flow, exactly like iMessage renders a photo/link-card outside the bubble. The **message flow list** is the composite that interleaves the three heterogeneous item types (`[text bubble][code block][text bubble][media]…`) — all three message composites share the flow envelope (per-item variant data → which composite to render) so interleaving is supported from the start, even though Phase 1 only exercises text. Non-text content data still flows through the same FFI stream; only the flow's rendering branches.
- **D-21:** **App shell = left rail with space→room tree.** Standard chat layout (Slack/Discord/iMessage sidebar): left rail holds spaces, each expanding to its rooms; the user must be able to switch spaces and rooms from the rail in Phase 1 (not a Phase 2 deferral — the rail is navigation, not full space management). Main pane = the active room's message flow + composer.
- **D-22:** **Theming via scaffold's existing light/dark toggle.** Reuse the scaffold light/dark toggle (`ScaffoldPalette.lightPalette` + theme/ from Phase 8) as-is — no new toggle work; D-12's design_tokens.json drives both palettes.
- **D-23:** **App class is `GCSChat`.** The stock `src/app/lib/main.dart` `MyApp` is renamed/replaced by `GCSChat` (small Dart update, part of the D-10/D-11/D-12 rewrite); `src/app/test/widget_test.dart` (which references the nonexistent `MyApp` and fails analyze today) is rewritten to pump `GCSChat`, not patched.
- **D-24:** **Generate both halves per message composite (Dart widget + C++ Cubit-state).** Consistent with the ai-boss frontend-templates C++-interface model (`../apps/genius-ai-boss/.planning/workstreams/frontend-templates/phases/07-c-interface-templates/07-CONTEXT.md`): their D-02 "always generate both halves" — the C++ half in Phase 1 is minimal (one `chat_message_state.hpp.jinja2` template driven by the bubble's `_vars.json`, serialized via `nlohmann_json` already linked), establishing the both-halves invariant without over-building. Scaffold's `templates/cpp/` is an empty consumer-owned placeholder; our C++ state template lives in `src/app/templates/cpp/`. **Merged constraints from their 07 discussion (contract ingestion):** the generated store is **contract-agnostic with a pluggable importer seam** (their D-04) — FFI/JSON is Phase 1's only importer, but the wire format never leaks into the store's public surface (no FFI types in the store API); **reuses the contract types directly as state fields** (their D-05 — no independent canonical struct + translate layer); and the message contract (`_vars.json` → FFI payload) is **append-only from day one** (their D-06/D-07 — only add fields, never remove/retype/reorder; the CI breaking-change guard itself lands with their phase, we adopt the discipline now). Ownership stays per-instance at composition (their D-01 — consistent with our D-04/D-05/D-11).

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

### Contract-ingestion ownership model (merged 2026-08-21)
- `../apps/genius-ai-boss/.planning/workstreams/frontend-templates/phases/07-c-interface-templates/07-CONTEXT.md` — the hybrid composable C++/Dart ownership model our D-24 adopts: per-instance ownership at composition (D-01), always generate both halves (D-02), recursive composition (D-03), contract-agnostic store with pluggable importers (D-04), reuse generated contract types directly (D-05), append-only schema evolution + CI guard (D-06/D-07), GNUS.AI C++ standards (D-08).

### FFI topic pub/sub pattern (synced 2026-08-26)
- `../apps/genius-tube/.planning/phases/00-architecture-schema-freeze/00-CONTEXT.md` — frozen GeniusTube decisions our D-27/D-28 import: D-21 (in-process topic pub/sub, C++ Cubits, thin Dart subscribers, raw error strings), D-03a/D-25 (protobuf for the FFI data plane only, schema sources over hand-authored .proto), D-26 (single stamp chain, generated output never committed). NOTE: GT's own FFI implementation is still Phase-0 error/version stubs — the contract lives in these decisions, not in GT code. *(Post-sync 2026-08-26: D-29 lands GCS ON protobuf, aligned with GT's D-03a/D-25 — codec-tagged bytes with per-store binding. GT's never-hand-author rule is moot for GCS: it guards against two-source drift, and GCS's only schema source is the `.proto` itself.)*
- `../SuperGenius/.planning/notes/ELM-bridging-gaps.md` — ELM v1.1 model behind D-27's invisible-orchestration rule: ONE funded SuperGenius job with N ELM work items at the fixed rate; SuperGenius's grid owns scheduling/participant selection; minimal `ElmExecutionEvent` oneof protos for streaming when it's needed later.

### Scaffold (Flutter widget library)
- `src/app/scaffold/CLAUDE.md` — submodule contract: `lib/` is a public API surface consumed by 3 repos; generated widget families are committed Jinja2 output (edit templates, never the .dart); pre-finish checks (`dart analyze --fatal-infos`, `flutter test`).
- `src/app/scaffold/lib/components/` — ~60 M3 atoms incl. Cubit-pattern widgets (`scaffold_card_cubit`, `scaffold_state_view_cubit`, `scaffold_search_bar_cubit`).
- `src/app/scaffold/design_tokens.json` + `src/app/scaffold/lib/theme/` — theming source of truth (D-12).
- `../apps/genius-tube/src/scaffold/.planning/ROADMAP.md` — scaffold's own roadmap: Phases 9–11 in flight (StreamingRichText, CodeBlock, SelectionActions, Chart/Scrubber, coverage gate). D-10 adopts Phase 9 atoms when they land.
- `../apps/genius-ai-boss/frontend/` + `../apps/genius-ai-boss/backend/` — canonical consumer pattern (D-11/D-14): consumer-space widgets import `package:frontend_scaffold`; CMake submodule consumption.
- `../apps/genius-ai-boss/cmake/CommonBuildParameters.cmake` — frontend gating pattern (dart/flutter detection); GCS applies the gating inside `src/CMakeLists.txt` instead (D-16).

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
- Flutter app at `src/app/` (pubspec already has `ffi: ^2.2.0`); `src/app/pubspec.yaml` currently depends on `neoswarm_ffi` via path — will migrate to gcs bindings as they appear (not this phase). It also depends on `flutter_chat_ui`/`flutter_chat_core` (dropped per D-10) and `frontend_scaffold` via `path: scaffold` (kept, becomes the sole UI kit).
- `src/app/lib/main.dart` — legacy single-screen SLM chat (hardcoded Mistral path, `neoswarm_ffi` bridge, stock `Chat` widget); rewritten per D-10/D-11/D-12 in the 01-05 Dart spike.
- Scaffold pin bump (D-13) is a prerequisite for the Dart UI work — `ScaffoldComposer` et al. don't exist at the current pin.

</code_context>

<specifics>
## Specific Ideas

- "Cubit style C++ state storage with a thin flutter UI wrapper" — the Bloc/Cubit state objects live in C++; Flutter only renders and forwards user input.
- "probably only send/receive FFI calls to dart... C++ handles most everything" + callbacks instead of polling for arriving data.
- CI/CD: "kick-off those builds on our self-hosted runners and they will pick-up thirdparty & SuperGenius & GeniusSDK and download dependencies" — realized as the SuperGenius/GeniusSDK release-download pattern (D-07).
- "might as well compose chat from our scaffold primitives" (2026-08-19) — D-10 pure-scaffold UI, chosen over keeping `flutter_chat_ui` as a bridge.
- "scaffold structure, that's the whole point" — D-11 app-shell from scaffold patterns.
- "we've made this where you would never edit generated files" — D-14, pointing at ai-boss as the worked example.
- "this front end is under src/app and it's src/CMakeLists.txt builds the ffi, libs and flutter app" — D-16 correction: frontend wiring lives in `src/CMakeLists.txt`, not `cmake/CommonBuildParameters.cmake`.
- "the C FFI surface should really be a pub/subscribe to update from C++ Cubit Data to the Flutter UI bi-directionally" (2026-08-25) — D-27.
- "dart should publish a command, but the underlying messages a protobuf anyway … C++ will orchestrate and create its own pub/sub streaming for ELMs … Dart is really a very thin layer" (2026-08-25) — D-27.
- "the 100 line static proto is fine as long as messages are broken up inside it, don't think we need an OpenAPI doc" (2026-08-26) — requirement carried into D-29's authored `.proto` (the jinja-generation half of D-28 was superseded; the broken-up-messages half survives).
- "you can use OpenAPI or .protobuf, since jinja can handle both right? Not need to have both an OpenAPI and protobuf codec for this project" (2026-08-26) — D-29.
- "the OpenAI api will be exposed when the app is running (background mode) on Windows/Linux/MacOS only which is just a proxy to run decentralized AI llm simulated models with ELMs" (2026-08-26) — D-29 server-facing scope note.
- "flip to protobuf … isn't the same issue for ../apps/genius-ai-boss who picked json … can bind protobuf codec per channel it opens, so … it's not that JSON should be used exclusively, just when needed" (2026-08-26) — D-29 per-store codec principle + GCS protobuf binding.
- "Good point and each project can pick what is canonical anyway" (2026-08-26) — D-29 schema-source canonicality is per-project; GCS keeps the authored `.proto`, OpenAPI only ever a generated output.

</specifics>

<deferred>
## Deferred Ideas

- Removing/retiring `neoswarm_ffi` in favor of `gcs_ffi` — re-evaluate at Phase 6 (GCS Bot) when inference routes through gcs_core.
- Flutter app builds in CI (iOS/Android app bundles) — later phase; Phase 1 CI covers C++ core only.
- zkLLVM in CI — not needed by GCS; skip.
- Message CRDT schema design — tracked in `.planning/todos/pending/design-message-crdt-schema.md`, resolves Phase 3.
- GCS bot identity mapping — tracked in `.planning/todos/pending/define-gcs-bot-identity-mapping.md`, resolves Phase 6.
- Adopting scaffold Phase 9–10 atoms beyond chat (`ScaffoldChart`/`ScaffoldChartScrubber`, `ScaffoldSelectionActions`) — evaluate when they ship on scaffold develop; not Phase 1 scope.
- `flutter_chat_ui`/`flutter_chat_core` removal lands with the 01-05 Dart rewrite (D-10); if any chat-kit capability is missed, note it for Phase 3 (Messaging) rather than re-adding the dep.

</deferred>

---

*Phase: 1-Foundation*
*Context gathered: 2026-08-15; updated 2026-08-19 (scaffold/UI decisions D-10..D-16); updated 2026-08-20 (D-17 data-driven composites; D-18..D-22 chat variant taxonomy + app shell); updated 2026-08-21 (D-23 GCSChat app class + widget_test; D-24 both-halves C++ Cubit-state + merged ai-boss 07 contract-ingestion ownership model; D-25 GcsGlobalDb moves to root src/lib); updated 2026-08-26 (D-27 symmetric topic pub/sub FFI + D-28 generated gcs_chat.proto from Jinja2 — synced with GeniusTube 00-architecture-schema-freeze and ELM v1.1 notes; then D-29 org-wide FFI data plane same day — codec-tagged bytes with per-store codec binding, amended same-day from a JSON-only draft to protobuf stores (`.proto` as the authored schema source, protoc as the both-sides generator); supersedes D-28's jinja-generated proto, and D-26's wire bullet resolves back to protobuf)*
