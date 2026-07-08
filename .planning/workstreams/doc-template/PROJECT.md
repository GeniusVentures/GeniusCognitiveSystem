# gendoc-template

## Core Value

A reusable, config-driven MkDocs documentation template that any GNUS C++ project can add as a git submodule to produce a complete documentation site — combining hand-written markdown with Doxygen-generated API reference, deployable to Cloudflare Pages via Wrangler.

## What This Is

A parameterized generalization of the `../documentation` reference implementation. Instead of hardcoding SuperGenius paths, the template accepts a config file pointing to the host project's hand-written docs directory and C++ source directory. It provides:
- MkDocs + Material theme configuration with GNUS brand styling
- Doxygen config template for C++ API documentation
- Navigation builder that merges hand-written and generated docs
- Build scripts (local and Cloudflare Pages)
- Wrangler deployment configuration

**Shipped v0.1:** 8 plans across 6 phases. Zero hardcoded project paths — everything config-driven from a single `gendoc.yml`.

## What This Is NOT

- NOT a documentation hosting service
- NOT specific to any one GNUS project
- NOT a replacement for the existing `../documentation` setup — it's a template derived from it

## Current State

**Shipped v0.1 Initial Template** (2026-07-04).

The template is complete and verified: `git submodule add` → fill out `gendoc.yml` → run `build.sh` → deployed to Cloudflare Pages. All 14 requirements satisfied. `mkdocs build --strict` produces a clean static site with zero warnings. The doxygen → doxybook2 → navigation → mkdocs pipeline works end-to-end.

**Next milestone:** TBD — no active requirements.

## Requirements

### Validated

- ✓ **CFG-01**: Template exposes a single config file (`gendoc.yml`) — v0.1
- ✓ **CFG-02**: `git submodule add` into a host project, run one setup command, all paths resolve from config — v0.1
- ✓ **CFG-03**: Config includes Wrangler deployment target — v0.1
- ✓ **MKD-01**: Pre-configured Material theme matching GNUS visual style — v0.1
- ✓ **MKD-02**: literate-nav plugin merges hand-written and generated API reference nav — v0.1
- ✓ **MKD-03**: Site works locally (`mkdocs serve`) and built for deployment (`mkdocs build`) — v0.1
- ✓ **API-01**: Generic Doxygen config template — project name, source dir, output dir from config — v0.1
- ✓ **API-02**: doxybook2 converts Doxygen XML to markdown — v0.1
- ✓ **API-03**: Navigation builder produces literate-nav entries for Classes, Files, Namespaces, Modules, Pages — v0.1
- ✓ **BLD-01**: Single build script runs Doxygen → doxybook2 → navigation → MkDocs — v0.1
- ✓ **BLD-02**: Cloudflare Pages deploy script using Wrangler — v0.1
- ✓ **BLD-03**: Scripts work on macOS and Linux — v0.1
- ✓ **TPL-01**: Clean submodule layout — no hardcoded project paths — v0.1
- ✓ **TPL-02**: README with setup instructions for host projects — v0.1

### Active

_(none — all v0.1 requirements shipped and validated)_

### Out of Scope

- Hosting or serving the documentation (Cloudflare Pages handles this)
- Custom MkDocs plugins beyond what Material theme + literate-nav provide
- CI/CD integration (can be added by host projects)

## Key Decisions

- **Phase 01:** Template is its own standalone git repo outside GeniusCogntiveSystem to enable independent git-submodule workflows
- **Phase 01:** Single 60-line `gendoc.yml` config file drives all paths — no scattered configuration
- **Phase 02:** MkDocs Material theme with GNUS cyan-blue color ramp, Doxygen CSS integration ported from reference
- **Phase 02:** 5 JS integrations for nav persistence, anchor handling, smooth scrolling, ToC highlighting, image lightbox
- **Phase 02:** Config-driven via gendoc.yml → load_gendoc_config.py hook (no hardcoded site names or paths)
- **Phase 02:** Python deps pinned: mkdocs==1.6.1, mkdocs-material==9.5.27, pymdown-extensions>=10.14, mkdocs-literate-nav==0.6.1, pyyaml>=6.0
- **Phase 03:** Doxygen config uses `{{TOKEN}}` placeholders resolved at build time from gendoc.yml
- **Phase 03:** doxybook2 converts Doxygen XML output to markdown for literate-nav consumption
- **Phase 04:** Navigation builder merges hand-written SUMMARY.md with generated API reference into a single SUMMARY_EXT.md
- **Phase 05:** Single `build.sh` orchestrates the full pipeline (Doxygen → doxybook2 → navigation → MkDocs)
- **Phase 05:** `wrangler.toml.template` with `{{PLACEHOLDER}}` tokens for Cloudflare Pages deployment
- **Phase 06:** README expanded from 65-line stub to 244-line guide covering full setup workflow

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

_Last updated: 2026-07-04 after v0.1 milestone_
