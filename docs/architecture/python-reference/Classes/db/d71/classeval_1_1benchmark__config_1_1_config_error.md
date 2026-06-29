---
title: eval::benchmark_config::ConfigError

---

# eval::benchmark_config::ConfigError



 [More...](#detailed-description)

Inherits from Exception

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| tuple | **[BENCHMARK_REQUIRED_FIELDS](/python-reference/Classes/db/d71/classeval_1_1benchmark__config_1_1_config_error/#variable-benchmark_required_fields)**  |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| tuple | **[_THRESHOLD_FIELDS](/python-reference/Classes/db/d71/classeval_1_1benchmark__config_1_1_config_error/#variable-_threshold_fields)**  |

## Detailed Description

```python
class eval::benchmark_config::ConfigError;
```




```
Raised when a benchmark config or specialist mapping fails validation.

The error message names the file and the missing/invalid field so the
operator can pinpoint the bad YAML without a stack dive.
```

## Public Attributes Documentation

### variable BENCHMARK_REQUIRED_FIELDS

```python
static tuple BENCHMARK_REQUIRED_FIELDS =  (
    ("name", str),
    ("task_name", str),
    ("num_fewshot", int),
    ("output_type", str),
    ("blocking", bool),
    ("hard_floor", float),
    ("regression_max_pct", float),
    ("deviation_max_pct", float),
);
```


## Protected Attributes Documentation

### variable _THRESHOLD_FIELDS

```python
static tuple _THRESHOLD_FIELDS =  (
    "hard_floor",
    "regression_max_pct",
    "deviation_max_pct",
);
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700