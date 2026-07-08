# 18A Streaming Proxy Addendum for OpenAI-Compatible API Router

## 18A.1 Purpose

This addendum expands the streaming behavior for the OpenAI-compatible API router described in `openai-compatible-api-router-and-gcs-job-queue.md`.

The base PTDS defines the OpenAI-compatible SSE chunk shape, GCS stream channels, stream sequence numbers, and the internal-to-external chunk conversion model. This addendum makes the API proxy behavior explicit so streaming is treated as a first-class transport mode rather than a delayed blocking response.

The key requirement is:

> The API proxy must stream chunks incrementally to standard OpenAI SDK clients while bridging the GNUS.ai p2p stream channel safely, with bounded buffering, cancellation, timeout handling, and usage accounting.

---

## 18A.2 Streaming Proxy Responsibilities

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

---

## 18A.3 Recommended Streaming Headers

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

---

## 18A.4 First-Token Behavior

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

---

## 18A.5 Internal-to-External Stream Bridge

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

---

## 18A.6 Backpressure and Slow Clients

The API proxy must not allow one slow client to create unbounded memory growth.

Required behavior:

- maintain a bounded outgoing stream buffer per request
- pause or slow reads from the internal stream when the client is backpressured, if supported by the runtime
- drop or cancel the job if the outgoing buffer exceeds the configured maximum
- record cancellation reason as `client_backpressure` when the client cannot keep up
- do not continue expensive p2p generation after the client is gone unless the request explicitly asked for detached/background completion

MVP can use a simple bounded buffer and cancellation policy.

Recommended MVP defaults:

```text
max_stream_buffer_bytes: 262144
stream_keepalive_interval_ms: 15000
first_token_timeout_ms: 10000
idle_stream_timeout_ms: 30000
max_stream_wall_time_ms: 120000
```

These values should be tenant/model configurable.

---

## 18A.7 Disconnect and Cancellation Semantics

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

---

## 18A.8 Requeue During Streaming

If a worker fails before producing any user-visible token, the gateway may requeue the job transparently as long as the external first-token timeout and wall-clock timeout still allow it.

If a worker fails after tokens have already been sent to the client, the proxy should not silently switch to a different worker unless the response format supports continuation safely.

For MVP, the correct behavior after visible tokens have been sent is:

- terminate the stream with an error if possible, or
- close the stream and mark the job failed, or
- return a final chunk with `finish_reason: "error"` only if the client compatibility layer supports it

The system should avoid producing a Frankenstein stream where the first half came from one model/node and the second half came from another without explicit aggregation support.

---

## 18A.9 OpenAI-Compatible Streaming Edge Cases

The proxy should handle these cases explicitly:

- `stream: false`: wait for final result and return normal JSON
- `stream: true`: return SSE chunks
- no worker claim before timeout: send OpenAI-compatible error before any token is sent
- worker claim accepted but no first token: keepalive until first-token timeout
- worker error before first token: requeue or return error
- worker error after first token: terminate stream safely
- client disconnect: cancel job and record partial usage
- gateway restart: recover job state from CRDT where possible
- duplicate chunks: ignore duplicates
- out-of-order chunks: reorder briefly, then fail if gap persists
- final result without prior chunks: send final chunk and `[DONE]`

---

## 18A.10 Error Handling During Streaming

OpenAI-compatible streaming is awkward when an error happens after the HTTP status has already been sent as `200 OK`.

Therefore the proxy should distinguish:

```text
pre-stream errors
post-stream errors
```

### 18A.10.1 Pre-stream errors

If no SSE bytes have been sent yet, return a normal OpenAI-compatible JSON error with the correct HTTP status.

Example:

```json
{
  "error": {
    "message": "No GNUS nodes are currently available for the requested model and routing policy.",
    "type": "service_unavailable",
    "code": "gnus_no_capable_nodes"
  }
}
```

### 18A.10.2 Post-stream errors

If SSE output has already started, the proxy cannot reliably change the HTTP status code. For MVP, the proxy should emit an SSE error-shaped chunk only when compatible clients tolerate it, then close the stream.

Suggested internal policy:

```text
if first_token_sent == false:
    requeue_or_return_json_error
else:
    emit_stream_error_if_supported
    close_stream
    mark_job_failed_after_partial_output
```

The final usage record must record that the request failed after partial output.

---

## 18A.11 Stream Ordering and Replay Protection

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

---

## 18A.12 Cloudflare and Runtime Notes

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

---

## 18A.13 Test Matrix

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

---

## 18A.14 Streaming Acceptance Criteria

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

## 18A.15 Summary

The base API-router PTDS defines the distributed job model. This addendum defines the streaming proxy contract.

The streaming proxy must be boring and strict on the outside:

```text
OpenAI-compatible SSE in real time.
```

And GNUS-native on the inside:

```text
Signed GCS stream chunks from a claimed p2p job.
```

That bridge is what lets normal OpenAI clients consume a distributed GNUS.ai execution without knowing anything about the swarm.
