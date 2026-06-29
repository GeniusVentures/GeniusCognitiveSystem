---
title: eval::metric_store::MetricStore

---

# eval::metric_store::MetricStore



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-__init__)**(self self, Optional project_root[Path] =None) |
| Path | **[record_sgfp4_metrics](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-record_sgfp4_metrics)**(self self, str niche_name, dict fp4_stats, ** kwargs) |
| Optional[dict] | **[load_sgfp4_metrics](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-load_sgfp4_metrics)**(self self, str niche_name) |
| Dict[str, dict] | **[list_all_metrics](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-list_all_metrics)**(self self) |
| Path | **[record_benchmark_results](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-record_benchmark_results)**(self self, str niche_name, str benchmark_name, dict results) |
| Optional[dict] | **[load_benchmark_results](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-load_benchmark_results)**(self self, str niche_name, Optional benchmark_name[str] =None) |
| List[dict] | **[load_all_benchmark_results](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-load_all_benchmark_results)**(self self, str niche_name) |
| Optional[dict] | **[load_benchmark_run_by_fingerprint](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-load_benchmark_run_by_fingerprint)**(self self, str niche_name, str benchmark_name, str fingerprint_hash_value) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| None | **[_validate_stats_dict](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-_validate_stats_dict)**(dict fp4_stats, str niche_name) |
| float | **[_compute_fp4_mse](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-_compute_fp4_mse)**(dict fp4_stats) |
| float | **[_compute_t158_ratio](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#function-_compute_t158_ratio)**(dict fp4_stats) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_project_root](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#variable-_project_root)**  |
| str | **[_metrics_dir](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#variable-_metrics_dir)**  |
| str | **[_benchmarks_dir](/python-reference/Classes/de/de1/classeval_1_1metric__store_1_1_metric_store/#variable-_benchmarks_dir)**  |

## Detailed Description

```python
class eval::metric_store::MetricStore;
```




```
Structured persistence for SGFP4 quantization metrics.

Reads the stats dict produced by FP4Exporter (Plan 03-01), derives gate-relevant
metrics, and persists them to `artifacts/evaluations/{niche}_sgfp4_metrics.json`.

This class does not depend on SpecialistEvaluator or Benchmarker — it reads the
stats.json format by contract (dict shape), not by code import.
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Optional project_root[Path] =None
)
```




```
Initialize MetricStore.

Args:
    project_root: Root of the gnus-poc project. Auto-located if None.
```


### function record_sgfp4_metrics

```python
Path record_sgfp4_metrics(
    self self,
    str niche_name,
    dict fp4_stats,
    ** kwargs
)
```




```
Record SGFP4 quantization metrics for a specialist niche/run.

Extracts and computes gate-relevant metrics from the fp4_stats dict
produced by FP4Exporter.export_to_file (Plan 03-01).

Metrics derived:
- ``fp4_mse``: Weighted average of per-block mean squared error.
  If ``fp4_stats["per_block_errors"]`` is present and non-empty,
  the mean is used directly. Otherwise a proxy is computed from
  effective bitrate deviation: ``max(0.0, (effective_bpw - 2.5) / 100.0)``.
  **Note:** The proxy is a placeholder until Phase 4 benchmark data
  provides true per-block MSE values. Replace when ``per_block_errors``
  becomes available from the benchmark pipeline.
- ``fp4_effective_bitrate``: Directly from ``fp4_stats["effective_bpw"]``.
- ``fp4_t158_ratio``: ``t158_blocks / (fp4_blocks + t158_blocks)``
  if total blocks > 0, else 0.0.

Args:
    niche_name: Specialist niche name (e.g., "code", "medical").
    fp4_stats: Stats dict from FP4Exporter.export_to_file.
        Expected keys: shape, num_superblocks, layout_distribution,
        fp4_blocks, t158_blocks, effective_bpw, total_bytes.
        Optional: per_block_errors (list of float).
    **kwargs: Additional metadata (reserved for future use).

Returns:
    Path to the written JSON file.

Raises:
    ValueError: If required keys are missing or metric values are non-numeric.
```


### function load_sgfp4_metrics

```python
Optional[dict] load_sgfp4_metrics(
    self self,
    str niche_name
)
```




```
Load the most recent SGFP4 metrics file for a given niche.

Globs ``{metrics_dir}/{niche_name}_sgfp4_metrics.json``.
Since timestamp filenames sort lexicographically (ISO 8601),
returns the last matched file.

Args:
    niche_name: Specialist niche name.

Returns:
    Parsed metrics dict, or None if no metrics file exists.
```


### function list_all_metrics

```python
Dict[str, dict] list_all_metrics(
    self self
)
```




```
Load all SGFP4 metrics files.

Globs all ``*_sgfp4_metrics.json`` files and returns a dict
mapping niche_name to the parsed metrics dict.

Returns:
    Dict mapping niche_name -> metrics dict. Empty if no files exist.
```


### function record_benchmark_results

```python
Path record_benchmark_results(
    self self,
    str niche_name,
    str benchmark_name,
    dict results
)
```




```
Persist a benchmark results payload as the source of truth (D-11).

Writes ``results`` to
``artifacts/benchmarks/{niche}_{benchmark}_{YYYYMMDD-HHMMSS}.json``.
Validates the required payload keys before writing and flags an invalid
fingerprint non-destructively (T-04-16: bad input is recorded with a
``fingerprint_valid: False`` flag rather than silently dropping data).

Args:
    niche_name: Specialist niche (e.g. ``"medical"``).
    benchmark_name: Benchmark identifier (e.g. ``"mmlu"``).
    results: Results payload per the Plan 04-01 schema. Must contain
        ``niche``, ``timestamp_utc``, ``mode``, ``fingerprint``, ``results``.

Returns:
    Path to the written JSON file.

Raises:
    ValueError: If a required key is missing.
```


### function load_benchmark_results

```python
Optional[dict] load_benchmark_results(
    self self,
    str niche_name,
    Optional benchmark_name[str] =None
)
```




```
Load the most recent benchmark result for a niche (+ optional benchmark).

Per D-11 the artifacts/benchmarks/ directory is the source of truth.
Files are named ``{niche}_{benchmark}_{timestamp}.json`` and timestamps
sort lexicographically (``YYYYMMDD-HHMMSS``), so the lexicographic max
is the most recent run.

Args:
    niche_name: Specialist niche.
    benchmark_name: Optional benchmark filter. If ``None``, the most
        recent result for ANY benchmark for that niche is returned.

Returns:
    Parsed results dict, or ``None`` if no results exist.
```


### function load_all_benchmark_results

```python
List[dict] load_all_benchmark_results(
    self self,
    str niche_name
)
```




```
Load ALL benchmark results for a niche, sorted by timestamp ascending.

Args:
    niche_name: Specialist niche.

Returns:
    List of parsed results dicts. Empty if no results exist.
```


### function load_benchmark_run_by_fingerprint

```python
Optional[dict] load_benchmark_run_by_fingerprint(
    self self,
    str niche_name,
    str benchmark_name,
    str fingerprint_hash_value
)
```




```
Locate a specific run by its fingerprint hash (Plan 04-03 linkage).

WR-09: reject ``None`` / empty ``fingerprint_hash_value`` up front and
skip records whose own ``fingerprint_hash`` is ``None``. The earlier
implementation compared ``payload.get("fingerprint_hash") ==
fingerprint_hash_value``, so a caller passing ``None`` would match
EVERY record whose hash failed to compute (set to ``None`` at write
time), returning an arbitrary first record. ``None`` query now returns
``None`` (no match) and ``None`` records are skipped rather than
spuriously matching.

Args:
    niche_name: Specialist niche.
    benchmark_name: Benchmark identifier.
    fingerprint_hash_value: SHA256 hex digest from
        ``benchmark_fingerprint.fingerprint_hash``.

Returns:
    Parsed results dict whose ``fingerprint_hash`` matches, or ``None``.
```


## Protected Functions Documentation

### function _validate_stats_dict

```python
static None _validate_stats_dict(
    dict fp4_stats,
    str niche_name
)
```




```
Validate required keys and types in the fp4_stats dict.

T-03-10 mitigation: Validate fp4_stats dict keys before access;
handle missing keys with clear error messages; reject non-numeric values.

Args:
    fp4_stats: Stats dict from FP4Exporter.
    niche_name: Specialist niche name (for error messages).

Raises:
    ValueError: If required keys are missing or have wrong types.
```


### function _compute_fp4_mse

```python
static float _compute_fp4_mse(
    dict fp4_stats
)
```




```
Compute fp4_mse from available stats data.

If per_block_errors is present and non-empty, returns the mean.
Otherwise computes a proxy from effective bitrate deviation:
``max(0.0, (effective_bpw - 2.5) / 100.0)``.

The proxy is a placeholder — replace when Phase 4 benchmark data
provides true per-block MSE values.
```


### function _compute_t158_ratio

```python
static float _compute_t158_ratio(
    dict fp4_stats
)
```




```
Compute T158 ratio: t158_blocks / (fp4_blocks + t158_blocks).

Returns 0.0 if total blocks is zero.
```


## Protected Attributes Documentation

### variable _project_root

```python
_project_root =  project_root;
```


### variable _metrics_dir

```python
str _metrics_dir =  project_root / "artifacts" / "evaluations";
```


### variable _benchmarks_dir

```python
str _benchmarks_dir =  project_root / "artifacts" / "benchmarks";
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700