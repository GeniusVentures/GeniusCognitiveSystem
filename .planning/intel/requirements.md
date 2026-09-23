# Requirements (PRD intel)

**Generated:** 2026-09-21 | **Mode:** merge (app workstream) | **Run:** 30 docs from `docs/architecture/`

**PRD-type documents in this ingest set: 0.** No user-story/acceptance-criteria PRDs were classified.

To keep requirement-flavored content routable for `gsd-roadmapper`, the acceptance-criteria and implementation-requirement sections embedded in SPEC documents are extracted below as **derived requirements**. They are preserved per source with all variants intact — nothing is merged or deduplicated across sources. Their authority is SPEC-level (below ADR), not PRD-level.

---

## REQ-context-lifecycle — Context efficiency contract

- **Source:** `docs/architecture/context-lifecycle-caching-governance.md` (§17.23 Implementation Requirements, §17.24 Acceptance Criteria, §17.25 Rollout Plan)
- **Scope:** Context Compiler, Prefix Cache Manager, Context Compaction Manager, ExpertDigest, Context Inspector
- **Implementation requirements (15, MUST):** deterministic Context Compiler with versioned canonicalization; explicit stable-prefix/volatile-suffix boundary; tenant/privacy-scoped Prefix Cache Manager; completion-token reservation before optional allocation; explainable inclusion decisions with per-section token attribution; duplicate/supersession filtering; off-window artifact references; bounded ExpertDigest contract; task-boundary/budget/emergency compaction modes; compaction preservation checks with rollback; Context Inspector; cache hit/miss/bypass/invalidation/churn metrics; context-efficiency regression tests; purge/revocation for private derived context; cache-affinity as non-authoritative ranking signal.
- **Acceptance (15 items, key):** byte-identical stable prefix for identical inputs; changing only the current request does not change the prefix hash; model/tokenizer/adapter/runtime/policy/contract/tenant/privacy/version changes prevent incompatible reuse; no volatile data before the cache boundary; every injected item carries inclusion reason, source, privacy scope, trust class, token count; output tokens cannot be consumed by retrieval/expansion without explicit fallback; every specialist returns a bounded ExpertDigest; compaction preserves objectives/constraints/decisions/questions/evidence/tool state/privacy; failed compaction restores prior generation; cache entries cannot cross tenant/privacy boundaries via content-hash equality alone; revocation purges affected cache/derived context; Inspector attributes weight without exposing unauthorized content; efficiency changes pass the same quality/safety suite as baseline; regressions produce actionable alerts.
- **Rollout:** 6 phases — instrumentation → deterministic compilation → prefix cache management → expert isolation/lazy schemas → compaction/inspection → adaptive optimization.

## REQ-openai-api-mvp — OpenAI-compatible API router + GCS job queue MVP

- **Source:** `docs/architecture/openai-compatible-api-router-and-gcs-job-queue.md` (§26.3 Goals, §26.4 Non-Goals, §26.24 MVP Plan, §26.25 Acceptance Criteria)
- **Scope:** api.gnus.ai ingress, GCSApiRequestJob, gateway bridge, node registration/claim, worker execution, child processing jobs, metering/settlement, private/hybrid routing
- **Acceptance (19 items, key):** standard OpenAI SDK can call `https://api.gnus.ai/v1/chat/completions`; Cloudflare ingress + API key auth; request converted to signed API request job; gateway publishes to GCS network; registered node can claim; first-valid-claim wins for MVP; leases on claimed jobs; failed jobs requeue; stream chunks converted to incremental OpenAI-compatible SSE (not buffered); client disconnect cancels the GCS job; slow clients cannot cause unbounded proxy memory growth; non-streaming results in OpenAI-compatible JSON; usage recorded per tenant/project/API key/node; generated vs delivered tokens distinguished; existing processing chunk jobs keep working; API jobs can spawn child processing jobs; routing policy represented in the job envelope.
- **MVP non-goals:** Assistants/fine-tuning API compat, image gen, realtime audio, arbitrary external tool execution, full distributed token-by-token decoding, multi-node speculative decoding, global consensus per response, perfect pre-pricing, full zk verification, marketplace bidding, automatic private routing without tenant policy.
- **Open questions (9, §26.26):** lease reuse vs separate type; single claim validator vs CRDT-visible; alias scoping; `gpt-4o`-style aliases; inline payloads vs gateway references; minimum settlement record; host repo (GCS vs SuperGenius vs separate); protobuf-first vs JSON-first; post-token stream failure representation.

## REQ-capability-system — Capability system first implementation

- **Source:** `docs/architecture/capability-system.md` (§30.14 Initial Implementation Requirements)
- **Scope:** canonical capability contracts, connector registry/discovery, MCP adapter, Tool Intermediary integration, credential handling, reputation, drift detection
- **Requirements (10):** canonical schemas for Capability/Connector/CapabilityProvider/Permission/CapabilityContract/CredentialReference; connector registry + discovery service; MCP adapter translating tools into proposed contracts; deterministic validators + contract hashing; Tool Intermediary integration for all capability execution; local read-only connector (files/notes/calendar/email); remote read-only connector (MCP or OpenAPI); provider/connector health + reputation tracking; manifest drift detection + revocation; administrative review for new/changed capabilities.

## REQ-forecast-cognition — Forecast-driven cognition first production implementation

- **Source:** `docs/architecture/forecast-driven-cognition.md` (§28.21 Phases, §28.22 Implementation Requirements)
- **Scope:** Anticipatory Cognition Engine (ACE), Cognitive Execution Scheduler (CES), Forecast Graph, Personal Forecast Models, Anticipatory Distillation
- **Requirements (12):** ACE emitting ranked calibrated forecast candidates; Forecast Graph schema (confidence/horizon/resources/cost/scope/cancellation); CES applying budgets, scheduling preparation, reclaiming speculative resources; GAML forecast-candidate/prefetch/commit/release/outcome interfaces; expert/adapter/tool/node forecast targets; incremental voice+text state updates; distillation trace/label schemas; EGGROLL metrics for forecast quality; privacy enforcement before speculative retrieval/remote preparation; full observability; tests for over-prefetching, cancellation, cache pressure, privacy boundaries, turn-taking; deterministic fallback when ACE is unavailable/disabled.
- **Phases (5):** local voice/text forecasting → personal forecast models → anticipatory distillation → distributed predictive scheduling → future multimodal evidence.

## REQ-second-brain — Local Cognitive Second Brain first implementation

- **Source:** `docs/architecture/local-cognitive-second-brain.md` (§27.16 Implementation Requirements)
- **Scope:** local connectors, GAML object schemas, Second Brain Agent, supporting agents, memory mirror, privacy modes, EGGROLL signals
- **Requirements (8):** local source connectors (files/notes/email/calendar/transcripts); GAML schemas for people/orgs/projects/decisions/commitments/deadlines/tasks/facts/claims/preferences/style/contradictions/traces; Second Brain Agent with retrieval, context-packet assembly, local Expert Model invocation, permitted tool use, writeback; supporting agents (observation, ingestion, extraction, entity resolution, scoring, permission checks, verification, contradiction detection, writeback, EGGROLL signals); human-readable memory mirror; privacy modes (local-only/private enterprise/hybrid/explicit swarm); EGGROLL signal emission; validation for stale memory, conflicting commitments, source grounding, permission boundaries.

## REQ-secure-agent-targets — Secure agent operational/security targets

- **Source:** `docs/architecture/secure-agent-architecture.md` (§18.1.1.2 Operational targets)
- **Scope:** Tool Intermediary enforcement, provenance, memory writes
- **Targets:** 100% of tool executions have a valid intermediary attestation; 0 direct side-effect executions from Semantic Core or Expert Model workers; 100% of durable memory writes derived from tools/external content contain provenance metadata and policy-compatible trust classification; Tool Intermediary overhead bounded and reported separately from reasoning latency; swarm overhead bounded and separately observable.

---

## Phased rollouts captured as SPEC scope (not repeated here)

VTG (§23.21, 6 phases), speculative decoding (§24.15, 6 phases), Frozen Micro-MTP (§25.13, 5 phases), EGGROLL (§19.18, 5 phases), context lifecycle (§17.25, 6 phases) — see `constraints.md` entries per source.

## Gap notes vs existing app workstream REQUIREMENTS.md

Existing: `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/GeniusCogntiveSystem/.planning/workstreams/app/REQUIREMENTS.md` (defined 2026-08-15, 20 v1 requirements).

- **GCSB-04** ("Bot response routes through ELM specialist selection") predates the 2026-09-21 Expert Model generalization; refreshed `model-and-router.md` frames routing as capability-first Expert Model selection (ELM/EJM/EDM) with a rule-based MVP router. See INGEST-CONFLICTS.md WARNING.
- All other ingest-derived requirements above are **system-level** and have no counterpart in app workstream requirements — candidates for separate workstreams/milestones, not app v1.0 scope changes (roadmapper's call).
