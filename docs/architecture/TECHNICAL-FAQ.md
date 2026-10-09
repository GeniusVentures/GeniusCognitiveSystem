# GNUS.ai Technical Diligence FAQ

**October 9, 2026.** Short answers to recurring source-code and investor-review questions. Read with [Implementation Status](IMPLEMENTATION-STATUS.md); these answers do not certify a live public mainnet, third-party audit or benchmark.

### Is GNUS.ai just an inexpensive GPU-cloud provider?

No. The intended stack combines SuperGenius distributed processing with GCS task planning, specialist models, governed memory, and answer checking. NEO-SWARM already has C++ router/ELM modules; the full GCS system workstream is still in planning. [Architecture](ARCHITECTURE-AT-A-GLANCE.md).

### Is mainnet finished or launched?

Leadership reports the native SuperGenius mainnet implementation complete. **Public activation has not occurred**, coordinated with GCS and launch applications. Implementation, launch, operational assurance and audit status are distinct. [Platform Status](https://docs.gnus.ai/about-gnus.ai/release-status/).

### Does everyone vote with reputation points?

No. [TrustedPeerRegistry](https://github.com/GeniusVentures/SuperGenius/blob/develop/src/trustedpeer/TrustedPeerRegistry.hpp) governs an approved signer set and its changes. [ValidatorRegistry](https://github.com/GeniusVentures/SuperGenius/blob/develop/src/blockchain/ValidatorRegistry.hpp) implements weighted consensus roles and vote thresholds. Do not substitute older 'any node / two-thirds reputation' wording for these rules.

### Is the processing burn 10%, and do burns guarantee token appreciation?

No. [BurnConfig](https://github.com/GeniusVentures/SuperGenius/blob/develop/src/account/BurnConfig.hpp) defines a **1% genesis default** with quorum-governed updates. The active setting requires confirmed network-state evidence. EVM bridge/redemption burns are distinct, and burns do not guarantee an investment outcome. [Tokenomics](https://docs.gnus.ai/about-gnus.ai/features-and-benefits/tokenomics/).

### Is a GNUS divided into a billion Minions?

Not in the current native `TokenAmount`: [the C++ type](https://github.com/GeniusVentures/SuperGenius/blob/develop/src/account/TokenAmount.hpp) uses **six decimal places**, so **1 GNUS = 1,000,000 Minions**. EVM contract precisions must be verified separately.

### Does a matching GPU result hash prove a correct answer?

No. The [historical processing/hash example](https://github.com/GeniusVentures/gitbook/blob/main/technical-information/super-genius-blockchain-technical-details/verification-and-hash-results-from-processing.md) is a draft, not proof that FP32 kernels agree bit-for-bit across vendors. GCS [EIS](execution-integrity-system.md) defines execution-contract and determinism checks **in draft form**. Answer correctness needs independent grounding and semantic checks.

### What about the old 'dummy for now' verifier seed?

That line occurs in [early slicing pseudocode](https://github.com/GeniusVentures/gitbook/blob/main/technical-information/super-genius-blockchain-technical-details/slicing-data-for-macro-microjobs.md), not demonstrated current production code. Verify current native scheduling, entropy sources and attack resistance against active C++ and tests before claiming the design is secure.

### Does GCS run a complete expert/memory/evaluation pipeline today?

Separate code exists for NEO-SWARM ELM/routing and GCS Chat messaging. NEO-SWARM GAML contains a placeholder; the GCS system roadmap has no completed phases yet. The architecture is substantial, but the complete pipeline, governed long-term memory and public API are **not established as deployed**. [Status register](IMPLEMENTATION-STATUS.md).

### Who schedules the distributed ELMs?

GCS selects cognitive work. The planned [bridge](developer-api-and-compute-bridge.md) submits **one funded native ELM job containing work items**, while SuperGenius owns its workers and SubTasks. Independent non-ELM workloads may use other native jobs; local-only work may use none.

### What is the verified price?

Native general processing uses an implemented **estimated-work GNUS escrow quote** with unit caveats. The owner-set **$0.0003 per funded ELM processing-hour** belongs to a future bridge, with pooled vs per-ELM hours unresolved. It is not a retail OpenAI API tariff. Old $0.005/node-hour and GNUS TFLOPS/$ comparisons are historical scenarios. [Pricing methodology](https://docs.gnus.ai/about-gnus.ai/features-and-benefits/pricing-methodology/).

### Have the node, bridges and GCS been audited?

The public [contracts page](https://github.com/GeniusVentures/gitbook/blob/main/resources/contracts.md) links a Solidproof smart-contract audit dated February 24, 2024. That report cannot be extended to the latest entire C++ node, the cross-chain deployment, GCS or all operating networks without audit evidence for each scope.

### Are token supply, returns, and tax status confirmed by architecture documents?

No. Issued/circulating/treasury/bridged balances require audited definitions and chain data. Neither price forecasts nor QSBS treatment nor legal status of token instruments can be concluded from system design. Finance and legal teams must separately approve investor-specific claims.
