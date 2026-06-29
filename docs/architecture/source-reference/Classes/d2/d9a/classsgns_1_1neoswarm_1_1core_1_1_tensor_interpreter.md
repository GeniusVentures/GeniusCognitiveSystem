---
title: sgns::neoswarm::core::TensorInterpreter
summary: Converts raw MNN tensor output bytes to a human-readable string. 

---

# sgns::neoswarm::core::TensorInterpreter



Converts raw [MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) tensor output bytes to a human-readable string.  [More...](#detailed-description)


`#include <tensor_interpreter.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[TensorInterpreter](/source-reference/Classes/d2/d9a/classsgns_1_1neoswarm_1_1core_1_1_tensor_interpreter/#function-tensorinterpreter)**() =default |
| | **[~TensorInterpreter](/source-reference/Classes/d2/d9a/classsgns_1_1neoswarm_1_1core_1_1_tensor_interpreter/#function-~tensorinterpreter)**() =default |
| void | **[SetTokenizer](/source-reference/Classes/d2/d9a/classsgns_1_1neoswarm_1_1core_1_1_tensor_interpreter/#function-settokenizer)**(std::shared_ptr< [Tokenizer](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/) > tok)<br/>Attach a tokenizer for token-decoding mode (optional).  |
| outcome::result< std::string > | **[Interpret](/source-reference/Classes/d2/d9a/classsgns_1_1neoswarm_1_1core_1_1_tensor_interpreter/#function-interpret)**(const std::vector< uint8_t > & bytes, sgns::InputFormat format) const<br/>Convert raw tensor bytes to a human-readable string.  |

## Detailed Description

```cpp
class sgns::neoswarm::core::TensorInterpreter;
```

Converts raw [MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) tensor output bytes to a human-readable string. 

Supported formats: FLOAT32, FLOAT16, INT32, INT8. When a [Tokenizer](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/) is attached and the format is FLOAT32, the bytes are treated as a logit vector and the highest-probability token is decoded. 

## Public Functions Documentation

### function TensorInterpreter

```cpp
TensorInterpreter() =default
```


### function ~TensorInterpreter

```cpp
~TensorInterpreter() =default
```


### function SetTokenizer

```cpp
void SetTokenizer(
    std::shared_ptr< Tokenizer > tok
)
```

Attach a tokenizer for token-decoding mode (optional). 

**Parameters**: 

  * **tok** [Tokenizer](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/) instance. 


### function Interpret

```cpp
outcome::result< std::string > Interpret(
    const std::vector< uint8_t > & bytes,
    sgns::InputFormat format
) const
```

Convert raw tensor bytes to a human-readable string. 

**Parameters**: 

  * **bytes** Raw output bytes from SGProcessingManager. 
  * **format** Tensor element format. 


**Return**: Decoded string or InferenceFailed / InvalidArgument. 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700