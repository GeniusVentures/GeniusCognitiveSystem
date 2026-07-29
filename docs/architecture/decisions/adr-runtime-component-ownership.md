# ADR: Runtime Component Ownership Boundaries

- **Status:** accepted
- **Date:** 2026-07-28
- **Scope:** GCS, GNUS-NEO-SWARM, SGProcessingManager, SuperGenius, MNN, SGFP4
- **Decision type:** architecture, component ownership, runtime boundaries

## Context

GeniusCognitiveSystem spans cognitive orchestration, local and distributed model execution, processing pipelines, network scheduling, verification, and user-facing applications. Several repositories participate in that system, and overlapping terms such as *execution*, *runtime*, and *processing manager* have caused responsibilities to be assigned to the wrong layer.

In particular, SGProcessingManager was at risk of becoming a second LLM runtime and being treated as the global execution manager for GCS. SGProcessingManager pull request `GeniusVentures/SGProcessingManager#6` attempted to add a custom autoregressive MNN loop and an `FP4_ULTRA` input processor. That design duplicated MNN responsibilities and represented SGFP4 model weights as input data.

The architecture needs an authoritative ownership boundary that preserves separation of concerns while allowing components to be composed.

## Decision

### 1. GeniusCognitiveSystem defines cognitive architecture and public contracts

GeniusCognitiveSystem owns the system-level definitions for:

- cognitive request and response contracts;
- routing and planning semantics;
- execution-plan semantics;
- context, memory, grounding, and policy semantics;
- expert and ELM invocation semantics;
- verification, arbitration, synthesis, and finalization semantics;
- runtime-independent streaming, cancellation, status, artifact, and error contracts;
- stable embedding contracts used by applications and SDKs.

GeniusCognitiveSystem defines what the cognitive system must do. It does not directly own device-specific model kernels, network scheduling, or every concrete runtime implementation.

### 2. GNUS-NEO-SWARM provides the primary GCS runtime implementation

GNUS-NEO-SWARM owns the concrete runtime coordinator that implements GCS execution semantics. It coordinates the full request lifecycle, including:

- receiving a GCS runtime request;
- constructing or consuming an execution plan;
- maintaining request and session state;
- invoking routing, memory, grounding, experts, verification, arbitration, and synthesis;
- deciding which work runs locally and which work is delegated;
- coordinating deadlines, cancellation, budgets, partial results, and fallbacks;
- constructing the final GCS result.

The runtime coordinator composes lower-level services. It is not itself the model runtime or distributed worker scheduler.

### 3. SGProcessingManager is a partial processing-workload executor

SGProcessingManager executes selected workloads that fit its processing contract, including:

- declared processing passes and pass graphs;
- typed input, output, and intermediate resource bindings;
- model-inference passes delegated to an available model runtime;
- Vulkan compute and render passes;
- tensor, media, and data-transformation workloads;
- workload-scoped progress, cancellation, deadlines, artifacts, and manifests.

SGProcessingManager does **not** own:

- the global GCS request lifecycle;
- cognitive routing or planning;
- conversation or session semantics;
- expert-selection policy;
- memory or grounding strategy;
- verification, arbitration, or final synthesis;
- tokenizer behavior, chat templates, autoregressive generation, sampling, or KV-cache management.

A GCS request may invoke zero, one, or multiple SGProcessingManager jobs. The GCS runtime coordinator remains responsible for the surrounding cognitive lifecycle.

### 4. MNN owns model-runtime semantics

MNN owns individual model execution behavior, including:

- model and tokenizer loading;
- model-specific prompt and chat-template handling;
- tokenization and detokenization;
- autoregressive generation;
- KV-cache management;
- sampling and decoding behavior;
- speculative decoding where supported;
- CPU and Vulkan execution;
- MoltenVK-backed Vulkan execution on Apple platforms;
- native SGFP4 model-weight loading, decode, and fused execution paths.

SGProcessingManager and GNUS-NEO-SWARM invoke MNN through stable runtime interfaces rather than reimplementing MNN internals.

### 5. SGFP4 is a model-weight format

SGFP4 is a versioned model-weight encoding and runtime decode contract. It is not:

- a prompt or inference-input data type;
- an LLM execution type;
- a standalone SGProcessingManager processor that expands complete models to FP32 before inference.

SGFP4 metadata belongs to model manifests and execution requirements. Native loading and GPU decode belong in MNN and its Vulkan kernels.

### 6. SuperGenius owns distributed execution infrastructure

SuperGenius owns distributed workload infrastructure, including:

- node discovery and worker availability;
- capability matching;
- task queues, assignment, attempts, and retries;
- network transport and distributed artifact movement;
- result collection and execution attestations;
- accounting and settlement integration.

SuperGenius distributes selected work. It does not determine the cognitive strategy or replace the GCS runtime coordinator.

## Reference hierarchy

```text
Application / Flutter UI
        |
GeniusCognitiveSystem public contracts
        |
GNUS-NEO-SWARM RuntimeCoordinator implementation
        |-- MNN model runtime
        |-- memory, grounding, experts, verification, arbitration
        |-- SGProcessingManager processing workloads
        |       `-- MNN / Vulkan / other processors
        `-- SuperGenius distributed execution
                `-- remote processing and model-runtime workers
```

## Rejected alternatives

### SGProcessingManager as the global GCS execution manager

Rejected because processing-workload execution is only one part of the cognitive lifecycle. This design would couple processing passes to conversation state, expert policy, memory, grounding, verification, and synthesis.

### A second LLM generation loop in SGProcessingManager

Rejected because MNN already owns model-specific generation, tokenizer behavior, KV cache, sampling, templates, and backend execution. A second loop would be incomplete, slower, and likely incompatible with model-specific behavior.

### SGFP4 as an SGProcessingManager input `DataType`

Rejected because SGFP4 describes model weights, not prompt data or intermediate tensors. Model quantization must not change the semantic type of a prompt or token tensor.

### SGFP4 decoded entirely to FP32 before MNN execution

Rejected because it defeats compressed residency, increases bandwidth and memory use, and prevents fused GPU decode and matrix execution.

## Consequences

- The GCS runtime coordinator must be explicit in GNUS-NEO-SWARM rather than being hidden inside SGProcessingManager.
- SGProcessingManager dispatch must be pass- and backend-aware, but it remains subordinate to the runtime coordinator.
- MNN integration becomes the only supported location for native SGFP4 model execution and LLM generation semantics.
- Existing code that marks prompts as `FP4_ULTRA` or interprets SGProcessing result hashes as inference output must be corrected.
- Public SDK and UI layers depend on GCS contracts rather than NeoSwarm internal classes.
- Future planning ingest should treat this ADR as higher precedence than descriptive codebase snapshots.
