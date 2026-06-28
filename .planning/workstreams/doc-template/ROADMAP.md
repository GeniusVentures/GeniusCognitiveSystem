# Roadmap: gendoc-template v0.1

## Overview

Extract the `../documentation` MkDocs + Doxygen pattern into a reusable, config-driven git submodule. A host C++ project adds gendoc-template as a submodule, fills out a single config file, and gets a complete documentation site — hand-written markdown plus auto-generated C++ API reference, deployable to Cloudflare Pages. Each phase delivers one coherent capability, building from template skeleton through to end-to-end validated workflow.

## Phases

- [x] **Phase 1: Template Skeleton & Config** — Submodule-ready directory layout with `gendoc.yml` config file (completed 2026-06-27)
- [ ] **Phase 2: MkDocs Site** — Material-themed MkDocs renders host project's hand-written docs
- [x] **Phase 3: API Reference Pipeline** — Doxygen + doxybook2 generates C++ API docs as markdown (completed 2026-06-28)
- [x] **Phase 4: Navigation Integration** — Hand-written and generated API docs merge into unified navigation (completed 2026-06-28)
- [x] **Phase 5: Build & Deploy** — Single-command full build and Cloudflare Pages deployment (completed 2026-06-28)
- [x] **Phase 6: Documentation & Validation** — README, setup instructions, end-to-end workflow verified (completed 2026-06-28)

## Phase Details

### Phase 1: Template Skeleton & Config
**Goal**: Template exists as a git-submodule-ready directory with a config file driving all paths
**Depends on**: Nothing (first phase)
**Requirements**: CFG-01, CFG-03, TPL-01
**Success Criteria** (what must be TRUE):
  1. Template can be added to a host C++ project via `git submodule add`
  2. A single `gendoc.yml` config file exists with fields for: project name, hand-written docs directory, C++ source directory, and Cloudflare Pages deployment target
  3. Directory layout separates concerns cleanly: config template, scripts/, theme assets/, Doxygen template/ — zero hardcoded project paths
**Plans**: 1 plan
Plans:
- [x] 01-01-PLAN.md — Directory skeleton, gendoc.yml config file schema, and sanity audit

### Phase 2: MkDocs Site
**Goal**: MkDocs with Material theme renders a GNUS-styled site from the host project's hand-written markdown docs
**Depends on**: Phase 1
**Requirements**: MKD-01, MKD-03
**Success Criteria** (what must be TRUE):
  1. `mkdocs serve` renders a site with GNUS visual styling (colors, navigation, search) driven by config from `gendoc.yml`
  2. `mkdocs build` produces a complete static site with zero build errors
  3. Site supports mermaid diagrams and mathjax rendering out of the box
**Plans**: 2 plans
**UI hint**: yes
Plans:
- [x] 02-01-PLAN.md — mkdocs.yml with Material theme, gendoc.yml config hook, and theme assets (CSS + JS)
- [ ] 02-02-PLAN.md — requirements.txt with pinned Python dependencies and mkdocs build verification

### Phase 3: API Reference Pipeline
**Goal**: Doxygen + doxybook2 pipeline converts C++ source to markdown API reference pages
**Depends on**: Phase 1
**Requirements**: API-01, API-02, API-03
**Success Criteria** (what must be TRUE):
  1. Doxygen generates XML documentation from the C++ source directory specified in `gendoc.yml`
  2. doxybook2 converts Doxygen XML output to markdown pages in the docs directory
  3. Navigation builder produces well-structured literate-nav entries for Classes, Files, Namespaces, Modules, and Pages from parsed Doxygen index files
**Plans**: 2 plans
Plans:
- [x] 03-01-PLAN.md — Parameterized Doxyfile template ({{TOKEN}} placeholders) and doxybook2 config
- [x] 03-02-PLAN.md — build_api_reference.sh pipeline script and generalized build_navigation.py

### Phase 4: Navigation Integration
**Goal**: Hand-written docs and generated API reference appear together in a single unified site navigation
**Depends on**: Phase 2, Phase 3
**Requirements**: MKD-02
**Success Criteria** (what must be TRUE):
  1. Hand-written markdown docs from the host project's docs directory appear in site navigation
  2. Generated API reference pages appear alongside hand-written docs in the same navigation structure
  3. Navigation has zero broken links between hand-written and generated sections across the full site
**Plans**: 1 plan
Plans:
- [x] 04-01-PLAN.md — Merge hand-written SUMMARY.md with generated API reference nav into literate-nav-compatible SUMMARY_EXT.md
**UI hint**: yes

### Phase 5: Build & Deploy
**Goal**: One command builds the complete documentation site and deploys to Cloudflare Pages
**Depends on**: Phase 4
**Requirements**: BLD-01, BLD-02, BLD-03
**Success Criteria** (what must be TRUE):
  1. A single build script executes the complete pipeline: Doxygen -> doxybook2 -> navigation -> MkDocs build
  2. Wrangler deployment script publishes the built site to Cloudflare Pages using credentials from `gendoc.yml`
  3. Both build and deploy scripts run successfully on macOS and Linux without platform-specific workarounds
**Plans**: 1 plan
Plans:
- [x] 05-01-PLAN.md — build.sh (full pipeline orchestrator), wrangler.toml.template, and deploy.sh (Cloudflare Pages)

### Phase 6: Documentation & Validation
**Goal**: Template is self-documenting and the full end-to-end workflow is proven
**Depends on**: Phase 5
**Requirements**: CFG-02, TPL-02
**Success Criteria** (what must be TRUE):
  1. README provides step-by-step setup instructions that a new developer can follow from scratch
  2. Following the README from `git submodule add` through to a deployed Cloudflare Pages site works end-to-end with no gaps
  3. All paths resolve from `gendoc.yml` — a host project needs no manual edits beyond filling out the config file
**Plans**: 1 plan
Plans:
- [x] 06-01-PLAN.md — README expansion, .gitignore audit, build_api_reference.sh path fix, and end-to-end verification sweep

## Progress

| Phase | Plans Complete | Status | Completed |
|-------|----------------|--------|-----------|
| 1. Template Skeleton & Config | 1/1 | Complete   | 2026-06-27 |
| 2. MkDocs Site | 1/2 | In Progress|  |
| 3. API Reference Pipeline | 2/2 | Complete   | 2026-06-28 |
| 4. Navigation Integration | 1/1 | Complete   | 2026-06-28 |
| 5. Build & Deploy | 1/1 | Complete   | 2026-06-28 |
| 6. Documentation & Validation | 1/1 | Complete   | 2026-06-28 |
