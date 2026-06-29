---
title: config::loader::ConfigLoader

---

# config::loader::ConfigLoader



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| None | **[__init__](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-__init__)**(self self, Path project_root) |
| Dict[str, Any] | **[get_effective_config](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-get_effective_config)**(self self, str niche) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| Dict[str, Any] | **[_load_global_config](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_load_global_config)**(self self) |
| Dict[str, Path] | **[_load_specialist_configs](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_load_specialist_configs)**(self self) |
| None | **[_validate](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_validate)**(self self) |
| None | **[_validate_endpoints](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_validate_endpoints)**(self self) |
| None | **[_validate_models](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_validate_models)**(self self) |
| None | **[_validate_teacher](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_validate_teacher)**(self self) |
| None | **[_validate_teacher_benchmark](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_validate_teacher_benchmark)**(self self) |
| None | **[_validate_pipeline_specialists](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_validate_pipeline_specialists)**(self self) |
| None | **[_validate_fp4_export](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_validate_fp4_export)**(self self) |
| None | **[_apply_specialist_overrides](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_apply_specialist_overrides)**(self self, Dict effective[str, Any], Dict specialist_data[str, Any]) |
| Dict[str, Any] | **[_load_yaml](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#function-_load_yaml)**(Path path) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_project_root](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#variable-_project_root)**  |
| Dict[str, Any] | **[_global_config](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#variable-_global_config)**  |
| Dict[str, Dict[str, Any]] | **[_specialist_configs](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/#variable-_specialist_configs)**  |

## Detailed Description

```python
class config::loader::ConfigLoader;
```




```
Loads, validates, and resolves the two-layer pipeline configuration.

On construction, loads config/pipeline.yaml and all config/specialists/*.yaml
files. Validation runs immediately — a ConfigValidationError is raised for
any schema violation.
```

## Public Functions Documentation

### function __init__

```python
None __init__(
    self self,
    Path project_root
)
```


### function get_effective_config

```python
Dict[str, Any] get_effective_config(
    self self,
    str niche
)
```




```
Return the effective config for *niche*, with per-specialist overrides applied.

The effective config starts as a deep copy of the global config. If a
specialist config exists for ``niche``, its values are deep-merged:
dict values merge recursively, lists and scalars replace the global
default.

Raises ConfigValidationError if *niche* is not in ``pipeline.specialists``.
```


## Protected Functions Documentation

### function _load_global_config

```python
Dict[str, Any] _load_global_config(
    self self
)
```


### function _load_specialist_configs

```python
Dict[str, Path] _load_specialist_configs(
    self self
)
```


### function _validate

```python
None _validate(
    self self
)
```


### function _validate_endpoints

```python
None _validate_endpoints(
    self self
)
```


### function _validate_models

```python
None _validate_models(
    self self
)
```


### function _validate_teacher

```python
None _validate_teacher(
    self self
)
```


### function _validate_teacher_benchmark

```python
None _validate_teacher_benchmark(
    self self
)
```


### function _validate_pipeline_specialists

```python
None _validate_pipeline_specialists(
    self self
)
```


### function _validate_fp4_export

```python
None _validate_fp4_export(
    self self
)
```




```
Validate the fp4_export configuration block.

Per D-08: fp4_export is optional (Phase 1/2 may run without quantization).
When present, validates error_thresholds per block size, ternary_delta range,
min_block_size power-of-2, laplacian_levels, and log_mode_enabled type.
```


### function _apply_specialist_overrides

```python
None _apply_specialist_overrides(
    self self,
    Dict effective[str, Any],
    Dict specialist_data[str, Any]
)
```




```
Deep-merge specialist overrides into *effective* config in-place.```


### function _load_yaml

```python
static Dict[str, Any] _load_yaml(
    Path path
)
```


## Protected Attributes Documentation

### variable _project_root

```python
_project_root =  project_root;
```


### variable _global_config

```python
Dict[str, Any] _global_config =  self._load_global_config();
```


### variable _specialist_configs

```python
Dict[str, Dict[str, Any]] _specialist_configs =  self._load_specialist_configs();
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700