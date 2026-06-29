---
title: eval::benchmark_runner

---

# eval::benchmark_runner



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[eval::benchmark_runner::BenchmarkRunner](/python-reference/Classes/df/d1b/classeval_1_1benchmark__runner_1_1_benchmark_runner/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| List[str] | **[build_task_list](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#function-build_task_list)**(str niche, str mode) |
| dict | **[collect_fingerprint_fields](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#function-collect_fingerprint_fields)**(str task_name, str task_revision, str dataset_revision, str prompt_hash, int fewshot_seed, str chat_template_hash, str answer_extraction, dict generation_params) |
| None | **[validate_results_schema](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#function-validate_results_schema)**(dict data) |
| None | **[main](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#function-main)**() |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[logger](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#variable-logger)**  |
| dict | **[CANONICAL_PARAMS](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#variable-canonical_params)**  |
| dict | **[SPECIALIST_BENCHMARKS](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#variable-specialist_benchmarks)**  |
| frozenset | **[kNotImplementedBenchmarks](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#variable-knotimplementedbenchmarks)**  |
| dict | **[kBenchmarkFewShot](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#variable-kbenchmarkfewshot)**  |
| int | **[kDefaultFewShot](/python-reference/Namespaces/db/d3d/namespaceeval_1_1benchmark__runner/#variable-kdefaultfewshot)**  |

## Detailed Description




```
Benchmark runner entry point — invokes lm-eval simple_evaluate() for specialists.

Per D-01 (multi-mode): dataset source (huggingface/local), model backend (MLX).
Per D-02 (reproducibility fingerprint): 11-field fingerprint per benchmark run.
Per D-03 (canonical vs diagnostic): canonical = frozen params, diagnostic = overrides.
Per D-04 (MMLU universal baseline): every specialist runs MMLU, never blocks.
Per D-05 (specialist-benchmark mapping): domain-specific blocking + MMLU diagnostic.

Pipeline invocation: ``python eval/benchmark_runner.py --niche {niche}``

Threat mitigations:
- T-04-02: lm-eval import wrapped in try/except with clear message.
- T-04-05: local dataset paths validated with Path.resolve() prefix check.
```


## Functions Documentation

### function build_task_list

```python
List[str] build_task_list(
    str niche,
    str mode
)
```




```
Build the list of benchmark task names for a specialist niche and mode.

Args:
    niche: Specialist niche name (e.g., "medical", "code").
    mode: "canonical" or "diagnostic" (both include the same tasks,
          differentiated at simple_evaluate() call time params).

Returns:
    List of lm-eval task names (e.g., ["mmlu", "medmcqa", "pubmedqa"]).

Raises:
    ValueError: If *niche* is not in SPECIALIST_BENCHMARKS.
```


### function collect_fingerprint_fields

```python
dict collect_fingerprint_fields(
    str task_name,
    str task_revision,
    str dataset_revision,
    str prompt_hash,
    int fewshot_seed,
    str chat_template_hash,
    str answer_extraction,
    dict generation_params
)
```




```
Collect the 11-field reproducibility fingerprint per D-02.

``model_manifest_sha256`` and ``sgfp4_manifest_sha256`` are stub
placeholders until the fingerprint module is added in Plan 04-03.

Args:
    task_name: lm-eval task name.
    task_revision: Task YAML revision string.
    dataset_revision: Dataset version/pin.
    prompt_hash: SHA256 of the rendered prompt template.
    fewshot_seed: Seed used for few-shot example sampling.
    chat_template_hash: SHA256 of the chat template used.
    answer_extraction: Method name for answer extraction.
    generation_params: Decoding parameters used.

Returns:
    Dict with all 11 fingerprint fields.
```


### function validate_results_schema

```python
None validate_results_schema(
    dict data
)
```




```
Validate that *data* conforms to the benchmark results JSON schema.

Required top-level fields: ``niche``, ``timestamp_utc``, ``model_version``,
``mode``, ``results``. Each entry in ``results`` must have ``score`` and
``per_category``.

Args:
    data: Parsed results dict to validate.

Raises:
    ValueError: If the schema is violated.
```


### function main

```python
None main()
```




```
Parse CLI arguments and run benchmarks for a specialist niche.

Usage: python eval/benchmark_runner.py --niche medical --mode canonical
```



## Attributes Documentation

### variable logger

```python
logger =  logging.getLogger(__name__);
```


### variable CANONICAL_PARAMS

```python
dict CANONICAL_PARAMS =  {
    "temperature": 0.0,
    "do_sample": False,
    "num_fewshot": None,
};
```


### variable SPECIALIST_BENCHMARKS

```python
dict SPECIALIST_BENCHMARKS =  {
    "code": {
        "blocking": ["humaneval", "livecodebench"],
        "diagnostic": ["mmlu"],
    },
    "medical": {
        "blocking": ["medmcqa", "pubmedqa", "medhelm"],
        "diagnostic": ["mmlu"],
    },
    "qa_technical": {
        "blocking": ["gpqa_main_n_shot"],
        "diagnostic": ["mmlu"],
    },
    "encyclopedic": {
        "blocking": ["rag_pipeline_eval"],
        "diagnostic": ["mmlu"],
    },
    "patents": {
        "blocking": ["bigpatent", "uspto_classification"],
        "diagnostic": ["mmlu"],
    },
};
```


### variable kNotImplementedBenchmarks

```python
frozenset kNotImplementedBenchmarks =  frozenset({
    "livecodebench", "medhelm", "rag_pipeline_eval", "uspto_classification",
});
```


### variable kBenchmarkFewShot

```python
dict kBenchmarkFewShot =  {
    "mmlu": 5,
    "humaneval": 0,
    "medmcqa": 5,
    "pubmedqa": 0,
    "gpqa_main_n_shot": 0,
    "bigpatent": 0,
};
```


### variable kDefaultFewShot

```python
int kDefaultFewShot =  0;
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700