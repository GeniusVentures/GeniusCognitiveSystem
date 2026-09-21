# Requirements: Genius Cognitive System — System Workstream

**Defined:** 2026-09-21
**Core Value:** A distributed, reputation-weighted cognitive system in which Expert Models execute behind a mandatory security boundary, adapt through swarm retraining, and serve local second-brain users and public OpenAI-compatible API consumers — all behind a stable embeddable SDK.
**Source:** Derived from `docs/architecture/` ingest intel (`.planning/intel/requirements.md`, `constraints.md`, `decisions.md`) — 0 PRD-type docs existed; these are SPEC-derived requirements. Authority is SPEC-level (below the two LOCKED ADRs).

## Requirements by Milestone

### v1.0 System Substrate

#### Structure & SDK (STRUCT)

Source: ADR-01 (LOCKED) + CON-01 `agent-module-development-inventory.md`

- [ ] **STRUCT-01**: Root CMake delegates via `add_subdirectory()` to per-module owners; targets `gcs_common`, `gcs_runtime`, `gcs_chat_core`, `gcs_api`, `gcs_ffi`, `neoswarm_runtime` each declare only direct dependencies (ADR-01 §4–5)
- [ ] **STRUCT-02**: `include/gcs/` exposes public runtime request/event/status/result/artifact contracts, abstract `RuntimeCoordinator`, `GeniusCognitiveSystem` facade, chat session/streaming client contracts, and the stable C ABI; heavy deps (MNN, Vulkan/MoltenVK, SGProcessingManager, SuperGenius, libp2p, RocksDB) never leak through (ADR-01 §1, §6)
- [ ] **STRUCT-03**: Flutter consumes backend-neutral `packages/{gcs_client,gcs_chat,gcs_native}`; the `neoswarm_ffi` pubspec dependency is removed via migration, not deletion of function (ADR-01 §3 + consequences)
- [ ] **STRUCT-04**: Every CON-01 §31.21 inventory item (agent, deterministic service, store, connector, security service, distributed infrastructure) maps to an owning module with trust tier and validation gate

#### Agentic Memory — GAML v1 (GAML)

Source: CON-02 `agentic-memory-layer.md`

- [ ] **GAML-01**: MemoryObject and Cognitive Asset models implement the CON-02 type set (Fact, Goal, Constraint, Policy, Bridge Block, Procedure, Tool Result, Capability, Connector, Plan, Execution Claim, Checkpoint Calibration, Verification Result, Execution Verdict, Arbitration Result, Consensus Record, Benchmark Result, Distillation Sample, Specialist Trace)
- [ ] **GAML-02**: Privacy scopes (7 classes, local-only → public) are enforced on every read/write; scope-crossing access is rejected
- [ ] **GAML-03**: Ingestion pipeline, staged retrieval, and surprise-gated writes operate end-to-end on a local corpus
- [ ] **GAML-04**: CRDT replication converges memory state across nodes; swarm memory consensus produces verifiable Consensus Records

#### Secure Execution (SEC)

Source: CON-15 `secure-agent-architecture.md` + REQ-secure-agent-targets (§18.1.1.2)

- [ ] **SEC-01**: 100% of tool executions have a valid Tool Intermediary attestation
- [ ] **SEC-02**: 0 direct side-effect executions from Semantic Core or Expert Model workers
- [ ] **SEC-03**: 100% of durable memory writes derived from tools/external content contain provenance metadata and policy-compatible trust classification
- [ ] **SEC-04**: Tool Intermediary overhead and swarm overhead are each bounded and reported separately from reasoning latency
- [ ] **SEC-05**: Instruction scrubber + trap detection, approval gates on side-effect proposals, and node trust tiers A–D are enforced

#### Reputation & Consensus (REP)

Source: CON-14 `reputation-consensus.md`

- [ ] **REP-01**: Role/domain-aware node reputation scores persist (wallet-core + RocksDB) and sync via CRDT
- [ ] **REP-02**: Weighted updates (accuracy/quality, latency, consistency, safety — including safety penalties per CON-03) apply clipped to [0,1]
- [ ] **REP-03**: Weighted consensus reaches decisions under the Byzantine tolerance and liveness rules of the spec's test scenarios
- [ ] **REP-04**: Reputation-gated participation excludes sub-threshold nodes; genesis anchor bootstrap nodes bring up a fresh swarm

### v1.1 External Surfaces

#### Capability System (CAP)

Source: CON-04 `capability-system.md` §30.14 (10 requirements, count preserved from SPEC)

- [ ] **CAP-01**: Canonical schemas exist for Capability/Connector/CapabilityProvider/Permission/CapabilityContract/CredentialReference
- [ ] **CAP-02**: Connector registry and discovery service are operational
- [ ] **CAP-03**: MCP adapter translates external tools into proposed canonical contracts
- [ ] **CAP-04**: Deterministic validators and contract hashing gate every registered capability
- [ ] **CAP-05**: All capability execution routes through the Tool Intermediary
- [ ] **CAP-06**: Local read-only connector retrieves files/notes/calendar/email
- [ ] **CAP-07**: Remote read-only connector works via MCP or OpenAPI
- [ ] **CAP-08**: Provider/connector health and reputation are tracked in the registry
- [ ] **CAP-09**: Manifest drift detection triggers revocation
- [ ] **CAP-10**: Administrative review is required for new/changed capabilities

#### Public API (API)

Source: CON-13 `openai-compatible-api-router-and-gcs-job-queue.md` §26.3–26.25

- [ ] **API-01**: A standard OpenAI SDK completes a call against `https://api.gnus.ai/v1/chat/completions` (Cloudflare ingress + API key auth)
- [ ] **API-02**: Requests convert to signed GCSApiRequestJob; gateway publishes to the GCS network; registered nodes claim (first-valid-claim for MVP) under lease; failed jobs requeue
- [ ] **API-03**: Stream chunks convert to incremental OpenAI-compatible SSE (not buffered); client disconnect cancels the GCS job
- [ ] **API-04**: Slow clients cannot cause unbounded proxy memory growth (backpressure enforced)
- [ ] **API-05**: Non-streaming requests return OpenAI-compatible JSON
- [ ] **API-06**: Usage is recorded per tenant/project/API key/node with generated vs delivered tokens distinguished
- [ ] **API-07**: Existing processing chunk jobs keep working; API jobs can spawn child processing jobs
- [ ] **API-08**: Routing policy (private/hybrid) is represented in the job envelope

**Open questions (§26.26 — recorded open, NOT commitments; resolve at Phase 6 planning):** lease reuse vs separate job type; single claim validator vs CRDT-visible; alias scoping; `gpt-4o`-style aliases; inline payloads vs gateway references; minimum settlement record; host repo (GCS vs SuperGenius vs separate); protobuf-first vs JSON-first; post-token stream failure representation.

**MVP non-goals (§26.4, out of scope):** Assistants/fine-tuning API compat, image gen, realtime audio, arbitrary external tool execution, full distributed token-by-token decoding, multi-node speculative decoding, global consensus per response, perfect pre-pricing, full zk verification, marketplace bidding, automatic private routing without tenant policy.

### v1.2 Inference Efficiency

#### Context Lifecycle & Caching (CTX)

Source: CON-06 `context-lifecycle-caching-governance.md` §17.23–17.25

- [ ] **CTX-01**: Deterministic Context Compiler with versioned canonicalization produces byte-identical stable prefixes for identical inputs; changing only the current request does not change the prefix hash
- [ ] **CTX-02**: Prefix cache identity/compatibility spans tenant, privacy, authorization, model/tokenizer/quantization/runtime/adapter hashes, policy hashes, capability contract set hash, and canonicalization version; incompatible reuse and cross-tenant/privacy reuse via content-hash equality alone are prevented
- [ ] **CTX-03**: Tenant/privacy-scoped cache lifecycle, invalidation, and revocation purge affected cache entries and derived context
- [ ] **CTX-04**: Hierarchical token allocation reserves completion tokens before optional allocation; output tokens cannot be consumed by retrieval/expansion without explicit fallback
- [ ] **CTX-05**: Every injected item carries inclusion reason, source, privacy scope, trust class, and token count, attributable via the Context Inspector without exposing unauthorized content
- [ ] **CTX-06**: Every specialist returns a bounded ExpertDigest; capability schemas load lazily; no volatile data precedes the cache boundary
- [ ] **CTX-07**: Compaction (task-boundary/budget/emergency) preserves objectives/constraints/decisions/questions/evidence/tool state/privacy, with failed compaction restoring the prior generation
- [ ] **CTX-08**: Cache hit/miss/bypass/invalidation/churn metrics and context-efficiency regression tests exist; cache-affinity is a non-authoritative ranking signal; efficiency changes pass the same quality/safety suite as baseline

#### Objective Memory / VTG (VTG)

Source: CON-12 `objective-memory-vtg.md` (rollout §23.21)

- [ ] **VTG-01**: Transition edge and learning-event models validate against the JSON schemas; state identity (hash/HMAC construction) verifies
- [ ] **VTG-02**: Candidate frontier update semantics resist poisoning under adversarial test inputs
- [ ] **VTG-03**: Storage stack persists content-addressed data across RocksDB, IPFS-lite, and CRDT replication
- [ ] **VTG-04**: Tenant privacy scopes enforced; explicit non-goals (§23.22) hold — VTG is not a GAML/Semantic Core replacement, not an unverified output cache, and cannot bypass policy

#### Speculative Decoding (SDEC)

Source: CON-17 `speculative-decoding-and-vtg.md` (rollout §24.15; baseline §24.16)

- [ ] **SDEC-01**: Drafter backend classes (VTG lookup, rule/grammar/schema, Frozen Micro-MTP, micro-diffusion, tiny causal tree) plug in behind a common verification interface
- [ ] **SDEC-02**: Confidence-scheduled verification accepts/rejects drafts with prefix retention; speculative output matches non-speculative decoding on the test suite
- [ ] **SDEC-03**: Operating budgets hold on target nodes (100–350MB active model, 5–50MB drafter, 10–100MB VTG hot shard)
- [ ] **SDEC-04**: Node capability advertisement (JSON schema) is consumed by the Router; signed (ed25519) swarm outcome events are recorded
- [ ] **SDEC-05**: Role-specific speculation policy and backend-selection ordering implemented; production baseline is compact local speculation (no distributed speculation)

#### Frozen Micro-MTP (MTP)

Source: CON-11 `frozen-mtp-and-vtg.md` (rollout §25.13)

- [ ] **MTP-01**: Micro-MTP head operates within the §25.5 budget envelope on a frozen backbone; candidate records validate against the §25.7 schema
- [ ] **MTP-02**: Mandatory local verification (§25.9) gates every MTP candidate before use
- [ ] **MTP-03**: Node capability advertisement (§25.10) includes MTP capability; EGGROLL outcome events (§25.12) are emitted with results
- [ ] **MTP-04**: Router policy chooses among generation/VTG/rule-drafter/MTP/tree/diffusion per §25.13.4

### v1.3 Adaptive Cognition

#### EGGROLL Swarm Retraining (EGG)

Source: CON-07 `eggroll-swarm-retraining.md` (rollout §19.18)

- [ ] **EGG-01**: Deterministic perturbation reconstruction (seed-addressed) reproduces training artifacts; compact signed fitness packets validate against the JSON schema
- [ ] **EGG-02**: Beehives form as locality-aware sub-swarms over the GNUS processing room mapping
- [ ] **EGG-03**: Artifacts distribute via IPFS-lite; coordination via libp2p pub/sub; results via gRPC
- [ ] **EGG-04**: Reputation-gated validation and promotion operate over the canonical decision corpus with Distillation View Manifest identity lists; StateShard/BranchShard data models persist
- [ ] **EGG-05**: Safety/governance policy hashes gate promotions; canary promotion succeeds end-to-end

#### Forecast-Driven Cognition (FCST)

Source: CON-10 `forecast-driven-cognition.md` §28.16–28.22 (rollout §28.21; stage 5 multimodal deferred — see Out of Scope)

- [ ] **FCST-01**: Anticipatory Cognition Engine (ACE) emits ranked, calibrated forecast candidates; a deterministic fallback works when ACE is unavailable/disabled
- [ ] **FCST-02**: Forecast Graph records carry confidence/horizon/resources/cost/scope/cancellation per schema
- [ ] **FCST-03**: Cognitive Execution Scheduler (CES) applies budgets, schedules preparation, and reclaims speculative resources
- [ ] **FCST-04**: GAML forecast-candidate/prefetch/commit/release/outcome interfaces round-trip; distributed forecast requests carry cancellation token + expiration
- [ ] **FCST-05**: Anticipatory distillation trace/label schemas capture outcomes; EGGROLL metrics record forecast quality
- [ ] **FCST-06**: Privacy enforcement precedes speculative retrieval/remote preparation; full observability; tests cover over-prefetching, cancellation, cache pressure, privacy boundaries, and turn-taking

### v1.4 Higher Cognition & Integrity

#### Local Cognitive Second Brain (CSB)

Source: `local-cognitive-second-brain.md` §27.16 (8 requirements, count preserved)

- [ ] **CSB-01**: Local source connectors ingest files/notes/email/calendar/transcripts
- [ ] **CSB-02**: GAML schemas cover people/orgs/projects/decisions/commitments/deadlines/tasks/facts/claims/preferences/style/contradictions/traces
- [ ] **CSB-03**: Second Brain Agent performs retrieval, context-packet assembly, local Expert Model invocation, permitted tool use, and writeback
- [ ] **CSB-04**: Supporting agents run observation, ingestion, extraction, entity resolution, scoring, permission checks, verification, contradiction detection, writeback, and EGGROLL signal emission
- [ ] **CSB-05**: Human-readable memory mirror renders what the system knows with provenance
- [ ] **CSB-06**: Privacy modes (local-only, private enterprise, hybrid, explicit swarm) are enforced
- [ ] **CSB-07**: EGGROLL adaptation signals are emitted from second-brain outcomes
- [ ] **CSB-08**: Validation catches stale memory, conflicting commitments, source grounding failures, and permission boundary violations

#### Epistemic Arbitration & Cognitive OS (EPI)

Source: CON-08 `epistemic-arbitration-and-cognitive-os.md`

- [ ] **EPI-01**: GQHSM hierarchical state machines defined in JSON (schema §21.14.2) coordinate expert-stage lifecycle on the requestor node
- [ ] **EPI-02**: Generic callback/guard vocabulary (§21.15) drives arbiter transitions; the plugin C++ ABI (§21.16.3) loads an out-of-tree arbiter
- [ ] **EPI-03**: EpistemicContext carries all required fields (§21.18.1); thinking traces validate against the §21.20.1 JSON schema
- [ ] **EPI-04**: At least one arbitration framework (Sanskrit Nyaya/pramana, Kripke modal, or hybrid) resolves a disputed-synthesis scenario with memory writeback and retraining signal integration

#### Execution Integrity System (EIS)

Source: CON-09 `execution-integrity-system.md` — **DRAFT-SPEC GATED: self-declares "Status: Draft for review" with 7 open items (§29.9). Do not plan this phase until the spec exits draft.**

- [ ] **EIS-01**: Execution contracts verify declared model lineage, expert artifact, adapter/decision head/readout, SGFP4 container, processor contract, tokenizer/template contract, kernel manifest, determinism class, sampling/decision config, and execution profile
- [ ] **EIS-02**: Determinism classes, checkpoint-band matching, and teacher-forced spot-checks detect a seeded dishonest execution in the test harness
- [ ] **EIS-03**: Sampling-consistency verification and EJM decision head/readout substitution checks pass
- [ ] **EIS-04**: Fraud verdicts route to slashing; calibration records store as Cognitive Assets; verification runs sampled/asynchronous without blocking the interactive path

## Out of Scope

Explicitly excluded. Documented to prevent scope creep.

| Item | Reason |
|------|--------|
| Legacy 4-phase roadmap (`roadmap-and-risks.md`) | Superseded (user decision 2026-09-21); per-topic SPEC rollout plans are the roadmap source |
| Sloth/Unsloth integration | Exploratory only; no ADR assigns training-stack ownership |
| SGFP4 codec implementation | MNN-owned per ADR-02; only integration points tracked here |
| Forecast stage 5 (multimodal evidence) | Source marks it "future" (§28.21); revisit at v1.3 close |
| GQHSM WASM extension path | Explicitly future in CON-08 |
| API router MVP non-goals (§26.4) | Assistants/fine-tuning compat, image gen, realtime audio, distributed token-by-token decoding, multi-node speculation, global per-response consensus, zk verification, marketplace bidding |
| App workstream scope | Owned by `.planning/workstreams/app/` — this workstream must not scope-change it |

## Traceability

Which phases cover which requirements. Updated during roadmap creation.

| Requirement | Milestone | Phase | Status |
|-------------|-----------|-------|--------|
| STRUCT-01 | v1.0 | Phase 1 | Pending |
| STRUCT-02 | v1.0 | Phase 1 | Pending |
| STRUCT-03 | v1.0 | Phase 1 | Pending |
| STRUCT-04 | v1.0 | Phase 1 | Pending |
| GAML-01 | v1.0 | Phase 2 | Pending |
| GAML-02 | v1.0 | Phase 2 | Pending |
| GAML-03 | v1.0 | Phase 2 | Pending |
| GAML-04 | v1.0 | Phase 2 | Pending |
| SEC-01 | v1.0 | Phase 3 | Pending |
| SEC-02 | v1.0 | Phase 3 | Pending |
| SEC-03 | v1.0 | Phase 3 | Pending |
| SEC-04 | v1.0 | Phase 3 | Pending |
| SEC-05 | v1.0 | Phase 3 | Pending |
| REP-01 | v1.0 | Phase 4 | Pending |
| REP-02 | v1.0 | Phase 4 | Pending |
| REP-03 | v1.0 | Phase 4 | Pending |
| REP-04 | v1.0 | Phase 4 | Pending |
| CAP-01 | v1.1 | Phase 5 | Pending |
| CAP-02 | v1.1 | Phase 5 | Pending |
| CAP-03 | v1.1 | Phase 5 | Pending |
| CAP-04 | v1.1 | Phase 5 | Pending |
| CAP-05 | v1.1 | Phase 5 | Pending |
| CAP-06 | v1.1 | Phase 5 | Pending |
| CAP-07 | v1.1 | Phase 5 | Pending |
| CAP-08 | v1.1 | Phase 5 | Pending |
| CAP-09 | v1.1 | Phase 5 | Pending |
| CAP-10 | v1.1 | Phase 5 | Pending |
| API-01 | v1.1 | Phase 6 | Pending |
| API-02 | v1.1 | Phase 6 | Pending |
| API-03 | v1.1 | Phase 6 | Pending |
| API-04 | v1.1 | Phase 6 | Pending |
| API-05 | v1.1 | Phase 6 | Pending |
| API-06 | v1.1 | Phase 6 | Pending |
| API-07 | v1.1 | Phase 6 | Pending |
| API-08 | v1.1 | Phase 6 | Pending |
| CTX-01 | v1.2 | Phase 7 | Pending |
| CTX-02 | v1.2 | Phase 7 | Pending |
| CTX-03 | v1.2 | Phase 7 | Pending |
| CTX-04 | v1.2 | Phase 7 | Pending |
| CTX-05 | v1.2 | Phase 7 | Pending |
| CTX-06 | v1.2 | Phase 7 | Pending |
| CTX-07 | v1.2 | Phase 7 | Pending |
| CTX-08 | v1.2 | Phase 7 | Pending |
| VTG-01 | v1.2 | Phase 8 | Pending |
| VTG-02 | v1.2 | Phase 8 | Pending |
| VTG-03 | v1.2 | Phase 8 | Pending |
| VTG-04 | v1.2 | Phase 8 | Pending |
| SDEC-01 | v1.2 | Phase 9 | Pending |
| SDEC-02 | v1.2 | Phase 9 | Pending |
| SDEC-03 | v1.2 | Phase 9 | Pending |
| SDEC-04 | v1.2 | Phase 9 | Pending |
| SDEC-05 | v1.2 | Phase 9 | Pending |
| MTP-01 | v1.2 | Phase 10 | Pending |
| MTP-02 | v1.2 | Phase 10 | Pending |
| MTP-03 | v1.2 | Phase 10 | Pending |
| MTP-04 | v1.2 | Phase 10 | Pending |
| EGG-01 | v1.3 | Phase 11 | Pending |
| EGG-02 | v1.3 | Phase 11 | Pending |
| EGG-03 | v1.3 | Phase 11 | Pending |
| EGG-04 | v1.3 | Phase 11 | Pending |
| EGG-05 | v1.3 | Phase 11 | Pending |
| FCST-01 | v1.3 | Phase 12 | Pending |
| FCST-02 | v1.3 | Phase 12 | Pending |
| FCST-03 | v1.3 | Phase 12 | Pending |
| FCST-04 | v1.3 | Phase 12 | Pending |
| FCST-05 | v1.3 | Phase 12 | Pending |
| FCST-06 | v1.3 | Phase 12 | Pending |
| CSB-01 | v1.4 | Phase 13 | Pending |
| CSB-02 | v1.4 | Phase 13 | Pending |
| CSB-03 | v1.4 | Phase 13 | Pending |
| CSB-04 | v1.4 | Phase 13 | Pending |
| CSB-05 | v1.4 | Phase 13 | Pending |
| CSB-06 | v1.4 | Phase 13 | Pending |
| CSB-07 | v1.4 | Phase 13 | Pending |
| CSB-08 | v1.4 | Phase 13 | Pending |
| EPI-01 | v1.4 | Phase 14 | Pending |
| EPI-02 | v1.4 | Phase 14 | Pending |
| EPI-03 | v1.4 | Phase 14 | Pending |
| EPI-04 | v1.4 | Phase 14 | Pending |
| EIS-01 | v1.4 | Phase 15 | Pending (draft-gated) |
| EIS-02 | v1.4 | Phase 15 | Pending (draft-gated) |
| EIS-03 | v1.4 | Phase 15 | Pending (draft-gated) |
| EIS-04 | v1.4 | Phase 15 | Pending (draft-gated) |

**Coverage:**
- Total requirements: 83
- Mapped to phases: 83
- Unmapped: 0

Cross-cutting SPEC constraints without dedicated requirements (cited in phases, tracked as constraints, not orphaned): CON-03 AI safety NFR (cited Phase 3/4 success criteria), CON-05 cognitive evolution control (cited Phases 2/11/12), CON-16 SGFP4 (ADR-02 boundary; cited Phases 9/10/15).

---
*Requirements defined: 2026-09-21*
*Last updated: 2026-09-21 after roadmap creation*
