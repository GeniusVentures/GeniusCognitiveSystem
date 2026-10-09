# Document Index and Source Authority

**October 9, 2026.** Use this short index when building a diligence summary or AI-generated description. **Source code, tests, reported deployment, and architecture specifications are different types of evidence.**

## Start here

1. [Architecture at a Glance](ARCHITECTURE-AT-A-GLANCE.md) — readable GNUS/GCS split and execution choices.
2. [Implementation Status and Evidence](IMPLEMENTATION-STATUS.md) — component-level source revisions, planned items, unresolved risks.
3. [Technical FAQ](TECHNICAL-FAQ.md) — precise answers to consensus, audit, token, GPU-hash and pricing questions.
4. [Performance Benchmarks](PERFORMANCE-BENCHMARKS.md) — the required test protocol and currently missing measured results.
5. [Developer API and Compute Bridge](developer-api-and-compute-bridge.md) — future OpenAI-compatible client path vs native SuperGenius jobs.

## Normative or current references

| Purpose | Preferred source |
| --- | --- |
| SuperGenius release and public network status | [GNUS Platform Status](https://docs.gnus.ai/about-gnus.ai/release-status/) |
| GCS detailed cognitive design | [System Overview](system-overview.md), [Executive Summary](executive-summary.md) and topic specifications |
| GCS delivery plan | [System Workstream ROADMAP](https://github.com/GeniusVentures/GeniusCognitiveSystem/blob/main/.planning/workstreams/system/ROADMAP.md) and [Chat App ROADMAP](https://github.com/GeniusVentures/GeniusCognitiveSystem/blob/main/.planning/workstreams/app/ROADMAP.md) |
| Native SuperGenius code and ledger policy | [SuperGenius develop](https://github.com/GeniusVentures/SuperGenius/tree/develop) at a stated commit; verify deployed pins separately |
| NEO-SWARM prototype/ELMs | [GNUS-NEO-SWARM](https://github.com/GeniusVentures/GNUS-NEO-SWARM), including C++ router/ELM tests and Python POC |
| Native processing support | [SGProcessingManager](https://github.com/GeniusVentures/SGProcessingManager) |
| Implemented native work quote vs planned ELM-hour pricing | [GNUS Pricing Methodology](https://docs.gnus.ai/about-gnus.ai/features-and-benefits/pricing-methodology/) |
| Token units, genesis burn default and EVM distinctions | [Tokenomics](https://docs.gnus.ai/about-gnus.ai/features-and-benefits/tokenomics/) |
| Security audit publication and scope | [Contracts/audit listing](https://github.com/GeniusVentures/gitbook/blob/main/resources/contracts.md) |

## Material that must be labeled historical or superseded

- [Older four-phase GCS roadmap](roadmap-and-risks.md) — explicitly **superseded** by the 15-phase System Workstream plan. Do not cite its milestone dates as current.
- [Early sampled-block slicing pseudocode](https://github.com/GeniusVentures/gitbook/blob/main/technical-information/super-genius-blockchain-technical-details/slicing-data-for-macro-microjobs.md) — has a 'dummy for now' seed; **not** current verified verifier implementation.
- [Historical processing/hash example](https://github.com/GeniusVentures/gitbook/blob/main/technical-information/super-genius-blockchain-technical-details/verification-and-hash-results-from-processing.md) — not proof of identical FP32 output across vendors.
- Earlier xAI node-hour/10%-burn/price-appreciation scenarios — see [Pricing Methodology](https://docs.gnus.ai/about-gnus.ai/features-and-benefits/pricing-methodology/) before citing figures.
- Any pitch promising a dated public mainnet launch, guaranteed QSBS, audited/unrestricted rewards, actual 99% margins, or token-price multiples — **not current verified product evidence**.

## Investor-reader rule

Use current code and reviewed documents for architecture and status. Use logged tests or independent audit reports for performance/security. Use dated primary cap-table, chain, finance and legal records for economic claims. If a primary record is unavailable, mark the claim **unverified**, not false and not true by assumption. This index is not an offering document.
