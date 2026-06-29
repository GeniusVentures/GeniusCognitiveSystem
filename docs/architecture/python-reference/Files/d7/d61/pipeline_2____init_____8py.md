---
title: GNUS-NEO-SWARM/gnus-poc/pipeline/__init__.py

---

# GNUS-NEO-SWARM/gnus-poc/pipeline/__init__.py





## Namespaces

| Name           |
| -------------- |
| **[pipeline](/python-reference/Namespaces/db/d27/namespacepipeline/)**  |




## Source code

```python
"""GNUS-POC pipeline orchestration — stage runner, checkpoint validator, and experiment tracker."""

from pipeline.checkpoint import CheckpointValidator, StageValidationResult

__all__ = ["CheckpointValidator", "StageValidationResult"]
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700
