---
title: GNUS-NEO-SWARM/test/core/test_fp4_codec.cpp
summary: Unit tests for FP4Codec. 

---

# GNUS-NEO-SWARM/test/core/test_fp4_codec.cpp



Unit tests for [FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/).  [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[TEST](/source-reference/Files/da/dbf/test__fp4__codec_8cpp/#function-test)**([FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/) , RoundtripSmallMatrix ) |
| | **[TEST](/source-reference/Files/da/dbf/test__fp4__codec_8cpp/#function-test)**([FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/) , RoundtripLargeMatrix ) |
| | **[TEST](/source-reference/Files/da/dbf/test__fp4__codec_8cpp/#function-test)**([FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/) , TensorDimensions ) |
| | **[TEST](/source-reference/Files/da/dbf/test__fp4__codec_8cpp/#function-test)**([FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/) , ZeroWeights ) |
| | **[TEST](/source-reference/Files/da/dbf/test__fp4__codec_8cpp/#function-test)**([FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/) , InvalidInput ) |
| | **[TEST](/source-reference/Files/da/dbf/test__fp4__codec_8cpp/#function-test)**([FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/) , MacroblockCount ) |

## Detailed Description

Unit tests for [FP4Codec](/source-reference/Classes/d5/dc3/classsgns_1_1neoswarm_1_1fp4_1_1_f_p4_codec/). 

**Date**: 2026-05-08 

## Functions Documentation

### function TEST

```cpp
TEST(
    FP4Codec ,
    RoundtripSmallMatrix 
)
```


### function TEST

```cpp
TEST(
    FP4Codec ,
    RoundtripLargeMatrix 
)
```


### function TEST

```cpp
TEST(
    FP4Codec ,
    TensorDimensions 
)
```


### function TEST

```cpp
TEST(
    FP4Codec ,
    ZeroWeights 
)
```


### function TEST

```cpp
TEST(
    FP4Codec ,
    InvalidInput 
)
```


### function TEST

```cpp
TEST(
    FP4Codec ,
    MacroblockCount 
)
```




## Source code

```cpp


#include "core/fp4/fp4_codec.hpp"
#include <gtest/gtest.h>

#include <cmath>
#include <numeric>
#include <random>
#include <vector>

using namespace sgns::neoswarm;
using namespace sgns::neoswarm::fp4;

TEST( FP4Codec, RoundtripSmallMatrix )
{
    FP4Codec codec;
    const size_t rows = 4;
    const size_t cols = 4;
    std::vector<float> weights = { 0.1f, -0.2f, 0.5f, -0.8f, 1.0f,  -1.0f, 0.3f,  -0.3f,
                                   0.7f, -0.7f, 0.0f, 0.9f,  -0.9f, 0.4f,  -0.4f, 0.6f };

    auto enc_res = codec.Encode( weights.data(), rows, cols );
    ASSERT_TRUE( enc_res.has_value() );

    std::vector<float> decoded( rows * cols );
    auto dec_res = codec.Decode( enc_res.value(), decoded.data() );
    ASSERT_TRUE( dec_res.has_value() );

    float mse = 0.0f;
    for ( size_t i = 0; i < weights.size(); ++i )
    {
        float diff = weights[i] - decoded[i];
        mse += diff * diff;
    }
    mse /= static_cast<float>( weights.size() );
    EXPECT_LT( mse, 0.05f ) << "MSE too high: " << mse;
}

TEST( FP4Codec, RoundtripLargeMatrix )
{
    FP4Codec codec;
    const size_t rows = 128;
    const size_t cols = 128;
    std::vector<float> weights( rows * cols );

    std::mt19937 rng( 42 );
    std::normal_distribution<float> dist( 0.0f, 0.5f );
    for ( auto& w : weights )
        w = dist( rng );

    auto enc_res = codec.Encode( weights.data(), rows, cols );
    ASSERT_TRUE( enc_res.has_value() );

    std::vector<float> decoded( rows * cols );
    auto dec_res = codec.Decode( enc_res.value(), decoded.data() );
    ASSERT_TRUE( dec_res.has_value() );

    float mse = codec.ComputeError( weights.data(), enc_res.value() );
    EXPECT_LT( mse, 0.02f ) << "MSE too high for large matrix: " << mse;
}

TEST( FP4Codec, TensorDimensions )
{
    FP4Codec codec;
    const size_t rows = 65;
    const size_t cols = 65;
    std::vector<float> weights( rows * cols, 0.5f );

    auto enc_res = codec.Encode( weights.data(), rows, cols );
    ASSERT_TRUE( enc_res.has_value() );
    EXPECT_EQ( enc_res.value().rows_, rows );
    EXPECT_EQ( enc_res.value().cols_, cols );

    std::vector<float> decoded( rows * cols );
    auto dec_res = codec.Decode( enc_res.value(), decoded.data() );
    ASSERT_TRUE( dec_res.has_value() );
}

TEST( FP4Codec, ZeroWeights )
{
    FP4Codec codec;
    const size_t rows = 8;
    const size_t cols = 8;
    std::vector<float> weights( rows * cols, 0.0f );

    auto enc_res = codec.Encode( weights.data(), rows, cols );
    ASSERT_TRUE( enc_res.has_value() );

    std::vector<float> decoded( rows * cols );
    auto dec_res = codec.Decode( enc_res.value(), decoded.data() );
    ASSERT_TRUE( dec_res.has_value() );

    for ( float v : decoded )
        EXPECT_NEAR( v, 0.0f, 1e-5f );
}

TEST( FP4Codec, InvalidInput )
{
    FP4Codec codec;
    auto res = codec.Encode( nullptr, 4, 4 );
    EXPECT_FALSE( res.has_value() );
}

TEST( FP4Codec, MacroblockCount )
{
    FP4Codec codec;
    const size_t rows = 128;
    const size_t cols = 128;
    std::vector<float> weights( rows * cols, 1.0f );
    auto enc_res = codec.Encode( weights.data(), rows, cols );
    ASSERT_TRUE( enc_res.has_value() );
    EXPECT_EQ( enc_res.value().NumMacroblocks(), 4u );
}
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
