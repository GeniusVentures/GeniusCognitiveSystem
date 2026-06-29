---
title: GNUS-NEO-SWARM/gnus-poc/quantize/__init__.py

---

# GNUS-NEO-SWARM/gnus-poc/quantize/__init__.py





## Namespaces

| Name           |
| -------------- |
| **[quantize](/python-reference/Namespaces/d1/d35/namespacequantize/)**  |




## Source code

```python
"""GNUS-POC quantization — FP4 binary export for C++ engine.

Provides:
- FP4Exporter: SGFP4 v1 fixed 64x64 and v2 adaptive quadtree export
- ManifestBuilder: provenance manifest generation with SHA256 hashing
- LaplacianWeightedError: encode-side Laplacian pyramid error analysis
- QuadtreeEncoder: adaptive block-size selection via quadtree recursion
"""

from quantize.fp4_exporter import FP4Exporter
from quantize.laplacian import LaplacianWeightedError
from quantize.manifest import ManifestBuilder
from quantize.quadtree import QuadtreeEncoder

__all__ = [
    "FP4Exporter",
    "LaplacianWeightedError",
    "ManifestBuilder",
    "QuadtreeEncoder",
]
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700
