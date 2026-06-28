---
title: sgns::neoswarm::fp4::FP4Codec
summary: Encodes and decodes FP32 weight matrices to/from FP4. 

---

# sgns::neoswarm::fp4::FP4Codec



Encodes and decodes FP32 weight matrices to/from FP4. 


`#include <fp4_codec.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/#function-fp4codec)**() =default |
| outcome::result< [FP4Tensor](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/) > | **[Encode](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/#function-encode)**(const float * weights, size_t rows, size_t cols, const float * activation_stats =nullptr) const<br/>Quantize a row-major FP32 weight matrix to FP4.  |
| outcome::result< void > | **[Decode](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/#function-decode)**(const [FP4Tensor](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/) & tensor, float * output) const<br/>Dequantize an [FP4Tensor](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/) to a FP32 output buffer.  |
| float | **[ComputeError](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/#function-computeerror)**(const float * original, const [FP4Tensor](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/) & encoded) const<br/>Compute mean squared error between original and round-tripped weights.  |

## Public Functions Documentation

### function FP4Codec

```cpp
FP4Codec() =default
```


### function Encode

```cpp
outcome::result< FP4Tensor > Encode(
    const float * weights,
    size_t rows,
    size_t cols,
    const float * activation_stats =nullptr
) const
```

Quantize a row-major FP32 weight matrix to FP4. 

**Parameters**: 

  * **weights** Pointer to rows×cols FP32 values. 
  * **rows** Number of rows. 
  * **cols** Number of columns. 
  * **activation_stats** Optional per-column activation magnitudes (may be nullptr). 


**Return**: Encoded [FP4Tensor](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/) or FP4DecodeFailed. 

### function Decode

```cpp
outcome::result< void > Decode(
    const FP4Tensor & tensor,
    float * output
) const
```

Dequantize an [FP4Tensor](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/) to a FP32 output buffer. 

**Parameters**: 

  * **tensor** Encoded tensor. 
  * **output** Pre-allocated buffer of tensor.rows_ × tensor.cols_ floats. 


**Return**: outcome::success or FP4DecodeFailed. 

### function ComputeError

```cpp
float ComputeError(
    const float * original,
    const FP4Tensor & encoded
) const
```

Compute mean squared error between original and round-tripped weights. 

**Parameters**: 

  * **original** Original FP32 weights. 
  * **encoded** Encoded [FP4Tensor](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/). 


**Return**: MSE value. 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700