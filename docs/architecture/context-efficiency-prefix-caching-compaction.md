# 30. Context Efficiency, Prefix Caching, and Compaction

## 30.1 Purpose

This document defines the normative GCS contract for assembling, caching, inspecting, compacting, and governing model-visible context.

GCS already provides the architectural ingredients for compact context through GAML, the Memory Governor, Bridge Blocks, the Context Packet Builder, specialist execution, the Budget and Constraint Manager, the Router, and the execution trace. This chapter specifies how those components cooperate so that context efficiency is deterministic, measurable, privacy-safe, and testable rather than an informal implementation preference.

The core architectural rule is:

> Every token admitted to an ELM context MUST be authorized, relevant, budgeted, attributable, and removable when it is no longer useful. Stable context SHOULD be reusable without rebuilding or repricing it as fresh input on every turn.

This chapter does not promise a fixed token reduction. Efficiency claims MUST be measured against a deployment-specific baseline at equal or better task quality, safety, privacy, and completion rate.

---

## 30.2 Document Placement and Scope

This is a separate top-level chapter because context efficiency is a cross-cutting execution contract rather than only a memory, swarm, API, or capability concern.

It extends and constrains:

* Chapter 6: Router Design
* Chapter 8.4: GNUS Agentic Memory Layer
* Chapter 9: Execution Modes and Performance
* Chapter 16: Distributed Swarm Thinking Context Architecture
* Chapter 17: Secure Agent Architecture
* Chapter 24: OpenAI-Compatible API Router and GCS Job Queue
* Chapter 25: Local Cognitive Second Brain Mode
* Chapter 26: Forecast-Driven Cognition
* Chapter 28: GCS Capability System
* Chapter 29: Agent and Module Development Inventory

The requirements apply to:

* local, private, hybrid, and public-swarm execution
* Semantic Core and ELM inference
* provider-hosted and locally hosted models
* interactive, batch, streaming, and agent workflows
* single-agent and multi-specialist execution
* prompt caches, local prefix or KV caches, and compiled context artifacts

---

## 30.3 Design Objectives

The context system MUST optimize for the following outcomes:

1. **Minimum sufficient context** rather than maximum available context.
2. **Stable-prefix reuse** where model and privacy compatibility permit it.
3. **Off-window storage** for large documents, logs, traces, and artifacts.
4. **Specialist isolation** so noisy work does not contaminate parent context.
5. **Task-boundary compaction** instead of uncontrolled mid-task compression.
6. **Reserved completion capacity** before retrieval and fan-out consume the window.
7. **Full inspectability** of what was included, excluded, compacted, cached, and spent.
8. **Quality-adjusted efficiency** rather than token reduction alone.
9. **Tenant and privacy isolation** for every context and cache artifact.
10. **Provider neutrality** so the same policy can map to hosted prompt caching or local KV-prefix reuse.

---

## 30.4 Context Asset Classes

Every model-visible input MUST be represented as a typed context asset before packet assembly.

Recommended classes are:

```text
policy
system_instruction
tenant_instruction
user_profile
project_profile
bridge_block
fact
retrieved_evidence
capability_summary
capability_contract
tool_state
execution_state
expert_digest
current_request
output_schema
volatile_metadata
```

Each context asset SHOULD carry:

```text
asset_id
asset_type
content_hash
source_refs[]
privacy_scope
owner_id
trust_class
freshness
supersedes[]
token_estimate
priority
required_or_optional
cache_eligibility
expires_at
```

Raw files, complete transcripts, crawler output, test logs, stack traces, compiler logs, and large tool responses SHOULD remain content-addressed Cognitive Assets outside the active model window. Context packets SHOULD contain only the required slice, summary, normalized claim set, or artifact reference.

---

## 30.5 Canonical Context Packet Layout

The Context Packet Builder MUST operate as a deterministic, cache-aware context compiler.

The canonical ordering is:

```text
Stable prefix
├── system and safety policy
├── tenant and workspace policy
├── stable execution contract requirements
├── stable capability summaries
├── stable user and project profile
└── stable task-family instructions

Declared cache boundary

Volatile suffix
├── current user request
├── selected Bridge Blocks and facts
├── retrieved evidence
├── selected capability contracts
├── current tool and execution state
├── contradictions and uncertainty
├── current output schema
└── volatile metadata
```

Normative requirements:

* Stable sections MUST precede volatile sections.
* Section ordering MUST be deterministic.
* Serialization MUST be canonical and versioned.
* Timestamps, request IDs, usage counters, node health, random ordering, and current queue state MUST NOT appear before the declared cache boundary.
* Completion tokens MUST be reserved before optional retrieval, expert fan-out, or tool schemas are admitted.
* Full capability schemas SHOULD be loaded only after capability selection unless a schema is both stable and required for the active task family.
* Identical stable inputs MUST produce a byte-identical stable prefix.
* Each admitted asset MUST have an inclusion reason and token estimate recorded in the packet manifest.

A packet MAY include multiple cacheable segments when the active inference backend supports segmented caching, but GCS MUST still expose one canonical logical boundary and a deterministic segment order.

---

## 30.6 Context Admission and Retrieval Governance

The Memory Governor and Retrieval Planner MUST not treat retrieval success as automatic permission to inject content.

The admission sequence is:

```text
candidate discovery
    ↓
authorization and privacy filtering
    ↓
trust, freshness, and supersession resolution
    ↓
deduplication and contradiction analysis
    ↓
token and completion-reserve check
    ↓
minimum-sufficient asset selection
    ↓
canonical packet assembly
```

Each decision SHOULD be recorded as:

```text
ContextAdmissionDecision {
  asset_id
  decision: include | exclude | summarize | reference_only
  reason
  token_estimate_before
  token_estimate_after
  privacy_scope
  trust_class
  freshness
  conflicts[]
}
```

Admission policy MUST:

* reject unauthorized assets before semantic ranking
* suppress exact and semantic duplicates
* prefer a current fact over a superseded fact while preserving lineage
* avoid injecting both a summary and its full source unless the task requires both
* prefer a narrow source slice over an entire document
* prefer an `ExpertDigest` over an expert transcript
* stop retrieval when the expected marginal value is lower than its token, latency, or privacy cost
* preserve explicit user-pinned context unless doing so would violate a hard safety or window constraint

---

## 30.7 Deterministic Serialization and Prefix Identity

GCS MUST distinguish between a general context packet hash and an executable prefix-cache key.

A context packet hash may identify semantic content for tracing, lineage, and VTG integration. A prefix-cache key additionally proves runtime compatibility.

Recommended prefix identity:

```text
prefix_cache_key = H(
    canonicalization_version,
    tenant_scope,
    privacy_scope,
    model_hash,
    tokenizer_hash,
    quantization_hash,
    adapter_hash,
    runtime_compatibility_class,
    system_policy_hash,
    tenant_policy_hash,
    capability_summary_set_hash,
    stable_context_hash
)
```

The canonicalizer MUST define:

* UTF-8 normalization rules
* whitespace rules
* line-ending rules
* map and list ordering
* numeric formatting
* null and optional-field handling
* tool and capability ordering
* schema versioning
* separator and boundary encoding

Any change to canonicalization behavior MUST change `canonicalization_version`.

---

## 30.8 Cache Scope, Compatibility, and Invalidation

GCS may use three related cache forms:

1. **Compiled context cache**: reusable canonical stable-prefix artifacts.
2. **Hosted prompt cache**: provider-managed reuse of matching input prefixes.
3. **Local prefix or KV cache**: node-local model state reuse.

A cache entry MUST be scoped by tenant and privacy boundary even when visible text is identical.

A cache entry MUST be invalidated or bypassed when any compatibility input changes, including:

* model or model revision
* tokenizer
* quantization format or runtime compatibility class
* adapter or composed adapter set
* system, safety, tenant, or workspace policy
* capability summary or contract set included in the stable prefix
* stable user or project state included in the prefix
* privacy scope or authorization context
* canonicalization version
* backend cache format

Cache entries MUST NOT contain plaintext credentials, secret values, private tool responses, or unauthorized cross-tenant material.

A cache miss MUST record a machine-readable reason such as:

```text
not_found
expired
model_changed
tokenizer_changed
adapter_changed
policy_changed
capability_set_changed
stable_context_changed
privacy_scope_changed
canonicalization_changed
backend_incompatible
bypassed_by_policy
```

Cache lookup failure MUST degrade to normal uncached execution rather than task failure unless a hard latency or cost policy explicitly says otherwise.

---

## 30.9 Context Lifecycle and Compaction

Context is a governed lifecycle, not an ever-growing transcript.

GCS defines three compaction modes.

### 30.9.1 Task-Boundary Compaction

Task-boundary compaction is the preferred mode. It runs after a task, workflow phase, or major decision boundary and before unrelated work enters the same active context.

### 30.9.2 Budget-Triggered Compaction

Budget-triggered compaction runs when the next planned stage would violate the context or completion reserve. It SHOULD occur before the stage begins.

### 30.9.3 Emergency Compaction

Emergency compaction is a last-resort action used to prevent request failure when the active window is nearly exhausted. It MUST be marked in the execution trace and SHOULD trigger a quality-risk flag.

Every compaction MUST preserve, when present:

```text
active objective
hard constraints and policies
confirmed decisions
unresolved questions
current plan and progress
tool side effects
pending approvals
critical facts and source references
contradictions and uncertainty
artifact references
privacy and authorization scope
output obligations
```

Compaction SHOULD remove or externalize:

```text
superseded plans
resolved questions
repeated instructions
raw tool noise
full specialist transcripts
large source bodies already represented by references
stale execution state
redundant summaries
```

The compaction record MUST include:

```text
CompactionRecord {
  compaction_id
  request_id
  mode: task_boundary | budget_triggered | emergency
  source_span
  source_asset_ids[]
  retained_asset_ids[]
  externalized_asset_ids[]
  summary_asset_id
  tokens_before
  tokens_after
  preservation_checks[]
  privacy_scope
  created_at
}
```

A compacted summary MUST NOT replace its source assets until preservation checks pass. Source artifacts MAY be moved to colder storage according to retention policy, but lineage MUST remain available.

Bridge Blocks are the preferred compact continuity artifact. They SHOULD be generated incrementally as deltas and MAY be merged hierarchically from turn-span to task, workflow, and project levels.

---

## 30.10 Specialist Isolation and Expert Digest Contract

Specialists MUST be treated as bounded lossy filters for parent context.

A specialist may consume large internal context, logs, tests, retrieved documents, or traces. The parent agent SHOULD receive only a compact, typed result.

Recommended contract:

```text
ExpertDigest {
  expert_id
  role
  answer_or_patch
  normalized_claims[]
  decisions[]
  evidence_refs[]
  artifact_refs[]
  unresolved_questions[]
  confidence
  uncertainty_reasons[]
  verification_requests[]
  internal_tokens
  return_tokens
  latency_ms
  privacy_scope
  execution_contract_hash
}
```

Requirements:

* Raw test output, crawler output, stack traces, compiler logs, and long retrieved documents MUST remain external artifacts by default.
* Parent agents MUST receive the digest rather than the complete specialist transcript unless an explicit policy permits expansion.
* Each expert role MUST have a parent-return token limit.
* The digest MUST preserve evidence and artifact references needed for verification.
* Reopening raw material MUST be an explicit retrieval event recorded in the trace.
* The digest MUST inherit the most restrictive privacy scope of its contributing assets.
* Digest generation MUST NOT expose unrestricted hidden chain-of-thought.

The existing Expert Output Packager SHOULD implement this contract rather than introducing an unrelated packaging path.

---

## 30.11 Hierarchical Budget Governance

The Budget and Constraint Manager MUST allocate budgets by execution stage rather than enforce only a request-wide ceiling.

Recommended hierarchy:

```text
Request budget
├── classification and planning
├── memory retrieval
├── context assembly
├── primary generation
├── specialist execution
├── capability and tool execution
├── verification
├── synthesis
├── response delivery
└── memory writeback
```

Each stage SHOULD define:

```text
hard_token_limit
soft_token_limit
hard_time_limit_ms
soft_time_limit_ms
cost_limit
completion_reserve
borrowing_policy
fallback_policy
cancellation_policy
```

Normative rules:

* Required completion capacity MUST be reserved before optional work is scheduled.
* A stage MUST NOT silently consume another stage's hard reserve.
* Budget borrowing MUST be explicit, bounded, and traceable.
* Optional expert fan-out MUST stop before it threatens required synthesis or response delivery.
* Tool output MUST be summarized, sliced, or externalized when it exceeds its admission budget.
* Cancelled work SHOULD stop promptly and report generated versus delivered tokens separately.

---

## 30.12 Adaptive Expert and Verification Scheduling

GCS MUST not equate maximum fan-out with maximum quality.

The Router, Planner, and Scheduler SHOULD estimate the marginal value of additional work using:

* task risk
* uncertainty
* disagreement
* prior expert performance
* expected verification yield
* remaining token, time, and cost budget
* cache warmth and locality
* privacy restrictions

Recommended verification ladder:

```text
Tier 0: parser, schema, and deterministic rule checks
Tier 1: tests, static analysis, or tool dry-run
Tier 2: one specialist verifier
Tier 3: independent verifier plus grounding
Tier 4: arbitration or reputation-weighted consensus
Tier 5: EIS sampling or execution-integrity escalation
```

Additional experts or verifiers SHOULD stop when:

* the configured confidence threshold is reached
* independent outputs agree on the normalized claims
* remaining disagreement is isolated and explicitly represented
* expected quality gain is lower than additional cost or latency
* the next invocation would violate a hard reserve

High-risk side effects MAY still require a fixed approval or verification path regardless of marginal-gain estimates.

---

## 30.13 Capability Schema Loading

The Capability System SHOULD use two-stage schema disclosure.

Stage one exposes compact, stable capability summaries to general planning:

```text
calendar.event.search — read-only event retrieval
email.draft.create — creates a draft; no external delivery
payment.submit — external financial side effect; approval required
```

Stage two loads the full signed capability contract only after selection and only into the component that requires it.

This prevents large connector ecosystems from turning full schemas into permanent prompt overhead and reduces stable-prefix churn when connectors are installed, removed, or updated.

The full contract remains authoritative for validation, authorization, sandboxing, and execution.

---

## 30.14 Routing and Cache Affinity

Routing SHOULD include cache affinity as one factor, never as the sole authority.

A node with a compatible warm model, adapter, artifact set, and stable prefix may be preferable to a nominally faster cold node when policy, reputation, and privacy permit it.

Recommended ranking factors include:

```text
eligibility
privacy compatibility
execution contract compatibility
reputation
current load
expected latency
expected cost
model and adapter locality
artifact locality
prefix-cache affinity
failure probability
```

Cache affinity MUST NOT override:

* authorization
* safety policy
* execution-integrity requirements
* minimum reputation
* data residency
* tenant isolation

The scheduler SHOULD use shortlist-and-claim behavior rather than unrestricted broadcast when the eligible node set is large. Hedged execution MAY start a secondary worker after a delay and MUST cancel losing work once sufficient verified output exists.

---

## 30.15 Context Inspector and Control Surface

GCS MUST provide a first-class Context Inspector distinct from the Memory Inspector.

The Memory Inspector explains stored memory. The Context Inspector explains what is active for a specific request or session and why.

For every request, the Context Inspector SHOULD expose:

```text
total input tokens
reserved output tokens
tokens by packet section
retrieved tokens versus injected tokens
inclusion and exclusion reasons
duplicate and superseded assets
stable-prefix hash
stable-prefix change since prior request
cache hit, miss, write, bypass, and invalidation reason
compaction generation and history
specialist internal and return tokens
estimated cached and uncached cost
privacy scope by section
```

Recommended control operations are:

```text
context.inspect
context.explain
context.compact
context.pin
context.unpin
context.evict
```

These operations may be presented through an API, CLI, desktop UI, or agent interface. Manual controls MUST still respect safety, privacy, retention, and required-context policies.

---

## 30.16 Telemetry, SLOs, and Quality Gates

Context optimization MUST be evaluated using quality-adjusted metrics.

Required metrics include:

| Metric | Definition |
|---|---|
| Stable-prefix reuse rate | Requests reusing a compatible stable prefix divided by eligible requests |
| Cached input ratio | Cached input tokens divided by total cache-eligible input tokens |
| Prefix churn by cause | Prefix invalidations grouped by reason |
| Context compression ratio | Candidate raw tokens divided by injected tokens |
| Retrieval utilization | Injected retrieved assets that materially support the result |
| Duplicate admission ratio | Duplicate or superseded tokens admitted into active context |
| Expert compression ratio | Specialist internal tokens divided by parent-return tokens |
| Verification yield | Material errors caught per verification cost |
| Cancelled-compute ratio | Generated work discarded after cancellation |
| Warm-routing rate | Eligible requests routed to a compatible warm node |
| Cost per accepted task | Total execution cost divided by tasks meeting acceptance criteria |
| Quality at fixed budget | Evaluation quality under a fixed token, latency, and cost envelope |

A deployment SHOULD define SLOs by task family rather than adopt one global cache-hit or compression target.

A context optimization MUST NOT be promoted when it materially degrades:

* factual accuracy
* task completion
* safety compliance
* privacy compliance
* output-schema validity
* side-effect correctness
* calibrated uncertainty

Token savings without preserved quality are not considered an optimization.

---

## 30.17 Privacy, Security, and Multi-Tenant Isolation

Context and cache artifacts inherit the privacy and authorization requirements of their source assets.

Requirements:

* Prefix-cache entries MUST be tenant-scoped and privacy-scoped.
* Two tenants MUST NOT share an inference cache entry solely because their visible prefixes hash to the same value.
* Private memory authorization MUST run before cache lookup when the cache identity depends on private assets.
* Secret material MUST be removed before context serialization unless a capability contract explicitly requires a secure secret reference.
* Secret values MUST NOT appear in traces, inspector output, or cache keys.
* Compacted assets MUST preserve privacy scope and revocation lineage.
* Deleting or revoking a source asset MUST invalidate derived cache and compaction artifacts where policy requires it.
* Cross-node cache transfer MUST use authenticated, authorized, integrity-checked transport and MAY be disabled entirely for private scopes.

---

## 30.18 Component Ownership and Inventory Delta

Chapter 29 SHOULD be interpreted with the following required extensions.

### 30.18.1 Context Packet Builder

The Context Packet Builder becomes the deterministic context compiler and owns:

* typed section assembly
* stable and volatile partitioning
* canonical serialization
* token estimation
* completion reservation enforcement
* packet manifest generation
* stable-prefix hash generation

### 30.18.2 Prefix Cache Manager

A new Prefix Cache Manager owns:

* compatible cache lookup and write
* cache scope enforcement
* invalidation
* expiration and eviction
* backend adapters for hosted and local caches
* cache telemetry

### 30.18.3 Context Compaction Coordinator

A new Context Compaction Coordinator owns:

* compaction trigger evaluation
* preservation checks
* Bridge Block generation requests
* compaction records
* source lineage
* rollback when compaction validation fails

### 30.18.4 Context Inspector

A new Context Inspector owns per-request context explainability, token attribution, cache diagnostics, compaction history, and privacy-scope display.

### 30.18.5 Expert Output Packager

The Expert Output Packager MUST implement the `ExpertDigest` boundary and externalize noisy working artifacts by default.

### 30.18.6 Budget and Constraint Manager

The Budget and Constraint Manager MUST support hierarchical stage budgets, output reservation, bounded borrowing, and cancellation rules.

### 30.18.7 Router, Planner, and Scheduler

These components SHOULD incorporate cache affinity, expected marginal value, and early-stop rules while preserving policy, reputation, and privacy precedence.

### 30.18.8 Metrics and Evaluation Services

Metrics and evaluation services MUST capture context-efficiency metrics and gate optimization changes against task-quality regressions.

---

## 30.19 Reference Context Packet Manifest

A context packet SHOULD emit a manifest similar to:

```text
ContextPacketManifest {
  packet_id
  request_id
  canonicalization_version
  model_hash
  tokenizer_hash
  adapter_hash
  tenant_scope
  privacy_scope
  stable_prefix_hash
  prefix_cache_key
  cache_eligible
  cache_result
  cache_reason
  output_tokens_reserved
  sections[] {
    name
    stable_or_volatile
    token_count
    asset_ids[]
  }
  admission_decisions[]
  compaction_generation
  total_input_tokens
}
```

The manifest is an execution and observability artifact. It SHOULD NOT duplicate full private context content.

---

## 30.20 Failure Modes and Fallbacks

### 30.20.1 Prefix Churn

Cause: volatile data, unstable ordering, changing tool schemas, or policy drift before the boundary.

Fallback: execute uncached, record the cause, and alert when churn exceeds the task-family threshold.

### 30.20.2 Over-Compaction

Cause: a summary removes a constraint, unresolved question, source reference, or pending side effect.

Fallback: reject the compacted asset, restore the prior generation, and retry with a stricter preservation contract.

### 30.20.3 Under-Compaction

Cause: redundant or noisy context remains active.

Fallback: reduce optional retrieval, externalize raw artifacts, and schedule task-boundary compaction.

### 30.20.4 Retrieval Flood

Cause: too many semantically similar assets pass ranking.

Fallback: enforce deduplication, diversity, source caps, and marginal-value stopping.

### 30.20.5 Specialist Transcript Leakage

Cause: raw specialist work is returned to the parent.

Fallback: replace it with an `ExpertDigest`, preserve the raw output as a restricted artifact, and record the packaging violation.

### 30.20.6 Cache Isolation Failure

Cause: incompatible privacy or tenant scopes share a cache entry.

Fallback: block the result, invalidate the entry, emit a security event, and require authorization review.

### 30.20.7 Completion Starvation

Cause: retrieval, schemas, or expert outputs consume reserved response capacity.

Fallback: evict optional assets, stop fan-out, compact at the safest available boundary, or return an explicit bounded response.

---

## 30.21 Initial Implementation Phases

### 30.21.1 Phase One: Measurement

* record tokens by context section
* record output reservation
* record retrieval admission and utilization
* record specialist internal and return tokens
* establish task-quality baselines

### 30.21.2 Phase Two: Deterministic Context Compilation

* implement canonical serialization
* declare the stable-prefix boundary
* generate stable-prefix hashes
* add byte-identity tests
* move volatile metadata below the boundary

### 30.21.3 Phase Three: Cache Management

* implement the Prefix Cache Manager
* add hosted prompt-cache adapters
* add local prefix or KV-cache adapters
* implement invalidation and tenant isolation
* add cache-affinity signals to routing

### 30.21.4 Phase Four: Compaction and Specialist Isolation

* implement task-boundary and budget-triggered compaction
* add preservation checks and rollback
* implement `ExpertDigest`
* externalize raw logs and large tool output

### 30.21.5 Phase Five: Inspection and Adaptive Scheduling

* ship the Context Inspector
* add hierarchical budgets
* add marginal-value early stopping
* add quality-adjusted dashboards and regression gates

---

## 30.22 Acceptance Criteria

The initial implementation is acceptable when all of the following are true:

1. Identical stable inputs produce a byte-identical stable prefix and identical prefix key.
2. Changing only the current request does not change the stable-prefix hash.
3. Changing model, tokenizer, adapter, policy, capability set, tenant, privacy scope, or canonicalization version invalidates incompatible reuse.
4. No volatile timestamp, request ID, usage counter, or node-health value appears before the declared boundary.
5. Every admitted context asset has a recorded inclusion reason and token count.
6. Completion tokens are reserved before optional retrieval and specialist fan-out.
7. Duplicate and superseded assets are suppressed or explicitly justified.
8. Every specialist returns a bounded `ExpertDigest`; raw logs remain external unless explicitly reopened.
9. Compaction preserves objectives, hard constraints, decisions, unresolved questions, evidence references, tool state, approvals, and privacy scope.
10. Failed compaction validation leaves the prior context generation recoverable.
11. Cache entries never cross unauthorized tenant or privacy boundaries.
12. The Context Inspector explains section-level tokens, cache behavior, admission decisions, and compaction history.
13. Cancelled work reports generated and delivered usage separately and stops within the configured grace period.
14. Efficiency changes pass the same safety, privacy, and task-quality evaluations as the baseline path.
15. Cache, context, compaction, and quality regressions produce actionable alerts.

---

## 30.23 Non-Goals

This chapter does not require:

* one universal provider-specific cache implementation
* exposing raw hidden chain-of-thought
* retaining every historical turn in active context
* eliminating large-context models
* sharing caches across privacy boundaries
* caching secret values
* compacting in the middle of every long task
* using the public swarm for private context
* guaranteeing a fixed percentage reduction in tokens or cost

---

## 30.24 Cross-References

This chapter should be read with:

* Router Design
* GNUS Agentic Memory Layer
* Distributed Swarm Thinking Context Architecture
* Secure Agent Architecture
* Objective Memory and Verified Transition Graph
* OpenAI-Compatible API Router and GCS Job Queue Architecture
* Local Cognitive Second Brain Mode
* Forecast-Driven Cognition and Predictive Prefetching
* Execution Integrity System
* GCS Capability System
* Agent and Module Development Inventory

---

## 30.25 Summary

GCS context efficiency is a system contract, not a prompt-writing trick.

The architecture combines:

```text
minimum-sufficient retrieval
+ deterministic context compilation
+ stable-prefix caching
+ off-window Cognitive Assets
+ bounded specialist digests
+ task-boundary compaction
+ hierarchical budgets
+ cache-aware routing
+ context inspection
+ quality-adjusted telemetry
```

Together, these mechanisms keep active context small, reusable, inspectable, privacy-safe, and aligned with task quality. The governing principle is simple: GCS should know exactly what each model is holding, why it is holding it, what it costs, and when it can be removed.
