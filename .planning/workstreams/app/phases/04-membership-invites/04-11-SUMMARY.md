---
phase: 04-membership-invites
plan: 11
subsystem: ffi-lifecycle
tags: [ffi, teardown, membership-topic, gossipsub, invite-auth, scope-authority, wr-01, wr-06, wr-07]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: "04-09 kMembershipTopic, KeyWrapSeam mintAuth/verifyAuth, crypto invite-auth adapters, InviteLink.scope_id; 04-10 scope-keyed Dart consumer"
provides:
  - "Teardown evicts and destroys g_membership alongside Messaging, before the session and catalog it borrows (WR-01)"
  - "Every gcs_init session joins kMembershipTopic listen-then-broadcast, or fails init before g_session is installed (WR-06 registration)"
  - "Production seam carries all four callables, so every production invite is bearer-gated (T-04-11-05)"
  - "Pushed InviteLink carries the command's scope_id (WR-07 push half)"
affects: [04-12, wave-6 verification]

tech-stack:
  added: []
  patterns:
    - "Session-borrowing components evict under g_mutex, destroy with the lock released, and reset before the borrowed session"
    - "Infrastructure replication topics register without entering the user-facing room projection"
    - "FFI wires seam callables to the crypto owner; it never implements crypto"

key-files:
  created: []
  modified:
    - src/ffi/gcs_core_ffi.cpp
    - test/test_gcs_ffi.cpp

key-decisions:
  - "Membership resets in the same destruction phase as Messaging (before session->Shutdown); it owns no worker thread to drain"
  - "kMembershipTopic is registered immediately after the smoke-topic loop and before g_session = std::move(session), so its failure path shuts down the LOCAL session"
  - "The membership topic is deliberately never pushed into g_roomTopics — no phantom room may appear in RoomList/SpaceTree"
  - "mintAuth/verifyAuth are assigned beside wrap/unwrap by member assignment, matching the existing seam style and InviteAuthEnabled()'s all-four requirement"
  - "BuildInviteLinkEvent takes the scope id from the command because MintInvite returns only the URL"

requirements-completed: [MEMB-01, MEMB-05]

# Metrics
duration: 6min
completed: 2026-10-18
---

# Phase 4 Plan 11: FFI Lifecycle, Membership Topic, and Scoped Invite Push Summary

**Membership now dies with the session it borrows, every node subscribes to the membership replication topic at init, production invites are bearer-gated through all four seam callables, and the pushed InviteLink names its scope**

## Performance

- **Duration:** ~6 min
- **Started:** 2026-10-18T14:15:00-07:00 (approx; reconstructed from commit times)
- **Completed:** 2026-10-18T14:21:24-07:00
- **Tasks:** 3
- **Files modified:** 2

## Accomplishments

- WR-01 closed: `gcs_shutdown` moves `g_membership` into the eviction set and resets it beside `messaging` — before `session->Shutdown()`, `entities.reset()`, and `session.reset()` — so a component that borrows both the session and the entity catalog can no longer outlive either.
- WR-06 registration closed: `gcs_init` joins `gcs::Membership::kMembershipTopic` listen-then-broadcast immediately after the smoke-topic loop and before the `g_session` move. A failed join logs the topic, shuts down the LOCAL session, and returns `nullptr` without touching the session global.
- The topic stays out of `g_roomTopics`, so no phantom room can reach the pushed `RoomList`/`SpaceTree`.
- Production seam parity with 04-09: `mintAuth` and `verifyAuth` are assigned beside `wrap`/`unwrap`, making `InviteAuthEnabled()` true so production invites are bearer-gated with no FFI-side crypto.
- WR-07 push half closed: `BuildInviteLinkEvent(url, scopeId)` stamps `InviteLink.scope_id`, and the `kInvite` arm passes the already-read command scope id.
- Full C++ suite passes 13/13.

## Task Commits

Each task was committed atomically:

1. **Task 1: Evict and destroy g_membership in gcs_shutdown** - `d6212b7` (fix)
2. **Task 2: Register membership topic and wire invite-auth seam at init** - `8616cd0` (fix)
3. **Task 3: Stamp scope id onto the pushed InviteLink** - `3ccf699` (fix)

## Files Created/Modified

- `src/ffi/gcs_core_ffi.cpp` - Teardown membership eviction/destruction, `kMembershipTopic` listen/broadcast join with fail-init path, `mintAuth`/`verifyAuth` seam assignment, two-argument `BuildInviteLinkEvent` with `set_scope_id`, scoped push call site.
- `test/test_gcs_ffi.cpp` - `InviteMintsSpaceInviteLinkWithCapabilityShape` now asserts `link->scope_id() == spaceId`.

## Decisions Made

- Membership is destroyed in the same phase as Messaging: it borrows the session and catalog but owns no worker thread, so no drain precedes its reset.
- Registration precedes installing the session global, mirroring the smoke-topic fail-init contract; the failure path operates on the local `session` unique_ptr.
- `kMembershipTopic` is infrastructure and never enters `g_roomTopics` (planner decision 2, T-04-11-04).
- The FFI only points the seam at `gcs::crypto::MintInviteAuthenticator` / `VerifyInviteAuthenticator`; no authenticator logic was added here.
- The pushed link's scope authority comes from the command, since `MintInvite` returns only the URL; Dart never parses the URL for scope.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- **ASan/LeakSanitizer run not performed:** there is no sanitizer build tree in `build/`; the canonical configuration is Debug without sanitizers. The coldboot init → shutdown → init cycle passed in the canonical build. The leak/lifetime claim rests on the eviction/destruction ordering and that cycle.
- **clang-format environment mismatch (known):** Homebrew `clang-format 22.1.7` reports violations on untouched baseline lines (for example `src/ffi/gcs_core_ffi.cpp:53`, the `const char*` declarations), the same mismatch documented by 04-08/04-09. It is not a usable gate in this environment; changed regions follow the repository's existing hand style.
- **clang-tidy:** no `.clang-tidy` config applies to `src/` (the only one is inside the untouched `GNUS-NEO-SWARM` submodule), so it was not run as a gate.
- Full `ctest` passed 13/13 after the changes.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- 04-12 can prove cross-node space-invite convergence: both nodes now auto-subscribe to `kMembershipTopic` with no per-scope registration.
- The Dart consumer from 04-10 receives `InviteLink.scope_id` exactly as the FFI stamps it.
- Teardown no longer leaks a session-bound Membership across init/shutdown cycles.

---

*Phase: 04-membership-invites*
*Completed: 2026-10-18*

## Self-Check: PASSED

- Both modified files and this SUMMARY exist on disk.
- Commits `d6212b7`, `8616cd0`, and `3ccf699` exist in the repository history.
- Full C++ suite: 13/13 tests passed.
