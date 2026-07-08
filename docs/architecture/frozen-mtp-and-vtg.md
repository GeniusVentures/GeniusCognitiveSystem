# **23 Frozen Micro-MTP and VTG Edge Inference**

## **23.1 Purpose**

This document defines the GNUS edge-inference form of Frozen Multi-Token Prediction: **Frozen Micro-MTP**.

Frozen Micro-MTP attaches a small multi-token prediction head to an already deployed frozen Semantic Core or ELM backbone.

The backbone remains unchanged.

The MTP head proposes a short continuation, usually one to four tokens or one small structured state block. The local verifier then accepts the usable prefix before commitment.

This document should be read as a companion to:

- [Objective Memory and Verified Transition Graph](./objective-memory-vtg.md)
- [Speculative Decoding and VTG Candidate Scheduling](./speculative-decoding-and-vtg.md)

Reference example:

- Google Research, **Accelerating Gemini Nano models on Pixel with frozen Multi-Token Prediction**
  - https://research.google/blog/accelerating-gemini-nano-models-on-pixel-with-frozen-multi-token-prediction/

---

## **23.2 Operating Envelope**

Frozen Micro-MTP is designed for ubiquitous GNUS nodes with constrained local memory, power, and GPU throughput.

The head is treated as a compact efficiency artifact attached to a frozen model artifact.

Target execution shape:

```text
small frozen backbone / ELM
    ↓
shared hidden state or KV cache
    ↓
tiny role-specific MTP head
    ↓
1 to 4 token or small-state proposal
    ↓
local verification
    ↓
commit accepted prefix
    ↓
compact VTG / EGGROLL outcome event
```

The head improves latency by reusing state that the local model already computed.

---

## **23.3 Why this matters**

A separate speculative drafter can increase local memory pressure because it carries its own weights, prefill work, KV cache, runtime buffers, and model-switching overhead.

Frozen Micro-MTP keeps the drafter close to the active model.

```text
already-running frozen model
        ↓
small attached MTP head
        ↓
short draft
        ↓
same model or local verifier validates
```

This fits GNUS nodes because the drafter does not need to reconstruct context in a second model.

---

## **23.4 Core Design Principle**

> If the local model already computed the context, the drafter should reuse that state.

A constrained node should use Frozen Micro-MTP when the memory overhead of the head is lower than the cost of repeated autoregressive steps or a separate drafter.

---

## **23.5 Micro-MTP Budget**

Recommended starting budget:

```text
MTP head size: 5MB to 50MB
speculative depth: 1 to 4 tokens
branch factor: 1
verification: mandatory
rollback tolerance: near zero
```

A larger head or deeper speculative path is promoted only when measurement shows accepted-prefix latency savings that justify the local memory cost.

The key metric is:

> verified accepted prefix length per megabyte and millisecond.

---

## **23.6 Relationship to VTG**

Frozen Micro-MTP and VTG provide complementary signals.

| Layer | Role |
|------|------|
| Frozen Micro-MTP | Fast local neural guess from current hidden state |
| VTG | Historical memory of verified transitions across prior executions |
| Confidence Scheduler | Selects the prefix to verify |
| Local Verifier | Confirms before commitment |

Combined path:

```text
GAML Context Packet
    ↓
Frozen Semantic Core / ELM Forward Pass
    ↓
VTG Hot Shard Candidate Lookup
    ↓
Frozen Micro-MTP Proposal
    ↓
Confidence Scheduler
    ↓
Local Verification
    ↓
Commit Accepted Prefix
    ↓
Update VTG + EGGROLL Signals
```

VTG can bias or validate the Micro-MTP proposal.

Micro-MTP can propose when no strong VTG edge exists.

Together:

```text
Micro-MTP = immediate local neural guess
VTG = distributed historical verified guess
```

---

## **23.7 Candidate Record**

A Frozen Micro-MTP proposal should carry enough metadata to update VTG and EGGROLL without storing raw private prompt text.

```json
{
  "candidate_source": "frozen_micro_mtp",
  "backbone_model": "formatter_elm_vx",
  "mtp_head": "formatter_elm_vx_micro_mtp_v1",
  "backbone_frozen": true,
  "context_packet_hash": "hash",
  "previous_state": "state_id",
  "current_state": "state_id",
  "proposed_length": 4,
  "position_confidence": [0.96, 0.91, 0.74, 0.52],
  "scheduled_prefix_length": 2,
  "verified_prefix_length": 2,
  "regenerated_suffix_length": 2,
  "verification_mode": "schema",
  "latency_saved_ms": 18,
  "verification_required": true
}
```

The candidate remains a proposal until the local verifier accepts it.

---

## **23.8 Best Initial Targets**

Frozen Micro-MTP should start with narrow roles where validation is cheap.

| Target | Why |
|-------|-----|
| Formatter / Schema ELM | high structural predictability and schema validation |
| Tool-Support ELM | repeated call shapes with dry-run validation |
| Code Specialist | common syntax and patch continuations with compiler/test validation |
| JSON / DSL Specialist | deterministic structure and low ambiguity |
| Primary Draft ELM | short low-risk repeated prose patterns |

---

## **23.9 Local Verification Requirements**

Frozen Micro-MTP is useful because verification keeps commitment bounded.

Verification may be performed by:

- the same frozen backbone
- a role-specific verifier ELM
- schema validation
- parser validation
- compiler or tests
- static analysis
- tool dry-run
- grounding agreement
- epistemic arbitration

The commitment rule is:

> A speculative token or state is committed after acceptance by the configured local verifier path.

---

## **23.10 Node Capability Advertisement**

A GNUS node should advertise Micro-MTP support in its capability profile.

```json
{
  "supports_frozen_micro_mtp": true,
  "max_mtp_depth": 4,
  "mtp_head_budget_mb": 24,
  "shared_state_mode": "hidden_state_or_kv_reuse",
  "mtp_heads": [
    "formatter_micro_mtp_v1",
    "schema_micro_mtp_v1"
  ],
  "verification_modes": [
    "target_model",
    "schema",
    "tool_dry_run",
    "compiler",
    "verifier_elm"
  ]
}
```

The Router uses this profile to decide whether a node is eligible for latency-sensitive micro-speculation.

---

## **23.11 Relationship to Micro-Diffusion and Tiny Tree Drafting**

Frozen Micro-MTP is the first neural target for ubiquitous nodes.

Micro-diffusion and tiny JetSpec-style tree heads are complementary role-specific backends.

| Backend | GNUS Role |
|--------|-----------|
| Frozen Micro-MTP | first neural backend; short local prefixes |
| Tiny Causal Tree Head | branch factor 2, depth 2 to 4, verified locally |
| Micro-Diffusion Block Drafter | masked/block fill for structured low-entropy regions |
| Larger Block-Diffusion Models | research and benchmark environments |

---

## **23.12 Relationship to EGGROLL**

EGGROLL can optimize Frozen Micro-MTP deployment without changing the frozen backbone.

Targets include:

- MTP head selection
- MTP depth by role
- confidence threshold by verifier type
- prefix survival policy
- MTP vs VTG vs rule vs micro-diffusion selection
- tenant-private MTP scheduling
- promotion criteria for new MTP heads

Example outcome event:

```json
{
  "event_type": "frozen_micro_mtp_outcome",
  "backbone_model": "formatter_elm_vx",
  "mtp_head": "formatter_micro_mtp_v1",
  "role": "formatter",
  "proposed_length": 4,
  "accepted_length": 3,
  "verification_cost_ms": 9,
  "latency_saved_ms": 21,
  "memory_overhead_mb": 18,
  "standalone_drafter_avoided": true,
  "hardware_profile": "ubiquitous_low_power_gpu",
  "policy_hash": "policy_hash",
  "signature": "ed25519"
}
```

---

## **23.13 Initial Implementation Path**

### **23.13.1 Phase 1 — Measurement**

Instrument local inference to measure repeated patterns, accepted depth, cache pressure, and verifier cost.

### **23.13.2 Phase 2 — Formatter / Schema Micro-MTP**

Attach a small head to the most deterministic local specialist.

### **23.13.3 Phase 3 — Code Specialist Micro-MTP**

Extend to code paths where compiler, tests, static analysis, or verifier ELMs can validate.

### **23.13.4 Phase 4 — Router Policy**

Allow the Router to choose among:

- standard autoregressive generation
- VTG lookup
- rule / schema drafter
- Frozen Micro-MTP
- tiny causal tree head
- micro-diffusion block drafter

### **23.13.5 Phase 5 — Swarm Learning**

Use compact VTG and EGGROLL events to tune depth, thresholds, and backend choice by role and device class.

---

## **23.14 Summary**

Frozen Micro-MTP gives GeniusCognitiveSystem a practical first neural speculative backend for ubiquitous GNUS nodes.

It reuses local model state, avoids a separate drafter, keeps the backbone frozen, proposes short prefixes, verifies locally, and publishes compact outcome signals so the swarm can learn where the head is useful.

The value is that many small nodes become incrementally faster and more reliable together.
