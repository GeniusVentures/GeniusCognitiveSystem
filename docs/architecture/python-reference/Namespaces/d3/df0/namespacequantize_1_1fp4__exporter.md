---
title: quantize::fp4_exporter

---

# quantize::fp4_exporter



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[quantize::fp4_exporter::FP4Exporter](/python-reference/Classes/d6/d9b/classquantize_1_1fp4__exporter_1_1_f_p4_exporter/)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| int | **[MACROBLOCK_SIZE](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-macroblock_size)**  |
| int | **[PAYLOAD_BYTES](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-payload_bytes)**  |
| int | **[PAYLOAD_U32](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-payload_u32)**  |
| int | **[ALIGNMENT](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-alignment)**  |
| int | **[MODE_FP4_AFFINE](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-mode_fp4_affine)**  |
| int | **[MODE_T158_AFFINE](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-mode_t158_affine)**  |
| str | **[SGFP4_MAGIC](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-sgfp4_magic)**  |
| int | **[SGFP4_VERSION_V2](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-sgfp4_version_v2)**  |
| int | **[LAYOUT_UNIFORM_64](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-layout_uniform_64)**  |
| int | **[LAYOUT_UNIFORM_32](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-layout_uniform_32)**  |
| int | **[LAYOUT_UNIFORM_16](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-layout_uniform_16)**  |
| int | **[LAYOUT_UNIFORM_8](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-layout_uniform_8)**  |
| int | **[LAYOUT_MIXED](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-layout_mixed)**  |
| int | **[LAYOUT_FULL_4x4](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-layout_full_4x4)**  |
| dict | **[DEFAULT_V2_THRESHOLDS](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-default_v2_thresholds)**  |
| | **[parser](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-parser)**  |
| | **[required](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-required)**  |
| | **[True](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-true)**  |
| | **[help](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-help)**  |
| | **[action](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-action)**  |
| | **[args](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-args)**  |
| | **[project_root](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-project_root)**  |
| | **[exporter](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-exporter)**  |
| float | **[dummy_weights](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-dummy_weights)**  |
| str | **[output_dir](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-output_dir)**  |
| | **[bin_path](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-bin_path)**  |
| | **[stats](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-stats)**  |
| | **[adaptive](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-adaptive)**  |
| dict | **[manifest](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-manifest)**  |
| | **[f](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-f)**  |
| | **[indent](/python-reference/Namespaces/d3/df0/namespacequantize_1_1fp4__exporter/#variable-indent)**  |

## Detailed Description




```
FP4 Ultra binary exporter — SGFP4 v1 (fixed 64x64) and v2 (adaptive quadtree).

Container layout v1: headers[B] | offsets[B] | codes_blob[B*2048]
Container layout v2: magic[4] | version[1] | num_superblocks[4] |
                     superblock_offsets[B] | superblock_data[0..B-1]

v1 (fixed, backward compatible):
- 64x64 macroblocks
- Fixed 2048-byte payload per block
- FP4_AFFINE (mode 0): 4-bit signed codes, 8 per uint32
- T158_AFFINE (mode 1): ternary as 2-bit symbols, 16 per uint32

v2 (adaptive, SGFP4 v2):
- Variable block sizes 4x4..64x64 selected by quadtree + Laplacian error
- Layout enum per superblock (0-5) identifies block structure
- Variable payloads scale with block area
- Dual-mode per-block: FP4_AFFINE vs T158_AFFINE via error comparison
- 4-byte magic header (b'SGF4') + version byte (0x02) for format detection
- Superblock offset table for paging
- 16-byte payload alignment per block
- Manifest generation via ManifestBuilder
```



## Attributes Documentation

### variable MACROBLOCK_SIZE

```python
int MACROBLOCK_SIZE =  64;
```


### variable PAYLOAD_BYTES

```python
int PAYLOAD_BYTES =  2048;
```


### variable PAYLOAD_U32

```python
int PAYLOAD_U32 =  PAYLOAD_BYTES // 4;
```


### variable ALIGNMENT

```python
int ALIGNMENT =  16;
```


### variable MODE_FP4_AFFINE

```python
int MODE_FP4_AFFINE =  0;
```


### variable MODE_T158_AFFINE

```python
int MODE_T158_AFFINE =  1;
```


### variable SGFP4_MAGIC

```python
str SGFP4_MAGIC =  b'SGF4';
```


### variable SGFP4_VERSION_V2

```python
int SGFP4_VERSION_V2 =  0x02;
```


### variable LAYOUT_UNIFORM_64

```python
int LAYOUT_UNIFORM_64 =  0;
```


### variable LAYOUT_UNIFORM_32

```python
int LAYOUT_UNIFORM_32 =  1;
```


### variable LAYOUT_UNIFORM_16

```python
int LAYOUT_UNIFORM_16 =  2;
```


### variable LAYOUT_UNIFORM_8

```python
int LAYOUT_UNIFORM_8 =  3;
```


### variable LAYOUT_MIXED

```python
int LAYOUT_MIXED =  4;
```


### variable LAYOUT_FULL_4x4

```python
int LAYOUT_FULL_4x4 =  5;
```


### variable DEFAULT_V2_THRESHOLDS

```python
dict DEFAULT_V2_THRESHOLDS =  {
    64: {"max_mse": 0.01, "max_relative": 0.05},
    32: {"max_mse": 0.005, "max_relative": 0.03},
    16: {"max_mse": 0.002, "max_relative": 0.02},
    8:  {"max_mse": 0.001, "max_relative": 0.01},
    4:  {"max_mse": 0.0005, "max_relative": 0.005},
};
```


### variable parser

```python
parser =  argparse.ArgumentParser(
        description="Export specialist weights to FP4 Ultra format (v1 or v2)"
    );
```


### variable required

```python
required;
```


### variable True

```python
True;
```


### variable help

```python
help;
```


### variable action

```python
action;
```


### variable args

```python
args =  parser.parse_args();
```


### variable project_root

```python
project_root =  Path(__file__).resolve().parent.parent;
```


### variable exporter

```python
exporter =  FP4Exporter(project_root);
```


### variable dummy_weights

```python
float dummy_weights =  np.random.randn(512, 512).astype(np.float32) * 0.01;
```


### variable output_dir

```python
str output_dir =  project_root / "models" / "specialists_mlx" / args.niche / "fp4";
```


### variable bin_path

```python
bin_path;
```


### variable stats

```python
stats;
```


### variable adaptive

```python
adaptive;
```


### variable manifest

```python
dict manifest =  {
            "model_name": args.niche,
            "niche": args.niche,
            "base_model_ref": "",
            "adapter_ref": "",
            "quantization_params": {"format": "fp4_ultra"},
            "encoder_version": "0.1.0",
            "timestamp_utc": "",
        };
```


### variable f

```python
f;
```


### variable indent

```python
indent;
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700