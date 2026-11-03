---
phase: 04-membership-invites
plan: 18
subsystem: membership
tags: [multinode, invite-issuer-signature, secp256k1, genius-signer, forged-record, cr-08, cpp17, gtest]

# Dependency graph
requires:
  - phase: 04-12
    provides: Two-node multinode fixture conventions (baselines, event draining, wire-level writes, public key prefixes)
  - phase: 04-15
    provides: Issuer-signed invite records, canonical payload builder, IssuerAuthEnabled gate, scoped URLs, kSignatureBytes
  - phase: 04-16
    provides: Production FFI signer seam pattern (address, signature, data) and kSignatureBytes length guard
provides:
  - Real secp256k1 signer identities in the multinode membership fixture
  - Real-crypto issuer seam callables bound to each node's signer
  - Genuine cross-node signed-invite redeem positive control under real crypto
  - Wire-level forged-issuer rejection with a pre-redeem baseline capture
affects: [04-VERIFICATION CR-08 wire proof]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Fixture-owned real signer (unique_ptr, declared before the nodes) captured by reference in the issuer seam"
    - "verifyIssuer reorders the seam to VerifySignature(address, signature, data) and guards kSignatureBytes"
    - "Forged-issuer baseline captured BEFORE the redeem; absent member record asserted absent before and after"
    - "Attacker bearer key is the existing kWellFormedInviteKey (base64url of 32 zero bytes) so the test needs no second base64 encoder"

key-files:
  created: []
  modified:
    - test/test_gcs_membership_multinode.cpp

key-decisions:
  - "The fixture's two identities are real GeniusSigner instances generated in SetUp; the signer pair is declared before the nodes so it outlives every membership that binds it by reference"
  - "The seam binds the signer by reference and rejects any issuer that is not that signer's address; ShutdownNode evicts the membership before fixture destruction (T-04-18-03)"
  - "verifyIssuer strips a leading 0x, requires exactly Membership::kSignatureBytes, and calls the byte overload as (address, signature, data)"
  - "The forged test mints a valid authenticator with a 32-zero-byte key whose base64url is the existing kWellFormedInviteKey, so only issuer binding can reject the record"
  - "The negative assertion is record-count/absence based: B's member record is absent before and after, never a byte comparison of a record that must not exist"

patterns-established:
  - "Real-signer multinode fixture: generated identities plus real-crypto issuer callables, no permissive double"

requirements-completed: [MEMB-01, MEMB-02]

# Metrics
duration: 5min
completed: 2026-10-18
---

# Phase 4 Plan 18: Real-Signer Multinode Issuer Proof Summary

**The two-node suite now signs and verifies with real secp256k1 keys: a genuine creator-signed invite redeems across the mesh, while a wire-level record claiming the creator but signed by an attacker key is rejected before any member write**

## Performance

- **Duration:** 5 min
- **Started:** 2026-10-18T00:07:17Z
- **Completed:** 2026-10-18T00:12:16Z
- **Tasks:** 2
- **Files modified:** 1

## Accomplishments

- Replaced the two hard-coded fake sender constants with fixture-owned `sgns::GeniusSigner` instances; every test now uses the real 128-hex addresses the nodes actually hold.
- `ArmMembership` installs the real-crypto issuer pair: `sign` is bound to the node's signer and encodes the raw signature binary-safely; `verifyIssuer` strips `0x`, enforces `kSignatureBytes`, and passes `(address, signature, data)` to `GeniusSigner::VerifySignature`.
- The genuine cross-node space-invite redeem (Test 3) passes under real crypto, validating 04-15's payload/hex/seam contract outside the unit double.
- Added `ForgedIssuerSignatureCannotGrantARoleOnTheWire`: the record carries a valid bearer authenticator, `created_by = SenderA()`, `role = ROLE_SUPER_ADMIN`, and an attacker signature over the canonical payload. Redeem fails, the member-record count is unchanged, B's member record is still absent, and B's roster resolves `ROLE_UNSPECIFIED`.
- The pre-existing wire-level forged case now uses the scoped URL shape and the public `kInvitesKeyPrefix` scope key (no private `InviteKey` helper).

## Task Commits

Each task was committed atomically:

1. **Task 1: Real signer identities in the fixture and a real-crypto issuer seam** - `8df0359` (test)
2. **Task 2: Genuine cross-node redeem stays green; forged issuer is rejected with baseline capture** - `ec0bff3` (test)

**Plan metadata:** `_pending_` (docs: complete plan)

## Files Created/Modified

- `test/test_gcs_membership_multinode.cpp` - real signer members and `SenderA()`/`SenderB()` accessors, `ArmMembership` issuer seam, scoped forged URL, and the new `ForgedIssuerSignatureCannotGrantARoleOnTheWire` case

## Decisions Made

- The signer pair is declared before `m_nodeA`/`m_nodeB`, so reverse declaration-order destruction releases the nodes first; `ShutdownNode` additionally evicts the membership before fixture destruction, satisfying T-04-18-03.
- The seam captures the signer by reference (not `shared_ptr`): the fixture owns it for the whole test, and the membership is reset in `TearDown`.
- The forged case uses `kWellFormedInviteKey` as the attacker bearer key because it is the unpadded base64url encoding of `kRoomKeyLengthBytes` zero bytes; the authenticator is minted from those same bytes, so the test does not duplicate a base64 encoder.
- The forged record keeps the genuine record's `scope_id`/`is_space` so only the issuer signature can fail; the role is escalated to `ROLE_SUPER_ADMIN` to pin T-04-18-02.

## Deviations from Plan

None - plan executed exactly as written. No production, proto, or fixture-support change was needed.

## Issues Encountered

- **clang-format environment mismatch (known):** Homebrew `clang-format 22.1.7` reports whole-file violations on untouched baseline lines (including pre-existing `MultinodeNode` members and `ArmMembership`'s continuation indent), the same condition documented in 04-11/04-12/04-13/04-14/04-15/04-16. No whole-file reformat was applied; added lines follow the surrounding file style.
- **Acceptance-grep wording:** the plan's `grep -c "InviteKey" ... is 0` is not literally satisfiable because the plan itself retains the existing `kWellFormedInviteKey` constant, whose name contains that substring. The intended check was verified instead: `grep -cE '\bInviteKey[[:space:]]*\('` is 0, so the private `InviteKey` helper is never named or used.
- **clang-tidy:** no repository-root `.clang-tidy` applies (the only one lives in the untouched `GNUS-NEO-SWARM` submodule), so it was not run as a gate. The build produced no new warnings.

## Verification

- `ninja -C build/OSX/Debug test_gcs_membership_multinode` - success, no new warnings
- `ctest --test-dir build/OSX/Debug -R "^test_gcs_membership_multinode$" --output-on-failure` - passed (6 cases)
- `ctest --test-dir build/OSX/Debug --output-on-failure` - **13/13 suites passed** (the last expected red is gone)
- Intended-check inspection: `--gtest_filter=*ForgedIssuerSignatureCannotGrantARoleOnTheWire` logs `redeem rejected — invite 'invite-forged-issuer' issuer signature did not verify`, proving the rejection is issuer binding, not the bearer gate, URL parse, or role cap
- Source checks: `kSenderA`/`kSenderB` count is 0; `ArmMembership` takes three parameters and both call sites pass a signer; `VerifySignature` is called with the signature vector second and the payload vector third under `kSignatureBytes`; every test key uses a public prefix; no `sleep_for`/`sleep_until`

## Known Stubs

None.

## Threat Flags

None - test-only change; no new network endpoint, auth path, file access pattern, or schema change.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- CR-08's wire-level claim is now proven with real crypto: signature verification is what stands between a replicated invite record and a member grant.
- 04-VERIFICATION can cite `test_gcs_membership_multinode` (6 cases) for the genuine-signed-redeem and forged-issuer truths.
- Phase 04's full C++ suite is green; no expected intermediate reds remain.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-18*

## Self-Check: PASSED

- FOUND: `.planning/workstreams/app/phases/04-membership-invites/04-18-SUMMARY.md`
- FOUND: `8df0359` (Task 1)
- FOUND: `ec0bff3` (Task 2)
- FOUND: `test/test_gcs_membership_multinode.cpp`
