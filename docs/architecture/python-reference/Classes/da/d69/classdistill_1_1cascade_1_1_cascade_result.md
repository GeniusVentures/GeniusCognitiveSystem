---
title: distill::cascade::CascadeResult

---

# distill::cascade::CascadeResult



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#function-__init__)**(self self, [final_content](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-final_content) final_content, [level1_confidence](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-level1_confidence) level1_confidence, [level2_confidence](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-level2_confidence) level2_confidence, [level1_model](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-level1_model) level1_model, [escalated](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-escalated) escalated, [attempts](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-attempts) attempts, [level2_model](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-level2_model) level2_model =None) |
| | **[to_dict](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#function-to_dict)**(self self) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| | **[final_content](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-final_content)**  |
| | **[level1_confidence](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-level1_confidence)**  |
| | **[level2_confidence](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-level2_confidence)**  |
| | **[level1_model](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-level1_model)**  |
| | **[level2_model](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-level2_model)**  |
| | **[escalated](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-escalated)**  |
| | **[attempts](/python-reference/Classes/da/d69/classdistill_1_1cascade_1_1_cascade_result/#variable-attempts)**  |

## Detailed Description

```python
class distill::cascade::CascadeResult;
```




```
Result of a multi-teacher cascade execution.

Attributes:
    final_content: The response text returned to the caller.
    level1_confidence: Confidence score from Level 1 (0.0 if Level 1
        failed with an exception).
    level2_confidence: Confidence score from Level 2, or ``None`` if
        no escalation occurred.
    level1_model: The Level 1 model name.
    level2_model: The Level 2 model that produced ``final_content``,
        or ``None`` if no escalation occurred.
    escalated: ``True`` if Level 2 was invoked.
    attempts: List of per-attempt records, each a dict with keys
        ``model_name``, ``confidence``, and ``error`` (str or None).
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    final_content final_content,
    level1_confidence level1_confidence,
    level2_confidence level2_confidence,
    level1_model level1_model,
    escalated escalated,
    attempts attempts,
    level2_model level2_model =None
)
```


### function to_dict

```python
to_dict(
    self self
)
```




```
Return a JSON-serialisable representation for logging.```


## Public Attributes Documentation

### variable final_content

```python
final_content =  final_content;
```


### variable level1_confidence

```python
level1_confidence =  level1_confidence;
```


### variable level2_confidence

```python
level2_confidence =  level2_confidence;
```


### variable level1_model

```python
level1_model =  level1_model;
```


### variable level2_model

```python
level2_model =  level2_model;
```


### variable escalated

```python
escalated =  escalated;
```


### variable attempts

```python
attempts =  attempts;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700