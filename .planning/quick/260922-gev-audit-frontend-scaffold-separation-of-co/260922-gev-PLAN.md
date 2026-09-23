---
phase: 260922-gev-audit-frontend-scaffold-separation-of-co
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - .planning/quick/260922-gev-audit-frontend-scaffold-separation-of-co/260922-gev-SUMMARY.md
autonomous: true
read_only: true
must_haves:
  truths:
    - "Every submodule-integrity check (dirty tree, local commits, detached pointer, parent pointer-bump history) has a verdict with command-output evidence"
    - "Every scaffold import in GCS app code is classified as derive (import/extend/compose) or violation, with file:line evidence"
    - "GCS consumption pattern is compared against the ../apps/genius-ai-boss reference, with deviations listed or CLEAN declared"
    - "FFI boundary swept in both directions (app→core includes, core→Flutter deps) with verdicts"
    - "Zero source files modified — only the findings SUMMARY was created"
  artifacts:
    - path: ".planning/quick/260922-gev-audit-frontend-scaffold-separation-of-co/260922-gev-SUMMARY.md"
      provides: "Verdict table (CLEAN / VIOLATION / SMELL per check) with evidence excerpts"
  key_links: []
---

<objective>
READ-ONLY audit of separation of concerns between the GCS app frontend (`src/app/lib/`) and the frontend widget-library scaffold submodule (`src/app/scaffold` = `../openapi-client-scaffold.git`, Dart package `frontend_scaffold`).

The rule under audit: the app always DERIVES FROM the scaffold (import / extend / compose / regenerate via templates), never INTEGRATES INTO it (never edits files inside the submodule, never forks its widgets, never forces upstream changes to accommodate app code). Reference implementation: `../apps/genius-ai-boss/frontend` consuming its own `frontend/scaffold` (same upstream repo). GCS must not repeat the mistake AI made in genius-ai-boss where backend code was integrated into scaffolds and had to be corrected.

Purpose: Verify the decoupling contract holds before Phase 3 messaging work builds further on it.
Output: `.planning/quick/260922-gev-audit-frontend-scaffold-separation-of-co/260922-gev-SUMMARY.md` — findings only. Violations are REPORTED, not fixed. Fixing is a user decision.
</objective>

<execution_context>
@$HOME/.claude/get-shit-done/workflows/execute-plan.md
@$HOME/.claude/get-shit-done/templates/summary.md
</execution_context>

<context>
@src/app/scaffold/CLAUDE.md

<interfaces>
<!-- Facts pre-verified during planning. Executor uses these as baseline; re-run to confirm current state. -->

Wiring:
- .gitmodules: `src/app/scaffold` -> `../openapi-client-scaffold.git`
- src/app/pubspec.yaml line ~44: `frontend_scaffold: { path: scaffold }` (path dependency — the standard consumption shape)
- Baseline at planning time: `git submodule status src/app/scaffold` -> ` 7760ee2a92ded18ea5efe8b8ade494d0f1ceaad6 (v1.1-199-g7760ee2)` — no leading `+` (index in sync)

Scaffold contract (from src/app/scaffold/CLAUDE.md — upstream's own docs):
- `lib/` is flat, ALL public API; consumers deep-import (`package:frontend_scaffold/components/scaffold_badge.dart`). Deep imports are the DOCUMENTED convention, not a smell.
- Widget families `scaffold_{animated_display,formatted_value,image_placeholder,selection_indicator}_*` and `scaffold_{card,state_view,search_bar}*` are committed Jinja2 output — upstream edits go to `templates/components/*.jinja2`, never the .dart (past upstream drift: commit `ed8859b`).
- Consumer-space generated output is gitignored under `generated/` in the consuming repo.
- Scaffold widgets consume M3 `Theme.of(context)` only — no Riverpod, no app-specific theme.

GCS app structure (the derive mechanism):
- `src/app/templates/components/` — app-owned Jinja2 templates (drives scaffold codegen)
- `src/app/lib/generated/{chat,proto}/` — generated derived widgets (e.g. chat_message_bubble.dart composes scaffold_surface + scaffold_colors)
- `src/app/CMakeLists.txt` — codegen wiring
- Known scaffold importers in `src/app/lib/` (8 files at planning time): generated/chat/chat_message_{media,bubble,code_block,flow}.dart, shell/gcs_shell.dart, shell/space_room_dialog.dart, theme/gcs_theme.dart (+1 more — re-inventory)

Reference (../apps/genius-ai-boss/frontend): has `scaffold/`, `templates/`, `generated/`, `apps/`, `packages/`, `modules/`, `plugins/` — a workspace-style layout. NOTE: `frontend_scaffold` is NOT in its top-level pubspec.yaml; the wiring lives elsewhere (likely a workspace pubspec under apps//packages/) — executor must locate it.

FFI boundary baseline:
- `dart:ffi` in app code: src/app/lib/cubits/session_cubit.dart + src/app/lib/gcs_bindings_generated.dart (legitimate FFI consumption points)
- src/ffi/dart_api.h, dart_api_dl.{c,h}, dart_native_api.h, dart_version.h — vendored Dart native API headers; these ARE the FFI boundary itself, expected, not a violation
- No SuperGenius/GlobalDB matches in src/app/lib/ at planning time (verify including src/app/src/ and src/app/test/)

Every Bash call starts a FRESH shell at the repo root — always use `git -C <path>` and repo-relative paths; never rely on cwd. Never `cd` into the submodule.
</interfaces>
</context>

<tasks>

<task type="auto">
  <name>Task 1: Scaffold submodule integrity audit</name>
  <files>(read-only — no file modifications)</files>
  <action>
Run all checks read-only from the repo root using `git -C`. Record verdict + evidence for each:

1. Working-tree dirt: `git -C src/app/scaffold status --porcelain` — any output (modified/untracked) means GCS-local edits inside the submodule. Untracked `.planning/` scratch would be a SMELL; modified tracked files = VIOLATION.
2. Pointer sync: `git submodule status src/app/scaffold` — leading `+` means checked-out SHA differs from the SHA recorded in the parent index (uncommitted pointer move); leading space = in sync.
3. Detached vs branch tracking: `git -C src/app/scaffold branch -vv` and `git -C src/app/scaffold log --oneline -5` — submodules are normally detached HEAD; note what HEAD sits on.
4. GCS-local commits not on upstream: `git -C src/app/scaffold log --oneline --all --not --remotes` (commits reachable locally but not from any remote ref). Also get the true remote tip WITHOUT fetching (read-only): `git -C src/app/scaffold ls-remote origin` then compare `git -C src/app/scaffold rev-parse HEAD`. Any local-only commit = VIOLATION (GCS authored into the scaffold).
5. Cross-check against the sibling checkout of the same upstream: `git -C ../apps/genius-ai-boss/frontend/scaffold rev-parse HEAD` — divergence between the two consumers is informational (consumers pin different upstream SHAs), not itself a violation.
6. Parent pointer-bump history: `git log --oneline -- src/app/scaffold | head -20` — each bump should read as "update submodule pointer / pull upstream". A bump immediately following app-feature commits, or bumps authored to "make the build pass" after local edits, is a SMELL.
7. Authorship sanity: `git -C src/app/scaffold log --format='%h %an %s' -15` — recent commits should be upstream authors/maintainers, not GCS feature commits.

Baseline expectation from planning: clean tree, index in sync at 7760ee2. Deviations from that baseline are findings.
  </action>
  <verify>
    <automated>test -z "$(git -C src/app/scaffold status --porcelain)" && echo TREE_CLEAN || echo TREE_DIRTY</automated>
  </verify>
  <done>Each of the 7 checks has a verdict (CLEAN/VIOLATION/SMELL) with a command-output excerpt captured for the SUMMARY.</done>
</task>

<task type="auto">
  <name>Task 2: Derives-not-integrates audit of GCS app code</name>
  <files>(read-only — no file modifications)</files>
  <action>
Audit how app code consumes the scaffold. All greps read-only.

1. Full import inventory: `grep -rn "package:frontend_scaffold" src/app/lib/ src/app/test/ src/app/src/ 2>/dev/null` (src/app/src/ may not exist — tolerate). Classify each importing file: does it import/extend/compose scaffold widgets (CLEAN — deep imports are the documented convention per upstream CLAUDE.md), or does it do something else?
2. Fork/copy detection — app code re-declaring scaffold widgets instead of importing: `grep -rn "^class Scaffold" src/app/lib/ --include="*.dart"` and eyeball any widget files under src/app/lib/ that mirror scaffold filenames (`ls src/app/lib/shell/ src/app/lib/theme/ src/app/lib/generated/chat/`). A copied-and-modified scaffold widget = VIOLATION (fork-in-place); an app widget that wraps/composes scaffold atoms = CLEAN (that IS derive).
3. Generated-output hygiene: confirm `src/app/lib/generated/` is derived-not-committed: `git check-ignore -v src/app/lib/generated/chat/chat_message_bubble.dart`; also `git ls-files src/app/lib/generated/ | head` — if generated files are tracked in git, verdict SMELL (drift risk; upstream convention gitignores consumer generated output) and check whether regeneration instructions exist in src/app/README.md or CMakeLists.txt.
4. Template-first authorship: confirm the derive mechanism flows through app-owned templates: `ls src/app/templates/components/` and skim `src/app/CMakeLists.txt` for the codegen target that consumes them. App feature work should edit `src/app/templates/` (+ regenerate), never `src/app/lib/generated/*.dart` directly, and NEVER `src/app/scaffold/`. Check for any build rule or script that writes into `src/app/scaffold/`: `grep -rn "app/scaffold" src/app/CMakeLists.txt src/app/templates/ 2>/dev/null` — writes pointing INTO the submodule = VIOLATION.
5. Reverse-direction reach: `grep -rn "scaffold" src/app/lib/cubits/ src/app/lib/main.dart` — cubits (state layer) importing widget-library theme/components would be a layering SMELL (state should not consume widget internals beyond tokens).
  </action>
  <verify>
    <automated>grep -rn "package:frontend_scaffold" src/app/lib/ | wc -l</automated>
  </verify>
  <done>Every importing file classified; fork/copies ruled out or evidenced; generated-output hygiene verdict; no write-path into the submodule. All verdicts captured with file:line evidence.</done>
</task>

<task type="auto">
  <name>Task 3: Reference comparison (genius-ai-boss) + FFI boundary sweep + findings report</name>
  <files>.planning/quick/260922-gev-audit-frontend-scaffold-separation-of-co/260922-gev-SUMMARY.md</files>
  <action>
PART A — Reference comparison. Compare GCS against how ../apps/genius-ai-boss/frontend consumes the SAME upstream:
1. Locate the wiring: `grep -rn "frontend_scaffold" ../apps/genius-ai-boss/frontend --include="*.yaml" -l 2>/dev/null | head` (top-level pubspec does NOT reference it — it is a workspace-style layout with apps//packages/; find which pubspec carries the path dependency).
2. Sample its import pattern: `grep -rn "package:frontend_scaffold" ../apps/genius-ai-boss/frontend --include="*.dart" -l 2>/dev/null | head -10` — note whether it also deep-imports, and whether it wraps scaffold widgets in app-local wrapper components vs using them raw. Record the convention.
3. Find the derive-don't-integrate rule documented anywhere in the reference: check `../apps/genius-ai-boss/frontend/scaffold/CLAUDE.md`, `../apps/genius-ai-boss/CLAUDE.md`, READMEs, and `.planning/` dirs under ../apps/genius-ai-boss/ (search for the post-mortem of the backend-integrated-into-scaffold mistake: `grep -rln "scaffold" ../apps/genius-ai-boss/.planning/ 2>/dev/null | head` then read the most relevant hit). Cite what you find.
4. Diff the consumption shapes: reference vs GCS — path dependency vs other, wrapper conventions, templates/generated layout, gitignore hygiene. List deviations; classify each deviation VIOLATION (breaks derive-only), SMELL (works but risks coupling), or CLEAN (equivalent pattern, different style).

PART B — FFI boundary sweep (both directions, in this repo):
5. App must not reach past the FFI boundary: `grep -rn "SuperGenius\|GlobalDB" src/app/ --include="*.dart" 2>/dev/null | grep -v "^\s*src/app/build/" | grep -v gcs_bindings_generated` — expected: zero direct includes; the only sanctioned crossing is src/app/lib/gcs_bindings_generated.dart + `dart:ffi` in src/app/lib/cubits/session_cubit.dart.
6. C++ core must not depend on Flutter/UI: `grep -rln "flutter\|Flutter" src/lib/ src/proto/ 2>/dev/null` — expected empty. The vendored Dart native API headers under src/ffi/ (dart_api.h, dart_api_dl.c, dart_native_api.h, dart_version.h) ARE the boundary itself — verdict CLEAN by design, note as expected, do not flag.

PART C — Write the findings report to `.planning/quick/260922-gev-audit-frontend-scaffold-separation-of-co/260922-gev-SUMMARY.md`: a verdict table with columns Check | Verdict (CLEAN/VIOLATION/SMELL) | Evidence (command excerpt or file:line) covering ALL checks from Tasks 1-3, then a one-paragraph overall conclusion ("GCS derives from the scaffold / GCS has repeated the integrate-into mistake / mixed"). Violations get a recommended-fix NOTE but NO code is changed — fixes are the user's decision.

Final read-only proof: after writing the SUMMARY, run `git status --porcelain` at the repo root and `git -C src/app/scaffold status --porcelain` — the ONLY new/changed path must be the SUMMARY file (untracked is fine). Anything else = the audit broke its own contract; report it.
  </action>
  <verify>
    <automated>grep -c "Verdict\|CLEAN\|VIOLATION\|SMELL" .planning/quick/260922-gev-audit-frontend-scaffold-separation-of-co/260922-gev-SUMMARY.md</automated>
  </verify>
  <done>SUMMARY exists with a verdict + evidence row for every check in Tasks 1-3, an overall conclusion, and the post-audit git status proving zero source modifications (only the SUMMARY file added).</done>
</task>

</tasks>

<threat_model>
## Trust Boundaries

| Boundary | Description |
|----------|-------------|
| n/a | Read-only audit; no production code, config, or dependencies touched. Findings report written inside .planning/quick/ only. |

## STRIDE Threat Register

| Threat ID | Category | Component | Disposition | Mitigation Plan |
|-----------|----------|-----------|-------------|-----------------|
| T-260922-01 | Tampering | audited repos (GCS + submodule + reference) | mitigate | Read-only commands only (status/log/ls-remote/grep); `ls-remote` used instead of `fetch` so no local refs are mutated; post-audit `git status --porcelain` gate proves zero modifications |
| T-260922-02 | Repudiation | findings evidence | mitigate | Every verdict must carry a command-output excerpt or file:line citation so the user can independently reproduce |
</threat_model>

<verification>
- All 7 integrity checks (Task 1), all 5 consumption checks (Task 2), and all reference/FFI checks (Task 3) have verdicts with evidence in the SUMMARY.
- `git status --porcelain` at repo root shows only the new SUMMARY file; `git -C src/app/scaffold status --porcelain` is empty.
- No file under src/, lib/, templates/, or the submodule was modified.
</verification>

<success_criteria>
- Verdict table complete: every check labeled CLEAN / VIOLATION / SMELL with reproducible evidence.
- Overall conclusion answers the audit question directly: has GCS derived from the scaffold (like the reference intends) or integrated into it (the genius-ai-boss backend mistake)?
- Zero code changes; violations reported with recommended fixes left for the user to decide.
</success_criteria>

<output>
Create `.planning/quick/260922-gev-audit-frontend-scaffold-separation-of-co/260922-gev-SUMMARY.md` when done
</output>
