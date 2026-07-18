# **27. Execution Integrity System (EIS)**

**Status:** Draft for review  
**Applies to:** SGFP4 kernel specification, Reputation-Weighted Consensus, Speculative Decoding, GAML / Cognitive Asset Model  
**Supersedes:** N/A

---

## **27.1 Purpose**

The **Execution Integrity System (EIS)** is the GCS subsystem responsible for verifying that distributed computation was executed faithfully according to a declared execution contract.

EIS verifies **execution honesty**, not semantic answer quality. It answers:

> Did this node run the declared model, adapter, SGFP4 container, kernel manifest, determinism class, sampling seed, and execution profile?

It does not answer:

> Is the answer true, useful, safe, well-grounded, or well-written?

Those semantic responsibilities remain with grounding, verifier specialists, arbitration, synthesis, and reputation-weighted semantic consensus.

This separation is foundational. Semantic consensus is good at evaluating answer quality over time, but it is structurally weak against a cheaper attack: serving semantically plausible answers from a smaller or cheaper substitute while charging as if the declared specialist executed. EIS closes that execution-honesty gap without adding full per-query consensus or zk proof overhead to the interactive inference path.

---

## **27.2 Motivation and Threat Model**

### **27.2.1 Why zk is out of scope at the GCS layer**

Zero-knowledge computation verification remains available at the substrate layer for low-level deterministic computation verification. It is deliberately not required for GCS inference.

zk proofs can verify deterministic execution, but they cannot verify that an answer is true, useful, safe, or high quality. For interactive inference, proof generation also creates an unacceptable latency and energy tax. Semantic quality is therefore handled by reputation-weighted consensus, grounding, verification, and synthesis. EIS addresses the remaining gap when zk is not used for inference: **execution honesty**.

### **27.2.2 The lazy-node / model-substitution attack**

With no execution attestation, the cheapest rational attack on a decentralized inference network is not producing obviously bad answers. It is producing cheap answers.

Examples:

- An operator advertises a 4B specialist, serves output from a 1B model, and bills full price.
- A node serves a generic base model instead of the declared specialist adapter.
- A node uses a lower-precision derivative of the required SGFP4 container.
- A node skips layers or exits early on requests it predicts are easy.
- A node serves a stale adapter after a swarm retraining epoch.

On easy queries, the smaller or stale substitute may be semantically close enough that consensus does not detect the substitution quickly. This is especially dangerous because easy queries are common and economically important.

### **27.2.3 Security economics**

EIS is designed to make honesty the best economic strategy.

Without execution integrity:

```text
Advertise expensive model
↓
Run cheaper substitute
↓
Collect full reward
↓
Hope semantic consensus misses it
```

With EIS:

```text
Accept execution contract
↓
Risk unpredictable teacher-forced spot checks
↓
Substitution becomes slashable fraud
↓
Running the declared model becomes the higher expected-return strategy
```

EIS does not make cheating impossible. It makes the cheapest cheating strategy economically irrational by combining unpredictable spot checks, execution claims, checkpoint-band comparison, fraud verdicts, slashing, and reputation penalties.

### **27.2.4 Design principles**

1. **Integrity is independent of semantic correctness.** EIS proves whether the declared computation was executed, not whether the answer is good.
2. **Verification must be cheaper than execution.** Checking should be cheaper than serving so a verification market is economically viable.
3. **Honest hardware diversity must be permitted.** Heterogeneous GPUs, CPUs, and NPUs should be schedulable when their drift is measurable and bounded.
4. **Substitution must remain detectable.** Tolerance bands may forgive honest hardware drift, but they must not forgive different weights, adapters, kernels, or decode processes.
5. **Verification should stay off the latency-critical path.** Spot checks are sampled and asynchronous unless a policy explicitly requires synchronous verification.
6. **Determinism should be economically rewarded.** More deterministic kernels should require less redundancy and therefore earn better effective margins.

---

## **27.3 Execution Contracts**

### **27.3.1 Definition**

An **Execution Contract** is the concrete, attestable profile a node accepts before running a job.

At minimum, it includes:

- model hash
- adapter version hash
- SGFP4 container hash
- kernel manifest ID
- determinism class
- sampling seed
- checkpoint schedule
- execution profile, including accumulation and fusion order where applicable
- allowed hardware or kernel path constraints

A node accepting a job attests that it executed under exactly this contract. Serving under any other profile is a substitution.

### **27.3.2 Contract pinning**

The execution contract pins every property that can materially affect output or verification:

- **Weights:** model, specialist, adapter, and quantization container hashes.
- **Kernels:** kernel manifest, determinism class, reduction order, fusion order, and runtime profile.
- **Sampling:** seed, sampler type, temperature, top-k/top-p settings, and any other decoding controls.
- **Checkpoints:** checkpoint tensor schedule, digest format, comparison domain, and class band.

This turns distributed inference from a vague promise into a numerical contract that EIS can test.

---

## **27.4 Determinism Classes**

### **27.4.1 Definition**

Bit-exact reproducibility across heterogeneous hardware is not always achievable once FP16 accumulation, fused kernels, or vendor NPU graphs enter the path. EIS therefore assigns each `(kernel implementation × hardware path)` combination a **determinism class** in its kernel manifest.

Cross-node verification is performed only within compatible classes. Each class carries its own comparison semantics and spot-check redundancy factor.

| Class | Name | Guarantee | Comparison semantics | Spot-check redundancy |
|-------|------|-----------|----------------------|-----------------------|
| **A** | Reference-integer | Bit-exact | Output hash equality | k = 1-2 |
| **B** | Bounded-drift | Deterministic per device; bounded numeric drift across devices | Checkpoint-band match | k = 2-3 |
| **C** | Non-deterministic | No cross-run guarantee, such as vendor-fused NPU graphs or nondeterministic atomics | Checkpoint-band match with widened bands | k = 3-5, scaled by observed variance |

### **27.4.2 Class A - Reference-integer semantics**

The SGFP4 ternary path can be integer-dominant and should target **reference integer semantics**: a normative specification of accumulation order, kernel fusion order, rounding mode, and seeded sampling such that conforming implementations produce bit-identical outputs for identical inputs.

Verification is an output hash comparison. This is the cheapest possible attestation path and should be the preferred target for kernel authors.

A kernel claiming Class A must pass bit-exact conformance vectors before registration.

### **27.4.3 Class B - Bounded-drift semantics**

FP4-affine paths with FP16 accumulation on well-behaved hardware are often deterministic on a given device but may drift across devices. Class B kernels must declare:

- accumulation width
- reduction order
- fusion profile
- per-op or per-block drift bounds
- checkpoint-band derivation method

Class B verification uses checkpoint-band matching rather than raw equality.

### **27.4.4 Class C - Non-deterministic paths**

Vendor-fused NPU graphs and kernels using nondeterministic parallel reductions may not guarantee same-device reproducibility. These are not banned because excluding NPUs would forfeit important perf/watt advantages, especially on mobile.

Class C kernels carry higher redundancy, wider bands, and lower effective reward after verification cost. This creates a natural economic gradient toward Class A and Class B without banning useful hardware.

### **27.4.5 Constraints on kernel authors**

These constraints are normative from the first release:

1. Every kernel must register a determinism class in its manifest; unclassified kernels are not schedulable.
2. Class A/B kernels must not change reduction order based on runtime conditions unless each variant is registered as a distinct manifest ID.
3. Sampling must be seeded from the execution contract; no entropy is drawn locally.
4. Checkpoint tensors must be exportable at negligible cost, without requiring re-execution to produce them.
5. Registration fails if honest drift cannot be separated from plausible substitution margins.

---

## **27.5 Checkpoint-Band Matching**

### **27.5.1 Checkpoints, not per-op comparison**

Per-op tolerance comparison fails in deep networks because small allowed differences compound across transformer blocks. Honest hardware variants may begin failing at deep layers even though every local operation stayed inside tolerance.

EIS therefore compares a small set of **checkpoint tensors** rather than every op.

Representative checkpoints:

- output activation of each transformer block, or every Nth block depending on model size
- final pre-sampling logits
- optional session or KV-cache checkpoint digests for long-running contexts

Bands at each checkpoint are sized for accumulated drift up to that depth, not single-op drift.

### **27.5.2 Comparison domain**

Raw FP16/FP32 equality is neither achievable nor required for Class B/C paths. Checkpoint tensors are compared in a reduced-precision comparison domain. The default target is an FP12-width rounded or truncated representation, though the exact comparison domain is declared in the kernel manifest.

Two executions match at a checkpoint if their domain-reduced tensors agree within the class band.

- **Class A:** exact digest equality; band = 0.
- **Class B:** per-element agreement in the reduced domain at sampled positions, with an allowed exception rate `epsilon` for tie-straddling elements near rounding boundaries.
- **Class C:** Class B semantics with widened bands and higher `epsilon`, derived from measured cross-device variance during registration.

### **27.5.3 Registration invariant: the band forgives hardware, not weights**

The security argument depends on a separation of scales: honest hardware drift must be materially smaller than the checkpoint-tensor distance between the declared model and plausible substitutes.

At registration, EIS measures:

1. the cross-device drift distribution of honest executions, and
2. the checkpoint-tensor distance between the declared specialist and nearest plausible substitutes, including smaller models, lower quantization levels, pruned variants, stale adapters, and generic base models.

Registration fails if the band required to keep honest false-positive rates below target overlaps the substitution-detection margin. In that case, one of the following must happen:

- the kernel tightens determinism and moves toward Class A/B,
- checkpoint density increases,
- the comparison domain changes,
- or the model/kernel/hardware combination is not schedulable for the claimed class.

This turns "the band forgives hardware, not weights" into an enforced registration invariant.

### **27.5.4 Calibration and drift monitoring**

Band parameters are stored as Cognitive Assets with provenance:

- band width
- exception rate `epsilon`
- checkpoint density
- drift distribution
- substitution margin
- hardware class
- kernel manifest
- model and adapter version

They are recalibrated when a new hardware class joins the network, a model/adapter version ships, or observed honest-mismatch rates move outside control limits.

Verifier disagreement statistics feed the same reputation and telemetry pipeline as semantic consensus, but remain a separate execution-integrity signal.

---

## **27.6 Teacher-Forced Spot-Check Protocol**

### **27.6.1 The autoregressive divergence problem**

Free-running generation cannot be compared reliably across nodes. Near a tie-break token, a sub-band numeric wiggle can legitimately flip the argmax or the seeded sampler choice. From that token onward the two sequences diverge, even if both executions are honest.

A scheme that regenerates and diffs sequences will therefore false-accuse honest nodes after the first close call.

### **27.6.2 Teacher-forced replay**

EIS never lets verification branch.

Protocol:

1. The serving node returns its response plus an execution claim: token sequence, execution-contract hash, and domain-reduced checkpoint digests at a protocol-chosen stride.
2. A spot-checker replays the claimed token sequence as fixed input using teacher forcing: one prefill pass over prompt + claimed output.
3. The checker compares domain-reduced logits and checkpoint tensors at sampled positions against the claim under the class band.
4. Positions are sampled unpredictably using a VRF seeded from job hash and checker identity.

The server cannot know in advance which token positions and checkpoints will be tested, making precomputed partial honesty unattractive.

### **27.6.3 Cost profile**

Teacher-forced verification is `O(prefill)` rather than `O(generation)`: one parallel forward pass over the full sequence, no sequential decode loop.

For typical response lengths, this makes checking cheaper than serving. That is the correct economic ordering for a decentralized verification market.

The same machinery is also used by speculative decoding. Verifying claimed tokens against full-model logits in a single pass is the acceptance step of speculative decoding, so the speculative decode path and the EIS verification path can share kernels, scheduling, and SGFP4 container handling.

### **27.6.4 Sampling-consistency check**

Because sampling is seeded by the execution contract, the checker also verifies that each claimed token is consistent with the seeded sampler applied to the recomputed domain-reduced logits at that position, within the tie-straddle exception rate `epsilon`.

This closes the loophole where a node computes honest logits but emits tokens from a different or cheaper decode process.

### **27.6.5 Scheduling and economics**

- Spot-check rate: `k` randomized checks per node per epoch, set by determinism class and modulated by reputation.
- New, low-reputation, recently flagged, or high-value nodes are checked more aggressively.
- Checkers are selected by VRF from nodes holding the same compatible model/kernel class.
- Check work is paid from the verification margin priced into jobs.
- A confirmed mismatch beyond band + `epsilon` at unpredictable positions is execution fraud.
- Fraud produces stake slashing and reputation penalties through the existing reputation channel.
- Borderline results trigger escalation to additional independent checkers before penalty.

All claims, check results, and verdicts are recorded as Cognitive Assets with provenance and trust class.

---

## **27.7 Interaction with Semantic Consensus**

| Concern | Mechanism | Cost |
|---------|-----------|------|
| Did the node run the specified model, weights, adapter, kernel, and sampler honestly? | EIS checkpoint-band attestation + teacher-forced spot checks | O(prefill), sampled, usually off the latency path |
| Is the answer good, correct, grounded, or safe? | Reputation-weighted semantic consensus, grounding, verifier specialists, arbitration, synthesis | Applied where quality matters |
| Was routing or arbitration good? | Semantic consensus + Cognitive Asset records | Unchanged |

Consequences:

- Per-query multi-node execution redundancy is not required for execution honesty.
- Redundancy can be reserved for quality-critical, high-value, or low-trust contexts.
- The interactive path can serve at single-node latency.
- Verification can be asynchronous and sampled.
- Reputation integrates two independent signals: execution honesty and semantic quality.
- A Sybil fleet must spend real compute running real models to survive spot-checks before it can try to game semantic scores.

---

## **27.8 Interaction with GAML and Cognitive Assets**

EIS produces and consumes Cognitive Assets.

Representative EIS assets:

- kernel registration records
- determinism-class manifests
- execution contracts
- checkpoint calibration records
- substitution-margin measurements
- execution claims
- teacher-forced spot-check results
- sampling-consistency results
- escalation records
- fraud verdicts
- reputation evidence

GAML stores these assets with provenance, trust class, signatures where applicable, and graph relationships to the node, model, adapter, kernel, hardware class, and job.

This makes EIS auditable by the same memory machinery that stores reasoning traces, consensus records, tool results, and distillation samples.

---

## **27.9 Open Items**

1. **Checkpoint stride tuning** per model depth and size, balancing digest bandwidth against mismatch localization.
2. **Epsilon calibration methodology** for tie-straddle rates per determinism class during kernel registration.
3. **Class C variance measurement** across vendor SDK versions and driver updates, especially for NPU-fused graphs.
4. **KV-cache checkpointing** for long-session serving: decide whether session-boundary KV digests are required or whether logit-level checks subsume them.
5. **Adapter hot-swap windows** for swarm retraining epochs: define grace-period semantics to avoid penalizing version skew as fraud.
6. **Driver and firmware identity**: define whether driver, firmware, and vendor runtime versions are part of the execution contract for each determinism class.
7. **Synchronous verification policy**: identify which workloads, if any, require blocking EIS verification before finalization.

---

## **27.10 Cross-References**

- **SGFP4 container/kernel spec:** determinism-class manifest fields, reference integer semantics for the ternary path, and conformance vectors.
- **Reputation-weighted consensus:** verdict ingestion, slashing, checker selection, and Sybil economics.
- **Speculative decoding:** shared teacher-forced acceptance machinery.
- **GAML / Cognitive Asset Model:** execution claims, calibration parameters, registration records, and verdicts stored as Cognitive Assets with provenance and trust class.
- **Executive Controller:** execution-contract and EIS policy selection before dispatch.
