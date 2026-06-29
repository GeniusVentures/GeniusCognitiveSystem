---
title: config::loader::ConfigValidationError

---

# config::loader::ConfigValidationError



 [More...](#detailed-description)

Inherits from Exception

## Public Functions

|                | Name           |
| -------------- | -------------- |
| None | **[__init__](/python-reference/Classes/dc/d66/classconfig_1_1loader_1_1_config_validation_error/#function-__init__)**(self self, str key_path, str message) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| | **[key_path](/python-reference/Classes/dc/d66/classconfig_1_1loader_1_1_config_validation_error/#variable-key_path)**  |
| | **[message](/python-reference/Classes/dc/d66/classconfig_1_1loader_1_1_config_validation_error/#variable-message)**  |

## Detailed Description

```python
class config::loader::ConfigValidationError;
```




```
Raised when pipeline or specialist config fails schema validation.

The error message includes the YAML key path (e.g., "endpoints.litellm.url")
to help diagnose the exact location of the invalid field.
```

## Public Functions Documentation

### function __init__

```python
None __init__(
    self self,
    str key_path,
    str message
)
```


## Public Attributes Documentation

### variable key_path

```python
key_path =  key_path;
```


### variable message

```python
message =  message;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700