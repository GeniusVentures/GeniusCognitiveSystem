---
title: distill::teacher::_ResponseWrapper

---

# distill::teacher::_ResponseWrapper



 [More...](#detailed-description)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[__init__](/python-reference/Classes/d4/d23/classdistill_1_1teacher_1_1___response_wrapper/#function-__init__)**(self self, dict uniform) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| | **[message](/python-reference/Classes/d4/d23/classdistill_1_1teacher_1_1___response_wrapper/#variable-message)**  |
| | **[content](/python-reference/Classes/d4/d23/classdistill_1_1teacher_1_1___response_wrapper/#variable-content)**  |
| | **[prompt_tokens](/python-reference/Classes/d4/d23/classdistill_1_1teacher_1_1___response_wrapper/#variable-prompt_tokens)**  |
| | **[completion_tokens](/python-reference/Classes/d4/d23/classdistill_1_1teacher_1_1___response_wrapper/#variable-completion_tokens)**  |
| list | **[choices](/python-reference/Classes/d4/d23/classdistill_1_1teacher_1_1___response_wrapper/#variable-choices)**  |
| | **[usage](/python-reference/Classes/d4/d23/classdistill_1_1teacher_1_1___response_wrapper/#variable-usage)**  |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[_raw_response](/python-reference/Classes/d4/d23/classdistill_1_1teacher_1_1___response_wrapper/#variable-_raw_response)**  |

## Detailed Description

```python
class distill::teacher::_ResponseWrapper;
```




```
Lightweight adapter that makes a uniform backend dict look like an
OpenAI ``chat.completions.create`` response for backward compatibility.```

## Public Functions Documentation

### function __init__

```python
__init__(
    self self,
    dict uniform
)
```


## Public Attributes Documentation

### variable message

```python
message =  _Choice._Message(msg_content);
```


### variable content

```python
content =  content;
```


### variable prompt_tokens

```python
prompt_tokens =  prompt_tokens;
```


### variable completion_tokens

```python
completion_tokens =  completion_tokens;
```


### variable choices

```python
list choices =  [_Choice(uniform["content"])];
```


### variable usage

```python
usage =  _Usage(uniform["prompt_tokens"], uniform["completion_tokens"]);
```


## Protected Attributes Documentation

### variable _raw_response

```python
_raw_response =  uniform["raw_response"];
```


-------------------------------

Updated on 2026-06-28 at 23:28:44 -0700