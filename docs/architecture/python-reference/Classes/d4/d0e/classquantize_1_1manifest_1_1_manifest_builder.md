---
title: quantize::manifest::ManifestBuilder

---

# quantize::manifest::ManifestBuilder





## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d4/d0e/classquantize_1_1manifest_1_1_manifest_builder/#function-__init__)**(self self, Optional project_root[Path] =None) |
| dict | **[build](/python-reference/Classes/d4/d0e/classquantize_1_1manifest_1_1_manifest_builder/#function-build)**(self self, str niche_name, str base_model, dict training_metadata, Path fp4_bin_path, dict fp4_stats, Optional eval_results[dict] =None) |
| | **[save](/python-reference/Classes/d4/d0e/classquantize_1_1manifest_1_1_manifest_builder/#function-save)**(self self, dict manifest, str niche_name) |
| | **[save_catalog](/python-reference/Classes/d4/d0e/classquantize_1_1manifest_1_1_manifest_builder/#function-save_catalog)**(self self, list manifests) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| str | **[_file_sha256](/python-reference/Classes/d4/d0e/classquantize_1_1manifest_1_1_manifest_builder/#function-_file_sha256)**(self self, Path path) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_root](/python-reference/Classes/d4/d0e/classquantize_1_1manifest_1_1_manifest_builder/#variable-_root)**  |
| str | **[_artifacts_dir](/python-reference/Classes/d4/d0e/classquantize_1_1manifest_1_1_manifest_builder/#variable-_artifacts_dir)**  |

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    Optional project_root[Path] =None
)
```


### function build

```python
dict build(
    self self,
    str niche_name,
    str base_model,
    dict training_metadata,
    Path fp4_bin_path,
    dict fp4_stats,
    Optional eval_results[dict] =None
)
```


### function save

```python
save(
    self self,
    dict manifest,
    str niche_name
)
```


### function save_catalog

```python
save_catalog(
    self self,
    list manifests
)
```


## Protected Functions Documentation

### function _file_sha256

```python
str _file_sha256(
    self self,
    Path path
)
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