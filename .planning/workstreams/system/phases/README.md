# Phase Artifacts — System Workstream

Phase working files live here, one directory per phase, mirroring the app workstream convention:

```
phases/
  01-sdk-module-structure/     01-01-PLAN.md, 01-01-SUMMARY.md, 01-RESEARCH.md, ...
  02-gaml-memory-core/
  ...
```

Conventions:
- Directory names: `NN-kebab-case-name` matching the ROADMAP.md phase
- Plans: `NN-MM-PLAN.md` (e.g., `01-02-PLAN.md`), summaries `NN-MM-SUMMARY.md`
- Phase-level artifacts: `NN-RESEARCH.md`, `NN-CONTEXT.md`, `NN-PATTERNS.md`, `NN-VERIFICATION.md`, `NN-VALIDATION.md`, `NN-REVIEW.md`, `NN-DISCUSSION-LOG.md`
- Directories are created by `/gsd:plan-phase` — none exist yet (workstream created 2026-09-21, nothing planned)
