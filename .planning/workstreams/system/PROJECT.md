# Genius Cognitive System — System Workstream

## What This Is

System-level workstream for the Genius Cognitive System (GCS) cognitive operating system: the embeddable SDK boundary, agentic memory substrate, secure execution, reputation/consensus, capability and public API surfaces, inference efficiency (context lifecycle, VTG, speculation), swarm adaptation (EGGROLL, forecast-driven cognition), and higher cognition (second brain, epistemic arbitration, execution integrity).

Derived 2026-09-21 from the `docs/architecture/` ingest (30 classified docs: 2 LOCKED ADRs, 17 SPECs, 11 DOCs). Intel of record: `.planning/intel/{SYNTHESIS,decisions,requirements,constraints,context}.md`; conflict report: `.planning/INGEST-CONFLICTS.md`.

## Core Value

A distributed, reputation-weighted cognitive system in which Expert Models (ELM/EJM/EDM contracts) execute behind a mandatory security boundary, adapt through swarm retraining, and serve local second-brain users and public OpenAI-compatible API consumers — all behind a stable embeddable SDK whose heavy dependencies never leak.

## Workstream Boundary (binding)

- This workstream does **not** change the app workstream (`.planning/workstreams/app/`) — v1.0 GCS Chat scope, requirements, and roadmap are separate and remain authoritative for the chat application.
- The app "no central servers" constraint governs chat state sync only. The Cloudflare Edge ingress / GCS Gateway Node in the OpenAI-compatible API router SPEC (CON-13) is a separate system-level product surface and is in scope here (see INGEST-CONFLICTS.md INFO on this boundary).
- Phase 1 (SDK/module migration) touches shared repo structure the app builds on — it must be sequenced against app workstream execution, not merged into it.

## Current Milestone: v1.0 System Substrate

**Goal:** Make the system buildable and secure — ADR-01 module structure, GAML v1 memory core, Tool Intermediary execution boundary, reputation/consensus core.

**Target capabilities:**
- Per-module CMake build with `include/gcs/` public SDK surface
- Backend-neutral Flutter packages (`packages/{gcs_client,gcs_chat,gcs_native}`)
- GAML v1 MemoryObject/Cognitive Asset substrate with privacy scopes and CRDT replication
- 100% Tool Intermediary-governed execution with provenance-tagged memory writes
- Reputation-weighted consensus with Byzantine tolerance

## Requirements

See `REQUIREMENTS.md` (83 requirements across 15 categories, 5 milestones).

### Active

All 83 requirements are Active (none validated — nothing shipped yet). See `REQUIREMENTS.md` traceability.

### Out of Scope

| Item | Reason |
|------|--------|
| Legacy 4-phase roadmap (`docs/architecture/roadmap-and-risks.md`) | SUPERSEDED (user decision 2026-09-21). Its genius-core-alpha → genius-swarm-beta → v1 Beta plan and "FP4 v3" language are not imported. System roadmap derives from per-topic SPEC rollout plans. |
| Sloth/Unsloth training-stack integration | Exploratory only (`docs/architecture/exploratory/sloth-integration.md`, no ADR). Needs an ADR assigning training-stack ownership before any commitment. |
| SGFP4 codec implementation | Owned by MNN per ADR-02 (native execution + weight decode). This workstream tracks only integration points (node advertisement, execution contracts in Phases 9/10/15). |
| App v1.0 chat scope (messaging, membership, moderation, bot, lobby) | Owned by `.planning/workstreams/app/`. |
| Forecast rollout stage 5 (future multimodal evidence) | Source marks it "future" (CON-10 §28.21). Deferred; revisit at v1.3 close. |
| GQHSM WASM extension path | Explicitly future in CON-08. |

## Context

- **Intel of record:** `.planning/intel/` (2026-09-21 run — the 2026-07-19 all-DOC synthesis is superseded)
- **Rollout sources:** per-topic SPEC rollout plans — API router §26.24 (8 phases), context lifecycle §17.25 (6), VTG §23.21 (6), speculative decoding §24.15 (6), Frozen Micro-MTP §25.13 (5), EGGROLL §19.18 (5), forecast §28.21 (5)
- **Draft-status content (not commitment):** `execution-integrity-system.md` is "Status: Draft for review" (7 open items §29.9) — Phase 15 is gated on it exiting draft. `openai-compatible-api-router-and-gcs-job-queue.md` §26.26 has 9 open questions — recorded open, resolved at Phase 6 planning.
- **Cross-repo boundaries (ADR-02):** MNN owns model runtime/generation/SGFP4; SuperGenius owns distributed infra (queues, transport, settlement); SGProcessingManager never owns the GCS request lifecycle; GNUS-NEO-SWARM provides the RuntimeCoordinator implementation behind the GCS contract.
- **Known structure gap (Phase 1 driver):** `neoswarm_ffi` is still a Flutter pubspec dependency; `packages/{gcs_client,gcs_chat,gcs_native}`, `include/gcs/`, and the per-module `src/{common,runtime,chat,api,ffi}` split do not exist yet (current: `src/lib/gcs_core.cpp`, `src/lib/gcs_storage/`, `src/ffi/`, `src/proto/`, `src/app/`).

## Constraints

- **Tech stack:** C++17 only (no C++20 features), CMake per-module ownership, thirdparty-only dependencies (never system libs)
- **Platform:** macOS first, then Linux, Windows, iOS, Android; platform via CMake `GENIUS_PLATFORM` resolution, no OS preprocessor guards in source
- **ADR precedence:** ADR > SPEC > PRD > DOC; both LOCKED ADRs bind all phases
- **Cross-ref cycles** among model-and-router/sgfp4/system-overview and frozen-mtp/speculative-decoding are informational "see also" citations (INGEST-CONFLICTS WARNING 1, downgraded) — no action needed

## Key Decisions

| Decision | Rationale | Outcome |
|----------|-----------|---------|
| ADR-01: Embeddable GCS SDK and modular native build — per-module CMake targets (`gcs_common`, `gcs_runtime`, `gcs_chat_core`, `gcs_api`, `gcs_ffi`, `neoswarm_runtime`), `include/gcs/` public surface, stable C ABI, backend-neutral Flutter packages; existing NeoSwarm bridges migrate, not duplicate | Stable SDK boundary; heavy deps never leak through; Flutter backends swappable (native FFI / OpenAI-compatible HTTP/SSE / gRPC / mock) | — Locked (ADR `docs/architecture/decisions/adr-embeddable-sdk-and-modular-build.md`, 2026-07-28) |
| ADR-02: Runtime component ownership — MNN is the only native SGFP4/generation runtime; SGProcessingManager never owns the GCS request lifecycle; SGFP4 is a weight encoding, not an input type; SuperGenius owns distributed execution infra | Prevents duplicate generation loops and SGFP4-as-input misuse | — Locked (ADR `docs/architecture/decisions/adr-runtime-component-ownership.md`, 2026-07-28) |
| `roadmap-and-risks.md` is superseded; system roadmap derives from per-topic SPEC rollout plans | Legacy 4-phase plan predates the Expert Model generalization and per-topic rollouts; importing it would fold superseded intent into planning | — Locked (user decision at ingest, 2026-09-21) |
| System workstream exists alongside, never inside, the app workstream | v1.0 chat scope is separate; prevents scope creep in both directions | — Locked (user decision at ingest, 2026-09-21) |

## Evolution

This document evolves at phase transitions and milestone boundaries.

**After each phase transition** (via `/gsd-transition`):
1. Requirements invalidated? → Move to Out of Scope with reason
2. Requirements validated? → Move to Validated with phase reference
3. New requirements emerged? → Add to Active
4. Decisions to log? → Add to Key Decisions
5. "What This Is" still accurate? → Update if drifted

**After each milestone** (via `/gsd:complete-milestone`):
1. Full review of all sections
2. Core Value check — still the right priority?
3. Audit Out of Scope — reasons still valid?
4. Update Context with current state

---
*Last updated: 2026-09-21 after workstream creation from docs/architecture ingest*
