# Roadmap: gendoc-template

## Milestones

- ✅ **v0.1 Initial Template** — Phases 1-6 (shipped 2026-07-04)

## Phases

<details>
<summary>✅ v0.1 Initial Template (Phases 1-6) — SHIPPED 2026-07-04</summary>

- [x] Phase 1: Template Skeleton & Config (1/1 plan) — completed 2026-06-27
- [x] Phase 2: MkDocs Site (2/2 plans) — completed 2026-07-03
- [x] Phase 3: API Reference Pipeline (2/2 plans) — completed 2026-06-28
- [x] Phase 4: Navigation Integration (1/1 plan) — completed 2026-06-28
- [x] Phase 5: Build & Deploy (1/1 plan) — completed 2026-06-28
- [ ] Phase 5.1: CI/CD Deployment (INSERTED) (0/2 plans)
- [x] Phase 6: Documentation & Validation (1/1 plan) — completed 2026-06-28

</details>

Full milestone details: [milestones/v0.1-ROADMAP.md](milestones/v0.1-ROADMAP.md)

### 🚧 Phase 5.1: CI/CD Deployment (In Progress)

**Goal:** GitHub Actions CI/CD pipeline automatically builds and deploys the documentation site to Cloudflare Pages on push. Host projects get a deploy.yaml.template to drop into `.github/workflows/`.

**Depends on:** Phase 5

**Requirements:** CI-01, CI-02

**Plans:** 2 plans

Plans:
- [ ] 05.1-01-PLAN.md — GitHub Actions deploy.yaml.template workflow (verbatim-copy contract, full pipeline + Wrangler deploy)
- [ ] 05.1-02-PLAN.md — setup_ci.sh interactive onboarding script + README CI/CD Deployment section

**Success Criteria:**
1. `deploy.yaml.template` in gendoc-template can be copied VERBATIM to a host repo's `.github/workflows/deploy.yaml` (no token substitution needed)
2. GitHub Actions workflow runs the full build pipeline (Doxygen → doxybook2 → navigation → MkDocs build) and deploys to Cloudflare Pages via Wrangler
3. `scripts/setup_ci.sh` detects/verifies Cloudflare API tokens and stores them as GitHub secrets via `gh secret set`
4. README includes a CI/CD Deployment section with prerequisites, setup flow, and required GitHub secrets table



## Progress

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|----------------|--------|-----------|
| 1. Template Skeleton & Config | v0.1 | 1/1 | Complete | 2026-06-27 |
| 2. MkDocs Site | v0.1 | 2/2 | Complete | 2026-07-03 |
| 3. API Reference Pipeline | v0.1 | 2/2 | Complete | 2026-06-28 |
| 4. Navigation Integration | v0.1 | 1/1 | Complete | 2026-06-28 |
| 5. Build & Deploy | v0.1 | 1/1 | Complete | 2026-06-28 |
| 5.1. CI/CD Deployment | v0.1 | 0/2 | Not started | — |
| 6. Documentation & Validation | v0.1 | 1/1 | Complete | 2026-06-28 |
