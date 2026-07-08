# 24 OpenAI-Compatible API Router and GCS Job Queue Architecture

## 24.1 Product Technical Design Specification

This document specifies an OpenAI-compatible API router for the Genius Cognitive System (GCS). The feature allows existing OpenAI-compatible clients, SDKs, agents, IDE integrations, websites, IDE plugins, enterprise applications, and automation systems to submit standard OpenAI-style API requests while the actual work is executed through the GNUS.ai peer-to-peer cognitive network.

The core design rule is:

> The API router does not schedule inference directly. It translates OpenAI-compatible calls into signed GNUS.ai democratized queue jobs.

This preserves the GNUS.ai distributed execution model while giving developers a familiar API surface. Developers should be able to switch existing tools by changing the API base URL and API key, without learning the p2p network internals.

This design extends the existing processing task queue with a higher-level job class for API request/response orchestration. That job class is intentionally different from strict AI processing chunk jobs. Existing processing chunk jobs remain the low-level unit of compute. API request jobs become the higher-level unit that owns external client lifecycle, OpenAI API compatibility, streaming, authentication, policy, metering, orchestration, and result packaging.

---

## 24.2 Background and Current Queue Context

The current GNUS/SuperGenius processing queue is a CRDT-backed task queue built around `SGProcessing::Task`, `SGProcessing::SubTask`, task locks, completion records, and task results.

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

That design is good for democratized distributed work pickup. However, OpenAI-compatible API requests have lifecycle requirements that do not fit cleanly into the current strict processing chunk model:

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

Therefore, the queue should support a higher-level **GCS API Request Job** type.

---

## 24.3 Goals

### 24.3.1 Primary Goals

- Provide an OpenAI-compatible API surface for GCS.
- Allow existing OpenAI SDK users to switch to GNUS.ai by changing `base_url` and API key.
- Convert API calls into signed GCS API request jobs.
- Allow online GNUS/GCS nodes to register capabilities through pub/sub.
- Allow eligible nodes to pick up API jobs using GNUS.ai democratized queue mechanics.
- Keep Cloudflare/API ingress lightweight.
- Keep actual execution inside the p2p GNUS.ai system.
- Add a higher-level queue job type for request/response orchestration.
- Preserve existing processing chunk behavior for lower-level compute tasks.
- Support public, private, local-only, gateway-local, and hybrid routing policies.
- Support streaming and blocking responses.
- Support metering, billing, reward, reputation, and settlement hooks.

### 24.3.2 Developer Experience Goals

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

## 24.4 Non-Goals for MVP

MVP should not attempt to implement every OpenAI API or every future GCS orchestration mode.

MVP does not need:

- Assistants API compatibility.
- Fine-tuning API compatibility.
- Image generation.
- Realtime audio.
- Arbitrary external tool execution.
- Full distributed token-by-token decoding.
- Multi-node speculative decoding.
- Global consensus for every API response.
- Perfect pricing prediction before execution.
- Full zero-knowledge verification of all model output.
- Public marketplace bidding beyond first valid claim / democratized queue pickup.
- Automatic private data routing without explicit tenant policy.

---

## 24.5 Core Design Principle

The API layer should not become a centralized inference scheduler.

Instead:

1. API clients send OpenAI-compatible HTTP requests.
2. Cloudflare authenticates and rate-limits the request.
3. The API router normalizes the request.
4. The router creates a signed `GCS_API_REQUEST_JOB`.
5. The job is published into the GNUS.ai pub/sub and CRDT-backed queue system.
6. Online nodes that have registered matching capabilities can claim the job.
7. The winning node or queue-selected node executes the API request job.
8. The API request job may create one or more lower-level processing jobs.
9. Results are published back to a result channel or stream channel.
10. The API router converts results into OpenAI-compatible JSON or SSE chunks.

The router is an adapter, not the brain.

---

## 24.6 Job Type Split

### 24.6.1 Existing Job Type: Processing Chunk Job

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

### 24.6.2 New Job Type: API Request Job

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
- may be routed through public, private, hybrid, local-only, or gateway-local queues
- is signed by the API gateway or trusted tenant ingress
- records audit metadata and request provenance

### 24.6.3 Why This Split Matters

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

## 24.7 Architecture Overview

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

## 24.8 Components

### 24.8.1 Cloudflare Edge

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

### 24.8.2 GCS API Router

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
- create signed `GCS_API_REQUEST_JOB` envelope
- publish to GCS Gateway
- subscribe to result/stream channels
- convert internal errors to OpenAI-compatible errors
- convert internal chunks to OpenAI-compatible SSE chunks
- record API-level usage
- handle client disconnect and cancellation

### 24.8.3 GCS Gateway Node

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

### 24.8.4 Online GCS Worker Nodes

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

### 24.8.5 Router / Planner Node

A Router / Planner node may execute the API request job if the request requires decomposition.

Responsibilities:

- classify request
- determine whether memory, RAG, ELMs, tools, or verification are needed
- choose execution topology
- create child processing jobs when needed
- combine child results
- return final answer or stream to aggregator

### 24.8.6 Aggregator Node

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

## 24.9 Pub/Sub Channels

### 24.9.1 Capability Registration Channels

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

### 24.9.2 API Job Channels

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

### 24.9.3 Processing Chunk Channels

Existing processing jobs can continue to use existing processing topics.

Optional future split:

```text
gcs.processing.jobs.all
gcs.processing.jobs.model
gcs.processing.jobs.vector
gcs.processing.jobs.verify
gcs.processing.jobs.ipfs
```

### 24.9.4 Result, Stream, and Claim Channels

Each API job should receive unique channels:

```text
gcs.api.results.<job_id>
gcs.api.stream.<job_id>
gcs.api.claims.<job_id>
```

---

## 24.10 Node Registration

### 24.10.1 Registration Envelope

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

### 24.10.2 Heartbeat Rules

- Registrations are short-lived.
- Nodes must refresh before expiration.
- Stale registrations must be ignored.
- Job claims from stale nodes must be rejected.
- Heartbeat interval should be significantly shorter than expiration.
- MVP target: heartbeat every 10 seconds, expiration after 30 seconds.
- Production values should be configurable per network and tenant.

---

## 24.11 API Request Job Envelope

### 24.11.1 Required Fields

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
    "first_token_timeout_ms": 10000,
    "idle_stream_timeout_ms": 30000,
    "wall_timeout_ms": 120000,
    "max_cost_gnus": "auto",
    "max_requeues": 2
  },
  "reply_to": {
    "result_channel": "gcs.api.results.gcsapi_01J...",
    "stream_channel": "gcs.api.stream.gcsapi_01J...",
    "claim_channel": "gcs.api.claims.gcsapi_01J..."
  },
  "created_at_ms": 1783468800000,
  "expires_at_ms": 1783468920000,
  "signature": "..."
}
```

### 24.11.2 Payload Storage Modes

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

MVP can use `inline` for public test traffic and `gateway_ref` for larger bodies. Production should support encrypted payload references so job discovery does not leak sensitive prompts.

### 24.11.3 Routing Modes

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

## 24.12 Claim, Lock, and Lease Semantics

### 24.12.1 First Valid Claim MVP

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

### 24.12.2 Claim Envelope

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

### 24.12.3 Lease Rules

API request jobs may live longer than short processing locks, especially for streaming. Therefore API job locks should be leases, not just one-shot locks.

Rules:

- worker must renew lease while executing
- gateway should requeue if lease expires
- stream chunks may count as progress but should not replace lease renewal
- final result closes lease
- cancellation revokes lease

Existing `LockTask()` behavior can remain for processing chunks. API jobs should use a separate lock namespace so API request leases and processing chunk locks are not confused.

Proposed namespaces:

```text
/gcs/api/jobs/<job_id>
/gcs/api/claims/<job_id>/<claim_id>
/gcs/api/leases/<job_id>
/gcs/api/results/<job_id>
/gcs/api/streams/<job_id>/<sequence>

/gcs/processing/tasks/<task_id>
/gcs/processing/subtasks/<task_id>/<subtask_id>
/gcs/processing/locks/<task_key>
/gcs/processing/results/<task_id>
```

---

## 24.13 API Job Lifecycle

### 24.13.1 States

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

### 24.13.2 Lifecycle Flow

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

### 24.13.3 Cancellation Flow

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

## 24.14 Child Processing Jobs

### 24.14.1 When to Create Child Jobs

An API request job may create child processing jobs when:

- RAG retrieval requires distributed vector search
- multiple ELMs are needed
- verification or arbitration is needed
- model inference must be chunked
- embeddings need distributed batch processing
- response requires code specialist plus general synthesis
- private and public hybrid execution is needed
- memory hydration requires multiple shards

### 24.14.2 Child Job Reference

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

The parent API job is not complete until all required child jobs complete, enough child jobs complete to satisfy quorum, timeout policy allows partial result, or failure policy aborts the request.

Aggregation can happen at the claiming worker, router/planner node, aggregator node, or gateway node for MVP.

---

## 24.15 OpenAI-Compatible API Surface

### 24.15.1 `/v1/models`

Returns available GNUS model aliases.

Example response:

```json
{
  "object": "list",
  "data": [
    { "id": "gnus-auto", "object": "model", "owned_by": "gnus.ai" },
    { "id": "gnus-small", "object": "model", "owned_by": "gnus.ai" },
    { "id": "gnus-rag", "object": "model", "owned_by": "gnus.ai" },
    { "id": "gnus-code", "object": "model", "owned_by": "gnus.ai" },
    { "id": "gnus-embed", "object": "model", "owned_by": "gnus.ai" }
  ]
}
```

### 24.15.2 `/v1/chat/completions`

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

### 24.15.3 `/v1/embeddings`

Embeddings route to embedding-capable nodes.

MVP supported fields:

```json
{
  "model": "gnus-embed",
  "input": "text or array of texts"
}
```

### 24.15.4 GNUS Extension Object

Advanced clients may include a `gnus` object.

```json
{
  "model": "gnus-auto",
  "messages": [
    { "role": "user", "content": "Summarize these docs." }
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
      "wall_timeout_ms": 120000
    }
  }
}
```

OpenAI clients that do not use this object remain compatible.

---

## 24.16 Streaming Proxy Requirements

Streaming must be treated as a first-class transport mode, not as a delayed blocking response.

The API proxy must stream chunks incrementally to standard OpenAI SDK clients while bridging the GNUS.ai p2p stream channel safely, with bounded buffering, cancellation, timeout handling, ordering, and usage accounting.

### 24.16.1 Streaming Proxy Responsibilities

For `stream: true`, the API proxy is responsible for maintaining a live Server-Sent Events response to the client while bridging one or more internal GCS stream channels from the p2p network.

Required proxy responsibilities:

- send HTTP response headers before the first model token when possible
- use `Content-Type: text/event-stream`
- use `Cache-Control: no-cache, no-transform`
- disable response buffering where supported
- flush each OpenAI-compatible chunk as soon as it is available
- preserve chunk order using internal sequence numbers
- emit keepalive comments while waiting for first token or during long gaps
- detect client disconnects
- publish cancellation to the GCS job when the client disconnects
- enforce first-token timeout
- enforce idle stream timeout
- enforce total wall-clock timeout
- convert internal GCS stream chunks into OpenAI-compatible SSE chunks
- convert terminal worker errors into OpenAI-compatible error events when the client is still connected
- always send `data: [DONE]` after a clean finish
- record partial usage even when the client disconnects before completion

The API proxy should never buffer the full completion before returning it to the client when `stream: true`.

### 24.16.2 Recommended Streaming Headers

For streaming responses, the API proxy should send:

```http
HTTP/1.1 200 OK
Content-Type: text/event-stream; charset=utf-8
Cache-Control: no-cache, no-transform
Connection: keep-alive
X-Accel-Buffering: no
```

When running behind Cloudflare or another edge proxy, the implementation must verify that response buffering is not delaying chunks. The system must test real client-visible time-to-first-byte and time-to-first-token, not just internal worker timings.

Important metrics:

- client-visible time-to-first-byte
- client-visible time-to-first-token
- internal job publication latency
- claim latency
- model warmup latency
- worker first-token latency
- proxy flush latency

### 24.16.3 First-Token Behavior

The API proxy should open the SSE stream quickly after the API request is accepted and the job is published.

If the worker has not produced a token yet, the proxy may send an SSE keepalive comment:

```text
: gcs job accepted

```

This prevents some clients and intermediaries from treating the connection as idle while the GCS queue is claiming the job or the worker is warming the model.

The keepalive comment must not be sent as an OpenAI `data:` chunk because OpenAI SDKs expect `data:` frames to contain valid completion chunk JSON or `[DONE]`.

Recommended first-token flow:

```text
1. Client sends stream request.
2. Proxy validates request.
3. Proxy publishes GCS API request job.
4. Proxy opens SSE response.
5. Proxy sends optional keepalive comment.
6. Worker claims job.
7. Worker emits first internal stream chunk.
8. Proxy converts it to OpenAI-compatible SSE.
9. Proxy flushes chunk immediately.
```

### 24.16.4 Internal-to-External Stream Bridge

The proxy bridges:

```text
gcs.api.stream.<job_id>
```

into:

```text
data: {openai-compatible chunk json}\n\n
```

The bridge should maintain per-job stream state:

```json
{
  "job_id": "gcsapi_01J...",
  "client_request_id": "req_abc",
  "stream_id": "chatcmpl_gnus_123",
  "last_sequence_sent": 12,
  "first_byte_sent": true,
  "first_token_sent": true,
  "client_connected": true,
  "worker_node_id": "gnusnode_abc",
  "started_at_ms": 1783468800000,
  "last_chunk_at_ms": 1783468801200
}
```

The proxy should reject, ignore, or quarantine chunks that:

- fail signature verification
- are for the wrong job ID
- are from a node that does not hold the accepted job lease
- repeat a sequence number already sent
- arrive after a terminal result
- violate the requested response format

### 24.16.5 Internal Stream Chunk

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

### 24.16.6 External OpenAI-Compatible Chunk

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

### 24.16.7 Backpressure and Slow Clients

The API proxy must not allow one slow client to create unbounded memory growth.

Required behavior:

- maintain a bounded outgoing stream buffer per request
- pause or slow reads from the internal stream when the client is backpressured, if supported by the runtime
- drop or cancel the job if the outgoing buffer exceeds the configured maximum
- record cancellation reason as `client_backpressure` when the client cannot keep up
- do not continue expensive p2p generation after the client is gone unless the request explicitly asked for detached/background completion

Recommended MVP defaults:

```text
max_stream_buffer_bytes: 262144
stream_keepalive_interval_ms: 15000
first_token_timeout_ms: 10000
idle_stream_timeout_ms: 30000
max_stream_wall_time_ms: 120000
```

These values should be tenant/model configurable.

### 24.16.8 Disconnect and Cancellation Semantics

When the client disconnects, the proxy should immediately publish a cancellation message:

```json
{
  "message_type": "GCS_API_CANCELLATION",
  "version": 1,
  "job_id": "gcsapi_01J...",
  "reason": "client_disconnected",
  "last_sequence_sent": 42,
  "partial_usage": {
    "output_tokens_sent": 42
  },
  "created_at_ms": 1783468810000,
  "signature": "..."
}
```

Workers must treat cancellation as a best-effort stop signal. If the worker already generated additional chunks, those chunks should not be forwarded after disconnect.

The usage record should distinguish:

- tokens generated
- tokens sent to client
- compute already consumed
- cancellation reason

This matters for billing, node rewards, and abuse detection.

### 24.16.9 Requeue During Streaming

If a worker fails before producing any user-visible token, the gateway may requeue the job transparently as long as the external first-token timeout and wall-clock timeout still allow it.

If a worker fails after tokens have already been sent to the client, the proxy should not silently switch to a different worker unless the response format supports continuation safely.

For MVP, the correct behavior after visible tokens have been sent is:

- terminate the stream with an error if possible, or
- close the stream and mark the job failed, or
- return a final chunk with `finish_reason: "error"` only if the client compatibility layer supports it

The system should avoid producing a stream where the first half came from one model/node and the second half came from another without explicit aggregation support.

### 24.16.10 Pre-Stream vs Post-Stream Errors

OpenAI-compatible streaming is awkward when an error happens after the HTTP status has already been sent as `200 OK`.

Therefore the proxy should distinguish:

```text
pre-stream errors
post-stream errors
```

If no SSE bytes have been sent yet, return a normal OpenAI-compatible JSON error with the correct HTTP status.

If SSE output has already started, the proxy cannot reliably change the HTTP status code. For MVP, the proxy should emit an SSE error-shaped chunk only when compatible clients tolerate it, then close the stream.

Suggested policy:

```text
if first_token_sent == false:
    requeue_or_return_json_error
else:
    emit_stream_error_if_supported
    close_stream
    mark_job_failed_after_partial_output
```

The final usage record must record that the request failed after partial output.

### 24.16.11 Stream Ordering and Replay Protection

Internal chunks must include monotonic sequence numbers and signatures.

Required behavior:

- `sequence` starts at 0 or 1 and increases by 1
- proxy tracks `last_sequence_sent`
- duplicate chunks are ignored
- small out-of-order gaps may be buffered briefly
- large or persistent gaps terminate the stream
- chunks after terminal result are ignored
- chunks from non-lease-holder nodes are rejected
- chunk signatures must cover job ID, node ID, sequence, delta hash, and timestamp

Recommended reorder buffer:

```text
max_reorder_gap: 8 chunks
max_reorder_wait_ms: 250
```

For MVP, if chunk ordering becomes complicated, prefer fail-fast over delivering corrupt token order.

### 24.16.12 Cloudflare and Runtime Boundary

Cloudflare can be the public HTTP edge, but the implementation should avoid relying on a Cloudflare Worker as a permanent p2p node.

Recommended split:

```text
Cloudflare Worker / edge route
    handles public HTTPS request, auth precheck, rate limit, and SSE response

GCS Gateway service
    maintains libP2P connections, publishes jobs, receives stream chunks, verifies worker messages
```

The Cloudflare edge and GCS Gateway may communicate over a normal internal HTTP/WebSocket stream, Durable Object, queue, or tunnel-backed service depending on deployment.

The important architectural boundary is:

```text
Cloudflare handles web ingress.
GCS Gateway handles p2p participation.
```

### 24.16.13 Streaming Test Matrix

Streaming is not done until it works with real clients.

Required tests:

- OpenAI JavaScript SDK with `stream: true`
- OpenAI Python SDK with `stream: true`
- `curl -N` SSE test
- browser `EventSource` or fetch streaming test where applicable
- slow client test
- client disconnect test
- worker timeout before first token
- worker timeout after partial tokens
- duplicate internal chunk test
- out-of-order internal chunk test
- gateway restart during stream
- Cloudflare buffering regression test
- large response test
- concurrent streaming requests test

### 24.16.14 Streaming Acceptance Criteria

Streaming is acceptable for MVP only when:

- a standard OpenAI JavaScript SDK can consume `stream: true`
- a standard OpenAI Python SDK can consume `stream: true`
- chunks arrive incrementally, not buffered until completion
- first-token latency is measured externally at the client
- disconnect cancels the GCS job
- slow clients cannot create unbounded proxy memory growth
- duplicate and out-of-order internal chunks do not corrupt the external stream
- every clean stream ends with `data: [DONE]`
- usage records distinguish generated tokens from delivered tokens

---

## 24.17 Result Envelope

### 24.17.1 Final Result

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
    "tokens_sent_to_client": 50,
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

### 24.17.2 Error Result

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

## 24.18 Queue Fairness and Democratized Pickup

### 24.18.1 MVP Policy

MVP uses:

```text
first valid claim wins
```

This matches the simplest form of democratized pickup and is easy to reason about.

### 24.18.2 Later Policies

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

### 24.18.3 Fairness State

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

## 24.19 Requeue and Retry

### 24.19.1 Requeue Reasons

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

### 24.19.2 Requeue Envelope

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

Each API request job should include maximum requeue count, total wall-clock timeout, first-token timeout, result timeout, excluded failed nodes, and partial result policy.

---

## 24.20 Security and Privacy

### 24.20.1 API Key Security

- API keys must be hashed at rest.
- API keys must be scoped to tenant/project.
- API keys may restrict models, routing modes, and spend.
- API keys may restrict public network usage.
- API keys may require private-only execution.

### 24.20.2 Job Signature Requirements

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

### 24.20.3 Prompt Privacy

Supported privacy levels:

```text
standard
encrypted_payload
private_pool
local_only
metadata_minimized
```

MVP can start with `standard` and `private_pool`. Production should support encrypted payloads and metadata-minimized job publication.

### 24.20.4 Public Queue Leakage

Public job channels must not leak sensitive prompts by default.

For sensitive jobs:

- publish only opaque job handle
- publish only capability requirements
- encrypt payload
- restrict allowed nodes
- use tenant private channel
- avoid exposing collection names when possible

---

## 24.21 Metering, Rewards, and Settlement

### 24.21.1 Usage Record

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
  "output_tokens_generated": 266,
  "output_tokens_sent_to_client": 266,
  "embedding_tokens": 0,
  "retrieval_chunks": 0,
  "child_jobs": 0,
  "compute_ms": 1840,
  "first_token_ms": 800,
  "wall_ms": 7000,
  "cost_gnus": "0.0031",
  "cancellation_reason": null,
  "created_at_ms": 1783468807000,
  "signature": "..."
}
```

### 24.21.2 Settlement Hooks

The usage record should feed:

- tenant billing
- public node reward
- private enterprise accounting
- developer dashboard
- abuse analytics
- reputation updates
- token settlement or burn/buyback logic where applicable

---

## 24.22 Data Model Changes

### 24.22.1 New Message Concepts

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

### 24.22.2 Existing Processing Messages Remain

Existing concepts remain:

```text
SGProcessing::Task
SGProcessing::SubTask
SGProcessing::TaskLock
SGProcessing::TaskResult
```

### 24.22.3 Optional Unified Queue Wrapper

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

## 24.23 CRDT Keyspace Proposal

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

## 24.24 MVP Implementation Plan

### 24.24.1 Phase 1: API Compatibility Shell

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

### 24.24.2 Phase 2: API Request Job Schema

Deliver:

- `GCSApiRequestJob`
- signed job envelope
- job ID and idempotency key
- result channel
- stream channel
- claim channel
- OpenAI payload normalization
- OpenAI error mapping

### 24.24.3 Phase 3: GCS Gateway Bridge

Deliver:

- Gateway service that maintains p2p connectivity
- publish API jobs to pub/sub
- subscribe to result/stream channels
- first valid claim handling
- lease and timeout handling
- cancellation on client disconnect

### 24.24.4 Phase 4: Node Registration and Claim

Deliver:

- node registration message
- heartbeat
- capability channels
- job claim message
- claim validation
- API job lease
- stale node rejection

### 24.24.5 Phase 5: Direct Worker Execution

Deliver:

- worker can claim chat completion job
- worker can execute local model
- worker can stream chunks
- worker can publish final result
- gateway converts to OpenAI-compatible response

### 24.24.6 Phase 6: Child Processing Jobs

Deliver:

- API job can create child `SGProcessing::Task` jobs
- parent job tracks child job IDs
- existing processing queue executes children
- parent aggregates child results
- parent publishes final result

### 24.24.7 Phase 7: Metering and Settlement

Deliver:

- usage record
- signed metering event
- node reward hook
- tenant billing hook
- dashboard data

### 24.24.8 Phase 8: Private and Hybrid Routing

Deliver:

- tenant private channels
- local-only routing
- hybrid fallback
- private node allowlist
- encrypted payload reference support

---

## 24.25 Acceptance Criteria

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
- Streaming chunks arrive incrementally, not buffered until completion.
- Client disconnect cancels the GCS job.
- Slow clients cannot create unbounded proxy memory growth.
- Final result is returned in OpenAI-compatible JSON for non-streaming requests.
- Usage is recorded per tenant/project/API key/node.
- Usage records distinguish generated tokens from delivered tokens.
- Existing processing chunk jobs still work independently.
- API request jobs can create child processing jobs in a later MVP phase.
- Public/private/local routing policy is represented in the job envelope.

---

## 24.26 Open Questions

- Should API job leases reuse any existing `TaskLock` behavior internally, or should API jobs use a fully separate lease type from day one?
- Should the first API gateway act as the only claim validator during MVP, or should claim acceptance be CRDT-consensus visible immediately?
- Should model aliases be global, tenant-specific, or both?
- Should `gpt-4o`-style compatibility aliases be allowed, or should GNUS only expose `gnus-*` names?
- Should public API jobs allow inline prompt payloads during MVP, or should every API job use a gateway payload reference from day one?
- What is the minimum useful settlement record for a public node reward?
- Should the first implementation live in the GCS repo, SuperGenius repo, or a separate API gateway repo?
- Should `GCS_API_REQUEST_JOB` be protobuf-first, JSON-first, or dual-format during early integration?
- Should post-token streaming worker failure be represented as an SSE error chunk, a closed stream, or an OpenAI-compatible final chunk with an error finish reason?

---

## 24.27 Summary

This feature gives GNUS.ai a simple developer-facing wedge:

> Change the OpenAI base URL. GNUS.ai turns the request into a distributed p2p job.

Under the hood, this requires a clean queue distinction:

- **Processing chunk jobs** remain the low-level distributed compute units.
- **API request jobs** become the high-level request/response orchestration units.

Streaming is part of the core API request job contract, not an addendum. The proxy must bridge signed GCS stream chunks into real-time OpenAI-compatible SSE while handling buffering, ordering, disconnect cancellation, partial usage, and failure boundaries.

That split lets GNUS.ai preserve its democratized queue and p2p architecture while exposing a familiar OpenAI-compatible API to the outside world.
