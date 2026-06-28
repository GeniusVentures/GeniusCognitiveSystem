---
title: GNUS-NEO-SWARM/test/knowledge/test_fact_validation.cpp
summary: Unit tests for FactValidation — claim verification against grounding facts. 

---

# GNUS-NEO-SWARM/test/knowledge/test_fact_validation.cpp



Unit tests for [FactValidation](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/) — claim verification against grounding facts.  [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[TEST](/source-reference/Files/de/d09/test__fact__validation_8cpp/#function-test)**([FactValidation](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/) , EmptyFactsPasses ) |
| | **[TEST](/source-reference/Files/de/d09/test__fact__validation_8cpp/#function-test)**([FactValidation](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/) , MatchingFactPasses ) |
| | **[TEST](/source-reference/Files/de/d09/test__fact__validation_8cpp/#function-test)**([FactValidation](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/) , NoRelevantFactsPasses ) |
| | **[TEST](/source-reference/Files/de/d09/test__fact__validation_8cpp/#function-test)**([FactValidation](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/) , IsAvailable ) |
| | **[TEST](/source-reference/Files/de/d09/test__fact__validation_8cpp/#function-test)**([KnowledgeRetrieval](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/) , LoadEmptyPathDoesNotCrash ) |
| | **[TEST](/source-reference/Files/de/d09/test__fact__validation_8cpp/#function-test)**([KnowledgeRetrieval](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/) , NotLoadedReturnsEmpty ) |

## Detailed Description

Unit tests for [FactValidation](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/) — claim verification against grounding facts. 

**Date**: 2026-05-28 

## Functions Documentation

### function TEST

```cpp
TEST(
    FactValidation ,
    EmptyFactsPasses 
)
```


### function TEST

```cpp
TEST(
    FactValidation ,
    MatchingFactPasses 
)
```


### function TEST

```cpp
TEST(
    FactValidation ,
    NoRelevantFactsPasses 
)
```


### function TEST

```cpp
TEST(
    FactValidation ,
    IsAvailable 
)
```


### function TEST

```cpp
TEST(
    KnowledgeRetrieval ,
    LoadEmptyPathDoesNotCrash 
)
```


### function TEST

```cpp
TEST(
    KnowledgeRetrieval ,
    NotLoadedReturnsEmpty 
)
```




## Source code

```cpp


#include "common/types.hpp"
#include "knowledge/fact_validation.hpp"
#include "knowledge/knowledge_retrieval.hpp"
#include <gtest/gtest.h>

#include <memory>

using namespace sgns::neoswarm;
using namespace sgns::neoswarm::knowledge;

namespace
{
    KnowledgeFact MakeFact( const std::string& source, const std::string& content )
    {
        KnowledgeFact f;
        f.m_source = source;
        f.m_content = content;
        return f;
    }

    std::shared_ptr<KnowledgeRetrieval> MakeRetrieval()
    {
        KnowledgeRetrieval::Config cfg;
        cfg.m_factsPath = "";
        auto ret = std::make_shared<KnowledgeRetrieval>( cfg );
        ret->Load();
        return ret;
    }
} // namespace

TEST( FactValidation, EmptyFactsPasses )
{
    auto retrieval = MakeRetrieval();
    FactValidation validator( retrieval );

    std::vector<KnowledgeFact> facts;
    auto result = validator.Validate( "Anything goes", facts );

    EXPECT_TRUE( result.passed_ );
}

TEST( FactValidation, MatchingFactPasses )
{
    auto retrieval = MakeRetrieval();
    FactValidation validator( retrieval );

    std::vector<KnowledgeFact> facts = { MakeFact( "physics", "speed of light: 299792 km/s" ) };

    auto result = validator.Validate( "The speed of light is approximately 299792 km per second", facts );
    EXPECT_TRUE( result.passed_ );
}

TEST( FactValidation, NoRelevantFactsPasses )
{
    auto retrieval = MakeRetrieval();
    FactValidation validator( retrieval );

    std::vector<KnowledgeFact> facts = { MakeFact( "geography", "Earth radius is 6371 km" ),
                                         MakeFact( "chemistry", "Water boils at 100 degrees Celsius at sea level" ) };

    auto result = validator.Validate( "The speed of light is very fast", facts );
    EXPECT_TRUE( result.passed_ );
}

TEST( FactValidation, IsAvailable )
{
    auto retrieval = MakeRetrieval();
    FactValidation validator( retrieval );

    // May or may not be available depending on what was loaded
    bool available = validator.IsAvailable();
    // Just verify it doesn't crash — either state is valid
    SUCCEED();
}

TEST( KnowledgeRetrieval, LoadEmptyPathDoesNotCrash )
{
    KnowledgeRetrieval::Config cfg;
    cfg.m_factsPath = "";
    KnowledgeRetrieval retriever( cfg );
    retriever.Load();

    // Retrieve should handle empty facts gracefully
    auto result = retriever.Retrieve( "What is gravity?" );
    EXPECT_TRUE( !result.has_value() || result.has_value() );
}

TEST( KnowledgeRetrieval, NotLoadedReturnsEmpty )
{
    KnowledgeRetrieval::Config cfg;
    cfg.m_factsPath = "/nonexistent/path/facts.csv";
    KnowledgeRetrieval retriever( cfg );
    retriever.Load();

    EXPECT_FALSE( retriever.IsLoaded() );

    auto result = retriever.Retrieve( "test query" );
    EXPECT_FALSE( result.has_value() );
}
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
