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

A Qualified Cognitive Event is a versioned Cognitive Asset that records enough information to support later evaluation, replay, adaptation, and promotion. Its signed envelope also carries the effective governance derived from the request, tenant, and every referenced source asset.

```json
{
  "schema_version": "gcs.qualified-cognitive-event.v1",
  "event_id": "uuid",
  "request_id": "uuid",
  "session_id": "optional_uuid",
  "event_type": "memory|transition|routing|verification|arbitration|tool|training|benchmark",
  "source_assets": ["asset_id"],
  "source_policy_hashes": ["policy_hash"],
  "model_and_expert_versions": ["version_or_cid"],
  "policy_hash": "effective_policy_hash",
  "tenant_scope": "optional_tenant_id",
  "owner_id": "optional_owner_id",
  "creator": "user|model|expert|tool|system|swarm",
  "source_node": "node_id",
  "privacy_scope": "local_only|user_private|trusted_devices|enterprise_private|tenant_private|shared|public",
  "allowed_principals": ["principal_id"],
  "allowed_roles": ["role_id"],
  "replication_policy": "none|trusted_devices|private_subnet|tenant_nodes|public_swarm",
  "inference_policy": "local_only|private_nodes|public_with_redaction|public_allowed",
  "training_policy": "prohibited|local_only|tenant_only|anonymized_opt_in|allowed",
  "export_policy": "prohibited|approval_required|allowed",
  "retention_policy": "policy_or_reference",
  "policy_tags": ["tag"],
  "provenance": {
    "derived_from": ["asset_id"],
    "execution_claims": ["asset_id"],
    "verification_results": ["asset_id"]
  },
  "outcome": "structured_outcome",
  "scores": {
    "confidence": 0.0,
    "verification": 0.0,
    "grounding": 0.0,
    "execution_success": 0.0,
    "user_feedback": 0.0
  },
  "created_at": 0,
  "signing_identity": "node_or_service_id",
  "signature": "signature_over_canonical_event"
}
```

The event may reference existing Cognitive Assets rather than duplicating large prompts, traces, tool outputs, or model artifacts.

### Schema and Signature Contract

`schema_version` is required and forms part of the signed canonical event. The major version identifies the decoding and validation contract; minor-compatible extensions may add optional fields while preserving the meaning of existing fields. Stored and replayed events retain the schema version under which they were created.

The signature covers the canonical event envelope, including its schema version, source references, source policy hashes, effective policy hash, governance fields, outcome, scores, component versions, and provenance. This lets every receiving subsystem verify both event integrity and the policy context under which the event may be used.

### Governance Resolution

Qualified Cognitive Events inherit the governance of their source Cognitive Assets. Before dispatch, the RuntimeCoordinator or its attached policy service resolves the referenced assets and computes one effective governance envelope:

- `allowed_principals` and `allowed_roles` are the intersection of the applicable source, request, tenant, and deployment permissions;
- privacy, replication, inference, training, and export permissions use the most restrictive applicable scope;
- retention follows the earliest applicable expiry or the strictest governing retention rule;
- policy tags and provenance references are preserved across derivation;
- `source_policy_hashes` bind the event to the policies resolved from its sources;
- `policy_hash` identifies the resulting effective governance used for dispatch.

A derived event therefore remains within the same or a narrower authorization boundary than its sources. Receiving subsystems verify the schema, signature, effective policy, and source-policy linkage before using the event for memory, replay, adaptation, replication, export, or training.

## Coordination Flow

1. The RuntimeCoordinator completes the normal request path.
2. Existing subsystems emit structured Cognitive Assets and outcome signals.
3. The coordinator or an attached policy service assembles Qualified Cognitive Events.
4. Source assets are resolved, their governance is combined into the effective policy envelope, and the canonical event is signed before adaptation dispatch.
5. The event is routed to one or more existing adaptive targets within that effective governance:
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

- signed schema version and canonical event identity;
- request, session, tenant, owner, and policy identity;
- source Cognitive Asset and source-policy references;
- effective privacy, authorization, replication, inference, training, export, and retention governance;
- component and artifact versions;
- structured outcomes and reward signals;
- provenance and signatures.

Subsystem-specific payloads remain allowed. VTG transition outcomes, EGGROLL fitness packets, Cognitive Training Events, verifier results, and benchmark results may keep their specialized fields while sharing the common envelope.

### Replay Contract

A replayable event should identify:

- the Qualified Cognitive Event schema version and effective policy hash;
- the execution plan or plan reference;
- model, expert, adapter, tokenizer, quantization, and runtime versions;
- context packet and policy hashes;
- tool and capability contract versions;
- deterministic seeds or execution class where available;
- expected validators and measurable outcome criteria.

Replay uses the public GCS request and event contracts. It may execute locally, through SGProcessingManager workloads, or through SuperGenius distributed execution under the existing runtime ownership rules and the event's effective governance.

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

Cognitive Evolution Coordination gives GCS one structured path from completed work to memory, transition, policy, and model improvement. It unifies existing learning mechanisms through signed, governed event, replay, and promotion contracts while preserving the current RuntimeCoordinator, component ownership, security, memory, VTG, and EGGROLL architecture.
