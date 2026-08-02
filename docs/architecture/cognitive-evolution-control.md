# Cognitive Evolution Coordination

## Purpose

GeniusCognitiveSystem already produces the information needed for continuous improvement through thinking traces, Cognitive Assets, verification results, arbitration records, consensus outcomes, VTG transition events, user feedback, benchmarks, and EGGROLL fitness signals.

This document defines the coordination contract that connects those existing mechanisms. It does not introduce a separate runtime. The GCS RuntimeCoordinator remains responsible for the cognitive request lifecycle and for routing qualified learning events to the correct adaptive subsystem.

## Architectural Position

```text
Client / API
    ↓
RuntimeCoordinator
    ↓
Context, Routing, Memory, Expert Execution, Tools
    ↓
Verification, Arbitration, Consensus, Grounding
    ↓
Final Response
    ↓
Qualified Cognitive Events
    ↓
Evolution Coordination
    ├── GAML memory and policy updates
    ├── Objective Memory / VTG transition updates
    ├── router, planner, verifier, and arbitration tuning
    ├── specialist or adapter retraining through EGGROLL
    └── benchmark, replay, canary, and promotion workflows
```

Evolution coordination is part of orchestration. GNUS-NEO-SWARM provides the primary implementation behind the GCS RuntimeCoordinator contract. Existing runtime ownership boundaries remain unchanged.

## Qualified Cognitive Event

A Qualified Cognitive Event is a versioned Cognitive Asset that records enough information to support later evaluation, replay, adaptation, and promotion.

```json
{
  "event_id": "uuid",
  "request_id": "uuid",
  "session_id": "optional_uuid",
  "event_type": "memory|transition|routing|verification|arbitration|tool|training|benchmark",
  "source_assets": ["asset_id"],
  "model_and_expert_versions": ["version_or_cid"],
  "policy_hash": "policy_hash",
  "tenant_scope": "scope",
  "privacy_scope": "scope",
  "training_policy": "policy",
  "outcome": "structured_outcome",
  "scores": {
    "confidence": 0.0,
    "verification": 0.0,
    "grounding": 0.0,
    "execution_success": 0.0,
    "user_feedback": 0.0
  },
  "created_at": 0,
  "signature": "optional_signature"
}
```

The event may reference existing Cognitive Assets rather than duplicating large prompts, traces, tool outputs, or model artifacts.

## Coordination Flow

1. The RuntimeCoordinator completes the normal request path.
2. Existing subsystems emit structured Cognitive Assets and outcome signals.
3. The coordinator or an attached policy service assembles Qualified Cognitive Events.
4. Privacy, authorization, retention, replication, and training policies are applied before any adaptation path receives the event.
5. The event is routed to one or more existing adaptive targets:
   - GAML write evaluation;
   - Objective Memory / VTG edge and ranking updates;
   - routing, planning, verifier, critic, or arbitration tuning;
   - specialist, adapter, or micro-model retraining through EGGROLL;
   - benchmark and replay datasets.
6. Candidate changes are evaluated through the validation and promotion process owned by the target subsystem.
7. Promoted artifacts remain versioned, signed, content-addressed where appropriate, and visible to runtime policy and rollback controls.

## Shared Contracts

### Event Contract

All adaptive paths should accept a common event envelope with:

- request, session, tenant, and policy identity;
- source Cognitive Asset references;
- component and artifact versions;
- structured outcomes and reward signals;
- privacy, inference, training, export, and retention policy;
- provenance and signatures where required.

Subsystem-specific payloads remain allowed. VTG transition outcomes, EGGROLL fitness packets, Cognitive Training Events, verifier results, and benchmark results may keep their specialized fields while sharing the common envelope.

### Replay Contract

A replayable event should identify:

- the execution plan or plan reference;
- model, expert, adapter, tokenizer, quantization, and runtime versions;
- context packet and policy hashes;
- tool and capability contract versions;
- deterministic seeds or execution class where available;
- expected validators and measurable outcome criteria.

Replay uses the public GCS request and event contracts. It may execute locally, through SGProcessingManager workloads, or through SuperGenius distributed execution under the existing runtime ownership rules.

### Promotion Contract

Adaptive artifacts should use a shared promotion shape:

```text
candidate
    ↓
validation and benchmark replay
    ↓
policy and safety checks
    ↓
versioned artifact publication
    ↓
canary scope
    ↓
reputation and outcome monitoring
    ↓
broader promotion
```

The target subsystem defines its own metrics and artifact format. GAML governs memory writes, VTG governs transition confidence and replication, EGGROLL governs trained artifacts, and routing or arbitration modules govern their policy artifacts.

## Runtime and Repository Ownership

- **GeniusCognitiveSystem** defines Qualified Cognitive Event, replay, promotion, and public observability contracts.
- **GNUS-NEO-SWARM** implements orchestration and coordinates event creation and dispatch through the RuntimeCoordinator.
- **MNN** executes model inference and exposes the runtime information needed for replay and artifact identity.
- **SGProcessingManager** executes declared replay, benchmark, transformation, or evaluation workloads when selected by the RuntimeCoordinator.
- **SuperGenius** distributes selected workloads and returns execution results and attestations.
- **GAML, Objective Memory / VTG, EGGROLL, routing, verification, and arbitration modules** remain owners of their domain-specific update and promotion rules.

## Observability

The public runtime event stream may expose policy-safe summaries of:

- which adaptive event classes were emitted;
- which target subsystem received them;
- candidate artifact identifiers;
- validation and canary status;
- active artifact versions;
- provenance, policy, and approval state.

Raw hidden reasoning and restricted source material remain governed by the existing thinking-context, privacy, and Cognitive Asset rules.

## Summary

Cognitive Evolution Coordination gives GCS one structured path from completed work to memory, transition, policy, and model improvement. It unifies existing learning mechanisms through shared event, replay, and promotion contracts while preserving the current RuntimeCoordinator, component ownership, security, memory, VTG, and EGGROLL architecture.
