---
phase: 260922-gev-audit-frontend-scaffold-separation-of-co
plan: 01
subsystem: app-frontend
tags: [audit, read-only, scaffold, submodule, separation-of-concerns, ffi]
requires: []
provides:
  - "Verdict table (CLEAN / VIOLATION / SMELL per check) with reproducible evidence excerpts"
  - "Overall conclusion: GCS derives from the scaffold; one procedural VIOLATION (unpushed ticket commit pinned by parent), zero code-level integrate-into"
affects: []
key-files:
  created:
    - .planning/quick/260922-gev-audit-frontend-scaffold-separation-of-co/260922-gev-SUMMARY.md
  modified: []
decisions:
  - "Audit-only: violations reported with recommended fixes; nothing applied (user decision)"
metrics:
  duration: "~5 min (18:52-18:57 UTC)"
  completed: 2026-09-22
  tasks: 3
---

# Quick Task 260922-gev: Audit Frontend Scaffold Separation of Concerns — Summary

One-liner: READ-ONLY audit of the GCS app (`src/app/lib/`) vs the `frontend_scaffold` submodule (`src/app/scaffold`). **Verdict: GCS derives from the scaffold and has NOT repeated the genius-ai-boss integrate-into mistake in code — but there is one procedural VIOLATION (an unpushed, GCS-authored docs ticket commit inside the submodule that the parent pins), one related SMELL (a dormant stash of WIP edits to the scaffold's CMakeLists.txt), and one hygiene SMELL (committed generated output).**

All commands were run read-only from the repo root (`git -C <path>`, `ls-remote` instead of `fetch`). Zero files modified outside this SUMMARY.

## Verdict Table

### Task 1 — Scaffold submodule integrity

| # | Check | Verdict | Evidence |
|---|-------|---------|----------|
| 1.1 | Working-tree dirt | **CLEAN** | `git -C src/app/scaffold status --porcelain` → empty (exit 0). No modified or untracked files inside the submodule. |
| 1.2 | Pointer sync (index vs checkout) | **CLEAN** | `git submodule status src/app/scaffold` → ` 7760ee2a92ded18ea5efe8b8ade494d0f1ceaad6 src/app/scaffold (v1.1-199-g7760ee2)` — leading space = in sync; matches planning baseline. |
| 1.3 | Detached vs branch tracking | **SMELL** | `git -C src/app/scaffold branch -vv` → `* develop 7760ee2 [origin/develop: ahead 1]`. Submodule sits on local branch `develop`, not detached HEAD — which is how the local-only commit (1.4) came to exist. Risk: `git submodule update` will fight the branch checkout. |
| 1.4 | GCS-local commits not on upstream | **VIOLATION** | `git -C src/app/scaffold log --oneline --all --not --remotes` → `7760ee2 docs(scaffold): ticket TextEntryFieldWidget light-mode contrast for GCS chat phase 2`. `git ls-remote origin` (all 22 refs): no ref tip equals 7760ee2; remote `refs/heads/develop` tip = `a968b88c` (not even a local object). Local-only commit `7760ee2` (2026-09-19, "Super Genius <ken+git@gnus.ai>") adds exactly one file: `.planning/workstreams/scaffold/TICKET-text-entry-field-light-mode-contrast.md` (39 lines, docs only — **no `lib/` code touched**). It is ahead 1 of local `origin/develop` (6016e3f) and the parent pins it → a fresh clone cannot fetch this SHA. Also surfaced: `stash@{0}` (2026-08-17, `63d14c5`) holds WIP edits relativizing the scaffold's `CMakeLists.txt` (`WORKING_DIRECTORY .`, `ENGINE_SCRIPT "engine.py"`, etc.) — a dormant integrate-into attempt made inside this checkout (see 1.4b). |
| 1.4b | Stash hygiene (subset of `--all`) | **SMELL** | `git -C src/app/scaffold show 63d14c5 --stat` → `CMakeLists.txt | 34 +++---` — unpushed/uncommitted WIP editing scaffold build config, stashed in the submodule's git dir since 2026-08-17. Tree is clean today; the stash is the residue of an abandoned consumer-accommodation edit. GCS later solved the same problem correctly consumer-side (cache vars set before `add_subdirectory`, src/app/CMakeLists.txt:17-20 "Pitfall U1"). |
| 1.5 | Cross-check vs sibling checkout | **CLEAN** (informational) | `git -C ../apps/genius-ai-boss/frontend/scaffold rev-parse HEAD` → `8cb2a977...` = remote branch `refs/heads/gsd/phase-09-scaffold-decoupling` tip (remote-reachable). Consumers pin different upstream SHAs — fine. The finding is the contrast: sibling's pin is fetchable, GCS's is not (counted in 1.4). |
| 1.6 | Parent pointer-bump history | **SMELL** | `git log --oneline -- src/app/scaffold` → `3d9ebab docs(02-04): complete space/room create-edit dialog plan`, `26e6189 chore(app): bump scaffold submodule to latest develop (6016e3f)`, `82d30c8 chore(scaffold): bump pin to ef16a0c (develop; --api-specs-dir fix)`, `dcb02ee wip for app-restructure`. `git show 3d9ebab --submodule=log` → `Submodule src/app/scaffold 6016e3f..7760ee2` — an app **docs** commit silently bundled the bump to the unpushed ticket SHA. The other two bumps are clean upstream pulls (82d30c8 corresponds to the upstream-pushed `--api-specs-dir` fix documented in the reference's `.planning/quick/260820-g1k-*`). |
| 1.7 | Authorship sanity | **CLEAN** (caveat) | `git -C src/app/scaffold log --format='%h %an %s' -15` → all "Super Genius" — the same org identity maintains the scaffold repo itself, so authorship cannot distinguish GCS from upstream; the distinguishing signal is remote-reachability (1.4). Only the unpushed 7760ee2 has a GCS-flavored subject ("for GCS chat phase 2"). |

### Task 2 — Derives-not-integrates in GCS app code

| # | Check | Verdict | Evidence |
|---|-------|---------|----------|
| 2.1 | Full scaffold import inventory | **CLEAN** | `grep -rn "package:frontend_scaffold" src/app/lib/ src/app/test/` → 36 imports across 8 lib files + 5 test files (re-inventory: `room_rail.dart` is the "+1" vs planning). Every import is a deep import of `components/` or `theme/` — the documented convention per `src/app/scaffold/CLAUDE.md` ("lib/ is flat… consumers deep-import"). All files import/compose; none do anything else. |
| 2.2 | Fork/copy detection | **CLEAN** | `grep -rn "^class Scaffold" src/app/lib/ --include="*.dart"` → empty; `grep -rn "extends Scaffold\|with Scaffold\|implements Scaffold"` → empty. App widgets are app-named (`shell/gcs_shell.dart`, `shell/room_rail.dart`, `shell/space_room_dialog.dart`, `theme/gcs_theme.dart`) and compose scaffold atoms — that IS derive. No mirrored `scaffold_*` filenames in app lib/. |
| 2.3 | Generated-output hygiene | **SMELL** | `git check-ignore -v src/app/lib/generated/chat/chat_message_bubble.dart` → exit 1 (not ignored); `git ls-files src/app/lib/generated/` → 15 tracked files (12 chat + 3 proto). Upstream convention and the reference gitignore generated output (see 3.A4). Mitigations present: src/app/CMakeLists.txt:20 documents it as deliberate (`"GCS generated composite output (committed)"`) and a full codegen target exists — but `src/app/README.md` is the untouched Flutter template with zero regeneration instructions. Drift risk: hand-edits to `lib/generated/*.dart` would be invisible. |
| 2.4 | Template-first authorship / no write into submodule | **CLEAN** | `ls src/app/templates/components/` → 16 app-owned files (4 families x widget+cubit+state jinja2 + vars.json). src/app/CMakeLists.txt sets `TEMPLATES_DIR`/`GENERATED_DIR` before `add_subdirectory(scaffold)` (lines 17-24) and renders with `--template-dir app/templates` FIRST, then `scaffold/templates` (first-match-wins). `grep -rn "app/scaffold" src/app/CMakeLists.txt src/app/templates/` → empty — no build rule or script writes into the submodule; all submodule references are reads (templates, engine, add_subdirectory). |
| 2.5 | Reverse-direction reach (state layer) | **CLEAN** | `grep -rn "scaffold" src/app/lib/cubits/ src/app/lib/main.dart` → only a doc comment in main.dart:3 ("registers the scaffold token themes"). Cubits (`composer`, `message_flow`, `rail`, `session`) have zero scaffold references. |

### Task 3 — Reference comparison + FFI boundary

| # | Check | Verdict | Evidence |
|---|-------|---------|----------|
| 3.A1 | Reference wiring shape | **CLEAN** (equivalent) | Reference is workspace-style: `frontend/apps/touch-pos-system/pubspec.yaml:20-21` and `frontend/generated/consumer_test/pubspec.yaml:12-13` carry `frontend_scaffold: { path: ../../scaffold }`. GCS: `src/app/pubspec.yaml:44-45` → `frontend_scaffold: { path: scaffold }`. Same path-dependency consumption shape, different directory depth. `.gitmodules`: `src/app/scaffold -> ../openapi-client-scaffold.git`. |
| 3.A2 | Reference import/wrapper convention | **CLEAN** (equivalent) | Reference deep-imports identically (`apps/touch-pos-system/lib/theme/touch_pos_theme.dart:2-3` imports `scaffold_dimens.dart`, `scaffold_palette.dart`). Its per-app widget files are themselves generated output (`apps/admin/lib/widgets/scaffold_card.dart` header: "Generated from card.dart.jinja2 -- do not edit by hand", importing scaffold atoms). GCS renders app-authored composite templates (`chat_message_*.jinja2`) into `lib/generated/` — both are codegen derive; GCS authors its own composite templates where the reference re-renders the scaffold's component templates. Style difference only. |
| 3.A3 | Derive-don't-integrate rule documented in reference | **CLEAN** (found, cited) | (a) `apps/genius-ai-boss/.planning/quick/260815-okn-bifurcate-frontend-templates-workstream-/SUMMARY.md` — the 2026-08-16 bifurcation record: "UI widget ownership lives in the frontend/scaffold submodule… frontend-templates and touch-pos consume it" (decision D-v2-08). (b) `src/app/scaffold/CLAUDE.md` (same upstream doc both repos): "It is consumed as a git submodule by three repos… treat anything under lib/… as a public contract"; committed widget families are Jinja2 output — edit templates, never the .dart. (c) `apps/genius-ai-boss/.planning/quick/260820-g1k-fix-scaffold-api-specs-dir/260820-g1k-SUMMARY.md` — the sanctioned upstream-first flow in action: the `--api-specs-dir` fix was made in the scaffold repo on `develop` (ef16a0c), verified (`flutter test` 273/273), pushed, and the parent gitlink bumped — the same upstream SHA GCS later pulled in commit `82d30c8`. |
| 3.A4 | Consumption-shape deviations vs reference | **SMELL** (one) | (1) Generated output: reference NEVER commits it — `frontend/apps/admin/.gitignore:47-50` ignores `lib/widgets/*` ("Scaffold-generated composable widgets (regenerated by ninja generate_all_components)") un-ignoring only hand-written `error_boundary.dart`/`toast.dart`; top-level `frontend/.gitignore:1-5` ignores `generated/*` except a test fixture. GCS commits 15 generated files (see 2.3) — SMELL. (2) Layout: single flat app vs workspace apps/packages — CLEAN style difference. (3) App-authored composite templates vs re-rendered scaffold templates — CLEAN, both derive. |
| 3.B5 | App reaching past FFI boundary | **CLEAN** | `grep -rn "SuperGenius\|GlobalDB" src/app/ --include="*.dart" | grep -v build | grep -v gcs_bindings_generated` → zero matches (exit 1). `dart:ffi` appears only in the sanctioned pair: `src/app/lib/gcs_bindings_generated.dart` + `src/app/lib/cubits/session_cubit.dart` — matches the planning baseline exactly. |
| 3.B6 | C++ core depending on Flutter/UI | **CLEAN** | `grep -rln "flutter\|Flutter" src/lib/ src/proto/` → empty (exit 1). Vendored Dart native API headers under `src/ffi/` (`dart_api.h`, `dart_api_dl.{c,h}`, `dart_native_api.h`, `dart_version.h`) are the FFI boundary itself — expected, not flagged, per plan. |

## Overall Conclusion

**GCS derives from the scaffold; it has NOT repeated the genius-ai-boss integrate-into mistake.** The architecture-level contract holds completely: zero modifications to any file under `src/app/scaffold` (clean tree), zero forked or re-declared scaffold widgets in app code, zero build rules or scripts that write into the submodule, all consumption via deep imports and composition of scaffold atoms, app theming built from scaffold tokens, app-owned Jinja2 templates driving codegen into app space, and a clean FFI boundary in both directions. Where the scaffold genuinely needed to change for a consumer (`--api-specs-dir`), the change went through the scaffold repo, was verified and pushed upstream, and the parent merely bumped its pin — the exact flow the reference documents. The genius-ai-boss failure mode (backend logic integrated into scaffolds and later having to be corrected) has no code-level counterpart here.

The one VIOLATION is procedural, not architectural: commit `7760ee2` — a 39-line docs-only ticket (`.planning/workstreams/scaffold/TICKET-text-entry-field-light-mode-contrast.md`) filed from the GCS submodule checkout for a GCS need — was never pushed upstream, yet the parent pins it (bundled into app docs commit `3d9ebab`). Filing the ticket in the scaffold's own planning structure is arguably the sanctioned consumer-to-library channel, but pinning an unpushed SHA breaks fresh-clone reproducibility (`git submodule update` cannot fetch it). Secondary findings: a dormant 2026-08-17 stash holding WIP edits to the scaffold's CMakeLists.txt (an abandoned integrate-into attempt; the problem was later solved correctly consumer-side via cache vars), the submodule riding a local `develop` branch instead of a detached pin, and committed generated output with no regeneration docs — both consumers' conventions differ here, and the reference never commits generated files.

## Recommended Fixes (NOT applied — user decision)

1. **[VIOLATION 1.4] Unpushed pin `7760ee2`:** either push the commit from the scaffold repo to its remote (e.g. push `develop` or a `ticket/…` branch and advance the remote tip past it), or re-pin the parent to a remote-reachable SHA (`6016e3f`) and carry the ticket as a scaffold-side task. Until then, fresh clones of GCS cannot initialize `src/app/scaffold`.
2. **[SMELL 1.3] Branch checkout in submodule:** return the submodule to detached HEAD at the pinned SHA (standard submodule discipline) so `git submodule update` is deterministic.
3. **[SMELL 1.4b] Stale stash in submodule:** inspect `stash@{0}` (CMakeLists.txt path-relativization WIP, 2026-08-17) and drop it if obsolete — the consumer-side cache-var solution in src/app/CMakeLists.txt:17-20 superseded it. Do not pop it onto the scaffold tree.
4. **[SMELL 2.3 / 3.A4] Committed generated output:** either (a) keep committing and add regeneration instructions to `src/app/README.md` (the CMake target already exists), or (b) match the reference and gitignore `lib/generated/` (requires CI/CI-local regen step). Current state is intentional per src/app/CMakeLists.txt:20 but undocumented for developers.
5. **[SMELL 1.6] Pointer-bump hygiene:** future submodule bumps should be standalone `chore(scaffold): bump pin to <sha>` commits (like `26e6189`/`82d30c8`), never bundled into app-feature/docs commits — bundling is how an unpushed SHA slipped through review.

## Post-Audit Cleanliness Gate

```
$ git status --porcelain          (repo root — run after SUMMARY write; see Self-Check below)
$ git -C src/app/scaffold status --porcelain
(empty — verified in check 1.1 and re-verified below)
```

Read-only contract held: no `fetch`/`pull` was run in the submodule (`ls-remote` only); no file under `src/`, `lib/`, `templates/`, or any submodule path was modified.

## Self-Check: PASSED

- SUMMARY file exists at `.planning/quick/260922-gev-audit-frontend-scaffold-separation-of-co/260922-gev-SUMMARY.md` — FOUND
- All 18 check rows present with verdicts (Task 1: 8 rows incl. 1.4b, Task 2: 5 rows, Task 3: 6 rows) — FOUND
- Plan verify gates: Task 1 `TREE_CLEAN` (porcelain empty) — PASS; Task 2 import count 36 — PASS; Task 3 verdict-keyword grep — PASS (this file)
- No per-task code commits required (read-only plan); docs commit is the orchestrator's per constraints
