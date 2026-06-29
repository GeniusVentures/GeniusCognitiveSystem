---
title: config::loader

---

# config::loader



 [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[config::loader::ConfigValidationError](/python-reference/Classes/dc/d66/classconfig_1_1loader_1_1_config_validation_error/)**  |
| class | **[config::loader::ConfigLoader](/python-reference/Classes/d8/da5/classconfig_1_1loader_1_1_config_loader/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| None | **[check](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#function-check)**(str name, bool condition, str detail ="") |

## Attributes

|                | Name           |
| -------------- | -------------- |
| | **[project_root](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-project_root)**  |
| int | **[passed](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-passed)**  |
| int | **[failed](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-failed)**  |
| | **[loader](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-loader)**  |
| | **[eff_code](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-eff_code)**  |
| | **[eff_med](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-eff_med)**  |
| | **[fp4_export](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-fp4_export)**  |
| | **[saved_fp4](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-saved_fp4)**  |
| | **[saved_et](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-saved_et)**  |
| dict | **[bad_et](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-bad_et)**  |
| | **[saved_64](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-saved_64)**  |
| | **[saved_delta](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-saved_delta)**  |
| | **[saved_mbs](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-saved_mbs)**  |
| | **[saved_fp4_block](/python-reference/Namespaces/d7/de9/namespaceconfig_1_1loader/#variable-saved_fp4_block)**  |

## Detailed Description




```
ConfigLoader — centralized YAML config loading, validation, and per-specialist override resolution.

Loads the two-layer pipeline configuration (endpoints + models) from config/pipeline.yaml,
validates the schema, and deep-merges per-specialist overrides from config/specialists/<niche>.yaml.

Usage:
    loader = ConfigLoader(Path("."))
    code_config = loader.get_effective_config("code")
```


## Functions Documentation

### function check

```python
None check(
    str name,
    bool condition,
    str detail =""
)
```



## Attributes Documentation

### variable project_root

```python
project_root =  Path(__file__).resolve().parent.parent;
```


### variable passed

```python
int passed =  0;
```


### variable failed

```python
int failed =  0;
```


### variable loader

```python
loader =  ConfigLoader(project_root);
```


### variable eff_code

```python
eff_code =  loader.get_effective_config("code");
```


### variable eff_med

```python
eff_med =  loader.get_effective_config("medical");
```


### variable fp4_export

```python
fp4_export =  loader._global_config.get("fp4_export", {});
```


### variable saved_fp4

```python
saved_fp4 =  loader._global_config.get("fp4_export");
```


### variable saved_et

```python
saved_et =  dict(saved_fp4["error_thresholds"]);
```


### variable bad_et

```python
dict bad_et =  {k: v for k, v in saved_et.items() if k != 4 and k != "4"};
```


### variable saved_64

```python
saved_64 =  dict(saved_et.get(64, saved_et.get("64", {})));
```


### variable saved_delta

```python
saved_delta =  saved_fp4.get("ternary_delta");
```


### variable saved_mbs

```python
saved_mbs =  saved_fp4.get("min_block_size");
```


### variable saved_fp4_block

```python
saved_fp4_block =  loader._global_config.pop("fp4_export", None);
```





-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700