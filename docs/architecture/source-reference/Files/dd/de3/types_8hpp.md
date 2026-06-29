---
title: GNUS-NEO-SWARM/src/common/types.hpp
summary: Shared data types for the GNUS NEO SWARM engine. 

---

# GNUS-NEO-SWARM/src/common/types.hpp



Shared data types for the GNUS NEO SWARM engine. 

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[sgns::neoswarm::Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/)**  |
| struct | **[sgns::neoswarm::InferenceResponse](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/)**  |
| struct | **[sgns::neoswarm::RouteDecision](/source-reference/Classes/db/d13/structsgns_1_1neoswarm_1_1_route_decision/)**  |
| struct | **[sgns::neoswarm::PromptFeatures](/source-reference/Classes/d4/dc5/structsgns_1_1neoswarm_1_1_prompt_features/)**  |
| struct | **[sgns::neoswarm::NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/)**  |
| struct | **[sgns::neoswarm::NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/)**  |
| struct | **[sgns::neoswarm::KnowledgeFact](/source-reference/Classes/d5/d9b/structsgns_1_1neoswarm_1_1_knowledge_fact/)**  |

## Types

|                | Name           |
| -------------- | -------------- |
| enum class uint8_t | **[ExecutionMode](/source-reference/Files/dd/de3/types_8hpp/#enum-executionmode)** { SingleNode = 0, Specialist = 1, Swarm = 2} |
| enum class uint8_t | **[RouteTarget](/source-reference/Files/dd/de3/types_8hpp/#enum-routetarget)** { CoreOnly = 0, CorePlusMath = 1, CorePlusGrammar = 2, CorePlusCode = 3} |

## Types Documentation

### enum ExecutionMode

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| SingleNode | 0| Mode 1 — Core LLM only, fast.   |
| Specialist | 1| Mode 2 — Core + Grammar/Math, sequential.   |
| Swarm | 2| Mode 3 — Multiple nodes, weighted consensus.   |




### enum RouteTarget

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| CoreOnly | 0|   |
| CorePlusMath | 1|   |
| CorePlusGrammar | 2|   |
| CorePlusCode | 3| Future.   |







## Source code

```cpp


#ifndef NEOSWARM_COMMON_TYPES_HPP
#define NEOSWARM_COMMON_TYPES_HPP

#include <chrono>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sgns::neoswarm
{
    // -----------------------------------------------------------------------
    // Execution modes (PTDS §9)
    // -----------------------------------------------------------------------
    enum class ExecutionMode : uint8_t
    {
        SingleNode = 0, 
        Specialist = 1, 
        Swarm = 2       
    };

    // -----------------------------------------------------------------------
    // Routing targets (PTDS §6)
    // -----------------------------------------------------------------------
    enum class RouteTarget : uint8_t
    {
        CoreOnly = 0,
        CorePlusMath = 1,
        CorePlusGrammar = 2,
        CorePlusCode = 3 
    };

    // -----------------------------------------------------------------------
    // A task coming in from a client
    // -----------------------------------------------------------------------
    struct Task
    {
        std::string m_id;
        std::string m_prompt;
        ExecutionMode m_mode = ExecutionMode::SingleNode;
        uint32_t m_maxTokens = 512;
        float m_temperature = 0.7f;
        std::string m_nodeId; 
    };

    // -----------------------------------------------------------------------
    // What each node returns after inference
    // -----------------------------------------------------------------------
    struct InferenceResponse
    {
        std::string m_output;
        std::string m_taskId;
        ExecutionMode m_modeUsed = ExecutionMode::SingleNode;
        RouteTarget m_routeUsed = RouteTarget::CoreOnly;
        double m_totalLatencyMs = 0.0;
        float m_perplexity = 1.0f;
        double m_latencyMs = 0.0;
        std::string m_nodeId;
        bool m_success = true;
        std::string m_errorMessage;
    };

    // -----------------------------------------------------------------------
    // Routing decision produced by the router
    // -----------------------------------------------------------------------
    struct RouteDecision
    {
        RouteTarget m_target = RouteTarget::CoreOnly;
        float confidence_ = 1.0f;
        std::string m_reasoning;
        ExecutionMode m_mode = ExecutionMode::SingleNode;
    };

    // -----------------------------------------------------------------------
    // Prompt analysis features (PTDS §6.1)
    // -----------------------------------------------------------------------
    struct PromptFeatures
    {
        float numeric_density_ = 0.0f; 
        bool has_code_syntax_ = false;
        float complexity_ = 0.0f; 
        size_t token_count_ = 0;
        bool has_math_keywords_ = false;
        bool has_grammar_request_ = false;
    };

    // -----------------------------------------------------------------------
    // Node output used in consensus
    // -----------------------------------------------------------------------
    struct NodeOutput
    {
        std::string m_nodeId;
        std::string m_output;
        float m_perplexity = 1.0f;
        double m_latencyMs = 0.0;
        double reputation_ = 0.5;
    };

    // -----------------------------------------------------------------------
    // Reputation data model (PTDS §7.1)
    // -----------------------------------------------------------------------
    struct NodeReputation
    {
        std::string m_identityKey;
        double m_globalScore = 0.5;
        double m_mathScore = 0.5;
        double m_grammarScore = 0.5;
        double m_latencyScore = 0.5;
        double m_consistencyScore = 0.5;
        uint64_t m_taskCount = 0;
        uint64_t m_lastUpdatedMs = 0; 

        static constexpr uint64_t kMinTasksForHighTrust = 10;
    };

    // -----------------------------------------------------------------------
    // Grokipedia fact entry
    // -----------------------------------------------------------------------
    struct KnowledgeFact
    {
        std::string m_source;
        std::string m_content;
        float m_relevanceScore = 0.0f;
    };

    // -----------------------------------------------------------------------
    // Final API response
    // -----------------------------------------------------------------------
    

} // namespace sgns::neoswarm

#endif // NEOSWARM_COMMON_TYPES_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
