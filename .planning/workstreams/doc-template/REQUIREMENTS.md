# Requirements — gendoc-template v0.1

## Active Requirements

### Config & Setup
- [ ] **CFG-01**: Template exposes a single config file (`gendoc.yml`) specifying: hand-written docs directory, C++ source directory, project name, Cloudflare account details
- [ ] **CFG-02**: `git submodule add` into a host project, run one setup command, and all paths resolve from config
- [ ] **CFG-03**: Config includes Wrangler deployment target (zone ID, route, or pages project name)

### MkDocs Site
- [ ] **MKD-01**: Pre-configured Material theme matching GNUS visual style (colors, nav, search, mermaid, mathjax)
- [ ] **MKD-02**: literate-nav plugin merges hand-written `SUMMARY.md` with generated API reference nav
- [ ] **MKD-03**: Site works both locally (`mkdocs serve`) and built for deployment (`mkdocs build`)

### API Reference (Doxygen)
- [ ] **API-01**: Generic Doxygen config template — project name, source dir, output dir come from config
- [ ] **API-02**: doxybook2 converts Doxygen XML to markdown pages in the docs directory
- [ ] **API-03**: Navigation builder parses Doxygen index files and produces literate-nav entries for Classes, Files, Namespaces, Modules, Pages

### Build & Deploy
- [ ] **BLD-01**: Single build script that runs Doxygen → doxybook2 → navigation → MkDocs
- [ ] **BLD-02**: Cloudflare Pages deploy script using Wrangler (from config entry)
- [ ] **BLD-03**: Scripts work on macOS and Linux

### Template Structure
- [ ] **TPL-01**: Submodule directory layout is clean: config template, scripts/, theme assets, Doxygen template — no hardcoded project paths
- [ ] **TPL-02**: README with setup instructions for host projects

## Future Requirements

_(none yet)_

## Out of Scope

- Hosting or serving the documentation (Cloudflare Pages handles this)
- Custom MkDocs plugins beyond what Material theme + literate-nav provide
- CI/CD integration (can be added by host projects)

## Traceability

| REQ-ID | Phase | Status |
|--------|-------|--------|
| CFG-01 | — | — |
| CFG-02 | — | — |
| CFG-03 | — | — |
| MKD-01 | — | — |
| MKD-02 | — | — |
| MKD-03 | — | — |
| API-01 | — | — |
| API-02 | — | — |
| API-03 | — | — |
| BLD-01 | — | — |
| BLD-02 | — | — |
| BLD-03 | — | — |
| TPL-01 | — | — |
| TPL-02 | — | — |

---

_Last updated: 2026-06-27_
