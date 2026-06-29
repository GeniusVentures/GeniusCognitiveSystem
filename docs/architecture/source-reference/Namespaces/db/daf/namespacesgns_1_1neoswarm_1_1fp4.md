---
title: sgns::neoswarm::fp4

---

# sgns::neoswarm::fp4





## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[sgns::neoswarm::fp4::FP4Tensor](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/)** <br/>Packed FP4 tensor: each byte holds two nibbles (high = even index).  |
| class | **[sgns::neoswarm::fp4::FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/)** <br/>Encodes and decodes FP32 weight matrices to/from FP4.  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| size_t | **[kMacroblockRows](/source-reference/Namespaces/db/daf/namespacesgns_1_1neoswarm_1_1fp4/#variable-kmacroblockrows)**  |
| size_t | **[kMacroblockCols](/source-reference/Namespaces/db/daf/namespacesgns_1_1neoswarm_1_1fp4/#variable-kmacroblockcols)**  |
| size_t | **[kMacroblockSize](/source-reference/Namespaces/db/daf/namespacesgns_1_1neoswarm_1_1fp4/#variable-kmacroblocksize)**  |
| int | **[kScaleSearchSteps](/source-reference/Namespaces/db/daf/namespacesgns_1_1neoswarm_1_1fp4/#variable-kscalesearchsteps)**  |
| float[16] | **[kFP4LUT](/source-reference/Namespaces/db/daf/namespacesgns_1_1neoswarm_1_1fp4/#variable-kfp4lut)** <br/>NF4-style symmetric lookup table: 16 representable values in [-1, 1].  |



## Attributes Documentation

### variable kMacroblockRows

```cpp
static size_t kMacroblockRows = 64;
```


### variable kMacroblockCols

```cpp
static size_t kMacroblockCols = 64;
```


### variable kMacroblockSize

```cpp
static size_t kMacroblockSize = kMacroblockRows * kMacroblockCols;
```


### variable kScaleSearchSteps

```cpp
static int kScaleSearchSteps = 32;
```


### variable kFP4LUT

```cpp
static float[16] kFP4LUT = { -1.0f,   -0.6962f, -0.5251f, -0.3949f, -0.2844f, -0.1848f, -0.0911f, 0.0f,
        0.0796f, 0.1609f,  0.2461f,  0.3379f,  0.4407f,  0.5626f,  0.7230f,  1.0f };
```

NF4-style symmetric lookup table: 16 representable values in [-1, 1]. 




-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700