---
phase: 04-membership-invites
plan: 09
subsystem: membership-security
tags: [crdt, membership, invites, hkdf, aes-256-gcm, protobuf, gossipsub, security]

# Dependency graph
requires:
  - phase: 04-membership-invites
    provides: "04-03 invite mint/redeem + D-06 key-wrap seam and 04-05 role/approval guards"
  - phase: 03-messaging
    provides: "HKDF-SHA256 + AES-256-GCM substrate (gcs::crypto) and per-room key store"
provides:
  - "Rank-strict removal guard: an author must strictly outrank the target (WR-02)"
  - "Rank-based dormancy predicate for admin demotions (WR-03)"
  - "InviteRecord.is_space and InviteRecord.authenticator wire fields (append-only, field 4/5)"
  - "InviteLink.scope_id wire field for wave-6 push consumers (WR-07)"
  - "Bearer-key-derived invite authenticator mint/verify in gcs::crypto (WR-04)"
  - "Every-invite bearer gate in RedeemInvite, including childless space invites"
  - "Single well-known membership replication topic: Membership::kMembershipTopic (WR-06)"
  - "Tombstone arrivals push the tombstone-filtered roster (WR-05)"
  - "Multinode fixture subscribed to the membership topic with no per-scope registration"
affects: [04-10, 04-11, 04-12, wave-6 push consumers]

tech-stack:
  added: []
  patterns:
    - "Bearer proof: HKDF-SHA256 auth key -> AES-256-GCM seal of a SHA-256 (scope, inviteId) marker"
    - "Proto3 string fields carry binary envelopes only through hex encoding"
    - "One registered replication topic per domain; the scope id stays in the CRDT key"
    - "Rank comparisons via PrivilegeRank as the single ordering authority"

key-files:
  created: []
  modified:
    - src/proto/gcs_chat.proto
    - src/lib/gcs_crypto.hpp
    - src/lib/gcs_crypto.cpp
    - src/lib/gcs_membership.hpp
    - src/lib/gcs_membership.cpp
    - test/test_gcs_membership.cpp
    - test/test_gcs_membership_multinode.cpp

key-decisions:
  - "The authenticator is an AES-256-GCM envelope of SHA-256(prefix||scopeId||inviteId), keyed by HKDF(inviteKey); binding both ids prevents authenticator transplant between invites"
  - "The envelope is hex-encoded because InviteRecord.authenticator is a proto3 string (UTF-8 required); WrappedRoomKey.envelope is bytes and stays binary"
  - "RedeemInvite verifies the authenticator unconditionally when the seam is enabled; an empty or mismatched authenticator rejects the redeem (never skip-and-log)"
  - "InviteRecord.is_space is stamped at mint and cross-checked against the URL kind so a space invite cannot redeem as a room (and vice versa)"
  - "Member and invite records replicate on one constant topic, gcs/membership/replication; the scope id remains only in the record key"
  - "AdminCount and the admin-demotion predicate both use PrivilegeRank so the ordering rule has one source of truth"

patterns-established:
  - "Authenticator mint/verify lives in gcs::crypto and is injected through the existing KeyWrapSeam"
  - "Tombstone arrivals are filtered by the roster read, not by dropping the push"

requirements-completed: [MEMB-01, MEMB-02, MEMB-03, MEMB-04, MEMB-05]

# Metrics
duration: 22min
completed: 2026-10-18
---

# Phase 4 Plan 9: Every-Invite Bearer Gate and Membership Replication Summary

**Bearer-key-authenticated invites on a single well-known membership topic, with rank-strict removal guards, rank-based demotion dormancy, and tombstone-arrival roster convergence**

## Performance

- **Duration:** ~22 min
- **Started:** 2026-10-18T13:43:00-07:00 (approx; reconstructed from commit times)
- **Completed:** 2026-10-18T14:05:00-07:00
- **Tasks:** 4
- **Files modified:** 7

## Accomplishments

- WR-02/WR-03: removal requires a destructive tier AND a strict rank outrank; admin demotion dormancy is rank-based rather than enum-equality based.
- WR-04: every invite redeem presents a bearer key that authenticates the invite; the authenticator binds the scope id and invite id, so the invite id alone can never write a member record. Childless space invites are gated too.
- WR-04 hardening: invites now carry the scope kind (`is_space`), and a cross-kind URL is rejected before any member write; invite-id collisions across scopes are rejected.
- WR-05: a tombstone arrival pushes the tombstone-filtered roster so remote removals converge live.
- WR-06: membership records replicate on `Membership::kMembershipTopic`; the multinode fixture subscribes to that one topic and registers no per-room topic.
- WR-07 prerequisites: append-only `InviteLink.scope_id` and invite authority fields are on the wire for wave-6 consumers.
- No wave-5 suite left red: the full C++ suite passes 13/13, including `test_gcs_membership_multinode`.

## Task Commits

Each task was committed atomically:

1. **Task 1: Rank-strict removal guard and rank-based demotion dormancy** - `f9b6b3a` (fix)
2. **Task 2: Invite authority wire fields and bearer-key authenticator** - `02bab3c` (feat)
3. **Task 3: Gate every invite redeem on the bearer-key authenticator** - `3ad5d68` (fix)
4. **Task 4: Single membership topic and tombstone-arrival push** - `bf2e489` (fix)

**Plan metadata:** (docs commit created after this summary)

## Files Created/Modified

- `src/proto/gcs_chat.proto` - Append-only `InviteLink.scope_id`, `InviteRecord.is_space`, `InviteRecord.authenticator`
- `src/lib/gcs_crypto.hpp` - Invite-auth constants plus `InviteAuthMarker`, `MintInviteAuthenticator`, `VerifyInviteAuthenticator`
- `src/lib/gcs_crypto.cpp` - HKDF helper extraction, hex codec, marker digest, authenticator mint/verify
- `src/lib/gcs_membership.hpp` - `MintAuthFn`/`VerifyAuthFn` seam callables, `InviteAuthEnabled()`, `kMembershipTopic`
- `src/lib/gcs_membership.cpp` - Rank guards, kind/collision validation, unconditional authenticator gate, single topic, tombstone push
- `test/test_gcs_membership.cpp` - Rank-guard, dormancy, collision, bearer-gate, and tombstone-arrival coverage
- `test/test_gcs_membership_multinode.cpp` - Membership-topic subscription, auth seam wiring, no per-room registration

## Decisions Made

- The authenticator seals a SHA-256 marker over `(scopeId, inviteId)` under an HKDF-derived key; both ids are bound so an authenticator cannot be transplanted between invites.
- The envelope is hex-encoded for the proto3 `string` field; `bytes` fields keep binary envelopes.
- Authenticator verification is unconditional when the seam is enabled and happens before any member-record write; failure is a rejection, never skip-and-log.
- `InviteRecord.is_space` is stamped at mint and cross-checked against the URL kind at redeem.
- Membership replication uses one constant topic; the scope id stays in the CRDT key.
- `PrivilegeRank` is the single ordering authority for removal, demotion dormancy, and admin counting.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Hex-encoded the invite authenticator**
- **Found during:** Task 3 (every-invite bearer gate)
- **Issue:** The AES-256-GCM envelope is arbitrary binary, but `InviteRecord.authenticator` is a proto3 `string` field that requires valid UTF-8. Three tests failed at `ParseFromString` / `RedeemInvite` because the binary authenticator could not be stored.
- **Fix:** `MintInviteAuthenticator` returns `HexEncode(envelope)`; `VerifyInviteAuthenticator` decodes with `HexDecode` before unwrapping. The marker/key derivation is unchanged. `WrappedRoomKey.envelope` stays binary because it is a `bytes` field.
- **Files modified:** `src/lib/gcs_crypto.cpp`, `src/lib/gcs_crypto.hpp`
- **Verification:** `test_gcs_crypto`, `test_gcs_membership`, and `test_gcs_membership_multinode` all pass.
- **Committed in:** `3ad5d68` (Task 3 commit)

**2. [Rule 2 - Missing Critical] Rank-based AdminCount**
- **Found during:** Task 1 (rank-strict guards)
- **Issue:** `AdminCount` compared `record.role() == chat::ROLE_ADMIN`, a second, enum-equality copy of the admin-ordering rule that the rank guards had just centralized.
- **Fix:** `AdminCount` now counts records where `PrivilegeRank(role) >= PrivilegeRank(ROLE_ADMIN)`, matching the demotion predicate's ordering authority.
- **Files modified:** `src/lib/gcs_membership.cpp`
- **Verification:** No `record.role() == chat::ROLE_ADMIN` remains; all membership tests pass.
- **Committed in:** `f9b6b3a` (Task 1 commit)

---

**Total deviations:** 2 auto-fixed (1 bug, 1 missing critical)
**Impact on plan:** Both were required for correctness. The hex encoding is the only wire-visible encoding choice; it is documented in the crypto header and matches the plan's "return the encoded envelope" intent.

## Issues Encountered

- **clang-format environment mismatch:** Homebrew `clang-format 22.1.7` reports violations on untouched files (for example `src/lib/gcs_messaging.cpp`) under this repository's `.clang-format`, so the check is not a usable gate in this environment. This is the same known mismatch documented in 04-08. Changed regions were kept in the repository's existing Allman/vertical-alignment style by hand.
- Diagnosis of the Task 3 parse failures was by inspection: lldb attach is blocked in this sandbox. The protobuf UTF-8 constraint was confirmed from the field type in `gcs_chat.proto`.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- Production registration of `Membership::kMembershipTopic` at node init is still owned by 04-11; the fixture exercises it now.
- Behavioral proof for cross-node invitation with the bearer gate is owned by 04-12.
- WR-07 push consumers can rely on `InviteLink.scope_id` and the appended invite authority fields.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-18*

## Self-Check: PASSED

- All 7 modified files and this SUMMARY exist on disk.
- Commits `f9b6b3a`, `02bab3c`, `3ad5d68`, and `bf2e489` exist in the repository history.
- Full C++ suite: 13/13 tests passed.
