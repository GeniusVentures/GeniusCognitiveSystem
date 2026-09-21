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
  "adaptive_artifact_refs": {
    "model_lineage_id": "optional_lineage",
    "target_expert_id": "optional_expert",
    "capability": "optional_generate|judge|classify|rank|refine|infill|embed",
    "judgment_family": "optional_family",
    "distillation_view_ref": "optional_content_id",
    "state_shard_ref": "optional_content_id",
    "branch_shard_ref": "optional_content_id",
    "adapter_or_head_version": "optional_version_or_cid",
    "readout_kind": "optional_next_token|candidate_readout|decision_head|diffusion_structured_read"
  },
  "policy_hash": "effective_policy_hash",
  "tenant_scope": "optional_tenant_id",
  "owner_id": "optional_owner_id",
  "creator": "user|model|expert|tool|system|swarm",
  "source_node": "node_id",
  "privacy_scope": "local_only|user_private|trusted_devices|enterprise_private|tenant_private|shared|public|compound",
  "privacy_boundary": {
    "constraints": [
      {"kind": "user|enterprise|tenant|device_group|network|workspace|project", "id": "boundary_id"}
    ]
  },
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

The signature covers the canonical event envelope, including its schema version, source references, source policy hashes, effective policy hash, privacy boundary, governance fields, outcome, scores, component versions, and provenance. This lets every receiving subsystem verify both event integrity and the policy context under which the event may be used.

### Governance Resolution

Qualified Cognitive Events inherit the governance of their source Cognitive Assets. Before dispatch, the RuntimeCoordinator or its attached policy service resolves the referenced assets and computes one effective governance envelope.

Privacy scopes are **authorization boundaries, not a total ordering**. Values such as `user_private`, `enterprise_private`, and `tenant_private` can be incomparable, so the resolver must not collapse them by selecting a single nominally "most restrictive" enum value. Instead, each applicable scope expands to concrete boundary constraints and the effective privacy boundary is the conjunction/intersection of all of them.

For example, an event derived from a `user_private` asset and an `enterprise_private` asset is usable only where **both** constraints hold: by the authorized user/principal set **and** inside the authorized enterprise boundary. The event may use `privacy_scope: "compound"` to summarize that condition, while `privacy_boundary.constraints` carries the enforceable boundary. The summary scope is never sufficient authorization by itself.

Governance resolution therefore follows these rules:

- `allowed_principals` and `allowed_roles` are set intersections across the applicable source, request, tenant, and deployment permissions;
- privacy boundaries are intersected as concrete identity, tenant, device-group, network, workspace, project, or equivalent constraints; no source boundary may be dropped merely because another boundary appears narrower;
- replication and inference policy are resolved by intersecting the destinations and execution locations permitted by every applicable policy;
- training and export policy are resolved by intersecting the operations permitted by every applicable policy;
- retention follows the earliest applicable expiry or the strictest governing retention rule;
- policy tags and provenance references are preserved across derivation;
- `source_policy_hashes` bind the event to the policies resolved from its sources;
- `policy_hash` identifies the resulting effective governance used for dispatch.

If any required intersection is empty, contradictory, cannot be represented by the event schema, or cannot be enforced by the selected execution path, the derived event must be rejected from that dispatch/adaptation path rather than widened to make it usable. A controlled redaction or declassification may create a separately governed derived asset only when an explicit source policy authorizes that transformation.

A derived event therefore remains within the same or a narrower authorization boundary than every source. Receiving subsystems verify the schema, signature, effective policy, concrete privacy boundary, and source-policy linkage before using the event for memory, replay, adaptation, replication, export, or training.

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
- effective privacy boundary plus authorization, replication, inference, training, export, and retention governance;
- component and artifact versions;
- structured outcomes and reward signals;
- provenance and signatures.

Subsystem-specific payloads remain allowed. VTG transition outcomes, EGGROLL fitness packets, Cognitive Training Events, EJM canonical decision records, Distillation View manifests, verifier results, and benchmark results may keep their specialized fields while sharing the common envelope.

For EJM learning, the shared envelope governs every derived layer: canonical decision state/branch artifacts, target-specific Distillation Views, EGGROLL training-shard manifests, and promoted adapter/head/readout artifacts. Derived artifacts inherit the effective privacy boundary and training/export restrictions of all source events.

### EJM Decision Replay and Distillation Lineage

A bounded judgment replay must distinguish **semantic source data** from **target-specific compiled training data**.

The semantic layer references a canonical decision state plus one or more decision branches containing criteria, semantic choices, probability targets or verified outcomes, dependency semantics, provenance, and governance. The compiled layer references a Distillation View for one target model/expert lineage, tokenizer/template, readout/head kind, objective, and validation policy.

Replay must therefore be able to answer:

- Which canonical state and branch records produced this training example?
- Which target model lineage and expert/capability compiled the example?
- Which tokenizer/template and readout/head representation was used?
- Which EGGROLL state/branch shard was evaluated?
- Which privacy/training/export policies were effective?
- Which artifact version was eventually promoted or rolled back?

Independent decision branches may share a canonical state and processor prefix/cache, but replay must verify that they did not consume sibling questions, answers, or intermediate state unless their dependency graph explicitly permits it.

### Replay Contract

A replayable event should identify:

- the Qualified Cognitive Event schema version and effective policy hash;
- the execution plan or plan reference;
- model lineage, expert, capability, adapter/head/readout, tokenizer/template, quantization, processor, and runtime versions;
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
- target model lineage and expert/capability identity where applicable;
- Distillation View, state-shard, and branch-shard identifiers for EJM training/replay;
- validation and canary status;
- active model, adapter, head/readout, tokenizer/template, and runtime artifact versions;
- provenance, policy, and approval state.

Raw hidden reasoning and restricted source material remain governed by the existing thinking-context, privacy, and Cognitive Asset rules.

## Summary

Cognitive Evolution Coordination gives GCS one structured path from completed work to memory, transition, policy, and model improvement. It unifies existing learning mechanisms through signed, governed event, replay, and promotion contracts while preserving the current RuntimeCoordinator, component ownership, security, memory, VTG, and EGGROLL architecture.
