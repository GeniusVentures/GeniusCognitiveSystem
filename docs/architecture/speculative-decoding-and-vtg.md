# **Speculative Decoding and VTG Candidate Scheduling**

## **Purpose**

This document defines the speculative decoding architecture for GeniusCognitiveSystem using the GNUS network design center: ubiquitous constrained peers, compact local models, local verification, and swarm-level learning.

Speculative decoding in GNUS is implemented as **micro-speculation**.

A node proposes a short local continuation, verifies it through the configured local path, commits only the accepted prefix, and publishes compact outcome metadata back into VTG and EGGROLL.

This keeps acceleration compatible with GNUS nodes that operate under small model/runtime budgets while allowing the network as a whole to improve over time.

This document should be read as a companion to:

- [Objective Memory and Verified Transition Graph](./objective-memory-vtg.md)
- [Frozen Micro-MTP and VTG Edge Inference](./frozen-mtp-and-vtg.md)

---

## **Operating Envelope**

The baseline GNUS speculative decoding profile is optimized for nodes with limited memory, power, and GPU throughput.

Representative local budget:

```text
active model / ELM budget: 100MB to 350MB
micro drafter or MTP head: 5MB to 50MB
hot VTG shard: 10MB to 100MB
runtime buffers, KV cache, and shader state: remaining local budget
```

The architectural rule is:

> The swarm provides scale. The individual node provides a small verified contribution.

This drives the implementation toward:

- short accepted prefixes
- small role-specific prediction heads
- deterministic schema and grammar helpers
- compact VTG hot shards
- mandatory local verification
- compact signed outcome events
- device-class-aware scheduling

---

## **Why this layer exists**

Autoregressive inference generates one token at a time.

Speculative decoding accelerates generation by proposing a short continuation and validating it before commitment.

For GNUS, speculative decoding is most useful when applied to repeated low-entropy regions such as:

- JSON and schema-constrained generation
- formatter output
- tool-call templates
- common code continuations
- known API usage patterns
- verifier corrections
- tenant workflow transitions

The local loop is:

```text
local context
    ↓
small drafter / MTP head / rule helper / VTG lookup
    ↓
draft 1 to 4 tokens or one small state block
    ↓
local verifier, schema check, target ELM, compiler, tool dry-run, or arbiter check
    ↓
commit accepted prefix
    ↓
publish compact VTG outcome
    ↓
EGGROLL tunes policy across the swarm
```

---

## **Core Components**

| Component | Responsibility |
|----------|----------------|
| **Micro Drafter** | Proposes a tiny future prefix or one small state block. |
| **Confidence Scheduler** | Selects the portion of the proposal that should be verified. |
| **Local Verifier** | Accepts the prefix using the local target model, schema validator, tool dry-run, test result, or ELM verifier. |
| **VTG Hot Shard** | Supplies locally relevant verified transition candidates. |
| **Swarm Learning Loop** | Publishes compact outcome events for VTG and EGGROLL. |

The drafter may be neural, symbolic, memory-backed, or hybrid.

The output remains provisional until the configured verifier path accepts it.

---

## **Micro-Speculation Backend Classes**

GNUS supports four practical local drafter classes.

| Backend | Role | Best Initial Uses |
|--------|------|-------------------|
| **VTG Lookup Drafter** | Cheapest path; proposes transitions already verified in similar contexts. | tenant workflows, API patterns, schema fragments, code patches |
| **Frozen Micro-MTP Head** | Tiny prediction head attached to a frozen Semantic Core or ELM. | formatter, schema, code, primary draft low-risk text |
| **Rule / Grammar / Schema Drafter** | Deterministic constrained expansion without model guessing. | JSON, tool calls, structured output, DSLs |
| **Micro-Diffusion Block Drafter** | Tiny masked/block denoiser for low-entropy structured regions. | fill missing JSON fields, code patch skeletons, template repair |

Recommended backend ordering:

```text
if deterministic schema or grammar exists:
    use rule / schema drafter
elif strong VTG hit exists:
    use VTG lookup drafter
elif local model exposes a micro-MTP head:
    use Frozen Micro-MTP
elif constrained block-fill task has a tiny denoiser:
    use micro-diffusion drafter
else:
    use standard autoregressive generation
```

---

## **Confidence-Scheduled Prefix Retention**

The scheduler keeps the longest useful prefix and routes that prefix through verification.

```text
Draft:       A B C D
Confidence:  .96 .91 .72 .51
Threshold:   .90
Verify:      A B
Regenerate:      C D
```

Initial operating profile:

```text
branch factor: 1 to 2
prefix depth: 1 to 4 tokens or one small state block
verification: mandatory
rollback budget: near zero
```

The primary metric is:

> verified accepted length per unit of latency, memory, and risk.

---

## **VTG as the Primary Swarm Advantage**

A single node may have limited compute, but the swarm has history.

VTG allows each node to benefit from repeated verified transitions without needing a large local model.

A VTG candidate may represent:

- token-block continuation
- schema segment
- tool-call fragment
- code patch block
- verifier correction
- workflow transition
- synthesis move
- micro-diffusion fill pattern
- MTP prefix survival profile

The hot shard on a node contains graph fragments relevant to:

- local model or ELM role
- tenant boundary
- current policy hash
- recent workload family
- device class
- available verifier path

---

## **Micro-Diffusion Block Drafting**

Diffusion-style generation contributes a useful block-refinement pattern for GNUS when scaled down to constrained local execution.

The GNUS form is:

```text
small masked block
    ↓
tiny role-specific denoiser
    ↓
few refinement steps
    ↓
schema / code / verifier check
    ↓
commit accepted block
```

Examples:

```text
partially filled JSON block -> fill missing fields -> schema verifies
code patch skeleton -> fill likely syntax -> compiler/verifier checks
tool-call template -> fill arguments -> dry-run validates
```

Micro-diffusion is best suited to low-entropy structured regions where verification is cheap and deterministic.

---

## **Tiny Causal Tree Drafting**

JetSpec-style causal parallel drafting contributes a useful pattern for maintaining causal consistency across a small speculative tree.

The GNUS implementation profile is:

```text
tiny causal draft head
small branch factor
short depth
local target verification
VTG outcome feedback
```

Recommended initial limits:

```text
branch factor: 2
depth: 2 to 4 tokens
roles: formatter, schema, code, tool-support
verification: mandatory
```

This preserves causal branch faithfulness while staying within the ubiquitous-node budget.

---

## **Frozen Micro-MTP as the First Neural Target**

Frozen Multi-Token Prediction is the most practical neural speculative backend for GNUS edge nodes.

It keeps the backbone frozen and attaches a small prediction head that reuses the main model state.

This reduces duplicated context processing and avoids a separate standalone drafter.

GNUS deployment profile:

```text
frozen ELM or Semantic Core
    ↓
small role-specific MTP head
    ↓
1 to 4 token proposal
    ↓
confidence scheduler
    ↓
local verification
    ↓
VTG outcome update
```

The first targets are formatter/schema and code-specialist paths.

---

## **Role-Specific Speculation Policy**

| Role | Policy |
|------|--------|
| Formatter / Schema ELM | Highest speculative depth; deterministic validation is cheap. |
| Tool-Support ELM | Short depth with tool dry-run validation before side effects. |
| Code Specialist | Moderate depth with compiler, tests, static analysis, or code verifier. |
| Primary Draft ELM | Short depth for low-risk repeated text patterns. |
| Verifier ELM | Minimal depth; consistency is prioritized. |
| Grounding ELM | Minimal depth when evidence alignment is required. |
| Synthesizer / Arbiter | Short depth over known synthesis patterns; arbitration remains authoritative. |

Speculation depth is a policy decision, not a fixed model property.

---

## **Node Capability Advertisement**

Nodes advertise micro-speculation capabilities in a compact profile.

```json
{
  "supports_micro_speculation": true,
  "active_model_budget_mb": 300,
  "hot_vtg_budget_mb": 64,
  "drafter_backends": [
    "vtg_lookup",
    "schema_rule",
    "frozen_micro_mtp"
  ],
  "max_spec_depth": 4,
  "max_branch_factor": 2,
  "verification_modes": [
    "target_model",
    "schema",
    "tool_dry_run",
    "compiler",
    "verifier_elm"
  ]
}
```

The Router uses this profile when choosing local or swarm execution paths.

---

## **Swarm Outcome Events**

Each node emits compact outcome events rather than large traces.

```json
{
  "event_type": "micro_speculation_outcome",
  "state_family": "json_schema_formatter",
  "drafter_backend": "frozen_micro_mtp",
  "model_budget_mb": 220,
  "hot_vtg_budget_mb": 32,
  "proposed_length": 4,
  "scheduled_prefix_length": 3,
  "accepted_length": 3,
  "latency_saved_ms": 18,
  "verification_mode": "schema",
  "hardware_profile": "ubiquitous_low_power_gpu",
  "policy_hash": "policy_hash",
  "signature": "ed25519"
}
```

These events let the swarm learn:

```text
which tiny drafters work for JSON on weak GPUs
which VTG edges work for tenant API calls
which micro-diffusion blocks converge quickly
which formatter heads are safe to depth four
which code patch patterns require compiler verification
```

---

## **Integration with EGGROLL**

EGGROLL optimizes micro-speculation policy rather than assuming large model retraining.

Targets include:

- drafter backend selection
- maximum prefix depth by role
- confidence threshold by verifier type
- branch factor by model budget
- VTG hot-shard promotion policy
- micro-MTP head selection
- micro-diffusion use policy
- rollback penalty weighting
- device-class scheduling policy

The backbone remains stable unless a separate retraining flow explicitly promotes a new artifact.

---

## **Initial Implementation Plan**

### **Phase 1 — Instrumentation**

Measure repeated low-entropy transitions, candidate acceptance, verifier cost, and latency savings without committing speculative output.

### **Phase 2 — VTG Lookup + Rule Drafter**

Implement the cheapest paths first:

- schema fragments
- tool-call templates
- JSON repair
- known workflow transitions

### **Phase 3 — Frozen Micro-MTP Head**

Attach a small MTP head to the formatter/schema ELM or another deterministic specialist.

### **Phase 4 — Tiny Causal Tree Head**

Prototype a JetSpec-inspired micro tree head with branch factor 2 and depth 2 to 4.

### **Phase 5 — Micro-Diffusion Block Drafter**

Prototype a tiny masked denoiser for structured low-entropy blocks.

### **Phase 6 — Swarm Optimization**

Use VTG and EGGROLL events to tune the backend choice for each role, device class, and tenant workflow.

---

## **Scope Boundaries**

This architecture targets normal GNUS node execution.

Larger experimental backends may exist in research or benchmark environments, but the production design baseline remains compact local speculation, local verification, and swarm-level improvement.

---

## **Design Principle**

GNUS treats diffusion, tree drafting, Frozen MTP, schema rules, and VTG lookup as micro-speculative primitives.

They run locally, verify cheaply, and improve collectively through VTG and EGGROLL.

The system does not try to make one weak node brilliant.

It lets many small nodes become reliable together.

---

[Companion: Objective Memory and Verified Transition Graph](./objective-memory-vtg.md) | [Companion: Frozen Micro-MTP and VTG Edge Inference](./frozen-mtp-and-vtg.md) | [Architecture Index](./INDEX.md)
