# Milestones

## v0.1 Initial Template (Shipped: 2026-07-04)

**Phases completed:** 6 phases, 8 plans, 12 tasks

**Key accomplishments:**

- Submodule-ready gendoc-template directory with a 60-line gendoc.yml config file that parameterizes every path the reference implementation hardcoded
- Material-themed mkdocs.yml with GNUS cyan-blue palette, mermaid/mathjax extensions, 5 JS integrations, and runtime config resolution from gendoc.yml via Python hook
- Pinned requirements.txt plus a clean `mkdocs build --strict` run proving the template renders a complete Material-themed static site with gendoc.yml-driven site_name
- Parameterized Doxyfile template ({{TOKEN}} placeholders) and doxybook2 pipeline converting C++ source to markdown API reference pages
- Unified navigation merging hand-written SUMMARY.md with generated API reference (Classes, Files, Namespaces) into a single literate-nav structure
- Single-command build pipeline (Doxygen → doxybook2 → navigation → MkDocs) plus Cloudflare Pages deployment via Wrangler
- Complete README with step-by-step setup instructions and end-to-end workflow verified from `git submodule add` through to deployed site
- Zero hardcoded project paths — everything config-driven from gendoc.yml

---
