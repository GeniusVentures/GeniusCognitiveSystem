---
title: distill::backends::base::TeacherBackend

---

# distill::backends::base::TeacherBackend



 [More...](#detailed-description)

Inherits from ABC

Inherited by [distill.backends.anthropic_backend.AnthropicBackend](/python-reference/Classes/d6/d29/classdistill_1_1backends_1_1anthropic__backend_1_1_anthropic_backend/), [distill.backends.openai_backend.OpenAIBackend](/python-reference/Classes/d8/dea/classdistill_1_1backends_1_1openai__backend_1_1_open_a_i_backend/)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#function-__init__)**(self self, dict endpoint_config, str model_id, str api_key) |
| dict | **[generate](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#function-generate)**(self self, list messages, int max_tokens, float temperature, ** kwargs) |
| str | **[backend_type](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#function-backend_type)**(self self) |
| float | **[estimate_cost](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#function-estimate_cost)**(int prompt_tokens, int completion_tokens) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_endpoint_config](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#variable-_endpoint_config)**  |
| | **[_model_id](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#variable-_model_id)**  |
| | **[_api_key](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/#variable-_api_key)**  |

## Detailed Description

```python
class distill::backends::base::TeacherBackend;
```




```
Abstract interface that every teacher API backend must implement.

Concrete backends wrap their respective SDKs (openai, anthropic),
converting native responses into a uniform response dict with keys:
    content:           str   — the completion text
    prompt_tokens:     int   — tokens consumed by the prompt
    completion_tokens: int   — tokens produced by the completion
    raw_response:      object — the original SDK response object
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




```
Initialise the backend with connection details.

Args:
    endpoint_config: The resolved ``endpoints[name]`` dict from
        pipeline.yaml (contains ``url``, ``apiType``, etc.).
    model_id: The literal model identifier from the ``models``
        config block (e.g. ``"deepseek-v4-pro[1m]"``).
    api_key: The API key to authenticate requests.
```


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


**Reimplemented by**: [distill::backends::anthropic_backend::AnthropicBackend::generate](/python-reference/Classes/d6/d29/classdistill_1_1backends_1_1anthropic__backend_1_1_anthropic_backend/#function-generate), [distill::backends::openai_backend::OpenAIBackend::generate](/python-reference/Classes/d8/dea/classdistill_1_1backends_1_1openai__backend_1_1_open_a_i_backend/#function-generate)




```
Send a completion request and return a uniform response dict.

Returns:
    dict with keys ``content``, ``prompt_tokens``,
    ``completion_tokens``, ``raw_response``.
```


### function backend_type

```python
str backend_type(
    self self
)
```


**Reimplemented by**: [distill::backends::anthropic_backend::AnthropicBackend::backend_type](/python-reference/Classes/d6/d29/classdistill_1_1backends_1_1anthropic__backend_1_1_anthropic_backend/#function-backend_type), [distill::backends::openai_backend::OpenAIBackend::backend_type](/python-reference/Classes/d8/dea/classdistill_1_1backends_1_1openai__backend_1_1_open_a_i_backend/#function-backend_type)




```
Return a short identifier for this backend (e.g. ``"openai"``).```


### function estimate_cost

```python
static float estimate_cost(
    int prompt_tokens,
    int completion_tokens
)
```




```
Estimate the USD cost of a completion.

Default formula: (prompt_tokens * 0.27 + completion_tokens * 1.10) / 1_000_000.
Subclasses that use different pricing SHOULD override this.
```


## Protected Attributes Documentation

### variable _endpoint_config

```python
_endpoint_config =  endpoint_config;
```


### variable _model_id

```python
_model_id =  model_id;
```


### variable _api_key

```python
_api_key =  api_key;
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700