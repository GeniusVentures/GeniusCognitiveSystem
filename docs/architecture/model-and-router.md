# **5 Model Architecture**

---

## **5.1 Semantic Core**

The Semantic Core serves as the central reasoning substrate for generating foundational responses within GeniusCognitiveSystem. It is optimized for high-throughput, distributed inference across the decentralized network and provides broad comprehension, synthesis, and fallback response generation when specialist escalation is not required.

### 5.1.1 Base Model

The Semantic Core is intentionally selected from high-performing, medium-sized model families, with candidate classes including efficient general-purpose backbones suitable for quantized distributed deployment. This choice supports the system's objective of distributed inference and modularity, contrasting with monolithic scaling.

### 5.1.2 Quantization

To achieve energy-efficient inference and minimize memory usage, the Semantic Core is heavily optimized using custom weight compression techniques. Full details are in [16 SGFP4 Adaptive Quantization Format](./sgfp4-format.md).

* **SGFP4 Macroblocks:** Model weights are quantized using the SGFP4 adaptive format operating on 64x64 macroblocks with fixed 2048-byte payloads.
* **Per-Block Affine Decode:** Each macroblock stores a scale + bias header (packed FP16), with all modes using `w_hat = S * code + Bias`.
* **Adaptive Dual-Mode:** The encoder selects per block between **FP4_AFFINE** (4-bit signed codes) and **T158_AFFINE** (ternary, ~1.58-bit class) via a 32-step scale search and error minimization.
* **GPU-Decoded:** Weights are decoded in shared memory at inference time; mode flags are embedded in aligned offset low bits for zero-cost per-block branching.

The Semantic Core runs efficiently on the **GNUS compute nodes** using the MNN model runtime and GPU acceleration via Vulkan / MoltenVK.

---

## **5.2 Expert Models (EMs) and Specialist Modules**

The specialist execution layer is a set of **Expert Models (EMs)** and supporting specialist modules. This architecture promotes **specialization over monolithic scaling** by allowing GCS to deploy targeted expertise for distinct reasoning roles, subject domains, bounded judgments, and structured refinement tasks without assuming that every expert is a language generator.

### 5.2.1 Expert Model taxonomy

**Expert Model (EM)** is the neutral umbrella term for a model-backed cognitive expert. GCS describes expert execution using four orthogonal dimensions:

* **Cognitive role** — Planner, Verifier, Router, Formatter, Grounding, or a domain specialist.
* **Public contract/capability** — generation, bounded judgment, classification, ranking, refinement, infill, embedding, vision, or another operation.
* **Processor architecture** — autoregressive, direct-logit/classifier, diffusion/denoising, encoder, reranker, multimodal, or another computation mechanism.
* **Execution backend** — MNN, SGProcessing, CPU/Vulkan, or a remote SuperGenius node.

Two contract labels remain useful:

* **ELM — Expert Language Model:** an expert exposing language-generation or language-transformation behavior.
* **EJM — Expert Judgment Model:** an expert exposing bounded typed semantic judgments such as probabilities, classifications, rankings, scores, or choice distributions.

**Diffusion is a processor architecture, not a mutually exclusive contract.** A diffusion-backed expert may expose refinement/infill, bounded judgment, or both. **EDM — Expert Diffusion Model** may be used as shorthand for such an implementation, but an EDM may simultaneously act as an EJM when its public contract is judgment.

This separation prevents backbone or processor provenance from being confused with cognitive semantics. A Verifier, Planner, Formatter, Grounding expert, or domain specialist may combine multiple contracts while sharing one model artifact and runtime state.

### 5.2.2 Expert Language Models (ELMs)

An **ELM** is a specialized language-model expert optimized for a specific reasoning role, subject domain, operational function, or action-support task where the required output is language or text transformation.

An ELM may be implemented as:

* a compact standalone SLM
* a distilled language expert
* an adapter-augmented expert on a shared backbone
* a constrained service exposing a language-generation interface
* a secure language reasoning module behind a policy boundary

ELM remains a precise language-specific term. Non-language judgment or diffusion contracts should not be forced through a `string -> string` ELM abstraction.

### 5.2.3 Expert Judgment Models (EJMs)

An **EJM** is an Expert Model specialization for bounded semantic judgments where downstream software needs a typed machine-consumed result rather than generated prose.

EJM outputs may include:

* **boolean probability** — probability that a bounded condition is true
* **choice distribution** — probabilities across a fixed set of allowed choices
* **classification** — one or more labels with probability or uncertainty metadata
* **ordinal or continuous score** — a bounded score with calibrated confidence or uncertainty
* **ranking** — an ordering or score distribution across bounded candidates
* **parallel judgment bundle** — multiple independent judgments evaluated against the same context packet when shared-prefix execution is more efficient than separate calls

EJMs are suitable for intent and domain classification, risk estimation, expert and execution-mode selection, grounding or verification selection, tool-support and policy scoring, claim and evidence classification, contradiction detection, and bounded verifier or arbiter judgments.

A JEV-style implementation is an EJM even when it uses a causal language-model-derived backbone internally. A diffusion structured-read implementation is also an EJM when it exposes the same bounded judgment contract. The processor architecture does not define the expert contract.

For token-label judgment adapters, candidate labels must be validated at the actual answer boundary. When the selected architecture requires fixed single-token slots, every choice must map to exactly one distinct tokenizer token. The processor should request/evaluate logits for **all allowed candidate token ids**, rather than relying on global top-k logprobs that may omit a valid bounded choice.

Judgment execution should be bounded/read-only where possible: stop once the required probability-bearing state is available rather than completing an unnecessary generation or commit loop. Additional sampling or diffusion noise draws should be triggered only when uncertainty warrants them, with explicit bounds.

EJM output is advisory. Deterministic services remain authoritative for authorization, policy enforcement, capability grants, privacy boundaries, side effects, and execution approval. Low-confidence or poorly calibrated judgments should trigger fallback, bounded rereads, additional evidence collection, specialist escalation, verification, swarm execution, or human approval according to task risk.

### 5.2.4 Diffusion-Backed Expert Processing

A **diffusion processor** performs iterative denoising, masked/block infill, or structured reads over a seeded canvas. **EDM — Expert Diffusion Model** is shorthand for an expert implementation using this processor architecture.

The initial GCS refinement use remains deliberately small and bounded: **Micro-Diffusion Block Drafting** for low-entropy structured regions such as partially filled JSON, code-patch skeletons, schema/template repair, and tool-call argument infill.

A compatible diffusion processor may also implement **JUDGE** by seeding a mostly fixed answer canvas, leaving bounded answer slots unresolved, and reading the exact allowed-token distributions at those positions. In that use, the expert is an **EJM contract implemented by a diffusion processor**.

Refinement output remains provisional until accepted by the configured verifier path. Verification may be deterministic schema validation, compiler/test/static analysis, a tool dry-run, target-model verification, or another expert verifier. Judgment output follows normal EJM calibration and escalation rules.

This architecture does not make image, audio, or video diffusion an MVP dependency.

### 5.2.5 Role-Based Experts

Representative role-based experts include:

* **Planner Expert:** Interprets the task, estimates complexity, and recommends an execution path. It may use an EJM for bounded routing judgments and an ELM when an explicit generated plan is required.
* **Primary Draft ELM:** Produces an initial language answer quickly when a specialist draft is preferred over direct Semantic Core output.
* **Verifier Expert:** Checks correctness, consistency, and policy adherence. An EJM may return bounded correctness or contradiction probabilities; an ELM may generate an explanation when requested.
* **Arbiter Expert:** Resolves conflicts between multiple drafts or critiques using bounded judgments, generated synthesis, or both.
* **Refiner / Formatter Expert:** Improves clarity, structure, style, or schema compliance. It may use an ELM, a diffusion processor for bounded structured repair, or deterministic schema machinery.
* **Grounding Expert:** Aligns claims with trusted public or private knowledge sources and may combine bounded evidence judgments with generated explanation.
* **Tool-Support Expert:** Helps prepare tool calls and interpret tool results before intermediary enforcement. Judgment output never replaces deterministic permission and side-effect checks.

A single cognitive role may expose more than one expert contract while sharing a model artifact, adapter, cached context, or runtime state.

### 5.2.6 Domain-Specific Experts

Representative domain-specific experts include:

* **Math Specialist:** Focused on numerical and structured mathematical reasoning.
* **Code Specialist:** Focused on source reasoning, implementation support, and development workflows.
* **Scientific Specialist:** Focused on scientific reasoning and evidence-heavy tasks.
* **Legal / Compliance Specialist:** Focused on regulated reasoning and policy-heavy domains.
* **Operations or Workflow Specialist:** Focused on tenant-specific procedures and internal execution patterns.
* **Customer Support Specialist:** Focused on support workflows and user-facing operational tasks.
* **Finance Specialist:** Focused on finance-heavy reasoning and structured analytical tasks.

A domain-specific expert is not required to be an ELM. Its contract capabilities and processor architecture are selected independently according to the execution stage.

### 5.2.7 Private Expert Models

Organizations may deploy private EMs trained or adapted on proprietary data, workflows, and response patterns. Private ELMs, EJMs, diffusion-backed experts, and future capability/processor combinations may run fully inside a tenant boundary, on local infrastructure, on permissioned GNUS nodes, or within restricted private or hybrid swarms.

### 5.2.8 Expert Invocation Patterns

Expert Models may be invoked in several ways:

* **single-pass support** — one expert supports the Semantic Core
* **sequential content chain** — one language expert drafts, another verifies, another refines
* **parallel swarm participation** — multiple experts produce competing or complementary artifacts
* **parallel judgment fan-out** — multiple bounded EJM judgments share one context packet and return typed probability-bearing results
* **sequential judgment chain** — dependent bounded judgments are staged so later judgments may condition on earlier semantic results
* **bounded diffusion/refinement** — a diffusion processor proposes a repaired/refined block that is verified before commitment
* **arbiter-mediated synthesis** — an Arbiter consumes expert artifacts and resolves or merges distributed proposals

The execution graph must distinguish **content artifacts** from **judgment/control artifacts**. An EJM result normally informs routing, verification, arbitration, capability selection, or escalation and should not replace the current generated content unless the stage contract explicitly says so.

### 5.2.9 Model Processing and Execution Backends

Expert identity describes **what cognitive job is being performed**. A model processor describes **how the model computes that result**, and an execution backend describes **where/how the tensor workload runs**.

Conceptually:

```text
Cognitive Expert Role
    ↓
Required Contract / Capabilities
    ├── ELM: GENERATE / STREAM
    ├── EJM: JUDGE / CLASSIFY / RANK / SCORE
    └── REFINE / INFILL / EMBED / VISION / ...
    ↓
Processor Architecture
    ├── autoregressive / direct-logit
    ├── diffusion / denoising
    ├── encoder / reranker
    └── multimodal / future
    ↓
Execution Backend (MNN / SGProcessing / CPU / Vulkan / remote node)
```

This separation allows the same `JUDGE` contract to be implemented by a causal selected-logit processor or a diffusion structured-read processor, allows one artifact to expose multiple capabilities, and prevents the RuntimeCoordinator from depending on model-runtime internals.

### 5.2.10 Legacy MVP Specialists

The earlier Grammar Specialist and Math Specialist remain compatible as concrete instances of the broader Expert Model architecture. Grammar correction belongs naturally under refiner / formatter roles, while math remains one domain-specific specialist family. Existing `IELM` code remains a compatibility path for language experts during migration to the neutral EM abstraction.

---

# 6 Router Design

The Router Layer serves as the critical initial processing point for client requests, acting as the task orchestrator in the system's architecture. Its primary function is to intelligently route incoming tasks to the appropriate execution mode, Semantic Core path, Expert Model set, grounding path, memory path, and verification strategy.

## 6.1 Router and Planner Responsibilities

The orchestration path is responsible for:

* classifying task type and complexity
* deciding whether retrieval is required
* selecting execution mode
* selecting the Semantic Core and required Expert Models
* selecting the required expert capability first, then a compatible processor architecture and backend; for example ELM generation, EJM judgment through causal or diffusion execution, or bounded refinement
* deciding whether verification or arbitration is required
* determining whether private knowledge grounding is required
* deciding whether tenant-scoped or private memory should be loaded
* enforcing policy constraints
* determining latency, privacy, token, and spend budgets
* producing an execution graph for local or distributed completion

## 6.2 Initial MVP Router

The initial implementation relies on a **rule-based detection** system for rapid deployment and predictable routing behavior. This system analyzes the characteristics of the incoming prompt to determine the required execution path:

* **Numeric density -> Math Specialist / Verifier path:** Prompts containing a high density of numerical data or mathematical keywords are routed to a math-oriented specialist path, optionally followed by verification.
* **Code syntax -> Code Specialist path:** Queries identified as containing code syntax or complex programming logic are routed toward code-capable execution.
* **Grounding-sensitive request -> Grounding path:** Questions that require factual validation or trusted references trigger grounding retrieval and validation.
* **Formatting-sensitive request -> Refiner / Formatter path:** Requests that require structured or style-constrained output trigger formatting-aware expert support.
* **Low complexity -> Semantic Core only:** Simple or general-purpose prompts that require only foundational language understanding are routed exclusively to the Semantic Core.
* **High complexity / uncertainty -> multi-stage path:** Complex, ambiguous, or high-stakes prompts may trigger a planner, verifier, arbiter, or swarm-assisted execution path.

## 6.3 Future Router Evolution

The roadmap includes upgrading the router to a more sophisticated model-based system to enhance routing accuracy and performance. This future iteration can progress through multiple stages:

* **Heuristic MVP router:** fast rule-based triggers for numeric, code, formatting, grounding, tool, and workflow cues.
* **Lightweight classifier / EJM router:** a compact learned Expert Judgment Model trained on prompt embeddings, task outcomes, and execution traces that returns calibrated distributions for bounded routing judgments.
* **Cognitive planner:** a planner-level expert capable of decomposing tasks into multi-step workflows involving reasoning, retrieval, tools, verification, arbitration, and private expert selection.

* **Execution Awareness:** Future routing should incorporate latency budget, policy constraints, privacy mode, prior expert success, disagreement risk, calibrated uncertainty, escalation thresholds, and tenant boundary requirements.
