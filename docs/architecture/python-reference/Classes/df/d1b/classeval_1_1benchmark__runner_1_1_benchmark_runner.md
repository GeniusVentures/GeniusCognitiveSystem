---
title: eval::benchmark_runner::BenchmarkRunner

---

# eval::benchmark_runner::BenchmarkRunner



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-__init__)**(self self, Optional project_root[Path] =None) |
| List[Path] | **[run_benchmarks](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-run_benchmarks)**(self self, str niche, str mode ="canonical", str source ="huggingface", bool force_download =False, bool quantized =True) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| dict | **[_run_lm_eval](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-_run_lm_eval)**(self self, model model, List tasks[str], str mode, dict gen_params, bool force_download =False, Optional num_fewshot[int] =None) |
| dict | **[_build_benchmark_entry](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-_build_benchmark_entry)**(self self, str niche, str timestamp_str, str mode, str source, str task_name, dict raw_results, dict gen_params, dict specialist_config, bool quantized =True, Optional run_id[str] =None) |
| dict | **[_build_fingerprint](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-_build_fingerprint)**(self self, str task_name, dict gen_params, dict specialist_config) |
| dict | **[_build_not_implemented_entry](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-_build_not_implemented_entry)**(self self, str niche, str timestamp_str, str mode, str source, str task_name, bool quantized =True, Optional run_id[str] =None) |
| dict | **[_load_specialist_config](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-_load_specialist_config)**(self self, str niche) |
| Path | **[_default_model_path](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-_default_model_path)**(self self, str niche) |
| None | **[_validate_local_source](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-_validate_local_source)**(self self) |
| Optional[float] | **[_extract_primary_score](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-_extract_primary_score)**(str task_name, dict task_results) |
| Dict[str, float] | **[_extract_per_category](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#function-_extract_per_category)**(str task_name, dict raw_results) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| str | **[_kModelVersionPlaceholder](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#variable-_kmodelversionplaceholder)**  |
| | **[_project_root](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#variable-_project_root)**  |
| str | **[_benchmarks_dir](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/#variable-_benchmarks_dir)**  |

## Detailed Description

```python
class eval::benchmark_runner::BenchmarkRunner;
```




```
Orchestrates benchmark evaluation for a single specialist niche.

Loads the MLX model once (per RESEARCH.md Pitfall 3), invokes
``simple_evaluate()`` with the specialist's task list, and writes
structured results JSON to ``artifacts/benchmarks/``.
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
Initialize the benchmark runner.

Args:
    project_root: Root of the gnus-poc project. Auto-located if None.
```


### function run_benchmarks

```python
List[Path] run_benchmarks(
    self self,
    str niche,
    str mode ="canonical",
    str source ="huggingface",
    bool force_download =False,
    bool quantized =True
)
```




```
Run all benchmarks for a specialist niche and return output paths.

Steps:
1. Load per-specialist config (model_path, quantization params).
2. Create MLXBenchmarkModel once.
3. Build task list from specialist mapping.
4. Call ``simple_evaluate()`` with all tasks.
5. Extract per-benchmark scores and per-category breakdowns.
6. Write results JSON to ``artifacts/benchmarks/``.
7. Return list of output file paths.

Args:
    niche: Specialist niche name (e.g., "medical", "code").
    mode: "canonical" (frozen params per D-03) or "diagnostic"
          (allows overrides from config/benchmarks/<name>.yaml).
    source: "huggingface" (default, via datasets library) or
            "local" (reads from data/benchmarks/).
    force_download: If True, re-download datasets even when cached.
    quantized: If True (default), the run is the SGFP4 quantized model
        -- entries are stamped with ``"quantized": True`` so the
        benchmarker's canonical-quantized gate dimension (D-08) finds
        them. Set False for the unquantized-adapter comparison run
        that the mandatory SGFP4 regression check consumes.

Returns:
    List of Paths to written results JSON files.

Raises:
    NotImplementedError: If source is "api".
    RuntimeError: If lm-eval is not installed.
```


## Protected Functions Documentation

### function _run_lm_eval

```python
dict _run_lm_eval(
    self self,
    model model,
    List tasks[str],
    str mode,
    dict gen_params,
    bool force_download =False,
    Optional num_fewshot[int] =None
)
```




```
Invoke lm-eval simple_evaluate() with the model and task list.

Per T-04-02 mitigation: lm-eval import is wrapped in try/except
with a clear error message.

Args:
    num_fewshot: If not None, applied to ALL tasks in this group
        via ``eval_kwargs["num_fewshot"]``. Per CR-04, tasks must be
        grouped by their fewshot count BEFORE calling this -- a single
        ``simple_evaluate()`` call applies one scalar fewshot value
        across every task in ``tasks``.
```


### function _build_benchmark_entry

```python
dict _build_benchmark_entry(
    self self,
    str niche,
    str timestamp_str,
    str mode,
    str source,
    str task_name,
    dict raw_results,
    dict gen_params,
    dict specialist_config,
    bool quantized =True,
    Optional run_id[str] =None
)
```




```
Build a single benchmark result entry conforming to the D-02 schema.

Extracts the primary score metric and per-category breakdown from
the raw lm-eval results dict.

Args:
    quantized: Whether this run is the SGFP4 quantized model (True)
        or the unquantized-adapter comparison (False). Stamped into
        the payload so the benchmarker's D-08 gate can distinguish
        canonical-quantized (gated) from canonical-unquantized
        (the SGFP4 regression baseline) without relying on filename
        tokens.
```


### function _build_fingerprint

```python
dict _build_fingerprint(
    self self,
    str task_name,
    dict gen_params,
    dict specialist_config
)
```




```
Build the D-02 reproducibility fingerprint for a benchmark entry.

WR-01: prefer ``benchmark_fingerprint.compute_fingerprint`` (Plan
04-03) when the specialist config supplies ``model_manifest_path``
and ``sgfp4_manifest_path``. When manifests are unavailable, FAIL
CLOSED by returning a fingerprint with ``model_manifest_sha256`` /
``sgfp4_manifest_sha256`` set to ``None`` -- ``validate_fingerprint``
then marks ``fingerprint_valid: False`` (T-04-16 pattern) so the
record is visibly flagged as non-reproducible rather than silently
carrying ``"stub"`` placeholders that pass validation.
```


### function _build_not_implemented_entry

```python
dict _build_not_implemented_entry(
    self self,
    str niche,
    str timestamp_str,
    str mode,
    str source,
    str task_name,
    bool quantized =True,
    Optional run_id[str] =None
)
```




```
Build a result entry for a not-yet-implemented benchmark.```


### function _load_specialist_config

```python
dict _load_specialist_config(
    self self,
    str niche
)
```




```
Load per-specialist config from config/specialists/{niche}.yaml.```


### function _default_model_path

```python
Path _default_model_path(
    self self,
    str niche
)
```




```
Return the default model path for a niche.```


### function _validate_local_source

```python
None _validate_local_source(
    self self
)
```




```
Validate that the local benchmarks data directory exists.

T-04-05 mitigation: Validate local dataset paths are within
data/benchmarks/ using Path.resolve() + prefix check.
```


### function _extract_primary_score

```python
static Optional[float] _extract_primary_score(
    str task_name,
    dict task_results
)
```




```
Extract the primary metric score from raw lm-eval task results.

Prefers metrics in order: ``acc_norm``, ``acc``, ``pass@1``, ``f1``,
``exact_match``, ``rouge1``, ``rougeL``, ``rouge``. Returns None if no
metric found or results are empty.

Note (CR-05): BIGPATENT (patents blocking benchmark) declares
``metric_list: [rouge1, rougeL]`` in bigpatent.yaml. Without rouge in
the preferred list, the patents gate always scores ``None`` -> ``0.0``
and fails its ``hard_floor: 0.20`` on every run.
```


### function _extract_per_category

```python
static Dict[str, float] _extract_per_category(
    str task_name,
    dict raw_results
)
```




```
Extract per-category/subject breakdown from raw lm-eval results.

For MMLU group tasks, extracts per-subject ``mmlu_<subject>`` entries.
For other tasks, returns an empty dict (per-category not applicable).
```


## Protected Attributes Documentation

### variable _kModelVersionPlaceholder

```python
static str _kModelVersionPlaceholder =  "sgfp4-v2-unknown";
```


### variable _project_root

```python
_project_root =  project_root;
```


### variable _benchmarks_dir

```python
str _benchmarks_dir =  project_root / "artifacts" / "benchmarks";
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700