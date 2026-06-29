---
title: training::memory

---

# training::memory



 [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| float | **[get_available_ram_gb](/python-reference/Namespaces/dd/d0a/namespacetraining_1_1memory/#function-get_available_ram_gb)**() |
| float | **[estimate_model_memory_gb](/python-reference/Namespaces/dd/d0a/namespacetraining_1_1memory/#function-estimate_model_memory_gb)**(float num_params_b, int batch_size =4, bool use_qlora =True) |
| Optional[str] | **[check_memory](/python-reference/Namespaces/dd/d0a/namespacetraining_1_1memory/#function-check_memory)**(float num_params_b, int batch_size =4, bool use_qlora =True) |

## Detailed Description




```
Pre-flight memory estimator for Apple Silicon training.```


## Functions Documentation

### function get_available_ram_gb

```python
float get_available_ram_gb()
```


### function estimate_model_memory_gb

```python
float estimate_model_memory_gb(
    float num_params_b,
    int batch_size =4,
    bool use_qlora =True
)
```


### function check_memory

```python
Optional[str] check_memory(
    float num_params_b,
    int batch_size =4,
    bool use_qlora =True
)
```






-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700