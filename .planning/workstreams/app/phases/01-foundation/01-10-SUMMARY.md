---
phase: 01-foundation
plan: 10
subsystem: ui
tags: [flutter, dart, dart3-sealed, jinja2, codegen, bloc-cubit, frontend-scaffold, chat-flow]

# Dependency graph
requires:
  - phase: 01-foundation/01-08
    provides: bubble composite triple + per-role variant classes exposing ChatMessageBubbleState.kValidRoles/kValidStates
  - phase: 01-foundation/01-09
    provides: code_block/media composite triples + the generalized _chat_composites codegen loop with per-composite generate_composite_* targets
provides:
  - chat_message_flow Dart triple (widget/cubit/state) generated into src/app/lib/generated/chat/
  - Sealed ChatFlowItem hierarchy (ChatFlowItemTextBubble/CodeBlock/Media) — the D-20 interleave envelope
  - ChatMessageFlow list widget rendering List<ChatFlowItem> via a compile-time exhaustive switch into the 01-08/01-09 composites
  - ChatMessageFlowCubit list-holder with kMaxFlowItems cap (T-01-10-02)
affects: [01-foundation/01-11 (cubits + shell — consumes ChatMessageFlow/ChatFlowItem)]

# Tech tracking
tech-stack:
  added: []  # no new deps — reuses scaffold engine.py, jinja2, flutter_bloc, frontend_scaffold atoms
  patterns:
    - "Sealed-envelope composite: Dart 3 sealed base + final subclass per item_types axis; exhaustive switch expression (no default) as the render dispatch"
    - "Unknown-axis sentinel branches: an unrecognized item_type in vars renders a deliberate syntax error — append-only violations break the build, never silently skip (T-01-10-01)"
    - "Cross-composite axis coupling assert: registry keys validated against the referenced composite's kValid* axis snapshot at render time"
    - "Snapshot seeding via parent-owned cubits: the flow seeds each leaf's cubit from the pushed item (state axis included) since leaf widget params carry payload seeds only"

key-files:
  created:
    - src/app/templates/components/chat_message_flow_vars.json
    - src/app/templates/components/chat_message_flow.dart.jinja2
    - src/app/templates/components/chat_message_flow_cubit.dart.jinja2
    - src/app/templates/components/chat_message_flow_state.dart.jinja2
    - src/app/lib/generated/chat/chat_message_flow.dart
    - src/app/lib/generated/chat/chat_message_flow_cubit.dart
    - src/app/lib/generated/chat/chat_message_flow_state.dart
  modified:
    - src/app/CMakeLists.txt

key-decisions:
  - "ChatMessageFlow is a pure renderer taking List<ChatFlowItem> (plan text), not a BlocBuilder — 01-11's shell binds its own MessageFlowCubit and passes the list"
  - "Bottom-anchoring via reverse CustomScrollView (growth offset 0 = visual bottom) rather than jump-to-bottom controller logic — newest stays visible without imperative scrolling"
  - "Flow seeds per-item leaf cubits from item fields (initialMessageState: item.state) — leaf widget params carry payload seeds only, and the leaves deliberately never re-seed from params at runtime (D-04)"
  - "kMaxFlowItems = 500 as a template constant on ChatMessageFlowState (vars stay the plan's exact 3-key shape); threat register named the constant but not the value"
  - "Bubble role->variant-class registry is template-local mirroring 01-08's roles axis, with _debugCheckBubbleVariantCoverage asserting coverage of ChatMessageBubbleState.kValidRoles (drift guard per mission context)"

patterns-established:
  - "Sealed dispatch envelope: item_types vars axis -> sealed base + one final class per axis value + one switch case per value, no default, unknown values break the build"
  - "Fixed file-private helper names (_bubbleCubit/_buildFlowItem/_cappedItems/_debugCheckBubbleVariantCoverage) — never embed widget_class_name into top-level function names (01-09 lint lesson applied)"

requirements-completed: [CORE-05]

# Metrics
duration: 8 min
completed: 2026-08-26
---

# Phase 01 Plan 10: Chat Message Flow Envelope Summary

**Sealed ChatFlowItem envelope (TextBubble/CodeBlock/Media) rendering the D-20 interleaved flow through a compile-time exhaustive switch into the 01-08/01-09 composites, wired as the fourth composite in the codegen loop**

## Performance

- **Duration:** ~8 min (2026-08-26T23:50:44Z → 2026-08-26T23:58:30Z)
- **Started:** 2026-08-26T23:50:44Z
- **Completed:** 2026-08-26T23:58:30Z
- **Tasks:** 2 of 2
- **Files modified:** 8 (4 templates/vars, 3 generated Dart, 1 CMakeLists)

## Accomplishments
- chat_message_flow composite triple authored under src/app/templates/components/ (item_types dispatch axis [text_bubble, code_block, media], exactly the plan's 3-key vars shape)
- Generated chat_message_flow.dart contains the Dart 3 sealed ChatFlowItem + ChatFlowItemTextBubble/CodeBlock/Media and renders List<ChatFlowItem> via an exhaustive no-default switch — text_bubble -> ChatMessageBubble{UserSelf,UserPeer,Assistant,System} via a role registry, code_block -> ChatMessageCodeBlock, media -> ChatMessageMedia
- Codegen loop extended: _chat_composites += chat_message_flow; per-composite target generate_composite_chat_message_flow and the app_generate_chat_components aggregate now cover all four triples (12 stamps)
- Verified: `dart analyze lib/generated/chat` clean (analyzer proves sealed-switch exhaustiveness); forced stamp re-render byte-identical (3/3, drift-free); incremental invariant held (only flow stamps re-rendered)

## Task Commits

Each task was committed atomically:

1. **Task 1: Author chat_message_flow composite (sealed ChatFlowItem envelope, D-20/D-17)** - `e6aa476` (feat)
2. **Task 2: Extend the codegen loop with flow and render** - `a9f03ee` (build)

**Plan metadata:** this commit (docs: complete plan)

## Files Created/Modified
- `src/app/templates/components/chat_message_flow_vars.json` - item_types axis [text_bubble, code_block, media] (interfaces-block keys exactly)
- `src/app/templates/components/chat_message_flow.dart.jinja2` - sealed hierarchy + role->variant registry + exhaustive switch + bottom-anchored ChatMessageFlow widget
- `src/app/templates/components/chat_message_flow_cubit.dart.jinja2` - append/replaceAll/clear appliers with oldest-out capping (_cappedItems)
- `src/app/templates/components/chat_message_flow_state.dart.jinja2` - immutable List<ChatFlowItem> state + kValidItemTypes + kMaxFlowItems
- `src/app/CMakeLists.txt` - flow added to _chat_composites; comments updated (aggregate = all four)
- `src/app/lib/generated/chat/chat_message_flow{,_cubit,_state}.dart` - committed render output (never hand-edited)

## Decisions Made
- **Pure-renderer widget, not a BlocBuilder.** The plan's action text ("takes List<ChatFlowItem>") plus 01-11's contract ("ChatMessageFlow bound to MessageFlowCubit's List<ChatFlowItem>") fix the shape: the shell owns the FFI-subscribing cubit and passes the list; the generated flow renders it. The generated ChatMessageFlowCubit is the reusable list-holder for shells that want the generated cubit instead.
- **Reverse CustomScrollView for bottom-anchoring.** UI-SPEC: "anchored to the bottom (latest message visible without scrolling)". reverse:true makes offset 0 the visual bottom — no imperative jump logic; the scroll-to-bottom affordance stays a shell contract line as the UI-SPEC itself scopes it.
- **Per-item cubit seeding from snapshots.** The leaves read payload/state through their cubits and deliberately never re-seed from widget params at runtime (01-08 D-04 comment). The flow therefore seeds a parent-owned leaf cubit from each item (initialMessageState: item.state) so the pushed D-19 state actually drives the leaf chrome registry; leaf payload params are passed alongside for API self-consistency.
- **Unknown-item-type sentinels break the build.** Without them, a vars item_type lacking a template branch would generate no subclass and no case — the switch stays exhaustive and the type is silently ignored. Both loops now emit deliberate syntax errors naming the missing branch, making T-01-10-01's "compile error, not a silent miss" real.
- **kMaxFlowItems = 500** on ChatMessageFlowState (T-01-10-02 names the constant; no value specified anywhere — Phase 1 default, chat-domain constant, documented in-code).
- **Unknown pushed role degrades to the system variant** in release builds (neutral centered notice); debug builds surface it via the bubble cubit's existing kValidRoles assert — same debug-loud/release-total split as the leaf chrome registries.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Lint] unintended_html_in_doc_comment in generated cubit**
- **Found during:** Task 2 (dart analyze on first render)
- **Issue:** Cubit doc comment contained raw `List<ChatFlowItem>` — fatal under `app_analyze --fatal-infos`.
- **Fix:** Backticked the type reference in the template; re-rendered (generated file never hand-edited).
- **Files modified:** src/app/templates/components/chat_message_flow_cubit.dart.jinja2
- **Verification:** re-render + `dart analyze lib/generated/chat` → "No issues found!"
- **Committed in:** `a9f03ee` (fix cycle 1 of max 2)

**2. [Rule 1 - Cosmetic] glued section banner from whitespace control**
- **Found during:** Task 2 (rendered-output inspection)
- **Issue:** The `{% set -%}` role-registry block stripped the newline before the registry banner (`}// ---` on one line).
- **Fix:** Whitespace-control adjustment in the template (stripping comment + non-stripping set); re-rendered.
- **Files modified:** src/app/templates/components/chat_message_flow.dart.jinja2
- **Verification:** re-render shows a clean single blank line; drift check re-run byte-identical
- **Committed in:** `a9f03ee`

---

**Total deviations:** 2 auto-fixed (2 x Rule 1 — lint fix + rendered-output cosmetics, both inside the plan's own "render, inspect, fix templates, re-render" loop)
**Impact on plan:** None beyond the intended render-fix cycle. Codegen loop extension, aggregate wiring, sealed hierarchy, and exhaustiveness all executed exactly as planned.

## TDD Note

Task 1 carries `tdd="true"` but defines no `<behavior>`/`<implementation>` blocks — the executable verification for template authoring IS the Task 2 render + analyze + drift cycle (same shape and precedent as 01-08/01-09 Task 1s, both also flagged `tdd="true"` and committed as single feat commits). Plan type is `execute`, not `tdd`, so the plan-level RED/GREEN/REFACTOR gate does not apply.

## Issues Encountered
- None. Build dir reconfigured cleanly on the CMakeLists change; stamps live at build/OSX/Debug/gcs_src/app/template_gen/ (the `src` -> `gcs_src` rename already known from 01-09).

## Known Stubs

| Stub | File | Line (approx) | Reason | Resolved by |
|------|------|-------|--------|-------------|
| Media thumbnail not passed from flow items (renders MediaCard's plain surface) | `src/app/lib/generated/chat/chat_message_flow.dart` (ChatFlowItemMedia case) | ~252 | Inherited 01-09 retrieval seam: ImageProviders cannot cross FFI and Phase 1 pushes metadata only (D-04/D-29) | Media pipeline (shell resolves mediaRef -> provider) |
| Flow horizontal inset uses space8 (phone baseline) everywhere | `src/app/lib/generated/chat/chat_message_flow.dart` (SliverPadding) | ~303 | UI-SPEC specifies space8 phone / space12 desktop; no breakpoint token exists and inventing one is shell territory | Shell layout or a later append-only template iteration |

Neither stub prevents this plan's goal (the D-20 interleave envelope, generated and exhaustive).

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- 01-11 (cubits + shell) can proceed: ChatMessageFlow/ChatFlowItem export exactly the interface its plan's <interfaces> block names — `ChatMessageFlow(items: List<ChatFlowItem>)`, `ChatFlowItemTextBubble(role/state/text/senderName)` for MessageFlowCubit's proto mapping, plus ChatMessageBubbleState.kValidRoles for role mapping
- The full D-17/D-20 composite set (bubble + code_block + media + flow) is generated, analyze-clean, and drift-free under one aggregate target
- No blockers

## Self-Check: PASSED

All 8 created/modified files verified present on disk; task commits `e6aa476` and `a9f03ee` verified in git log; plan `<verify>` commands re-run PASS for both tasks (template greps, `sealed class ChatFlowItem` in generated output, CMake grep); `dart analyze lib/generated/chat` clean; forced re-render drift-free (byte-identical, 3/3).

---
*Phase: 01-foundation*
*Completed: 2026-08-26*
