---
title: distill::backends::anthropic_backend::AnthropicBackend

---

# distill::backends::anthropic_backend::AnthropicBackend



 [More...](#detailed-description)

Inherits from [distill.backends.base.TeacherBackend](/python-reference/Classes/de/d97/classdistill_1_1backends_1_1base_1_1_teacher_backend/), ABC

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d6/d29/classdistill_1_1backends_1_1anthropic__backend_1_1_anthropic_backend/#function-__init__)**(self self, dict endpoint_config, str model_id, str api_key) |
| str | **[backend_type](/python-reference/Classes/d6/d29/classdistill_1_1backends_1_1anthropic__backend_1_1_anthropic_backend/#function-backend_type)**(self self) |
| dict | **[generate](/python-reference/Classes/d6/d29/classdistill_1_1backends_1_1anthropic__backend_1_1_anthropic_backend/#function-generate)**(self self, list messages, int max_tokens, float temperature, ** kwargs) |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| dict | **[_convert_messages](/python-reference/Classes/d6/d29/classdistill_1_1backends_1_1anthropic__backend_1_1_anthropic_backend/#function-_convert_messages)**(list messages) |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_client](/python-reference/Classes/d6/d29/classdistill_1_1backends_1_1anthropic__backend_1_1_anthropic_backend/#variable-_client)**  |

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
class distill::backends::anthropic_backend::AnthropicBackend;
```




```
Teacher backend that talks to the Anthropic Messages API.

Used for endpoints whose ``apiType`` is ``"anthropic"`` — either a
direct Anthropic API connection or an Anthropic-compatible proxy.
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
Send a completion via the Anthropic Messages API.

Converts OpenAI-format messages to Anthropic format, extracts
the first system message as the top-level ``system`` parameter,
and normalises the response into the uniform dict.

Extended thinking (``thinking={"type": "enabled", ...}``) is
passed through in ``**kwargs`` if supplied.

Returns:
    Uniform dict with ``content``, ``prompt_tokens``,
    ``completion_tokens``, ``raw_response``.
```


## Protected Functions Documentation

### function _convert_messages

```python
static dict _convert_messages(
    list messages
)
```




```
Convert an OpenAI-format message list to Anthropic API parameters.

Args:
    messages: List of dicts with ``role`` and ``content`` keys.
        Roles may be ``"system"``, ``"user"``, or ``"assistant"``.

Returns:
    dict with keys ``system`` (str or None) and ``messages`` (list
    of ``{"role": ..., "content": ...}`` dicts containing only
    ``"user"`` and ``"assistant"`` roles).
```


## Protected Attributes Documentation

### variable _client

```python
_client =  Anthropic(**kwargs);
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700