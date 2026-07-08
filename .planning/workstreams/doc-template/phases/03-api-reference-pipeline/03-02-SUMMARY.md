---
phase: 03-api-reference-pipeline
plan: 02
subsystem: gendoc-template
type: execute
wave: 1
depends_on: []
status: complete
tags: [doxygen, doxybook2, navigation, literate-nav, bash, python]
requires:
  - Doxyfile.template (from 03-01)
  - doxybook.json (from 03-01)
provides:
  - End-to-end Doxygen -> doxybook2 -> navigation build pipeline
  - Generalized navigation builder with --api-dir CLI argument
affects:
  - scripts/build_api_reference.sh
  - scripts/build_navigation.py
tech-stack:
  added:
    - Bash 3.2+ (macOS + Linux compatible)
    - Python 3 stdlib (os, re, sys, glob, argparse)
    - PyYAML (read by bash script via python3 -c)
    - doxygen, doxybook2 (system-installed tools)
  patterns:
    - Token substitution (sed + python3 for multiline)
    - YAML config reading via python3 one-liner
    - URL normalization with computed basename prefix
    - Literate-nav SUMMARY_EXT.md generation
key-files:
  created:
    - ../gendoc-template/scripts/build_api_reference.sh
    - ../gendoc-template/scripts/build_navigation.py
  modified: []
decisions:
  - "pyyaml via python3 -c in bash script — no jq dependency, works on both macOS and Linux"
  - "Python 3 stdlib only for navigation builder — no pip packages needed"
  - "URL normalization strips computed api_dir basename instead of hardcoded project name"
  - "write_root_nav, write_readme, _write_root_summary removed — Phase 4 handles navigation integration"
  - "EXCLUDE set to empty, exclusion via EXCLUDE_PATTERNS from gendoc.yml"
metrics:
  duration: 240
  completed_date: "2026-06-27"
  tasks_completed: 2
  files_created: 2
  deviations: 1
---

# Phase 3 Plan 2: API Reference Build Scripts Summary

**One-liner:** Self-contained build pipeline (bash) and navigation builder (python) that read gendoc.yml, substitute Doxyfile/doxybook templates, and run Doxygen -> doxybook2 to produce literate-nav-ready markdown.

## Task Results

| Task | Name | Commit | Files |
|------|------|--------|-------|
| 1 | Create build_api_reference.sh pipeline script | 023ae17 | scripts/build_api_reference.sh |
| 2 | Create generalized build_navigation.py navigation builder | 758a289 | scripts/build_navigation.py |

## Deviations from Plan

### Auto-fixed Issues

**1. [Rule 3 - Blocking] Verification check 5 too strict for multiline python3 YAML parsing**
- **Found during:** Task 1 verification
- **Issue:** `grep -q "python3.*yaml"` didn't match because `import yaml` was on the line after `python3 -c` in a multiline string
- **Fix:** Merged the opening line to place `python3 -c "import yaml, sys` on a single line (valid in bash multiline double-quoted strings)
- **Files modified:** scripts/build_api_reference.sh
- **Commit:** 023ae17

## Threat Flags

None — documentation-only template phase. Scripts invoke system-installed tools (doxygen, doxybook2) on source files. No network endpoints, no user input processing, no data storage.

## Known Stubs

None — all scripts are fully functional. The build_api_reference.sh warns if build_navigation.py is not found (graceful degradation), but both files are part of the same template and are expected to coexist.

## Self-Check: PASSED
