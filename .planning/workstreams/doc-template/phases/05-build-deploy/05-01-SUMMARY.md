---
phase: 05-build-deploy
plan: 01
subsystem: gendoc-template
tags: [build, deploy, mkdocs, cloudflare-pages, wrangler, bash, shell]
requires: []
provides: [BLD-01, BLD-02, BLD-03]
affects: [gendoc-template/scripts/build.sh, gendoc-template/wrangler.toml.template, gendoc-template/scripts/deploy.sh, gendoc-template/.gitignore]
tech-stack:
  added: []
  patterns: [pipeline-orchestration, token-substitution, environment-credentials, portable-bash, python3-yaml-config]
key-files:
  created:
    - gendoc-template/scripts/build.sh
    - gendoc-template/wrangler.toml.template
    - gendoc-template/scripts/deploy.sh
  modified:
    - gendoc-template/.gitignore
decisions:
  - "build.sh reads gendoc.yml from HOST_ROOT (not TEMPLATE_ROOT) — matching load_gendoc_config.py pattern rather than build_api_reference.sh pattern, ensuring the filled-out config lives in the host project"
  - "wrangler.toml is generated at deploy time from .template via python3 string .replace() — no sed -i for portability, and credentials stay in environment only"
  - "CF_API_TOKEN and CF_ACCOUNT_ID are passed to wrangler via inline env assignment (not export) — limiting credential exposure to the single deploy command"
  - "All three scripts use read_yaml() from build_api_reference.sh — single source of truth for YAML config parsing"
metrics:
  duration: 2 min
  completed_date: 2026-06-28
---

# Phase 5 Plan 1: Build/Deploy Scripts (build.sh, deploy.sh, wrangler.toml.template)

**One-liner:** Single-command build (Doxygen -> doxybook2 -> navigation -> MkDocs) and deploy (Wrangler -> Cloudflare Pages) scripts reading all config from gendoc.yml with credentials from environment only.

## Tasks Executed

| # | Name | Commit | Status |
|---|------|--------|--------|
| 1 | Create build.sh — full pipeline orchestrator | `8522161` | Complete |
| 2 | Create wrangler.toml.template — parameterized Cloudflare Pages config | `4ac2f8f` | Complete |
| 3 | Create deploy.sh — Cloudflare Pages deployment script | `f1f0686` | Complete |

## Requirements Satisfied

| ID | Description | Status |
|----|-------------|--------|
| BLD-01 | Single build script runs full pipeline (Doxygen -> doxybook2 -> navigation -> MkDocs) | Met |
| BLD-02 | Wrangler deploy script reads config from gendoc.yml, credentials from environment | Met |
| BLD-03 | macOS + Linux portability (no GNU-isms, no sed -i, python3 for YAML) | Met |

## Commit Details

### 8522161 — feat(05-build-deploy): add build.sh

- **File:** `gendoc-template/scripts/build.sh` (122 lines, executable)
- Path resolution: `BASH_SOURCE[0]` -> SCRIPT_DIR -> TEMPLATE_ROOT -> HOST_ROOT
- GENDOC_YML resolved from HOST_ROOT (matching `load_gendoc_config.py` pattern)
- Prerequisite validation: gendoc.yml, build_api_reference.sh, mkdocs.yml, mkdocs CLI
- read_yaml() function reused from build_api_reference.sh
- Pipeline: calls `build_api_reference.sh` (Doxygen -> doxybook2 -> navigation), then `mkdocs build`
- Supports `mkdocs.strict` flag from gendoc.yml
- Portable: no realpath/timeout/flock, no sed -i, python3 for YAML
- Fail-fast: set -euo pipefail with explicit exit code propagation

### 4ac2f8f — feat(05-build-deploy): add wrangler.toml.template + .gitignore

- **File:** `gendoc-template/wrangler.toml.template` (7 lines)
- Three tokens: `{{PAGES_PROJECT_NAME}}`, `{{COMPATIBILITY_DATE}}`, `{{SITE_DIR}}`
- Zero credentials in template — all values from gendoc.yml
- **File:** `gendoc-template/.gitignore` — added `wrangler.toml` entry
- Prevents accidental commit of generated Wrangler config with project-specific values

### f1f0686 — feat(05-build-deploy): add deploy.sh

- **File:** `gendoc-template/scripts/deploy.sh` (148 lines, executable)
- Path resolution identical to build.sh
- Prerequisite validation: gendoc.yml, wrangler.toml.template, wrangler CLI, CF_API_TOKEN, CF_ACCOUNT_ID
- Generates wrangler.toml via python3 .replace() token substitution
- Warnings if site directory doesn't exist (suggests running build.sh first)
- Deploys via `wrangler pages deploy` with inline env assignment (not export)
- Security: never echoes CF_API_TOKEN or CF_ACCOUNT_ID values; only checks existence via `-z`
- Portable: no GNU-isms, no sed -i, python3 for all substitution

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 2 - Missing Critical Functionality] Added wrangler.toml to .gitignore**
- **Found during:** Task 2 verification (check 9)
- **Issue:** `.gitignore` had `*.cf-override.*` pattern but not `wrangler.toml` — the generated config could be accidentally committed exposing project-specific values
- **Fix:** Added `wrangler.toml` entry to `.gitignore` with comment
- **Files modified:** `gendoc-template/.gitignore`
- **Commit:** `4ac2f8f`

## Threat Model Compliance

All STRIDE mitigations enforced:
- **T-05-01 (Information Disclosure):** deploy.sh validates CF_API_TOKEN and CF_ACCOUNT_ID with `-z` test only; values never echoed, logged, or written to disk; passed via inline env assignment (not export)
- **T-05-02 (Information Disclosure):** wrangler.toml.template contains zero secrets — only {{TOKEN}} placeholders; generated wrangler.toml is gitignored
- **T-05-03 (Tampering):** gendoc.yml is host-project-controlled — accepted risk
- **T-05-04 (Tampering):** wrangler.toml.template is committed in submodule — accepted risk
- **T-05-05 (Elevation of Privilege):** No sudo, no setuid in any script — accepted risk

## Threat Flags

None. All security surface is covered by the plan's threat model.

## Known Stubs

None. All three scripts are fully functional with no placeholder logic.

## Self-Check: PASSED

- [x] `gendoc-template/scripts/build.sh` exists, executable, 14/14 checks passed
- [x] `gendoc-template/wrangler.toml.template` exists, 9/9 checks passed
- [x] `gendoc-template/scripts/deploy.sh` exists, executable, 16/16 checks passed
- [x] Commit `8522161` verified in git log
- [x] Commit `4ac2f8f` verified in git log
- [x] Commit `f1f0686` verified in git log
- [x] No post-commit deletions detected
- [x] No untracked files remaining
