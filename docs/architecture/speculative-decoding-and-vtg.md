# **Speculative Decoding and VTG Candidate Scheduling**

## **Purpose**

This document defines the speculative decoding architecture for GeniusCognitiveSystem under the correct GNUS deployment assumption:

> GNUS nodes are ubiquitous constrained peers, not server-class accelerators.

The design center is a heterogeneous network of low-power nodes with small local model/runtime budgets, local GPU or integrated GPU execution, compact VTG shards, and swarm-level learning.

Speculative decoding in GNUS must therefore be implemented as **micro-speculation**:

- small local drafts
- short accepted prefixes
- narrow role-specific heads
- deterministic schema or grammar helpers where possible
- local verification before commitment
- compact VTG outcome publication
- EGGROLL optimization across the swarm

This document should be read as a companion to:

- [Objective Memory and Verified Transition Graph](./objective-memory-vtg.md)
- [Frozen Multi-Token Prediction and VTG Edge Inference](./frozen-mtp-and-vtg.md)

---

## **Non-Server Assumption**

The speculative decoding layer must not assume:

- a high-end verifier node
- large standalone drafter models
- server-class tree attention kernels
- full DiffusionGemma-class model execution on a normal node
- long speculative horizons
- large branch factors
- large global VTG shards resident on every device

A normal GNUS node should be able to operate with an approximate local budget such as:

```text
active model / ELM budget: 100MB to 350MB
micro drafter or MTP head: 5MB to 50MB
hot VTG shard: 10MB to 100MB
runtime buffers, KV cache, and shader state: remaining local budget
```

The exact budget may vary by device, but the architectural rule is stable:

> The swarm provides scale. The individual node provides a small verified contribution.

---

## **Why this extension exists**

Most autoregressive inference generates one token at a time.

Speculative decoding accelerates generation by drafting future tokens or states and then verifying them before commitment.

For GNUS, the point is not to make one node act like a server GPU.

The point is to let many weak nodes learn where **tiny local speculation** is safe.

The core loop is:

```text
local context
    ↓
small drafter / MTP head / rule helper / VTG lookup
    ↓
draft 1 to 4 tokens or one small state block
    ↓
local verifier, schema check, target ELM, compiler, tool dry-run, or arbiter check
    ↓
commit only if accepted
    ↓
publish compact VTG outcome
    ↓
EGGROLL tunes policy across the swarm
```

This keeps speculation useful even when each node is small.

---

## **Core Components**

| Component | GNUS-Constrained Responsibility |
|----------|----------------------------------|
| **Micro Drafter** | Proposes a tiny future prefix or one small state block. |
| **Confidence Scheduler** | Decides how much of the proposal survives before verification. |
| **Local Verifier** | Accepts or rejects using the local target model, schema validator, tool dry-run, test result, or ELM verifier. |
| **VTG Hot Shard** | Supplies locally relevant verified transition candidates. |
| **Swarm Learning Loop** | Publishes compact outcome events for VTG and EGGROLL. |

The drafter may be neural, symbolic, memory-backed, or hybrid.

The output is always provisional until verified.

---

## **Micro-Speculation Backend Classes**

GNUS should support four practical local drafter classes.

| Backend | Role | Best Initial Uses |
|--------|------|-------------------|
| **VTG Lookup Drafter** | Cheapest path; proposes transitions already verified in similar contexts. | tenant workflows, API patterns, schema fragments, code patches |
| **Frozen Micro-MTP Head** | Tiny prediction head attached to a frozen Semantic Core or ELM. | formatter, schema, code, primary draft low-risk text |
| **Rule / Grammar / Schema Drafter** | Deterministic constrained expansion without model guessing. | JSON, tool calls, structured output, DSLs |
| **Micro-Diffusion Block Drafter** | Tiny masked/block denoiser for low-entropy structured regions. | fill missing JSON fields, code patch skeletons, template repair |

The Router should prefer cheaper and more deterministic drafters first.

```text
if deterministic schema or grammar exists:
    use rule / schema drafter
elif strong VTG hit exists:
    use VTG lookup drafter
elif local model exposes a micro-MTP head:
    use Frozen MTP
elif constrained block-fill task has a tiny denoiser:
    use micro-diffusion drafter
else:
    use normal autoregressive generation
```

---

## **Confidence-Scheduled Prefix Retention**

Speculation should not be all-or-nothing.

The scheduler keeps the longest safe prefix and drops the uncertain suffix.

```text
Draft:       A B C D
Confidence:  .96 .91 .72 .51
Threshold:   .90
Keep:        A B
Drop:            C D
```

For GNUS nodes, the scheduler should be conservative by default.

Typical initial settings:

```text
branch factor: 1 to 2
prefix depth: 1 to 4 tokens or one small state block
verification: mandatory
rollback budget: near zero
```

The goal is not maximum draft length.

The goal is:

> verified accepted length per unit of latency, memory, and risk.

---

## **VTG as the Primary Swarm Advantage**

A single node may have little compute, but the swarm has history.

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

The hot shard on a node should only contain the graph fragments relevant to:

- local model or ELM role
- tenant boundary
- current policy hash
- recent workload family
- device class
- available verifier path

The node does not need the full graph.

---

## **Diffusion as Micro-Diffusion, Not Full Diffusion LM**

DiffusionGemma-style block-diffusion language modeling is architecturally interesting, but it is not a direct target for normal GNUS nodes.

GNUS should borrow the idea, not the scale.

The usable pattern is:

```text
small masked block
    ↓
tiny role-specific denoiser
    ↓
few refinement steps
    ↓
schema / code / verifier check
    ↓
commit if accepted
```

Examples:

```text
partially filled JSON block -> fill missing fields -> schema verifies
code patch skeleton -> fill likely syntax -> compiler/verifier checks
tool-call template -> fill arguments -> dry-run validates
```

Micro-diffusion should be limited to low-entropy, structured regions where verification is cheap.

It should not be used as the general language generator on a normal node.

---

## **JetSpec-Inspired Causal Tree Drafting as a Tiny Head**

JetSpec-style causal parallel drafting is useful as an architectural pattern, but the GNUS implementation must be tiny.

The server-style version may use larger candidate trees and specialized high-end attention kernels.

The GNUS version should be:

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

This preserves the useful idea of causal branch faithfulness without assuming a high-end verifier node.

---

## **Frozen Micro-MTP as the First Neural Target**

Frozen Multi-Token Prediction is the most practical neural speculative backend for GNUS edge nodes.

It keeps the backbone frozen and attaches a small prediction head that reuses the main model state.

This avoids:

- standalone drafter weights
- duplicate prefill
- duplicate KV cache
- extra model switching
- broad retraining of the deployed backbone

In GNUS, Frozen MTP should be deployed as **micro-MTP**:

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

The first targets should be formatter/schema and code-specialist paths, not high-stakes factual generation.

---

## **Role-Specific Speculation Policy**

| Role | Policy |
|------|--------|
| Formatter / Schema ELM | Aggressive relative to other roles; deterministic validation is cheap. |
| Tool-Support ELM | Conservative; validate before side effects. |
| Code Specialist | Moderate; rely on compiler, tests, static analysis, or code verifier. |
| Primary Draft ELM | Conservative for general prose; stronger only for low-risk text. |
| Verifier ELM | Very conservative; consistency beats speed. |
| Grounding ELM | Very conservative; evidence alignment is required. |
| Synthesizer / Arbiter | Conservative; arbitration remains authoritative. |

Speculation depth is a policy decision, not a fixed model property.

---

## **Node Capability Advertisement**

Nodes should advertise micro-speculation capabilities in a compact profile.

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

The Router should use this profile when choosing local or swarm execution paths.

---

## **Swarm Outcome Events**

Each node should emit compact outcome events rather than large traces.

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
this tiny drafter works for JSON on weak GPUs
this VTG edge works for tenant API calls
this micro-diffusion block fails after two steps
this formatter head is safe up to four tokens
this code patch pattern needs compiler verification
```

This is the GNUS advantage.

---

## **Integration with EGGROLL**

EGGROLL should optimize micro-speculation policy rather than assuming large model retraining.

Targets include:

- drafter backend selection
- maximum prefix depth by role
- confidence threshold by verifier type
- branch factor by model budget
- VTG hot-shard promotion policy
- micro-MTP head selection
- micro-diffusion use/no-use policy
- rollback penalty weighting
- device-class scheduling policy

The backbone should remain stable unless a separate retraining flow explicitly promotes a new artifact.

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

Prototype a tiny masked denoiser only for structured low-entropy blocks.

### **Phase 6 — Swarm Optimization**

Use VTG and EGGROLL events to tune which backend works for each role, device class, and tenant workflow.

---

## **Non-Goals**

This architecture does not require:

- full DiffusionGemma-class inference on normal GNUS nodes
- server-scale JetSpec kernels on normal GNUS nodes
- large standalone drafter models
- long speculative horizons
- global graph residency on each device
- unverified speculative commitment

---

## **Design Principle**

GNUS should not treat diffusion or tree speculation as server-scale accelerators.

It should treat them as micro-speculative primitives that run locally, verify cheaply, and improve collectively through VTG and EGGROLL.

The system should not try to make one weak node brilliant.

It should let many weak nodes become reliable together.

---

[Companion: Objective Memory and Verified Transition Graph](./objective-memory-vtg.md) | [Companion: Frozen Multi-Token Prediction and VTG Edge Inference](./frozen-mtp-and-vtg.md) | [Architecture Index](./INDEX.md)
