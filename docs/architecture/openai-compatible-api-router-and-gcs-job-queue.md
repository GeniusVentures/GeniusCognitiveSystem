# 18 OpenAI-Compatible API Router and GCS Job Queue Architecture

## 18.1 Product Technical Design Specification

This document specifies an OpenAI-compatible API router for the Genius Cognitive System (GCS). The feature allows existing OpenAI-compatible clients, SDKs, agents, IDE integrations, and enterprise applications to submit API requests through a Cloudflare-fronted endpoint while the actual work is executed by the GNUS.ai peer-to-peer cognitive network.

The core design rule is:

> The API router does not schedule inference directly. It translates OpenAI-compatible calls into signed GNUS.ai democratized queue jobs.

This keeps the public developer surface familiar while preserving the GNUS.ai distributed execution model underneath.

The design extends the existing processing task queue with a new higher-level job class for API request/response orchestration. This job class is intentionally different from strict AI processing chunk jobs. Existing processing chunk jobs remain the low-level unit of compute. API request jobs become the higher-level unit that owns external client lifecycle, OpenAI API compatibility, streaming, authentication, policy, metering, orchestration, and result packaging.

---

## 18.2 Background and Current Queue Context

The current GNUS/SuperGenius queue implementation is a CRDT-backed processing queue built around `SGProcessing::Task`, `SGProcessing::SubTask`, locks, completion records, and task results.

The current queue behavior can be summarized as:

- `EnqueueTask(task, subTasks)` stores subtasks and the parent task into GlobalDB through an atomic transaction.
- Subtasks are stored under the subtask list key space.
- Parent tasks are stored under the task list key space.
- `GrabTask()` queries the task list, skips blacklisted jobs, skips completed jobs, skips locked jobs, locks the first available task, and returns it to the worker.
- If no unlocked task is available, `GrabTask()` checks locked tasks and may move an expired lock.
- `CompleteTask(taskKey, taskResult)` stores a task result under the results key space.
- `IsTaskCompleted(taskId)` checks for an existing result record.
- `LockTask(taskKey)` writes a task lock into GlobalDB and publishes it over the processing topic.
- `MoveExpiredTaskLock(taskKey, task)` allows a timed-out locked task to be reclaimed.
- `MarkTaskBad(taskKey)` blacklists a bad task locally so the worker will not keep retrying broken jobs.

That design is good for democratized distributed work pickup. However, OpenAI-compatible API requests have lifecycle needs that do not fit cleanly into the current strict AI processing chunk model:

- HTTP request/response lifecycle
- streaming Server-Sent Events
- client disconnect handling
- API key and tenant/project authorization
- request-level policy and privacy envelope
- OpenAI-compatible error format
- request usage accounting
- node capability matching
- user-visible model aliases
- long-running orchestration
- child task creation
- partial result streaming
- retry/requeue semantics that preserve the external API contract

Therefore, the queue should support a new higher-level job type: the **GCS API Request Job**.

---

## 18.3 Goals

### 18.3.1 Primary goals

- Provide an OpenAI-compatible API surface for GCS.
- Allow existing OpenAI SDK users to switch to GNUS.ai by changing `base_url` and API key.
- Convert API calls into signed GCS jobs.
- Allow online GNUS/GCS nodes to register capabilities through pub/sub.
- Allow eligible nodes to pick up API jobs using the GNUS.ai democratized queue mechanics.
- Keep Cloudflare/API ingress lightweight.
- Keep actual execution inside the p2p GNUS.ai system.
- Add a higher-level queue job type for request/response orchestration.
- Preserve existing processing chunk behavior for lower-level compute tasks.
- Support public, private, local-only, and hybrid routing policies.
- Support streaming and blocking responses.
- Support metering, billing, reward, and settlement hooks.

### 18.3.2 Developer experience goals

A developer should be able to use a standard OpenAI SDK:

```bash
OPENAI_BASE_URL=https://api.gnus.ai/v1
OPENAI_API_KEY=gnus_xxx
```

Then call:

```js
import OpenAI from "openai";

const client = new OpenAI({
  apiKey: process.env.GNUS_API_KEY,
  baseURL: "https://api.gnus.ai/v1"
});

const response = await client.chat.completions.create({
  model: "gnus-auto",
  messages: [
    { role: "user", content: "Explain GNUS.ai in one paragraph." }
  ],
  stream: true
});
```

The client should receive normal OpenAI-compatible responses while GCS handles distributed routing and execution underneath.

---

## 18.4 Non-goals for MVP

MVP should not attempt to implement every OpenAI API or every future GCS orchestration mode.

MVP does not need:

- Assistants API compatibility.
- Fine-tuning API compatibility.
- Image generation.
- Realtime audio.
- Tool-call execution against arbitrary external systems.
- Full distributed token-by-token decoding.
- Multi-node speculative decoding.
- Global consensus for every API response.
- Perfect pricing prediction before execution.
- Full zero-knowledge verification of all model output.
- Public marketplace bidding beyond first valid claim / democratized queue pickup.
- Automatic private data routing without explicit tenant policy.

---

## 18.5 Core Design Principle

The API layer should not become a centralized inference scheduler.

Instead:

1. API clients send OpenAI-compatible HTTP requests.
2. Cloudflare authenticates and rate-limits the request.
3. The API router normalizes the request.
4. The router creates a signed `GCS_API_REQUEST` job.
5. The job is published into the GNUS.ai pub/sub and CRDT-backed queue system.
6. Online nodes that have registered matching capabilities can claim the job.
7. The winning node or queue-selected node executes the API request job.
8. The API request job may create one or more lower-level processing jobs.
9. Results are published back to a result channel.
10. The API router converts results into OpenAI-compatible JSON or SSE chunks.

The router is an adapter, not the brain.

---

## 18.6 Job Type Split

### 18.6.1 Existing job type: processing chunk job

The existing processing queue should continue to support strict AI processing chunk jobs.

A processing chunk job is a low-level compute unit.

Typical examples:

- process model chunk
- run embedding operation
- perform vector search over assigned shard
- execute inference on a specific model
- verify output hash
- process a specific IPFS block
- process a specific subtask from `SGProcessing::Task`

Properties:

- optimized for distributed compute
- can be retried or reclaimed through task locks
- may not map directly to an external user request
- generally has no direct HTTP client lifecycle
- generally stores result through `TaskResult`
- may be one of many child jobs under a larger request

### 18.6.2 New job type: API request job

A GCS API request job is a higher-level orchestration job.

Typical examples:

- OpenAI-compatible chat completion request
- OpenAI-compatible streaming chat request
- embedding request
- RAG request
- code-specialist request
- enterprise private-node request
- gateway-level request that decomposes into child processing jobs

Properties:

- maps directly to one external API request
- owns request deadline and client lifecycle
- owns response format
- owns streaming state
- owns request policy
- owns metering and billing envelope
- may create zero, one, or many processing chunk jobs
- may return a result without child chunks if one node can execute directly
- may be routed through public, private, hybrid, or local-only queues
- is signed by the API gateway or trusted tenant ingress
- records audit metadata and request provenance

### 18.6.3 Why this split matters

Do not force OpenAI-compatible HTTP semantics into processing chunks.

A chat completion request is not just an AI chunk. It is a customer-facing transaction with:

- external timeout expectations
- streaming expectations
- API compatibility rules
- safety/policy context
- tenant/account identity
- model alias mapping
- cost ceiling
- privacy policy
- result formatting
- billing records
- retry and requeue semantics
- child task fan-out

The correct model is:

```text
API Request Job
    owns the external request contract
    may create Processing Chunk Jobs
        each processing chunk owns distributed compute
```

---

## 18.7 Architecture Overview

```text
OpenAI-Compatible Client
      |
      | HTTPS /v1/chat/completions
      v
Cloudflare Edge
      |
      | TLS, WAF, auth precheck, rate limits
      v
GCS API Router
      |
      | Normalize OpenAI request
      | Attach tenant/project/policy
      | Create signed API request job
      v
GCS Gateway Node
      |
      | Publish job to pub/sub + CRDT queue
      | Subscribe to result/stream channels
      v
GNUS.ai Democratized Queue
      |
      | Online nodes observe available jobs
      | Nodes claim jobs based on capability and policy
      v
GCS Worker / Router / Planner / ELM Nodes
      |
      | Execute directly or decompose into processing chunk jobs
      v
Processing Task Queue
      |
      | Existing low-level AI processing chunks
      v
Aggregator / Result Publisher
      |
      | Final result, usage, attestations, stream chunks
      v
API Router
      |
      | Convert to OpenAI-compatible JSON/SSE
      v
Client
```

---

## 18.8 Components

### 18.8.1 Cloudflare Edge

Cloudflare provides the public HTTP edge.

Responsibilities:

- TLS termination
- WAF and abuse controls
- IP and tenant rate limits
- API key pre-validation
- request body size limits
- streaming HTTP support
- geolocation-aware routing to nearest GNUS API gateway
- request ID assignment
- emergency kill-switch rules
- optional bot mitigation

Cloudflare should not maintain long-lived libP2P participation. It should forward valid requests to a GCS API Router or Gateway service that can maintain p2p network connections.

### 18.8.2 GCS API Router

The API Router speaks OpenAI-compatible HTTP externally and GCS job protocol internally.

Responsibilities:

- implement `/v1/models`
- implement `/v1/chat/completions`
- implement `/v1/completions` if required for older clients
- implement `/v1/embeddings`
- validate request shape
- normalize model aliases
- attach tenant/project identity
- attach policy envelope
- estimate token/cost limits
- create signed `GCS_API_REQUEST` job envelope
- publish to GCS Gateway
- subscribe to result/stream channels
- convert internal errors to OpenAI-compatible errors
- convert internal chunks to OpenAI-compatible SSE chunks
- record API-level usage
- handle client disconnect and cancellation

### 18.8.3 GCS Gateway Node

The GCS Gateway Node bridges the HTTP/API world into the GNUS.ai p2p world.

Responsibilities:

- maintain libP2P connections
- publish API request jobs to pub/sub
- write API jobs into CRDT-backed queue state when durable queueing is required
- subscribe to result and stream channels
- verify node registrations
- verify job claims
- verify worker result signatures
- handle requeue if worker fails
- emit metering events
- expose job status back to API Router
- optionally act as aggregator for MVP

### 18.8.4 Online GCS Worker Nodes

Worker nodes register their availability and capabilities.

Responsibilities:

- heartbeat on capability channels
- subscribe to relevant job channels
- evaluate job requirements
- claim eligible jobs
- execute API request jobs directly or through child processing jobs
- publish stream chunks
- publish final result
- sign claims and results
- report usage and execution metrics

### 18.8.5 Router / Planner Node

A Router / Planner node may execute the API request job if the request requires decomposition.

Responsibilities:

- classify request
- determine whether memory, RAG, ELMs, tools, or verification are needed
- choose execution topology
- create child processing jobs when needed
- combine child results
- return final answer or stream to aggregator

### 18.8.6 Aggregator Node

The aggregator receives partial results and produces a final response.

Responsibilities:

- merge partial outputs
- rank candidate outputs
- handle disagreement
- assemble final answer
- produce OpenAI-compatible choice structure
- generate usage summary
- sign final response metadata

For MVP, the API Gateway or first worker can act as aggregator.

---

## 18.9 Pub/Sub Channels

### 18.9.1 Capability registration channels

Nodes should register on capability channels.

Suggested channels:

```text
gcs.capabilities.all
gcs.capabilities.chat
gcs.capabilities.embedding
gcs.capabilities.rag
gcs.capabilities.code
gcs.capabilities.router
gcs.capabilities.aggregator
gcs.capabilities.private.<tenant_id>
```

### 18.9.2 API job channels

API request jobs should publish to API-specific channels.

Suggested channels:

```text
gcs.api.jobs.all
gcs.api.jobs.chat
gcs.api.jobs.embedding
gcs.api.jobs.rag
gcs.api.jobs.code
gcs.api.jobs.private.<tenant_id>
gcs.api.jobs.local.<swarm_id>
```

### 18.9.3 Processing chunk channels

Existing processing jobs can continue to use existing processing topics.

Optional future split:

```text
gcs.processing.jobs.all
gcs.processing.jobs.model
gcs.processing.jobs.vector
gcs.processing.jobs.verify
gcs.processing.jobs.ipfs
```

### 18.9.4 Result channels

Each API job should receive a unique result channel.

```text
gcs.api.results.<job_id>
```

### 18.9.5 Stream channels

Streaming jobs should receive a unique stream channel.

```text
gcs.api.stream.<job_id>
```

### 18.9.6 Claim channels

Claims may be published to a job-specific claim channel or written as CRDT claim records.

```text
gcs.api.claims.<job_id>
```

---

## 18.10 Node Registration

### 18.10.1 Registration envelope

A node registration is a signed, short-lived capability advertisement.

```json
{
  "message_type": "GCS_NODE_REGISTRATION",
  "version": 1,
  "node_id": "gnusnode_abc",
  "public_key": "0x...",
  "swarm_id": "public",
  "tenant_id": null,
  "status": "online",
  "capabilities": {
    "api_request_job": true,
    "processing_chunk_job": true,
    "chat": true,
    "completion": true,
    "embedding": true,
    "rag": true,
    "code": false,
    "streaming": true,
    "router_planner": false,
    "aggregator": false
  },
  "models": [
    {
      "model_id": "gnus-small",
      "aliases": ["gnus-auto"],
      "context_tokens": 32768,
      "quantization": "q4",
      "backend": "mnn",
      "tasks": ["chat", "completion"]
    }
  ],
  "hardware": {
    "cpu_threads": 16,
    "ram_mb": 32768,
    "gpu": "Apple M2 Ultra",
    "vram_mb": 196608,
    "backends": ["mlx", "mnn", "ggml", "vulkan"]
  },
  "queue": {
    "max_concurrent_api_jobs": 2,
    "max_concurrent_processing_jobs": 8,
    "current_api_jobs": 0,
    "current_processing_jobs": 0,
    "accepts_public_jobs": true,
    "accepts_private_jobs": false,
    "accepts_local_only_jobs": false
  },
  "trust": {
    "reputation_score": 0.98,
    "trust_tier": "C",
    "attestation_modes": ["signed_result"]
  },
  "heartbeat": {
    "created_at_ms": 1783468800000,
    "expires_at_ms": 1783468830000,
    "ttl_ms": 30000
  },
  "signature": "..."
}
```

### 18.10.2 Heartbeat rules

- Registrations are short-lived.
- Nodes must refresh before expiration.
- Stale registrations must be ignored.
- Job claims from stale nodes must be rejected.
- Heartbeat interval should be significantly shorter than expiration.
- MVP target: heartbeat every 10 seconds, expiration after 30 seconds.
- Production values should be configurable per network and tenant.

---

## 18.11 API Request Job Envelope

### 18.11.1 Required fields

```json
{
  "message_type": "GCS_API_REQUEST_JOB",
  "version": 1,
  "job_id": "gcsapi_01J...",
  "idempotency_key": "req_hash_or_client_key",
  "source": {
    "kind": "openai_compatible_api",
    "endpoint": "/v1/chat/completions",
    "method": "POST",
    "client_request_id": "req_abc"
  },
  "tenant": {
    "tenant_id": "tenant_123",
    "project_id": "proj_456",
    "api_key_id": "key_789"
  },
  "request": {
    "api_type": "chat.completion",
    "model": "gnus-auto",
    "stream": true,
    "payload_ref": {
      "mode": "inline",
      "encrypted": false,
      "content_hash": "0x..."
    }
  },
  "routing": {
    "network": "public",
    "swarm_id": "public",
    "privacy_mode": "standard",
    "claim_policy": "first_valid_claim",
    "requires_router_planner": false,
    "requires_aggregator": false,
    "replication_factor": 1,
    "verification_mode": "none"
  },
  "requirements": {
    "min_context_tokens": 8192,
    "supports_streaming": true,
    "supports_chat": true,
    "supports_embeddings": false,
    "allowed_model_aliases": ["gnus-auto", "gnus-small"],
    "max_input_tokens": 16000,
    "max_output_tokens": 1024
  },
  "limits": {
    "claim_timeout_ms": 1000,
    "first_token_timeout_ms": 5000,
    "wall_timeout_ms": 30000,
    "max_cost_gnus": "auto",
    "max_requeues": 2
  },
  "reply_to": {
    "result_channel": "gcs.api.results.gcsapi_01J...",
    "stream_channel": "gcs.api.stream.gcsapi_01J...",
    "claim_channel": "gcs.api.claims.gcsapi_01J..."
  },
  "created_at_ms": 1783468800000,
  "expires_at_ms": 1783468830000,
  "signature": "..."
}
```

### 18.11.2 Payload storage modes

The job envelope should not always inline the prompt.

Supported modes:

```text
inline
encrypted_inline
crdt_ref
ipfs_cid
gateway_ref
tenant_private_ref
```

MVP can use `inline` for public test traffic and `gateway_ref` for larger bodies.

Production should support encrypted payload references so job discovery does not leak sensitive prompts.

### 18.11.3 Routing modes

Supported routing modes:

```text
public
private
hybrid
local_only
gateway_local
```

Definitions:

- `public`: route to public GNUS nodes.
- `private`: route only to tenant-approved private nodes.
- `hybrid`: try private nodes first, then public nodes if policy allows.
- `local_only`: route only inside local swarm.
- `gateway_local`: execute only on the gateway or directly attached local node.

---

## 18.12 Claim and Lock Semantics

### 18.12.1 First valid claim MVP

MVP should use a simple policy:

```text
first valid claim wins
```

A claim is valid if:

- job exists
- job has not expired
- node registration is fresh
- node supports the requested API job type
- node supports the requested model or alias
- node supports streaming when required
- node satisfies minimum context size
- node is allowed under privacy/routing policy
- node has capacity
- node signature verifies
- job has not already been claimed or completed

### 18.12.2 Claim envelope

```json
{
  "message_type": "GCS_API_JOB_CLAIM",
  "version": 1,
  "job_id": "gcsapi_01J...",
  "claim_id": "claim_01J...",
  "node_id": "gnusnode_abc",
  "claim_policy": "first_valid_claim",
  "capability_match": {
    "model_id": "gnus-small",
    "supports_streaming": true,
    "context_tokens": 32768,
    "api_request_job": true
  },
  "estimated": {
    "start_ms": 50,
    "first_token_ms": 800,
    "wall_ms": 7000
  },
  "created_at_ms": 1783468800100,
  "expires_at_ms": 1783468801100,
  "signature": "..."
}
```

### 18.12.3 Claim acceptance

The gateway or queue layer accepts the first valid claim and writes a job lock/lease.

The lock should include:

```json
{
  "job_id": "gcsapi_01J...",
  "claim_id": "claim_01J...",
  "node_id": "gnusnode_abc",
  "lease_started_at_ms": 1783468800100,
  "lease_expires_at_ms": 1783468830100,
  "renewable": true,
  "signature": "..."
}
```

### 18.12.4 Lease renewal

API request jobs may live longer than the existing short processing lock timeout. Therefore API job locks should be leases, not just one-shot locks.

Rules:

- worker must renew lease while executing
- gateway should requeue if lease expires
- stream chunks may count as progress but should not replace lease renewal
- final result closes lease
- cancellation revokes lease

### 18.12.5 Relationship to existing task locks

Existing `LockTask()` behavior can remain for processing chunks.

API jobs should use a new lock namespace to avoid confusing:

- API request leases
- processing chunk locks

Proposed namespaces:

```text
/api/jobs/<job_id>
/api/claims/<job_id>/<claim_id>
/api/locks/<job_id>
/api/results/<job_id>
/api/streams/<job_id>/<sequence>

/processing/tasks/<task_id>
/processing/subtasks/<task_id>/<subtask_id>
/processing/locks/<task_key>
/processing/results/<task_id>
```

---

## 18.13 API Job Lifecycle

### 18.13.1 States

```text
created
published
claiming
claimed
accepted
executing
streaming
child_jobs_created
waiting_child_jobs
aggregating
completed
failed
cancelled
expired
requeued
```

### 18.13.2 Lifecycle flow

```text
1. API request received.
2. Router validates request.
3. Router creates API request job.
4. Gateway publishes job.
5. Nodes observe job.
6. Qualified node claims job.
7. Gateway accepts first valid claim.
8. Worker starts execution.
9. Worker either:
   a. executes directly, or
   b. creates child processing jobs.
10. Worker publishes stream chunks if streaming.
11. Worker publishes final result.
12. Gateway verifies final result.
13. Router sends OpenAI-compatible response.
14. Usage and settlement records are emitted.
```

### 18.13.3 Cancellation flow

Cancellation can occur when:

- client disconnects
- API timeout is reached
- tenant quota is exceeded mid-flight
- admin cancels job
- gateway detects invalid worker behavior

Flow:

```text
1. Gateway marks API job cancelled.
2. Gateway publishes cancellation message.
3. Worker stops generation if possible.
4. Child processing jobs are cancelled or orphan-marked.
5. Partial usage is recorded.
6. No final client response is required if client disconnected.
```

---

## 18.14 Child Processing Jobs

### 18.14.1 When to create child jobs

An API request job may create child processing jobs when:

- RAG retrieval requires distributed vector search
- multiple ELMs are needed
- verification or arbitration is needed
- model inference must be chunked
- embeddings need distributed batch processing
- response requires code specialist plus general synthesis
- private and public hybrid execution is needed
- memory hydration requires multiple shards

### 18.14.2 Child job reference

Each child processing job should reference the parent API job.

```json
{
  "parent_job": {
    "job_type": "GCS_API_REQUEST_JOB",
    "job_id": "gcsapi_01J...",
    "phase": "retrieval"
  },
  "processing_task": {
    "task_id": "task_abc",
    "subtask_id": "subtask_123"
  }
}
```

### 18.14.3 Parent/child result aggregation

The parent API job is not complete until:

- all required child jobs complete,
- enough child jobs complete to satisfy quorum,
- timeout policy allows partial result,
- or failure policy aborts the request.

Aggregation can happen at:

- claiming worker
- router/planner node
- aggregator node
- gateway node for MVP

---

## 18.15 OpenAI-Compatible API Surface

### 18.15.1 `/v1/models`

Returns available GNUS model aliases.

Example response:

```json
{
  "object": "list",
  "data": [
    {
      "id": "gnus-auto",
      "object": "model",
      "owned_by": "gnus.ai"
    },
    {
      "id": "gnus-small",
      "object": "model",
      "owned_by": "gnus.ai"
    },
    {
      "id": "gnus-rag",
      "object": "model",
      "owned_by": "gnus.ai"
    },
    {
      "id": "gnus-code",
      "object": "model",
      "owned_by": "gnus.ai"
    },
    {
      "id": "gnus-embed",
      "object": "model",
      "owned_by": "gnus.ai"
    }
  ]
}
```

### 18.15.2 `/v1/chat/completions`

MVP supported fields:

```json
{
  "model": "gnus-auto",
  "messages": [],
  "temperature": 0.7,
  "top_p": 1,
  "max_tokens": 1024,
  "stream": true
}
```

MVP should tolerate unsupported OpenAI fields by ignoring them unless strict compatibility mode is enabled.

### 18.15.3 `/v1/embeddings`

Embeddings route to embedding-capable nodes.

MVP supported fields:

```json
{
  "model": "gnus-embed",
  "input": "text or array of texts"
}
```

### 18.15.4 GNUS extension object

Advanced clients may include a `gnus` object.

```json
{
  "model": "gnus-auto",
  "messages": [
    {
      "role": "user",
      "content": "Summarize these docs."
    }
  ],
  "stream": true,
  "gnus": {
    "network": "hybrid",
    "privacy_mode": "enterprise",
    "routing": {
      "claim_policy": "first_valid_claim",
      "replication_factor": 1,
      "verification_mode": "none"
    },
    "retrieval": {
      "collections": ["company_docs", "website_docs"],
      "max_chunks": 12
    },
    "limits": {
      "max_cost_gnus": "auto",
      "wall_timeout_ms": 30000
    }
  }
}
```

OpenAI clients that do not use this object remain compatible.

---

## 18.16 Streaming

### 18.16.1 Internal stream chunk

```json
{
  "message_type": "GCS_API_STREAM_CHUNK",
  "version": 1,
  "job_id": "gcsapi_01J...",
  "node_id": "gnusnode_abc",
  "sequence": 12,
  "delta": {
    "role": null,
    "content": "distributed"
  },
  "usage_delta": {
    "output_tokens": 1
  },
  "finish_reason": null,
  "created_at_ms": 1783468801200,
  "signature": "..."
}
```

### 18.16.2 External OpenAI-compatible chunk

```text
data: {
  "id": "chatcmpl_gnus_123",
  "object": "chat.completion.chunk",
  "created": 1783468801,
  "model": "gnus-auto",
  "choices": [
    {
      "index": 0,
      "delta": {
        "content": "distributed"
      },
      "finish_reason": null
    }
  ]
}
```

Final event:

```text
data: [DONE]
```

### 18.16.3 Stream ordering

- chunks must include monotonic sequence numbers
- duplicate sequence numbers should be ignored
- missing sequence numbers should trigger a short reorder buffer
- final result should include total usage
- worker signature should cover job ID, node ID, sequence, delta hash, and timestamp

---

## 18.17 Result Envelope

### 18.17.1 Final result

```json
{
  "message_type": "GCS_API_FINAL_RESULT",
  "version": 1,
  "job_id": "gcsapi_01J...",
  "node_id": "gnusnode_abc",
  "status": "completed",
  "response": {
    "format": "openai.chat.completion",
    "body": {
      "id": "chatcmpl_gnus_123",
      "object": "chat.completion",
      "created": 1783468801,
      "model": "gnus-auto",
      "choices": [
        {
          "index": 0,
          "message": {
            "role": "assistant",
            "content": "..."
          },
          "finish_reason": "stop"
        }
      ],
      "usage": {
        "prompt_tokens": 100,
        "completion_tokens": 50,
        "total_tokens": 150
      }
    }
  },
  "metering": {
    "input_tokens": 100,
    "output_tokens": 50,
    "compute_ms": 1840,
    "child_jobs": 0,
    "cost_gnus": "0.0031"
  },
  "attestation": {
    "claim_id": "claim_01J...",
    "registration_id": "reg_01J...",
    "result_hash": "0x..."
  },
  "created_at_ms": 1783468807000,
  "signature": "..."
}
```

### 18.17.2 Error result

```json
{
  "message_type": "GCS_API_FINAL_RESULT",
  "version": 1,
  "job_id": "gcsapi_01J...",
  "status": "failed",
  "error": {
    "type": "service_unavailable",
    "code": "gnus_no_capable_nodes",
    "message": "No GNUS nodes are currently available for the requested model and routing policy."
  },
  "created_at_ms": 1783468807000,
  "signature": "..."
}
```

External OpenAI-compatible error:

```json
{
  "error": {
    "message": "No GNUS nodes are currently available for the requested model and routing policy.",
    "type": "service_unavailable",
    "code": "gnus_no_capable_nodes"
  }
}
```

---

## 18.18 Queue Fairness and Democratized Pickup

### 18.18.1 MVP policy

MVP uses:

```text
first valid claim wins
```

This matches the simplest form of democratized pickup and is easy to reason about.

### 18.18.2 Later policies

Future queue policies may include:

```text
first_valid_claim
weighted_fair_claim
reputation_weighted_claim
stake_weighted_claim
price_weighted_claim
latency_weighted_claim
tenant_preferred_nodes
private_pool_round_robin
verification_required_multi_claim
```

### 18.18.3 Fairness state

The queue should eventually track:

- node wins per time window
- node failures
- timeout count
- invalid claim count
- average first token latency
- average completion latency
- successful jobs by class
- earned rewards
- tenant preference
- hardware class utilization

This allows democratized routing without letting the fastest spammer always win every job.

---

## 18.19 Requeue and Retry

### 18.19.1 Requeue reasons

An API job may be requeued when:

- no valid claim arrives before claim timeout
- winning worker does not acknowledge
- worker lease expires
- worker fails before first token
- worker disconnects
- worker returns invalid result
- worker violates policy
- child processing jobs fail
- stream stalls beyond timeout
- result signature fails verification

### 18.19.2 Requeue envelope

```json
{
  "message_type": "GCS_API_JOB_REQUEUE",
  "version": 1,
  "job_id": "gcsapi_01J...",
  "requeue_count": 1,
  "reason": "worker_timeout",
  "excluded_nodes": ["gnusnode_abc"],
  "remaining_wall_timeout_ms": 18000,
  "created_at_ms": 1783468810000,
  "signature": "..."
}
```

### 18.19.3 Requeue limits

Each API request job should include:

- maximum requeue count
- total wall-clock timeout
- first-token timeout
- result timeout
- excluded failed nodes
- partial result policy

---

## 18.20 Security and Privacy

### 18.20.1 API key security

- API keys must be hashed at rest.
- API keys must be scoped to tenant/project.
- API keys may restrict models, routing modes, and spend.
- API keys may restrict public network usage.
- API keys may require private-only execution.

### 18.20.2 Job signature requirements

These messages must be signed:

- node registration
- API request job
- job claim
- job lock/lease
- lease renewal
- stream chunk
- final result
- usage record
- cancellation
- requeue

### 18.20.3 Prompt privacy

Supported privacy levels:

```text
standard
encrypted_payload
private_pool
local_only
metadata_minimized
```

MVP can start with `standard` and `private_pool`.

Production should support encrypted payloads and metadata-minimized job publication.

### 18.20.4 Public queue leakage

Public job channels must not leak sensitive prompts by default.

For sensitive jobs:

- publish only opaque job handle
- publish only capability requirements
- encrypt payload
- restrict allowed nodes
- use tenant private channel
- avoid exposing collection names when possible

---

## 18.21 Metering, Rewards, and Settlement

### 18.21.1 Usage record

```json
{
  "message_type": "GCS_API_USAGE_RECORD",
  "version": 1,
  "job_id": "gcsapi_01J...",
  "tenant_id": "tenant_123",
  "project_id": "proj_456",
  "api_key_id": "key_789",
  "node_id": "gnusnode_abc",
  "model": "gnus-auto",
  "api_type": "chat.completion",
  "network": "public",
  "input_tokens": 812,
  "output_tokens": 266,
  "embedding_tokens": 0,
  "retrieval_chunks": 0,
  "child_jobs": 0,
  "compute_ms": 1840,
  "first_token_ms": 800,
  "wall_ms": 7000,
  "cost_gnus": "0.0031",
  "created_at_ms": 1783468807000,
  "signature": "..."
}
```

### 18.21.2 Settlement hooks

The usage record should feed:

- tenant billing
- public node reward
- private enterprise accounting
- developer dashboard
- abuse analytics
- reputation updates
- token settlement or burn/buyback logic where applicable

---

## 18.22 Data Model Changes

### 18.22.1 New protobuf/message concepts

Recommended new messages:

```text
GCSApiRequestJob
GCSNodeRegistration
GCSApiJobClaim
GCSApiJobLease
GCSApiLeaseRenewal
GCSApiStreamChunk
GCSApiFinalResult
GCSApiUsageRecord
GCSApiCancellation
GCSApiRequeue
```

### 18.22.2 Existing processing messages remain

Existing concepts remain:

```text
SGProcessing::Task
SGProcessing::SubTask
SGProcessing::TaskLock
SGProcessing::TaskResult
```

### 18.22.3 Optional unified queue wrapper

A future unified queue wrapper could use a `oneof` style model:

```proto
message GCSQueueJob {
  string job_id = 1;
  GCSJobKind kind = 2;

  oneof body {
    SGProcessingTask processing_task = 10;
    GCSApiRequestJob api_request_job = 11;
    GCSMaintenanceJob maintenance_job = 12;
    GCSRetrainingJob retraining_job = 13;
  }
}
```

Job kinds:

```proto
enum GCSJobKind {
  GCS_JOB_KIND_UNSPECIFIED = 0;
  GCS_JOB_KIND_PROCESSING_CHUNK = 1;
  GCS_JOB_KIND_API_REQUEST = 2;
  GCS_JOB_KIND_ROUTER_PLAN = 3;
  GCS_JOB_KIND_RETRIEVAL = 4;
  GCS_JOB_KIND_EMBEDDING = 5;
  GCS_JOB_KIND_VERIFICATION = 6;
  GCS_JOB_KIND_AGGREGATION = 7;
  GCS_JOB_KIND_MAINTENANCE = 8;
  GCS_JOB_KIND_RETRAINING = 9;
}
```

---

## 18.23 CRDT Keyspace Proposal

To avoid breaking the existing processing queue, use a parallel keyspace.

```text
/gcs/api/jobs/<job_id>
/gcs/api/jobs_by_tenant/<tenant_id>/<job_id>
/gcs/api/claims/<job_id>/<claim_id>
/gcs/api/leases/<job_id>
/gcs/api/results/<job_id>
/gcs/api/streams/<job_id>/<sequence>
/gcs/api/usage/<job_id>
/gcs/api/cancel/<job_id>
/gcs/api/requeue/<job_id>/<attempt>

/gcs/nodes/registrations/<node_id>
/gcs/nodes/capabilities/<capability>/<node_id>
/gcs/nodes/heartbeat/<node_id>
```

Existing processing queue keyspace remains separate.

---

## 18.24 MVP Implementation Plan

### Phase 1: API compatibility shell

Deliver:

- Cloudflare route for `api.gnus.ai`
- `/v1/models`
- `/v1/chat/completions`
- blocking response
- streaming SSE response
- API key auth
- tenant/project lookup
- static model aliases
- basic usage logging

Execution can initially route to one controlled GCS node.

### Phase 2: API request job schema

Deliver:

- `GCSApiRequestJob`
- signed job envelope
- job ID and idempotency key
- result channel
- stream channel
- claim channel
- OpenAI payload normalization
- OpenAI error mapping

### Phase 3: GCS gateway bridge

Deliver:

- Gateway service that maintains p2p connectivity
- publish API jobs to pub/sub
- subscribe to result/stream channels
- first valid claim handling
- lease and timeout handling
- cancellation on client disconnect

### Phase 4: Node registration and claim

Deliver:

- node registration message
- heartbeat
- capability channels
- job claim message
- claim validation
- API job lease
- stale node rejection

### Phase 5: Direct worker execution

Deliver:

- worker can claim chat completion job
- worker can execute local model
- worker can stream chunks
- worker can publish final result
- gateway converts to OpenAI-compatible response

### Phase 6: Child processing jobs

Deliver:

- API job can create child `SGProcessing::Task` jobs
- parent job tracks child job IDs
- existing processing queue executes children
- parent aggregates child results
- parent publishes final result

### Phase 7: Metering and settlement

Deliver:

- usage record
- signed metering event
- node reward hook
- tenant billing hook
- dashboard data

### Phase 8: Private and hybrid routing

Deliver:

- tenant private channels
- local-only routing
- hybrid fallback
- private node allowlist
- encrypted payload reference support

---

## 18.25 Acceptance Criteria

MVP is complete when:

- A standard OpenAI SDK can call `https://api.gnus.ai/v1/chat/completions`.
- The request is accepted through Cloudflare.
- API key authentication works.
- The API router converts the request into a signed API request job.
- The gateway publishes the job to the GCS network.
- An online registered node can claim the job.
- First valid claim wins for MVP.
- Claimed jobs receive leases.
- Failed jobs can be requeued.
- Streaming chunks are converted to OpenAI-compatible SSE.
- Final result is returned in OpenAI-compatible JSON.
- Usage is recorded per tenant/project/API key/node.
- Existing processing chunk jobs still work independently.
- API request jobs can create child processing jobs in a later MVP phase.
- Public/private/local routing policy is represented in the job envelope.

---

## 18.26 Open Questions

- Should API job leases reuse any existing `TaskLock` behavior internally, or should API jobs use a fully separate lease type from day one?
- Should the first API gateway act as the only claim validator during MVP, or should claim acceptance be CRDT-consensus visible immediately?
- Should model aliases be global, tenant-specific, or both?
- Should `gpt-4o`-style compatibility aliases be allowed, or should GNUS only expose `gnus-*` names?
- Should public API jobs allow inline prompt payloads during MVP, or should every API job use a gateway payload reference from day one?
- What is the minimum useful settlement record for a public node reward?
- Should the first implementation live in the GCS repo, SuperGenius repo, or a separate API gateway repo?
- Should `GCS_API_REQUEST_JOB` be protobuf-first, JSON-first, or dual-format during early integration?

---

## 18.27 Recommended Initial File Placement

Recommended path:

```text
docs/architecture/openai-compatible-api-router-and-gcs-job-queue.md
```

Recommended index title:

```text
18 OpenAI-Compatible API Router and GCS Job Queue Architecture
```

This document should sit beside `secure-agent-architecture.md` because it defines the external API ingress and queue mechanics that can feed the secure agent architecture, router/planner layer, ELM execution, memory services, verification services, and settlement/reputation layers.

---

## 18.28 Summary

This feature gives GNUS.ai a simple developer-facing wedge:

> Change the OpenAI base URL. GNUS.ai turns the request into a distributed p2p job.

Under the hood, this requires a clean queue distinction:

- **Processing chunk jobs** remain the low-level distributed compute units.
- **API request jobs** become the high-level request/response orchestration units.

That split lets GNUS.ai preserve its democratized queue and p2p architecture while exposing a boring, familiar, OpenAI-compatible API to the outside world.
