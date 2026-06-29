---
title: GNUS-NEO-SWARM/test/integration/test_pipeline.cpp
summary: Integration tests — full pipeline in stub mode. 

---

# GNUS-NEO-SWARM/test/integration/test_pipeline.cpp



Integration tests — full pipeline in stub mode.  [More...](#detailed-description)

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[PipelineTest](/source-reference/Classes/db/d7a/class_pipeline_test/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[TEST_F](/source-reference/Files/d3/db4/test__pipeline_8cpp/#function-test_f)**([PipelineTest](/source-reference/Classes/db/d7a/class_pipeline_test/) , SingleNodeMode ) |
| | **[TEST_F](/source-reference/Files/d3/db4/test__pipeline_8cpp/#function-test_f)**([PipelineTest](/source-reference/Classes/db/d7a/class_pipeline_test/) , MathRoutingAutoDetect ) |
| | **[TEST_F](/source-reference/Files/d3/db4/test__pipeline_8cpp/#function-test_f)**([PipelineTest](/source-reference/Classes/db/d7a/class_pipeline_test/) , GrammarRoutingAutoDetect ) |
| | **[TEST_F](/source-reference/Files/d3/db4/test__pipeline_8cpp/#function-test_f)**([PipelineTest](/source-reference/Classes/db/d7a/class_pipeline_test/) , ExplicitSpecialistMode ) |
| | **[TEST_F](/source-reference/Files/d3/db4/test__pipeline_8cpp/#function-test_f)**([PipelineTest](/source-reference/Classes/db/d7a/class_pipeline_test/) , SwarmFallsBackToSingleWithoutNetwork ) |
| | **[TEST_F](/source-reference/Files/d3/db4/test__pipeline_8cpp/#function-test_f)**([PipelineTest](/source-reference/Classes/db/d7a/class_pipeline_test/) , ResponseHasTaskId ) |
| | **[TEST_F](/source-reference/Files/d3/db4/test__pipeline_8cpp/#function-test_f)**([PipelineTest](/source-reference/Classes/db/d7a/class_pipeline_test/) , LatencyIsPositive ) |

## Detailed Description

Integration tests — full pipeline in stub mode. 

**Date**: 2026-05-08 

## Functions Documentation

### function TEST_F

```cpp
TEST_F(
    PipelineTest ,
    SingleNodeMode 
)
```


### function TEST_F

```cpp
TEST_F(
    PipelineTest ,
    MathRoutingAutoDetect 
)
```


### function TEST_F

```cpp
TEST_F(
    PipelineTest ,
    GrammarRoutingAutoDetect 
)
```


### function TEST_F

```cpp
TEST_F(
    PipelineTest ,
    ExplicitSpecialistMode 
)
```


### function TEST_F

```cpp
TEST_F(
    PipelineTest ,
    SwarmFallsBackToSingleWithoutNetwork 
)
```


### function TEST_F

```cpp
TEST_F(
    PipelineTest ,
    ResponseHasTaskId 
)
```


### function TEST_F

```cpp
TEST_F(
    PipelineTest ,
    LatencyIsPositive 
)
```




## Source code

```cpp


#include "api/api_server.hpp"
#include <gtest/gtest.h>

using namespace sgns::neoswarm;
using namespace sgns::neoswarm::api;

class PipelineTest : public ::testing::Test
{
    protected:
    void SetUp() override
    {
        ApiServer::Config cfg;
        cfg.m_modelPath = ""; // stub mode
        cfg.m_enableNetwork = false;
        cfg.m_enableKnowledge = true;
        cfg.m_reputationDbPath = ":memory:";
        cfg.m_nodeKeyFile = "/tmp/test_genius_node.key";

        server_ = std::make_unique<ApiServer>( cfg );
        ASSERT_TRUE( server_->Initialize().has_value() );
    }

    std::unique_ptr<ApiServer> server_;
};

TEST_F( PipelineTest, SingleNodeMode )
{
    Task task;
    task.m_prompt = "Tell me about the history of Rome.";
    task.m_mode = ExecutionMode::SingleNode;
    task.m_maxTokens = 32;
    task.m_temperature = 0.7f;

    auto res = server_->Process( task );
    ASSERT_TRUE( res.has_value() );
    EXPECT_EQ( res.value().m_modeUsed, ExecutionMode::SingleNode );
    EXPECT_FALSE( res.value().m_taskId.empty() );
}

TEST_F( PipelineTest, MathRoutingAutoDetect )
{
    Task task;
    task.m_prompt = "Calculate 847 × 963";
    task.m_mode = ExecutionMode::SingleNode;
    task.m_maxTokens = 32;

    auto res = server_->Process( task );
    ASSERT_TRUE( res.has_value() );
    EXPECT_NE( res.value().m_routeUsed, RouteTarget::CoreOnly );
}

TEST_F( PipelineTest, GrammarRoutingAutoDetect )
{
    Task task;
    task.m_prompt = "Please fix my grammar: I goes to store yesterday.";
    task.m_mode = ExecutionMode::SingleNode;
    task.m_maxTokens = 64;

    auto res = server_->Process( task );
    ASSERT_TRUE( res.has_value() );
    EXPECT_EQ( res.value().m_routeUsed, RouteTarget::CorePlusGrammar );
}

TEST_F( PipelineTest, ExplicitSpecialistMode )
{
    Task task;
    task.m_prompt = "What is the square root of 144?";
    task.m_mode = ExecutionMode::Specialist;
    task.m_maxTokens = 32;

    auto res = server_->Process( task );
    ASSERT_TRUE( res.has_value() );
    EXPECT_EQ( res.value().m_modeUsed, ExecutionMode::Specialist );
}

TEST_F( PipelineTest, SwarmFallsBackToSingleWithoutNetwork )
{
    Task task;
    task.m_prompt = "Complex question requiring swarm";
    task.m_mode = ExecutionMode::Swarm;
    task.m_maxTokens = 32;

    auto res = server_->Process( task );
    ASSERT_TRUE( res.has_value() );
    EXPECT_TRUE( res.value().m_success );
}

TEST_F( PipelineTest, ResponseHasTaskId )
{
    Task task;
    task.m_prompt = "Hello";
    task.m_maxTokens = 16;

    auto res = server_->Process( task );
    ASSERT_TRUE( res.has_value() );
    EXPECT_FALSE( res.value().m_taskId.empty() );
}

TEST_F( PipelineTest, LatencyIsPositive )
{
    Task task;
    task.m_prompt = "What is 2 + 2?";
    task.m_maxTokens = 16;

    auto res = server_->Process( task );
    ASSERT_TRUE( res.has_value() );
    EXPECT_GT( res.value().m_totalLatencyMs, 0.0 );
}
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
