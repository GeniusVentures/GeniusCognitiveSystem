---
title: eval::benchmark_fingerprint

---

# eval::benchmark_fingerprint



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| dict | **[compute_fingerprint](/python-reference/Namespaces/db/d84/namespaceeval_1_1benchmark__fingerprint/#function-compute_fingerprint)**(str task_name, int fewshot_seed, str prompt_template, Path model_manifest_path, Path sgfp4_manifest_path, dict generation_params, Optional task_revision[str] =None, Optional dataset_revision[str] =None, Optional chat_template[str] =None, str answer_extraction ="default") |
| tuple | **[validate_fingerprint](/python-reference/Namespaces/db/d84/namespaceeval_1_1benchmark__fingerprint/#function-validate_fingerprint)**(dict fp) |
| str | **[fingerprint_hash](/python-reference/Namespaces/db/d84/namespaceeval_1_1benchmark__fingerprint/#function-fingerprint_hash)**(dict fp) |
| bool | **[fingerprints_match](/python-reference/Namespaces/db/d84/namespaceeval_1_1benchmark__fingerprint/#function-fingerprints_match)**(dict fp_a, dict fp_b) |

## Attributes

|                | Name           |
| -------------- | -------------- |
| tuple | **[REQUIRED_FIELDS](/python-reference/Namespaces/db/d84/namespaceeval_1_1benchmark__fingerprint/#variable-required_fields)**  |

## Detailed Description




```
Reproducibility fingerprint for canonical benchmark runs (Plan 04-03 Task 1, D-02).

Implements D-02: every canonical benchmark run records an 11-field fingerprint that
identifies the exact harness, prompt, dataset, decoding, and manifest state of the run.
Without this, trend analysis across runs is meaningless -- score drift from unrecorded
prompt or generation-parameter changes cannot be distinguished from real model regressions.

The fingerprint is intentionally lightweight and stdlib-only (hashlib + json + importlib).
All field values are simple scalars/dicts/strings so the fingerprint is JSON-serializable
and stable across processes (sort_keys=True in fingerprint_hash).
```


## Functions Documentation

### function compute_fingerprint

```python
dict compute_fingerprint(
    str task_name,
    int fewshot_seed,
    str prompt_template,
    Path model_manifest_path,
    Path sgfp4_manifest_path,
    dict generation_params,
    Optional task_revision[str] =None,
    Optional dataset_revision[str] =None,
    Optional chat_template[str] =None,
    str answer_extraction ="default"
)
```




```
Compute the 11-field reproducibility fingerprint per D-02.

Args:
    task_name: lm-eval task identifier (e.g. ``medmcqa``).
    fewshot_seed: Integer seed for deterministic few-shot sampling.
    prompt_template: Rendered prompt template string.
    model_manifest_path: Path to the model manifest JSON file (Phase 3 output).
    sgfp4_manifest_path: Path to the SGFP4 quantization manifest JSON file.
    generation_params: Dict of decoding parameters
        (``temperature``, ``do_sample``, ``max_gen_toks``, ``top_p``).
    task_revision: Optional pinned lm-eval task revision; ``None`` if not pinned.
    dataset_revision: Optional pinned dataset revision; ``None`` if not pinned.
    chat_template: Optional chat template string; ``None`` -> hash is ``"none"``.
    answer_extraction: Answer extraction mode (default ``"default"``).

Returns:
    Dict with all 11 D-02 fingerprint fields populated.
```


### function validate_fingerprint

```python
tuple validate_fingerprint(
    dict fp
)
```




```
Validate that a fingerprint dict contains all 11 D-02 fields.

Presence check only; field types are not validated. The two revision
fields (``task_revision``, ``dataset_revision``) are explicitly nullable
per D-02 -- a ``None`` value is valid for them but the key must be present.
All other fields must be present and non-None.

Args:
    fp: Fingerprint dict to validate.

Returns:
    Tuple of ``(is_valid: bool, missing_fields: list[str])``.
```


### function fingerprint_hash

```python
str fingerprint_hash(
    dict fp
)
```




```
SHA256 hex digest of the fingerprint dict (sorted keys for determinism).

Args:
    fp: Fingerprint dict.

Returns:
    Lowercase 64-character hex digest.
```


### function fingerprints_match

```python
bool fingerprints_match(
    dict fp_a,
    dict fp_b
)
```




```
Return True when two fingerprints share an identical fingerprint_hash.

Args:
    fp_a: First fingerprint dict.
    fp_b: Second fingerprint dict.

Returns:
    True iff fingerprint_hash(fp_a) == fingerprint_hash(fp_b).
```



## Attributes Documentation

### variable REQUIRED_FIELDS

```python
tuple REQUIRED_FIELDS =  (
    "harness_commit",
    "task_name",
    "task_revision",
    "dataset_revision",
    "prompt_hash",
    "fewshot_seed",
    "chat_template_hash",
    "answer_extraction",
    "generation_params",
    "model_manifest_sha256",
    "sgfp4_manifest_sha256",
);
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700