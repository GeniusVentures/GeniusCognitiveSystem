---
phase: 04-membership-invites
plan: 15
subsystem: membership
tags: [invite-issuer-signature, forged-record, role-cap, scope-binding, crdt, cpp17, gtest]

# Dependency graph
requires:
  - phase: 04-09
    provides: Invite mint/redeem path with the bearer authenticator and D-07 dormant approval model
  - phase: 04-13
    provides: RedeemInvite state-preservation merge and tombstone rejection
  - phase: 04-14
    provides: Catalog-consistent destructive entity writes and fail-closed AdminCount
provides:
  - InviteRecord.created_by/issuer_signature with a canonical issuer payload
  - KeyWrapSeam SignFn/VerifyIssuerFn and the fail-closed IssuerAuthEnabled gate
  - Scope-bearing invite URLs and exact (scope, invite) key lookup at redeem
  - Issuer-signature, authority, GUEST role-cap, and wrapped-key scope validation
  - Public HexEncode/HexDecode and Membership::kSignatureBytes
affects: [04-16 FFI issuer seam wiring, 04-18 real-signer multinode proof, 04-17 role-change hardening]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Canonical issuer payload: domain separator + invite id + serialized record with the signature field cleared"
    - "Reject-before-side-effect ordering: signature/role/scope checks precede the tombstone read, unwrap loop, and member merge"
    - "Exact-key invite lookup: the URL carries the scope so InviteKey(scope, invite) replaces the global scan"
    - "Test-double signer binding: a deterministic token that verifies only against the claimed issuer"

key-files:
  created:
    - test/test_gcs_invite_helpers.hpp
  modified:
    - src/proto/gcs_chat.proto
    - src/lib/gcs_crypto.hpp
    - src/lib/gcs_crypto.cpp
    - src/lib/gcs_membership.hpp
    - src/lib/gcs_membership.cpp
    - test/test_gcs_membership.cpp

key-decisions:
  - "Invite minting and redeeming fail closed without the issuer seam; no unsigned record is written or accepted"
  - "The issuer signature covers domain + invite id + the serialized record with issuer_signature cleared"
  - "Redeem looks the record up under InviteKey(urlScopeId, inviteId) and requires invite.scope_id() == urlScopeId"
  - "The D-05 GUEST role cap runs on EVERY redeem, resolved or not, before the 04-13 merge"
  - "Issuer authority is checked only when the scope resolves locally; the cold cross-node path relies on the signature plus the GUEST cap (T-04-15-04)"
  - "A space-invite wrapped key for an unresolved room is dropped; a resolved parent mismatch or a room-invite foreign key is a hard rejection"
  - "The test double's signature token is hex-encoded before it is stored, exactly like MintInvite, because the token embeds binary envelope bytes"

patterns-established:
  - "Issuer-attributed invite record: created_by + signature verified before any member write"

requirements-completed: [MEMB-01, MEMB-02, MEMB-05]

# Metrics
duration: 16min
completed: 2026-10-18
---

# Phase 4 Plan 15: Issuer-Signed Invites Summary

**Invite records now name and are signed by their issuer; redeem verifies the signature, the issuer's authority, the GUEST role cap, and the wrapped-key scope before writing any member state**

## Performance

- **Duration:** 16 min
- **Completed:** 2026-10-18T23:49:00Z
- **Tasks:** 3
- **Files modified:** 6 (plus 1 created)

## Accomplishments

- `InviteRecord` gained append-only `created_by = 6` and `issuer_signature = 7` (CR-08 / MEMB-01); the existing hex codec was promoted to the `gcs::crypto` public surface instead of duplicating it (invariant 11)
- `KeyWrapSeam` gained `SignFn`/`VerifyIssuerFn`; `IssuerAuthEnabled()` gates both mint and redeem, so the component never writes or accepts an unsigned record
- `MintInvite` stamps `created_by`, signs the canonical payload (domain + invite id + serialized record with the signature field cleared), hex-encodes the raw bytes, and returns a scope-bearing URL: `gcs://invite/<kind>/<scope>/<invite>?key=<IK>`
- `RedeemInvite` replaced the global invite scan with an exact `InviteKey(urlScopeId, inviteId)` Get, cross-checks `invite.scope_id()`, verifies the bearer authenticator and the issuer signature against the CLAIMED issuer, enforces the D-05 GUEST cap on every redeem, checks issuer authority when the scope resolves, and validates wrapped room keys against the invite's scope before any unwrap or store
- `CanInvite` now delegates to `CanInviteFor(scopeId, address)`, the single authority predicate used for third-party issuers
- Eight new/renamed unit cases pin mint stamping, disabled-seam fail-closed behaviour, tampered role, forged issuer identity, revoked authority, authorized-issuer escalation, and wrapped-key scope violations

## Task Commits

Each task was committed atomically:

1. **Task 1: Append-only proto fields, public hex codec, issuer-verification seam** - `0b84bdf` (feat)
2. **Task 2: Sign at mint; verify scope, signature, role cap, issuer authority, and room-key scope at redeem** - `4ed03bf` (fix)
3. **Task 3: Unit tests, shared payload builder, and rejection coverage** - `9138ad8` (test)

**Plan metadata:** `_pending_` (docs: complete plan)

## Files Created/Modified

- `src/proto/gcs_chat.proto` - `created_by`/`issuer_signature` appended after `authenticator = 5`; no existing field renumbered
- `src/lib/gcs_crypto.hpp` / `.cpp` - `HexEncode`/`HexDecode` moved from the anonymous namespace to the public surface; constants and `HexValue` stay private
- `src/lib/gcs_membership.hpp` - `SignFn`, `VerifyIssuerFn`, seam members, `IssuerAuthEnabled`, `CanInviteFor`, `kSignatureBytes`; Mint/Redeem Doxygen updated for the new URL and validation
- `src/lib/gcs_membership.cpp` - `kInviteIssuerDomain` + `InviteIssuerPayload`, `IssuerAuthEnabled`, `CanInviteFor`, signed mint, exact-key redeem with signature/role/authority/scope validation, filtered wrapped-key merge
- `test/test_gcs_invite_helpers.hpp` - the single `BuildInviteIssuerPayload` test builder shared with the future 04-18 multinode proof
- `test/test_gcs_membership.cpp` - address-bound `MakeSeam`, invite read/write/scan helpers, the rewritten malformed-URL case, the retargeted-scope case, and six new rejection cases

## Decisions Made

- Signature verification binds the CLAIMED issuer (`created_by`): a record signed by X while claiming Y fails.
- The GUEST role cap is unconditional because an authorized `membersCanInvite` MEMBER can re-sign a higher role under its own identity; signature plus authority cannot stop that.
- On the cold cross-node path (scope unresolved) the only residual rule is a valid signature plus the GUEST cap (T-04-15-04).
- An unresolved space-invite room id is dropped with a warn and never reaches `crypto::StoreRoomKey`; a resolved parent mismatch and a room-invite foreign key are hard rejections.
- `kSignatureBytes = 64` is the single raw-signature-length home for 04-16 and 04-18.

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 1 - Bug] Test-double signatures embedding binary bytes were written un-hexed**
- **Found during:** Task 3 (full new-test run logged `invalid UTF-8 data` while parsing `issuer_signature`)
- **Issue:** The plan's literal re-sign snippet stores `"testsig:" + issuer + ":" + payload` directly in the proto3 `string` field. The payload embeds the serialized record, whose `bytes` wrapped-key envelope can be binary; a proto3 string field then fails UTF-8 validation on parse. In `RedeemRejectsWrappedRoomKeyOutsideScope` the record became unparseable, so the scope check did not actually fire — a false pass.
- **Fix:** `TestIssuerSignatureHex` hex-encodes the token before it is stored, mirroring `MintInvite`'s `crypto::HexEncode` at the proto boundary. Confirmed the intended warnings now fire (`carries a key for foreign room`, `carries its own scope ... as a room`, `grants a non-Guest role`, `lacks invite authority`, `issuer signature did not verify`).
- **Files modified:** `test/test_gcs_membership.cpp`
- **Commit:** `9138ad8`

## Expected Intermediate Reds

These are sequenced dependencies recorded in the plan and `04-VALIDATION.md`, not regressions; no permissive fallback was added.

- `test_gcs_ffi` — 1 case red: `GcsFFI.InviteMintsSpaceInviteLinkWithCapabilityShape` (the production FFI seam has no sign/verify callables yet; 04-16 restores them). The other 15 FFI cases pass.
- `test_gcs_membership_multinode` — all 5 cases red: `RemoteMemberListConvergesAfterRedeem`, `RemoteRoleChangeConverges`, `SpaceInviteConvergesWithoutTheRecipientJoiningTheSpace`, `ForgedOrKeylessSpaceInviteWritesNoMemberRecord`, `RemovedMemberReceivesTheConvergedRoster` (`ArmMembership` supplies no issuer callables; 04-18 restores them).

All other 11 suites pass.

## Issues Encountered

- `clang-format --dry-run --Werror` (v22.1.7 with the repository `.clang-format`) reports whole-file violations on every touched file, including pre-existing lines (namespace/declaration indentation, `( ... )` spacing). The same condition was documented in 04-13 and 04-14. No reformat was applied to avoid an unrelated whole-file diff; the added lines follow the surrounding file style.
- `clang-tidy -p build/OSX/Debug` cannot run: the generated `compile_commands.json` entries contain two compiler jobs (x86_64 and arm64 universal build) and clang-tidy requires exactly one. The build completed with no new warnings for all changed targets.

## Verification

- `ninja -C build/OSX/Debug gcs_proto gcs_core` - success; generated `gcs_chat.pb.h` exposes `created_by()`/`issuer_signature()`
- `ninja -C build/OSX/Debug test_gcs_membership` - success, no new warnings
- `ctest --test-dir build/OSX/Debug -R "^test_gcs_membership$" --output-on-failure` - passed
- New cases run explicitly by `--gtest_filter` and their rejection warnings inspected to confirm the intended check (not an incidental parse failure) fired
- `ctest --test-dir build/OSX/Debug --output-on-failure` - 11/13 suites pass; the 2 expected reds above
- Source checks: `matchCount` is gone; `VerifySignature` never appears in `gcs_membership.cpp`; `IssuerAuthEnabled()` gates mint and redeem; the minted URL has kind/scope/invite/key segments; `verifyIssuer` precedes `WriteMember`; the role cap precedes the merge on every path; no `sleep_for`/`sleep_until` in the test file

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- CR-08 is closed at the membership component boundary: a permissionless-topic writer can no longer produce a believable invite without a controlled issuer key, and every accepted record is capped at GUEST.
- 04-16 can wire the production FFI adapter using `Membership::kSignatureBytes` and the reorder note: `verifyIssuer(issuer, payload, signature)` at the seam maps to `GeniusSigner::VerifySignature(address, signature, data)`.
- 04-18 can include `test/test_gcs_invite_helpers.hpp` for the real-signer multinode proof.

---

*Phase: 04-membership-invites*
*Completed: 2026-10-18*

## Self-Check: PASSED

- FOUND: `.planning/workstreams/app/phases/04-membership-invites/04-15-SUMMARY.md`
- FOUND: `0b84bdf` (Task 1)
- FOUND: `4ed03bf` (Task 2)
- FOUND: `9138ad8` (Task 3)
- FOUND: `src/proto/gcs_chat.proto`, `src/lib/gcs_crypto.hpp`, `src/lib/gcs_crypto.cpp`, `src/lib/gcs_membership.hpp`, `src/lib/gcs_membership.cpp`, `test/test_gcs_membership.cpp`, `test/test_gcs_invite_helpers.hpp`
