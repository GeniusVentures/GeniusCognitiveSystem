---
phase: 04-navigation-integration
plan: 01
subsystem: gendoc-template
tags: [navigation, literate-nav, SUMMARY.md, merge, build-pipeline]
requires: []
provides: [04-01-NAV-MERGE]
affects: [build_api_reference.sh, mkdocs.yml, build_navigation.py]
tech-stack:
  added: []
  patterns: [literate-nav, pipeline-orchestration, SUMMARY.md-merging]
key-files:
  created: []
  modified:
    - gendoc-template/scripts/build_navigation.py
    - gendoc-template/mkdocs.yml
    - gendoc-template/scripts/build_api_reference.sh
decisions:
  - "write_root_nav() produces SUMMARY_EXT.md via build_literate_nav() — reuses existing formatting logic rather than duplicating string building"
  - "HANDWRITTEN_DOCS_ABS already resolved by build_api_reference.sh (line 128) — no new variable needed"
  - "Missing SUMMARY.md is a soft warning, not a hard error — allows API-reference-only sites"
metrics:
  duration: 5 min
  completed_date: 2026-06-28
---

# Phase 4 Plan 1: Root Nav Merge for Hand-Written + API Reference

**One-liner:** Merges hand-written SUMMARY.md with generated API reference categories into a single SUMMARY_EXT.md for literate-nav, wired end-to-end through the build pipeline.

## Completion Checklist

- [x] `write_root_nav(docs_dir, api_dir)` function added to `build_navigation.py`
- [x] `--docs-dir` CLI argument registered (optional, backward compatible)
- [x] `mkdocs.yml` `nav_file` changed from `SUMMARY.md` to `SUMMARY_EXT.md`
- [x] `build_api_reference.sh` passes `--docs-dir "$HANDWRITTEN_DOCS_ABS"` to `build_navigation.py`
- [x] Merged SUMMARY_EXT.md includes hand-written sections and API Reference categories
- [x] Hand-written SUMMARY.md preserved as read-only input
- [x] Missing SUMMARY.md handled gracefully (warning, API-reference-only)
- [x] Categories without generated content are excluded from the root nav

## Commit History

| Task | Name | Commit | Files |
|------|------|--------|-------|
| 1 | Add write_root_nav() and --docs-dir | `3649778` | `scripts/build_navigation.py` |
| 2 | Wire into mkdocs.yml and build pipeline | `dd91330` | `mkdocs.yml`, `scripts/build_api_reference.sh` |

## Deviations from Plan

None — plan executed exactly as written.

## Verification Results

### Task 1: write_root_nav() function

- Import test: `write_root_nav` importable from `build_navigation` — PASS
- `--docs-dir` appears in `--help` output — PASS
- E2E with fake SUMMARY.md: merged output has correct format (`<!--nav-->`, 4-space indent, section headings, links) — PASS
- API Reference section from SUMMARY.md correctly skipped and replaced — PASS
- Missing SUMMARY.md: warning to stderr, API-reference-only output — PASS
- Categories without SUMMARY_EXT.md excluded (e.g., Modules, Pages when no content) — PASS

### Task 2: wire into mkdocs.yml and build pipeline

- `nav_file: SUMMARY_EXT.md` in `mkdocs.yml` — PASS
- `--docs-dir "$HANDWRITTEN_DOCS_ABS"` in `build_api_reference.sh` — PASS
- `HANDWRITTEN_DOCS_ABS` already resolved at line 128 (no new variable needed) — PASS
- Pipeline step ordering correct (category SUMMARY_EXT.md files exist before root nav call) — PASS

## Decisions Made

1. **Reuse `build_literate_nav()` for output formatting.** Instead of writing SUMMARY_EXT.md content manually, `write_root_nav()` builds an items list and passes it to the existing `build_literate_nav()` function. This ensures consistent formatting between category-level and root-level navigation files.

2. **Soft warning for missing SUMMARY.md.** The function prints a warning to stderr and proceeds with API-reference-only navigation, rather than treating it as a fatal error. This allows the template to be used on projects that don't yet have hand-written docs.

3. **Categories verified by file existence.** API Reference categories only appear in the root nav if `{api_dir}/{category}/SUMMARY_EXT.md` exists — a reliable signal that doxybook2 and the per-category nav builder produced content for that category.

## Threat Flags

None — all security surface covered by existing T-04-01 and T-04-02 threat register entries.

## Self-Check: PASSED

- [x] 04-01-SUMMARY.md exists
- [x] Commit `3649778` (Task 1) found in gendoc-template
- [x] Commit `dd91330` (Task 2) found in gendoc-template
