## Conflict Detection Report

Run: 2026-09-21 | Mode: merge (app workstream) | Set: 30 docs from docs/architecture/ (2 ADR locked, 17 SPEC, 11 DOC, 0 PRD, 0 UNKNOWN)
This report replaces the 2026-07-19 report (which manifest-typed all 29 docs as DOC and extracted no typed intel).

### BLOCKERS (0)

None.

- LOCKED-vs-LOCKED check: docs/architecture/decisions/adr-embeddable-sdk-and-modular-build.md and docs/architecture/decisions/adr-runtime-component-ownership.md are complementary (SDK/embedding boundary vs runtime ownership hierarchy); both place GNUS-NEO-SWARM behind the GCS RuntimeCoordinator contract. No contradiction on any shared scope.
- LOCKED-vs-existing check: no decision in .planning/workstreams/app/PROJECT.md (Key Decisions all "Pending"), STATE.md (phase implementation notes), ROADMAP.md, REQUIREMENTS.md, or MILESTONES.md is marked locked; neither ADR contradicts them.
- SPEC-vs-ADR check: no SPEC contradicts either LOCKED ADR (SGFP4-as-weight-format, MNN-only native SGFP4 execution, and per-module CMake ownership are consistent across sgfp4-format.md, system-overview.md, and cognitive-evolution-control.md).
- No UNKNOWN-type or low-confidence classifications in this set.

### WARNINGS (3)

[WARNING] Cross-reference cycles in the ingest graph (process default is BLOCKER; downgraded — see note)
  Found: Two cyclic components among cross_refs. Component A (3 docs): docs/architecture/model-and-router.md -> ./sgfp4-format.md; docs/architecture/sgfp4-format.md -> ./model-and-router.md AND ./system-overview.md; docs/architecture/system-overview.md -> ./sgfp4-format.md. Component B (2 docs): docs/architecture/frozen-mtp-and-vtg.md -> ./speculative-decoding-and-vtg.md; docs/architecture/speculative-decoding-and-vtg.md -> ./frozen-mtp-and-vtg.md.
  Impact: The synthesizer process maps cycles to BLOCKER because reference-chasing synthesis loops. Downgraded to WARNING because: (1) these edges are mutual "see also" citations between peer documents, not content-inclusion edges; (2) this run's extraction is per-document and non-recursive, so no loop is possible; (3) the prior ingest of this same corpus (2026-07-19 INGEST-CONFLICTS.md, now replaced) adjudicated these exact edges as "intentional documentation structure, not a decision dependency cycle"; (4) quarantining the 5 docs would exclude model-and-router.md — refreshed 2026-09-21 and central to the requested gap analysis. All 5 docs were synthesized and are flagged in intel.
  → Acknowledge that citation cycles are informational and proceed, or trim cross_refs via --manifest and re-run to make the graph acyclic.

[WARNING] Competing roadmap framings: legacy 4-phase system roadmap vs per-topic SPEC rollout plans vs app workstream roadmap
  Found: docs/architecture/roadmap-and-risks.md (last touched 2026-06-28) defines a 4-phase system roadmap (genius-core-alpha -> genius-modular-alpha -> genius-swarm-beta -> GeniusCognitiveSystem v1 Beta) using legacy "FP4 v3" language. Refreshed SPECs each carry their own phased rollout plans that do not reference it: context-lifecycle-caching-governance.md 17.25 (6 phases), openai-compatible-api-router-and-gcs-job-queue.md 26.24 (8 phases), objective-memory-vtg.md 23.21 (6), speculative-decoding-and-vtg.md 24.15 (6), frozen-mtp-and-vtg.md 25.13 (5), eggroll-swarm-retraining.md 19.18 (5), forecast-driven-cognition.md 28.21 (5). Existing: .planning/workstreams/app/ROADMAP.md (7 app phases, v1.0 GCS Chat, Phases 1-2 complete).
  Impact: The roadmapper cannot pick a roadmap baseline without losing intent — treating roadmap-and-risks.md as live would silently fold a superseded, partially-renamed plan into roadmap updates; treating only the app roadmap as live would ignore the system-level phased plans the refreshed SPECs introduce.
  → Decide the framing before routing: (a) mark roadmap-and-risks.md superseded by per-topic rollout plans and keep the app workstream roadmap as the only execution roadmap, or (b) derive a new system-level roadmap from the per-topic plans as a separate workstream. Do not merge the 4-phase plan into either.

[WARNING] Existing GCSB-04 requirement predates the 2026-09-21 Expert Model generalization
  Found: .planning/workstreams/app/REQUIREMENTS.md GCSB-04 requires "Bot response routes through ELM specialist selection" (defined 2026-08-15, unchanged). docs/architecture/model-and-router.md (refreshed 2026-09-21) specifies routing as capability-first Expert Model selection across ELM/EJM/EDM contracts with a rule-based MVP router (section 6.2) and names existing IELM code a "compatibility path during migration to the neutral EM abstraction" (section 5.2.10). ROADMAP.md Phase 6 success criterion 4 repeats the ELM-only wording.
  Impact: Phase 6 (GCS Bot) acceptance may be validated against superseded terminology; the refreshed architecture routes through Expert Model contracts of which ELM is only the language-generation subset.
  → Update GCSB-04 and Phase 6 criterion 4 to Expert Model routing (or explicitly pin Phase 6 to the ELM contract subset for v1.0) before the roadmapper touches Phase 6.

### INFO (6)

[INFO] Auto-resolved: refreshed Expert Model taxonomy supersedes legacy ELM-only framing
  Note: Older-generation docs use ELM-centric language that the 2026-09-21 refresh generalized to Expert Model (EM) with ELM/EJM/EDM contracts: docs/architecture/execution-and-performance.md ("ELM-Assisted Mode", "smallest effective set of ELMs"), docs/architecture/grounding.md ("Grounding ELM"), docs/architecture/ai-safety.md ("Genius ELM" as the inference core), docs/architecture/reputation-consensus.md (role scores like Planner_score/Math_score). Sources: docs/architecture/model-and-router.md sections 5.2.1-5.2.10, docs/architecture/system-overview.md section 4.3.1, docs/architecture/executive-summary.md section 2.3. The refreshed framing wins in synthesized intel (constraints.md/context.md carry legacy flags); legacy docs are retained as background only. Same treatment for quantization naming: "FP4 v3" (roadmap-and-risks.md), "FP4 Ultra / Turbo Quant / Sparse-V" (distributed-swarm-thinking-context.md 16.14, exploratory/sloth-integration.md) are superseded by the SGFP4 adaptive format spec (docs/architecture/sgfp4-format.md).

[INFO] Auto-resolved: LOCKED ADR-01 vs completed Phase 1 work retaining neoswarm_ffi and a partial repo layout
  Note: docs/architecture/decisions/adr-embeddable-sdk-and-modular-build.md (LOCKED) requires Flutter packages to be backend-neutral with no direct NeoSwarm dependency, and lays out include/gcs/, src/{common,runtime,chat,api,ffi}/, and packages/{gcs_client,gcs_chat,gcs_native}. Current reality: .planning/workstreams/app/ROADMAP.md plan 01-07 kept the neoswarm_ffi dependency in pubspec; the repo has src/lib/gcs_core.cpp, src/lib/gcs_storage/, src/ffi/, src/proto/, src/app/ but no packages/, include/gcs/, or per-module src split yet. ADR wins (it is forward-looking and explicitly says existing NeoSwarm bridges "should be evaluated for migration"); recorded as a migration/structure gap for the roadmapper, not a contradiction.

[INFO] Scope boundary: app "no central servers" constraint vs Cloudflare Edge ingress in the OpenAI-compatible API router SPEC
  Note: .planning/workstreams/app/PROJECT.md constrains the chat app to "No central servers: All state sync via CRDT over IPFS pubsub". docs/architecture/openai-compatible-api-router-and-gcs-job-queue.md introduces a managed Cloudflare Edge ingress, a GCS Gateway Node, and metering/settlement for the public developer API product. Not a contradiction: the app constraint governs chat state sync, while the API router is a separate system-level product surface (and LOCKED ADR-01 explicitly lists OpenAI-compatible HTTP/SSE as a permitted Flutter backend). Recorded so the roadmapper does not misread either document.

[INFO] Draft-status and open-question content flagged, not treated as decided
  Note: docs/architecture/execution-integrity-system.md self-declares "Status: Draft for review" with 7 open items (section 29.9) — captured in constraints.md CON-09 as a draft-spec constraint, not a locked decision. docs/architecture/openai-compatible-api-router-and-gcs-job-queue.md section 26.26 lists 9 unresolved questions (host repo, protobuf-vs-JSON-first, lease semantics, alias policy, others). docs/architecture/exploratory/sloth-integration.md is an exploratory "Integrate Unsloth" recommendation with no ADR structure. None of these should be routed as commitments.

[INFO] Exclusions honored; no dangling refs into excluded material
  Note: GNUS-NEO-SWARM/docs, docs/architecture/GNUSNeoSwarm/, and docs/architecture/PythonOrchestratedCompression/ were excluded by user instruction as stale/superseded. No cross_refs among the 30 classifications point into those paths. Non-doc cross_refs observed: epistemic-arbitration-and-cognitive-os.md points to github.com/Super-Genius/GQHSM and headers (GQHSM.h, EpistemicContext.h); execution-integrity-system.md names target subsystems by title rather than path (no edges created). README.md/SUMMARY.md/SUMMARY_EXT.md were outside the 30-doc set by design.

[INFO] Prior synthesis artifacts superseded
  Note: The 2026-07-19 synthesis (all-DOC manifest typing; empty decisions/requirements/constraints intel; doc-template workstream comparison) is superseded by this run's typed extraction against the app workstream. .planning/intel/cross-submodule-capabilities.md (generated 2026-07-18) was left in place; its capability-spanning findings remain consistent with this run and are not contradicted by it.
