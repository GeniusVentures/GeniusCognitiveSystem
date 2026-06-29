---
title: quantize::laplacian

---

# quantize::laplacian



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[quantize::laplacian::LaplacianWeightedError](/python-reference/Classes/de/dde/classquantize_1_1laplacian_1_1_laplacian_weighted_error/)**  |

## Detailed Description




```
Encode-side Laplacian pyramid error analysis for weight quantization.

Per D-07: Laplacian pyramid analysis is encode-side only -- NOT decoded at runtime.
Separates low-frequency structure from high-frequency residual error,
preventing outliers from dominating per-block scale and making T158 more
viable on residuals near zero.

Adapts pyramid levels to block size (per RESEARCH.md Pitfall 2):
- 4x4 and 8x8 blocks: skip Laplacian entirely, use plain L2 (MSE)
- 16x16 blocks: 1 level
- 32x32 blocks: 2 levels
- 64x64 blocks: 3 levels

Uses scipy.ndimage.gaussian_filter for Gaussian smoothing with
configurable sigma and mode parameters.
```






-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700