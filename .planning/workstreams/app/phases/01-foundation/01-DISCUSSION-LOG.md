# Phase 1: Foundation - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-08-15
**Phase:** 1-Foundation
**Areas discussed:** FFI surface shape, FFI boundary / library layout, async/event model across FFI, CI/CD structure

---

## FFI Surface Shape

| Option | Description | Selected |
|--------|-------------|----------|
| Per-module `extern "C"` headers | One header per module (`gcs_chat.h`, `gcs_rooms.h`, ...) | |
| Single opaque-handle session API | One header; `gcs_init` → handle; all calls take the handle | ✓ |

**User's choice:** "a single opaque-handle API"
**Notes:** Rationale given with area 2: Cubit-style C++ state storage with a thin Flutter UI wrapper.

## FFI Boundary / Library Layout

| Option | Description | Selected |
|--------|-------------|----------|
| Thin C wrapper inside `gcs_core` | Flutter links gcs_core static lib directly | |
| Separate `gcs_ffi` shared library | Own CMake target, links gcs_core; Flutter loads dylib/so/dll | ✓ |

**User's choice:** "separate gcs_ffi shared library, not sure neoswarm_ffi is needed any more, but if so, that's ok"
**Notes:** neoswarm_ffi left working; removal re-evaluated at Phase 6. Recorded as D-03.

## Async / Event Model Across FFI

| Option | Description | Selected |
|--------|-------------|----------|
| Dart polls C++ | Dart calls receive/poll on timer or after send | |
| Native→Dart callbacks | C++ pushes events via ffigen NativeFunction + ReceivePort | ✓ |

**User's choice:** "No, dart doesn't need to poll, but the point is that C++ handles most everything and Dart is a thin wrapper for input/output text or selecting channels, config, etc. But it of course can receive callbacks instead of polling for that data arriving."
**Notes:** Initial "send/receive FFI calls to dart" phrasing was clarified in follow-up — callbacks, not polling. Same pattern as neoswarm_ffi already uses.

## CI/CD Structure

| Option | Description | Selected |
|--------|-------------|----------|
| Monolithic matrix workflow | One workflow, 5-platform matrix, SuperGenius-style | ✓ |
| Per-platform workflows | Separate workflow files per platform | |
| Deps pre-installed on runners | Runner images carry thirdparty/SuperGenius/GeniusSDK | |
| Deps downloaded as prebuilt releases | gh release download from thirdparty/SuperGenius/GeniusSDK release tags | ✓ |

**User's choice:** "Check how ../SuperGenius repo does the CI/CD"
**Notes:** SuperGenius `cmake.yml` inspected: resolve-runners job (self-hosted labels sg-ubuntu-linux / sg-arm-linux / SG-WIN11 / gv-OSX-Large with GitHub-hosted fallback), matrix Android(arm64-v8a, armeabi-v7a)/iOS/OSX/Linux(x86_64, aarch64)/Windows × Debug/Release, prebuilt thirdparty release tarballs, ctest, release upload. GeniusSDK workflow confirmed as the consumer-repo analog (downloads SuperGenius + thirdparty). GCS CI copies this, adds SuperGenius + GeniusSDK download steps, skips zkLLVM. Phase 1 CI builds C++ core only (no Flutter app build).

## Claude's Discretion

- Exact C API names/signatures (C-only, opaque handle, C++17)
- GTest smoke-test specifics (wait-condition templates, no sleep_for)
- `gcs_ffi` target placement under `src/ffi/` (expected home per user's repo restructure)

## Deferred Ideas

- Retiring `neoswarm_ffi` → re-evaluate Phase 6 (GCS Bot)
- Flutter app bundles in CI → later phase
- zkLLVM in CI — not needed by GCS
- Message CRDT schema → Phase 3 (todo tracked)
- GCS bot identity mapping → Phase 6 (todo tracked)

---

# Round 2 — Scaffold / Flutter UI (2026-08-19)

**Trigger:** "the submodule under src/app/scaffold has been updated and will be updated some more with new widgets."

**Areas discussed:** Scaffold widget adoption, app shell structure, theming, scaffold update workflow, codegen drift guard, build system integration.

## Scaffold Widget Adoption

| Option | Description | Selected |
|--------|-------------|----------|
| A — Pure scaffold | Drop flutter_chat_ui/flutter_chat_core; compose chat from ScaffoldComposer + surfaces/cards; adopt Phase 9 StreamingRichText when it lands | ✓ |
| B — Hybrid bridge | Keep flutter_chat_ui Chat widget this phase, scaffold for chrome; swap in Phase 3 | |

**User's choice:** "A." (after "might as well compose chat from our scaffold primitives")
**Notes:** Recorded as D-10. Scaffold Phases 9–11 (genius-tube .planning/ROADMAP.md) reviewed — StreamingRichText/CodeBlock/SelectionActions/Chart are additive atoms, no blocker. Pin at discussion time (1cd3759) was 49 commits behind origin/develop and lacked ALL Phase 8 atoms — pin bumped 2026-08-20 to ef16a0c (D-13), which also includes the Source A template-dir and --api-specs-dir bug fixes.

## App Shell Structure

**User's choice:** "scaffold structure, that's the whole point."
**Notes:** Recorded as D-11 — app_screen_view pattern, per-screen Cubits, session Cubit owns FFI handle.

## Theming

**User's choice:** "use scaffold theming"
**Notes:** Recorded as D-12 — design_tokens.json + scaffold theme/; delete hardcoded ColorScheme.fromSeed in main.dart.

## Scaffold Update Workflow

**User's choice:** "track scaffold develop"
**Notes:** Recorded as D-13 — pin bumped 2026-08-20: 1cd3759 → ef16a0c (includes Phase 8 atoms + Source A template-dir fix + --api-specs-dir override fix); track develop going forward.

## Codegen Drift Guard

**User's choice:** "never edit generated families and you shouldn't need to, we've made this where you would never edit generated files... look at ../apps/genius-ai-boss/backend and ../apps/genius-ai-boss/frontend for how to do it."
**Notes:** Recorded as D-14. ai-boss pattern verified: consumer-space widgets import package:frontend_scaffold atoms; scaffold lib/ is read-only contract; generated families (scaffold_{animated_display,formatted_value,...}) are committed Jinja2 output edited only via templates/components/*.jinja2 + regenerate (scaffold CLAUDE.md).

## Build System Integration

**User's choice:** Build from root: `mkdir -p build/OSX/Debug && cd build/OSX/Debug && cmake .. -G Ninja -DCMAKE_BUILD_TYPE=Debug && ninja`. "cmake/CommonBuildParameters.cmake includes the other CMakeLists.txt files for submodule building. You can model this off of ../apps/genius-ai-boss repo."
**Correction (user):** "this front end is under src/app and it's src/CMakeLists.txt builds the ffi, libs and flutter app" — D-16 revised: frontend add_subdirectory lives in src/CMakeLists.txt (not cmake/CommonBuildParameters.cmake); dart/flutter gating inside src/ tree.
**Notes:** Recorded as D-15/D-16. ai-boss CommonBuildParameters.cmake reviewed (FRONTEND_BUILD_ENABLED + dart/flutter detection pattern). Verified `frontend_template` target does not exist (scaffold pin, scaffold origin/develop, ai-boss, genius-tube all checked) — actual template targets: scaffold_generate_templates / generate_all_components; flagged to user.

## Round 2 Claude's Discretion

- Exact src/app/CMakeLists.txt layout (custom targets for pub get / analyze / test)
- Message-list composition details from scaffold atoms (grouping, timestamps) until Phase 9 atoms land
- Timing of the scaffold pin bump within the plan wave structure

## Round 2 Deferred Ideas

- ScaffoldChart/Scrubber/SelectionActions adoption beyond chat → evaluate when shipped
- flutter_chat_ui capability gaps, if any surface → Phase 3 (Messaging), don't re-add dep

## Round 2 Amendment — Data-driven composites (2026-08-20)

**Trigger:** "I don't agree... the whole point of the jinja2 is so we use data driven development, not hardcoding with dart."
**Root cause of the original D-10/D-11 implication:** the hand-written-Dart assumption was a remnant of `flutter_chat_ui` (Flutter's LLM chat widgets) carried over when D-10 dropped it.
**Resolution:** D-17 — multi-variant chat composites (message bubble roles × states) are generated from our own `.jinja2` + `_vars.json` via the scaffold `engine.py` (repeatable `--template-dir`, `--tokens` + `--vars`, StrictUndefined). Built-in `generate_all_components` is a closed 6-composite list and does NOT read `TEMPLATES_DIR` (the Source B comment claiming otherwise is aspirational); the engine itself is general-purpose, so we drive it from `src/app/CMakeLists.txt`. One-off screens stay plain Dart.

---

# Round 3 — Chat Variant Taxonomy & App Shell (2026-08-20)

**Trigger:** Replan prep for scope-B Phase 1 (C++ skeleton + Flutter app shell + theming + message-bubble composite). D-17's guessed taxonomy (`user`/`assistant`/`system` × `streaming`/`thinking`/`error`/`complete`) needed locking before UI-SPEC + replan.

**Areas discussed:** (1) bubble roles, (2) bubble states, (3) message payload/composition, (4) app-shell left rail, (5) theming application.

## Roles

**User's choice:** "yes, all that" (user / assistant / system), with error as a state not a role.
**Notes:** Recorded as D-18. `user` carries a `self`/`peer` distinction (mine vs theirs alignment/color) exposed as a variant flag, not a fourth role. `system` = join/leave/moderation notices.

## States

**User's choice:** "all and thinking is a state, but visual just like the ... animating when getting a message"
**Notes:** Recorded as D-19. Five states: `pending` (send in-flight), `streaming`, `thinking`, `complete`, `error`. `thinking` is a distinct visual state (the "…" typing-style animation shown pre-generation) — NOT collapsed into `streaming`, which implies tokens are already arriving.

## Message Payload / Composition

**User's choice:** "message bubbles should be for text only, so for chat text only, other parts, like media, code, etc, don't appear in a bubble just like iMessage." Follow-up: interleaving flow vs text-only-Phase-1 → "Correct" (to the interleave-capable reading).
**Notes:** Recorded as D-20. Bubble payload = text only. Code/media are separate composites rendered inline in the flow, not wrapped in bubble chrome (iMessage model — photo/link-card sits outside the bubble). Three message composites (text bubble, code block, media) share an interleave-capable message-flow envelope from the start; Phase 1 only exercises text, but the flow supports mixed lists. Non-text content data flows through the same FFI stream; only rendering branches.

## App Shell / Left Rail

**User's choice:** "Rooms, etc, can be in left bar, just like any other chat system." Follow-up: flat room list vs space→room tree → "b, not really premature, as you have to be able to switch" (space→room tree).
**Notes:** Recorded as D-21. Left rail = space→room tree (spaces expand to rooms); switching spaces/rooms from the rail is Phase 1 scope (navigation, not full space management). Main pane = active room's message flow + composer. (A doc link was attached to the reply and later rescinded by the user — decision stands on the text alone.)

## Theming Application

**User's choice:** "light/dark toggle is already implemented in scaffold so might as well use it."
**Notes:** Recorded as D-22. Reuse scaffold's existing light/dark toggle (Phase 8 `ScaffoldPalette.lightPalette` + theme/) as-is; no new toggle work. D-12's design_tokens.json drives both palettes.

## Round 3 Claude's Discretion

- Exact variant axes naming in `_vars.json` (role/state enum values, self/peer flag) — planner + UI-SPEC detail.
- Which scaffold atom hosts the "…" thinking animation (likely a state/pending atom) — UI-SPEC.
- Whether the code-block composite wraps scaffold's Phase 9 `ScaffoldCodeBlock` directly or a placeholder until it lands on develop — research refresh.

