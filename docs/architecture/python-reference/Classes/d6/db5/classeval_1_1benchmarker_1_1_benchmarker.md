---
title: eval::benchmarker::Benchmarker

---

# eval::benchmarker::Benchmarker





## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-__init__)**(self self, Optional project_root[Path] =None, Optional evaluator[SpecialistEvaluator] =None, Optional config[dict] =None) |
| dict | **[compare_variants](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-compare_variants)**(self self, str niche_name, list variant_results) |
| | **[save_comparison](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-save_comparison)**(self self, str niche_name, dict comparison) |
| | **[print_comparison_table](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-print_comparison_table)**(self self, dict comparison) |
| dict | **[gate_check](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-gate_check)**(self self, str niche_name, dict config =None) |
| dict | **[composite_2_of_3](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-composite_2_of_3)**(self self, bool scores_pass, bool regression_pass, bool deviation_pass, bool scores_evaluated =True, bool regression_evaluated =True, bool deviation_evaluated =True) |
| dict | **[gate_check_benchmarks](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-gate_check_benchmarks)**(self self, str niche_name, Optional benchmark_results_path[Path] =None, Optional config[dict] =None) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| | **[_check_dimension](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_check_dimension)**(str dim_name, float actual_value, dict dim_config) |
| dict | **[_update_consecutive_failures](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_update_consecutive_failures)**(dict prev_state, dict now_failures) |
| Path | **[_gate_state_path](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_gate_state_path)**(self self, str niche_name) |
| dict | **[_load_gate_state](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_load_gate_state)**(self self, str niche_name) |
| | **[_save_gate_state](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_save_gate_state)**(self self, str niche_name, dict consecutive_failures, list checks) |
| dict | **[_load_yaml](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_load_yaml)**(self self, Path path) |
| dict | **[_load_specialist_mapping](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_load_specialist_mapping)**(self self, str niche_name) |
| dict | **[_load_benchmark_threshold](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_load_benchmark_threshold)**(self self, str benchmark_name) |
| Path | **[_bench_gate_state_path](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_bench_gate_state_path)**(self self, str niche_name) |
| dict | **[_load_bench_gate_state](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_load_bench_gate_state)**(self self, str niche_name) |
| | **[_save_bench_gate_state](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_save_bench_gate_state)**(self self, str niche_name, dict consecutive_failures, list checks) |
| | **[_find_canonical_results](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_find_canonical_results)**(self self, str niche_name, bool quantized_only =False) |
| dict | **[_load_baseline_scores](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_load_baseline_scores)**(self self, str niche_name) |
| float | **[_extract_score](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_extract_score)**(self self, result_entry result_entry) |
| dict | **[_sgfp4_regression_check](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_sgfp4_regression_check)**(self self, str niche_name, dict current_scores) |
| | **[_find_previous_canonical](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#function-_find_previous_canonical)**(self self, str niche_name) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_project_root](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#variable-_project_root)**  |
| | **[_evaluator](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#variable-_evaluator)**  |
| str | **[_benchmarks_dir](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#variable-_benchmarks_dir)**  |
| | **[_config](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#variable-_config)**  |
| | **[_metric_store](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#variable-_metric_store)**  |
| str | **[_gate_state_dir](/python-reference/Classes/d6/db5/classeval_1_1benchmarker_1_1_benchmarker/#variable-_gate_state_dir)**  |

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Optional project_root[Path] =None,
    Optional evaluator[SpecialistEvaluator] =None,
    Optional config[dict] =None
)
```


### function compare_variants

```python
dict compare_variants(
    self self,
    str niche_name,
    list variant_results
)
```


### function save_comparison

```python
save_comparison(
    self self,
    str niche_name,
    dict comparison
)
```


### function print_comparison_table

```python
print_comparison_table(
    self self,
    dict comparison
)
```


### function gate_check

```python
dict gate_check(
    self self,
    str niche_name,
    dict config =None
)
```




```
Evaluate SGFP4 quantization metrics against configurable thresholds.

Follows the Phase 2 auto-gating pattern: each gate dimension has a
numeric threshold and a consecutive-failure count that triggers
a blocking state.

Args:
    niche_name: Specialist niche to evaluate.
    config: Effective config dict containing the ``eval_gates`` block.
        If None, uses ``self._config`` set during construction.

Returns:
    Dict with keys: ``niche``, ``passed``, ``checks``, ``blocking``,
    ``consecutive_failures``, and ``detail``.
```


### function composite_2_of_3

```python
dict composite_2_of_3(
    self self,
    bool scores_pass,
    bool regression_pass,
    bool deviation_pass,
    bool scores_evaluated =True,
    bool regression_evaluated =True,
    bool deviation_evaluated =True
)
```




```
D-08 composite gate: passes when at least 2 of 3 dimensions pass.

WR-08: on a specialist's first run, regression (no previous run) and
deviation (no baseline) default-pass. Without tracking which dims were
actually measured, the composite reports ``passed_count = 3`` and the
gate can report "all green" without having measured 2 of the 3
dimensions. The ``evaluated`` flag per dimension surfaces this so
downstream consumers can distinguish "passed by measurement" from
"passed by absence of data". The composite still requires >= 2 passing
dims (D-08 contract unchanged), but each dimension dict now carries an
``evaluated`` flag.

Args:
    scores_pass: True iff ALL blocking benchmark scores >= hard_floor.
    regression_pass: True iff regression from previous run <= threshold.
    deviation_pass: True iff deviation from baseline <= threshold.
    scores_evaluated: True iff the scores dimension was actually
        measured (always True in practice -- hard floors always run).
    regression_evaluated: True iff a previous run existed to compare
        against. False on first run.
    deviation_evaluated: True iff an internal baseline existed. False
        when ``MissingBaselineError`` was caught.

Returns:
    Dict with ``passed`` (bool), ``passed_count`` (int 0-3),
    ``evaluated_count`` (int 0-3), and ``dimensions`` mapping each
    dimension name to {passed, evaluated, detail}.
```


### function gate_check_benchmarks

```python
dict gate_check_benchmarks(
    self self,
    str niche_name,
    Optional benchmark_results_path[Path] =None,
    Optional config[dict] =None
)
```




```
Evaluate canonical benchmark results against quality gates.

Implements D-06 (tiered gating), D-07 (internal baseline deviation),
D-08 (hard floors + 2-of-3 composite + mandatory SGFP4 regression),
D-09 (bootstrap placeholder).

Args:
    niche_name: Specialist niche.
    benchmark_results_path: Optional explicit path to results JSON.
        If None, the most recent canonical result is loaded.
    config: Optional effective config dict. If None, uses self._config.

Returns:
    Dict with keys: ``niche``, ``passed``, ``checks`` (per-benchmark),
    ``blocking``, ``consecutive_failures``, ``composite_result``,
    ``sgfp4_regression``, and ``detail``.
```


## Protected Functions Documentation

### function _check_dimension

```python
static _check_dimension(
    str dim_name,
    float actual_value,
    dict dim_config
)
```




```
Check a single gate dimension.

Args:
    dim_name: Dimension name (fp4_mse, fp4_effective_bitrate, fp4_t158_ratio).
    actual_value: Measured value from metrics.
    dim_config: Threshold config dict (max/min + consecutive_failures_to_block).

Returns:
    Tuple of (passed: bool, detail: str).
```


### function _update_consecutive_failures

```python
static dict _update_consecutive_failures(
    dict prev_state,
    dict now_failures
)
```




```
Update consecutive failure counters.

For each dimension: increment the counter if it failed this check,
reset to 0 if it passed.

Args:
    prev_state: Previous gate state dict (may be empty).
    now_failures: Current check results: {dim_name: 1 if failed, 0 if passed}.

Returns:
    Updated consecutive_failures dict.
```


### function _gate_state_path

```python
Path _gate_state_path(
    self self,
    str niche_name
)
```




```
Return the gate state file path for a niche.```


### function _load_gate_state

```python
dict _load_gate_state(
    self self,
    str niche_name
)
```




```
Load the persisted gate state for a niche.

T-03-11 mitigation: Corrupt state files are caught and recreated fresh.
Gate defaults to passing when state is unreadable (fail-open for POC).

Returns:
    Gate state dict, or empty dict if no state exists or state is corrupt.
```


### function _save_gate_state

```python
_save_gate_state(
    self self,
    str niche_name,
    dict consecutive_failures,
    list checks
)
```




```
Persist gate state for a niche.

Stores consecutive failure counters and a truncated history of recent
gate check results (max 20 entries).

T-03-13 mitigation: Gate state stored in artifacts/.gate_state/ which
is not user-writable during normal pipeline execution.

Args:
    niche_name: Specialist niche name.
    consecutive_failures: Updated failure counters dict.
    checks: Current gate check results list.
```


### function _load_yaml

```python
dict _load_yaml(
    self self,
    Path path
)
```




```
Load a YAML file; returns {} if missing. Import yaml lazily.

Args:
    path: YAML file path.

Returns:
    Parsed dict, or empty dict if the file does not exist.
```


### function _load_specialist_mapping

```python
dict _load_specialist_mapping(
    self self,
    str niche_name
)
```




```
Load blocking/diagnostic benchmark lists for a specialist (D-05).

Args:
    niche_name: Specialist niche key.

Returns:
    Dict with ``blocking_benchmarks`` and ``diagnostic_benchmarks`` lists.
    Returns empty lists if the mapping file or specialist is absent.
```


### function _load_benchmark_threshold

```python
dict _load_benchmark_threshold(
    self self,
    str benchmark_name
)
```




```
Load per-benchmark threshold config (D-08 hard_floor, regression, deviation).

Args:
    benchmark_name: Benchmark identifier.

Returns:
    Dict with ``hard_floor``, ``regression_max_pct``, ``deviation_max_pct``.
    Defaults: hard_floor=0.0, regression_max_pct=0.10, deviation_max_pct=0.20.
```


### function _bench_gate_state_path

```python
Path _bench_gate_state_path(
    self self,
    str niche_name
)
```




```
Return the BENCHMARK gate state file path (separate from SGFP4 state).```


### function _load_bench_gate_state

```python
dict _load_bench_gate_state(
    self self,
    str niche_name
)
```




```
Load benchmark gate state. Fail-open on corrupt files (Phase 3 pattern).```


### function _save_bench_gate_state

```python
_save_bench_gate_state(
    self self,
    str niche_name,
    dict consecutive_failures,
    list checks
)
```




```
Persist benchmark gate state with history (T-04-12 audit trail).```


### function _find_canonical_results

```python
_find_canonical_results(
    self self,
    str niche_name,
    bool quantized_only =False
)
```




```
Find the most recent canonical-mode benchmark results JSON for a niche.

Per D-03: diagnostic-mode results are NEVER used for gating.

The producer contract (``BenchmarkRunner.run_benchmarks`` /
``MetricStore.record_benchmark_results``) writes files named
``{niche}_{benchmark}_{ts}.json`` -- there is no ``canonical`` or
``quantized`` token in the filename. Instead we glob the producer
pattern and filter by the payload ``mode`` and ``quantized`` fields.
``_baseline`` / ``_comparison`` / ``_sgfp4_metrics`` sibling files
are excluded by stem.

Args:
    niche_name: Specialist niche.
    quantized_only: If True, restrict to quantized model results only
        (payload ``quantized`` is True or absent-but-not-explicitly-False
        for backward compatibility).

Returns:
    Parsed results dict, or None if no canonical result found.
```


### function _load_baseline_scores

```python
dict _load_baseline_scores(
    self self,
    str niche_name
)
```




```
Load internal untrained-backbone baseline scores (D-07).

The baseline is the untrained backbone model run through the same
benchmarks -- the floor against which deviation is measured.

Args:
    niche_name: Specialist niche.

Returns:
    Dict of {benchmark_name: score}.

Raises:
    MissingBaselineError: If no baseline file exists for the niche.
```


### function _extract_score

```python
float _extract_score(
    self self,
    result_entry result_entry
)
```




```
Extract a scalar score from a benchmark result entry.

Handles both ``{"score": x}`` and ``{"pass@1": x}`` schemas.

Args:
    result_entry: Dict from benchmark results.

Returns:
    Scalar score, or 0.0 if no score key is found.
```


### function _sgfp4_regression_check

```python
dict _sgfp4_regression_check(
    self self,
    str niche_name,
    dict current_scores
)
```




```
D-08 mandatory SGFP4 regression check.

Compares unquantized adapter benchmark scores against SGFP4 quantized
model scores. Isolates "model got worse because of training" from
"model got worse because SGFP4 damaged it."

Per D-09: a full bootstrap CI is the target; here we use a simple
per-benchmark percentage threshold as a placeholder and flag
``needs_bootstrap: true`` for Plan 04-04 to upgrade.

Args:
    niche_name: Specialist niche.
    current_scores: Current (quantized) results dict {benchmark: entry}.

Returns:
    Dict with ``passed`` (bool), ``deltas`` ({benchmark: delta}),
    ``needs_bootstrap`` (bool), and ``detail`` (str).

Note:
    Does NOT block on first run when no unquantized baseline exists.
```


### function _find_previous_canonical

```python
_find_previous_canonical(
    self self,
    str niche_name
)
```




```
Find the most recent canonical quantized result from the PREVIOUS run.

Per CR-01: glob the producer pattern ``{niche}_*_*.json`` and filter by
the payload ``mode == "canonical"`` AND ``quantized`` field (defaulting
True for backward compat). ``_baseline`` / ``_comparison`` /
``_sgfp4_metrics`` sibling files are excluded.

Per WR-07: a single benchmark *run* writes one file per task sharing
the same ``run_id`` (or ``timestamp_utc`` for legacy records without
``run_id``). Grouping by run_id avoids the earlier bug where the
"second-most-recent file" was likely a sibling task from the SAME run
rather than the previous run -- making the regression delta
meaningless. We pick the most recent run as "current" and the
next-most-recent distinct run as "previous", returning the first
canonical quantized payload from that previous run.

Returns None if fewer than two distinct runs exist.
```


## Protected Attributes Documentation

### variable _project_root

```python
_project_root =  project_root;
```


### variable _evaluator

```python
_evaluator =  evaluator or SpecialistEvaluator(project_root);
```


### variable _benchmarks_dir

```python
str _benchmarks_dir =  project_root / "artifacts" / "benchmarks";
```


### variable _config

```python
_config =  config or {};
```


### variable _metric_store

```python
_metric_store =  MetricStore(project_root);
```


### variable _gate_state_dir

```python
str _gate_state_dir =  project_root / "artifacts" / ".gate_state";
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700