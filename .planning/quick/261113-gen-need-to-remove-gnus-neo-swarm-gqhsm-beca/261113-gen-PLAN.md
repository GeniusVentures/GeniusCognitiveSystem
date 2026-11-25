---
phase: 261113-gen-need-to-remove-gnus-neo-swarm-gqhsm-beca
plan: 01
type: execute
wave: 1
depends_on: []
files_modified:
  - GNUS-NEO-SWARM/.gitmodules      # GQHSM section removed; "build" section stays
  - GNUS-NEO-SWARM/GQHSM            # DELETED: gitlink (mode 160000) + working tree
  - GNUS-NEO-SWARM                  # parent-repo gitlink bump (new submodule commit)
  - .planning/SUBREPOS.md           # 3 stale GQHSM rows removed
autonomous: true
must_haves:
  truths:
    - "GNUS-NEO-SWARM has zero trace of GQHSM as a submodule: no [submodule \"GQHSM\"] section in its .gitmodules, no 160000 gitlink in its index, no GQHSM/ working tree, no submodule.GQHSM config entries, no GQHSM dir in .git/modules metadata"
    - "The [submodule \"build\"] submodule inside GNUS-NEO-SWARM is completely untouched (section, gitlink, worktree, config)"
    - "The removal commit inside GNUS-NEO-SWARM is reachable on branch gsd/quick-remove-gqhsm (repo was at detached HEAD 4371205)"
    - "The parent repo HEAD commit records the new GNUS-NEO-SWARM gitlink and .planning/SUBREPOS.md no longer lists any GQHSM row"
    - "No push/fetch/pull ran, and the 5 unrelated dirty paths in the parent repo remain uncommitted"
  artifacts:
    - path: "GNUS-NEO-SWARM/.gitmodules"
      provides: "submodule registry containing only the build submodule"
      contains: "submodule \"build\""
    - path: ".planning/SUBREPOS.md"
      provides: "live submodule map without GQHSM rows, build row retained"
    - path: "GNUS-NEO-SWARM commit on branch gsd/quick-remove-gqhsm"
      provides: "atomic GQHSM submodule removal (gitlink deletion + .gitmodules edit)"
  key_links:
    - from: "GeniusCognitiveSystem index (gitlink)"
      to: "GNUS-NEO-SWARM commit on gsd/quick-remove-gqhsm"
      via: "parent stages GNUS-NEO-SWARM pathspec; staged SHA must equal Task 1's recorded rev-parse HEAD"
      pattern: "Subproject commit"
    - from: ".planning/SUBREPOS.md table rows"
      to: "actual submodule layout"
      via: "table must match reality: no GNUS-NEO-SWARM/GQHSM rows, GNUS-NEO-SWARM/build row remains"
      pattern: "GNUS-NEO-SWARM/GQHSM"
---

<objective>
Remove the nested `GQHSM` submodule from inside `GNUS-NEO-SWARM` and record the removal in the parent repo (GeniusCognitiveSystem).

Why: `GNUS-NEO-SWARM/GQHSM` is the old C# editor/library (README, StateProto, qf4net). The final C++ executor of GQHSM is the new `Super-Genius/GQHSM-CPP` repo, so this copy is retired. Adding GQHSM-CPP as a new submodule is OUT OF SCOPE — this plan only removes.

Two commits total, in two different repos:
1. Inside `GNUS-NEO-SWARM` (on a new branch — the repo sits at detached HEAD): atomic removal of the GQHSM gitlink, its .gitmodules section, working tree, and git metadata.
2. In the parent repo: gitlink bump for `GNUS-NEO-SWARM` + removal of 3 stale rows from `.planning/SUBREPOS.md`.

Output: two local commits (one per repo) and a clean GQHSM-free submodule layout.
</objective>

<execution_context>
@$HOME/.claude/get-shit-done/workflows/execute-plan.md
@$HOME/.claude/get-shit-done/templates/summary.md
</execution_context>

<context>
@.planning/SUBREPOS.md
@.planning/STATE.md

<verified_ground_truth>
All facts below were verified against the live repo at planning time. Re-check the preflight items, but do not re-derive the rest.

- `GNUS-NEO-SWARM` is at detached HEAD `4371205` ("Updating build submodule"), working tree clean. Its `main` and `origin/main` are 3 ahead / 26 behind HEAD — diverged; do NOT commit onto main.
- `GNUS-NEO-SWARM/.gitmodules` has exactly TWO entries: `[submodule "build"]` (path `build`, url `../cmaketemplate.git`) and `[submodule "GQHSM"]` (path `GQHSM`, url `../../Super-Genius/GQHSM`). After removal the FILE MUST STILL EXIST with the build section.
- Gitlink in GNUS-NEO-SWARM index: `160000 cbbcac2f126a48d4f488c6110facec6da8c2782c 0 GQHSM`.
- GNUS-NEO-SWARM local config has `submodule.GQHSM.active true` and `submodule.GQHSM.url git@github.com:Super-Genius/GQHSM` (plus the same pair for `build`) — `git submodule deinit` clears the GQHSM pair.
- The nested GQHSM git dir is NOT under the working tree. It lives at the GeniusNetwork superproject level:
  `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/.git/modules/GeniusCognitiveSystem/modules/GNUS-NEO-SWARM/modules/GQHSM`
  (confirmed by `git -C GNUS-NEO-SWARM rev-parse --git-dir` → `.../GeniusNetwork/.git/modules/GeniusCognitiveSystem/modules/GNUS-NEO-SWARM`). A sibling `modules/build` dir exists and must survive.
- Parent repo (GeniusCognitiveSystem) `.git/config` has NO `submodule.GQHSM.*` entries — nothing to clean there. Parent is on branch `gsd/phase-04.1-room-session-service`.
- No CMakeLists.txt, *.cmake, or *.txt outside `GNUS-NEO-SWARM/GQHSM/` references GQHSM — no build fallout.
- Parent dirty paths that must remain untouched and unstaged:
  `.idea/editor.xml` (M), `.idea/codeStyles/` (??), `.planning/workstreams/app/phases/01-foundation/01-VERIFICATION.md` (M), `.planning/workstreams/app/phases/02-spaces-rooms/02-VERIFICATION.md` (M), `.planning/workstreams/system/MILESTONES.md` (M)
- Historical planning docs mentioning GQHSM (`.planning/intel/`, `.planning/workstreams/system/*`, `GNUS-NEO-SWARM/.planning/`, `GNUS-NEO-SWARM/gnus-poc/.planning.consolidated/`) are records of past state — NEVER rewrite them.
</verified_ground_truth>

<execution_constraints>
- NEVER push, fetch, or pull in either repo. Local commits only.
- The executor runs WITHOUT worktree isolation (this task must commit across a submodule boundary).
- Every Bash call starts a fresh shell at the workspace root — always use `git -C <path>` or absolute paths; never rely on cwd.
- Stage files by exact pathspec only. The stage-allowlist gates in both tasks are hard gates: if anything unexpected appears staged, unstage it before committing.
</execution_constraints>
</context>

<tasks>

<task type="auto">
  <name>Task 1: Remove nested GQHSM submodule inside GNUS-NEO-SWARM (branch, deinit, rm, purge metadata, atomic commit)</name>
  <files>GNUS-NEO-SWARM/.gitmodules, GNUS-NEO-SWARM/GQHSM (deleted)</files>
  <action>
  All commands run from `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/GeniusCognitiveSystem` via `git -C GNUS-NEO-SWARM ...`.

  1. Preflight (abort and report if either fails): `git -C GNUS-NEO-SWARM status --porcelain` must be empty, and `git -C GNUS-NEO-SWARM rev-parse HEAD` must start with `4371205`.
  2. Create the rescue branch FIRST so the removal commit is reachable (repo is at detached HEAD; do not use the diverged `main`): `git -C GNUS-NEO-SWARM switch -c gsd/quick-remove-gqhsm`.
  3. `git -C GNUS-NEO-SWARM submodule deinit -f GQHSM` — empties the GQHSM working tree and removes the two `submodule.GQHSM.*` entries from that repo's local config. The `build` submodule must be untouched by this step.
  4. `git -C GNUS-NEO-SWARM rm -f GQHSM` — deletes the `160000` gitlink from the index and removes the `[submodule "GQHSM"]` section from `GNUS-NEO-SWARM/.gitmodules`. The `.gitmodules` file must still exist afterwards containing the `build` section.
  5. Purge the orphaned git metadata directory: `rm -rf /Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/.git/modules/GeniusCognitiveSystem/modules/GNUS-NEO-SWARM/modules/GQHSM`. If that path does not exist, derive it as `$(git -C GNUS-NEO-SWARM rev-parse --git-dir)/modules/GQHSM` and remove that instead — never remove the sibling `modules/build`.
  6. Stage gate: `git -C GNUS-NEO-SWARM diff --cached --name-status` must list EXACTLY two entries — `M .gitmodules` and `D GQHSM`. Anything else: `git -C GNUS-NEO-SWARM reset` and stop.
  7. Commit atomically: `git -C GNUS-NEO-SWARM commit -m 'chore: remove GQHSM submodule (superseded by Super-Genius/GQHSM-CPP)'`.
  8. Record the new SHA for Task 2: `git -C GNUS-NEO-SWARM rev-parse HEAD` (write it into the SUMMARY notes).
  9. No network operations in this task — no fetch, no push.
  </action>
  <verify>
    <automated>cd /Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/GeniusCognitiveSystem && test ! -e GNUS-NEO-SWARM/GQHSM && git -C GNUS-NEO-SWARM ls-files -s GQHSM | wc -l | grep -qx 0 && grep -v '^#' GNUS-NEO-SWARM/.gitmodules | grep -c GQHSM | grep -qx 0 && grep -q 'submodule "build"' GNUS-NEO-SWARM/.gitmodules && git -C GNUS-NEO-SWARM ls-files -s build | grep -q '^160000' && git -C GNUS-NEO-SWARM config --local --get-regexp '^submodule\.GQHSM' | wc -l | grep -qx 0 && git -C GNUS-NEO-SWARM status --porcelain | wc -l | grep -qx 0 && git -C GNUS-NEO-SWARM branch --show-current | grep -qx gsd/quick-remove-gqhsm && git -C GNUS-NEO-SWARM show --name-status --format= HEAD | wc -l | grep -qx 2 && test ! -e /Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/.git/modules/GeniusCognitiveSystem/modules/GNUS-NEO-SWARM/modules/GQHSM && echo TASK1-VERIFIED</automated>
  </verify>
  <done>
  GQHSM fully gone from GNUS-NEO-SWARM (gitlink, .gitmodules section, worktree, config entries, .git/modules metadata); build submodule intact; removal commit is the single HEAD commit on branch gsd/quick-remove-gqhsm touching exactly .gitmodules + GQHSM; tree clean; new SHA recorded.
  </done>
</task>

<task type="auto">
  <name>Task 2: Parent repo — gitlink bump + SUBREPOS.md cleanup in a separate scoped commit</name>
  <files>.planning/SUBREPOS.md, GNUS-NEO-SWARM (gitlink)</files>
  <action>
  Runs in the parent repo `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/GeniusCognitiveSystem` on its current branch `gsd/phase-04.1-room-session-service`.

  1. Edit `.planning/SUBREPOS.md`: delete EXACTLY the three table rows `GNUS-NEO-SWARM/GQHSM/`, `GNUS-NEO-SWARM/GQHSM/StateProto/`, `GNUS-NEO-SWARM/GQHSM/qf4net/`. Leave everything else untouched — especially the `GNUS-NEO-SWARM/build/` row, the `GNUS-NEO-SWARM/` row, and the footer. Historical docs elsewhere are out of bounds (see verified_ground_truth).
  2. After Task 1, parent `git status` shows `modified: GNUS-NEO-SWARM (new commits)` — expected. Stage exactly two pathspecs: `git add GNUS-NEO-SWARM .planning/SUBREPOS.md`. Nothing else, ever — the 5 unrelated dirty paths must stay unstaged.
  3. Stage gate 1: `git diff --cached --name-status` must list EXACTLY `M .planning/SUBREPOS.md` and `M GNUS-NEO-SWARM`. Anything else: unstage the extras and stop.
  4. Stage gate 2: `git diff --cached GNUS-NEO-SWARM` must show `-Subproject commit 4371205...` and `+Subproject commit <SHA>` where `<SHA>` equals the SHA recorded in Task 1 step 8. Mismatch means the submodule moved again — stop and report.
  5. Commit: `git commit -m 'chore: drop GQHSM submodule from GNUS-NEO-SWARM (superseded by GQHSM-CPP)'`. Do NOT push.
  </action>
  <verify>
    <automated>cd /Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/GeniusCognitiveSystem && git show --name-status --format= HEAD | wc -l | grep -qx 2 && git show --name-status --format= HEAD | grep -q SUBREPOS.md && git show --name-status --format= HEAD | grep -q GNUS-NEO-SWARM && grep -v '^#' .planning/SUBREPOS.md | grep -c 'GNUS-NEO-SWARM/GQHSM' | grep -qx 0 && grep -q 'GNUS-NEO-SWARM/build' .planning/SUBREPOS.md && git status --porcelain | grep -c GNUS-NEO-SWARM | grep -qx 0 && git status --porcelain | grep -q '.idea/editor.xml' && git status --porcelain | grep -q 'MILESTONES.md' && echo TASK2-VERIFIED</automated>
  </verify>
  <done>
  Parent HEAD commit contains only the GNUS-NEO-SWARM gitlink bump and the SUBREPOS.md row removals; staged gitlink equals Task 1's SHA; the 5 unrelated dirty paths remain uncommitted; nothing pushed.
  </done>
</task>

</tasks>

<threat_model>
## Trust Boundaries

| Boundary | Description |
|----------|-------------|
| (none) | Purely local git operations in two nested repos; no network input, no untrusted data, no application code touched |

## STRIDE Threat Register

| Threat ID | Category | Component | Disposition | Mitigation Plan |
|-----------|----------|-----------|-------------|-----------------|
| T-q261113-01 | Tampering | staged index (both repos) | mitigate | Hard stage gates: `git diff --cached --name-status` must equal exactly the expected 2 paths in each repo before committing |
| T-q261113-02 | Tampering | parent repo dirty tree | mitigate | Exact-pathspec `git add`; post-commit `git status` must still show the 5 unrelated paths unstaged |
| T-q261113-03 | Repudiation | detached-HEAD submodule history | mitigate | Branch `gsd/quick-remove-gqhsm` created before the commit so the removal stays reachable; SHA recorded in SUMMARY |
| T-q261113-04 | Tampering | sibling `build` submodule | mitigate | Verify build gitlink (`160000`), `.gitmodules` section, and `modules/build` metadata survive untouched |
| T-q261113-SC | Tampering | supply chain | accept | No package installs and no network operations at all (fetch/push forbidden by task actions); nothing to verify upstream |
</threat_model>

<verification>
Overall phase checks (after both tasks):

1. `git -C GNUS-NEO-SWARM log --oneline -1` shows the removal commit on branch `gsd/quick-remove-gqhsm`; `git -C GNUS-NEO-SWARM submodule status` lists only `build` or nothing.
2. `git log --oneline -2` in the parent shows the scoped removal commit; `git show --stat HEAD` touches only `GNUS-NEO-SWARM` and `.planning/SUBREPOS.md`.
3. `git remote -v` unchanged in both repos and no remote refs advanced: `git -C GNUS-NEO-SWARM log --oneline origin/main..HEAD` shows only local work (nothing was pushed).
4. Both TASK-VERIFIED chains from the tasks pass when re-run.
</verification>

<success_criteria>
- GNUS-NEO-SWARM/.gitmodules has no GQHSM section and still has the build section
- `git -C GNUS-NEO-SWARM ls-files -s GQHSM` returns nothing; `GNUS-NEO-SWARM/GQHSM/` does not exist; `git -C GNUS-NEO-SWARM status` clean
- GQHSM metadata dir gone from `.git/modules/.../GNUS-NEO-SWARM/modules/`; `modules/build` still present
- Removal commit reachable on `gsd/quick-remove-gqhsm`; parent gitlink bumped to that SHA in a commit that also removes the 3 SUBREPOS.md rows
- Zero pushes; 5 unrelated parent dirty paths untouched
</success_criteria>

<output>
Create `.planning/quick/261113-gen-need-to-remove-gnus-neo-swarm-gqhsm-beca/261113-gen-SUMMARY.md` when done. Record in it: the submodule removal commit SHA, the branch name, the parent commit SHA, and confirmation that no push occurred.
</output>
