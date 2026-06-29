---
title: eval::benchmark_config

---

# eval::benchmark_config



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[eval::benchmark_config::ConfigError](/python-reference/Classes/db/d71/classeval_1_1benchmark__config_1_1_config_error/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| Dict[str, Dict[str, Any]] | **[validate_benchmarks_config](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#function-validate_benchmarks_config)**(Path|None config_dir =None) |
| Dict[str, Dict[str, List[str]]] | **[load_specialist_mapping](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#function-load_specialist_mapping)**(Path|None config_dir =None) |
| Tuple[List[str], List[str]] | **[get_benchmarks_for_specialist](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#function-get_benchmarks_for_specialist)**(str specialist, Dict]] mapping[str, Dict[str, List[str]) |
| None | **[check](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#function-check)**(str name, bool condition, str detail ="") |

## Attributes

|                | Name           |
| -------------- | -------------- |
| Path | **[BENCHMARKS_CONFIG_DIR](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-benchmarks_config_dir)**  |
| str | **[SPECIALIST_MAPPING_FILENAME](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-specialist_mapping_filename)**  |
| int | **[passed](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-passed)**  |
| int | **[failed](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-failed)**  |
| Dict[str, Dict[str, Any]] | **[validated](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-validated)**  |
| dict | **[expected_benchmarks](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-expected_benchmarks)**  |
| Dict[str, Dict[str, List[str]]] | **[mapping](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-mapping)**  |
| dict | **[expected_specialists](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-expected_specialists)**  |
| | **[block](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-block)**  |
| | **[diag](/python-reference/Namespaces/d0/da3/namespaceeval_1_1benchmark__config/#variable-diag)**  |

## Detailed Description




```
ConfigLoader extension for per-benchmark YAML configs and specialist mapping.

Per Phase 04-02 Task 2: validates the schema of every per-benchmark config YAML
in ``config/benchmarks/`` (required fields: name, task_name, num_fewshot,
output_type, blocking, hard_floor, regression_max_pct, deviation_max_pct per
D-04/D-08) and loads the specialist-to-benchmark mapping per D-05
(``specialist_mapping.yaml``).

Threat mitigations:
- T-04-06: ``yaml.safe_load`` exclusively — never ``yaml.load`` or full_load.
- T-04-08: ``load_specialist_mapping`` cross-validates referenced benchmark
  names against the validated per-benchmark config set.
```


## Functions Documentation

### function validate_benchmarks_config

```python
Dict[str, Dict[str, Any]] validate_benchmarks_config(
    Path|None config_dir =None
)
```




```
Read and validate every per-benchmark YAML in ``config_dir``.

Each YAML must define all fields in ``BENCHMARK_REQUIRED_FIELDS`` with the
correct type. Threshold fields (hard_floor, regression_max_pct,
deviation_max_pct) must be strictly positive floats. ``num_fewshot`` must
be a non-negative int. ``blocking`` must be a Python ``bool`` (string
"true"/"false" from a misconfigured YAML is rejected).

Args:
    config_dir: Directory containing per-benchmark YAML files. Defaults to
        ``<project_root>/config/benchmarks/``.

Returns:
    Dict mapping ``name`` field -> validated config dict.

Raises:
    ConfigError: If any YAML is missing a required field, has an invalid
        type, or fails a value-range check. The error message names the
        file and the offending field.
    FileNotFoundError: If ``config_dir`` does not exist.
```


### function load_specialist_mapping

```python
Dict[str, Dict[str, List[str]]] load_specialist_mapping(
    Path|None config_dir =None
)
```




```
Load and validate ``specialist_mapping.yaml`` per D-05.

Validates that:
  - the file exists and parses as a YAML mapping,
  - the top-level key is ``specialists``,
  - each specialist entry has both ``blocking_benchmarks`` and
    ``diagnostic_benchmarks`` lists,
  - every referenced benchmark exists in the per-benchmark config set
    (T-04-08 mitigation).

Args:
    config_dir: Directory containing ``specialist_mapping.yaml``. Defaults
        to ``<project_root>/config/benchmarks/``.

Returns:
    Dict mapping specialist name -> {
        "blocking_benchmarks": [...],
        "diagnostic_benchmarks": [...],
    }.

Raises:
    ConfigError: On any schema violation, including a referenced benchmark
        that does not have a per-benchmark config YAML.
    FileNotFoundError: If the file or directory does not exist.
```


### function get_benchmarks_for_specialist

```python
Tuple[List[str], List[str]] get_benchmarks_for_specialist(
    str specialist,
    Dict]] mapping[str, Dict[str, List[str]
)
```




```
Return ``(blocking_benchmarks, diagnostic_benchmarks)`` for a specialist.

Args:
    specialist: Specialist name (e.g. "medical", "code").
    mapping: Loaded specialist mapping (output of ``load_specialist_mapping``).

Returns:
    Tuple of (blocking_benchmarks, diagnostic_benchmarks) lists.

Raises:
    KeyError: If *specialist* is not in *mapping*.
```


### function check

```python
None check(
    str name,
    bool condition,
    str detail =""
)
```



## Attributes Documentation

### variable BENCHMARKS_CONFIG_DIR

```python
Path BENCHMARKS_CONFIG_DIR =  _PROJECT_ROOT / "config" / "benchmarks";
```


### variable SPECIALIST_MAPPING_FILENAME

```python
str SPECIALIST_MAPPING_FILENAME =  "specialist_mapping.yaml";
```


### variable passed

```python
int passed =  0;
```


### variable failed

```python
int failed =  0;
```


### variable validated

```python
Dict[str, Dict[str, Any]] validated =  validate_benchmarks_config();
```


### variable expected_benchmarks

```python
dict expected_benchmarks =  {"mmlu", "humaneval", "medmcqa", "gpqa", "pubmedqa", "bigpatent"};
```


### variable mapping

```python
Dict[str, Dict[str, List[str]]] mapping =  load_specialist_mapping();
```


### variable expected_specialists

```python
dict expected_specialists =  {"code", "medical", "qa_technical", "encyclopedic", "patents"};
```


### variable block

```python
block;
```


### variable diag

```python
diag;
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700