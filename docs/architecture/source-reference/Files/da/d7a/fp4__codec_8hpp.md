---
title: GNUS-NEO-SWARM/src/core/fp4/fp4_codec.hpp
summary: FP4 v3 4-bit floating-point quantization codec (PTDS §4.1). 

---

# GNUS-NEO-SWARM/src/core/fp4/fp4_codec.hpp



FP4 v3 4-bit floating-point quantization codec (PTDS §4.1).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::fp4](/source-reference/Namespaces/db/daf/namespacesgns_1_1neoswarm_1_1fp4/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[sgns::neoswarm::fp4::FP4Tensor](/source-reference/Classes/d4/d68/structsgns_1_1neoswarm_1_1fp4_1_1_f_p4_tensor/)** <br/>Packed FP4 tensor: each byte holds two nibbles (high = even index).  |
| class | **[sgns::neoswarm::fp4::FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/)** <br/>Encodes and decodes FP32 weight matrices to/from FP4.  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| size_t | **[kMacroblockRows](/source-reference/Files/da/d7a/fp4__codec_8hpp/#variable-kmacroblockrows)**  |
| size_t | **[kMacroblockCols](/source-reference/Files/da/d7a/fp4__codec_8hpp/#variable-kmacroblockcols)**  |
| size_t | **[kMacroblockSize](/source-reference/Files/da/d7a/fp4__codec_8hpp/#variable-kmacroblocksize)**  |
| int | **[kScaleSearchSteps](/source-reference/Files/da/d7a/fp4__codec_8hpp/#variable-kscalesearchsteps)**  |
| float[16] | **[kFP4LUT](/source-reference/Files/da/d7a/fp4__codec_8hpp/#variable-kfp4lut)** <br/>NF4-style symmetric lookup table: 16 representable values in [-1, 1].  |

## Detailed Description

FP4 v3 4-bit floating-point quantization codec (PTDS §4.1). 

**Date**: 2026-05-06 


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


## Source code

```cpp


#ifndef NEOSWARM_CORE_FP4_FP4CODEC_HPP
#define NEOSWARM_CORE_FP4_FP4CODEC_HPP

#include "common/error.hpp"
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace sgns::neoswarm::fp4
{
    static constexpr size_t kMacroblockRows = 64;
    static constexpr size_t kMacroblockCols = 64;
    static constexpr size_t kMacroblockSize = kMacroblockRows * kMacroblockCols;
    static constexpr int kScaleSearchSteps = 32;

    static constexpr float kFP4LUT[16] = { -1.0f,   -0.6962f, -0.5251f, -0.3949f, -0.2844f, -0.1848f, -0.0911f, 0.0f,
                                           0.0796f, 0.1609f,  0.2461f,  0.3379f,  0.4407f,  0.5626f,  0.7230f,  1.0f };

    struct FP4Tensor
    {
        std::vector<uint8_t> data_; 
        std::vector<float> scales_; 
        size_t rows_ = 0;
        size_t cols_ = 0;

        size_t NumMacroblocks() const
        {
            size_t mb_rows = ( rows_ + kMacroblockRows - 1 ) / kMacroblockRows;
            size_t mb_cols = ( cols_ + kMacroblockCols - 1 ) / kMacroblockCols;
            return mb_rows * mb_cols;
        }
    };

    class FP4Codec
    {
        public:
        FP4Codec() = default;

        outcome::result<FP4Tensor> Encode( const float* weights,
                                           size_t rows,
                                           size_t cols,
                                           const float* activation_stats = nullptr ) const;

        outcome::result<void> Decode( const FP4Tensor& tensor, float* output ) const;

        float ComputeError( const float* original, const FP4Tensor& encoded ) const;

        private:
        float FindBestScale( const float* block, size_t n, const float* act_stats = nullptr ) const;
        static uint8_t QuantizeValue( float v, float scale );
        static float DequantizeValue( uint8_t idx, float scale );
    };

} // namespace sgns::neoswarm::fp4

#endif // NEOSWARM_CORE_FP4_FP4CODEC_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
