# Decisions (ADR intel)

**Generated:** 2026-09-21 | **Mode:** merge (app workstream) | **Run:** 30 docs from `docs/architecture/`
**Precedence:** ADR > SPEC > PRD > DOC (no per-doc overrides in this set)

---

## D-ADR-01 — Embeddable GCS SDK and Modular Native Build

- **Source:** `docs/architecture/decisions/adr-embeddable-sdk-and-modular-build.md`
- **Status:** LOCKED (accepted, 2026-07-28; classifier `locked: true`, high confidence)
- **Scope:** GeniusCognitiveSystem parent repo, native SDK boundary, Flutter integration, CMake structure

**Decision statement (7 parts):**

1. GCS parent repo becomes the stable SDK and embedding boundary. It owns: public runtime request/event/status/result/artifact contracts; the abstract `RuntimeCoordinator` interface; the public `GeniusCognitiveSystem` facade; chat session and streaming client contracts; a stable C ABI for Flutter/FFI consumers; platform-neutral Dart client contracts; reusable Flutter chat UI components.
2. Runtime implementation is injected behind the public interface — `NeoSwarmRuntimeCoordinator` (GNUS-NEO-SWARM) implements `RuntimeCoordinator`. Parent API must not expose NeoSwarm internals, MNN objects, SGProcessingManager objects, or SuperGenius clients.
3. Flutter consumes backend-neutral contracts (submit, stream, cancel, status, approvals/artifacts/usage/provenance/errors). Backends: native FFI into the GCS C ABI, OpenAI-compatible HTTP/SSE, gRPC, or mocks.
4. Native modules use per-directory CMake ownership — root delegates via `add_subdirectory()`.
5. Initial native targets: `gcs_common`, `gcs_runtime`, `gcs_chat_core`, `gcs_api`, `gcs_ffi`, `neoswarm_runtime`; each declares only direct dependencies.
6. Heavy dependencies (MNN, Vulkan/MoltenVK, SGProcessingManager, SuperGenius, libp2p, RocksDB) do not leak through the SDK; PImpl/abstract interfaces preserve ABI.
7. Flutter binds via stable C ABI (opaque handles, structured payloads); wire format may start as versioned canonical JSON, schema-based binary later.

**Key rejected alternatives:** direct Flutter→NeoSwarm linking; one monolithic native target; root CMake listing all sources; exposing C++ classes through Dart FFI.

**Consequences relevant to the app workstream:** existing NeoSwarm Flutter bridges/UI should migrate into `gcs_native`/`gcs_client`/`gcs_chat` rather than be duplicated; applications embed the same chat UI across native/HTTP/gRPC/mock backends.

---

## D-ADR-02 — Runtime Component Ownership Boundaries

- **Source:** `docs/architecture/decisions/adr-runtime-component-ownership.md`
- **Status:** LOCKED (accepted, 2026-07-28; classifier `locked: true`, high confidence)
- **Scope:** GCS, GNUS-NEO-SWARM, SGProcessingManager, SuperGenius, MNN, SGFP4

**Decision statement (6 parts):**

1. **GeniusCognitiveSystem** defines cognitive architecture and public contracts (request/response, routing/planning semantics, execution-plan semantics, context/memory/grounding/policy semantics, expert/ELM invocation, verification/arbitration/synthesis, streaming/cancellation/status/artifacts/errors, embedding contracts). It does not own device-specific kernels or every concrete runtime.
2. **GNUS-NEO-SWARM** provides the primary GCS runtime implementation — the concrete RuntimeCoordinator owning the full request lifecycle. It composes lower-level services; it is not the model runtime or distributed worker scheduler.
3. **SGProcessingManager** is a partial processing-workload executor (declared passes, typed resource bindings, Vulkan compute, media/tensor transforms). It does NOT own the GCS request lifecycle, routing/planning, session semantics, expert selection, memory/grounding strategy, verification/synthesis, or tokenizer/generation/KV-cache. A GCS request may invoke zero or many SGProcessingManager jobs.
4. **MNN** owns model-runtime semantics: model/tokenizer loading, chat templates, autoregressive generation, KV-cache, sampling, speculative decoding where supported, CPU/Vulkan execution, native SGFP4 weight loading/decode/fused execution.
5. **SGFP4** is a versioned model-weight encoding and runtime decode contract — not a prompt/inference-input data type, not a processor that expands models to FP32.
6. **SuperGenius** owns distributed execution infrastructure: discovery, capability matching, queues/assignment/retries, transport, attestations, settlement.

**Key rejected alternatives:** SGProcessingManager as global GCS execution manager; a second LLM generation loop in SGProcessingManager; SGFP4 as SGProcessingManager input DataType; decoding SGFP4 entirely to FP32 before MNN execution.

**Consequences:** MNN is the only supported location for native SGFP4 model execution and LLM generation semantics; prompts marked `FP4_ULTRA` or SGProcessing result hashes read as inference output must be corrected; "Future planning ingest should treat this ADR as higher precedence than descriptive codebase snapshots."

---

## Cross-ADR consistency check

D-ADR-01 and D-ADR-02 are complementary (SDK/embedding boundary vs runtime ownership hierarchy). Both place GNUS-NEO-SWARM behind the GCS `RuntimeCoordinator` contract. **No LOCKED-vs-LOCKED contradiction.**

## Merge-mode check vs existing app workstream context

`/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/GeniusCogntiveSystem/.planning/workstreams/app/PROJECT.md` Key Decisions are chat-UX decisions all marked outcome "Pending" (not locked); `STATE.md` Decisions are Phase 1/2 implementation notes (not locked architecture decisions). **No ingest LOCKED decision contradicts an existing locked decision.** See INGEST-CONFLICTS.md INFO entries for the `neoswarm_ffi` dependency and repo-layout gaps against D-ADR-01.

## Decision-flavored content that is NOT a decision record

- `docs/architecture/execution-integrity-system.md` self-declares **"Status: Draft for review"** — its zk-exclusion rationale is a proposed spec constraint, not a locked decision (see constraints.md).
- `docs/architecture/exploratory/sloth-integration.md` ends with an informal "Verdict: Integrate Unsloth" — exploratory rationale under `exploratory/`, not an ADR (see context.md).
- `docs/architecture/openai-compatible-api-router-and-gcs-job-queue.md` §26.26 lists 9 unresolved Open Questions (host repo, protobuf-vs-JSON-first, lease semantics, alias policy, etc.) — open, not decided.
