# Cross-Submodule Capabilities

**Generated:** 2026-07-18 | **Source:** `docs/architecture/` ingest (merge mode)
**Context:** Capabilities described in the Genius Cognitive System architecture that span multiple submodules or don't belong to any single existing workstream.

These were identified during the `/gsd:ingest-docs` merge into `GNUS-NEO-SWARM/.planning/`. They are documented here so future planning operations can route them to the correct workstream or create new ones.

---

## Capability System + Connectors

**Source:** `docs/architecture/capability-system.md` (DOC)

The GCS Capability System orchestrates tool execution across the cognitive stack. It manages connector registration, capability discovery, tool lifecycle, and sandboxing. This is a GCS-level orchestration layer that sits above individual swarm nodes.

**Likely home:** New GCS orchestration submodule, or co-located with the API Router

---

## OpenAI-Compatible API Router + GCS Job Queue

**Source:** `docs/architecture/openai-compatible-api-router-and-gcs-job-queue.md` (SPEC)

External-facing API that accepts OpenAI v1-compatible chat completions requests, routes them through the GCS job queue, and dispatches to swarm nodes for execution. Includes rate limiting, authentication, and job lifecycle management.

**Likely home:** New `gnus-api-gateway` submodule, or integrated into a GCS orchestration layer

---

## Speculative Decoding + VTG Candidate Scheduling

**Source:** `docs/architecture/speculative-decoding-and-vtg.md` (SPEC)

Uses Verified Transition Graphs (VTG) to predict and pre-compute likely next-token candidates during inference. Reduces end-to-end latency by parallelizing token generation.

**Likely home:** `GNUS-NEO-SWARM` Phase 4+ (SGProcessing integration) or a dedicated inference optimization submodule

---

## Objective Memory + Verified Transition Graph (VTG)

**Source:** `docs/architecture/objective-memory-vtg.md` (SPEC)

Chain-level cognitive state tracking using verified transition graphs. Records decision paths, evidence chains, and state transitions with cryptographic verification. Integrates with SuperGenius blockchain for tamper-proof audit trails.

**Likely home:** Partially in `GNUS-NEO-SWARM` Phase 11 (cognitive OS extensions), partially in SuperGenius chain-level state

---

## Local Cognitive Second Brain

**Source:** `docs/architecture/local-cognitive-second-brain.md` (DOC)

Offline/local operation mode that runs a complete cognitive stack on a single device without network connectivity. Caches GAML memory, pre-loads quantized ELMs, and syncs when connectivity is restored.

**Likely home:** Separate client application or Flutter integration, not a submodule of the core cognitive system

---

## EGGROLL Swarm Retraining

**Source:** `docs/architecture/eggroll-swarm-retraining.md` (SPEC)

**Note:** Already captured in `GNUS-NEO-SWARM` Phase 11 (Advanced Cognition) and the `gnus-poc` workstream (Python training pipeline). Listed here as a cross-reference — implementation spans the C++ inference engine (neoswarm) and the Python training pipeline (poc).

---

## Status

These capabilities need workstream assignment before they can be planned. The architecture docs describe their design but do not constitute formal requirements or decisions (no ADRs or PRDs exist for them).

To create workstreams for any of these:
```
/gsd:workstreams --create <name> --submodule <path>
/gsd:new-milestone --ws <name>
```
