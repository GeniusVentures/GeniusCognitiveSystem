# GCS Architecture Context (DOC intel)

**Generated:** 2026-09-21 | **Mode:** merge (app workstream) | **Run:** 30 docs from `docs/architecture/`
**11 DOC-type documents.** Descriptive/explanatory content keyed by topic, with source attribution. DOC is lowest precedence — where a refreshed SPEC (see `constraints.md`) or ADR (see `decisions.md`) diverges, those win; superseded framings are retained below only as background.

---

## System Identity, Objectives, and Component Roles

**Source:** `docs/architecture/executive-summary.md` (refreshed 2026-09-21)

GeniusCognitiveSystem is a "Specialized Adaptable Intelligence Fabric" organized as a cognitive operating system, not a prompt-to-model pipeline. Ten primary goals (distributed inference, quantized Semantic Core, modular Expert Model execution, reputation-weighted consensus, Grokipedia + private grounding, measurable improvement over single-model baseline, structured memory + inspectable reasoning, secure agentic workflows via mandatory tool intermediation, private customization, execution-integrity verification). Four subsystem families: Semantic Core + Expert Models (ELM generation contracts, EJM bounded judgment contracts, other processor capabilities), GAML, EIS, Consensus/Verification/Synthesis. Executive Controller is the top-level coordination function — initially deterministic routing/rules, later possibly a Planner ELM or learned routing; selects execution mode, budgets, expert set, verification path, and produces an execution graph.

## Component Mapping and Layered Stack

**Source:** `docs/architecture/system-overview.md` (refreshed 2026-09-17)

Request flow: Client API → Executive Controller/Router/Planner → Memory Governor + grounding selection → execution contract + EIS policy → execution nodes → verification/arbitration/synthesis → reputation-weighted consensus → grounding validation → final response → Qualified Cognitive Events → GAML/VTG/routing/arbitration/EGGROLL adaptation. Compute Layer: MNN, Vulkan/MoltenVK, SGFP4 codec, CUDA/Vulkan shaders, EIS. Distributed Layer: libp2p, IPFS-lite, RocksDB, CRDTs, gRPC. Security Layer: libsecp256k1 (node identity), ed25519 (message signing), OpenSSL (transport), wallet-core (trust state), Tool Intermediary boundary. Eight-layer cognitive stack (client/API, orchestration, expert execution, execution integrity, consensus/grounding, security/tool intermediary, memory/cognitive asset, distributed infrastructure). RuntimeCoordinator owns the request lifecycle; GQHSM-compatible state machines may coordinate expert-stage lifecycle (capability-first resolution, then processor registry); EIS verification is sampled/asynchronous, not blocking.

## Expert Model Taxonomy and Router Design

**Source:** `docs/architecture/model-and-router.md` (refreshed 2026-09-21) — NOTE: cross-ref cycle member, see INGEST-CONFLICTS.md WARNING

Expert Model (EM) is the neutral umbrella; four orthogonal dimensions: cognitive role, public contract/capability, processor architecture, execution backend. ELM = language generation/transformation contract; EJM = bounded typed judgment (boolean probability, choice distribution, classification, score, ranking, parallel judgment bundles); EDM = shorthand for diffusion-backed implementation (diffusion is a processor architecture, not a contract — an EDM may expose the EJM contract via JUDGE structured reads). EJM output is advisory; deterministic services remain authoritative for authorization/policy/side effects. Bounded judgment is a system-wide control primitive (Executive Controller, Router, Memory Governor, CES, Verifier, Arbiter, tool path, learning pipeline). Token-label judgment adapters must validate candidate labels at the actual answer boundary (evaluate all allowed candidate token ids, not global top-k). Legacy Grammar/Math specialists and existing `IELM` code remain compatibility paths during migration to the neutral EM abstraction. Router: rule-based MVP (numeric density → math specialist; code syntax → code path; grounding-sensitive → grounding; formatting-sensitive → refiner/formatter; low complexity → Semantic Core only; high complexity → multi-stage/swarm), evolving to lightweight EJM router, then cognitive planner.

## Grounding and Retrieval

**Source:** `docs/architecture/grounding.md` (last touched 2026-06-28 — pre-refresh generation)

Grokipedia as primary public grounding substrate; retrieval pipeline (query analysis → search approved sources → inject top-k tagged facts → pass evidence to generation/verification/arbitration); post-generation validation with contradiction handling; private knowledge grounding (internal docs, SOPs, wikis, catalogs, ticket histories); public/private/hybrid modes; grounding as expert role or on-demand service; retrieval vs memory vs private adaptation as complementary levers; GAML v1 extends grounding memory. Framed in legacy "Grounding ELM" terms — refreshed taxonomy uses Grounding Expert combining bounded evidence judgments with generated explanation.

## Execution Modes and Performance Targets

**Source:** `docs/architecture/execution-and-performance.md` (last touched 2026-06-28 — pre-refresh generation)

Four execution modes: Single Node (Semantic Core only), ELM-Assisted (core + experts — legacy naming; refreshed docs say "expert-assisted"), Swarm (multi-node, weighted consensus/arbiter synthesis, reputation-based selection), Agent (multi-step with memory, grounding, verification, tools via Tool Intermediary). Principles: local-first distributed-second, smallest effective cognitive set, roles over raw scale. Qualitative targets only: tokens/sec ≥ INT4 baseline where comparable, memory within low-bit envelope, grounded quality ≥ single-model baseline, near-linear multi-node scaling, bounded separately-reported tool safety overhead.

## Future Compatibility and Strategic Positioning

**Source:** `docs/architecture/future-and-positioning.md` (last touched 2026-06-28 — pre-refresh generation)

Semantic Core replaceable by Latent World Model Core or successor substrate while keeping expert roles, reputation, grounding, structured memory, swarm coordination, tool intermediation, hardware-efficient deployment, private customization. Positioning: distributed, reputation-weighted, knowledge-grounded, memory-aware, hardware-efficient, modular, future-ready, agent-capable under secure execution, enterprise/SMB-customizable; aligned with Superhuman Adaptable Intelligence (SAI).

## Distributed Swarm Thinking Context

**Source:** `docs/architecture/distributed-swarm-thinking-context.md` (refreshed 2026-09-21)

Swarm-native thinking context model across five layers: context/memory, routing/planning, primary/secondary expert execution, verification/grounding/synthesis, user-visible thinking context. Bridge blocks + fact-based context construction; specialist taxonomy (Planner/Memory Governor, Primary Draft, Verifier, Synthesizer/Arbiter, Refiner/Formatter, Grounding; domain: Numeric, Symbolic Math, Tool/Execution, Code); thinking trace schema (16.12) and trace-to-EJM-shard pipeline (16.15.3) framed as suggested/descriptive; explicitly unresolved choices: shared backbone vs separate models, adapter vs standalone specialists, per-role quantization policy. Section 16.14 references legacy quant branding (FP4 Ultra, Turbo Quant, Sparse-V) — superseded by SGFP4 SPEC naming.

## Targeted Retraining and HCTS

**Source:** `docs/architecture/cognitive-retaining-system.md` (refreshed 2026-09-21)

Targeted Retraining: continuous fine-grained adaptation of user/role-specific cognitive behavior via adapters, EJM decision heads/readouts and calibration artifacts, routing weights, critic/verifier weights, memory structures, arbitration logic — without full base-model retraining. Hierarchical Critical Thinking Specialists (HCTS): layered critique from generic human critic through cultural, regional/social, professional/domain, organizational/team, individual cognitive, to contrarian/adversarial critics. Bias-Aware Reasoning tags bias contexts; Cognitive Resistance Layer (Mirror/Nudge/Challenge/Adversarial); Cognitive Twin integration; continuous learning loop; compatible with EGGROLL-style ES optimization. Cross-ref: `./cognitive-evolution-control.md` (SPEC CON-05).

## Local Cognitive Second Brain Mode

**Source:** `docs/architecture/local-cognitive-second-brain.md` (refreshed 2026-09-21)

Private GAML-backed memory and reasoning mode: Orchestration Layer (control plane), GAML (memory substrate), local/private Expert Models including ELM generation and EJM judgment (capability engines), Second Brain Agent (behavior layer), EGGROLL (adaptation loop), human-readable memory mirror (inspectability). Local data source ingestion, entity resolution, memory lifecycle, context packet assembly, privacy modes. Implementation requirements captured as REQ-second-brain.

## Execution Roadmap and Risk Analysis (legacy framing)

**Source:** `docs/architecture/roadmap-and-risks.md` (last touched 2026-06-28 — LIKELY SUPERSEDED, see INGEST-CONFLICTS.md WARNING)

Four-phase system roadmap: Phase 1 Semantic Core foundations + FP4 v3 quantization (`genius-core-alpha`); Phase 2 experts + router/planner + memory governor (`genius-modular-alpha`); Phase 3 reputation/memory/consensus + CRDT sync (`genius-swarm-beta`); Phase 4 grounding/private customization/secure agent/benchmarks (`GeniusCognitiveSystem v1 Beta`). Seven risks with mitigations: FP4 underperformance (fallback INT4), reputation gaming (minimum history, verifier-aware scoring), swarm latency (limit width), routing instability (rule-based v1), memory contamination (provenance-aware write gates), unsafe tool execution (intermediary attestation), customization path confusion (separate governed levers). Uses legacy "FP4 v3" naming and predates per-topic rollout plans in refreshed SPECs.

## Sloth/Unsloth Integration (exploratory)

**Source:** `docs/architecture/exploratory/sloth-integration.md` (last touched 2026-06-13 — exploratory, NOT a decision)

Exploratory recommendation to integrate Unsloth for the Python training stack: LoRA-based Teacher→Parent→Specialist distillation, FP4/FP8 quantized training via `FastLanguageModel`, adapter export for specialists, EGGROLL swarm retraining compatibility (per-node LoRA deltas merged via CRDT consensus), DeepSpeed/FSDP compatibility. Informal "Verdict: Integrate Unsloth" — no ADR structure, lives under `exploratory/`, Python/HF-centric (bitsandbytes patching for GFP4 macroblock codec, Sparse-V kernel wrapping). Predates the Expert Model refresh; kept as background only. No ADR assigns training-stack ownership, so nothing in the ingest set contradicts it.

---

## Exclusions honored (per orchestrator instruction)

GNUS-NEO-SWARM/docs, docs/architecture/GNUSNeoSwarm/, and docs/architecture/PythonOrchestratedCompression/ (Doxygen dumps) were excluded from this ingest by user instruction. No cross_refs in the 30 classifications point into those paths (epistemic's external refs point to GitHub/GQHSM headers). README.md, SUMMARY.md, SUMMARY_EXT.md were not in the 30-doc classification set. Absence of these is not a gap.
