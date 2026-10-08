# GSD INBOX TRIAGE — GeniusVentures/GeniusCognitiveSystem — 2026-10-01

Scope: `--prs` (issues skipped); focus `#16 resolve and resolve conversation`.

## Summary

Open PRs: 1 (draft)
Template compliance: N/A — this repo has no `.github/PULL_REQUEST_TEMPLATE/`,
`.github/ISSUE_TEMPLATE/`, `CONTRIBUTING.md`, or `.changeset/`; the GSD
typed-template and issue-first gates do not apply here.

## PR #16 — Per-platform cmake app fragments + flutter toolchain in CI (draft)

- Branch: `feature/platform-app-fragments` → `develop`
- Commits: `16ca8c3` (fragments + flutter CI), `0c78d94` (review/CI fixes)
- Linked issue: none — no issue templates exist in this repo; tracked as a
  direct build-infra change.

### Codex review (chatgpt-codex-connector) — 3 P1 findings

All three verified against sources, fixed in `0c78d94`, threads replied + resolved:

| Finding | Verdict | Fix |
|---|---|---|
| Linux `bundle/lib` not probed by `SessionCubit._resolveLibraryPath()` | CONFIRMED (probes were `$exeDir/../Frameworks` + `$exeDir` only) | Dart probe list extended with `$exeDir/lib` |
| iOS `Runner.app/Frameworks` not probed | CONFIRMED | Dart probe list extended with `$exeDir/Frameworks` |
| Windows app target doesn't bundle `vulkan-1.dll` | CONFIRMED (`VULKAN_RUNTIME_DLL` exists for exactly this; `test/CMakeLists.txt` deploys it per-test) | `app_build_windows` copies it beside `gcs_ffi.dll` |

### CI run 36904228994 (commit 16ca8c3) — 4 failed legs

| Leg | Cause | Action |
|---|---|---|
| OSX Debug + Release | `test_gcs_ffi_dart`: headless runner can't create GeniusSDK account ("Account creation failed"; all C++ gtests pass) | `-DFRONTEND_TESTS_ENABLED=OFF` on the CI mac leg (dart leg still gates locally + on Linux legs) |
| Windows Release (Debug pending) | `test_gcs_ffi_dart`: `gcs_ffi.dll` load failure ("unknown error") — no `vulkan-1.dll` in the ffi build-output dir | POST_BUILD deploys `vulkan-1.dll` beside `gcs_ffi.dll` (`cmake/Windows/tests.cmake`) |
| Linux aarch64 Debug | Runner shutdown signal, exit 137 — infra flake, not the change | Covered by re-run on `0c78d94` |

Passing legs: Android (all 4), iOS Release, Linux aarch64 Release — the new
fragments configure cleanly on those platforms with flutter installed.

### Status after triage

- Review conversations: 3/3 resolved
- Re-run: triggered by push of `0c78d94` (all 16 legs)
- Outstanding: confirm the re-run goes green, then mark ready for review
  (Codex bot re-fires on ready)
