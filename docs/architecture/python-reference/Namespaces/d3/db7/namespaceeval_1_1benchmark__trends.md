---
title: eval::benchmark_trends

---

# eval::benchmark_trends



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| Path | **[append_to_trend_file](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#function-append_to_trend_file)**(str niche_name, dict results, Optional project_root[Path] =None) |
| dict | **[load_trend_file](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#function-load_trend_file)**(str niche_name, Optional project_root[Path] =None) |
| dict | **[compute_trend_deltas](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#function-compute_trend_deltas)**(str niche_name, Optional project_root[Path] =None) |
| tuple | **[bootstrap_ci](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#function-bootstrap_ci)**(sample_differences sample_differences, int n_bootstrap =[_K_DEFAULT_N_BOOTSTRAP](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#variable-_k_default_n_bootstrap), float confidence =[_K_DEFAULT_CONFIDENCE](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#variable-_k_default_confidence), Optional seed[int] =None) |
| dict | **[is_degradation_significant](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#function-is_degradation_significant)**(dict current_scores, dict previous_scores, float confidence =[_K_DEFAULT_CONFIDENCE](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#variable-_k_default_confidence), int n_bootstrap =[_K_DEFAULT_N_BOOTSTRAP](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#variable-_k_default_n_bootstrap), Optional seed[int] =None) |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[logger](/python-reference/Namespaces/d3/db7/namespaceeval_1_1benchmark__trends/#variable-logger)**  |

## Detailed Description




```
Derived trend views over MetricStore benchmark records (Plan 04-04 Task 1).

Per D-11: MetricStore (``artifacts/benchmarks/``) is the source of truth. The
``artifacts/trends/{niche}_trend.json`` files produced here are DERIVED views --
they can be regenerated from MetricStore at any time and carry no independent
state.

Per D-09: trend significance is determined by bootstrap 95% confidence intervals
on per-benchmark score differences. A regression is significant when the CI
excludes zero AND the point estimate (mean delta) is negative.
```


## Functions Documentation

### function append_to_trend_file

```python
Path append_to_trend_file(
    str niche_name,
    dict results,
    Optional project_root[Path] =None
)
```




```
Append a run record to ``artifacts/trends/{niche}_trend.json`` (D-11).

Schema (per RESEARCH.md Pattern 5)::

    {
      "niche": "<niche_name>",
      "runs": [
        {
          "timestamp": "<ISO8601 utc>",
          "model_version": "<str>",
          "quantization_config": {...},
          "results": {"<benchmark>": {"score": float, "per_category": {...}}}
        },
        ...
      ]
    }

The file is created if it does not exist. If it exists but is corrupt
(T-04-20 mitigation), the corrupt file is replaced with a fresh run list
rather than raising -- the MetricStore remains the recoverable source.

Args:
    niche_name: Specialist niche.
    results: Benchmark results payload (Plan 04-01 schema).
    project_root: Project root. Defaults to cwd.

Returns:
    Path to the trend file.
```


### function load_trend_file

```python
dict load_trend_file(
    str niche_name,
    Optional project_root[Path] =None
)
```




```
Load the full trend JSON for a niche.

T-04-20 mitigation: corrupt trend files fail open -- the caller gets a fresh
empty runs list and a warning is logged. MetricStore remains authoritative.

Args:
    niche_name: Specialist niche.
    project_root: Project root.

Returns:
    Dict with ``niche`` and ``runs`` keys. ``runs`` is ``[]`` if the file
    does not exist or is unreadable.
```


### function compute_trend_deltas

```python
dict compute_trend_deltas(
    str niche_name,
    Optional project_root[Path] =None
)
```




```
Compare the two most recent runs in the trend file.

For each benchmark present in BOTH runs, compute ``{metric: curr - prev}``
for every metric key in the benchmark's result entry (typically ``score``
plus any per-category aggregates).

Args:
    niche_name: Specialist niche.
    project_root: Project root.

Returns:
    Dict with ``status`` (``"ok"`` or ``"insufficient_data"``), ``deltas``
    ({benchmark: {metric: delta}}), and ``previous_timestamp`` /
    ``current_timestamp`` for traceability.
```


### function bootstrap_ci

```python
tuple bootstrap_ci(
    sample_differences sample_differences,
    int n_bootstrap =_K_DEFAULT_N_BOOTSTRAP,
    float confidence =_K_DEFAULT_CONFIDENCE,
    Optional seed[int] =None
)
```




```
Bootstrap 95% confidence interval for paired score differences (D-09).

Standard percentile bootstrap: resample ``sample_differences`` with
replacement ``n_bootstrap`` times, compute the mean of each replicate, and
take the ``(alpha/2)`` and ``(1 - alpha/2)`` percentiles of the replicate
means as the CI bounds.

Determinism (T-04-18 + reproducibility): pass an integer ``seed``. The RNG
is a fresh ``random.Random(seed)`` instance -- it does NOT touch the global
``random`` state, so test reproducibility is preserved.

Args:
    sample_differences: Per-item (or per-category) paired score differences.
        Positive = improvement, negative = regression.
    n_bootstrap: Number of bootstrap replicates. Capped at 100,000.
    confidence: Confidence level (0..1). Default 0.95.
    seed: Optional integer seed for deterministic output.

Returns:
    Tuple ``(lower, upper)`` of floats. Returns ``(0.0, 0.0)`` for empty
    input.
```


### function is_degradation_significant

```python
dict is_degradation_significant(
    dict current_scores,
    dict previous_scores,
    float confidence =_K_DEFAULT_CONFIDENCE,
    int n_bootstrap =_K_DEFAULT_N_BOOTSTRAP,
    Optional seed[int] =None
)
```




```
Per-benchmark degradation significance via bootstrap CI (D-09).

For each benchmark present in BOTH score dicts, collect per-category score
differences (``curr - prev``) as bootstrap samples, compute the 95% CI, and
flag degradation when the CI excludes zero AND the mean delta is negative.

NOTE: per Plan 04-04 Task 1, the bootstrap currently uses per-category
scores as pseudo-samples (``n = number of categories``). When per-item
scores become available from the harness, swap them in -- the CI tightens
with more samples.

Args:
    current_scores: ``{benchmark: {"score": float, "per_category": {...}}}``.
    previous_scores: Same shape as ``current_scores`` (prior run).
    confidence: Confidence level (0..1).
    n_bootstrap: Bootstrap replicate count.
    seed: Deterministic RNG seed.

Returns:
    ``{benchmark: {significant, ci_lower, ci_upper, mean_delta, n_samples}}``.
    Benchmarks present in only one run are omitted.
```



## Attributes Documentation

### variable logger

```python
logger =  logging.getLogger(__name__);
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700