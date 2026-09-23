---
phase: 03-messaging
plan: 03
subsystem: ui
tags: [dart, flutter, protobuf, cubit, messaging, crdt]

# Dependency graph
requires:
  - phase: 03-01
    provides: extended gcs_chat.proto (ChatMessageState.sender/deleted/deleted_at_ms + MessageHistory + GcsEvent.messageHistory)
  - phase: 03-02
    provides: storage widening (topics-aware Put, PutLocal, QueryKeyValues, RegisterNewElementCallback, raw Publish/Subscribe)
provides:
  - regenerated Dart protobuf bindings (gcs_chat.pb/.pbjson) carrying sender + tombstone fields and the MessageHistory batch
  - MessageFlowCubit.upsert (by-id pending -> complete replace) + replaceAll (full-list history replay)
  - SessionCubit messageHistory dispatch arm (D-06 replaceAll semantics)
  - ComposerCubit.send() -> bool (false only on transport refusal) + "Send failed" toast (D-07)
affects: [03-04, 03-05, 03-06]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "by-id upsert for pushed pending -> complete/error echo (D-07): pending replaces in place instead of duplicating"
    - "replaceAll full-list snapshot for pushed MessageHistory batch (D-06), capped by the generated cappedItems invariant (T-03-06)"
    - "sealed ChatFlowItem pattern-match drives the shell-owned error toast (once per error instanceId)"

key-files:
  created: []
  modified:
    - src/app/lib/generated/proto/gcs_chat.pb.dart
    - src/app/lib/generated/proto/gcs_chat.pbjson.dart
    - src/app/lib/cubits/message_flow_cubit.dart
    - src/app/lib/cubits/session_cubit.dart
    - src/app/lib/cubits/composer_cubit.dart
    - src/app/lib/shell/gcs_shell.dart
    - src/app/test/cubits/shell_cubits_test.dart

key-decisions:
  - "Sender short-form truncation constants kSenderShortPrefixLength/kSenderShortSuffixLength = 8 (D-04): '0x' + first 8 + '…' + last 8 hex chars; short addresses pass through verbatim"
  - "ComposerCubit.send() returns bool: true on publish/no-op, false only when a sendable draft's publishCommand is refused (D-07 transport failure)"
  - "Send failed toast dedupes by instanceId in a shell-owned Set<String>, fired on both refused publish and pushed ERROR-state messages (toast once per message id)"

patterns-established:
  - "Dart renders pushed snapshots only (D-04); the C++ side stamps user_self/user_peer on both live and history paths, so message.role is rendered directly via kRoleVariants"

requirements-completed: [CORE-04]

# Metrics
duration: 10min
completed: 2026-09-23
---

# Phase 3 Plan 3: Dart proto regen + message-flow upsert/replaceAll + send-failure toast Summary

**Regenerated Dart protobuf bindings (sender + tombstone + MessageHistory) with by-id upsert, full-list history replaceAll, and a "Send failed" transport-error toast**

## Performance

- **Duration:** 10 min
- **Started:** 2026-09-23T22:05:26Z
- **Completed:** 2026-09-23T22:11:50Z
- **Tasks:** 2
- **Files modified:** 7

## Accomplishments

- Regenerated `gcs_chat.pb.dart` / `gcs_chat.pbjson.dart` from the extended Phase 3 proto with the pinned `protoc_plugin 22.5.0`, exposing `ChatMessageState.sender` (tag 7), tombstone fields (8/9), and the `MessageHistory` batch + `GcsEvent.messageHistory` (tag 6).
- Added `MessageFlowCubit.upsert` (by-instanceId replace, D-07) and `replaceAll` (full-list history replay, D-06), both capped by the generated `cappedItems` invariant.
- Wired `SessionCubit._dispatchEvent` so a pushed `messageHistory` event replaces the flow (D-06) and a pushed `message` event upserts (D-07), with sender short-form truncation on peer bubbles (D-04).
- Converted `ComposerCubit.send()` to `bool` and added the D-07 "Send failed" toast on both refused publishes and pushed ERROR-state messages.

## Task Commits

Each task was committed atomically:

1. **Task 1: Regenerate Dart proto + upsert/replaceAll + messageHistory dispatch + transport-error toast** - `7c6de41` (feat)
2. **Task 2: Extend shell_cubits_test.dart for upsert + messageHistory + sender + composer send** - `bda3c68` (test)

**Plan metadata:** committed after this summary (`docs(03-03): complete ...`).

## Files Created/Modified

- `src/app/lib/generated/proto/gcs_chat.pb.dart` - regenerated bindings: sender/tombstone fields + MessageHistory + hasMessageHistory oneof accessor
- `src/app/lib/generated/proto/gcs_chat.pbjson.dart` - regenerated JSON bindings
- `src/app/lib/cubits/message_flow_cubit.dart` - upsert + replaceAll + `_truncateSender` + sender label
- `src/app/lib/cubits/session_cubit.dart` - hasMessage arm upserts; new hasMessageHistory arm replaceAll
- `src/app/lib/cubits/composer_cubit.dart` - `send()` returns bool (false on transport refusal)
- `src/app/lib/shell/gcs_shell.dart` - "Send failed" toast constants + toast on refused publish + ERROR-state BlocListener
- `src/app/test/cubits/shell_cubits_test.dart` - 7 new cubit tests (upsert, replaceAll, sender, messageHistory, composer send)

## Decisions Made

- Sender short-form uses named constants `kSenderShortPrefixLength`/`kSenderShortSuffixLength` (both 8), matching the plan's `'0x' + substring(2, 10) + '…' + last 8` shape.
- The `messageHistory` dispatch arm uses an untyped list literal (inferred `List<ChatFlowItemTextBubble>`), which is covariant-assignable to `replaceAll(List<ChatFlowItem>)` without importing the generated `chat_message_flow.dart` into `session_cubit.dart`.

## Deviations from Plan

None - plan executed as written (two deviations in tooling/verification are noted under Issues Encountered).

## Issues Encountered

- **`protoc` is not on PATH** — the README regen command references bare `protoc`, which is not installed (and no homebrew copy exists). Used the pinned thirdparty host tool `../thirdparty/build/OSX/Debug/protobuf_host/bin/protoc` with the `protoc-gen-dart` shim at `~/.pub-cache/bin/protoc-gen-dart` (already pinned to 22.5.0), per the project constraint to never use system/homebrew copies.
- **`gcs_chat.pbenum.dart` did not change** — the `MessageRole`/`MessageState` enums were already present from the 02-03 regen, so only `gcs_chat.pb.dart` and `gcs_chat.pbjson.dart` regenerated (the frontmatter listed all three as prospective).
- **`dart analyze --fatal-infos` from `src/app` reports 50 pre-existing errors in the scaffold submodule** (`scaffold/example/test/capture_images_test.dart` references ungenerated demo files). These are pre-existing and out of scope (the scaffold is a git submodule I must not touch). The app package itself is clean: `dart analyze --fatal-infos lib test` reports "No issues found!".
- **`grep -c "sleep"` is 1, not 0** — the only match is the pre-existing file-header doc comment "never sleeps"; no actual `sleep`/`sleep_for` calls exist, and the new tests use `pumpEventQueue`.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Dart renders pushed messages with by-id upsert, full-list history replace, and truncated peer sender labels; cubit tests green and app analyze clean.
- Ready for 03-04 (gcs::Messaging component: live publish + archive send, two-route dedupe, history scan), which produces the ERROR-state push and MessageHistory batch this plan already renders.

---
*Phase: 03-messaging*
*Completed: 2026-09-23*

## Self-Check: PASSED

