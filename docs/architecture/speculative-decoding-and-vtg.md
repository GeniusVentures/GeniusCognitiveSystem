# **Speculative Decoding and VTG Candidate Scheduling**

## **Purpose**

This document extends the Objective Memory and Verified Transition Graph (VTG) architecture with a dedicated speculative decoding execution model.

The goal is to define how GeniusCognitiveSystem can combine:

- speculative decoding
- semi-autoregressive generation
- confidence-scheduled prefix retention
- VTG candidate frontiers
- GAML context packets
- ELM verifier feedback
- EGGROLL optimization signals

This document should be read as a companion to [Objective Memory and Verified Transition Graph](./objective-memory-vtg.md).

Objective Memory defines the persistent transition-learning substrate.

This document defines one major runtime use of that substrate.

---

## **Why this extension exists**

The current Objective Memory architecture already states that speculative decoding helps when another model or head can draft likely tokens, while VTG adds a persistent graph of verified cognitive transitions.

That description is directionally correct, but incomplete.

Modern speculative decoding is not just:

```text
draft tokens -> verify tokens
```

The more useful pattern is:

```text
produce multiple draft positions
score confidence per position
keep the safe prefix
reject low-confidence suffix
verify accepted prefix with target model
feed outcome back into scheduling and memory
```

This matters because the system does not need to treat every drafted token equally.

A draft may be highly reliable for the first three positions and unreliable after the fourth.

A scheduler can therefore keep the accepted prefix and drop the uncertain tail before wasting verification capacity.

That is directly compatible with VTG.

---

## **Core Concept**

The speculative decoding runtime should use three cooperating components:

| Component | Responsibility |
|----------|----------------|
| **Drafter** | Proposes one or more future tokens, blocks, tool fragments, schema segments, or reasoning states. |
| **Confidence Scheduler** | Scores each draft position and decides how much of the proposed prefix should survive. |
| **Target Verifier** | Uses the Semantic Core, ELM, verifier, tool result, schema checker, or arbitration layer to accept or reject the candidate prefix. |

The VTG layer becomes the memory-backed proposal source.

Instead of relying only on a small neural drafter, the system can combine:

- neural draft heads
- small ELM drafters
- n-gram or prompt lookup
- VTG transition candidates
- tenant-private workflow transitions
- schema-aware formatters
- tool-call templates

The runtime then verifies the proposed path before committing output.

---

## **Semi-Autoregressive Drafting**

Semi-autoregressive drafting generates multiple future positions in a single round while still preserving an autoregressive anchor.

Conceptual flow:

```text
Input prefix
    ↓
Target model produces anchor token
    ↓
Drafter proposes positions N+1, N+2, N+3, N+4...
    ↓
Each position receives a confidence score
    ↓
Scheduler keeps the largest safe prefix
    ↓
Target verifier accepts or rejects
    ↓
Accepted prefix is committed
```

This is different from ordinary multi-token prediction because the system does not blindly trust all predicted future tokens.

It schedules acceptance based on confidence and verification cost.

---

## **Confidence-Scheduled Prefix Retention**

The key scheduling rule is:

> Keep the longest prefix whose confidence and compatibility exceed the required threshold; drop the suffix before verification or final commitment.

Example:

```text
Draft:       A B C D E
Confidence:  .97 .94 .91 .62 .41
Threshold:   .90
Keep:        A B C
Drop:              D E
```

This produces better behavior than all-or-nothing speculation.

The retained prefix can be verified efficiently, while low-confidence suffixes are discarded early.

The scheduler may use:

- per-token confidence
- per-block confidence
- VTG edge confidence
- verifier prior confidence
- role-specific acceptance history
- domain-specific acceptance history
- tenant-policy compatibility
- latency budget
- current model temperature
- entropy estimate
- context stability

---

## **VTG as a Speculative Candidate Source**

VTG can provide draft candidates in several forms:

| Candidate Type | Example |
|----------------|---------|
| Token-block continuation | common code, JSON, or prose fragment |
| Schema segment | next valid JSON key/value structure |
| Tool-call fragment | arguments for a known API call pattern |
| Code patch block | recurring fix pattern for compile/test failures |
| Planner transition | next step in a known workflow |
| Verifier correction | common repair after a known failure |
| Synthesis move | merge or rewrite pattern repeatedly accepted by arbitration |

This means speculative decoding is not limited to raw tokens.

For GeniusCognitiveSystem, the drafter can operate over **cognitive states**.

A token block is just the first implementation target.

---

## **Confidence Inputs**

The Confidence Scheduler should combine local and distributed signals.

Representative confidence inputs:

```json
{
  "draft_logprob": 0.92,
  "vtg_edge_confidence": 0.88,
  "acceptance_rate_by_position": [0.96, 0.93, 0.89, 0.61],
  "role_acceptance_rate": 0.91,
  "tenant_policy_compatible": true,
  "schema_valid_so_far": true,
  "context_packet_stable": true,
  "verifier_prior_score": 0.84,
  "latency_budget_ms": 120,
  "risk_class": "low"
}
```

The scheduler should produce:

```json
{
  "candidate_id": "candidate_state_id",
  "proposed_length": 8,
  "scheduled_prefix_length": 5,
  "dropped_suffix_length": 3,
  "reason": "confidence_decay_after_position_5",
  "requires_target_verification": true
}
```

---

## **Position-Wise Acceptance Tracking**

The system should track acceptance by draft position.

This is important because a drafter may be reliable at early positions and poor at later positions.

Recommended metrics:

- acceptance rate at position 1
- acceptance rate at position 2
- acceptance rate at position 3
- acceptance rate at position N
- conditional acceptance given previous positions accepted
- acceptance decay curve by domain
- acceptance decay curve by ELM role
- acceptance decay curve by tenant workflow
- acceptance decay after policy or context changes

This can be stored as edge metadata in VTG or as an associated scheduling profile.

Example:

```json
{
  "state_family": "code_patch_block",
  "draft_profile": "local_code_drafter_v2",
  "conditional_acceptance": [0.94, 0.91, 0.87, 0.73, 0.55],
  "recommended_max_prefix": 3,
  "last_updated_epoch": 42
}
```

---

## **Hardware-Aware Prefix Scheduling**

The scheduler should be hardware-aware.

Different GNUS nodes may have different optimal speculative depths.

A powerful GPU node may benefit from larger parallel draft blocks.

A mobile or low-memory node may benefit from shorter drafts to avoid verification waste.

Scheduling inputs should include:

- GPU class
- memory bandwidth
- batch size
- current queue depth
- model size
- quantization mode
- KV-cache pressure
- network latency
- expected verification cost
- expected rollback cost

The same VTG candidate frontier may therefore be scheduled differently on different devices.

---

## **Integration with Objective Memory**

Objective Memory should be extended with scheduling-aware metadata.

Transition edges should optionally store:

- draft depth attempted
- prefix length accepted
- suffix length dropped
- position-wise acceptance
- confidence threshold used
- verifier cost
- rollback cost
- latency saved
- target model version
- drafter version
- hardware profile

This allows VTG to learn not merely which transition is valid, but **how far ahead it is safe to speculate**.

That distinction is critical.

The graph should learn:

```text
This continuation is usually valid for 3 positions.
This continuation is risky after 5 positions.
This continuation works under the Code Specialist but fails under Formatter.
This continuation works on tenant-private API workflows but not globally.
```

---

## **Integration with GAML**

GAML provides the context packet that constrains speculation.

The same visible token prefix may require different speculative continuations depending on:

- selected Bridge Blocks
- active policy
- user preference
- tenant workflow
- current tool state
- prior codebase state
- grounding facts

Therefore, confidence scheduling must include the GAML context packet hash.

```text
spec_key = H(
  model_version,
  drafter_version,
  tokenizer_version,
  context_packet_hash,
  previous_state,
  current_state,
  role_id,
  policy_hash
)
```

This prevents speculative reuse across incompatible memory states.

---

## **Integration with ELMs**

Different ELM roles should have different speculative policies.

| ELM Role | Speculation Policy |
|---------|--------------------|
| Primary Draft ELM | Moderate to aggressive speculation for low-risk text and code. |
| Code Specialist | Aggressive speculation only when tests, compiler, or verifier can validate. |
| Formatter ELM | High speculation for schemas and deterministic formatting. |
| Tool-Support ELM | Conservative speculation because malformed calls can create downstream risk. |
| Verifier ELM | Conservative speculation; prioritize consistency over speed. |
| Grounding ELM | Conservative speculation when evidence alignment is required. |
| Synthesizer / Arbiter | Moderate speculation over known synthesis patterns, but arbitration remains authoritative. |

This avoids applying the same speculative depth across incompatible cognitive roles.

---

## **Integration with Epistemic Arbitration**

Epistemic Arbitration should treat speculative candidates as provisional.

A speculative candidate may be:

- accepted directly after verification
- accepted as a draft but challenged by critics
- rejected due to contradiction pressure
- shortened due to confidence decay
- escalated to a stronger ELM path
- blocked by policy or grounding requirements

The arbitration trace should record whether the final output relied on speculative candidates.

Example:

```json
{
  "speculative_decoding": {
    "enabled": true,
    "candidate_source": "VTG + local_drafter",
    "proposed_length": 6,
    "scheduled_prefix_length": 4,
    "verified_prefix_length": 4,
    "rejected_suffix_length": 2,
    "arbitration_effect": "accepted_after_verifier_check"
  }
}
```

This keeps speculation inspectable without exposing hidden chain-of-thought.

---

## **Integration with EGGROLL**

EGGROLL can optimize the speculative scheduler.

Possible optimization targets:

- confidence threshold by ELM role
- maximum draft length by hardware profile
- prefix survival policy
- candidate frontier ranking
- drafter selection
- rollback penalty weighting
- risk-class thresholds
- tenant-specific speculative depth
- position-wise acceptance prediction

A speculative scheduling event can be emitted as a compact learning signal:

```json
{
  "event_type": "speculative_schedule_outcome",
  "state_family": "formatter_schema_block",
  "candidate_source": "vtg_edge",
  "drafter_id": "formatter_drafter_v1",
  "target_model": "semantic_core_vx",
  "proposed_length": 7,
  "scheduled_prefix_length": 5,
  "accepted_length": 5,
  "rejected_suffix_length": 2,
  "latency_saved_ms": 83,
  "verification_cost_ms": 21,
  "hardware_profile": "mobile_gpu_low_memory",
  "policy_hash": "policy_hash",
  "result_signature": "ed25519"
}
```

This event can update VTG edge metadata or drive EGGROLL optimization of scheduling policies.

---

## **Recommended Runtime Flow**

```text
1. Router receives request.
2. GAML builds context packet.
3. Router selects Semantic Core / ELM path.
4. VTG returns candidate frontier.
5. Drafter proposes semi-autoregressive continuation.
6. Confidence Scheduler trims unsafe suffix.
7. Target model or verifier validates retained prefix.
8. Accepted prefix is committed.
9. Rejected suffix is dropped or regenerated.
10. Thinking trace records speculation outcome.
11. VTG edge metadata is updated.
12. EGGROLL receives compact scheduling event when useful.
```

This keeps speculation fast, measurable, and subordinate to verification.

---

## **Initial Implementation Targets**

The first speculative decoding targets should be low-risk and easy to verify.

Recommended targets:

1. **JSON and schema generation**
   - high structure
   - easy validation
   - strong formatter ELM fit

2. **Tool-call templates**
   - useful but requires conservative scheduling
   - validate before execution

3. **Code patch continuations**
   - validate with compiler, tests, static analysis, or code verifier

4. **Formatter and refiner output**
   - low factual risk
   - high repetition

5. **Known tenant workflows**
   - strong private VTG fit
   - policy-bound context

Avoid early use for:

- high-stakes legal or medical factual conclusions
- grounding-sensitive claims
- ambiguous arbitration decisions
- high-entropy creative synthesis
- tool execution with side effects before validation

---

## **Performance Metrics**

The system should track:

- proposed draft length
- scheduled prefix length
- accepted prefix length
- rejected suffix length
- acceptance rate by position
- conditional acceptance by position
- latency saved
- verification cost
- rollback cost
- memory lookup cost
- edge confidence improvement
- failure rate by role
- failure rate by hardware profile
- tenant-private vs global performance

The main success metric is not raw draft length.

The main success metric is:

> verified accepted length per unit of latency and risk.

---

## **Design Principle**

The speculative decoding layer should follow this rule:

> Draft aggressively when the domain is low entropy, schedule conservatively when confidence decays, and always verify before commitment.

For GeniusCognitiveSystem, the important enhancement is that speculation becomes memory-backed and swarm-improving.

The system should not only draft faster.

It should learn where, when, and how far ahead it is safe to draft.

---

[Companion: Objective Memory and Verified Transition Graph](./objective-memory-vtg.md) | [Architecture Index](./INDEX.md)
