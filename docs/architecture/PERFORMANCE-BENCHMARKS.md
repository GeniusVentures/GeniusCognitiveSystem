# Performance Benchmarks: Evidence and Protocol

**Reviewed October 9, 2026.** **No certified production benchmark table is published in this document.** NEO-SWARM has benchmark runners and test code, but this review did not reproduce their outputs on an identified set of devices. Do not turn model size targets, synthetic TFLOPS pricing estimates, test-file names or a successful software build into measured inference claims.

## What must be measured

| Category | Required report | Why it matters |
| --- | --- | --- |
| Expert/model artifact | Exact model and tokenizer SHA, base model, distillation corpus version, weight bytes (disk), quantization/adapter format, context length | '200–300 MB' must identify whether it is on-disk weights, memory-mapped footprint or active RAM. |
| Quality | Public benchmark suite/version, sample count, contamination check, exact prompts, task success, baseline model, repeated confidence bounds | Smaller experts must solve assigned tasks, not just fit in memory. |
| Inference speed | Prefill tokens/s, decode tokens/s, time-to-first-token, p50/p95 latency, batching, input/output tokens, warm/cold runs | Throughput is not peak FLOPs and latency can dominate UX. |
| Device fit | Hardware model, OS, runtime/driver, RAM peak/resident, CPU/GPU utilization, energy/power if measured, throttling | Older phones need measured RAM and sustained performance, not assumptions. |
| Routing | Number of external experts used (0–3 only as planning expectation), router precision/recall, selection overhead, fallbacks, total success | A route to the wrong expert can cost more or return a worse answer. |
| Memory | Provenance/retention tests, opt-in scope, write/read latency, forgetting and authorization checks | Storing a fact is not proof of safe governed GAML. |
| Distributed execution | Worker count, geography, payload size, network overhead, retries, total job time, signed job receipts, failure rate | Distributed throughput is not single-device throughput multiplied by node count. |
| Integrity | Hardware pairs/vendors, kernel manifests, input seeds, determinism class, drift bands, false-positive/false-negative rates | Float hashes across GPUs cannot be assumed equal. EIS is still a draft. |
| Economics | Current provider invoices, equipment/power assumptions, GNUS/USD quote timing, actual funded work, network/ops costs, workload match | Hourly ELM design rate and old TFLOPS/$ examples are **not** customer production bills. |

## Required baseline matrix

Collect comparable local/private and network runs for a **single model** and the **smallest effective specialist set**, then compare against an ordinary non-routed baseline using the same task set. Include at least one commodity laptop/desktop CPU, one consumer GPU, one relevant mobile device if supported, and a heterogeneous multi-node configuration. State **unsupported** rather than assuming deployment on iPhone 7 or every console.

## Reproducible result record

```yaml
benchmark_id: REQUIRED
run_utc: REQUIRED
repo_and_commit: REQUIRED
model_artifact_sha256: REQUIRED
hardware_os_driver_runtime: REQUIRED
task_suite_and_version: REQUIRED
sample_count: REQUIRED
precision_quantization_context_batch: REQUIRED
memory_ram_peak_bytes: REQUIRED
time_to_first_token_ms_p50_p95: REQUIRED
decode_tokens_per_second: REQUIRED
task_success_and_baseline: REQUIRED
distributed_topology_and_bytes: OPTIONAL
eis_kernel_manifest_and_drift: NOT_MEASURED
end_to_end_cost_usd: NOT_MEASURED
raw_logs_and_reproduction_command: REQUIRED
```

## Evidence status

- [NEO-SWARM C++ inference and router tests](https://github.com/GeniusVentures/GNUS-NEO-SWARM/tree/43712056734fc41bccc74fe3f9cd7db8767cb32e/test) are **code evidence**, not verified published measurements.
- [NEO-SWARM Python POC benchmark and distillation tests](https://github.com/GeniusVentures/GNUS-NEO-SWARM/tree/43712056734fc41bccc74fe3f9cd7db8767cb32e/gnus-poc/tests) provide a starting point for reproducible runs.
- [EIS determinism classes](execution-integrity-system.md) describe how cross-hardware comparison *should* work, not test results.
- Historical GNUS comparison numbers have been labeled illustrative in [Pricing Methodology](https://docs.gnus.ai/about-gnus.ai/features-and-benefits/pricing-methodology/).

**Publish condition:** Attach actual logs, versioned scripts and model/data licensing notes, then fill this page with clearly dated results. Until then, do not claim a measured 90% saving, a 99% SLA, 200–300 MB operational models, or iPhone 7 throughput.
