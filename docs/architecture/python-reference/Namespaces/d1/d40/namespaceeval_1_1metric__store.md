---
title: eval::metric_store

---

# eval::metric_store



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[eval::metric_store::MetricStore](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[logger](/python-reference/Namespaces/d1/d40/namespaceeval_1_1metric__store/#variable-logger)**  |

## Detailed Description




```
Structured persistence for SGFP4 quantization metrics per specialist/run.

MetricStore reads the stats.json format produced by FP4Exporter.export_to_file
(Plan 03-01) and persists gate-relevant derived metrics (fp4_mse, fp4_effective_bitrate,
fp4_t158_ratio) alongside the raw stats for auditability.

Implements D-09: SGFP4 error metrics become gate dimensions in eval_gates.

Plan 04-04 (D-11): MetricStore is the source of truth for benchmark results too.
``record_benchmark_results`` / ``load_benchmark_results`` /
``load_all_benchmark_results`` / ``load_benchmark_run_by_fingerprint`` extend the
Phase 3 SGFP4 API without altering it.
```



## Attributes Documentation

### variable logger

```python
logger =  logging.getLogger(__name__);
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700