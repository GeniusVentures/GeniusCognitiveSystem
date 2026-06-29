---
title: quantize::quadtree::QuadtreeEncoder

---

# quantize::quadtree::QuadtreeEncoder



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#function-__init__)**(self self, Dict] thresholds[int, Dict[str, float], float ternary_delta, Callable fit_fp4, Callable fit_t158, [laplacian](/python-reference/Namespaces/d4/d78/namespacequantize_1_1laplacian/) laplacian, int min_block_size =4) |
| List[dict] | **[encode](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#function-encode)**(self self, np.ndarray superblock_64x64) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| List[dict] | **[_try_block](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#function-_try_block)**(self self, np.ndarray superblock, int y, int x, int size, bool parent_accepted, int depth =0) |
| np.ndarray | **[_reconstruct](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#function-_reconstruct)**(np.ndarray region, dict result) |
| bool | **[_t158_has_outlier](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#function-_t158_has_outlier)**(np.ndarray region, dict t158_result) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_thresholds](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#variable-_thresholds)**  |
| | **[_ternary_delta](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#variable-_ternary_delta)**  |
| | **[_fit_fp4](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#variable-_fit_fp4)**  |
| | **[_fit_t158](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#variable-_fit_t158)**  |
| | **[_laplacian](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#variable-_laplacian)**  |
| | **[_min_block_size](/python-reference/Classes/d6/daa/classquantize_1_1quadtree_1_1_quadtree_encoder/#variable-_min_block_size)**  |

## Detailed Description

```python
class quantize::quadtree::QuadtreeEncoder;
```




```
Encode a 64x64 superblock into variable-sized blocks using quadtree recursion.

Constructor args:
    thresholds: Dict mapping block_size (int) -> {"max_mse": float, "max_relative": float}.
                Thresholds per block size for split decisions.
    ternary_delta: D-04 delta value for T158 preference:
                   prefer T158 when t158_err <= (1.0 + delta) * fp4_err.
    min_block_size: Minimum block edge size. Must be in {4, 8, 16, 32, 64}.
                    Default: 4.
    fit_fp4: Callable(region: np.ndarray) -> dict.
             Must return {scale, bias, l2_error, payload, n_weights}.
    fit_t158: Callable(region: np.ndarray) -> dict.
              Must return {scale, bias, l2_error, payload, n_weights}.
    laplacian: LaplacianWeightedError instance for error computation.
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Dict] thresholds[int, Dict[str, float],
    float ternary_delta,
    Callable fit_fp4,
    Callable fit_t158,
    laplacian laplacian,
    int min_block_size =4
)
```


### function encode

```python
List[dict] encode(
    self self,
    np.ndarray superblock_64x64
)
```




```
Encode a 64x64 superblock into a list of block dicts.

Each dict contains: {y, x, size, mode, payload, header, scale, bias, error}.

Args:
    superblock_64x64: 2D numpy array of shape (64, 64), float32.

Returns:
    List of dict, one per leaf block. Blocks cover the full 64x64 area
    without overlap or gaps.
```


## Protected Functions Documentation

### function _try_block

```python
List[dict] _try_block(
    self self,
    np.ndarray superblock,
    int y,
    int x,
    int size,
    bool parent_accepted,
    int depth =0
)
```




```
Recursive quadtree encode. Returns list of block dicts.

Args:
    superblock: The full 64x64 superblock array.
    y: Top-left row of this block.
    x: Top-left column of this block.
    size: Edge size of this block (power of 2).
    parent_accepted: Whether the parent block was accepted
                     (used for hysteresis).
    depth: Current recursion depth.

Returns:
    List of dict, one per leaf block.
```


### function _reconstruct

```python
static np.ndarray _reconstruct(
    np.ndarray region,
    dict result
)
```




```
Reconstruct a region from encode result for error computation.```


### function _t158_has_outlier

```python
static bool _t158_has_outlier(
    np.ndarray region,
    dict t158_result
)
```




```
Check if any individual weight error exceeds kT158MaxPerWeightErrorScale * scale.```


## Protected Attributes Documentation

### variable _thresholds

```python
_thresholds =  thresholds;
```


### variable _ternary_delta

```python
_ternary_delta =  ternary_delta;
```


### variable _fit_fp4

```python
_fit_fp4 =  fit_fp4;
```


### variable _fit_t158

```python
_fit_t158 =  fit_t158;
```


### variable _laplacian

```python
_laplacian =  laplacian;
```


### variable _min_block_size

```python
_min_block_size =  min_block_size;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700