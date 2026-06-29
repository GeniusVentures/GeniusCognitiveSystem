---
title: quantize::quadtree

---

# quantize::quadtree



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[quantize::quadtree::QuadtreeEncoder](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/)**  |

## Detailed Description




```
Quadtree adaptive block-size encoder for SGFP4 v2.

Per D-01: Full quadtree implementation. Encode tries largest block first
(64x64), measures Laplacian-weighted error, splits into 4 children if error
exceeds configurable threshold, recurses down to min_block_size (default 4x4).

The encoder is designed to be consumed by FP4Exporter. It accepts callable
hooks for FP4_AFFINE and T158_AFFINE fitting, keeping the quadtree logic
independent of the specific encode implementation.

Dual-mode selection per D-04: prefer T158 when t158_error <= (1.0 + delta) * fp4_error.
Per-weight max error guard per RESEARCH.md Pitfall 4: if any individual weight
reconstruction error exceeds 5 * scale, reject T158 and force FP4_AFFINE.

Hysteresis per RESEARCH.md Pitfall 1: if parent block was accepted, require child
error to be <= threshold * 0.8 (20% improvement) before splitting. Allow 10% slack
(accept if error <= threshold * 1.1) when min_block_size not yet reached.

Max recursion depth = 4 levels (64->32->16->8->4). Raises ValueError if exceeded.
```






-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700