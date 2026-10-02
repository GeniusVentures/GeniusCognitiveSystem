---
phase: 04-membership-invites
plan: 01
subsystem: protocol
tags: [protobuf, proto3, wire-contract, membership, roles, invites, append-only]

# Dependency graph
requires:
  - phase: 03-messaging
    provides: GcsCommand/GcsEvent oneof plane, append-only proto discipline (D-24/D-26), GcsEvent push precedent
  - phase: 02-spaces-rooms
    provides: SpaceRecord/RoomRecord per-key LWW records, `gcs_proto` build target, CreateSpace/CreateRoom/UpdateSpace commands
provides:
  - Five-tier `Role` enum and `PendingAction` dormancy enum
  - `MemberRecord` per-member LWW record + `WrappedRoomKey` (bytes envelope) carrier
  - `MemberList` flat pushed roster with `self_address`
  - `InviteLink` pushed event and `InviteRecord` write-at-redeem invite grant
  - `SpaceRecord`/`RoomRecord` creator + members_can_invite + approved_by + pending_action fields
  - Seven membership command messages and the GcsCommand/GcsEvent oneof arms
affects: [04-02, 04-03, 04-04, 04-05, 04-06, 04-07]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Append-only proto evolution: new Phase 4 section, no renumber/retype/remove"
    - "Binary payload fields typed proto3 `bytes` (never `string`) to avoid UTF-8 parse failure"

key-files:
  created: []
  modified:
    - src/proto/gcs_chat.proto

key-decisions:
  - "Phase 4 additions appended as a new `// === Phase 4 ... ===` section after the Phase 3 section, matching the existing section-comment convention (append-only)"
  - "WrappedRoomKey.envelope typed bytes (binary nonce||ct||tag), not string"
  - "Membership changes flow through the existing GcsCommand/GcsEvent oneofs only — no new FFI ABI surface"

patterns-established:
  - "Phase N proto section: `// === Phase N: <name> (append-only additions — <decisions>) ===` header precedes each new group"
  - "Each new proto field carries a trailing comment naming its decision/requirement id"

requirements-completed: [MEMB-01, MEMB-02, MEMB-03, MEMB-04, MEMB-05]

# Metrics
duration: 6min
completed: 2026-10-02
---

# Phase 4 Plan 01: Membership Wire Contract Summary

**Append-only Phase 4 membership surface added to `gcs_chat.proto` — five-tier Role, per-member MemberRecord, flat MemberList, InviteRecord/InviteLink, creator/members_can_invite/dormancy fields, and seven command + two event oneof arms.**

## Performance

- **Duration:** ~6 min
- **Started:** 2026-10-02T19:52:00Z
- **Completed:** 2026-10-02T19:57:25Z
- **Tasks:** 2
- **Files modified:** 1

## Accomplishments

- `enum Role` (ROLE_SUPER_ADMIN..ROLE_GUEST = 1..5) and `enum PendingAction` (D-07 dormancy marker) added.
- `WrappedRoomKey` (`bytes envelope`), `MemberRecord` (D-01), `MemberList` with `self_address` (D-04), `InviteLink` (D-05), and `InviteRecord` (write-at-redeem invite grant) added.
- `SpaceRecord` gained `creator=9`, `members_can_invite=10`, `approved_by=11`, `pending_action=12`; `RoomRecord` gained `creator=8`, `members_can_invite=9`, `approved_by=10`, `pending_action=11`.
- Seven data-only command messages (invite/redeem/change_role/remove_member/leave/delete_scope/approve) added; `GcsCommand` oneof extended with arms 6–12 and `GcsEvent` with `member_list=7`, `invite_link=8`.
- `CreateSpaceCommand`/`CreateRoomCommand`/`UpdateSpaceCommand` extended with `members_can_invite` (P3/MEMB-05).

## Task Commits

Each task was committed atomically:

1. **Task 1: Add Role/PendingAction enums, MemberRecord, MemberList, InviteLink, InviteRecord, and the creator/members_can_invite/approved_by/pending_action record fields** - `378f960` (feat)
2. **Task 2: Add the seven command messages and wire the GcsCommand/GcsEvent oneof arms** - `78f8db6` (feat)

**Plan metadata:** committed separately (docs: complete 04-01 plan summary)

## Files Created/Modified

- `src/proto/gcs_chat.proto` - Append-only Phase 4 membership wire contract (126 insertions, 0 deletions vs. wave base `2474930`)

## Decisions Made

- **Placement of the Phase 4 section:** appended after the existing Phase 3 section rather than immediately after `MessageState`, preserving the file's established `// === Phase N ... ===` append-at-end convention (Task 1's action line named `MessageState` as the preceding enum, but a mid-file insertion would have broken the append-only section layout the file already uses for Phases 2 and 3). Generated code is identical regardless of declaration order.
- **All `<...>` tokens in comments wrapped in backticks** (e.g. `` `gcs/members/<scopeId>/<wallet>` ``) per the Phase 02 P03 accumulated-context rule — raw angle brackets flow into generated Dart doc comments and trip `unintended_html_in_doc_comment`.

Both decisions are consistent with the plan's own append-only and section-comment requirements; no plan behaviour was altered.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

1. **Worktree base drift (corrected before any work):** The worktree branch `worktree-agent-a0fc7f6749d6c6fcf` was created from `main` (docs-only lineage at `6ddb7c7`) instead of the wave base `2474930` (`develop` / `gsd/phase-04-membership-invites`). The mandated HEAD assertion passed (agent-branch namespace), so the prescribed `git reset --hard 2474930` was applied to a clean working tree — nothing was lost.
2. **Build type for the verification target:** The prebuilt thirdparty tree at `GeniusNetwork/thirdparty/build/OSX/` ships `vk-bootstrap` only under `Debug/`, so a `Release` configure fails at `find_package(vk-bootstrap)`. Configured with `-DCMAKE_BUILD_TYPE=Debug`; `cmake --build build --target gcs_proto` then succeeded.
3. **Submodules:** `build`, `GNUS-NEO-SWARM`, and `src/app/scaffold` were initialized in the worktree (local git dirs already existed) to configure and build the named target.

No problems during planned work required code changes.

## Verification

- **Plan command (both tasks):** `cmake --build build --target gcs_proto` — exits 0; `[100%] Built target gcs_proto`, producing `libgcs_proto.a`. Configure was run as `cmake -S build/OSX -B build -DCMAKE_BUILD_TYPE=Debug`.
- **Independent protoc check:** the in-tree `add_proto_library` invocation is `protoc -I<repo>/src -I<protobuf include> --cpp_out=<build>/generated <proto>`; run standalone with the vendored `protoc` 34.0, it exited 0 and emitted `gcs_chat.pb.{h,cc}` exposing every new arm (`kInvite=6` … `kApprove=12`, `kMemberList=7`, `kInviteLink=8`).
- **Acceptance criteria greps (all matched):** `enum Role` (single definition, 5 tiers), `message MemberRecord` fields 1–11, `bytes envelope = 2;` (not `string`), `string self_address = 3;`, `message InviteRecord` fields 1–3, `string creator = 9;` (SpaceRecord) / `= 8;` (RoomRecord), `members_can_invite = 10;`/`= 9;`, all seven command messages, `invite = 6;`–`approve = 12;`, `member_list = 7;`/`invite_link = 8;`, and `members_can_invite = 4/3/5` on the three create/update commands.
- **Append-only:** `git diff --numstat 2474930 -- src/proto/gcs_chat.proto` → `126 0` (zero deletions; no existing field renumbered or retyped).

## Known Stubs

None — this plan adds a wire schema only; no runtime code paths, placeholders, or unwired data sources were introduced.

## Threat Flags

None beyond the plan's `<threat_model>`. This plan is contract-shaping only (schema), enforced by the `gcs_proto` compile gate; the runtime guards for T-04-01-EoP / T-04-02-T / T-04-03-T land in plans 04-02/04-03/04-05.

## Next Phase Readiness

- `gcs_proto` compiles cleanly and exposes every type, field, and oneof arm downstream plans (04-02 … 04-07) reference.
- The Dart regeneration in plan 04-06 (`gcs_chat.pb.dart`) will consume this contract unchanged; `WrappedRoomKey.envelope` surfaces as `Uint8List`.
- Scope note: this plan delivers the **wire contract** for MEMB-01..MEMB-05 only; the C++ mint/redeem/guard enforcement and Dart consumption land in 04-02..04-07.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-02*

## Self-Check: PASSED

- FOUND: `src/proto/gcs_chat.proto`
- FOUND: `.planning/workstreams/app/phases/04-membership-invites/04-01-SUMMARY.md`
- FOUND: `378f960` (Task 1)
- FOUND: `78f8db6` (Task 2)
- FOUND: `525b750` (plan summary commit)
- Working tree clean (no untracked or modified files)
