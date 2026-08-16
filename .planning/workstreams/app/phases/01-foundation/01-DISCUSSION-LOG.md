# Phase 1: Foundation - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-08-15
**Phase:** 1-Foundation
**Areas discussed:** FFI surface shape, FFI boundary / library layout, async/event model across FFI, CI/CD structure

---

## FFI Surface Shape

| Option | Description | Selected |
|--------|-------------|----------|
| Per-module `extern "C"` headers | One header per module (`gcs_chat.h`, `gcs_rooms.h`, ...) | |
| Single opaque-handle session API | One header; `gcs_init` → handle; all calls take the handle | ✓ |

**User's choice:** "a single opaque-handle API"
**Notes:** Rationale given with area 2: Cubit-style C++ state storage with a thin Flutter UI wrapper.

## FFI Boundary / Library Layout

| Option | Description | Selected |
|--------|-------------|----------|
| Thin C wrapper inside `gcs_core` | Flutter links gcs_core static lib directly | |
| Separate `gcs_ffi` shared library | Own CMake target, links gcs_core; Flutter loads dylib/so/dll | ✓ |

**User's choice:** "separate gcs_ffi shared library, not sure neoswarm_ffi is needed any more, but if so, that's ok"
**Notes:** neoswarm_ffi left working; removal re-evaluated at Phase 6. Recorded as D-03.

## Async / Event Model Across FFI

| Option | Description | Selected |
|--------|-------------|----------|
| Dart polls C++ | Dart calls receive/poll on timer or after send | |
| Native→Dart callbacks | C++ pushes events via ffigen NativeFunction + ReceivePort | ✓ |

**User's choice:** "No, dart doesn't need to poll, but the point is that C++ handles most everything and Dart is a thin wrapper for input/output text or selecting channels, config, etc. But it of course can receive callbacks instead of polling for that data arriving."
**Notes:** Initial "send/receive FFI calls to dart" phrasing was clarified in follow-up — callbacks, not polling. Same pattern as neoswarm_ffi already uses.

## CI/CD Structure

| Option | Description | Selected |
|--------|-------------|----------|
| Monolithic matrix workflow | One workflow, 5-platform matrix, SuperGenius-style | ✓ |
| Per-platform workflows | Separate workflow files per platform | |
| Deps pre-installed on runners | Runner images carry thirdparty/SuperGenius/GeniusSDK | |
| Deps downloaded as prebuilt releases | gh release download from thirdparty/SuperGenius/GeniusSDK release tags | ✓ |

**User's choice:** "Check how ../SuperGenius repo does the CI/CD"
**Notes:** SuperGenius `cmake.yml` inspected: resolve-runners job (self-hosted labels sg-ubuntu-linux / sg-arm-linux / SG-WIN11 / gv-OSX-Large with GitHub-hosted fallback), matrix Android(arm64-v8a, armeabi-v7a)/iOS/OSX/Linux(x86_64, aarch64)/Windows × Debug/Release, prebuilt thirdparty release tarballs, ctest, release upload. GeniusSDK workflow confirmed as the consumer-repo analog (downloads SuperGenius + thirdparty). GCS CI copies this, adds SuperGenius + GeniusSDK download steps, skips zkLLVM. Phase 1 CI builds C++ core only (no Flutter app build).

## Claude's Discretion

- Exact C API names/signatures (C-only, opaque handle, C++17)
- GTest smoke-test specifics (wait-condition templates, no sleep_for)
- `gcs_ffi` target placement under `src/ffi/` (expected home per user's repo restructure)

## Deferred Ideas

- Retiring `neoswarm_ffi` → re-evaluate Phase 6 (GCS Bot)
- Flutter app bundles in CI → later phase
- zkLLVM in CI — not needed by GCS
- Message CRDT schema → Phase 3 (todo tracked)
- GCS bot identity mapping → Phase 6 (todo tracked)
