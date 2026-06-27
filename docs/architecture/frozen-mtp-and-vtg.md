# **Frozen Multi-Token Prediction and VTG Edge Inference**

## **Purpose**

This document extends the speculative decoding and VTG candidate scheduling architecture with a production-oriented edge inference pattern: **Frozen Multi-Token Prediction (Frozen MTP)**.

Frozen MTP attaches a lightweight multi-token prediction head to an already trained and deployed model backbone while keeping the original backbone weights frozen.

For GeniusCognitiveSystem, this is important because many GNUS nodes will operate under strict memory, energy, and latency constraints.

The architecture should therefore support speculative decoding modes that avoid shipping a separate standalone drafter model whenever an integrated drafter head can reuse the Semantic Core or ELM internal state.

This document should be read as a companion to:

- [Objective Memory and Verified Transition Graph](./objective-memory-vtg.md)
- [Speculative Decoding and VTG Candidate Scheduling](./speculative-decoding-and-vtg.md)

Reference example:

- Google Research, **Accelerating Gemini Nano models on Pixel with frozen Multi-Token Prediction**
  - https://research.google/blog/accelerating-gemini-nano-models-on-pixel-with-frozen-multi-token-prediction/

---

## **Why this matters**

Traditional speculative decoding often assumes two models:

```text
small drafter model -> large verifier model
```

That works well in server environments, but it creates edge-device costs:

- extra model weights
- extra embedding tables
- extra prefill work
- extra KV cache
- additional memory pressure
- additional model-management complexity
- task-specific drafter tuning burden

On mobile, desktop, embedded, or low-memory GNUS nodes, these costs may erase much of the benefit of speculation.

Frozen MTP changes the shape:

```text
frozen main model backbone
        ↓
shared hidden states / KV cache
        ↓
lightweight MTP head
        ↓
parallel verification by the backbone
```

The drafter is no longer a separate model that must reconstruct context from token history.

It becomes an attached prediction head that uses the representations already computed by the main model.

---

## **Core Design Principle**

The core principle is:

> Reuse the main model's computed state whenever possible; do not duplicate context processing just to draft future tokens.

For GeniusCognitiveSystem, this means a speculative execution backend may be implemented as:

1. A standalone small ELM drafter.
2. A VTG candidate frontier.
3. A discrete diffusion or block drafter.
4. A lightweight MTP head attached to a frozen Semantic Core or ELM.
5. A hybrid of the above.

Frozen MTP is the preferred first target for low-memory edge deployments because it is compatible with existing frozen deployed models and does not require retraining the full backbone.

---

## **Frozen Backbone Model**

In Frozen MTP, the Semantic Core or ELM backbone remains unchanged.

Only the auxiliary MTP head is trained or updated.

This gives several architectural benefits:

- base model behavior remains stable
- safety alignment of the backbone is not modified
- compatibility with existing deployed model CIDs is preserved
- deployment risk is reduced
- MTP can be treated as an efficiency artifact rather than a new reasoning model

This is especially useful for GNUS because the swarm may contain many model versions and hardware tiers.

A node can advertise:

```json
{
  "model_cid": "semantic_core_vx",
  "mtp_head_cid": "semantic_core_vx_mtp_head_v1",
  "backbone_frozen": true,
  "mtp_supported": true,
  "verification_required": true
}
```

---

## **Zero-Copy Drafter State**

A key implementation goal is to avoid duplicated dynamic memory.

A standalone drafter normally maintains its own state:

```text
standalone drafter weights
standalone drafter KV cache
standalone drafter prompt prefill
```

Frozen MTP should instead use:

```text
main model hidden states
main model KV cache
lightweight MTP head state only
```

This creates a zero-copy or near-zero-copy execution path.

Recommended design target:

```text
Semantic Core forward pass
    ↓
final-layer hidden states
    ↓
MTP head predicts future tokens
    ↓
Semantic Core verifies in parallel
```

For GNUS, this maps naturally to MNN/Vulkan/MoltenVK execution because the MTP head can be treated as a small attached inference module with minimal memory overhead.

---

## **Relationship to VTG**

Frozen MTP and VTG solve different problems.

| Layer | Role |
|------|------|
| Frozen MTP | Fast local neural drafting from the current model state |
| VTG | Persistent memory of verified transitions across prior executions |
| Confidence Scheduler | Chooses how much of the draft should survive |
| Target Verifier | Confirms the accepted prefix before commitment |

The combined path is:

```text
GAML Context Packet
    ↓
Semantic Core / ELM Forward Pass
    ↓
VTG Candidate Frontier
    ↓
Frozen MTP Head Draft
    ↓
Confidence Scheduler
    ↓
Parallel Verification
    ↓
Commit Accepted Prefix
    ↓
Update VTG + EGGROLL Signals
```

VTG can bias, constrain, or rank the MTP head's candidate continuations.

The MTP head can also generate candidates when no strong VTG edge exists.

Together they create a local + learned speculative system:

```text
MTP = immediate neural guess from current hidden state
VTG = learned historical guess from verified prior transitions
```

---

## **MTP Candidate Record**

A speculative candidate produced by a Frozen MTP backend should include enough metadata to update VTG and EGGROLL.

Example:

```json
{
  "candidate_source": "frozen_mtp_head",
  "backbone_model": "semantic_core_vx",
  "mtp_head": "semantic_core_vx_mtp_head_v1",
  "backbone_frozen": true,
  "context_packet_hash": "hash",
  "previous_state": "state_id",
  "current_state": "state_id",
  "proposed_tokens": ["token_a", "token_b", "token_c"],
  "position_confidence": [0.94, 0.88, 0.71],
  "scheduled_prefix_length": 2,
  "verified_prefix_length": 2,
  "rejected_suffix_length": 1,
  "latency_saved_ms": 31,
  "verification_required": true
}
```

This record should not be treated as final output.

It is a proposal that must pass the same verification and arbitration boundaries as other speculative candidates.

---

## **Role-Aware MTP Heads**

Frozen MTP heads may be attached to different execution roles.

Recommended initial targets:

| Target | Why |
|-------|-----|
| Formatter ELM | high structural predictability and easy validation |
| Tool-Support ELM | repeated tool-call shapes, but must remain conservative |
| Code Specialist | common syntax and patch sequences, validated by compiler/tests/verifier |
| Primary Draft ELM | broad latency improvement for low-risk drafts |
| Schema / JSON Specialist | strong deterministic structure |

Avoid early Frozen MTP deployment on:

- high-risk factual conclusions
- grounding-sensitive claims
- final arbitration decisions
- safety-critical tool execution
- high-entropy creative synthesis

---

## **Verification Requirements**

Frozen MTP is safe because incorrect drafts are discarded.

Therefore, every MTP path must preserve the rule:

> Drafted tokens or states are provisional until the target model or appropriate verifier accepts them.

Verification may be performed by:

- the same frozen backbone
- a role-specific verifier ELM
- schema validation
- compiler or tests
- tool dry-run validation
- grounding agreement
- epistemic arbitration

The final output should remain compatible with the target model's accepted distribution or policy-bounded verifier behavior.

---

## **Hardware and Runtime Implications**

Frozen MTP is especially attractive for GNUS edge nodes because it reduces or avoids:

- standalone drafter weight storage
- standalone drafter prompt prefill
- duplicated KV cache
- task-specific drafter deployment
- extra model switching overhead

Node capability advertisement should include:

```json
{
  "supports_frozen_mtp": true,
  "supports_cross_attention_to_backbone_kv": true,
  "max_mtp_depth": 4,
  "mtp_heads": [
    "formatter_mtp_v1",
    "code_specialist_mtp_v1"
  ],
  "memory_savings_mode": "zero_copy_kv"
}
```

The Router can use this to route latency-sensitive tasks toward nodes with integrated MTP support.

---

## **Relationship to Diffusion-Assisted Drafting**

Frozen MTP and diffusion-assisted speculative drafting are complementary.

Frozen MTP is best when:

- the target model's hidden state is available
- memory is constrained
- low-latency local drafting is required
- the draft horizon is short to medium
- exact verification is required

Diffusion-assisted drafting is more interesting when:

- multiple futures should be explored in parallel
- block-level or lattice-level candidates are useful
- high uncertainty makes a single draft path weak
- the system wants a wider candidate frontier before arbitration

A future hybrid path may look like:

```text
VTG proposes high-probability transition families
Frozen MTP drafts the local prefix
Diffusion drafter explores branching alternatives
Confidence Scheduler selects safe prefixes
Target verifier accepts or rejects
EGGROLL tunes which drafter backend to use by task and hardware
```

---

## **Relationship to EGGROLL**

EGGROLL can optimize Frozen MTP deployment without touching the frozen backbone.

Possible EGGROLL targets:

- MTP head selection
- MTP depth by role
- confidence threshold by hardware profile
- prefix survival policy
- MTP vs VTG vs diffusion drafter selection
- tenant-private MTP scheduling policy
- edge promotion criteria for new MTP heads

A Frozen MTP learning event may look like:

```json
{
  "event_type": "frozen_mtp_outcome",
  "backbone_model": "semantic_core_vx",
  "mtp_head": "formatter_mtp_v1",
  "role": "formatter",
  "proposed_length": 4,
  "accepted_length": 3,
  "verification_cost_ms": 12,
  "latency_saved_ms": 46,
  "memory_overhead_mb": 18,
  "standalone_drafter_avoided": true,
  "hardware_profile": "mobile_gpu_low_memory",
  "policy_hash": "policy_hash",
  "signature": "ed25519"
}
```

This keeps the backbone stable while allowing the swarm to improve efficiency artifacts over time.

---

## **Initial Implementation Path**

### **Phase 1 — Measurement**

Instrument existing inference to measure:

- repeated token patterns
- accepted speculative depth
- KV-cache memory pressure
- potential MTP head attachment points
- role-specific predictability

### **Phase 2 — Formatter / Schema MTP Head**

Attach a small Frozen MTP head to the most deterministic local specialist.

Recommended first target:

```text
Formatter / Schema ELM
```

This offers high acceptance probability and simple validation.

### **Phase 3 — Code Specialist MTP Head**

Extend to code blocks where compiler, tests, static analysis, or verifier ELMs can validate the accepted prefix.

### **Phase 4 — Router-Aware MTP Selection**

Allow the Router to choose between:

- no speculation
- VTG-only proposal
- Frozen MTP
- standalone drafter
- diffusion-assisted drafter
- hybrid speculation

### **Phase 5 — EGGROLL Optimization**

Use compact outcome signals to tune thresholds, accepted depth, role policies, and MTP head promotion.

---

## **Design Principle**

Frozen MTP should follow this rule:

> If the backbone already computed the context, the drafter should reuse that state rather than reconstruct it.

For GNUS, this is an edge-first speculative decoding strategy.

It preserves base-model behavior, reduces memory pressure, avoids redundant prefill, and gives VTG another high-quality local candidate source.

---

## **Summary**

Frozen MTP gives GeniusCognitiveSystem a practical way to accelerate on-device and edge inference without deploying a separate drafter model.

It should be treated as an efficiency artifact attached to a frozen Semantic Core or ELM, verified before commitment, and instrumented so VTG and EGGROLL can learn when the MTP head is useful.

The strategic value is that GNUS nodes can gain speculative decoding speedups while preserving the stability of frozen deployed models and minimizing memory overhead.

---

[Companion: Speculative Decoding and VTG Candidate Scheduling](./speculative-decoding-and-vtg.md) | [Architecture Index](./INDEX.md)
