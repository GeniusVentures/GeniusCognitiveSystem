# **28. GCS Capability System**

## **28.1 Purpose**

The GCS Capability System provides a protocol-neutral way for GeniusCognitiveSystem to discover, describe, govern, route, execute, and evaluate external and local capabilities.

A capability is an operation the system may request, such as reading local email, searching a repository, querying a database, retrieving a document, creating a calendar event, invoking an enterprise API, or executing a GNUS network service.

The core architectural rule is:

> GCS reasons about capabilities, not connector protocols. Connectors expose capabilities, and every capability is translated into a canonical internal contract before execution.

MCP is one supported connector protocol. It is not the internal authority model and does not replace GCS policy, capability enforcement, sandboxing, attestation, reputation, or memory governance.

---

## **28.2 Architectural Position**

```text
External and Local Systems
    ├── MCP servers
    ├── REST / OpenAPI services
    ├── GraphQL services
    ├── gRPC services
    ├── local application APIs
    ├── operating-system services
    ├── databases and local data stores
    ├── native or WASM modules
    └── GNUS network services
            ↓
Connector Adapters
            ↓
Capability Discovery and Translation
            ↓
Canonical Capability Contract
            ↓
Router / Planner + Policy Evaluation
            ↓
Tool Intermediary
            ↓
Sandboxed and Attested Execution
            ↓
Sanitized Result
            ↓
Verification, GAML Write Evaluation, and Reputation Update
```

The Capability System sits between external connector protocols and the existing Tool Intermediary. It normalizes what an operation can do, but it does not authorize execution by itself.

---

## **28.3 Core Concepts**

### **28.3.1 Capability**

A capability describes an operation available to GCS independently of how that operation is transported.

Examples:

* `email.message.read`
* `calendar.event.search`
* `calendar.event.create`
* `filesystem.document.read`
* `github.issue.create`
* `database.query.readonly`
* `browser.page.retrieve`
* `gnus.job.submit`

### **28.3.2 Connector**

A connector implements communication with a provider or local system.

One connector may expose many capabilities. The same capability may also be available through several connectors.

```text
github.issue.create
    ├── GitHub MCP connector
    ├── GitHub REST / OpenAPI connector
    └── private enterprise GitHub connector
```

The Router may select among equivalent connectors using policy, privacy, reliability, reputation, latency, cost, location, and current availability.

### **28.3.3 Capability Provider**

A provider is the entity, service, application, device, node, or tenant system that supplies one or more connectors and capabilities.

### **28.3.4 Capability Contract**

A capability contract is the canonical internal representation used by GCS policy and execution layers. External schemas are discovery input only. They do not grant authority.

---

## **28.4 Canonical Capability Contract**

Representative schema:

```yaml
capability_id: email.message.read
version: 1.0.0

provider:
  provider_id: local.apple-mail
  connector_id: apple-mail-local
  connector_type: local_application

operation:
  class: read
  side_effect_class: none
  determinism_class: source_dependent

permissions:
  - email.message.read

data_classes_read:
  - private_communication

data_classes_written: []

privacy:
  supported_scopes:
    - local_only
    - user_private
  default_scope: local_only

execution:
  network: none
  filesystem: application_scoped
  credentials: operating_system_grant
  timeout_ms: 15000
  requires_sandbox: true

approval:
  discovery: user_or_admin
  read: policy_dependent
  mutation: not_applicable

policy_tags:
  - personal_data
  - local_source

manifest_hash: sha256:...
provider_signature: optional
```

The contract should include enough information to support:

* capability and provider identity
* versioning and manifest drift detection
* read and write permissions
* data classifications
* privacy and tenant boundaries
* side-effect classification
* sandbox and network requirements
* credential requirements
* human approval policy
* determinism and verification requirements
* cost and latency metadata where applicable
* provider signature and contract hash

---

## **28.5 Discovery and Contract Translation**

Connector adapters may discover operations from MCP tool schemas, OpenAPI documents, GraphQL schemas, gRPC descriptors, native manifests, application APIs, or manually defined interfaces.

A translation pipeline converts those descriptions into canonical capability contracts:

```text
Discover
  → Inspect
  → Normalize
  → Classify permissions and data
  → Generate capability contract
  → Validate deterministically
  → Apply policy
  → Human or administrator review where required
  → Register
```

AI may assist with schema interpretation, permission inference, documentation, test generation, and risk classification. AI-generated contracts remain proposals until deterministic validators and applicable policy or human review approve them.

The AI rewrites the connector's declared operation into the GCS contract model. It does not rewrite the external protocol.

---

## **28.6 MCP Connector Adapter**

The MCP adapter allows GCS to act as an MCP host or client while preserving the GCS authority model.

The adapter may:

* connect to approved MCP servers
* enumerate tools, resources, and prompts
* translate MCP tool schemas into capability contracts
* map MCP calls into GCS `ToolProposal` objects
* route all execution through the Tool Intermediary
* sanitize returned content before model or memory use
* expose selected GCS capabilities through an MCP server interface

No MCP operation executes directly from an ELM or Semantic Core worker.

```text
MCP tools/list
    ↓
Capability Translation
    ↓
Capability Contract
    ↓
ToolProposal
    ↓
Tool Intermediary
    ↓
Dry Run + Policy + Approval + Execution
    ↓
SanitizedData + ToolAttestation
```

MCP transport compatibility does not imply trust. Community, unverified, first-party, local, and enterprise MCP servers may receive different policy and sandbox profiles.

---

## **28.7 Connector Categories**

The initial connector catalog should emphasize operational and private-data capabilities rather than external AI dependence.

### **28.7.1 Local Personal Data**

* local email stores and mail applications
* local calendars and contacts
* notes and Markdown or Obsidian vaults
* local documents and PDFs
* browser research captures and history where approved
* meeting transcripts and voice notes
* notifications and local application databases

### **28.7.2 Developer and Engineering Systems**

* source repositories
* issue and project trackers
* CI/CD systems
* package registries
* code indexing and build systems

### **28.7.3 Enterprise and Productivity Systems**

* email and calendar services
* document and collaboration platforms
* CRM, ERP, support, and ticketing systems
* internal APIs, databases, and knowledge systems

### **28.7.4 Data and Knowledge Systems**

* relational and document databases
* vector and graph indexes
* internal wikis
* public and private grounding sources
* filesystem and object storage

### **28.7.5 Finance, Commerce, and Blockchain**

* accounting and invoicing systems
* payment processors
* banking APIs where explicitly authorized
* blockchain RPC and smart-contract interfaces
* wallet and settlement services

### **28.7.6 Device and Operating-System Services**

* local filesystem access
* secure storage
* sensors and IoT interfaces
* notifications
* application-scoped local APIs
* controlled process or shell execution

### **28.7.7 GNUS and GCS Services**

* GCS job submission and status
* GNUS node capabilities
* private and public swarm services
* GAML retrieval and proposed writes
* grounding, verification, EIS, and settlement services

### **28.7.8 Optional External Inference Providers**

External model services may be supported through policy-scoped connectors for distillation, benchmarking, verification, fallback, or explicitly authorized tenant workflows. They are not part of the trusted cognitive core and do not replace the Semantic Core, GAML, EIS, GCS routing, or GNUS distributed execution.

---

## **28.8 Local Capability Execution**

A capability may execute entirely on the same device as the user and GCS runtime.

Example local email flow:

```text
Local Mail Store
    ↓
Application-Scoped Email Connector
    ↓
email.message.read Capability
    ↓
Tool Intermediary
    ↓
Local extraction or local ELM
    ↓
Temporary context or proposed private GAML memory
```

Local access does not imply permission to memorize, replicate, train on, or transmit the resulting data. Those decisions are governed separately by GAML privacy policy and the Memory Governor.

---

## **28.9 Capability Routing and Reputation**

Capability selection may consider:

* required operation and data type
* privacy boundary
* tenant policy
* provider and connector reputation
* execution success rate
* latency and cost
* locality and network availability
* credential scope
* side-effect risk
* current health and version compatibility

Execution outcomes should update provider- and connector-specific reputation without allowing a connector to grant itself higher authority.

---

## **28.10 Credential and Secret Handling**

Credentials are not Cognitive Asset payloads and must not be stored directly in GAML.

The Capability System stores only credential references, ownership, approved scopes, expiration state, and policy metadata. Secret material remains in an approved operating-system keychain, enterprise secret manager, hardware-backed store, or other protected credential service.

Credentials must be bound to the requesting user, tenant, workspace, capability, and provider as narrowly as practical. Tokens obtained for one provider or audience must not be forwarded to another.

---

## **28.11 Tool Intermediary Integration**

Every capability invocation that reads external or local protected data, accesses credentials, performs a side effect, or may affect durable memory passes through the Tool Intermediary.

The Tool Intermediary remains responsible for:

* capability and policy validation
* deterministic dry-run where supported
* sandbox profile selection
* network and filesystem restriction
* human approval gating
* output sanitization and instruction scrubbing
* signed execution attestation
* independent `safe_to_execute` and `safe_to_memorize` decisions

The Capability System describes what may be requested. The Tool Intermediary decides whether a specific request may execute.

---

## **28.12 GAML Integration**

GAML may represent durable capability knowledge using Cognitive Assets such as:

* `Capability`
* `Connector`
* `CapabilityProvider`
* `CapabilityVersion`
* `Permission`
* `Policy`
* `CredentialReference`
* `SecurityReview`
* `ReputationRecord`
* `ToolResult`

Representative relationships include:

```text
Connector exposes Capability
Capability provided_by CapabilityProvider
Capability requires Permission
Capability governed_by Policy
Capability supersedes CapabilityVersion
Capability executed_through Connector
Capability verified_by ToolAttestation
ToolResult derived_from Capability
```

Live secrets are never stored in GAML. Capability-derived data inherits the privacy, ownership, retention, replication, training, and export restrictions of its source unless an explicit policy produces a more restrictive result.

---

## **28.13 Connector Lifecycle**

```text
Discover
  → Inspect
  → Translate
  → Validate
  → Test
  → Approve
  → Register
  → Monitor
  → Re-score
  → Update, Deprecate, or Revoke
```

Manifest changes must trigger drift detection and contract revalidation. Material permission, data-access, endpoint, provider, or execution-profile changes require renewed approval according to deployment policy.

---

## **28.14 Initial Implementation Requirements**

A first implementation should include:

1. Canonical schemas for Capability, Connector, CapabilityProvider, Permission, CapabilityContract, and CredentialReference.
2. A connector registry and capability discovery service.
3. An MCP adapter that translates discovered tools into proposed capability contracts.
4. Deterministic contract validators and contract hashing.
5. Tool Intermediary integration for all capability execution.
6. A local read-only connector, such as local files, notes, calendar, or email.
7. A remote read-only connector using MCP or OpenAPI.
8. Provider and connector health and reputation tracking.
9. Manifest drift detection and revocation.
10. Administrative review for newly discovered or materially changed capabilities.

---

## **28.15 Design Principle**

GCS should speak many protocols externally while using one governed capability model internally.

```text
Protocols and providers are adapters.
Capabilities are the canonical operations.
Policies grant authority.
The Tool Intermediary enforces execution.
GAML preserves durable knowledge and provenance.
```

This keeps the architecture compatible with MCP and other ecosystems without making any external protocol the security or authority boundary of GeniusCognitiveSystem.
