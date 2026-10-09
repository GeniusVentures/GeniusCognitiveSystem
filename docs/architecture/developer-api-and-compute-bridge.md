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
            v
Installed GCS /v1 gateway or hosted api.gnus.ai [both planned]
    OpenAI-compatible request / authentication / usage envelope
            |
            v
GCS Semantic Core: plan, select ELMs, apply privacy + memory policy
            |
            +---- Local or tenant-private execution (when permitted)
            |
            +---- Planned SuperGenius requestor bridge
                       |
                       | ONE funded native ELM processing job
                       | Task.json_data carries elms[] work items
                       | GNUS wallet + job escrow
                       v
                  EXISTING SuperGenius processing grid
                  (native queue, worker selection, ownership)
                       |
                       v
                  SGProcessingManager model/runtime on processor
                       |
                       v
                  Results (work-item ids), usage + settlement
            |
            v
GCS assembles result / SSE or JSON back to application
```

The API adapter converts transport formats and reports usage; **GCS** controls cognitive planning, memory, API authentication, request budgeting and final response synthesis. **SuperGenius** owns the existing distributed processing queue, native worker selection/ownership, node execution and GNUS escrow/settlement. Do **not** add a second scheduler, processor-bidding layer or child-task claim/lease protocol to SuperGenius. The previously specified higher-level GCS API request job, if needed to coordinate *GCS orchestrators*, is distinct from the one funded native child-processing job. These boundaries follow the corrected [SuperGenius ELM bridge design (#369)](https://github.com/GeniusVentures/SuperGenius/issues/369) and [ELM runtime scope (#17)](https://github.com/GeniusVentures/SGProcessingManager/issues/17). Neither a token on EVM nor API documentation proves native public mainnet activation.

The existing [OpenAI-compatible API router specification](openai-compatible-api-router-and-gcs-job-queue.md) defines `/v1/models`, `/v1/chat/completions`, and `/v1/embeddings` for the intended MVP. Streaming uses incremental SSE responses. A future `/v1/responses` endpoint and broad tool-calling/structured-output coverage require separate compatibility work; they are **not** claimed as implemented MVP features. Applications depending on unsupported behavior should receive a documented error instead of silently different behavior.

## Business models enabled by the same infrastructure

| Product | Commercial basis | Status |
| --- | --- | --- |
| Pooled distributed compute | Estimated or job-declared workload, quoted and escrowed in GNUS (current `dimensions.block_len` proxy; **not measured FLOPs, bytes or elapsed time**) | Native general-processing escrow implemented; public mainnet not active |
| OpenAI-compatible inference | Contracted API request or token usage | API design specified, external production gateway **planned** |
| GCS cognitive services | Specialist reasoning, governed memory, retrieval, verification, workflows | Integration and differentiated service design |
| Enterprise/private execution | License, private resource allocation and approved usage | Product-specific scope and pricing to be confirmed |

The API can expose commodity-style inference, differentiated multi-expert GCS cognition, or tenant-private workloads **without forcing the customer to learn the underlying network architecture**. Standard clients should not unknowingly opt into persistent memory, public worker execution, or disclosure of sensitive prompt content.

## Implemented native job funding: GNUS-priced escrow

Unlike the future GCS API gateway, **SuperGenius already implements job pre-funding** for its processing-job path. In [`GeniusNode::ProcessImage`](https://github.com/GeniusVentures/SuperGenius/blob/develop/src/account/GeniusNode.cpp#L3121-L3210), the requester computes a GNUS amount, checks spendable UTXOs, invokes `HoldEscrow()`, and enqueues the task. [`GetProcessCost`](https://github.com/GeniusVentures/SuperGenius/blob/develop/src/account/GeniusNode.cpp#L3252-L3299) obtains the live or recently cached GNUS/USD quote (CoinGecko token id `genius-ai`) and converts the dollar-denominated estimate into GNUS minions (10^-6 GNUS). The network later processes escrow payout using recorded subtask results and the network's burn configuration; see [`TransactionManager`](https://github.com/GeniusVentures/SuperGenius/blob/develop/src/transaction/TransactionManager.cpp#L1087-L1315).

The current [`TokenAmount` constants](https://github.com/GeniusVentures/SuperGenius/blob/develop/src/account/TokenAmount.hpp#L24-L33) and unit tests yield this **implemented estimate**, before fixed-point rounding and a minimum of one minion:

```text
estimated_USD = L * FLOPS_PER_BYTE * PRICE_PER_FLOP / 10^15
              = L * 20 * 500 / 10^15
              = L * 0.00000000001

GNUS_to_escrow (approx.) = estimated_USD / current_USD_per_GNUS
```

At the configured rate this is **$0.005 per 10 billion *estimated* FLOPs** ($0.50 per trillion), equivalent to **$0.005 per 500 million *assumed byte-like units***. It is **not $0.005 per hour**, and there is no fixed `$0.005` hourly multiplier in the implementation. The `token_amount_test.cpp` 500 MiB case expects 5,242 minions at $1/GNUS, reflecting truncation to six decimals.

**Implementation caveat requiring follow-up:** `L` comes from `SGProcessingManager::ParseBlockSize()`, which sums input `dimensions.block_len` values per model input rather than measuring actual FLOPs, elapsed seconds, or total file bytes. The [processing schema guide](https://github.com/GeniusVentures/SGProcessingManager/blob/91021875491925e08c26e1d3ffedbe5815f3871d/doc/processing-json-guide.md) uses `block_len` for a *patch depth* in a texture3D example. Therefore treating `L` as bytes can be dimensionally inconsistent for some processing formats. The implemented estimator needs **unit normalization and matched-workload tests before publication as a guaranteed per-work cost**.

This **implemented native general-processing escrow** remains the substrate for a planned funded ELM-job extension. The ELM integration is not yet complete, so current byte/proxy work estimates should not be presented as pricing for a working multi-ELM service. **OpenAI-compatible GCS calls are not yet wired to a deployed public API that automatically bills through this mechanism**.

## Owner-resolved ELM-job rate (designed, not yet deployed)

SuperGenius engineering planning **resolved the rate to $0.0003 per funded processing-hour on 2026-08-26**, explicitly choosing **dollars**, not the conflicting “$0.0003 cents/hour” wording in early notes. This is the **fixed rate chosen for Phase 13 ELM Job Bridging**, *not* an unapproved hypothetical and *not* the current general-processing `TokenAmount` formula. It is a project-design commitment; [SuperGenius #369](https://github.com/GeniusVentures/SuperGenius/issues/369) and [SGProcessingManager runtime #17](https://github.com/GeniusVentures/SGProcessingManager/issues/17) are still **open**, and [Phase 13/14 in the SuperGenius roadmap](https://github.com/GeniusVentures/SuperGenius/blob/develop/.planning/ROADMAP.md) remains incomplete.

The planned extension uses a **single native processing job** with `job_type: "elm_processing"`, `elms[]` work items in `Task.json_data`, and `funding.maximum_processing_hours`/GNUS escrow. The existing SuperGenius grid selects and runs participants; GCS does *not* bid for devices or maintain a parallel native scheduler. Model downloads may count toward billable work. No fixed retail OpenAI-API price follows from the engineering funding rate.

```text
planned native ELM job funding, USD-equivalent
  = $0.0003 / processing-hour * funded processing-hours

GNUS required for escrow (planned)
  = above USD amount / quoted USD price per GNUS
    [subject to an agreed quote-time, rounding and escrow policy]
```

**Unresolved aggregation semantics:** The design allows either a **single pooled processing-hour budget** for the job or **separate allocations per ELM work item**. Zero to roughly three external ELMs is a common workload expectation, *not* a requirement or pricing multiplier. For a hypothetical minute of allocated work, a one-minute pool is $0.000005. **Only if** three ELMs are independently charged one minute each would the funded estimate be $0.000015. Final behavior must define concurrency, billed versus elapsed time, overrun/refund rules, minimum GNUS units and quote staleness. Neither illustrative value is a production quote.

The ELM-hour rate **does not override** the implemented general-processing price formula above, and it is distinct from older **$0.005/node-hour** sales scenarios and the separate modeled [TFLOPS-per-$1-hour comparison](https://github.com/GeniusVentures/gnus-ai-pricing). A binding *API retail tariff* still requires a decision on what the customer buys (requests, tokens, subscriptions, or reserved hours), included memory/network/verification costs, and operating margin. See the canonical [pricing methodology](https://docs.gnus.ai/about-gnus.ai/features-and-benefits/pricing-methodology/).

## Delivery and validation

The public OpenAI-compatible gateway is a **v1.1 Phase 6** work item ([roadmap](https://github.com/GeniusVentures/GeniusCognitiveSystem/blob/main/.planning/workstreams/system/ROADMAP.md)); it must pass the specification's signed-job, worker-claim, cancellation, streaming, privacy, and usage-metering acceptance checks before anyone calls it live. Existing SDK clients are an adoption opportunity, not evidence of current revenue.

See the [full protocol and acceptance criteria](openai-compatible-api-router-and-gcs-job-queue.md), [system architecture](system-overview.md), and [GNUS platform release status](https://docs.gnus.ai/about-gnus.ai/release-status/).
