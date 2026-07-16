## **8.4 GNUS Agentic Memory Layer (GAML v1)**

## **8.4.1 Purpose**

The GNUS Agentic Memory Layer (GAML) introduces structured, reasoning-oriented long-term memory into GeniusCognitiveSystem.

Unlike traditional RAG pipelines that rely only on embedding similarity and vector databases, GAML treats retrieval as a governed cognitive process involving bridge blocks, facts, policies, events, trust metadata, privacy boundaries, execution-integrity evidence, and orchestration-aware selection.

GAML enables:

* Persistent structured memory across GNUS nodes
* Local-only, user-private, enterprise-private, tenant-scoped, and public memory
* Multi-hop reasoning over historical state
* Temporal coherence enforcement
* Swarm-consensus memory resolution where required
* Storage of execution claims, calibration data, and fraud verdicts as auditable Cognitive Assets
* Reduced dependency on brute-force transcript replay

This makes GeniusCognitiveSystem memory-native rather than prompt-extended.

---

## **8.4.2 Architectural Position**

GAML operates between:

* Router / Planner Layer
* Memory Governor
* Semantic Core / Expert Execution
* Execution Integrity System (EIS)
* Grounding and Verification

Updated flow:

Client API  
↓  
Router / Planner  
↓  
GAML Retrieval + Memory Governor  
↓  
Semantic Core / Expert Nodes  
↓  
EIS Claims + Verification / Arbitration  
↓  
Grounding Validation  
↓  
Final Response

---

## **8.4.3 Memory Object Model**

Long-term memory is stored as structured objects.

```ruby
MemoryObject {
  id: UUID
  entity: string
  type: {bridge_block, fact, policy, event, tenant_operational}
  payload: structured JSON
  timestamp: int64
  source_node: NodeID
  confidence_score: float
  provenance_score: float
  trust_class: {higher_trust, lower_trust, unverified}
  privacy_scope: {local_only, user_private, trusted_devices, enterprise_private, tenant_private, shared, public}
  owner_id: optional string
  replication_policy: {none, trusted_devices, private_subnet, tenant_nodes, public_swarm}
}
```

Stored via:

* RocksDB (local node)
* IPFS-lite or approved content-addressed storage where policy permits replication
* CRDT synchronization within the object's authorized replication boundary

Embeddings may still be used where useful, but they are not the sole definition of memory and inherit the privacy scope of their source.

---

## **8.4.4 Cognitive Asset Model**

GAML should treat long-term memory, bridge blocks, reasoning traces, tool results, plans, execution claims, verification outputs, consensus records, and distillation samples as related forms of a common abstraction: the **Cognitive Asset**.

A Cognitive Asset is any structured artifact produced or consumed by GeniusCognitiveSystem that contributes to reasoning, learning, execution, memory, verification, grounding, or coordination. Memory objects are therefore one important class of Cognitive Asset, but not the only class.

Representative Cognitive Asset types include:

* **Fact** — atomic claim, observation, or domain statement with confidence and provenance.
* **Goal** — desired outcome, user objective, tenant objective, or agent subgoal.
* **Constraint** — hard or soft boundary affecting reasoning, routing, execution, or output.
* **Policy** — safety, privacy, tenant, tool-use, formatting, or compliance rule.
* **Bridge Block** — compact continuity artifact summarizing a task span, workflow state, decision, or active context.
* **Procedure** — reusable workflow, tool sequence, coding pattern, deployment step, or tenant playbook.
* **Tool Result** — structured output from a tool call, dry run, execution attempt, or external service.
* **Capability** — normalized operation that GCS may request through a connector under a governed contract.
* **Connector** — protocol or provider adapter that exposes one or more capabilities.
* **Capability Provider** — service, application, device, node, or tenant system supplying capabilities.
* **Plan** — decomposition, execution graph, routing path, or specialist schedule.
* **Execution Claim** — node assertion describing the execution contract, token sequence, checkpoint digests, model/container hashes, determinism class, kernel manifest, and sampling seed used for a job.
* **Checkpoint Calibration** — drift-band, exception-rate, checkpoint-density, and substitution-margin parameters produced during kernel/model registration.
* **Verification Result** — factual, mathematical, code, grounding, policy, schema, tool-output, or execution-integrity check.
* **Execution Verdict** — EIS fraud, pass, escalation, or borderline verdict generated from teacher-forced spot checks and checkpoint-band comparison.
* **Arbitration Result** — decision resolving competing drafts, critiques, experts, or memory states.
* **Consensus Record** — reputation-weighted agreement, disagreement, voting result, or swarm finalization artifact.
* **Benchmark Result** — evaluation output used to measure model, specialist, router, memory, or execution-integrity quality.
* **Distillation Sample** — training example derived from planning, routing, verification, synthesis, tool use, consensus, or final response behavior.
* **Specialist Trace** — record of which ELM or service was invoked, what context it used, what it produced, and how it performed.

A Cognitive Asset should carry enough metadata to support future retrieval, verification, replication, privacy enforcement, and training governance:

```ruby
CognitiveAsset {
  id: UUID
  asset_type: enum
  payload: structured JSON
  created_at: int64
  updated_at: int64
  source_node: NodeID
  tenant_scope: optional string
  owner_id: optional string
  creator: {user, model, expert, tool, system, swarm}
  confidence_score: float
  provenance_score: float
  freshness_score: float
  trust_class: {higher_trust, lower_trust, unverified}
  privacy_scope: {
    local_only,
    user_private,
    trusted_devices,
    enterprise_private,
    tenant_private,
    shared,
    public
  }
  allowed_principals: string[]
  allowed_roles: string[]
  replication_policy: {none, trusted_devices, private_subnet, tenant_nodes, public_swarm}
  inference_policy: {local_only, private_nodes, public_with_redaction, public_allowed}
  training_policy: {prohibited, local_only, tenant_only, anonymized_opt_in, allowed}
  export_policy: {prohibited, approval_required, allowed}
  encryption_key_ref: optional string
  retention_policy: optional string
  policy_tags: string[]
  references: UUID[]
  supports: UUID[]
  contradicts: UUID[]
  supersedes: UUID[]
  derived_from: UUID[]
  embedding_refs: optional string[]
  signature: optional bytes
}
```

Trust and privacy are independent dimensions. A user-private email may be higher-trust while remaining local-only and prohibited from training. A public webpage may be public but lower-trust and usable only for grounding.

The graph relationships are as important as the payload. A fact may support a policy, a procedure may depend on a tool result, a bridge block may reference a conversation summary, an execution verdict may contradict a node claim, a verifier result may contradict a generated answer, and a distillation sample may be derived from a consensus record. This allows GAML to behave as a cognitive graph rather than a flat memory table.

Cognitive Assets should not all be injected into prompts. The Memory Governor decides which assets are authorized, relevant, fresh, trusted, policy-allowed, and compact enough for the current execution path. Some assets are useful only for offline evaluation, distillation, EIS calibration, reputation updates, or later reconciliation.

---

## **8.4.5 Ingestion Pipeline**

When new information enters the system (conversation, task result, user preference, local source, tool outcome, execution claim, or verifier verdict), GAML evaluates multiple memory-oriented functions:

1. **Fact Extraction** – converts raw output into atomic structured facts where appropriate.
2. **Context Mapping** – associates facts and events with session, task, workflow, node, model, user, tenant, and source context.
3. **Privacy Classification** – determines ownership, privacy scope, replication boundary, inference policy, training policy, retention, and export restrictions.
4. **Temporal Tracking** – resolves updates, contradictions, stale state, and superseded execution or adapter versions.
5. **Write Evaluation** – scores novelty, expected utility, provenance, contamination risk, and policy compatibility before durable storage.

The exact implementation may be heuristic, model-assisted, or hybrid depending on deployment stage. AI-assisted classification produces proposals; deterministic policy and authorization checks remain authoritative.

---

## **8.4.6 Agentic Retrieval Mechanism**

For each memory query, GAML performs staged retrieval rather than raw log replay.

Representative retrieval stages:

1. identity and authorization validation
2. privacy-scope and replication-boundary filtering
3. purpose, policy, and tenant boundary checks
4. trust and provenance filtering
5. metadata prefiltering
6. temporal resolution
7. semantic matching where available
8. Memory Governor selection of the final context set

Privacy enforcement occurs before relevance ranking. Unauthorized memory must not enter the candidate set merely because it is semantically similar.

EIS trust-class checks may also apply where execution evidence affects confidence. Results may be merged using reputation-weighted, policy-aware selection when multiple authorized nodes return conflicting or overlapping state.

---

## **8.4.7 Surprise-Gated Writes**

Surprise remains useful, but only as one signal among several. A memory write is favored when it is:

* novel
* useful for future routing or generation
* useful for future execution-integrity calibration or fraud detection
* durable enough to matter
* allowed under privacy, retention, and training policy
* consistent with the existing memory graph

A source being locally accessible does not automatically authorize durable storage. Reading, using as temporary context, memorizing, replicating, exporting, and training are separate permissions.

---

## **8.4.8 Memory as Support for Experts**

Memory is not only for the Semantic Core. It also provides expert-specific context, such as:

* user style and formatting preferences for a Refiner or Formatter ELM
* domain facts for a Math or Scientific ELM
* prior tool state for a Tool-Support ELM
* historical conflicts for an Arbiter ELM
* tenant workflow rules for a private Operations ELM
* EIS execution history, model-version evidence, and node-integrity signals for scheduling and trust decisions

Experts receive only memory permitted for their identity, role, execution location, tenant, purpose, and privacy boundary.

---

## **8.4.9 Swarm Memory Consensus**

When multiple authorized nodes return conflicting memory states:

1. Responses are scored using:
    * Node reputation
    * Confidence score
    * Recency
    * Provenance / trust class
    * EIS execution-integrity evidence where applicable
2. Conflict resolution is performed using CRDT plus policy-aware weighted selection.
3. Final resolved memory is injected into inference, grounding, or verification context only when the selected execution location is permitted to receive it.

This prevents memory poisoning and maintains decentralized trust integrity without weakening privacy boundaries.

Private memory does not require public swarm consensus. Consensus scope must remain within the object's authorized device group, private subnet, tenant, or other replication boundary.

---

## **8.4.10 Replication and Convergence**

Memory objects can be synchronized across authorized devices and nodes using CRDT-style convergence and content-addressed integrity. This preserves local autonomy while enabling distributed continuity.

Replication is policy-scoped:

* **Local-only** assets are not replicated.
* **Trusted-device** assets may replicate only between devices authorized by the owner.
* **Enterprise-private** assets may replicate only within the approved private subnet or enterprise boundary.
* **Tenant-private** assets may replicate only to authorized tenant nodes.
* **Public** assets may replicate through the public swarm when policy allows.

Storage and relay nodes may carry encrypted objects without being authorized to decrypt them. Unencrypted metadata must be minimized because entity names, timestamps, graph edges, and embeddings can reveal private information.

Deletion, revocation, and retention expiry must propagate through the same authorized convergence boundary using tombstones or equivalent version-aware records.

---

## **8.4.11 Private Memory, Ownership, and Derived Assets**

Private memory is a core GAML property rather than an optional deployment feature.

### **8.4.11.1 Privacy Scopes**

GAML supports at least:

* **Local-only memory** — remains on one device and is available only to approved local execution.
* **User-private memory** — belongs to one user and may synchronize only across approved devices.
* **Trusted-device memory** — shared among an explicitly authorized device group.
* **Enterprise-private memory** — shared only inside an approved enterprise boundary or private GNUS subnet.
* **Tenant-private memory** — available only to authorized identities and nodes within a tenant.
* **Shared memory** — deliberately shared with named principals, projects, workspaces, or groups.
* **Public memory** — approved for public retrieval or public swarm replication.

### **8.4.11.2 Ownership and Authorization**

Every non-public asset should identify an owner or governing tenant and may declare authorized principals and roles. Authorization must be evaluated against the requesting user, agent, node, workspace, tenant, execution location, and intended purpose.

### **8.4.11.3 Inference Boundaries**

Private memory may be used by:

* a local ELM on the same device
* an approved model on a trusted user device
* a private enterprise ELM or GNUS subnet
* a public swarm only through an explicitly allowed redacted context packet

The Memory Governor may transform private source material into a less identifying context packet only when policy permits. The redacted packet becomes a separately governed derived asset with provenance back to the private source.

### **8.4.11.4 Encryption and Key References**

Private payloads should be encrypted at rest and in transit using keys scoped to the user, trusted device group, enterprise, tenant, workspace, or project as appropriate.

GAML stores key references and policy metadata, not raw secret keys. Keys remain in approved keychains, hardware-backed stores, enterprise secret managers, or equivalent protected services.

### **8.4.11.5 Derived-Asset Inheritance**

Derived artifacts inherit the source's privacy and policy restrictions unless an explicit policy produces a more restrictive result or approves a controlled transformation.

This inheritance applies to:

* summaries
* extracted facts and entities
* graph relationships
* Bridge Blocks
* embeddings and vector indexes
* tool results
* reasoning and specialist traces
* distillation samples
* EGGROLL adaptation signals
* VTG state and transition artifacts
* benchmarks and evaluation records

A private document must not become public merely because its text was summarized, embedded, transformed, or used by a model.

### **8.4.11.6 Training and Adaptation**

Private memory is excluded from global training and adaptation by default. Local or tenant-scoped training may occur only under the asset's training policy. Anonymized or broader contribution requires explicit opt-in and validation that the derived artifact no longer exposes restricted source information.

### **8.4.11.7 Local Personal Data**

Local email, calendars, contacts, notes, browser research, files, application databases, meeting transcripts, and device events may be accessed through approved capabilities. Local availability grants neither durable-memory permission nor replication permission.

A representative flow is:

```text
Local Source
    ↓
Capability Connector
    ↓
Tool Intermediary
    ↓
Temporary Authorized Context
    ↓
Extraction and Privacy Classification
    ↓
Proposed Private GAML Assets
    ↓
Write Evaluation
```

---

## **8.4.12 Performance & Overhead Impact**

Estimated impact in Swarm Mode:

* Additional memory-related compute and storage overhead
* Privacy and authorization checks before retrieval and replication
* Encryption and key-resolution overhead for private assets
* Minimal GPU overhead relative to core inference in many deployments
* Horizontal scalability across authorized GNUS nodes
* Reduced hallucination risk via structured recall
* Better execution-integrity auditability through stored claims, calibration assets, and verdicts

Compared to purely transcript-based context replay:

* Lower prompt bloat
* Better temporal coherence
* Better policy-aware context control
* Stronger local, private, and tenant memory isolation

---

## **8.4.13 Strategic Impact**

GAML transforms GeniusCognitiveSystem v1 from:

Distributed Inference Engine  
→  
Distributed Cognitive System

It aligns directly with:

* Hierarchical Reasoning Model
* Semantic Core plus ELM execution
* Execution Integrity System evidence and audit records
* Reputation-weighted consensus
* Distributed GNUS infrastructure
* Secure memory governance for agentic workflows
* Local-first and enterprise-private cognition
* Protocol-neutral capability and connector integration

GAML v1 is intentionally practical. Future versions may deepen semantic indexing, memory governance, execution-integrity asset handling, encrypted replication, private operational memory, revocation, and cross-device continuity as needed.
