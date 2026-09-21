# Synthesis Summary

**Generated:** 2026-09-21 | **Mode:** merge (app workstream: .planning/workstreams/app/)
**Source:** docs/architecture/ — 30 classified docs (27 top-level, 2 decisions/ ADRs, 1 exploratory/)
**Purpose of this ingest (user):** find gaps and changes to planning and roadmap vs the current app workstream. Bias: gap/change detection, not wholesale re-description.
**Supersedes:** the 2026-07-19 synthesis (all-DOC manifest typing, no typed intel extracted).

## Doc Counts by Type

- ADR: 2 (both LOCKED, high confidence)
- SPEC: 17 (all medium confidence, unlocked)
- DOC: 11 (all medium confidence, unlocked)
- PRD: 0 | UNKNOWN: 0

16 of 30 docs were refreshed 2026-09-21 (the Expert Model generalization pass: secure-agent, objective-memory-vtg, model-and-router, local-cognitive-second-brain, frozen-mtp, executive-summary, execution-integrity, epistemic, eggroll, distributed-swarm-thinking, context-lifecycle, cognitive-retaining, cognitive-evolution-control, capability-system, agentic-memory-layer, agent-module-inventory). 2 more on 2026-09-17 (system-overview, speculative-decoding). Older generation: ADRs 2026-07-28; sgfp4/openai-api/forecast 2026-07-19; roadmap-and-risks, reputation-consensus, grounding, future-and-positioning, execution-and-performance, ai-safety 2026-06-28; sloth-integration 2026-06-13.

## Decisions Locked (2)

- D-ADR-01 Embeddable GCS SDK and Modular Native Build — docs/architecture/decisions/adr-embeddable-sdk-and-modular-build.md
- D-ADR-02 Runtime Component Ownership Boundaries — docs/architecture/decisions/adr-runtime-component-ownership.md

Complementary; no LOCKED-vs-LOCKED contradiction; no conflict with app workstream context (no locked decisions there). Detail: intel/decisions.md.

## Requirements Extracted (6 derived groups; 0 PRDs)

No PRD-type docs. Derived from SPEC acceptance/implementation sections, all variants preserved per source:
REQ-context-lifecycle (15 MUSTs + 15 acceptance), REQ-openai-api-mvp (19 acceptance + 9 open questions), REQ-capability-system (10), REQ-forecast-cognition (12), REQ-second-brain (8), REQ-secure-agent-targets (5). Detail: intel/requirements.md.

## Constraints (17)

Type breakdown: schema 8 (agent-inventory, agentic-memory, context-lifecycle, forecast, frozen-mtp, objective-memory-vtg, sgfp4, speculative-decoding), api-contract 4 (capability-system, epistemic, openai-api-router, secure-agent), protocol 4 (cognitive-evolution-control, eggroll, execution-integrity, reputation-consensus), nfr 1 (ai-safety). One self-declared draft (EIS). Detail: intel/constraints.md.

## Context Topics (11)

executive-summary, system-overview, model-and-router, grounding, execution-and-performance, future-and-positioning, distributed-swarm-thinking-context, cognitive-retaining-system, local-cognitive-second-brain, roadmap-and-risks (legacy-flagged), sloth-integration (exploratory). Detail: intel/context.md.

## Conflicts

- BLOCKERS: 0
- WARNINGS (competing variants / user must pick): 3 — cross-ref cycles (downgraded from process-default BLOCKER, rationale in report), competing roadmap framings, GCSB-04 ELM-vs-Expert-Model terminology
- INFO (auto-resolved / recorded): 6

Report: /Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/GeniusCogntiveSystem/.planning/INGEST-CONFLICTS.md

## Gap/Change Findings vs App Workstream Planning (for gsd-roadmapper)

Directly affects the app workstream:
1. GCSB-04 / ROADMAP Phase 6 criterion 4 use superseded ELM-only routing language; refreshed model-and-router.md specifies capability-first Expert Model (ELM/EJM/EDM) selection, rule-based MVP router, and calls existing IELM code a migration compatibility path. WARNING 3.
2. LOCKED ADR-01 gaps vs completed Phase 1: neoswarm_ffi still a pubspec dependency; packages/{gcs_client,gcs_chat,gcs_native}, include/gcs/, and the per-module src split do not exist yet. ADR-01's consequences direct migration, not removal. INFO 2.
3. ADR-01 already governs the app's native boundary and is consistent with Phase 1 work (gcs_ffi C ABI, D-26 push-not-pull); no rework implied.

System-level content with no app-workstream counterpart (candidate separate workstreams/milestones, not v1.0 chat scope): OpenAI-compatible API router + GCS job queue (8-phase MVP + open questions), GCS Capability System (10 requirements), Context Lifecycle/Caching (15 MUSTs), GAML v1, Objective Memory/VTG (6 phases), Speculative Decoding (6 phases), Frozen Micro-MTP (5 phases), EGGROLL retraining (5 phases), Forecast-Driven Cognition (5 phases), EIS (draft), Epistemic Arbitration/Cognitive OS, Local Cognitive Second Brain, reputation/consensus, secure agent architecture, agent/module inventory.

Roadmap framing decision required (WARNING 2): legacy 4-phase roadmap-and-risks.md vs per-topic rollout plans in refreshed SPECs vs the 7-phase app roadmap. Recommendation encoded in the report: mark roadmap-and-risks.md superseded; keep app roadmap as the execution roadmap; treat per-topic plans as system-level workstream inputs.

## Per-Type Intel Files

- intel/decisions.md — 2 LOCKED ADRs + non-ADR decision-flavored items
- intel/requirements.md — 6 derived requirement groups + gap notes vs app REQUIREMENTS.md
- intel/constraints.md — 17 SPEC entries with type, status, and provenance
- intel/context.md — 11 DOC topics with supersession flags
- intel/cross-submodule-capabilities.md — prior-run (2026-07-18) artifact, left in place, consistent with this run
