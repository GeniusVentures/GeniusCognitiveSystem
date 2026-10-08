# Milestones: System Workstream

## v1.0 System Substrate (In Progress)

**Started:** 2026-09-21
**Status:** Planning

**Goal:** Make the system buildable and secure — ADR-01 module structure, GAML v1 memory core, Tool Intermediary execution boundary, reputation/consensus core.

**Phases:** 1–4

**Target capabilities:**
- Per-module CMake build with `include/gcs/` public SDK surface
- Backend-neutral Flutter packages (`packages/{gcs_client,gcs_chat,gcs_native}`)
- GAML v1 MemoryObject/Cognitive Asset substrate with privacy scopes and CRDT replication
- 100% Tool Intermediary-governed execution with provenance-tagged memory writes
- Reputation-weighted consensus with Byzantine tolerance

---

## Future Milestones

### v1.1 External Surfaces
**Phases:** 5–6
- Capability system: canonical contracts, connector registry, MCP adapter, Tool Intermediary-governed execution, drift detection
- OpenAI-compatible API router MVP: api.gnus.ai ingress, signed job queue, node claim/lease, SSE streaming proxy, usage metering (blocked on §26.26 open questions at planning time)

### v1.2 Inference Efficiency
**Phases:** 7–10
- Context lifecycle and caching (deterministic compiler, privacy-scoped prefix cache, compaction with rollback)
- Objective Memory / VTG verified transition substrate
- Speculative decoding with drafter backends and operating budgets
- Frozen Micro-MTP on frozen backbones with mandatory local verification

### v1.3 Adaptive Cognition
**Phases:** 11–12
- EGGROLL swarm retraining: deterministic perturbation reconstruction, beehives, reputation-gated canary promotion
- Forecast-driven cognition: ACE/CES predictive prefetching under budgets and privacy policy (stage 5 multimodal deferred)

### v1.4 Higher Cognition & Integrity
**Phases:** 13–15
- Local Cognitive Second Brain: connectors, Second Brain Agent, memory mirror, privacy modes
- Epistemic Arbitration & Cognitive OS: GQHSM arbiters, plugin ABI, arbitration frameworks
- Execution Integrity System (gated on spec exiting draft): execution contract verification, spot-checks, fraud verdicts
