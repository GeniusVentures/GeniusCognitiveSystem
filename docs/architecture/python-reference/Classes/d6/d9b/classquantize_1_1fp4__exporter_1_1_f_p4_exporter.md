---
title: quantize::fp4_exporter::FP4Exporter

---

# quantize::fp4_exporter::FP4Exporter



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-__init__)**(self self, Optional project_root[Path] =None) |
| Tuple[bytes, dict] | **[export_weights](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-export_weights)**(self self, np.ndarray weights, str niche_name, bool prefer_ternary =False, float ternary_delta =0.10, bool adaptive =False, Optional]] thresholds[Dict[int, Dict[str, float] =None, int min_block_size =4, int laplacian_levels =3) |
| | **[export_to_file](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-export_to_file)**(self self, np.ndarray weights, str niche_name, Optional output_dir[Path] =None, bool adaptive =False, Optional]] thresholds[Dict[int, Dict[str, float] =None, int min_block_size =4, int laplacian_levels =3, str base_model ="", Optional training_metadata[dict] =None, ** kwargs) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| Tuple[bytes, dict] | **[_export_v1_fixed](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_export_v1_fixed)**(self self, np.ndarray weights, str niche_name, bool prefer_ternary =False, float ternary_delta =0.10) |
| Tuple[bytes, dict] | **[_export_v2_adaptive](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_export_v2_adaptive)**(self self, np.ndarray weights, str niche_name, bool prefer_ternary =False, float ternary_delta =0.10, Optional]] thresholds[Dict[int, Dict[str, float] =None, int min_block_size =4, int laplacian_levels =3) |
| dict | **[_encode_fp4_affine](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_encode_fp4_affine)**(self self, np.ndarray block) |
| dict | **[_encode_t158_affine](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_encode_t158_affine)**(self self, np.ndarray block) |
| dict | **[_encode_fp4_affine_variable](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_encode_fp4_affine_variable)**(self self, np.ndarray region) |
| dict | **[_encode_t158_affine_variable](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_encode_t158_affine_variable)**(self self, np.ndarray region) |
| Tuple[float, float] | **[_fit_affine](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_fit_affine)**(self self, np.ndarray values) |
| Tuple[float, float] | **[_fit_ternary](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_fit_ternary)**(self self, np.ndarray values) |
| int | **[_pack_half2](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_pack_half2)**(self self, float scale, float bias) |
| | **[_write_manifest](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_write_manifest)**(self self, str niche_name, Path bin_path, dict stats, str base_model, dict training_metadata, Path output_dir) |
| int | **[_float_to_half](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_float_to_half)**(float value) |
| int | **[_payload_u32](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_payload_u32)**(int size, int mode) |
| int | **[_classify_layout](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#function-_classify_layout)**(List blocks[dict]) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_root](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#variable-_root)**  |
| str | **[_artifacts_dir](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/#variable-_artifacts_dir)**  |

## Detailed Description

```python
class quantize::fp4_exporter::FP4Exporter;
```




```
SGFP4 weight exporter with v1 fixed and v2 adaptive modes.

Constructor args:
    project_root: Path to the gnus-poc project root. Defaults to parent of
                  this file if None.
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Optional project_root[Path] =None
)
```


### function export_weights

```python
Tuple[bytes, dict] export_weights(
    self self,
    np.ndarray weights,
    str niche_name,
    bool prefer_ternary =False,
    float ternary_delta =0.10,
    bool adaptive =False,
    Optional]] thresholds[Dict[int, Dict[str, float] =None,
    int min_block_size =4,
    int laplacian_levels =3
)
```




```
Export weight tensor to SGFP4 binary.

Args:
    weights: 2D float32 numpy array of shape (O, I).
    niche_name: Specialist niche name (e.g. "code", "medical").
    prefer_ternary: Prefer T158_AFFINE even in v1 mode.
    ternary_delta: D-04 delta for T158 preference.
    adaptive: If True, use SGFP4 v2 adaptive quadtree export.
              If False (default), use v1 fixed 64x64 export.
    thresholds: Per-block-size error thresholds (v2 only).
    min_block_size: Minimum block edge size for quadtree (v2 only).
    laplacian_levels: Max Laplacian pyramid levels (v2 only).

Returns:
    Tuple of (binary bytes, stats dict).
```


### function export_to_file

```python
export_to_file(
    self self,
    np.ndarray weights,
    str niche_name,
    Optional output_dir[Path] =None,
    bool adaptive =False,
    Optional]] thresholds[Dict[int, Dict[str, float] =None,
    int min_block_size =4,
    int laplacian_levels =3,
    str base_model ="",
    Optional training_metadata[dict] =None,
    ** kwargs
)
```




```
Export weights to file, optionally with manifest (v2).

Args:
    weights: 2D float32 numpy array.
    niche_name: Specialist niche name.
    output_dir: Target directory (default: artifacts/fp4/{niche}).
    adaptive: Use v2 adaptive export if True.
    thresholds: Per-block-size error thresholds (v2 only).
    min_block_size: Minimum block edge size (v2 only).
    laplacian_levels: Max Laplacian pyramid levels (v2 only).
    base_model: Base model reference for manifest (v2 only).
    training_metadata: Training metadata dict for manifest (v2 only).
    **kwargs: Additional arguments passed to export_weights.

Returns:
    Tuple of (bin_path, stats).
```


## Protected Functions Documentation

### function _export_v1_fixed

```python
Tuple[bytes, dict] _export_v1_fixed(
    self self,
    np.ndarray weights,
    str niche_name,
    bool prefer_ternary =False,
    float ternary_delta =0.10
)
```




```
v1 fixed 64x64 export — identical to pre-upgrade behavior.```


### function _export_v2_adaptive

```python
Tuple[bytes, dict] _export_v2_adaptive(
    self self,
    np.ndarray weights,
    str niche_name,
    bool prefer_ternary =False,
    float ternary_delta =0.10,
    Optional]] thresholds[Dict[int, Dict[str, float] =None,
    int min_block_size =4,
    int laplacian_levels =3
)
```




```
SGFP4 v2 adaptive quadtree export.

Binary format:
    magic[4] | version[1] | num_superblocks[4] |
    superblock_offsets[B] | superblock_data[0..B-1]

Each superblock:
    superblock_header[4] | block_headers[N*4] | payloads[var]
```


### function _encode_fp4_affine

```python
dict _encode_fp4_affine(
    self self,
    np.ndarray block
)
```




```
v1: encode a 64x64 block in FP4_AFFINE mode (4096 weights).```


### function _encode_t158_affine

```python
dict _encode_t158_affine(
    self self,
    np.ndarray block
)
```




```
v1: encode a 64x64 block in T158_AFFINE mode (4096 weights).```


### function _encode_fp4_affine_variable

```python
dict _encode_fp4_affine_variable(
    self self,
    np.ndarray region
)
```




```
v2: encode a variable-sized region in FP4_AFFINE mode.

Args:
    region: 2D numpy array of any NxN size, float32.

Returns:
    dict with keys: scale, bias, l2_error, payload, n_weights.
```


### function _encode_t158_affine_variable

```python
dict _encode_t158_affine_variable(
    self self,
    np.ndarray region
)
```




```
v2: encode a variable-sized region in T158_AFFINE mode.

Args:
    region: 2D numpy array of any NxN size, float32.

Returns:
    dict with keys: scale, bias, l2_error, payload, n_weights.
```


### function _fit_affine

```python
Tuple[float, float] _fit_affine(
    self self,
    np.ndarray values
)
```




```
Fit affine scale and bias for FP4 encoding (16-candidate search).```


### function _fit_ternary

```python
Tuple[float, float] _fit_ternary(
    self self,
    np.ndarray values
)
```




```
Fit scale and bias for T158 ternary encoding.```


### function _pack_half2

```python
int _pack_half2(
    self self,
    float scale,
    float bias
)
```




```
Pack two FP16 values into a uint32: scale in upper 16 bits, bias in lower.```


### function _write_manifest

```python
_write_manifest(
    self self,
    str niche_name,
    Path bin_path,
    dict stats,
    str base_model,
    dict training_metadata,
    Path output_dir
)
```




```
Write manifest.json using ManifestBuilder (D-10).```


### function _float_to_half

```python
static int _float_to_half(
    float value
)
```




```
Convert float to IEEE 754 half-precision bits via struct.```


### function _payload_u32

```python
static int _payload_u32(
    int size,
    int mode
)
```




```
Return number of uint32 words for a block payload (D-03).

Args:
    size: Block edge size (4, 8, 16, 32, or 64).
    mode: MODE_FP4_AFFINE (0) or MODE_T158_AFFINE (1).

Returns:
    Number of uint32 words in payload.
```


### function _classify_layout

```python
static int _classify_layout(
    List blocks[dict]
)
```




```
Classify superblock layout from quadtree output blocks (D-02).

Args:
    blocks: List of block dicts from QuadtreeEncoder.encode().

Returns:
    Layout enum value (0-5).
```


## Protected Attributes Documentation

### variable _root

```python
_root =  project_root;
```


### variable _artifacts_dir

```python
str _artifacts_dir =  project_root / "artifacts";
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700