---
phase: 04-membership-invites
plan: 16
subsystem: ffi
tags: [issuer-signature, cr-08, memb-01, ffi, genius-account, secp256k1, dart-codegen, protoc-plugin, invite-link]

# Dependency graph
requires:
  - phase: 04-11
    provides: Production KeyWrapSeam wiring (wrap/unwrap/mintAuth/verifyAuth) and teardown ordering
  - phase: 04-15
    provides: Issuer-signed invite records, canonical payload, IssuerAuthEnabled gate, scoped URLs
provides:
  - Account-backed issuer signer built once per session from the embedded node's active mnemonic
  - Fail-closed production seam when no active account is available
  - Production mint-then-redeem wiring proof in the FFI suite
  - Pinned-22.5.0 Dart bindings exposing createdBy/issuerSignature
  - Two-segment scoped join-link gate in Dart
affects: [04-18 real-signer multinode proof]

# Tech tracking
tech-stack:
  added: []
  patterns:
    - "Seam callables capture their own shared_ptr copy so the global reset cannot dangle them"
    - "Seam order (issuer, payload, signature) is explicitly reordered to VerifySignature(address, signature, data)"
    - "Issuer account is derived from GeniusSDKGetMnemonic() (by-value struct) and never logged or stored on the seam"
    - "Dart link regex is a UX gate only; C++ re-parses the same scoped shape"

key-files:
  created: []
  modified:
    - src/ffi/gcs_core_ffi.cpp
    - test/test_gcs_ffi.cpp
    - src/app/lib/generated/proto/gcs_chat.pb.dart
    - src/app/lib/generated/proto/gcs_chat.pbjson.dart
    - src/app/lib/shell/join_dialog.dart
    - src/app/test/shell/join_dialog_test.dart

key-decisions:
  - "The FFI builds one GeniusAccount per session from the node's active mnemonic and assigns sign/verifyIssuer only when construction succeeds"
  - "An empty mnemonic or nullptr account logs an error and leaves the issuer pair unset, so 04-15's IssuerAuthEnabled gate fails mint and redeem closed"
  - "g_inviteAccount is a shared_ptr; each seam lambda captures its own copy and the global resets after membership eviction"
  - "verifyIssuer strips a leading 0x, requires exactly Membership::kSignatureBytes, and passes (address, signature, data) in that order"
  - "The FFI test fixture pins a fixed BIP39 mnemonic so the tests exercise production wiring; no test-only seam override exists"
  - "Dart regenerates the proto with protoc_plugin 22.5.0 only and accepts kind/scope/invite segments before the key"

patterns-established:
  - "Production issuer signer: account-backed seam callables with fail-closed construction"

requirements-completed: [MEMB-01]

# Metrics
duration: 25min
completed: 2026-10-18
---

# Phase 4 Plan 16: Production Issuer-Signing Seam and Scoped Link Surfaces Summary

**Production invites are now signed at mint and verified at redeem by the embedded node's active account, with a fail-closed path when no account exists, and the Dart app accepts the scoped link shape against regenerated 22.5.0 bindings**

## Performance

- **Duration:** 25 min
- **Started:** 2026-10-18T23:33:00Z (approx)
- **Completed:** 2026-10-18T23:58:36Z
- **Tasks:** 3
- **Files modified:** 6

## Accomplishments

- `gcs_core_ffi.cpp` now builds `g_inviteAccount` (`std::shared_ptr<sgns::GeniusAccount>`) once per session from `GeniusSDKGetMnemonic()` (read as a by-value struct member, never a pointer) and `GeniusSDKGetNode()->GetTokenID()`, using the boot base path as `boost::filesystem::path`.
- `keyWrapSeam.sign` signs the canonical payload with the active account; `keyWrapSeam.verifyIssuer` strips `0x`, enforces the `kSignatureBytes` length, and calls `sgns::GeniusSigner::VerifySignature(address, signature, data)` with the seam's `(issuer, payload, signature)` order explicitly reordered.
- Both callables capture a copy of the shared_ptr and null-check it; `g_inviteAccount` resets after membership eviction and before session shutdown, so no live callable can observe a freed account.
- An empty mnemonic or failed `NewFromMnemonic` logs an error and leaves the issuer pair unset — every production mint/redeem then fails closed through 04-15's `IssuerAuthEnabled()` gate.
- The FFI suite pins a deterministic BIP39 mnemonic on the fixture config, asserts the scoped `gcs://invite/space/<scope>/<invite>?key=` URL, and proves an in-process mint-then-redeem succeeds with the creator still `ROLE_SUPER_ADMIN`.
- Dart bindings were regenerated with the pinned protoc_plugin 22.5.0 (no `BuilderInfo.aE`), exposing `createdBy` and `issuerSignature`; `kInviteLinkPattern` now requires two path segments and the widget tests cover the valid link plus single-segment, empty-scope, empty-id, and missing-key rejects.

## Task Commits

Each task was committed atomically:

1. **Task 1: Account-backed issuer signer wired into the FFI seam** - `5b23e7f` (feat)
2. **Task 2: Scoped URL assertion and signed mint/redeem wiring proof** - `39bc52a` (test)
3. **Task 3: Pinned pb regen and the two-segment join-link gate** - `7f6decf` (feat)

**Plan metadata:** `_pending_` (docs: complete plan)

## Files Created/Modified

- `src/ffi/gcs_core_ffi.cpp` - `g_inviteAccount` global, account construction at init, sign/verifyIssuer lambdas, teardown reset, SIX-callable seam comment
- `test/test_gcs_ffi.cpp` - `kTestMnemonic` fixture pin, scoped URL assertions, `RedeemOwnInviteIsAcceptedByTheProductionIssuerCheck`, expanded malformed-URL table
- `src/app/lib/generated/proto/gcs_chat.pb.dart` - generated `createdBy`/`issuerSignature` accessors (pinned 22.5.0)
- `src/app/lib/generated/proto/gcs_chat.pbjson.dart` - generated field descriptors for the same fields
- `src/app/lib/shell/join_dialog.dart` - two-segment `kInviteLinkPattern` and updated doc comments
- `src/app/test/shell/join_dialog_test.dart` - scoped valid link plus four new invalid-shape cases

## Decisions Made

- The issuer account is built only after `EnsureSdkBooted` returns true, so `GeniusSDKGetNode()` and the active mnemonic are guaranteed live.
- `sign` rejects an issuer that differs from `GeniusSDKGetAddress().address` (the exact string Membership receives) and returns a binary-safe `std::string` of the raw signature bytes.
- `verifyIssuer` treats a null account, a non-`kSignatureBytes` signature, or any signer failure as `success(false)` — verification never throws and never logs the signature.
- The test fixture uses one fixed mnemonic for every boot; tests exercise the production seam rather than an override.

## Deviations from Plan

None - plan executed exactly as written.

## Issues Encountered

- **`flutter analyze` whole-app scope:** the analyzer reports 50 pre-existing errors in `src/app/scaffold/example/test/capture_images_test.dart`, which lives in the hands-off `src/app/scaffold` submodule. A scoped `flutter analyze lib/shell/join_dialog.dart test/shell/join_dialog_test.dart lib/generated/proto` reports "No issues found!", and `flutter test test/shell/join_dialog_test.dart` passes 2/2. The submodule was not touched.
- **clang-format (known environment mismatch):** Homebrew `clang-format 22.1.7` reports whole-file violations on untouched baseline lines (e.g. `src/ffi/gcs_core_ffi.cpp:55`, the pre-existing `constexpr const char*` declarations), the same condition documented in 04-11/04-13/04-14/04-15. No reformat was applied; added lines follow the surrounding hand style.
- **clang-tidy:** there is no `.clang-tidy` configuration applying to `src/` (the only one is in the untouched `GNUS-NEO-SWARM` submodule), so it was not run as a gate. The build completed with no new warnings.
- **Expected intermediate red:** `test_gcs_membership_multinode` remains red until 04-18 supplies real issuer callables in `ArmMembership`; it was excluded from the verification run as recorded in 04-VALIDATION.md and the plan.

## Verification

- `ninja -C build/OSX/Debug gcs_ffi test_gcs_ffi` - success, no new warnings; full incremental `ninja` also relinked `test_gcs_ffi_sdk` / `test_gcs_ffi_coldboot`
- `ctest --test-dir build/OSX/Debug -R "^test_gcs_ffi$" --output-on-failure` - passed (17 cases, including the new production mint/redeem proof)
- `ctest --test-dir build/OSX/Debug --output-on-failure -E "^test_gcs_membership_multinode$"` - 12/12 suites passed
- `cd src/app && flutter analyze` (touched paths) - no issues; `flutter test test/shell/join_dialog_test.dart` - 2/2 passed
- Source checks: `g_inviteAccount` is a `shared_ptr` and resets after membership eviction; the mnemonic is read via `.mnemonic` on the by-value struct (no `->mnemonic`); `VerifySignature` receives `(bareAddress, signatureBytes, payloadBytes)`; the length guard names `gcs::Membership::kSignatureBytes`; the old two-segment literal is absent from the widget test; `BuilderInfo.aE` count is 0

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness

- CR-08's production half is closed: minting signs with the embedded node's active account and redeem verifies statically, with no unsigned fallback.
- 04-18 can reuse the fixture configuration pattern (`GcsConfig.mnemonic`) and `test/test_gcs_invite_helpers.hpp` for the real-signer multinode proof; it must still supply issuer callables in `ArmMembership`.
- The Dart join gate forwards the scoped link verbatim; C++ remains the canonical validator.

---
*Phase: 04-membership-invites*
*Completed: 2026-10-18*

## Self-Check: PASSED

- FOUND: `.planning/workstreams/app/phases/04-membership-invites/04-16-SUMMARY.md`
- FOUND: `5b23e7f` (Task 1)
- FOUND: `39bc52a` (Task 2)
- FOUND: `7f6decf` (Task 3)
- FOUND: all six modified source/test files
- Full C++ suite (excluding the documented 04-18 multinode red): 12/12 suites passed
- Dart: `flutter test test/shell/join_dialog_test.dart` 2/2 passed; touched-path analyze clean
