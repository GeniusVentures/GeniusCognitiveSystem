---
phase: 01-foundation
plan: 08
subsystem: ui
tags: [jinja2, codegen, flutter, cubit, bloc, chat-bubble, scaffold, cmake]

# Dependency graph
requires:
  - phase: 01-foundation plan 01-03
    provides: src/proto/gcs_chat.proto (MessageRole/MessageState enums — the C++ half this plan mirrors)
  - phase: 01-foundation plan 01-07
    provides: src/app/CMakeLists.txt FRONTEND_BUILD_ENABLED gate + cache vars (TEMPLATES_DIR/GENERATED_DIR/FRONTEND_TARGET) + scaffold add_subdirectory wiring
provides:
  - chat_message_bubble composite: 4 generated role variant widgets (ChatMessageBubble{UserSelf,UserPeer,Assistant,System}) + 5-state chrome registry (Dart triple only)
  - src/app/templates/components/ Jinja2 templates + _vars.json (the D-17 authoring pattern, proven on ONE composite)
  - CMake bubble stamp-triple loop + app_generate_chat_components aggregate (extend point for 01-09/01-10)
  - generated kValidRoles/kValidStates axis snapshots (append-only contract mirroring the proto taxonomy)
affects: [01-foundation plan 01-09 (code_block/media), 01-foundation plan 01-10 (flow envelope), 01-foundation plan 01-11 (shell wiring)]

# Tech tracking
tech-stack:
  added: []  # no new deps — jinja2 (system python) + flutter_bloc ^9.0.0 + frontend_scaffold already present
  patterns:
    - "D-17 composite authoring: _vars.json axes (roles/states/payload) iterated by templates; StrictUndefined is the drift guard"
    - "State chrome dispatch via a generated const registry map (data-driven lookup) instead of a runtime switch; identity fallback for unregistered values"
    - "Consumer-owned stamp-triple codegen loop mirroring scaffold CMakeLists §6, ours-first dual --template-dir in CMake list form (Pitfall 1762a45)"

key-files:
  created:
    - src/app/templates/components/chat_message_bubble_vars.json
    - src/app/templates/components/chat_message_bubble.dart.jinja2
    - src/app/templates/components/chat_message_bubble_cubit.dart.jinja2
    - src/app/templates/components/chat_message_bubble_state.dart.jinja2
    - src/app/lib/generated/chat/chat_message_bubble.dart
    - src/app/lib/generated/chat/chat_message_bubble_cubit.dart
    - src/app/lib/generated/chat/chat_message_bubble_state.dart
  modified:
    - src/app/CMakeLists.txt

key-decisions:
  - "State chrome selection = generated const registry map (state string -> per-state builder fn), never a runtime switch; _chromeIdentity is the fallback for values outside the registry (proto MESSAGE_STATE_UNSPECIFIED)"
  - "system role follows 01-UI-SPEC over the plan's looser prose: centered alignment, transparent fill, borderSubtle 1px outline, textSecondary"
  - "didUpdateWidget re-seeds the internal cubit on instanceId only, never on text — payload flows through the cubit (D-04) and re-seeding on text would reset the state axis mid-stream"
  - "kValidRoles/kValidStates generated as const snapshots on ChatMessageBubbleState; the cubit asserts against them (axis drift surfaces at construction, mirroring the proto taxonomy)"

patterns-established:
  - "Composite template shape: role axis -> one public StatefulWidget class per role (snake_case -> PascalCase suffix derived structurally), uniform constructor surface (instanceId/text/senderName/cubit)"
  - "Chrome builder typedef: (context, content, wrapSurface) lets a state restyle content, replace it (_ThinkingDots), or wrap the whole surface (Opacity dim, _ErrorOverlay tint)"
  - "Aggregate target app_generate_chat_components is the single extend point: 01-09/01-10 append stamps; app_analyze/app_test already chain through it"

requirements-completed: [CORE-05]

# Metrics
duration: 9min (execution window; excludes ~15min plan/context reading)
completed: 2026-08-26
---

# Phase 01 Plan 08: chat_message_bubble Composite (D-17) Summary

**Jinja2-generated Dart chat bubble: 4 role variant widget classes + 5-state chrome registry rendered by scaffold engine.py into lib/generated/chat/, wired as a bubble-only CMake stamp-triple loop (no C++ half — the proto IS the C++ half per D-26)**

## Performance

- **Duration:** 9 min execution window (plus ~15 min plan/context reading)
- **Started:** 2026-08-26T23:14:59Z
- **Completed:** 2026-08-26T23:23:27Z
- **Tasks:** 2 of 2
- **Files modified:** 8 (7 created, 1 modified)

## Accomplishments
- Authored the first D-17 data-driven composite: `chat_message_bubble` templates + `_vars.json` under `src/app/templates/components/` (roles/states/payload axes mirror `MessageRole`/`MessageState` in `src/proto/gcs_chat.proto`)
- Generated the Dart triple into `src/app/lib/generated/chat/` — four role classes (`ChatMessageBubbleUserSelf/UserPeer/Assistant/System`), state chrome via a generated const registry (`_ThinkingDots`, `_ErrorOverlay`, pending Opacity dim, identity), zero runtime switches
- Wired the consumer-owned CMake codegen loop (`generate_composite_chat_message_bubble` + `app_generate_chat_components` aggregate) and chained `app_analyze`/`app_test` through it; verified end-to-end through `cmake --build build/OSX/Debug`

## Task Commits

Each task was committed atomically:

1. **Task 1: Author chat_message_bubble _vars.json + three Dart templates** - `532faa4` (feat)
2. **Task 2: Wire the bubble codegen loop + render the Dart triple** - `5c6ab02` (build)

**Plan metadata:** see final docs commit below.

## Files Created/Modified
- `src/app/templates/components/chat_message_bubble_vars.json` - variant axes: roles [user_self,user_peer,assistant,system], states [pending,streaming,thinking,complete,error], payload text
- `src/app/templates/components/chat_message_bubble.dart.jinja2` - one widget class per role + state-chrome registry + helper widgets
- `src/app/templates/components/chat_message_bubble_cubit.dart.jinja2` - thin FFI-event applier cubit (applyState/updateText/reset, axis asserts)
- `src/app/templates/components/chat_message_bubble_state.dart.jinja2` - immutable role+state+text class with generated axis snapshots
- `src/app/lib/generated/chat/chat_message_bubble.dart` - generated (committed; never hand-edited): 4 role classes, 777 lines
- `src/app/lib/generated/chat/chat_message_bubble_cubit.dart` / `..._state.dart` - generated cubit/state
- `src/app/CMakeLists.txt` - bubble stamp-triple loop + aggregate targets + analyze/test DEPENDS chain

## Decisions Made
- **State chrome dispatch = generated registry map, not a switch.** `_stateChrome` is a `const Map<String, builder>` generated by iterating the states axis; `_chromeIdentity` covers unregistered values (proto `MESSAGE_STATE_UNSPECIFIED`). Append-only-safe: a new state in `_vars.json` regenerates an identity entry until a treatment is authored.
- **system role follows 01-UI-SPEC** (centered, transparent fill, borderSubtle outline, textSecondary) where the plan's prose loosely said "peer/assistant/system = leading" — the plan defers to UI-SPEC for the visual contract, and UI-SPEC line 225/144 is explicit.
- **didUpdateWidget re-seeds on `instanceId` only.** The scaffold card pattern re-seeds on its variant param; copying that onto `text` would reset a streaming message's state axis on every pushed payload. Payload updates flow exclusively through the cubit (D-04).
- **Axis snapshots as public consts** (`ChatMessageBubbleState.kValidRoles/kValidStates`) give 01-10's flow envelope the structural role->class dispatch data and make axis drift fail loudly at cubit construction.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Plan's dev-loop engine path was wrong**
- **Found during:** Task 2 (manual render)
- **Issue:** Plan's command cited `scaffold/tools/engine.py`; the engine actually lives at `scaffold/tools/scaffold_codegen/engine.py` (verified against scaffold CMakeLists `ENGINE_SCRIPT` cache var, line 84).
- **Fix:** Used the correct path for all manual renders; the CMake loop references `${ENGINE_SCRIPT}` so it was unaffected.
- **Files modified:** none (command-only correction)
- **Verification:** all six manual renders (3 to /tmp, 3 to lib/generated) succeeded
- **Committed in:** n/a

---

**Total deviations:** 1 auto-fixed (blocking path correction)
**Impact on plan:** None on scope or artifacts — the CMake wiring already used the correct `${ENGINE_SCRIPT}` reference; only the plan's illustrative manual command was stale.

## Issues Encountered
None. Template authoring went through one StrictUndefined-clean render cycle; a mid-authoring Jinja whitespace bug (`{%- endfor %}` trailing-comma edit) was caught by the render step itself and fixed before any commit.

## TDD Note (Task 1, tdd="true")

The plan ships no test files (its <verify> commands are grep oracles; the widget-test fixture is 01-11/VALIDATION scope). TDD was honored at the plan's own verification level: Task 1's `<verify>` command was executed RED before authoring (failed: files absent), then GREEN after authoring (passed). No separate RED test commit exists because the oracle is a shell predicate, not a committed test file.

## User Setup Required
None - no external service configuration required.

## Next Phase Readiness
- Pattern proven on one composite exactly as RESEARCH directed — 01-09 (code_block/media) and 01-10 (flow) extend `app_generate_chat_components` by appending stamps to the existing loop shape
- `dart analyze lib/generated/chat` is clean ("No issues found"); full-package analyze still waits on 01-11's shell rewrite (legacy `main.dart` remains)
- Run/tail rules (iMessage-style runs, max-width 78%/560px, timestamps) deliberately left to the 01-10 flow envelope, which owns run grouping

## Self-Check: PASSED

- Files exist: all 7 created files verified on disk (`[ -f ]` each)
- Commits exist: `532faa4`, `5c6ab02` present on `feature/app-ffi-data-plane` (`git log --oneline`)
- Plan <verification> re-run: generated markers present (4 role classes), analyze clean, re-render drift-free (md5-identical), no `chat_message_state.hpp`/`nlohmann` references in src/app/CMakeLists.txt

---
*Phase: 01-foundation*
*Completed: 2026-08-26*
