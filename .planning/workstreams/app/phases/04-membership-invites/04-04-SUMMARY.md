---
phase: 04-membership-invites
plan: 04
subsystem: entities/catalog
tags: [protobuf, crdt, entity-store, googletest, gcs-entity-store]

# Dependency graph
requires:
  - phase: 04-01
    provides: SpaceRecord.creator/members_can_invite + RoomRecord.creator/members_can_invite proto fields
provides:
  - Wallet-address-injected EntityStore constructor (defaulted, legacy call sites unchanged)
  - CreateSpace/CreateRoom stamp `creator` (spaces + standalone rooms) and `members_can_invite`
  - UpdateSpace preserves the stored `creator` across a full-state rewrite
  - 4 new test_gcs_entities cases covering stamping, P2 room scoping, preservation, and the legacy empty-address path
affects: [04-05 (FFI wiring passes the real address + command field), 04-03 (CanInvite gate consumes the persisted members_can_invite), membership-guard work]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Authority fields are C++-stamped from injected identity, never client-supplied; immutable fields (creator/created_at_ms/tombstone) are preserved on full-state rewrites"
    - "Defaulted constructor/parameter extension keeps every legacy call site compiling without a call-site edit (FFI wiring lands in a later plan)"

key-files:
  created: []
  modified:
    - src/lib/gcs_entity_store.hpp
    - src/lib/gcs_entity_store.cpp
    - test/test_gcs_entities.cpp

key-decisions:
  - "Default membersCanInvite = false matches the MEMB-05 'if enabled' wording (research A3) — the invite gate is opt-in"
  - "Nested rooms leave creator empty (their Super Admin is the parent space's creator, P2); only standalone rooms carry creator"
  - "UpdateSpace treats creator as an immutable authority field like created_at_ms — a client-supplied desired creator is discarded, members_can_invite is applied as desired state"

patterns-established:
  - "Wallet identity is injected into EntityStore as a plain std::string (keeps EntityStore SDK-free; the FFI owns GeniusSDKGetAddress)"

requirements-completed: [MEMB-03, MEMB-05]

# Metrics
duration: 7min
completed: 2026-10-02
---

# Phase 4 Plan 4: Creator + members_can_invite stamping Summary

**EntityStore now C++-stamps a wallet-injected `creator` on new spaces and standalone rooms plus `members_can_invite`, and UpdateSpace preserves the stored creator across full-state rewrites — proven by 11 green entity tests.**

## Performance

- **Duration:** 7 min
- **Started:** 2026-10-02T20:04:22Z (wave base bf0b73c)
- **Completed:** 2026-10-02T20:11:03Z
- **Tasks:** 2
- **Files modified:** 3

## Accomplishments
- `EntityStore(CoreSession&, std::string localAddress = {})` — wallet address injected at construction, defaulting to empty so every legacy call site compiles unchanged
- `CreateSpace` stamps `creator` (from the injected address) + `members_can_invite`; `CreateRoom` stamps `members_can_invite` on all rooms and `creator` only on standalone rooms (empty parent, P2)
- `UpdateSpace` preserves the stored `creator` alongside `created_at_ms`/tombstone state, so a full-state rewrite can never blank or overwrite the Super Admin from a client command (T-04-04-T); `members_can_invite` flows through as desired state
- 4 new `test_gcs_entities` cases: create-stamping + manifest-reload persistence proof, standalone-vs-nested room creator, UpdateSpace preservation with a hostile client-supplied creator, and the legacy empty-address path

## Task Commits

Each task was committed atomically:

1. **Task 1: Inject the local wallet address and stamp creator + members_can_invite on create** - `9609b51` (feat)
2. **Task 2: Preserve creator in UpdateSpace and extend test_gcs_entities.cpp** - `3fc7240` (feat)

**Plan metadata:** committed by orchestrator (this executor does not write STATE.md/ROADMAP.md in worktree mode)

## Files Created/Modified
- `src/lib/gcs_entity_store.hpp` - constructor takes `std::string localAddress = {}`; `CreateSpace`/`CreateRoom` gain defaulted `bool membersCanInvite = false`; new private `std::string m_localAddress`
- `src/lib/gcs_entity_store.cpp` - `m_localAddress(std::move(localAddress))`; creator + members_can_invite stamping in CreateSpace/CreateRoom; `updated.set_creator(existingIt->second.creator())` in UpdateSpace; added `<utility>` for `std::move`
- `test/test_gcs_entities.cpp` - `kTestCreatorAddress` constexpr + 4 new TEST_F cases

## Decisions Made
- **membersCanInvite defaults to false** — matches the MEMB-05 "if enabled" wording (research A3); the gate is opt-in, not opt-out.
- **creator is desired-authority, not desired-state** — UpdateSpace discards any client-supplied creator and re-stamps the stored one; `members_can_invite` is applied as-is.
- **Nested rooms carry no creator** — P2: their Super Admin is the parent space's creator; only standalone rooms (empty parent) are their own Super Admin scope.

## Deviations from Plan

### Plan-gate adjustments (documented, not auto-fixes)

**1. [CLAUDE.md precedence] Acceptance grep `set_creator(existingIt` does not match literally**
- **Found during:** Task 2 verification
- **Issue:** The plan's acceptance criterion requires `grep -n "set_creator(existingIt" src/lib/gcs_entity_store.cpp` to match. The repo's uniform convention spaces parentheses in calls (`set_x( arg )`): 35 spaced setter calls vs 0 unspaced across `src/lib/*.cpp`, plus every adjacent call in UpdateSpace (`set_created_at_ms( existingIt->second.created_at_ms() )`).
- **Fix:** Kept the codebase-consistent form `updated.set_creator( existingIt->second.creator() );`. Project CLAUDE.md forbids inventing a competing local C++ style, and it takes precedence over plan instructions.
- **Verification:** The substantive criterion — creator preserved from the stored record inside UpdateSpace — is met and proven by `GcsEntitiesTest.UpdateSpacePreservesCreatorAndAppliesMembersCanInvite` (green), including the hostile client-supplied-creator case.
- **Committed in:** `3fc7240`

**2. [Rule 3 - Blocking, environment] clang-format not applied (v22 conflicts with checked-in style)**
- **Found during:** Task 1
- **Issue:** Only clang-format 22.1.7 is available; running it against the repo's `.clang-format` reformats the whole file (2-space → 4-space indent, `namespace gcs {` → Allman, `( x )` spacing). The pristine (unmodified) `gcs_entity_store.cpp` already fails `clang-format --dry-run --Werror`, proving the checked-in files were formatted with a different version. Applying v22 would produce a ~300-line unrelated reformat, violating the minimal-change rule and the established file style.
- **Fix:** Reverted the reformat and matched the surrounding file style in all added lines. No whole-file reformat.
- **Verification:** `git diff --stat` shows 42 insertions / 14 deletions across the two source changes — no unrelated churn.
- **Committed in:** `9609b51` / `3fc7240`

---

**Total deviations:** 0 auto-fixed (Rules 1–4 not triggered); 2 documented plan-gate/environment adjustments
**Impact on plan:** None on behavior. Both adjustments preserve the plan's intent while honoring the project's minimal-change and coding-standard constraints.

## Issues Encountered

- **clang-tidy could not run.** `build/OSX/Debug/compile_commands.json` carries dual-arch (x86_64 + arm64) compiler jobs per TU, so clang-tidy rejects the file with "expected exactly one compiler job". Additionally, no project-level `.clang-tidy` or CMake tidy wiring exists for this repo (only inside the GNUS-NEO-SWARM submodule). Not run — environment/config limitation, unrelated to this change.
- **Compiler gate passed instead:** the build compiles `gcs_core` and `gcs_ffi` with `-Wall -Wextra -Werror=sign-compare -Werror=type-limits -Werror=unused-lambda-capture` and no warnings/errors.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Plan 04-05 (FFI wiring) can now construct `EntityStore(*g_session, GeniusSDKGetAddress().address)` and pass the command's `members_can_invite` through — the defaulted signatures keep the tree buildable until then.
- Plan 04-03's CanInvite gate can read the persisted `members_can_invite` from SpaceRecord/RoomRecord; the Super Admin guard / approval check can compare against the persisted `creator`.
- Retrofit boundary (documented, research P1): spaces/rooms created before Phase 4 carry an empty `creator`; the membership guard must treat an empty creator as "no Super Admin".

### Stub tracking

No stubs introduced. `creator` and `members_can_invite` are real, persisted, and asserted through a manifest-reload round-trip.

### Threat surface scan

No new surface beyond the plan's threat model. The change is exactly the mitigation for T-04-04-T (creator is C++-stamped from injected identity and preserved on rewrite) and T-04-04-EoP (members_can_invite defaults false).

---
*Phase: 04-membership-invites*
*Completed: 2026-10-02*

## Self-Check: PASSED

- FOUND: src/lib/gcs_entity_store.hpp
- FOUND: src/lib/gcs_entity_store.cpp
- FOUND: test/test_gcs_entities.cpp
- FOUND: .planning/workstreams/app/phases/04-membership-invites/04-04-SUMMARY.md
- FOUND: 9609b51 (Task 1)
- FOUND: 3fc7240 (Task 2)
