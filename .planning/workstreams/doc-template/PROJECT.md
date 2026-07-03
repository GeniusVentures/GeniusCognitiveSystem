# gendoc-template

## Core Value

A reusable, config-driven MkDocs documentation template that any GNUS C++ project can add as a git submodule to produce a complete documentation site — combining hand-written markdown with Doxygen-generated API reference, deployable to Cloudflare Pages via Wrangler.

## What This Is

A parameterized generalization of the `../documentation` reference implementation. Instead of hardcoding SuperGenius paths, the template accepts a config file pointing to the host project's hand-written docs directory and C++ source directory. It provides:
- MkDocs + Material theme configuration
- Doxygen config template for C++ API documentation
- Navigation builder that merges hand-written and generated docs
- Build scripts (local and Cloudflare Pages)
- Wrangler deployment configuration

## What This Is NOT

- NOT a documentation hosting service
- NOT specific to any one GNUS project
- NOT a replacement for the existing `../documentation` setup — it's a template derived from it

## Current Milestone: v0.1 Initial Template

**Goal:** Extract the `../documentation` pattern into a reusable, config-driven submodule.

**Target features:**
- Config file pointing to host project's hand-written docs dir and C++ source dir
- Generic Doxygen template (parameterized, not SuperGenius-specific)
- Navigation builder that merges hand-written and generated sources
- Submodule-ready: `git submodule add`, configure, build
- Cloudflare Pages deployment via Wrangler (script + config entry)
- Working example with a test project proving the template works

## Key Decisions

- **Phase 02:** MkDocs Material theme with GNUS brand colors (cyan-blue ramp), Doxygen CSS integration ported from reference, 5 JS integrations for nav/anchor/scrolling/toc/lightbox. Config-driven via gendoc.yml → load_gendoc_config.py hook. Python deps pinned: mkdocs==1.6.1, mkdocs-material==9.5.27, pymdown-extensions>=10.14, mkdocs-literate-nav==0.6.1, pyyaml>=6.0.
- **Phase 02:** `mkdocs build --strict` verified end-to-end with zero warnings — the configuration and theme produce a clean static site.

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition:**
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to Key Decisions
5. "What This Is" still accurate? → Update if drifted

**After each milestone:**
1. Full review of all sections
2. Core Value check — still the right priority?
3. Audit Out of Scope — reasons still valid?
4. Update Context with current state

---

_Last updated: 2026-07-03_
