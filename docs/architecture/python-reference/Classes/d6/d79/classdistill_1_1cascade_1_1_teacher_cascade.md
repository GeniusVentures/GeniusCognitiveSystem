---
title: distill::cascade::TeacherCascade

---

# distill::cascade::TeacherCascade



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d6/d79/classdistill_1_1cascade_1_1_teacher_cascade/#function-__init__)**(self self, teacher_client teacher_client, benchmark_table benchmark_table, level1_model level1_model, confidence_threshold confidence_threshold) |
| | **[execute](/python-reference/Classes/d6/d79/classdistill_1_1cascade_1_1_teacher_cascade/#function-execute)**(self self, messages messages, domain domain, ** kwargs) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_teacher](/python-reference/Classes/d6/d79/classdistill_1_1cascade_1_1_teacher_cascade/#variable-_teacher)**  |
| | **[_benchmark_table](/python-reference/Classes/d6/d79/classdistill_1_1cascade_1_1_teacher_cascade/#variable-_benchmark_table)**  |
| | **[_level1_model](/python-reference/Classes/d6/d79/classdistill_1_1cascade_1_1_teacher_cascade/#variable-_level1_model)**  |
| | **[_confidence_threshold](/python-reference/Classes/d6/d79/classdistill_1_1cascade_1_1_teacher_cascade/#variable-_confidence_threshold)**  |

## Detailed Description

```python
class distill::cascade::TeacherCascade;
```




```
Multi-teacher cascade orchestrator with benchmark-routed escalation.

Constructor arguments map directly to config values so that
``TeacherClient.__init__`` can wire them from ``pipeline.yaml``::

    cascade = TeacherCascade(
        teacher_client=self,
        benchmark_table=config["teacher_benchmark"],
        level1_model=config["teacher"]["level1"],
        confidence_threshold=config["teacher"]["confidence_threshold"],
    )
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    teacher_client teacher_client,
    benchmark_table benchmark_table,
    level1_model level1_model,
    confidence_threshold confidence_threshold
)
```




```
Initialise the cascade orchestrator.

Args:
    teacher_client: A ``TeacherClient`` instance whose
        ``generate_with_logprobs()`` method is used for all
        teacher calls within the cascade.
    benchmark_table: The ``teacher_benchmark`` dict from
        ``pipeline.yaml`` — domain key → ``{model: score}``.
    level1_model: The always-first teacher model name
        (e.g. ``"deepseek-v4-fast"``).
    confidence_threshold: Minimum logprobs confidence
        (0.0–1.0) to avoid Level 2 escalation.
```


### function execute

```python
execute(
    self self,
    messages messages,
    domain domain,
    ** kwargs
)
```




```
Run the confidence-gated teacher cascade.

Args:
    messages: List of message dicts (OpenAI format).
    domain: Specialist niche name (e.g. ``"code"``, ``"medical"``).
        Mapped to a benchmark table key via ``_DOMAIN_MAP``.
    **kwargs: Extra parameters forwarded to
        ``TeacherClient.generate_with_logprobs()``.

Returns:
    ``CascadeResult`` with the best-available response.

Raises:
    TeacherConfigError: If every teacher in the cascade raises an
        exception (no response could be produced).
```


## Protected Attributes Documentation

### variable _teacher

```python
_teacher =  teacher_client;
```


### variable _benchmark_table

```python
_benchmark_table =  benchmark_table;
```


### variable _level1_model

```python
_level1_model =  level1_model;
```


### variable _confidence_threshold

```python
_confidence_threshold =  confidence_threshold;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700