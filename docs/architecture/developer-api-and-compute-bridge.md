# Developer API and Distributed Compute Bridge

> **Document status — October 2026:** Product explanation and proposed commercial model, **not** an operational API price list. SuperGenius mainnet implementation is complete, but native public mainnet activation is deferred for GCS, Genius AI Boss, and other application readiness. The OpenAI-compatible GCS API is designed and tracked as **system workstream v1.1, Phase 6**, with its implementation acceptance criteria still outstanding.

## The two developer entry points

GNUS.ai is more than a marketplace for idle device compute. It has two complementary interfaces:

| Interface | What a customer asks for | What the system supplies |
| --- | --- | --- |
| **Distributed compute** | Execution of a declared job, processing chunk, or workload | Eligible node resources, distributed task handling, verification controls, and GNUS accounting/settlement |
| **GCS cognitive API** | A result from an AI task through a familiar application interface | GCS planning, Semantic Core, specialist Expert Models, governed GAML memory and grounding when needed, cognitive checking, and a suitable local/private/distributed execution path |

An existing OpenAI-compatible application should be able to use the planned GNUS endpoint with a changed **base URL, API key, and model alias**, subject to the API features the application uses. This is an integration path, **not** a guarantee of complete OpenAI API behavioral equivalence or an endorsement by OpenAI.

```text
Existing application / OpenAI-compatible client
            |
            | HTTPS, standard /v1 API shape
            v
api.gnus.ai  [planned, not public production]
    Cloudflare ingress and GCS API Router
            |
            | Signed GCS_API_REQUEST_JOB
            v
GCS gateway / queue and policy-aware worker selection
            |
            +---- Local execution or private tenant pool
            |
            +---- Eligible SuperGenius distributed nodes
                       |
                       +---- Semantic Core and selected experts
                       +---- GAML / retrieval / verification when permitted
            |
            v
Result aggregation + usage accounting
            |
            v
OpenAI-compatible JSON / SSE response
```

The API adapter converts transport formats and reports usage; **GCS** controls cognitive planning and memory, while **SuperGenius** provides the distributed queue, node execution and settlement when selected. Neither a token on an EVM chain nor a public API document proves that native public mainnet has been activated.

The existing [OpenAI-compatible API router specification](openai-compatible-api-router-and-gcs-job-queue.md) defines `/v1/models`, `/v1/chat/completions`, and `/v1/embeddings` for the intended MVP. Streaming uses incremental SSE responses. A future `/v1/responses` endpoint and broad tool-calling/structured-output coverage require separate compatibility work; they are **not** claimed as implemented MVP features. Applications depending on unsupported behavior should receive a documented error instead of silently different behavior.

## Business models enabled by the same infrastructure

| Product | Commercial basis | Status |
| --- | --- | --- |
| Pooled distributed compute | Measured compute usage | Native mainnet implementation complete; public launch not active |
| OpenAI-compatible inference | Contracted API request or token usage | API design specified, external production gateway **planned** |
| GCS cognitive services | Specialist reasoning, governed memory, retrieval, verification, workflows | Integration and differentiated service design |
| Enterprise/private execution | License, private resource allocation and approved usage | Product-specific scope and pricing to be confirmed |

The API can expose commodity-style inference, differentiated multi-expert GCS cognition, or tenant-private workloads **without forcing the customer to learn the underlying network architecture**. Standard clients should not unknowingly opt into persistent memory, public worker execution, or disclosure of sensitive prompt content.

## Compute-cost planning, not a confirmed API price

The latest lightweight-ELM **planning assumption** is **$0.0003 per active external ELM-hour**. It is a compute-cost input for analysis, **not an approved node payout, public service tariff, production benchmark, or price per API request**.

A simple illustrative model is:

```text
external ELM compute estimate (USD)
  = $0.0003 / active ELM-hour
    * sum(active hours of each externally executed ELM)
```

The expected small-model workload typically needs zero to about **three external experts** rather than three always-on workers. That is a planning expectation, not a contractual hard cap on model calls or compute usage. Three external ELMs each active for one minute would cost **$0.000015 in this hypothetical model**. Three active for a full hour each would cost **$0.0009**. An intermittent request is not a 24/7 rental.

This compute-only estimate **excludes** any unmeasured Semantic Core work, network ingress, memory retrieval and storage, trust/verification, bandwidth, retries, token conversion, accounting, operations, and margins. Actual metering can record individual expert durations, model classes, privacy modes, input/output tokens, and network resource usage without prematurely choosing a customer tariff.

Do **not** equate this active-ELM-hour estimate with either:

- The historical **$0.005 per node-hour** assumed in earlier GNUS network and AI Boss comparisons; hardware, available FLOPs, utilization, and billing intent are different.
- The **TFLOPS per $1 per hour** comparison in [GNUS AI Pricing Comparison](https://github.com/GeniusVentures/gnus-ai-pricing); that uses assumed effective throughput and GPU rental prices, not API-token costs or a measured ELM-hour.
- The USD-per-FLOP / processed-byte estimation constants in [SuperGenius `TokenAmount`](https://github.com/GeniusVentures/SuperGenius/blob/develop/src/account/TokenAmount.hpp), which are yet another unit and not a GCS public tariff.

The [GNUS pricing methodology and status](https://docs.gnus.ai/about-gnus.ai/features-and-benefits/pricing-methodology/) explains both historical comparisons and the cost normalizations. A binding retail price should be published only after the active unit, actual measurements, included costs, conversion rules, and customer-facing billing policy are decided.

## Delivery and validation

The public OpenAI-compatible gateway is a **v1.1 Phase 6** work item ([roadmap](https://github.com/GeniusVentures/GeniusCognitiveSystem/blob/main/.planning/workstreams/system/ROADMAP.md)); it must pass the specification's signed-job, worker-claim, cancellation, streaming, privacy, and usage-metering acceptance checks before anyone calls it live. Existing SDK clients are an adoption opportunity, not evidence of current revenue.

See the [full protocol and acceptance criteria](openai-compatible-api-router-and-gcs-job-queue.md), [system architecture](system-overview.md), and [GNUS platform release status](https://docs.gnus.ai/about-gnus.ai/release-status/).
