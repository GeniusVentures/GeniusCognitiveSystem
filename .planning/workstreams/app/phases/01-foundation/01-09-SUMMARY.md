---
phase: 01-foundation
plan: 09
subsystem: ui
tags: [flutter, dart, jinja2, codegen, bloc-cubit, frontend-scaffold, chat-composite]

# Dependency graph
requires:
  - phase: 01-foundation/01-08
    provides: bubble-only stamp-triple codegen loop (app_generate_chat_components) + the template/vars pattern under src/app/templates/components/
provides:
  - chat_message_code_block Dart triple (widget/cubit/state) generated into src/app/lib/generated/chat/, wrapping scaffold ScaffoldCodeBlock
  - chat_message_media Dart triple generated into src/app/lib/generated/chat/, wrapping scaffold MediaCard with a documented retrieval seam
  - Generalized composite codegen loop (_chat_composites list) + per-composite generate_composite_* dev-loop targets
affects: [01-foundation/01-10 (flow envelope), 01-foundation/01-11 (cubits + shell)]

# Tech tracking
tech-stack:
  added: []  # no new deps — reuses scaffold engine.py, jinja2, flutter_bloc, frontend_scaffold atoms
  patterns:
    - "Chrome-less payload composite: scaffold atom owns the surface (ScaffoldCodeBlock/MediaCard), composite owns state-axis chrome registry only"
    - "Snapshot payload model (D-04): cubit appliers replace full pushed values; no stream assembly or content synthesis Dart-side"
    - "Per-composite generate_composite_* targets alongside the app_generate_chat_components aggregate (RESEARCH D-17 shape)"

key-files:
  created:
    - src/app/templates/components/chat_message_code_block_vars.json
    - src/app/templates/components/chat_message_code_block.dart.jinja2
    - src/app/templates/components/chat_message_code_block_cubit.dart.jinja2
    - src/app/templates/components/chat_message_code_block_state.dart.jinja2
    - src/app/templates/components/chat_message_media_vars.json
    - src/app/templates/components/chat_message_media.dart.jinja2
    - src/app/templates/components/chat_message_media_cubit.dart.jinja2
    - src/app/templates/components/chat_message_media_state.dart.jinja2
    - src/app/lib/generated/chat/chat_message_code_block.dart
    - src/app/lib/generated/chat/chat_message_code_block_cubit.dart
    - src/app/lib/generated/chat/chat_message_code_block_state.dart
    - src/app/lib/generated/chat/chat_message_media.dart
    - src/app/lib/generated/chat/chat_message_media_cubit.dart
    - src/app/lib/generated/chat/chat_message_media_state.dart
  modified:
    - src/app/CMakeLists.txt

key-decisions:
  - "Wrapped the real ScaffoldCodeBlock/MediaCard atoms instead of the plan's placeholder branch — both have landed at the current scaffold pin (plan drift; working code wins)"
  - "vars carry states axis but NO roles axis — roles drive bubble chrome (alignment/fill), which D-20 excludes from these top-level items; one widget class per composite instead of per-role variants"
  - "Media thumbnail is a caller-supplied ImageProvider seam (widget param, never state) — ImageProviders cannot cross FFI and Phase 1 pushes metadata only"

patterns-established:
  - "Chrome-less composite: state-chrome registry (2-arg builder: context, item) without the bubble's wrapSurface seam"
  - "Fixed file-private helper names in templates (e.g. _snapshotLines) — never embed widget_class_name into top-level function names (lowerCamelCase lint)"

requirements-completed: [CORE-05]

# Metrics
duration: 5 min
completed: 2026-08-26
---

# Phase 01 Plan 09: Code Block + Media Composites Summary

**Data-driven chat_message_code_block (ScaffoldCodeBlock) + chat_message_media (MediaCard) payload composites rendered as chrome-less top-level flow items via the 01-08 codegen loop, drift-checked and analyze-clean**

## Performance

- **Duration:** ~5 min (2026-08-26T23:40:40Z → 2026-08-26T23:45:00Z)
- **Started:** 2026-08-26T23:40:40Z
- **Completed:** 2026-08-26T23:45:00Z
- **Tasks:** 2 of 2
- **Files modified:** 15 (8 templates/vars, 6 generated Dart, 1 CMakeLists)

## Accomplishments
- chat_message_code_block + chat_message_media template triples authored under src/app/templates/components/ (payload axes "code"/"media", states axis mirroring MessageState enum names per D-24)
- Codegen loop generalized from bubble-only to a `_chat_composites` list (bubble, code_block, media) with per-composite `generate_composite_*` dev-loop targets; `app_generate_chat_components` aggregate now covers all three triples; flow deliberately absent (01-10)
- All six generated Dart files rendered, committed, and verified: `dart analyze lib/generated/chat` clean; stamp-delete re-render byte-identical (6/6); incremental invariant held (single-template touch re-rendered exactly one stamp)
- Generated composites contain zero bubble chrome (no ScaffoldSurface/Align/ChatMessageBubble references) — D-20 top-level flow items

## Task Commits

Each task was committed atomically:

1. **Task 1: Author code_block + media composite templates + vars (top-level flow items)** - `f132c82` (feat)
2. **Task 2: Extend the codegen loop with code_block/media and render both** - `ad3304a` (build)

**Plan metadata:** this commit (docs: complete plan)

## Files Created/Modified
- `src/app/templates/components/chat_message_code_block_vars.json` - states axis + payload "code" (interfaces-block keys exactly)
- `src/app/templates/components/chat_message_code_block.dart.jinja2` - wraps ScaffoldCodeBlock; state-chrome registry; snapshot payload model (D-04)
- `src/app/templates/components/chat_message_code_block_cubit.dart.jinja2` - thin appliers (applyState/updateCode/updateLanguage/updateFilename) with kValidStates asserts
- `src/app/templates/components/chat_message_code_block_state.dart.jinja2` - immutable state (state/code/language/filename) + kValidStates axis snapshot
- `src/app/templates/components/chat_message_media_vars.json` - states axis + payload "media"
- `src/app/templates/components/chat_message_media.dart.jinja2` - wraps MediaCard; caller-supplied ImageProvider thumbnail seam
- `src/app/templates/components/chat_message_media_cubit.dart.jinja2` - thin appliers (applyState/updateMediaRef/updateTitle)
- `src/app/templates/components/chat_message_media_state.dart.jinja2` - immutable state (state/mediaRef/title) + kValidStates
- `src/app/CMakeLists.txt` - generalized `_chat_composites` loop + per-composite targets (bubble target name from 01-08 preserved)
- `src/app/lib/generated/chat/chat_message_{code_block,media}{,_cubit,_state}.dart` - committed render output (never hand-edited)

## Decisions Made
- **Wrapped the real scaffold atoms, not placeholders.** The plan instructed "wrap ScaffoldCodeBlock WHEN it lands; until then emit a placeholder" — but `scaffold/lib/components/scaffold_code_block.dart` (and `media_card.dart`) exist at the current pin. Working code wins; no hand-rolled code surface, no placeholder.
- **No roles axis in vars.** Task 1 said "same role/state axes as relevant". Roles drive bubble chrome (alignment/fill), which D-20 excludes from these composites, so each renders a single widget class. States (D-19) ARE relevant (streaming code, pending/error media) and drive the generated chrome registry.
- **Media retrieval seam placement.** `mediaRef` is FFI-pushed opaque data (state); the `ImageProvider` thumbnail is a widget parameter the shell resolves later — providers cannot cross the FFI boundary and Phase 1 pushes metadata only (UI-SPEC: media is a skeleton).
- **Fixed file-private helper names.** `_snapshotLines` instead of embedding the class name — `non_constant_identifier_names` fires on `_ChatMessageCodeBlockLines`-style names.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Plan drift vs working code] Scaffold atoms already landed — placeholder branch skipped**
- **Found during:** Task 1 (read_first scaffold atoms)
- **Issue:** Plan's code_block action prescribed a code-surface placeholder "until ScaffoldCodeBlock lands (scaffold Phases 9-11)". Both `ScaffoldCodeBlock` and `MediaCard` exist at the pinned scaffold (`src/app/scaffold/lib/components/`).
- **Fix:** Wrapped the real atoms directly — the plan's own preferred branch.
- **Files modified:** the two widget templates (vs. what a placeholder would have been)
- **Verification:** generated imports resolve; `dart analyze` clean
- **Committed in:** `f132c82`

**2. [Rule 1 - Bug] lowerCamelCase lint on generated helper name**
- **Found during:** Task 2 (dart analyze on first render)
- **Issue:** `_ChatMessageCodeBlockLines` (template embedded `{{ widget_class_name }}` into a top-level function name) triggered `non_constant_identifier_names`.
- **Fix:** Renamed to fixed file-private `_snapshotLines` in the template (mirrors the bubble's fixed-name helpers `_chromeIdentity`/`_ThinkingDots`); generated file never hand-edited — re-rendered.
- **Files modified:** `src/app/templates/components/chat_message_code_block.dart.jinja2`
- **Verification:** re-render + `dart analyze lib/generated/chat` → "No issues found!"
- **Committed in:** `ad3304a` (fix cycle 1 of max 2)

---

**Total deviations:** 2 auto-fixed (2 x Rule 1 — plan-drift resolution + lint fix)
**Impact on plan:** Both fixes required for correctness; no scope creep. Codegen loop extension, aggregate wiring, and flow-absence checks all executed exactly as planned.

## TDD Note

Task 1 carries `tdd="true"` but defines no `<behavior>`/`<implementation>` blocks — the executable verification for template authoring IS the Task 2 render + analyze + drift cycle (same shape and precedent as 01-08 Task 1, also flagged `tdd="true"`, committed as a single feat commit). Plan type is `execute`, not `tdd`, so the plan-level RED/GREEN/REFACTOR gate does not apply.

## Issues Encountered
- First drift-check attempt deleted stamps at the wrong build path (`build/OSX/Debug/src/app/...`); the `src` subdirectory is renamed `gcs_src` in the build tree. Re-ran against `build/OSX/Debug/gcs_src/app/template_gen/` — genuine forced re-render of all 6 stamps, byte-identical output confirmed by checksum diff.

## Known Stubs

| Stub | File | Line (approx) | Reason | Resolved by |
|------|------|-------|--------|-------------|
| Media thumbnail renders MediaCard's plain surface when `thumbnail` is null | `src/app/lib/generated/chat/chat_message_media.dart` (build method) | ~250 | Intentional retrieval seam: Phase 1 pushes metadata only; ImageProviders cannot cross FFI (D-04/D-29). Documented in template + generated docstring. | Media pipeline (post-Phase-1; shell resolves `mediaRef` → provider) |
| `thinking`/`streaming` chrome entries are identity for both composites | both generated widget files (chrome builders) | ~75-90 | Registry must enumerate the full states axis (append-only); no treatment authored yet — pending/error have treatments | Later UI pass if these states ever reach the leaf composites (flow envelope may own thinking entirely) |

Neither stub prevents this plan's goal (the D-20 payload taxonomy's leaf composites, generated and wired).

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- 01-10 (flow envelope) can proceed: both leaf payload composites exist with uniform `kValidStates` + `applyState` surfaces; the `_chat_composites` list + aggregate are ready for the flow entry
- 01-11 (cubits + shell) gets analyze-clean generated Dart under `lib/generated/chat/`
- No blockers

## Self-Check: PASSED

All 15 created/modified files verified present on disk; task commits `f132c82` and `ad3304a` verified in git log; plan `<verify>` commands re-run PASS (template greps, generated-file existence, CMake greps incl. no `chat_message_flow`); `dart analyze lib/generated/chat` clean; regeneration drift-free.

---
*Phase: 01-foundation*
*Completed: 2026-08-26*
