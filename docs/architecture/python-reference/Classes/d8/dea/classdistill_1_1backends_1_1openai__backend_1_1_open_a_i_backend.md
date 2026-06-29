---
title: distill::backends::openai_backend::OpenAIBackend

---

# distill::backends::openai_backend::OpenAIBackend



 [More...](#detailed-description)

Inherits from [distill.backends.base.TeacherBackend](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/), ABC

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d8/dea/classdistill_1_1backends_1_1openai__backend_1_1_open_a_i_backend/#function-__init__)**(self self, dict endpoint_config, str model_id, str api_key) |
| str | **[backend_type](/python-reference/Classes/d8/dea/classdistill_1_1backends_1_1openai__backend_1_1_open_a_i_backend/#function-backend_type)**(self self) |
| dict | **[generate](/python-reference/Classes/d8/dea/classdistill_1_1backends_1_1openai__backend_1_1_open_a_i_backend/#function-generate)**(self self, list messages, int max_tokens, float temperature, ** kwargs) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_client](/python-reference/Classes/d8/dea/classdistill_1_1backends_1_1openai__backend_1_1_open_a_i_backend/#variable-_client)**  |

## Additional inherited members

**Public Functions inherited from [distill.backends.base.TeacherBackend](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/)**

|                | Name           |
| -------------- | -------------- |
| float | **[estimate_cost](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#function-estimate_cost)**(int prompt_tokens, int completion_tokens) |

**Protected Attributes inherited from [distill.backends.base.TeacherBackend](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/)**

|                | Name           |
| -------------- | -------------- |
| | **[_endpoint_config](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#variable-_endpoint_config)**  |
| | **[_model_id](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#variable-_model_id)**  |
| | **[_api_key](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#variable-_api_key)**  |


## Detailed Description

```python
class distill::backends::openai_backend::OpenAIBackend;
```




```
Teacher backend that talks to any OpenAI-compatible endpoint.

This wraps the official ``openai`` SDK and is used for endpoints whose
``apiType`` is ``"openai"`` — including the local LiteLLM proxy and
direct OpenAI/DeepSeek API calls.
```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    dict endpoint_config,
    str model_id,
    str api_key
)
```


### function backend_type

```python
str backend_type(
    self self
)
```


**Reimplements**: [distill::backends::base::TeacherBackend::backend_type](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#function-backend_type)




```
Return a short identifier for this backend (e.g. ``"openai"``).```


### function generate

```python
dict generate(
    self self,
    list messages,
    int max_tokens,
    float temperature,
    ** kwargs
)
```


**Reimplements**: [distill::backends::base::TeacherBackend::generate](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#function-generate)




```
Call the OpenAI Chat Completions endpoint and normalise the response.

Returns:
    Uniform dict with ``content``, ``prompt_tokens``,
    ``completion_tokens``, and ``raw_response``.
```


## Protected Attributes Documentation

### variable _client

```python
_client =  OpenAI(
            api_key=api_key,
            base_url=endpoint_config["url"],
        );
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700