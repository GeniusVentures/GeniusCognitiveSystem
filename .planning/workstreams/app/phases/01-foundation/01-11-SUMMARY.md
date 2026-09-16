---
phase: 01-foundation
plan: 11
subsystem: ui
tags: [flutter, dart, ffi, bloc-cubit, frontend-scaffold, protobuf, theming, chat-shell]

# Dependency graph
requires:
  - phase: 01-foundation/01-05
    provides: gcs_ffi bindings (GcsBindings gcs_init/gcs_publish/gcs_subscribe/gcs_shutdown), Dart_InitializeApiDL contract, GCS_FFI_LIBRARY ctest injection
  - phase: 01-foundation/01-10
    provides: ChatMessageFlow + sealed ChatFlowItem envelope, ChatMessageFlowState.kMaxFlowItems cap (T-01-10-02)
  - phase: 01-foundation/01-08/01-09
    provides: bubble role/state variant composites + code/media composites, ChatMessageBubbleState.kValidRoles/kValidStates
provides:
  - GCSChat three-region shell (RoomRail + ChatMessageFlow + composer bar) with per-screen cubit injection (D-11/D-23)
  - SessionCubit — owns the FFI handle lifecycle (gcs_init config bytes, ReceivePort push listener, gcs_subscribe, idempotent close with port-before-shutdown ordering) and the pushed GcsEvent decode/dispatch pipeline (D-04/D-05/D-26)
  - RailCubit / MessageFlowCubit / ComposerCubit — thin D-04 state holders (pushed room list truth D-21, capped flow T-01-11-03, send_text publish D-27)
  - GcsCommandTransport interface — typed publish seam implemented by SessionCubit, fake-driven in tests
  - GcsTheme light/dark Material 3 ThemeData registering ScaffoldPalette/ScaffoldDimens extensions (D-12/D-22) + GCSChatApp root
  - theme_registration / chat_shell / generated composites smoke tests (RESEARCH success-criteria rows 864-866)
affects: [02+ phases (the shell is the app surface every later screen/feature mounts under)]

# Tech tracking
tech-stack:
  added: []  # no new deps — flutter_bloc 9.1.1, frontend_scaffold atoms, existing generated code only
  patterns:
    - "Inert-by-default FFI session: SessionCubit.openDefault stays bindings-free under FLUTTER_TEST or missing GCS_FFI_LIBRARY, so widget tests never touch the native library while ctest injects it only for the dedicated smoke entry"
    - "Cubit-to-cubit projection via stream subscription (ComposerCubit listens to RailCubit) with cancel-on-close ownership"
    - "Zone-local cubit construction in widget tests: cubits must be created inside the testWidgets body — cubits from setUp schedule async broadcast delivery outside the test's FakeAsync zone and never flush under tester.pump"
    - "_pumpUntil bounded 16ms frame-pump wait-condition helper (Flutter analog of the C++ condition-variable template; never sleeps)"

key-files:
  created:
    - src/app/lib/shell/gcs_shell.dart
    - src/app/lib/shell/room_rail.dart
    - src/app/lib/cubits/session_cubit.dart
    - src/app/lib/cubits/rail_cubit.dart
    - src/app/lib/cubits/message_flow_cubit.dart
    - src/app/lib/cubits/composer_cubit.dart
    - src/app/lib/theme/gcs_theme.dart
    - src/app/test/cubits/shell_cubits_test.dart
    - src/app/test/chat_shell_test.dart
    - src/app/test/theme_registration_test.dart
    - src/app/test/generated/chat_composites_test.dart
  modified:
    - src/app/lib/main.dart
    - src/app/test/widget_test.dart

key-decisions:
  - "SessionCubit.openDefault gates on FLUTTER_TEST=true or missing/absent GCS_FFI_LIBRARY -> inert cubit (composer disabled, no FFI); Dart_InitializeApiDL resolved from the opened library before gcs_subscribe (01-05 contract)"
  - "Flow cap reuses the generated ChatMessageFlowState.kMaxFlowItems (500) as the single source of truth rather than a duplicate shell-side constant (T-01-11-03/T-01-10-02)"
  - "publishCommand exposed through the GcsCommandTransport interface (implemented by SessionCubit) so ComposerCubit publishes without knowing about FFI, and tests record envelopes at the seam (D-27)"
  - "_closeNativeOnce: ReceivePort closed BEFORE gcs_shutdown, exactly once, null-handle no-op (T-01-11-04)"
  - "GCSChat owns only the cubits it created; injected cubits outlive the shell (dispose order composer -> session -> flow -> rail)"
  - "Widget tests construct cubits inside the test body (zone-local stream delivery) and close them via addTearDown registered in-body"

requirements-completed: [CORE-05]

# Metrics
duration: ~45 min
completed: 2026-09-16
---

# Phase 01 Plan 11: GCS Chat Shell + Cubits + Theming Summary

**GCSChat app end to end: three-region shell + four cubits wiring the FFI push plane (gcs_init/subscribe/publish/shutdown, protobuf GcsEvent decode and dispatch, 500-item flow cap, send_text publish) under Material 3 scaffold theming, with theme/shell/composites smoke tests**

## Performance

- **Duration:** ~45 min (2026-09-16T16:05Z-07:00 → 2026-09-16T16:47Z-07:00)
- **Tasks:** 4 of 4
- **Files:** 13 (11 created, 2 rewritten)

## Task Log

| # | Task | Commit | Notes |
|---|------|--------|-------|
| 1 | Shell layout (GCSChat + RoomRail) | 2971ecf | Row(rail, divider, center column) + _ComposerBar; atom-based rail rows with scaffold empty state |
| 2 | Four cubits (TDD RED) | d55ac8b | 16 failing tests — rail replace/stale-selection, flow cap + taxonomy map, composer send, session lifecycle/dispatch/malformed bytes |
| 2 | Four cubits (GREEN) | a76d4d5 | 16/16 green; recording GcsBindings fake via super.fromLookup; analyze gate reached -> unused shell import dropped |
| 3 | main.dart + theme rewrite | 9890e8c | GcsTheme light/dark (palette+dimens extensions, useMaterial3); GCSChatApp root; legacy GeniusSwarmApp/fromSeed/chat-kit gone; widget_test rewritten |
| 4 | Three smoke tests | 1119683 | theme_registration + chat_shell + generated/chat_composites (fixture-driven 4x5 role/state matrix + interleaved flow) |

## Deviations from Plan

### Verification-command scoping (pre-existing repo noise)

- **Found during:** Plan-level verification
- **Issue:** The plan's literal `cd src/app && flutter analyze --fatal-infos` (no path filter) sweeps the vendored scaffold's own example/warning surface (~50 pre-existing issues, WR-02 already documented in `src/app/CMakeLists.txt` next to the `app_analyze` gate).
- **Fix:** Used the repo's established gate `flutter analyze --fatal-infos lib test` — **No issues found**, matching what CMake enforces. Nothing in lib/ or test/ fails analysis.

### API deviations from the plan's `<interfaces>` contract

1. **flutter_bloc 9.1.1 does not re-export `SingleChildWidget`** (from package:nested) — the `MultiBlocProvider(providers: [...])` list is left to type inference instead of an explicit `<SingleChildWidget>` annotation.
2. **`scaffold_theme.dart` does not re-export `ScaffoldPalette`/`ScaffoldDimens`** — shell/theme files import `frontend_scaffold/theme/scaffold_palette.dart` and `scaffold_dimens.dart` directly for explicit type annotations.

### Atom limitation: live draft mirroring

- **Issue:** `ScaffoldComposer` (D-07 atom contract) owns its internal `TextEditingController` and exposes no `onChanged`/controller seam, so keystrokes cannot mirror into `ComposerCubit.updateDraft` live.
- **Mitigation:** The shell wires `onSubmit -> updateDraft(value) + send()` (keyboard submit) and the actionRow send button publishes the cubit-held draft; the send affordance, room projection, and readiness gating are all fully functional and test-covered. Live-draft mirroring lands when the atom grows a controller/onChanged seam.

### Test-infrastructure adaptations

1. **TDD RED gate** was necessarily a compile failure (cubit classes absent) followed by 16/16 green after implementation — the plan's Task 2 order (tests first) was preserved.
2. `throwsNothing` is not exported by this flutter_test version — replaced with a plain `await cubit.close()` in the inert-mode test.
3. ffigen's snake_case symbol names (`gcs_init`, ...) overridden in the recording fake trip `non_constant_identifier_names` — suppressed with `// ignore_for_file: non_constant_identifier_names`, the same directive the generated bindings carry.
4. **Zone-locality discovery (Rule 1 bug fix):** widget tests that construct cubits in `setUp` hang their async broadcast stream delivery outside the test's FakeAsync zone — emits reach listeners only on the real event loop, so `tester.pump` never renders them (order-dependent flakes). All cubits are now constructed inside each `testWidgets` body; `_pumpUntil` (bounded 16ms frame pumps) is the wait-condition helper.

## Threat Register Coverage

| Threat | Disposition in code |
|--------|--------------------|
| T-01-11-01 malformed pushed bytes | `GcsEvent.fromBuffer` wrapped in try/catch -> raw error string on the session state, never a crash (tested with `[0xFF,0xFF,0xFF]`) |
| T-01-11-03 unbounded flow list | `MessageFlowCubit` caps at generated `kMaxFlowItems` (500) dropping the OLDEST (tested: first retained id 'm2' after cap+2 appends) |
| T-01-11-04 FFI handle misuse | handle private to `SessionCubit`; `_closeNativeOnce` idempotent (tested shutdownCalls == 1 on double close); port closed before `gcs_shutdown` |

## Verification Results

- `flutter analyze --fatal-infos lib test` → `No issues found! (ran in 1.3s/1.9s)` (all runs)
- Full `flutter test` → `00:01 +30 ~1: All tests passed!` (30 passed, 1 skip = the 01-05 native-port smoke entry skipping without GCS_FFI_LIBRARY, by design)
- Task verifies: all four per-task automated verify chains passed before their commits
- Stability: shell + composites trio re-run 3-4x consecutively, all green (zone fix + fromSeed-comment fix)

## Self-Check: PASSED

- Files: all 13 key-files exist on disk (checked via git show of commits 2971ecf/a76d4d5/9890e8c/1119683).
- Commits: 2971ecf, d55ac8b, a76d4d5, 9890e8c, 1119683 present on feature/app-ffi-data-plane.
