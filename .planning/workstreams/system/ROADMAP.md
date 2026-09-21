# Roadmap: Genius Cognitive System — System Workstream

## Overview

Build the GCS cognitive operating system from the substrate up: first the ADR-01 embeddable SDK/module structure (closing the known `neoswarm_ffi` migration gap), then the memory, security, and reputation substrate every later capability depends on; then the two external surfaces (capability system, OpenAI-compatible public API); then inference efficiency (context lifecycle, VTG, speculative decoding, Frozen Micro-MTP); then the adaptation loop (EGGROLL swarm retraining, forecast-driven cognition); finally higher cognition and integrity (local second brain, epistemic arbitration, execution integrity). The roadmap is derived from the per-topic SPEC rollout plans plus the non-rollout requirement groups — `docs/architecture/roadmap-and-risks.md` is SUPERSEDED and its legacy 4-phase plan / "FP4 v3" language is not imported. This workstream does not modify the app workstream (`.planning/workstreams/app/`); Phase 1 must be sequenced against app execution because both build on the same repo structure.

## Phases

**Phase Numbering:**

- Integer phases (1, 2, 3): Planned milestone work
- Decimal phases (2.1, 2.2): Urgent insertions (marked with INSERTED)

Decimal phases appear between their surrounding integers in numeric order.

### v1.0 System Substrate

- [ ] **Phase 1: SDK & Module Structure** - ADR-01 migration: per-module CMake targets, `include/gcs/` public surface, backend-neutral Flutter packages, inventory reconciliation
- [ ] **Phase 2: GAML v1 Memory Core** - MemoryObject/Cognitive Asset substrate with privacy scopes, staged retrieval, CRDT replication
- [ ] **Phase 3: Secure Execution Boundary** - Tool Intermediary choke-point, attestations, provenance-tagged writes, instruction scrubber
- [ ] **Phase 4: Reputation & Consensus Core** - Role/domain-aware reputation, weighted updates, Byzantine-tolerant weighted consensus

### v1.1 External Surfaces

- [ ] **Phase 5: Capability System** - Canonical capability contracts, connector registry, MCP adapter, Tool Intermediary-governed execution
- [ ] **Phase 6: OpenAI-Compatible API Router MVP** - api.gnus.ai ingress, signed job queue, node claim/lease, SSE streaming proxy, usage metering

### v1.2 Inference Efficiency

- [ ] **Phase 7: Context Lifecycle & Caching** - Deterministic Context Compiler, privacy-scoped prefix cache, ExpertDigest isolation, compaction with rollback
- [ ] **Phase 8: Objective Memory / VTG** - Verified transition substrate: schemas, poisoning-resistant frontier, content-addressed storage
- [ ] **Phase 9: Speculative Decoding** - Drafter backends, confidence-scheduled verification, operating budgets, node advertisement
- [ ] **Phase 10: Frozen Micro-MTP** - Micro-MTP head within budget envelope, mandatory local verification, router selection policy

### v1.3 Adaptive Cognition

- [ ] **Phase 11: EGGROLL Swarm Retraining** - Deterministic perturbation reconstruction, beehives, reputation-gated promotion with canaries
- [ ] **Phase 12: Forecast-Driven Cognition** - ACE forecasts, CES predictive prefetching, GAML forecast interfaces, privacy-safe speculation

### v1.4 Higher Cognition & Integrity

- [ ] **Phase 13: Local Cognitive Second Brain** - Local connectors, Second Brain Agent, supporting-agent pipeline, memory mirror, privacy modes
- [ ] **Phase 14: Epistemic Arbitration & Cognitive OS** - GQHSM arbiters, plugin ABI, EpistemicContext, arbitration frameworks
- [ ] **Phase 15: Execution Integrity System (draft-gated)** - Execution contract verification, spot-checks, fraud verdicts — blocked until EIS spec exits draft

## Phase Details

### Phase 1: SDK & Module Structure
**Milestone**: v1.0
**Goal**: The repo satisfies ADR-01 — per-module CMake ownership, a leak-free public SDK surface, and backend-neutral Flutter packages — closing the migration gap where `neoswarm_ffi` is still a pubspec dependency and `packages/{gcs_client,gcs_chat,gcs_native}`, `include/gcs/`, and the `src/{common,runtime,chat,api,ffi}` split do not exist yet.
**Depends on**: Nothing (first phase)
**Requirements**: STRUCT-01, STRUCT-02, STRUCT-03, STRUCT-04
**Sources**: ADR-01 `docs/architecture/decisions/adr-embeddable-sdk-and-modular-build.md`; CON-01 `agent-module-development-inventory.md` §31.21; INGEST-CONFLICTS INFO 2
**Success Criteria** (what must be TRUE):
  1. Root CMake delegates via `add_subdirectory()` and the six ADR-01 targets (`gcs_common`, `gcs_runtime`, `gcs_chat_core`, `gcs_api`, `gcs_ffi`, `neoswarm_runtime`) build with only direct dependencies declared
  2. Public headers under `include/gcs/` compile standalone — no MNN, Vulkan/MoltenVK, SGProcessingManager, SuperGenius, libp2p, or RocksDB types leak through the SDK surface
  3. The Flutter app builds against `packages/{gcs_client,gcs_chat,gcs_native}` with `neoswarm_ffi` removed from pubspec, and existing chat functionality is unchanged (migration, not removal)
  4. Every CON-01 §31.21 inventory item maps to an owning module with trust tier and validation gate, recorded for later phases to build against
**Coordination note**: The app workstream (Phase 3: Messaging onward) builds on the current `src/ffi/` + `gcs_ffi` C ABI. Sequence this migration between app phases, never mid-phase; ADR-01 directs migration of existing bridges, and ADR-01's C ABI is already consistent with the app's Phase 1 work (D-26 push-not-pull), so no app rework is implied.
**Plans**: TBD

### Phase 2: GAML v1 Memory Core
**Milestone**: v1.0
**Goal**: The agentic memory substrate exists — MemoryObjects and the 19 Cognitive Asset types validate, privacy scopes enforce, and CRDT replication converges — so every later memory-consuming capability (capability assets, forecast, second brain, EGGROLL distillation) has a foundation.
**Depends on**: Phase 1
**Requirements**: GAML-01, GAML-02, GAML-03, GAML-04
**Sources**: CON-02 `agentic-memory-layer.md`; CON-05 `cognitive-evolution-control.md` (Qualified Cognitive Events → memory/policy updates routing)
**Success Criteria** (what must be TRUE):
  1. MemoryObjects and all 19 Cognitive Asset types can be created, validated, and persisted per the CON-02 schemas
  2. Reads/writes crossing a privacy scope (7 classes, local-only → public) are rejected
  3. Ingestion → staged retrieval → surprise-gated write operates end-to-end on a local test corpus
  4. Memory state converges via CRDT across two nodes, and swarm memory consensus emits verifiable Consensus Records
**Plans**: TBD

### Phase 3: Secure Execution Boundary
**Milestone**: v1.0
**Goal**: All side-effecting execution passes through the Tool Intermediary with attestations and provenance-tagged memory writes — the secure-agent operational targets are met on the test harness.
**Depends on**: Phase 2
**Requirements**: SEC-01, SEC-02, SEC-03, SEC-04, SEC-05
**Sources**: CON-15 `secure-agent-architecture.md` (§18.1.1.2 operational targets); CON-03 `ai-safety.md` (node-level screening + Tool Intermediary control layers)
**Success Criteria** (what must be TRUE):
  1. A tool execution attempted outside the Tool Intermediary is refused; 100% of executed tool calls in the test suite carry a valid attestation
  2. Durable memory writes derived from tools/external content carry provenance metadata and trust classification; writes without them are rejected
  3. The instruction scrubber detects injected trap prompts in the test corpus, and side-effect proposals route through approval gates under node trust tiers A–D
  4. Tool Intermediary and swarm overhead are reported separately from reasoning latency in metrics output
**Plans**: TBD

### Phase 4: Reputation & Consensus Core
**Milestone**: v1.0
**Goal**: Nodes earn role/domain-aware reputation and the swarm reaches weighted consensus under Byzantine tolerance — the participation substrate for EGGROLL, capability routing, and swarm execution.
**Depends on**: Phase 3
**Requirements**: REP-01, REP-02, REP-03, REP-04
**Sources**: CON-14 `reputation-consensus.md`; CON-03 `ai-safety.md` (safety penalties Δreputation_safety)
**Success Criteria** (what must be TRUE):
  1. Role/domain-aware reputation scores update via the weighted formulas (accuracy/quality, latency, consistency, safety) and stay clipped to [0,1]
  2. Reputation state persists (wallet-core + RocksDB) and replicates via CRDT across nodes
  3. Weighted consensus reaches decisions in the spec's Byzantine-node test scenarios within the liveness rules
  4. Reputation-gated participation excludes sub-threshold nodes, and genesis anchors bootstrap a fresh swarm
**Plans**: TBD

### Phase 5: Capability System
**Milestone**: v1.1
**Goal**: External tools and data sources become governed capabilities — canonical contracts in a registry, executed only through the Tool Intermediary, with drift detection and administrative review.
**Depends on**: Phase 3, Phase 4, Phase 2
**Requirements**: CAP-01, CAP-02, CAP-03, CAP-04, CAP-05, CAP-06, CAP-07, CAP-08, CAP-09, CAP-10
**Sources**: CON-04 `capability-system.md` §30.14 (all 10 implementation requirements)
**Success Criteria** (what must be TRUE):
  1. A tool described via MCP appears in the connector registry as a proposed canonical CapabilityContract with a deterministic hash validated before registration
  2. The local read-only connector retrieves files/notes/calendar/email, and a remote read-only connector works via MCP or OpenAPI
  3. Every capability execution routes through the Tool Intermediary with credential references only (no raw secrets)
  4. Manifest drift triggers revocation and administrative review; provider/connector health and reputation are visible in the registry
**Plans**: TBD

### Phase 6: OpenAI-Compatible API Router MVP
**Milestone**: v1.1
**Goal**: An external developer with a standard OpenAI SDK can hit `https://api.gnus.ai/v1/chat/completions` and have the request flow through signed jobs to a claiming GCS node and back as compliant streaming/non-streaming output.
**Depends on**: Phase 1 (gcs_api module; job/claim contracts)
**Requirements**: API-01, API-02, API-03, API-04, API-05, API-06, API-07, API-08
**Sources**: CON-13 `openai-compatible-api-router-and-gcs-job-queue.md` §26.3–26.25 (MVP plan §26.24 — 8-phase rollout; acceptance §26.25); ADR-02 §6 (SuperGenius owns queues/transport/settlement — this phase delivers the GCS-side gateway bridge, job contract, node claim/lease integration, SSE proxy)
**Planning gate**: The 9 open questions (§26.26 — host repo, protobuf-vs-JSON-first, lease semantics, alias policy, settlement record minimum, etc.) must be answered before planning this phase; they are recorded open in REQUIREMENTS.md, not pre-decided here.
**Success Criteria** (what must be TRUE):
  1. A standard OpenAI SDK completes a chat completion against `https://api.gnus.ai/v1/chat/completions` with API key auth via the Cloudflare ingress
  2. A registered node claims a signed request job (first-valid-claim), holds a lease, and a failed job requeues
  3. Streaming arrives as incremental OpenAI-compatible SSE chunks (not buffered), a client disconnect cancels the GCS job, and slow clients cannot grow proxy memory unboundedly
  4. Usage is recorded per tenant/project/API key/node with generated vs delivered tokens distinguished, existing processing chunk jobs keep working, and API jobs can spawn child processing jobs
**Plans**: TBD

### Phase 7: Context Lifecycle & Caching
**Milestone**: v1.2
**Goal**: Context assembly is deterministic, privacy-safe, and metered — stable prefixes cache correctly across the full compatibility identity, every injected token is explainable, and compaction never loses critical state.
**Depends on**: Phase 1, Phase 2
**Requirements**: CTX-01, CTX-02, CTX-03, CTX-04, CTX-05, CTX-06, CTX-07, CTX-08
**Sources**: CON-06 `context-lifecycle-caching-governance.md` §17.23–17.24 (15 MUSTs + 15 acceptance; rollout §17.25 — 6 stages: instrumentation → deterministic compilation → prefix cache management → expert isolation/lazy schemas → compaction/inspection → adaptive optimization)
**Success Criteria** (what must be TRUE):
  1. Identical inputs produce byte-identical stable prefixes, and changing only the current request does not change the prefix hash
  2. Model/tokenizer/adapter/runtime/policy/contract/tenant/privacy/version changes prevent incompatible reuse, and cache entries never cross tenant/privacy boundaries via content-hash equality alone
  3. Every injected item carries reason, source, privacy scope, trust class, and token count, attributable via the Context Inspector without exposing unauthorized content, and every specialist returns a bounded ExpertDigest
  4. Compaction preserves objectives/constraints/decisions/questions/evidence/tool state/privacy, and a failed compaction restores the prior generation
  5. Revocation purges affected cache and derived context; efficiency changes pass the same quality/safety suite as baseline, and regressions raise actionable alerts
**Plans**: TBD

### Phase 8: Objective Memory / VTG
**Milestone**: v1.2
**Goal**: The verified transition substrate exists — schemas validate, state identity verifies, updates resist poisoning, and storage replicates — within its explicit non-goals (not a GAML replacement, not an output cache, no policy bypass).
**Depends on**: Phase 2
**Requirements**: VTG-01, VTG-02, VTG-03, VTG-04
**Sources**: CON-12 `objective-memory-vtg.md` (rollout §23.21 — 6 phases; non-goals §23.22); CON-05 (VTG transitions as cognitive-evolution routing target)
**Success Criteria** (what must be TRUE):
  1. Transition edges and learning events validate against the schemas, and state identity (hash/HMAC construction) verifies
  2. Candidate frontier updates resist poisoning under the adversarial test inputs
  3. The storage stack persists content-addressed data across RocksDB and IPFS-lite and replicates via CRDT across two nodes
  4. The §23.22 non-goal tests hold: VTG cannot serve as an unverified output cache or bypass policy, and tenant privacy scopes enforce
**Plans**: TBD

### Phase 9: Speculative Decoding
**Milestone**: v1.2
**Goal**: Constrained nodes speed up generation via compact local speculation — pluggable drafter backends verified with confidence scheduling within hard memory budgets, advertised to the Router.
**Depends on**: Phase 8
**Requirements**: SDEC-01, SDEC-02, SDEC-03, SDEC-04, SDEC-05
**Sources**: CON-17 `speculative-decoding-and-vtg.md` (rollout §24.15 — 6 phases; scope baseline §24.16); ADR-02 §4 (MNN owns generation/speculative decoding semantics — this phase delivers GCS-side drafter contracts, verification scheduling, advertisement, and policy)
**Success Criteria** (what must be TRUE):
  1. Drafter backends (VTG lookup, rule/grammar/schema, Frozen Micro-MTP, micro-diffusion, tiny causal tree) plug in behind the common verification interface
  2. Confidence-scheduled verification accepts/rejects drafts with prefix retention, and speculative output matches non-speculative decoding on the test suite
  3. Operating budgets hold on the target node (100–350MB active model, 5–50MB drafter, 10–100MB VTG hot shard)
  4. The Router consumes node capability advertisement JSON, and signed (ed25519) swarm outcome events are recorded; the production baseline is compact local speculation (§24.16)
**Plans**: TBD

### Phase 10: Frozen Micro-MTP
**Milestone**: v1.2
**Goal**: Edge nodes gain multi-token prediction from a small head on a frozen backbone — budgeted, locally verified, and selectable by router policy alongside the other drafter classes.
**Depends on**: Phase 9, Phase 8
**Requirements**: MTP-01, MTP-02, MTP-03, MTP-04
**Sources**: CON-11 `frozen-mtp-and-vtg.md` (rollout §25.13 — 5 phases; budget §25.5, candidate schema §25.7, verification §25.9, advertisement §25.10, outcome events §25.12, router policy §25.13.4); ADR-02 (MNN execution)
**Success Criteria** (what must be TRUE):
  1. The Micro-MTP head operates within the §25.5 budget envelope on a frozen backbone, and candidate records validate against the §25.7 schema
  2. Every MTP candidate passes mandatory local verification (§25.9) before use
  3. Node capability advertisement includes MTP capability, and EGGROLL outcome events (§25.12) are emitted with results
  4. The router policy selects among generation/VTG/rule-drafter/MTP/tree/diffusion per §25.13.4
**Plans**: TBD

### Phase 11: EGGROLL Swarm Retraining
**Milestone**: v1.3
**Goal**: The swarm improves its specialists — deterministic perturbation reconstruction, locality-aware beehives, and reputation-gated canary promotion over the canonical decision corpus.
**Depends on**: Phase 4, Phase 2
**Requirements**: EGG-01, EGG-02, EGG-03, EGG-04, EGG-05
**Sources**: CON-07 `eggroll-swarm-retraining.md` (rollout §19.18 — 5 phases); CON-05 (specialist/adapter retraining via EGGROLL as routing target); ADR-02 §2 (GNUS-NEO-SWARM RuntimeCoordinator composes; no separate runtime)
**Success Criteria** (what must be TRUE):
  1. Seed-addressed perturbation reconstruction reproduces training artifacts deterministically, and compact signed fitness packets validate against the JSON schema
  2. Beehives form as locality-aware sub-swarms over the GNUS processing room mapping
  3. Artifacts distribute via IPFS-lite, coordination runs over libp2p pub/sub, and results transport via gRPC
  4. Reputation-gated validation and promotion operate over the canonical decision corpus with Distillation View Manifest identity lists (StateShard/BranchShard persist), safety/governance policy hashes gate promotions, and a canary promotion succeeds end-to-end
**Plans**: TBD

### Phase 12: Forecast-Driven Cognition
**Milestone**: v1.3
**Goal**: The system anticipates — ACE forecasts what is needed, CES prefetches under budgets and privacy policy, and forecast quality feeds EGGROLL — with a deterministic fallback when forecasting is off.
**Depends on**: Phase 11, Phase 7, Phase 2
**Requirements**: FCST-01, FCST-02, FCST-03, FCST-04, FCST-05, FCST-06
**Sources**: CON-10 `forecast-driven-cognition.md` §28.16–28.22 (rollout §28.21 — 5 phases; stage 5 multimodal deferred as "future" per source)
**Success Criteria** (what must be TRUE):
  1. ACE emits ranked, calibrated forecast candidates, and the system behaves correctly with ACE unavailable/disabled (deterministic fallback)
  2. Forecast Graph records carry confidence/horizon/resources/cost/scope/cancellation per schema
  3. CES schedules preparation within budgets and reclaims speculative resources on cancellation
  4. GAML forecast-candidate/prefetch/commit/release/outcome interfaces round-trip, and distributed forecast requests carry cancellation token + expiration
  5. Privacy enforcement precedes speculative retrieval/remote preparation; tests cover over-prefetching, cancellation, cache pressure, privacy boundaries, and turn-taking; EGGROLL metrics record forecast quality
**Plans**: TBD

### Phase 13: Local Cognitive Second Brain
**Milestone**: v1.4
**Goal**: A user's local corpus becomes a private cognitive memory — connectors ingest, the Second Brain Agent assembles context packets and answers with provenance, and a human-readable mirror makes it all inspectable.
**Depends on**: Phase 5, Phase 2, Phase 11
**Requirements**: CSB-01, CSB-02, CSB-03, CSB-04, CSB-05, CSB-06, CSB-07, CSB-08
**Sources**: `local-cognitive-second-brain.md` §27.16 (all 8 implementation requirements); CON-02 (GAML substrate)
**Success Criteria** (what must be TRUE):
  1. Local connectors ingest files/notes/email/calendar/transcripts into the GAML second-brain schemas (people/orgs/projects/decisions/commitments/deadlines/tasks/facts/claims/preferences/style/contradictions/traces)
  2. The Second Brain Agent answers a question from an assembled context packet citing memory sources, invoking local Expert Models and permitted tools, and writes outcomes back
  3. The supporting-agent pipeline runs observation → ingestion → extraction → entity resolution → scoring → permission checks → verification → contradiction detection → writeback → EGGROLL signals
  4. The human-readable memory mirror renders what the system knows and where it came from
  5. Privacy modes enforce (local-only default), and validation catches stale memory, conflicting commitments, ungrounded claims, and permission violations
**Plans**: TBD
**UI hint**: yes

### Phase 14: Epistemic Arbitration & Cognitive OS
**Milestone**: v1.4
**Goal**: The requestor node arbitrates disputed cognition — GQHSM state machines defined in data, a plugin ABI for out-of-tree arbiters, and at least one non-trivial arbitration framework resolving real disputes with memory writeback.
**Depends on**: Phase 1, Phase 2, Phase 11
**Requirements**: EPI-01, EPI-02, EPI-03, EPI-04
**Sources**: CON-08 `epistemic-arbitration-and-cognitive-os.md` (GQHSM schema §21.14.2, callback/guard vocabulary §21.15, plugin C++ ABI §21.16.3, EpistemicContext §21.18.1, thinking-trace schema §21.20.1)
**Success Criteria** (what must be TRUE):
  1. GQHSM hierarchical state machines defined in JSON validate against the §21.14.2 schema and coordinate expert-stage lifecycle on the requestor node
  2. The generic callback/guard vocabulary drives arbiter transitions without core changes, and the plugin C++ ABI loads an out-of-tree arbiter
  3. EpistemicContext carries all required fields, and thinking traces validate against the §21.20.1 JSON schema
  4. At least one arbitration framework (Sanskrit Nyaya/pramana, Kripke modal, or hybrid) resolves a disputed-synthesis scenario with memory writeback and a retraining signal
**Plans**: TBD

### Phase 15: Execution Integrity System (draft-gated)
**Milestone**: v1.4
**Goal**: The swarm can trust execution claims — contracts verify declared lineage and configuration, spot-checks catch dishonest execution, and fraud routes to slashing — without blocking the interactive path.
**Depends on**: Phase 9, Phase 10, Phase 4
**Requirements**: EIS-01, EIS-02, EIS-03, EIS-04
**Sources**: CON-09 `execution-integrity-system.md` — **"Status: Draft for review" with 7 open items (§29.9: checkpoint stride, epsilon calibration, Class C variance, KV checkpointing, adapter hot-swap windows, driver/firmware identity, synchronous verification policy). Do NOT plan this phase until the spec exits draft and the open items resolve.**
**Success Criteria** (what must be TRUE):
  1. Execution contracts verify declared model lineage, expert artifact, adapter/decision head/readout, SGFP4 container, processor contract, tokenizer/template contract, kernel manifest, determinism class, sampling/decision config, and execution profile
  2. Checkpoint-band matching and teacher-forced spot-checks detect a seeded dishonest execution in the test harness
  3. Sampling-consistency verification and EJM decision head/readout substitution checks pass
  4. Fraud verdicts route to slashing, calibration records store as Cognitive Assets, and verification runs sampled/asynchronously without blocking the interactive path
**Plans**: TBD

## Progress

**Execution Order:**
Phases execute in numeric order: 1 → 2 → ... → 15. Phase 1 must be sequenced against app workstream execution (shared repo structure). Phase 6 is blocked on the §26.26 open questions; Phase 15 is blocked on the EIS draft exit.

| Phase | Milestone | Plans Complete | Status | Completed |
|-------|-----------|----------------|--------|-----------|
| 1. SDK & Module Structure | v1.0 | 0/TBD | Not started | - |
| 2. GAML v1 Memory Core | v1.0 | 0/TBD | Not started | - |
| 3. Secure Execution Boundary | v1.0 | 0/TBD | Not started | - |
| 4. Reputation & Consensus Core | v1.0 | 0/TBD | Not started | - |
| 5. Capability System | v1.1 | 0/TBD | Not started | - |
| 6. OpenAI-Compatible API Router MVP | v1.1 | 0/TBD | Blocked (open questions §26.26) | - |
| 7. Context Lifecycle & Caching | v1.2 | 0/TBD | Not started | - |
| 8. Objective Memory / VTG | v1.2 | 0/TBD | Not started | - |
| 9. Speculative Decoding | v1.2 | 0/TBD | Not started | - |
| 10. Frozen Micro-MTP | v1.2 | 0/TBD | Not started | - |
| 11. EGGROLL Swarm Retraining | v1.3 | 0/TBD | Not started | - |
| 12. Forecast-Driven Cognition | v1.3 | 0/TBD | Not started | - |
| 13. Local Cognitive Second Brain | v1.4 | 0/TBD | Not started | - |
| 14. Epistemic Arbitration & Cognitive OS | v1.4 | 0/TBD | Not started | - |
| 15. Execution Integrity System | v1.4 | 0/TBD | Blocked (EIS draft) | - |
