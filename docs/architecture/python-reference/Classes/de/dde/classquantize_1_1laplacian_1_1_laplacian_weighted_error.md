---
title: quantize::laplacian::LaplacianWeightedError

---

# quantize::laplacian::LaplacianWeightedError



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/de/dde/classquantize_1_1laplacian_1_1_laplacian_weighted_error/#function-__init__)**(self self, float sigma =2.0, str mode ="reflect") |
| float | **[compute](/python-reference/Classes/de/dde/classquantize_1_1laplacian_1_1_laplacian_weighted_error/#function-compute)**(self self, np.ndarray original_2d, np.ndarray reconstructed_2d, int block_size) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_sigma](/python-reference/Classes/de/dde/classquantize_1_1laplacian_1_1_laplacian_weighted_error/#variable-_sigma)**  |
| | **[_mode](/python-reference/Classes/de/dde/classquantize_1_1laplacian_1_1_laplacian_weighted_error/#variable-_mode)**  |

## Detailed Description

```python
class quantize::laplacian::LaplacianWeightedError;
```




```
Compute Laplacian pyramid-weighted error for encode-side block selection.

Constructor kwargs (documented per RESEARCH.md Pattern 2 for tunability):
    sigma: Base sigma for Gaussian smoothing per level. Actual sigma per
           level is sigma * 2**level. Default: 2.0.
    mode:   Boundary handling mode passed to scipy.ndimage.gaussian_filter.
            Default: 'reflect'.
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    float sigma =2.0,
    str mode ="reflect"
)
```


### function compute

```python
float compute(
    self self,
    np.ndarray original_2d,
    np.ndarray reconstructed_2d,
    int block_size
)
```




```
Compute Laplacian-weighted MSE between original and reconstructed.

Args:
    original_2d: 2D numpy array of original float32 weights.
    reconstructed_2d: 2D numpy array of quantized+dequantized weights.
    block_size: Edge size of the block (4, 8, 16, 32, or 64).

Returns:
    float: Laplacian-weighted MSE. For blocks <= 8x8, returns plain
           MSE (Laplacian skipped per Pitfall 2).
```


## Protected Attributes Documentation

### variable _sigma

```python
_sigma =  sigma;
```


### variable _mode

```python
_mode =  mode;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700