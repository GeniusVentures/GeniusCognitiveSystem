---
title: GNUS-NEO-SWARM/gnus-poc/distill/__init__.py

---

# GNUS-NEO-SWARM/gnus-poc/distill/__init__.py





## Namespaces

| Name           |
| -------------- |
| **[distill](/python-reference/Namespaces/dc/db8/namespacedistill/)**  |




## Source code

```python
"""GNUS-POC distillation — teacher API client and knowledge distillation."""

from distill.teacher import TeacherClient
from distill.synthetic import SyntheticDataGenerator
from distill.teacher_errors import (
    BudgetExceededError,
    CircuitBreakerOpenError,
    SyntheticDataError,
    TeacherConfigError,
)

__all__ = [
    "TeacherClient",
    "SyntheticDataGenerator",
    "BudgetExceededError",
    "CircuitBreakerOpenError",
    "SyntheticDataError",
    "TeacherConfigError",
]
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700
