---
title: distill::distillation::Distiller

---

# distill::distillation::Distiller





## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d1/d3c/classdistill_1_1distillation_1_1_distiller/#function-__init__)**(self self, float temperature =2.0, float alpha =0.5) |
| float | **[compute_distillation_loss](/python-reference/Classes/d1/d3c/classdistill_1_1distillation_1_1_distiller/#function-compute_distillation_loss)**(self self, np.ndarray student_logits, list teacher_logprobs, list target_ids) |
| dict | **[sweep_temperature](/python-reference/Classes/d1/d3c/classdistill_1_1distillation_1_1_distiller/#function-sweep_temperature)**(self self, np.ndarray student_logits, list teacher_logprobs, list target_ids, Optional temperatures[list] =None) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| float | **[_cross_entropy_loss](/python-reference/Classes/d1/d3c/classdistill_1_1distillation_1_1_distiller/#function-_cross_entropy_loss)**(self self, np.ndarray logits, list target_ids) |
| float | **[_kl_divergence_loss](/python-reference/Classes/d1/d3c/classdistill_1_1distillation_1_1_distiller/#function-_kl_divergence_loss)**(self self, np.ndarray student_logits, list teacher_logprobs) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_temperature](/python-reference/Classes/d1/d3c/classdistill_1_1distillation_1_1_distiller/#variable-_temperature)**  |
| | **[_alpha](/python-reference/Classes/d1/d3c/classdistill_1_1distillation_1_1_distiller/#variable-_alpha)**  |

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    float temperature =2.0,
    float alpha =0.5
)
```


### function compute_distillation_loss

```python
float compute_distillation_loss(
    self self,
    np.ndarray student_logits,
    list teacher_logprobs,
    list target_ids
)
```


### function sweep_temperature

```python
dict sweep_temperature(
    self self,
    np.ndarray student_logits,
    list teacher_logprobs,
    list target_ids,
    Optional temperatures[list] =None
)
```


## Protected Functions Documentation

### function _cross_entropy_loss

```python
float _cross_entropy_loss(
    self self,
    np.ndarray logits,
    list target_ids
)
```


### function _kl_divergence_loss

```python
float _kl_divergence_loss(
    self self,
    np.ndarray student_logits,
    list teacher_logprobs
)
```


## Protected Attributes Documentation

### variable _temperature

```python
_temperature =  temperature;
```


### variable _alpha

```python
_alpha =  alpha;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700