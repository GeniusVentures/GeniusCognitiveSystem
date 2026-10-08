# Constraints (SPEC intel)

**Generated:** 2026-09-21 | **Mode:** merge (app workstream) | **Run:** 30 docs from `docs/architecture/`
**17 SPEC-type documents.** Types: schema (8), api-contract (4), protocol (4), nfr (1). All unlocked; all subordinate to the two LOCKED ADRs in `decisions.md`. No SPEC contradicts either ADR (pass-4 check clean).

---

## CON-01 — Agent and Module Development Inventory
- **Source:** `docs/architecture/agent-module-development-inventory.md` | **Type:** schema | **Refreshed:** 2026-09-21
- Implementation inventory consolidating every agent, deterministic service, store, connector, security service, and distributed infrastructure into buildable workstreams. Canonical schema list (§31.21), per-service API responsibilities, trust tiers, capability contract structure, deployment profiles, validation/test gates. Reference topology: ingress → identity/session → Executive Controller → intent/risk classifier → Router → Planner → execution graph compiler → policy/budget/scheduler → Semantic Core + Expert registries (role-based, domain-specific, private/local, evaluation harness) → forecast subsystem (ACE, PFM, CES) → memory (GAML, VTG) → capability/tool intermediary → grounding/verification/arbitration/synthesis → reputation/consensus → EGGROLL → distributed infrastructure.
- Principle: prefer the smallest implementation satisfying behavior, trust boundary, performance, and deployment model; a component may start as a library, in-process service, daemon, or distributed GNUS service.

## CON-02 — GNUS Agentic Memory Layer (GAML v1)
- **Source:** `docs/architecture/agentic-memory-layer.md` | **Type:** schema | **Refreshed:** 2026-09-21
- MemoryObject and Cognitive Asset models (Fact, Goal, Constraint, Policy, Bridge Block, Procedure, Tool Result, Capability, Connector, Plan, Execution Claim, Checkpoint Calibration, Verification Result, Execution Verdict, Arbitration Result, Consensus Record, Benchmark Result, Distillation Sample, Specialist Trace). Privacy scopes (local-only → public, 7 classes), ingestion pipeline, staged retrieval, surprise-gated writes, CRDT replication, swarm memory consensus, EIS evidence stored as auditable Cognitive Assets. Positioned between Router/Planner, Memory Governor, expert execution, EIS, and grounding/verification.

## CON-03 — AI Safety Philosophy
- **Source:** `docs/architecture/ai-safety.md` | **Type:** nfr | **Last touched:** 2026-06-28 (pre-refresh generation)
- Decentralized node-local safety enforcement, 4 layers: node-level screening (authoritative), reputation penalties (Δreputation_safety = -λ × violation_score), client preference filtering, Tool Intermediary control. Versioned signed safety profiles via IPFS; no centralized safety gateway; no GeoIP enforcement. Note: framed in legacy "Genius ELM" terminology (see INGEST-CONFLICTS.md INFO on terminology drift).

## CON-04 — GCS Capability System
- **Source:** `docs/architecture/capability-system.md` | **Type:** api-contract | **Refreshed:** 2026-09-21
- Protocol-neutral canonical capability contract (YAML schema §30.4), discovery/contract translation, MCP connector adapter, connector categories, local capability execution, capability routing + connector reputation, credential references (no raw secrets), Tool Intermediary-governed execution, GAML Cognitive Asset integration, connector lifecycle with manifest drift detection/revocation, administrative review. Implementation requirements in REQ-capability-system.

## CON-05 — Cognitive Evolution Coordination
- **Source:** `docs/architecture/cognitive-evolution-control.md` | **Type:** protocol | **Refreshed:** 2026-09-21
- Signed Qualified Cognitive Events, governance resolution, event signature contract, EJM decision replay contract, promotion contract, Distillation Views, observability. Routing targets: GAML memory/policy updates, Objective Memory/VTG transitions, router/planner/verifier/arbitration tuning, specialist/adapter retraining via EGGROLL, benchmark/replay/canary/promotion workflows. Explicitly no separate runtime — RuntimeCoordinator (GNUS-NEO-SWARM implementation) remains responsible; ownership boundaries unchanged (aligns with D-ADR-02).

## CON-06 — Context Lifecycle, Caching, and Governance
- **Source:** `docs/architecture/context-lifecycle-caching-governance.md` | **Type:** schema (with nfr budgets) | **Refreshed:** 2026-09-21
- Normative MUST/SHOULD context-efficiency contract: Context Compiler, stable-prefix contract, prefix cache identity/compatibility (tenant, privacy, authorization, model/tokenizer/quantization/runtime/adapter hashes, policy hashes, capability contract set hash, canonicalization version), cache lifecycle/invalidation, hierarchical token allocation, off-window artifacts, ExpertDigest expert isolation, capability schema loading (lazy), compaction lifecycle (task-boundary/budget/emergency + rollback), Context Inspector, cache-affinity routing signal, verification tiers, security/privacy/trust requirements, failure modes with required fallbacks. Requirements/acceptance in REQ-context-lifecycle.

## CON-07 — EGGROLL Swarm Retraining Architecture
- **Source:** `docs/architecture/eggroll-swarm-retraining.md` | **Type:** protocol | **Refreshed:** 2026-09-21
- Deterministic perturbation reconstruction (seed-addressed), compact signed fitness packets (JSON schema), beehives as locality-aware sub-swarms, GNUS processing room mapping, IPFS-lite artifact distribution, libp2p pub/sub coordination, gRPC result transport, reputation-gated validation/promotion, canonical decision corpus with Distillation View Manifest identity lists, StateShard/BranchShard data models, safety/governance policy hashes, canary promotion. Rollout: 5 phases (§19.18).

## CON-08 — Epistemic Arbitration and Cognitive OS Extensions
- **Source:** `docs/architecture/epistemic-arbitration-and-cognitive-os.md` | **Type:** api-contract (plugin ABI; with JSON schemas) | **Refreshed:** 2026-09-21
- Requestor Node as epistemic arbiter: GQHSM JSON-defined hierarchical state machines (schema §21.14.2), generic callback/guard vocabulary (§21.15), plugin C++ ABI (§21.16.3), EpistemicContext required fields (§21.18.1), JSON thinking-trace schema (§21.20.1), Sanskrit (Nyaya/pramana), Kripke modal, and hybrid arbitration frameworks, memory writeback + retraining signal integration, future WASM extension path. Schemas framed as representative/example (classifier note). External refs: github.com/Super-Genius/GQHSM, GQHSM.h, EpistemicContext.h.

## CON-09 — Execution Integrity System (EIS)
- **Source:** `docs/architecture/execution-integrity-system.md` | **Type:** protocol | **Status: "Draft for review" (self-declared)** | **Refreshed:** 2026-09-21
- Verifies execution honesty (declared model lineage, expert artifact, adapter/decision head/readout, SGFP4 container, processor contract, tokenizer/template contract, kernel manifest, determinism class, sampling/decision config, execution profile) — not semantic answer quality. Execution contract field lists, determinism classes, checkpoint-band matching, teacher-forced spot-check protocol, sampling-consistency verification, EJM decision head/readout substitution checks, fraud verdicts + slashing, Cognitive Asset calibration records. 7 open items (checkpoint stride, epsilon calibration, Class C variance, KV checkpointing, adapter hot-swap windows, driver/firmware identity, synchronous verification policy). Normally sampled and asynchronous vs the interactive path.

## CON-10 — Forecast-Driven Cognition and Predictive Prefetching
- **Source:** `docs/architecture/forecast-driven-cognition.md` | **Type:** schema (with nfr budgets) | **Last touched:** 2026-07-19
- ACE forecasts, CES prefetches (memory, experts, tools, nodes) under bounded privacy-safe policies. GAML forecast/prefetch/commit/release interface (§28.16), distributed forecast request schema with cancellation token + expiration (§28.17), observability fields + evaluation metrics (§28.20), Forecast Graph schema, 12-item implementation requirements (§28.22). Requirements in REQ-forecast-cognition.

## CON-11 — Frozen Micro-MTP and VTG Edge Inference
- **Source:** `docs/architecture/frozen-mtp-and-vtg.md` | **Type:** schema | **Refreshed:** 2026-09-21
- Small multi-token prediction head on frozen Semantic Core/ELM backbones for constrained edge nodes; Micro-MTP budget envelope (§25.5), candidate record schema (§25.7), mandatory local verification (§25.9), node capability advertisement (§25.10), EGGROLL outcome event schema (§25.12), router policy choosing among generation/VTG/rule-drafter/MTP/tree/diffusion (§25.13.4). Rollout: 5 phases. Note: member of a cross-ref cycle — see INGEST-CONFLICTS.md WARNING.

## CON-12 — Objective Memory and Verified Transition Graph (VTG)
- **Source:** `docs/architecture/objective-memory-vtg.md` | **Type:** schema | **Refreshed:** 2026-09-21
- Verified transition substrate: transition edge model + learning-event JSON schemas, state identity (hash/HMAC construction), candidate frontier, storage stack (content-addressed, RocksDB, IPFS-lite, CRDT replication), update semantics, tenant privacy scopes, poisoning resistance, performance metrics. Explicit non-goals list (§23.22): not a replacement for GAML/Semantic Core/ELMs/grounding/arbitration; not an unverified output cache; cannot bypass policy. Rollout: 6 phases (§23.21).

## CON-13 — OpenAI-Compatible API Router and GCS Job Queue
- **Source:** `docs/architecture/openai-compatible-api-router-and-gcs-job-queue.md` | **Type:** api-contract | **Last touched:** 2026-07-19
- Endpoint surface, signed job/claim/stream/result JSON schemas, proto data model, CRDT keyspace, pub/sub channels, claim/lease semantics, node registration + heartbeat, streaming SSE proxy requirements (backpressure, ordering, disconnect cancellation, partial usage), fairness/democratized pickup, requeue/retry, metering/rewards/settlement, private/hybrid routing. Self-labels "Product Technical Design Specification". Requirements/acceptance/open-questions in REQ-openai-api-mvp. Introduces Cloudflare Edge ingress + GCS Gateway Node (see INGEST-CONFLICTS.md INFO scope note vs app "no central servers" constraint).

## CON-14 — Reputation-Based Consensus System
- **Source:** `docs/architecture/reputation-consensus.md` | **Type:** protocol | **Last touched:** 2026-06-28 (pre-refresh generation)
- Node reputation data model (role/domain-aware scores; wallet-core + RocksDB + CRDT), weighted update formulas (accuracy/quality, latency, consistency, safety; clipped [0,1]), weighted consensus algorithm, consensus engine architecture, swarm execution flow, consensus message types, Byzantine tolerance + liveness rules, reputation-gated participation, genesis anchor bootstrap nodes, requestor-orchestrator model. Framed in legacy role names (Planner_score, Math_score) — see terminology-drift INFO.

## CON-15 — Secure Agent Architecture
- **Source:** `docs/architecture/secure-agent-architecture.md` | **Type:** api-contract | **Refreshed:** 2026-09-21
- Mandatory Tool Intermediary choke-point (tool proposals, deterministic dry-run, sanitization, attestation, approval gates), Execution Plan schema, Router/Planner service, structured memory trust classes (higher/lower-trust), node trust tiers A–D, capability manifests, zero-trust sandboxing (Firecracker micro-VM), CRDT memory replication, instruction scrubber + trap detection, task settlement/attestations. Self-declares it supersedes the older "Chat Genius"/MoA framing. Security targets in REQ-secure-agent-targets.

## CON-16 — SGFP4 Adaptive Quantization Format
- **Source:** `docs/architecture/sgfp4-format.md` | **Type:** schema | **Last touched:** 2026-07-19
- 64x64 macroblocks with fixed 2048-byte payloads; container layout (headers uint32[], offsets uint32[] with flags-in-offsets low 4 bits, codes_blob); per-block FP16 scale+bias affine decode (`w_hat = S*code + Bias`); dual modes FP4_AFFINE / T158_AFFINE selected by 32-step scale search; GPU decode procedure (Vulkan/MoltenVK, MNN); MUST-alignment constraints. Consistent with D-ADR-02 (weight format; native execution in MNN). Note: member of a cross-ref cycle — see INGEST-CONFLICTS.md WARNING.

## CON-17 — Speculative Decoding and VTG Candidate Scheduling
- **Source:** `docs/architecture/speculative-decoding-and-vtg.md` | **Type:** schema | **Last touched:** 2026-09-17
- Micro-speculation on constrained nodes: drafter backend classes (VTG lookup, rule/grammar/schema, Frozen Micro-MTP, micro-diffusion, tiny causal tree/JetSpec-style), confidence-scheduled verification + prefix retention, VTG hot shards, operating budgets (100–350MB active model, 5–50MB drafter, 10–100MB hot shard), node capability advertisement JSON schema consumed by Router, signed (ed25519) swarm outcome event schema, role-specific speculation policy, backend-selection ordering. Scope boundary: production baseline is compact local speculation (§24.16). Rollout: 6 phases. Note: member of a cross-ref cycle — see INGEST-CONFLICTS.md WARNING.
