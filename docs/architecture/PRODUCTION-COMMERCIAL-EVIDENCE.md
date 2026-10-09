# Production and Commercial Evidence Register

**Evidence snapshot: October 9, 2026 (UTC).** This is a point-in-time review of public GitHub Actions and repository sources, not an assertion that every deployment or test has passed. Detailed architecture/source inventory: [Implementation Status](IMPLEMENTATION-STATUS.md). Measurement protocol: [Performance Benchmarks](PERFORMANCE-BENCHMARKS.md).

## Reproducible build and test evidence

| System | Run and commit | Verified result | Meaning and next evidence |
| --- | --- | --- | --- |
| SuperGenius release build | [Run 37870557840](https://github.com/GeniusVentures/SuperGenius/actions/runs/37870557840), `develop@fd4194705aaf4901450ac7354d5acebce0bcd49e`, Oct 9 UTC | **Overall failed**. Individual jobs for Windows Debug, Linux x86_64 Release, Android variants, and iOS Debug succeeded. macOS Debug failed 1 of 142 CTest tests: `consensus_pending_lifecycle_test` (named case `ConsensusPendingLifecycleTest.CertificateCallbackStallsUntilPostCommitReadbackCanReleaseSameSlot`). Linux aarch64 Debug failed 2 of 134: `trust_first_boot_e2e_test`, `multi_account_test`. macOS Release failed during nested `evmrelay/build` submodule checkout due to a GitHub `cmaketemplate` fetch timeout. | Real multi-platform CI evidence exists, **not a fully green release**. Attach rerun on the same code and diagnose test failures separately from network checkout failures. |
| SuperGenius prior feature branch | [Run 37848757691](https://github.com/GeniusVentures/SuperGenius/actions/runs/37848757691), `dev_tokenprice@3c41ab3` | Release Build CI reported success | Useful targeted earlier evidence, **not** equivalent to a pass on subsequent `develop@fd41947`. |
| GCS docs publication | [Run 37881735320](https://github.com/GeniusVentures/GeniusCognitiveSystem/actions/runs/37881735320), `main@6d05e85`, Oct 9 UTC | **Build-and-deploy step succeeded; workflow failed custom-domain verification**: `gcs.gnus.ai` was attached to the expected Pages project but did not serve identical `/javascripts/ask/drawer.js` bytes to production Pages when probed. | Investigate custom-domain cache/routing/deployment lag; compare exact assets and repeat verifier. Do not report production site parity passed. |
| GCS release build | [Run 37804780684](https://github.com/GeniusVentures/GeniusCognitiveSystem/actions/runs/37804780684), `main@745dfd5`, Oct 8 UTC | Release Build CI failed | Capture failing job logs and re-run after corrections. Avoid claiming all current GCS desktop/mobile targets passed. |
| GNUS-NEO-SWARM Python POC | [Run 36962984762](https://github.com/GeniusVentures/GNUS-NEO-SWARM/actions/runs/36962984762), `develop@4371205`, Oct 2 UTC | `GNUS POC Python Tests` passed on the pinned revision | Confirms one tested Python prototype path, **not** cross-GPU inference, production distributed GCS integration or a benchmark number. |
| SGProcessingManager | [Run 37825020681](https://github.com/GeniusVentures/SGProcessingManager/actions/runs/37825020681), `main@db44126`, Oct 8 UTC | C++ header/schema generation passed | Does not measure MNN inference correctness, tokens per second or ELM scheduling. |

### Release-readiness priority

1. Reproduce the failing SuperGenius consensus/trust tests on the exact platforms, distinguish deterministic regression from race/flakiness, and rerun the **same pinned commit**. Do not dismiss a failing security/consensus test as infrastructure noise without a reproduction.
2. Diagnose the GCS production domain mismatch separately from the successful Cloudflare Pages deploy job. Compare production and custom-domain asset hashes and cache configuration.
3. Re-run the full GCS release matrix and collect job-level logs and artifacts.
4. Publish independent benchmark run records with workload, hardware, model hash, runtime, quality and throughput. Successful `pytest` or CMake builds are not performance comparisons.

## Commercial claims: evidence needed

| Claim | Present evidence status | Minimum acceptable corroboration |
| --- | --- | --- |
| Annual revenue / MRR / run rate | **Not verified from checked public repositories.** Avoid using historical deck forecasts or pipeline as recognized revenue. | Ledger or payment processor exports reconciled to the bookkeeping P&L, customer contract/invoice identifiers, recognized-versus-booked definitions, reporting period and responsible approver. |
| Gross margin and unit economics | **Not verified.** $0.0003 per funded ELM processing-hour is a future design setting, not actual margin. | Deployed bills, payout records, bandwidth/storage overhead, energy cost boundary, processing-hour aggregation, and matched compute workload. |
| Node count and uptime | **Not verified.** Proposed 256k nodes and ISP channels are scale goals rather than monitored active-user counts. | Dated unique participating-node/heartbeat logs, opt-in/active definition, sample availability window, uptime numerator/denominator, region and churn. |
| Partnerships and pilots | **Not verified as delivered revenue from these public records.** | Executed agreements (redacted where needed), milestone acceptance, paid invoices and signed usage or service evidence. |
| Financing ask and securities terms | **Not settled by technical material.** Historical $20M deck and $5M hybrid one-pager conflict. | Board-approved current financing summary, capitalization, stock terms, warrant terms and counsel review. |
| QSBS eligibility | **Not established by a pitch document.** | Qualified U.S. tax counsel opinion based on issuing entity, asset history, active-business test, issuance and holding period; do not promise exclusions. |
| GNUS issued supply | **User-provided October 8 RPC snapshot: 17,163,867.243430 GNUS**, total across Ethereum, Polygon, Base and BSC. | Retain chain-specific blocks and raw `eth_call(totalSupply)` outputs; separately classify treasury and locked holdings before publishing circulating supply. |

## Approval rule for external deck

Mark each numeric claim **measured** only when reproducible and dated; **reported** when supplied by management but not independently inspected; **modeled** for explicit scenarios; **planned** for design targets; and **unverified** otherwise. Remove amounts, ROI promises, audit breadth, QSBS assurances and node-level SLA numbers unsupported by this register.

This register documents evidence gaps and owner-required artifacts. It does not alter product scope, retroactively imply mainnet is unimplemented, or assert an audit verdict.
