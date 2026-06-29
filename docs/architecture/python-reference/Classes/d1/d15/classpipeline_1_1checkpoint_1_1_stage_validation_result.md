---
title: pipeline::checkpoint::StageValidationResult

---

# pipeline::checkpoint::StageValidationResult



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| Dict[str, Any] | **[to_dict](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/#function-to_dict)**(self self) |
| "StageValidationResult" | **[from_dict](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/#function-from_dict)**(cls cls, Dict data[str, Any]) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| bool | **[passed](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/#variable-passed)**  |
| List | **[checks](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/#variable-checks)**  |
| Optional | **[completed_at](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/#variable-completed_at)**  |
| | **[stage](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/#variable-stage)**  |
| | **[niche](/python-reference/Classes/d1/d15/classpipeline_1_1checkpoint_1_1_stage_validation_result/#variable-niche)**  |

## Detailed Description

```python
class pipeline::checkpoint::StageValidationResult;
```




```
Result of validating a pipeline stage's outputs.

Attributes:
    stage: Stage name (e.g., "train").
    niche: Specialist niche name (e.g., "code").
    passed: Whether all checks passed.
    checks: List of per-check results, each with ``name``, ``passed``, ``detail``.
    completed_at: ISO 8601 timestamp set when checkpoint is written.
```

## Public Functions Documentation

### function to_dict

```python
Dict[str, Any] to_dict(
    self self
)
```




```
Serialize to a JSON-compatible dictionary.```


### function from_dict

```python
"StageValidationResult" from_dict(
    cls cls,
    Dict data[str, Any]
)
```




```
Deserialize from a JSON-compatible dictionary.```


## Public Attributes Documentation

### variable passed

```python
static bool passed =  False;
```


### variable checks

```python
static List checks =  field(default_factory=list);
```


### variable completed_at

```python
static Optional completed_at =  None;
```


### variable stage

```python
stage;
```


### variable niche

```python
niche;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700