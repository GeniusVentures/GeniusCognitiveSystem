---
title: distill::synthetic::SyntheticDataGenerator

---

# distill::synthetic::SyntheticDataGenerator





## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/#function-__init__)**(self self, [TeacherClient](/python-reference/Classes/d1/de5/classdistill_1_1teacher_1_1_teacher_client/) teacher_client, Optional project_root[Path] =None, bool use_cascade =[True](/python-reference/Namespaces/d2/d86/namespacedistill_1_1synthetic/#variable-true), str domain ="encyclopedic") |
| list | **[generate_for_niche](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/#function-generate_for_niche)**(self self, str niche_name, str system_prompt, list user_prompts, int num_samples =500, Optional keywords[list] =None) |
| | **[save_to_jsonl](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/#function-save_to_jsonl)**(self self, list samples, Path output_path) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| bool | **[_passes_quality](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/#function-_passes_quality)**(self self, str text, Optional keywords[list] =None) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_client](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/#variable-_client)**  |
| | **[_use_cascade](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/#variable-_use_cascade)**  |
| | **[_default_domain](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/#variable-_default_domain)**  |
| | **[_project_root](/python-reference/Classes/d1/dbe/classdistill_1_1synthetic_1_1_synthetic_data_generator/#variable-_project_root)**  |

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    TeacherClient teacher_client,
    Optional project_root[Path] =None,
    bool use_cascade =True,
    str domain ="encyclopedic"
)
```


### function generate_for_niche

```python
list generate_for_niche(
    self self,
    str niche_name,
    str system_prompt,
    list user_prompts,
    int num_samples =500,
    Optional keywords[list] =None
)
```


### function save_to_jsonl

```python
save_to_jsonl(
    self self,
    list samples,
    Path output_path
)
```


## Protected Functions Documentation

### function _passes_quality

```python
bool _passes_quality(
    self self,
    str text,
    Optional keywords[list] =None
)
```


## Protected Attributes Documentation

### variable _client

```python
_client =  teacher_client;
```


### variable _use_cascade

```python
_use_cascade =  use_cascade;
```


### variable _default_domain

```python
_default_domain =  domain;
```


### variable _project_root

```python
_project_root =  project_root;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700