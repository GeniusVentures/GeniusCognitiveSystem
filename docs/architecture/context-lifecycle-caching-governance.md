# **17. Context Lifecycle, Caching, and Governance**

## **17.1 Purpose**

This document defines the normative context-efficiency contract for GeniusCognitiveSystem (GCS).

GCS already provides selective memory retrieval, Bridge Blocks, specialist execution, content-addressed artifacts, token and privacy budgets, canonical context packets, and distributed scheduling. This chapter specifies how those mechanisms must work together to minimize repeated prompt work without reducing correctness, safety, privacy, provenance, or inspectability.

The core architectural rule is:

> Every token entering an ELM must be authorized, relevant, budgeted, explainable, cacheable where possible, and removable when it stops being useful.

The objective is not simply to reduce prompt size. The objective is to reduce **fresh input work per accepted task** while preserving or improving task quality and completion.

---

## **17.2 Normative Language**

The terms **MUST**, **MUST NOT**, **SHOULD**, **SHOULD NOT**, and **MAY** are normative requirements in this chapter.

A deployment may use different model providers, runtimes, cache mechanisms, storage engines, and user interfaces, but it must preserve the behavioral guarantees defined here.

---

## **17.3 Scope**

This specification applies to:

* Semantic Core inference
* role-based and domain-specific ELM execution
* local, private, public, and hybrid execution modes
* single-node, expert-assisted, and distributed-swarm workflows
* OpenAI-compatible API requests
* Local Cognitive Second Brain Mode
* GAML retrieval and Bridge Block use
* capability and connector metadata exposed to models
* verification, arbitration, and synthesis stages
* hosted prompt caches and local prefix or KV caches
* context compaction and task-boundary summarization

This specification does not require one universal cache implementation. A hosted provider may expose prompt-cache accounting, while a local runtime may retain a compatible prefix or KV state. Both are governed by the same identity, privacy, compatibility, invalidation, and observability rules.

---

## **17.4 Non-Goals**

This chapter does not:

* guarantee a fixed percentage of token reduction
* permit quality, safety, or privacy regression in exchange for lower cost
* treat cache hits as evidence that cached content is correct
* make cached state an authorization mechanism
* require raw conversation transcripts to become durable memory
* require every component to run as a separate service
* expose unrestricted hidden chain-of-thought
* permit cross-tenant cache reuse solely because visible text matches
* permit compaction to discard active goals, constraints, approvals, or unresolved risks

External examples of very large token reductions are useful motivation, but they are workload-specific. GCS must establish deployment-specific baselines and measure savings at equal or better quality.

---

## **17.5 Architectural Position**

```text
User, Application, API, or Scheduled Workflow
                    ↓
Session, Identity, Privacy, and Policy Context
                    ↓
Executive Controller + Budget and Constraint Manager
                    ↓
Memory Governor + Retrieval Planner
                    ↓
Context Compiler
    ├── authorization and privacy filtering
    ├── relevance and freshness selection
    ├── duplicate and supersession filtering
    ├── hierarchical token allocation
    ├── stable-prefix construction
    ├── volatile-suffix construction
    └── canonical serialization and hashing
                    ↓
Prefix Cache Manager
    ├── hosted prompt-cache adapter
    ├── local prefix / KV cache
    ├── compatibility and isolation checks
    └── cache-affinity routing signals
                    ↓
Semantic Core or Specialist ELM
    ├── bounded internal context
    ├── external artifact references
    └── bounded ExpertDigest return
                    ↓
Verification, Arbitration, and Synthesis
                    ↓
Context Compaction Manager + Bridge Block Generator
                    ↓
GAML Write Evaluation, Trace, Metrics, and Learning Signals
```

The Context Compiler is the deterministic boundary between authorized source material and model-visible context. The Prefix Cache Manager may accelerate compatible inference, but it does not decide what a model is allowed to see. The Context Compaction Manager reduces active context only after preservation requirements are satisfied.

This chapter extends, rather than replaces:

* [Distributed Swarm Thinking Context Architecture](distributed-swarm-thinking-context.md)
* [GNUS Agentic Memory Layer](agentic-memory-layer.md)
* [OpenAI-Compatible API Router and GCS Job Queue Architecture](openai-compatible-api-router-and-gcs-job-queue.md)
* [Local Cognitive Second Brain Mode](local-cognitive-second-brain.md)
* [Forecast-Driven Cognition and Predictive Prefetching](forecast-driven-cognition.md)
* [GCS Capability System](capability-system.md)
* [Agent and Module Development Inventory](agent-module-development-inventory.md)

---

## **17.6 Core Design Principles**

### **17.6.1 Smallest Sufficient Context**

GCS MUST assemble the smallest context that is expected to satisfy the current execution stage.

A larger context is not automatically safer or more capable. Irrelevant material increases prefill cost, latency, distraction, privacy exposure, and the probability that stale or conflicting information influences the result.

### **17.6.2 Off-Window by Default for Large Material**

Large documents, codebases, logs, test output, retrieved pages, compiler output, and intermediate artifacts SHOULD remain outside the active model window.

Models SHOULD receive bounded excerpts, normalized facts, Bridge Blocks, or content-addressed references selected for the current task.

### **17.6.3 Stable Before Volatile**

Stable, reusable prompt material MUST appear before volatile request-specific material when the runtime supports prefix reuse.

Volatile values such as timestamps, request identifiers, health counters, random ordering, current user turns, and transient node state MUST NOT appear before the declared stable-prefix boundary.

### **17.6.4 Deterministic Construction**

Equivalent authorized inputs MUST produce byte-identical stable-prefix serialization under the same canonicalization version.

Ordering, whitespace, field omission, schema formatting, and capability-summary formatting MUST be deterministic.

### **17.6.5 Specialists as Lossy Filters**

A specialist may consume a large private working context, but the parent workflow SHOULD receive only the smallest verified digest required for synthesis.

Raw specialist logs and intermediate material SHOULD remain external artifacts unless explicitly reopened.

### **17.6.6 Compaction at Semantic Boundaries**

Compaction SHOULD occur at clean task, phase, or workflow boundaries.

Emergency compaction MAY occur to prevent failure, but it MUST preserve the active objective, constraints, decisions, approvals, unresolved questions, evidence references, tool state, and privacy scope.

### **17.6.7 Hierarchical Budgets**

Token, time, cost, storage, network, expert fan-out, and verification budgets MUST be allocated by stage rather than enforced only as one request-wide ceiling.

Required output capacity MUST be reserved before retrieval and expert expansion consume the remaining context budget.

### **17.6.8 Explainability Without Raw Hidden Reasoning**

GCS MUST explain why a context item was included, which source it came from, its privacy and trust scope, and how many tokens it consumed.

This does not require exposing unrestricted private chain-of-thought.

### **17.6.9 Privacy Never Widens Through Optimization**

Caching, summarization, embedding, compaction, expert digestion, and routing MUST preserve or strengthen the source privacy scope.

Optimization is never a reason to broaden access.

---

## **17.7 Canonical Context Packet Model**

A model-visible context packet SHOULD use the following logical order:

```text
ContextPacket
├── Stable Prefix
│   ├── system and safety policy
│   ├── tenant, workspace, and deployment policy
│   ├── stable task-family instructions
│   ├── stable user or project profile, when authorized
│   ├── compact capability summaries
│   └── stable output and protocol contracts
│
├── Cache Boundary
│
└── Volatile Suffix
    ├── current request
    ├── current execution-stage objective
    ├── selected Bridge Blocks and facts
    ├── active plan, constraints, and unresolved questions
    ├── current contradictions and uncertainty
    ├── current tool, node, and workflow state
    ├── volatile timestamps and identifiers
    └── output reservation and execution constraints
```

The packet MAY be physically represented as several provider messages or runtime buffers. The logical ordering and cache boundary must remain explicit and inspectable.

Each section MUST include or be traceable to:

```text
section_id
section_class
source_asset_ids
source_hashes
privacy_scope
trust_class
freshness
inclusion_reason
estimated_tokens
actual_tokens, when available
stable_or_volatile
canonicalization_version
```

---

## **17.8 Stable-Prefix Contract**

### **17.8.1 Permitted Stable Material**

Stable-prefix material MAY include:

* system behavior and safety policy
* deterministic protocol instructions
* tenant and workspace policy
* stable role definitions
* stable output schemas
* stable task-family instructions
* authorized profile facts that rarely change
* compact capability summaries in deterministic order
* stable glossary and domain conventions
* stable model-specific formatting instructions

### **17.8.2 Prohibited Volatile Material**

The stable prefix MUST NOT contain:

* current timestamps or dates unless the task family explicitly requires a stable date snapshot
* request, trace, task, or correlation identifiers
* current usage counters
* current queue depth or node-health values
* randomly ordered tools or assets
* current user input
* current retrieved evidence
* current authorization grants that are not represented through a stable policy version
* ephemeral secrets or credential material
* unbounded conversation history

### **17.8.3 Canonicalization**

Stable-prefix canonicalization MUST define:

* Unicode normalization
* newline convention
* whitespace policy
* deterministic field ordering
* deterministic list ordering
* omission rules for null or default fields
* schema rendering rules
* capability-summary ordering
* policy precedence ordering
* canonicalization version

A change to canonicalization rules MUST change the compatibility identity.

---

## **17.9 Prefix Cache Identity and Compatibility**

A representative cache identity is:

```text
prefix_cache_key = H(
    tenant_scope,
    privacy_scope,
    authorization_scope_hash,
    model_hash,
    tokenizer_hash,
    quantization_hash,
    runtime_hash,
    adapter_hash,
    system_policy_hash,
    tenant_policy_hash,
    capability_contract_set_hash,
    stable_context_hash,
    canonicalization_version
)
```

The exact encoding MAY vary, but all compatibility dimensions that could change model behavior or authorization MUST participate in the identity or in an equivalent eligibility check.

A cache entry is eligible for reuse only when:

* tenant, user, workspace, and privacy boundaries permit reuse
* authorization scope is compatible
* model, tokenizer, quantization, runtime, and adapter are compatible
* system and tenant policy versions are compatible
* capability contracts exposed in the prefix are compatible
* the stable prefix is byte-identical under the same canonicalization version
* the entry has not expired, been revoked, or failed integrity validation

Cache entries MUST NOT be shared across tenants, users, or privacy scopes merely because their visible text is identical.

Private cache keys SHOULD use tenant-scoped keyed hashes to reduce cross-scope correlation.

---

## **17.10 Cache Lifecycle and Invalidation**

A Prefix Cache Manager MUST support:

```text
create
validate
reuse
refresh
expire
revoke
purge
observe
```

Material changes that MUST invalidate or bypass reuse include:

* model or tokenizer change
* quantization, kernel, runtime, or adapter incompatibility
* system, tenant, workspace, or user-policy change
* authorization or privacy-scope change
* capability-contract change
* stable profile or project-state change
* canonicalization-version change
* security incident or suspected cache poisoning
* key rotation or tenant revocation requiring purge

Non-material changes after the cache boundary MUST NOT invalidate the stable-prefix entry.

Eviction MAY be driven by memory pressure, recency, value, or cost, but eviction policy MUST NOT bypass privacy or revocation requirements.

A security or privacy revocation MUST be able to purge affected entries without waiting for normal expiration.

---

## **17.11 Context Selection and Hierarchical Token Allocation**

### **17.11.1 Budget Order**

The Budget and Constraint Manager MUST reserve required output capacity before allocating optional input context.

A representative allocation order is:

```text
model context limit
    - protocol and safety reserve
    - required output reserve
    - tool and verification reserve
    = allocatable input budget
```

The allocatable input budget is then distributed across:

```text
current request
active objective and hard constraints
stable policy and protocol
required memory and evidence
active plan and unresolved questions
capability metadata
specialist results
optional supporting context
```

### **17.11.2 Stage Budgets**

A request SHOULD allocate sub-budgets for:

```text
classification and planning
memory retrieval
context assembly
primary generation
specialist execution
verification
arbitration
synthesis
memory writeback
```

Each stage MUST define:

* hard ceiling
* soft warning threshold
* required output reservation
* borrowing rule
* fallback behavior
* cancellation condition
* observability fields

### **17.11.3 Selection Priority**

When context must be reduced, GCS SHOULD preserve in this order:

1. system, safety, privacy, and authorization policy
2. active objective and explicit user request
3. hard constraints and approved tool state
4. confirmed decisions and unresolved blockers
5. high-trust evidence required for the current claim
6. current plan and progress
7. recent, relevant Bridge Blocks
8. supporting examples and optional background

Low-value, duplicated, stale, superseded, or weakly relevant material SHOULD be removed before higher-priority material.

### **17.11.4 Duplicate and Supersession Filtering**

The Context Compiler MUST detect and suppress avoidable duplication across:

* system policy
* tenant policy
* user profile
* project profile
* Bridge Blocks
* fact store
* retrieved excerpts
* expert outputs
* capability descriptions

When two assets conflict, the compiler MUST NOT silently select one merely to save tokens. It must preserve the contradiction or request arbitration when the conflict is material.

---

## **17.12 Off-Window Content and Artifact References**

Large or noisy material SHOULD be stored in the Content-Addressed Artifact Store or another approved scoped store.

Examples include:

* source documents
* codebases
* build and test logs
* compiler traces
* retrieved web pages
* full tool responses
* screenshots and media
* specialist scratch artifacts
* benchmark output
* detailed verification traces

A model receives an artifact only through an authorized bounded read such as:

```text
artifact_id
content_hash
source_identity
privacy_scope
trust_class
selected_span
selection_reason
retrieval_timestamp
sanitization_status
```

A task SHOULD retrieve a slice, normalized record, or summary rather than the whole artifact.

Raw material MAY be reopened when a verifier, arbiter, or user explicitly needs it. Reopening is a new context-allocation decision and must be recorded.

---

## **17.13 Expert Isolation and the ExpertDigest Contract**

Specialist execution MUST separate internal working context from parent-facing output.

A representative parent-facing contract is:

```text
ExpertDigest {
    expert_id
    expert_version
    task_id
    answer_or_patch
    normalized_claims[]
    decisions[]
    evidence_refs[]
    artifact_refs[]
    unresolved_questions[]
    confidence
    uncertainty_reasons[]
    verification_requests[]
    internal_input_tokens
    internal_output_tokens
    return_tokens
    latency_ms
    privacy_scope
    signature_or_attestation
}
```

Requirements:

* each expert role MUST declare a parent-return token budget
* raw logs, traces, retrieved documents, and intermediate drafts SHOULD remain artifacts
* the parent SHOULD receive the digest, not the entire expert transcript
* evidence and artifact references MUST remain reopenable under the original authorization scope
* expert summaries MUST inherit or strengthen source privacy
* malformed, unsigned, over-budget, or privacy-incompatible digests MUST be rejected or repaired through an explicit path
* a digest MUST distinguish conclusions from unresolved uncertainty

A specialist may use many tokens internally and still return a small, high-value digest. This is the intended behavior.

---

## **17.14 Capability Schema Loading**

The general reasoning path SHOULD receive compact capability summaries rather than every full connector schema.

A compact summary may contain:

```text
capability_id
human_description
read_or_write_class
side_effect_class
privacy_class
approval_requirement
provider_class
```

The full signed capability contract and provider-specific schema SHOULD be loaded only after the Router or Planner selects that capability for a bounded stage.

The selected Tool-Support ELM or deterministic adapter MAY receive the complete schema needed to construct and validate the invocation.

Capability discovery, authorization, and execution remain governed by the [GCS Capability System](capability-system.md) and Tool Intermediary. Lazy loading does not weaken validation.

---

## **17.15 Context Compaction Lifecycle**

### **17.15.1 Compaction Modes**

GCS defines three compaction modes.

**Task-boundary compaction** is preferred after a completed task or workflow phase.

**Budget-triggered compaction** occurs when the next approved stage would violate the active context budget.

**Emergency compaction** occurs only when required to avoid request failure or context overflow.

### **17.15.2 Preservation Requirements**

Every compaction MUST preserve, when present:

```text
active objective
current user request
hard constraints and policies
privacy and authorization scope
confirmed decisions
unresolved questions
current plan and progress
pending tool actions and approvals
tool side effects already performed
critical evidence and provenance references
material contradictions and uncertainty
artifact references
required output contract
```

### **17.15.3 Compaction Record**

Each compaction MUST create a record containing:

```text
compaction_id
request_id
task_or_phase_boundary
mode
reason
source_span_ids
assets_retained
assets_removed_or_externalized
summary_asset_id
privacy_scope
tokens_before
tokens_after
quality_check
model_or_service_version
created_at
```

### **17.15.4 Hierarchical Bridge Blocks**

Bridge Blocks SHOULD support hierarchical continuity:

```text
turn-span blocks
        ↓
task block
        ↓
workflow block
        ↓
project block
```

Retrieval SHOULD begin at the smallest sufficient level and drill down only when required.

Compaction SHOULD create delta summaries rather than repeatedly summarizing the entire history. Older raw transcripts may remain as cold artifacts subject to retention and privacy policy.

### **17.15.5 Compaction Quality Check**

Before a compacted state replaces active context, GCS MUST verify that required preservation fields remain represented.

High-risk workflows SHOULD use deterministic checks plus an independent verifier when summarization loss could create material harm.

---

## **17.16 Context Inspector**

GCS SHOULD expose a first-class Context Inspector alongside the Memory Inspector.

For each request and stage, it should display:

```text
total input tokens
reserved output tokens
tokens by context section
retrieved tokens versus injected tokens
stable-prefix hash and version
cache hit, miss, write, bypass, or invalidation
cache invalidation reason
prefix change from prior compatible request
why each asset was included
assets retrieved but not injected
duplicate or superseded material removed
compaction generation and history
expert internal tokens versus return tokens
capability summaries versus full schemas loaded
estimated cached and uncached cost
privacy and trust scope by section
```

The Context Inspector MUST NOT expose secrets, credential material, unauthorized private content, or unrestricted hidden reasoning.

A simulation mode SHOULD show what would be removed at lower context budgets without changing the live request.

---

## **17.17 Routing and Scheduling Integration**

The Router and Scheduler SHOULD consider cache compatibility and warmth as ranking inputs after authorization and correctness constraints are satisfied.

A compatible warm node may be preferable to a nominally faster cold node when it reduces prefill latency and cost.

Representative ranking inputs include:

```text
policy eligibility
privacy and placement eligibility
model and adapter compatibility
stable-prefix cache affinity
artifact locality
current health and load
expected latency
expected cost
reputation and quality
failure probability
```

Cache affinity MUST NOT override privacy, policy, required model identity, or quality constraints.

Switching models, tokenizers, adapters, or incompatible runtimes invalidates prefix reuse unless the cache implementation explicitly proves compatibility.

Distributed fan-out SHOULD use early cancellation. When sufficient verified output has arrived, unnecessary claims, specialists, or hedged requests SHOULD be cancelled, and discarded compute SHOULD be metered.

---

## **17.18 Verification and Early Exit**

Verification SHOULD be adaptive rather than maximally broad for every request.

A representative ladder is:

```text
Tier 0 — parser and schema checks
Tier 1 — deterministic rules, tests, or calculators
Tier 2 — one specialist verifier
Tier 3 — independent verifier plus grounding
Tier 4 — arbitration or reputation-weighted consensus
Tier 5 — EIS spot checks for execution-integrity risk
```

Additional verification MAY stop when:

* required deterministic checks pass
* calibrated independent verifiers agree on the material claims
* remaining disagreement is isolated and explicitly represented
* the expected quality gain of another verifier is lower than its cost and latency under policy

High-stakes, security-sensitive, or policy-mandated workflows may require fixed minimum verification regardless of expected marginal gain.

---

## **17.19 Security, Privacy, and Trust Requirements**

### **17.19.1 Cache Isolation**

Cache storage MUST be isolated according to tenant, user, workspace, privacy, authorization, model, and policy compatibility.

Private cached state SHOULD be encrypted at rest and protected in memory according to deployment policy.

### **17.19.2 No Secret Material in Model Prefixes**

Raw credentials, API keys, refresh tokens, private keys, and unscoped secrets MUST NOT be included in stable or volatile model context.

Credential references may be included only when authorized and necessary.

### **17.19.3 Revocation and Deletion**

Privacy revocation, user deletion, tenant deletion, or key revocation MUST invalidate affected derived context, summaries, embeddings, cache entries, and indexes according to retention policy.

### **17.19.4 Cache Poisoning and Context Injection**

Cached material and compacted summaries MUST retain provenance and integrity metadata.

External content MUST be sanitized before inclusion. A cache hit does not waive prompt-injection, memory-poisoning, or policy checks.

### **17.19.5 Trust Separation**

Cache validity means the bytes are compatible and reusable. It does not mean the content is true, current, high-trust, or suitable for memory.

Truth, freshness, provenance, authorization, and cache compatibility remain separate dimensions.

---

## **17.20 Observability and Evaluation**

GCS SHOULD report at least the following context-efficiency metrics:

| Metric | Purpose |
| --- | --- |
| stable-prefix reuse rate | Measures compatible prompt or prefix reuse |
| cached versus fresh input tokens | Separates reused work from new prefill work |
| prefix churn by cause | Identifies timestamps, ordering changes, policy changes, and schema drift |
| context compression ratio | Compares available source material with injected context |
| unused retrieval ratio | Identifies over-retrieval and poor selection |
| memory duplication ratio | Identifies repeated facts, summaries, and policy text |
| expert internal-to-return token ratio | Measures specialist isolation efficiency |
| capability schema loading ratio | Measures compact summaries versus full schemas |
| compaction savings | Measures tokens before and after approved compaction |
| compaction preservation failure rate | Detects lost objectives, constraints, evidence, or tool state |
| cancelled-compute ratio | Measures work started but no longer needed |
| warm-node routing rate | Measures cache and artifact locality use |
| cost per accepted task | Connects efficiency to useful completion |
| quality at fixed budget | Detects regressions hidden by lower cost |
| tokens at fixed quality | Measures genuine efficiency improvement |

Every optimization experiment MUST compare against a fixed quality, safety, privacy, and completion baseline.

Reducing tokens while increasing material errors, failed tasks, privacy incidents, or user corrections is not an optimization.

---

## **17.21 Failure Modes and Required Fallbacks**

### **17.21.1 Over-Compaction**

**Risk:** Active constraints, decisions, or evidence are lost.

**Fallback:** Restore the prior context generation or reopen source artifacts; mark the compacted summary as failed.

### **17.21.2 Prefix Churn**

**Risk:** Timestamps, random ordering, changing tool lists, or unstable formatting prevent cache reuse.

**Fallback:** Identify the first changed stable-prefix section, move volatile material below the boundary, and restore deterministic ordering.

### **17.21.3 Stale Compatible Cache**

**Risk:** Bytes are compatible but the represented policy or profile is no longer valid.

**Fallback:** Require versioned policy, authorization, freshness, and revocation checks before reuse.

### **17.21.4 Retrieval Omission**

**Risk:** Aggressive token reduction omits necessary context.

**Fallback:** Escalate the retrieval budget, drill down from Bridge Blocks to source artifacts, or request clarification.

### **17.21.5 Expert Digest Loss**

**Risk:** A digest omits a critical caveat or evidence item.

**Fallback:** Reopen referenced artifacts or request a verifier-specific digest with a larger bounded return budget.

### **17.21.6 Cross-Scope Leakage**

**Risk:** A cache entry, summary, or artifact reference crosses a tenant or privacy boundary.

**Fallback:** Fail closed, purge affected derived state, emit a security event, and require incident review.

### **17.21.7 Budget Starvation**

**Risk:** Retrieval or expert work consumes required output capacity.

**Fallback:** Cancel optional stages, compact at a safe boundary, use a smaller approved context set, or return an explicit partial result.

### **17.21.8 Cache-Centric Routing Regression**

**Risk:** The scheduler overvalues cache warmth and selects a lower-quality or policy-incompatible node.

**Fallback:** Authorization, model compatibility, reputation, and quality constraints remain hard gates; cache affinity is only a ranking feature.

---

## **17.22 Canonical Interfaces and Data Contracts**

The initial schema package SHOULD include:

```text
ContextSection
ContextPacket
ContextBudget
StageBudget
ContextInclusionDecision
StablePrefixManifest
PrefixCacheKey
PrefixCacheEntryMetadata
PrefixCacheEvent
ExpertDigest
ArtifactReference
CompactionRequest
CompactionRecord
ContextInspectionReport
ContextEfficiencyMetric
```

### **17.22.1 ContextInclusionDecision**

```yaml
asset_id: gaml:...
decision: include
reason: required_for_active_constraint
section: volatile.memory
rank: 2
estimated_tokens: 184
privacy_scope: workspace_private
trust_class: verified
freshness: current
source_hash: sha256:...
```

### **17.22.2 PrefixCacheEvent**

```yaml
request_id: req:...
event: hit
cache_key_hash: hmac-sha256:...
model_hash: sha256:...
stable_prefix_tokens: 3120
fresh_suffix_tokens: 742
privacy_scope: tenant_private
compatibility_version: 1
```

### **17.22.3 CompactionRecord**

```yaml
compaction_id: cmp:...
mode: task_boundary
source_generation: 4
result_generation: 5
tokens_before: 18420
tokens_after: 3650
preserved_fields:
  - active_objective
  - hard_constraints
  - decisions
  - unresolved_questions
  - tool_state
  - evidence_refs
summary_asset_id: gaml:bridge:...
quality_check: passed
privacy_scope: user_private
```

---

## **17.23 Initial Implementation Requirements**

A first implementation MUST include:

1. A deterministic Context Compiler with versioned canonicalization.
2. An explicit stable-prefix and volatile-suffix boundary.
3. A tenant- and privacy-scoped Prefix Cache Manager or provider adapter.
4. Completion-token reservation before optional context allocation.
5. Explainable inclusion decisions and token attribution by section.
6. Duplicate and supersession filtering.
7. Off-window artifact references for large documents and logs.
8. A bounded ExpertDigest contract.
9. Task-boundary, budget-triggered, and emergency compaction modes.
10. Compaction preservation checks and rollback support.
11. A Context Inspector or equivalent trace view.
12. Cache hit, miss, bypass, invalidation, and churn metrics.
13. Context-efficiency regression tests tied to quality and safety baselines.
14. Purge and revocation handling for private derived context and cache entries.
15. Router and Scheduler support for cache affinity as a non-authoritative ranking signal.

---

## **17.24 Acceptance Criteria**

The implementation is acceptable when all of the following hold:

1. Identical stable inputs produce a byte-identical stable prefix and identical compatibility hash.
2. Changing only the current request does not change the stable-prefix hash.
3. Changing model, tokenizer, adapter, runtime, policy, capability contract, tenant, authorization, privacy scope, or canonicalization version prevents incompatible reuse.
4. No volatile timestamp, request identifier, usage counter, or randomly ordered list appears before the declared cache boundary.
5. Every injected context item has an inclusion reason, source, privacy scope, trust class, and token count or estimate.
6. Required output tokens cannot be consumed by retrieval or expert expansion without an explicit fallback decision.
7. Large logs and documents remain external artifacts by default.
8. Every specialist returns a bounded ExpertDigest; raw working material is not automatically copied into the parent context.
9. Compaction preserves active objectives, hard constraints, decisions, unresolved questions, evidence references, tool state, and privacy scope.
10. Failed compaction can restore or reopen the prior generation.
11. Cache entries cannot cross tenant or privacy boundaries through content-hash equality alone.
12. Revocation can invalidate and purge affected cache and derived context.
13. The Context Inspector can attribute context weight and cache behavior without exposing unauthorized content or hidden reasoning.
14. Efficiency changes pass the same quality, safety, privacy, and task-completion evaluation suite as the baseline.
15. Cache and context regressions produce actionable alerts with a cause classification.

---

## **17.25 Rollout Plan**

### **17.25.1 Phase One — Instrumentation**

Deliver:

* token attribution by context section
* retrieved-versus-injected accounting
* completion reservation metrics
* prefix hashes and change reasons
* expert internal-versus-return token accounting
* compaction before-and-after metrics

### **17.25.2 Phase Two — Deterministic Context Compilation**

Deliver:

* versioned canonicalization
* stable and volatile sectioning
* duplicate and supersession filtering
* explicit inclusion decisions
* hierarchical stage budgets

### **17.25.3 Phase Three — Prefix Cache Management**

Deliver:

* hosted prompt-cache adapters
* local prefix or KV-cache adapter
* compatibility identity and invalidation
* tenant and privacy isolation
* cache-affinity scheduling signals

### **17.25.4 Phase Four — Expert Isolation and Lazy Schemas**

Deliver:

* ExpertDigest contracts
* raw-artifact boundaries
* parent-return token budgets
* compact capability summaries
* full schema loading only after selection

### **17.25.5 Phase Five — Compaction and Inspection**

Deliver:

* task-boundary and budget-triggered compaction
* emergency compaction fallback
* preservation validation and rollback
* hierarchical Bridge Blocks
* Context Inspector

### **17.25.6 Phase Six — Adaptive Optimization**

Deliver:

* marginal-value stopping for expert and verifier fan-out
* cache-affinity and artifact-locality scheduling
* cancellation and discarded-compute accounting
* quality-adjusted efficiency dashboards
* EGGROLL signals for context selection and compaction quality

---

## **17.26 Design Principle**

```text
Keep durable knowledge outside the model window.
Compile only the smallest authorized slice required for the current stage.
Keep the stable prefix deterministic.
Keep volatile state below the cache boundary.
Let specialists return digests, not noise.
Compact at semantic boundaries.
Measure savings only at equal or better quality.
```

GCS context efficiency is a maintained runtime property, not a one-time prompt optimization.

---

## **17.27 Non-Normative Motivation**

An external practitioner article summarized five useful operating patterns: move large material out of the active window, isolate noisy work in subagents, maintain selective persistent memory, preserve exact stable prompt prefixes, and compact between tasks. Its headline token-reduction example is illustrative rather than a GCS performance guarantee.

Reference: Codila, “Build self-managed agent system that uses 90% fewer tokens in 5 steps: subagents, memory, context,” July 2026, https://x.com/0xcodila/article/2077780537016488197.
