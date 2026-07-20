# GCS Architecture Context

Generated from `docs/architecture/` ingest (merge mode). All 29 source documents classified as DOC (high confidence).

## Topics

### System Identity and Core Architecture

**Source:** `docs/architecture/executive-summary.md`

GeniusCognitiveSystem is a distributed, modular, reputation-weighted cognitive system built on GNUS.ai infrastructure, with Genius Expert Language Model (Genius ELM) as the semantic core inference engine. It is organized as a cognitive operating system with four foundational subsystem families:

- **Semantic Core and ELMs** -- produce reasoning, language, specialist, and workflow outputs.
- **GAML** -- provides governed cognitive memory, knowledge, provenance, and asset storage.
- **EIS** -- verifies that distributed computation was executed according to the declared execution contract.
- **Consensus, Verification, and Synthesis** -- determine semantic quality, resolve disagreement, and form the final response.

The system separates cognitive functions from implementation mechanisms: memory, planning, routing, reasoning, execution integrity, verification, synthesis, tool use, and learning evolve independently while remaining part of one coherent execution model. The system is local-first but swarm-capable.

**Source:** `docs/architecture/system-overview.md`

The layered cognitive stack consists of 8 layers: Client/API, Orchestration, Expert Execution, Execution Integrity, Consensus/Grounding, Security/Tool Intermediary, Memory, and Distributed Infrastructure. Component mapping covers Compute Layer (MNN, Vulkan/MoltenVK, SGFP4 codec, EIS), Distributed Layer (libp2p, IPFS-lite, RocksDB, CRDTs, gRPC), and Security Layer (libsecp256k1, ed25519, OpenSSL, wallet-core, Tool Intermediary boundary).

**Source:** `docs/architecture/README.md`

Architecture documentation overview for the Genius Cognitive System, an integrated distributed cognitive platform.

### Semantic Core and Expert Language Models

**Source:** `docs/architecture/model-and-router.md`

The Semantic Core serves as the central reasoning substrate, optimized for high-throughput distributed inference using SGFP4 adaptive quantization with 64x64 macroblocks. Expert Language Models (ELMs) are specialized language-model-based experts optimized for specific reasoning roles, subject domains, or operational functions. An ELM may be implemented as a compact standalone SLM, distilled expert model, adapter-augmented expert, constrained service, or secure expert reasoning module.

Role-based ELMs: Planner, Primary Draft, Verifier, Arbiter, Refiner/Formatter, Grounding, Tool-Support.

Domain-specific experts: Math, Code, Scientific, Legal/Compliance, Operations/Workflow, Customer Support, Finance.

The Router Layer relies on rule-based detection initially (numeric density, code syntax, grounding-sensitive, formatting-sensitive, low/high complexity), with planned evolution toward a learned routing model and cognitive planner.

### Reputation-Based Consensus

**Source:** `docs/architecture/reputation-consensus.md`

Each node maintains role-aware and domain-aware reputation signals stored via wallet-core, RocksDB, and CRDT replicated state. Reputation updates use accuracy/quality, latency, consistency, and safety components with scores clipped to [0,1]. The consensus engine operates at the application layer independent of blockchain consensus. Design principles: fully peer-to-peer, requestor node as orchestrator, reputation-weighted agreement, liveness over perfection, deterministic finalization, arbitration over flat voting.

The swarm execution flow: client submits to GNUS node -> requestor-orchestrator selects candidate nodes -> broadcasts via libp2p -> nodes execute locally -> optional verifier/grounding/arbiter -> weighted consensus or arbiter-mediated synthesis -> final response.

Byzantine tolerance addressed via reputation decay, consistency penalties, latency penalties, verifier/grounding checks, and minimum history requirements.

### Grounding and Retrieval

**Source:** `docs/architecture/grounding.md`

Grokipedia acts as the primary public grounding layer. The retrieval pipeline: query analysis, search Grokipedia index, inject top-k structured facts, tag for traceability, pass to generation/verification/arbitration stages. Supports public, private tenant, and hybrid grounding modes. Grounding can be deployed as a dedicated Grounding ELM or grounding service. Retrieval, structured memory, and private ELM adaptation are complementary mechanisms.

### GAML -- GNUS Agentic Memory Layer

**Source:** `docs/architecture/agentic-memory-layer.md`

GAML introduces structured, reasoning-oriented long-term memory into GeniusCognitiveSystem. It treats retrieval as a governed cognitive process involving bridge blocks, facts, policies, events, trust metadata, privacy boundaries, execution-integrity evidence, and orchestration-aware selection.

Memory classes: semantic, episodic, procedural, working, policy/preference.

The Cognitive Asset model treats long-term memory, bridge blocks, reasoning traces, tool results, plans, execution claims, verification outputs, consensus records, and distillation samples as related forms of a common abstraction. Key Cognitive Asset types: Fact, Goal, Constraint, Policy, Bridge Block, Procedure, Tool Result, Capability, Connector, Plan, Execution Claim, Checkpoint Calibration, Verification Result, Execution Verdict, Arbitration Result, Consensus Record, Benchmark Result, Distillation Sample, Specialist Trace.

Privacy scopes: local-only, user-private, trusted-devices, enterprise-private, tenant-private, shared, public. GAML supports swarm memory consensus, CRDT replication, encryption, derived-asset inheritance, and surprise-gated writes.

### Execution Modes and Performance

**Source:** `docs/architecture/execution-and-performance.md`

Four execution modes: Single Node (Semantic Core only, fast), ELM-Assisted (Core + role/domain experts), Swarm Mode (multiple nodes, weighted consensus/arbitration), Agent Mode (multi-step with memory, grounding, verification, tools). Execution strategy: local-first distributed-second, smallest effective cognitive sets, roles over raw scale.

### Roadmap and Risks

**Source:** `docs/architecture/roadmap-and-risks.md`

Four-phase roadmap: Phase 1 (Semantic Core foundations + FP4 quantization), Phase 2 (Experts + Router/Planner + Memory Governor), Phase 3 (Reputation, Memory, Consensus with CRDT sync), Phase 4 (Grounding, Private Customization, Secure Agent Path, Benchmarks). Seven identified risks with mitigations: FP4 underperformance, reputation gaming, swarm latency, routing instability, memory contamination, unsafe tool execution, customization path confusion.

### Future Compatibility and Strategic Positioning

**Source:** `docs/architecture/future-and-positioning.md`

Architecture designed to allow Semantic Core replacement with Latent World Model Core while keeping ELMs, reputation system, grounding layer, structured memory, swarm coordination, secure tool intermediation, hardware-efficient deployment, and private customization layers intact.

### AI Safety

**Source:** `docs/architecture/ai-safety.md`

Decentralized multi-layer safety: Layer 1 (Node-level enforcement, authoritative), Layer 2 (Reputation-based enforcement), Layer 3 (Client-side preference filtering), Layer 4 (Tool Intermediary enforcement). Safety profiles are versioned, cryptographically signed, distributed via IPFS, immutable once adopted. No centralized safety gateway. No GeoIP enforcement. Node operators bear enforcement responsibility.

### Distributed Swarm Thinking Context

**Source:** `docs/architecture/distributed-swarm-thinking-context.md`

Formalizes a swarm-native thinking context model with five cooperating layers: Context/Memory, Routing/Planning, Primary/Secondary Expert Execution, Verification/Grounding/Synthesis, User-Visible Thinking Context. Structured collaborative reasoning over monolithic reasoning. Memory-guided context instead of brute-force long context. Inspectable swarm thinking traces. Reputation-aware specialization. Quantization-aware modularity.

Specialist taxonomy includes role specialists (Planner/Memory Governor, Primary Draft, Verifier, Synthesizer/Arbiter, Refiner/Formatter, Grounding) and domain specialists (Numeric, Symbolic Math, Tool/Execution, Code, Domain Grounding/Workflow).

Recommended evolution: near-term (6 specialists), medium-term (11 specialists). Open questions: shared backbone vs separate models, adapter composition vs standalone specialists, quantization policy per role, reputation/quantization interaction.

### Context Lifecycle, Caching, and Governance

**Source:** `docs/architecture/context-lifecycle-caching-governance.md`

Normative context-efficiency contract. Core rule: every token entering an ELM must be authorized, relevant, budgeted, explainable, cacheable where possible, and removable when it stops being useful. Defines the Context Compiler (deterministic boundary between authorized source material and model-visible context), Prefix Cache Manager, and Context Compaction Manager.

Key principles: smallest sufficient context, off-window by default for large material, stable before volatile, deterministic construction, specialists as lossy filters, compaction at semantic boundaries, hierarchical budgets, explainability without raw hidden reasoning, privacy never widens through optimization.

Defines canonical context packet model with stable prefix and volatile suffix. Cache identity includes tenant scope, privacy scope, authorization scope, model/tokenizer/quantization/runtime/adapter hashes, system/tenant policy hashes, capability contract set hash, stable context hash, canonicalization version. 15 acceptance criteria specified.

### Secure Agent Architecture

**Source:** `docs/architecture/secure-agent-architecture.md`

Product Technical Design Specification. Treats agent execution as one operating mode of the GNUS cognitive system. Layer model: Client/API, Orchestration, Expert Execution, Consensus/Grounding, Tool Intermediary, Memory, Distributed Infrastructure, Trust/Economics, Privacy.

The Tool Intermediary Service is the mandatory security choke-point: receives tool_calls[], performs deterministic dry-run, sanitizes outputs, enforces zero-trust capabilities, emits signed attestation, supports optional human approval gating. All tool executions require valid intermediary attestation. Zero direct side-effect executions from Semantic Core or ELM workers.

Memory model includes higher-trust vs lower-trust separation. Node trust tiers: Tier A (Settlement/Reputation, Tool Intermediary, higher-trust memory), Tier B (Router/Planner, Verifier/Arbiter, Grounding), Tier C (Semantic Core, ELM workers), Tier D (Opportunistic public compute).

### EGGROLL Swarm Retraining

**Source:** `docs/architecture/eggroll-swarm-retraining.md`

Swarm-native distributed retraining using deterministic perturbation reconstruction and compact fitness aggregation. Architecture positions: Semantic Core/Experts = inference, Router/Swarm Thinking = execution, GAML = memory, Reputation/Consensus = trust, EGGROLL = specialist refresh and adaptation.

Core primitive: base model reference + target adapter reference + deterministic perturbation seed + task shard + reward function -> compact fitness packet. Maps onto GNUS Processing Rooms. Beehives are locality-aware sub-swarms. Deterministic perturbation reconstruction from seeds. Fitness packets are small, signed, and auditable. Hierarchical aggregation: worker -> room host -> beehive aggregator -> broader promotion.

Recommended initial retraining targets: Numeric Specialist/Math Verifier, Router/Planner, Formatter/Schema Specialist, Grounding Specialist, Code Specialist. Five-phase rollout from single-machine proof to hierarchical swarm aggregation.

### Targeted Retraining and HCTS

**Source:** `docs/architecture/cognitive-retaining-system.md`

Targeted Retraining: continuous, fine-grained adaptation through lightweight updates to adapters, routing weights, critic weights, verification behavior, memory, and arbitration behavior without full base-model replacement. EGGROLL-based optimization for non-differentiable conditions.

Hierarchical Critical Thinking Specialists (HCTS): multi-layer structured critique from generic human critic through country/cultural, regional/social, professional/domain, organizational/team, individual cognitive, to contrarian/adversarial critic. Bias-Aware Reasoning explicitly models and tags bias contexts. Cognitive Resistance Layer with four modes: Mirror, Nudge, Challenge, Adversarial.

### Epistemic Arbitration and Cognitive OS

**Source:** `docs/architecture/epistemic-arbitration-and-cognitive-os.md`

Requestor Node evolves into a formal Epistemic Arbitration Layer using data-driven GQHSM state machines. Key distinction: consensus determines which outputs are viable; epistemic arbitration determines how viable outputs should be judged, challenged, and synthesized.

Supports multiple epistemic framework families: Sanskrit epistemology (Samshaya, Pramana, Pancha Avayava, Tarka, Hetvabhasa, Nirnaya mapped to GCS components), Kripke/modal reasoning (world construction, accessibility checks, fixed-point resolution), and hybrid frameworks. GQHSM is the data-driven hierarchical state machine runtime. JSON-defined arbitration machines with generic callback model. Plugin architecture via small shared libraries with future WASM extension path.

Six cooperating layers: Context/Memory, Routing/Planning, Primary/Secondary Expert Execution, Consensus/Trust, Epistemic Arbitration, Final Synthesis/Thinking Context.

### SGFP4 Adaptive Quantization Format

**Source:** `docs/architecture/sgfp4-format.md`

Weight compression format with 64x64 macroblocks, fixed 2048-byte payload per block, per-block FP16 scale+bias header, adaptive dual-mode per block (FP4_AFFINE or T158_AFFINE), flags-in-offsets for zero-cost metadata, GPU-decoded in shared memory at inference time. Container layout: headers array (uint32), offsets array (uint32 with low 4 bits as flags), codes_blob (bytes), shape metadata.

### Objective Memory and VTG

**Source:** `docs/architecture/objective-memory-vtg.md`

Objective Memory: verified cognitive execution substrate recording reusable low-entropy transitions. Distinct from GAML (structured long-term memory). VTG stores verified transition patterns between cognitive states. Candidate frontier proposes possible continuations; expert execution evaluates; verification/arbitration decides what to use. Transforms repeated successful inference patterns into durable swarm intelligence.

### Speculative Decoding and VTG Candidate Scheduling

**Source:** `docs/architecture/speculative-decoding-and-vtg.md`

Micro-speculation: node proposes short local continuation, verifies through local path, commits only accepted prefix, publishes compact outcome metadata to VTG and EGGROLL. Operating envelope: 100-350MB for active model/ELM, 5-50MB for micro drafter/MTP head, 10-100MB for hot VTG shard. Drafter variants: Frozen Micro-MTP, VTG Lookup Drafter, Rule/Schema Drafter, Tiny Causal Tree Drafting, Micro-Diffusion Block Drafter.

### Frozen Micro-MTP and VTG Edge Inference

**Source:** `docs/architecture/frozen-mtp-and-vtg.md`

Frozen Micro-MTP attaches small multi-token prediction head to frozen Semantic Core or ELM backbone. Head proposes 1-4 token continuation; local verifier accepts usable prefix. Confidence Scheduler and Router decide whether MTP candidate is used. Coupled with VTG hot shard and EGGROLL learning loop.

### OpenAI-Compatible API Router and GCS Job Queue

**Source:** `docs/architecture/openai-compatible-api-router-and-gcs-job-queue.md`

Translates OpenAI-compatible API calls into signed GNUS.ai democratized queue jobs. Preserves p2p architecture while providing familiar developer API surface. Introduces higher-level GCS API Request Job type distinct from low-level processing chunk jobs. Covers HTTP request/response lifecycle, streaming SSE, client disconnect handling, API key/tenant authorization, request-level policy, OpenAI-compatible error format, usage accounting, node capability matching.

### Local Cognitive Second Brain Mode

**Source:** `docs/architecture/local-cognitive-second-brain.md`

Private, user-owned memory and reasoning system running on local devices using five GCS components: Orchestration Layer (control plane), GAML (structured memory substrate), Local/Private ELMs (reasoning engine), Second Brain Agent (behavior layer), EGGROLL (adaptation loop). Supports personal, SMB, and enterprise workflows with configurable privacy boundaries.

### Forecast-Driven Cognition

**Source:** `docs/architecture/forecast-driven-cognition.md`

Allows GCS to anticipate future cognitive requirements during unfolding interactions and prepare resources in advance. Includes Anticipatory Cognition Engine, Cognitive Execution Scheduler, Personal Forecast Models, bidirectional voice communication support, and Anticipatory Distillation.

### Execution Integrity System (EIS)

**Source:** `docs/architecture/execution-integrity-system.md`

Verifies distributed computation was executed faithfully according to declared execution contract. Concerned with execution honesty, not semantic answer quality. Owns: kernel registration, execution-contract validation, checkpoint-band calibration, teacher-forced spot-check verification, execution claims/fraud verdicts, reputation evidence for execution honesty. Addresses lazy-node and model-substitution attacks.

### GCS Capability System

**Source:** `docs/architecture/capability-system.md`

Protocol-neutral system for discovering, governing, routing, and executing external and local operations through canonical capability contracts. Components: Capability Contract, MCP Connector Adapter, Tool Intermediary, GAML, Connector, Capability Provider, Credential Handling, Capability Routing, Reputation System.

### Agent and Module Development Inventory

**Source:** `docs/architecture/agent-module-development-inventory.md`

Consolidates agents, deterministic services, runtime modules, data stores, adapters, UIs, security boundaries, and distributed infrastructure required across GCS architecture into an implementation inventory with deployment profiles and workstreams.

### Index Documents

**Source:** `docs/architecture/SUMMARY.md`

Master index and reading guide spanning executive summary through agent and module development inventory.

**Source:** `docs/architecture/SUMMARY_EXT.md`

Extended table of contents and cross-reference index for the GNUS.ai architecture documentation.

---

## Relationship to Existing Context

These architecture docs describe the Genius Cognitive System, which is distinct from the `doc-template` workstream (a documentation template tool). The `cross-submodule-capabilities.md` file was generated from a previous ingest of these same architecture docs and captures cross-submodule routing for capabilities like the Capability System, OpenAI-Compatible API Router, Speculative Decoding/VTG, Objective Memory/VTG, Local Cognitive Second Brain, and EGGROLL Swarm Retraining. This ingest confirms and extends those findings.

No locked decisions from existing CONTEXT.md files conflict with this ingest.
