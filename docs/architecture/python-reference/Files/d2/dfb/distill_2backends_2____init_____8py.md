---
title: GNUS-NEO-SWARM/gnus-poc/distill/backends/__init__.py

---

# GNUS-NEO-SWARM/gnus-poc/distill/backends/__init__.py





## Namespaces

| Name           |
| -------------- |
| **[distill](/python-reference/Namespaces/dc/db8/namespacedistill/)**  |
| **[distill::backends](/python-reference/Namespaces/d7/dd3/namespacedistill_1_1backends/)**  |




## Source code

```python
"""Teacher API backends — OpenAI and Anthropic SDK integrations."""

from distill.backends.base import TeacherBackend
from distill.backends.openai_backend import OpenAIBackend
from distill.backends.anthropic_backend import AnthropicBackend

__all__ = [
    "TeacherBackend",
    "OpenAIBackend",
    "AnthropicBackend",
]
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700
