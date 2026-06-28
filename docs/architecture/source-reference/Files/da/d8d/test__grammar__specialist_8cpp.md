---
title: GNUS-NEO-SWARM/test/specialists/test_grammar_specialist.cpp
summary: Unit tests for GrammarSpecialist — happy, unhappy paths. 

---

# GNUS-NEO-SWARM/test/specialists/test_grammar_specialist.cpp



Unit tests for GrammarSpecialist — happy, unhappy paths.  [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[TEST](/source-reference/Files/da/d8d/test__grammar__specialist_8cpp/#function-test)**(GrammarSpecialist , Process_LoadedEngine_ReturnsRefinedOutput ) |
| | **[TEST](/source-reference/Files/da/d8d/test__grammar__specialist_8cpp/#function-test)**(GrammarSpecialist , GetName_ReturnsCorrectName ) |
| | **[TEST](/source-reference/Files/da/d8d/test__grammar__specialist_8cpp/#function-test)**(GrammarSpecialist , IsLoaded_InitiallyFalse ) |
| | **[TEST](/source-reference/Files/da/d8d/test__grammar__specialist_8cpp/#function-test)**(GrammarSpecialist , GetConfidence_InitiallyZero ) |
| | **[TEST](/source-reference/Files/da/d8d/test__grammar__specialist_8cpp/#function-test)**(GrammarSpecialist , Process_NotLoaded_ReturnsInputUnchanged ) |
| | **[TEST](/source-reference/Files/da/d8d/test__grammar__specialist_8cpp/#function-test)**(GrammarSpecialist , Load_NoEngine_ReturnsError ) |
| | **[TEST](/source-reference/Files/da/d8d/test__grammar__specialist_8cpp/#function-test)**(GrammarSpecialist , Process_NoEngine_ReturnsInputUnchanged ) |

## Detailed Description

Unit tests for GrammarSpecialist — happy, unhappy paths. 

**Date**: 2026-06-16 

## Functions Documentation

### function TEST

```cpp
TEST(
    GrammarSpecialist ,
    Process_LoadedEngine_ReturnsRefinedOutput 
)
```


### function TEST

```cpp
TEST(
    GrammarSpecialist ,
    GetName_ReturnsCorrectName 
)
```


### function TEST

```cpp
TEST(
    GrammarSpecialist ,
    IsLoaded_InitiallyFalse 
)
```


### function TEST

```cpp
TEST(
    GrammarSpecialist ,
    GetConfidence_InitiallyZero 
)
```


### function TEST

```cpp
TEST(
    GrammarSpecialist ,
    Process_NotLoaded_ReturnsInputUnchanged 
)
```


### function TEST

```cpp
TEST(
    GrammarSpecialist ,
    Load_NoEngine_ReturnsError 
)
```


### function TEST

```cpp
TEST(
    GrammarSpecialist ,
    Process_NoEngine_ReturnsInputUnchanged 
)
```




## Source code

```cpp


#include "specialists/grammar_specialist.hpp"
#include "core/engine/inference_engine.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <memory>

using namespace sgns::neoswarm;

namespace
{
    class MockEngine : public core::InferenceEngine
    {
    public:
        outcome::result<InferenceResponse> Infer( const Task& task ) override
        {
            InferenceResponse resp;
            resp.m_output = task.m_prompt + " [corrected]";
            resp.m_perplexity = 1.0f;
            resp.m_success = true;
            resp.m_taskId = task.m_id;
            return outcome::success( resp );
        }
        outcome::result<void> StreamInfer( const Task&,
                                            std::function<void( const std::string& )> ) override
        {
            return outcome::success();
        }
        outcome::result<void> LoadModel( const std::string& ) override
        {
            return outcome::success();
        }
        bool IsLoaded() const override
        {
            return true;
        }
        std::string BackendName() const override
        {
            return "mock";
        }
    };

    class FailingMockEngine : public core::InferenceEngine
    {
    public:
        outcome::result<InferenceResponse> Infer( const Task& ) override
        {
            return outcome::failure( Error::InferenceFailed );
        }
        outcome::result<void> StreamInfer( const Task&,
                                            std::function<void( const std::string& )> ) override
        {
            return outcome::failure( Error::InferenceFailed );
        }
        outcome::result<void> LoadModel( const std::string& ) override
        {
            return outcome::failure( Error::ModelLoadFailed );
        }
        bool IsLoaded() const override
        {
            return false;
        }
        std::string BackendName() const override
        {
            return "mock";
        }
    };
} // namespace

// =======================================================================
// Happy path
// =======================================================================

TEST( GrammarSpecialist, Process_LoadedEngine_ReturnsRefinedOutput )
{
    auto engine = std::make_shared<MockEngine>();
    specialists::GrammarSpecialist specialist( engine );
    ASSERT_TRUE( specialist.Load( "dummy" ).has_value() );
    ASSERT_TRUE( specialist.IsLoaded() );

    auto result = specialist.Process( "helo wrld" );
    ASSERT_TRUE( result.has_value() );
    EXPECT_NE( result.value().find( "helo" ), std::string::npos );
    EXPECT_GT( specialist.GetConfidence(), 0.0f );
}

TEST( GrammarSpecialist, GetName_ReturnsCorrectName )
{
    specialists::GrammarSpecialist specialist;
    EXPECT_EQ( specialist.GetName(), "GrammarSpecialist" );
}

TEST( GrammarSpecialist, IsLoaded_InitiallyFalse )
{
    specialists::GrammarSpecialist specialist;
    EXPECT_FALSE( specialist.IsLoaded() );
}

TEST( GrammarSpecialist, GetConfidence_InitiallyZero )
{
    specialists::GrammarSpecialist specialist;
    EXPECT_FLOAT_EQ( specialist.GetConfidence(), 0.0f );
}

// =======================================================================
// Unhappy path — fail-close
// =======================================================================

TEST( GrammarSpecialist, Process_NotLoaded_ReturnsInputUnchanged )
{
    auto engine = std::make_shared<MockEngine>();
    specialists::GrammarSpecialist specialist( engine );

    auto result = specialist.Process( "hello world" );
    ASSERT_TRUE( result.has_value() );
    EXPECT_EQ( result.value(), "hello world" );
    EXPECT_FLOAT_EQ( specialist.GetConfidence(), 0.0f );
}

TEST( GrammarSpecialist, Load_NoEngine_ReturnsError )
{
    specialists::GrammarSpecialist specialist;
    auto result = specialist.Load( "dummy" );
    EXPECT_FALSE( result.has_value() );
}

TEST( GrammarSpecialist, Process_NoEngine_ReturnsInputUnchanged )
{
    specialists::GrammarSpecialist specialist;
    auto result = specialist.Process( "hello world" );
    ASSERT_TRUE( result.has_value() );
    EXPECT_EQ( result.value(), "hello world" );
    EXPECT_FLOAT_EQ( specialist.GetConfidence(), 0.0f );
}
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
