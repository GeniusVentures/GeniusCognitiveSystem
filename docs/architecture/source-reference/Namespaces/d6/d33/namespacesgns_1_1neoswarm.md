---
title: sgns::neoswarm

---

# sgns::neoswarm





## Namespaces

| Name           |
| -------------- |
| **[sgns::neoswarm::api](/source-reference/Namespaces/d7/d2f/namespacesgns_1_1neoswarm_1_1api/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |
| **[sgns::neoswarm::core](/source-reference/Namespaces/d2/db7/namespacesgns_1_1neoswarm_1_1core/)**  |
| **[sgns::neoswarm::fp4](/source-reference/Namespaces/db/daf/namespacesgns_1_1neoswarm_1_1fp4/)**  |
| **[sgns::neoswarm::knowledge](/source-reference/Namespaces/d8/da0/namespacesgns_1_1neoswarm_1_1knowledge/)**  |
| **[sgns::neoswarm::security](/source-reference/Namespaces/d7/d75/namespacesgns_1_1neoswarm_1_1security/)**  |
| **[sgns::neoswarm::reputation](/source-reference/Namespaces/d7/d2c/namespacesgns_1_1neoswarm_1_1reputation/)**  |
| **[sgns::neoswarm::router](/source-reference/Namespaces/df/d79/namespacesgns_1_1neoswarm_1_1router/)**  |
| **[sgns::neoswarm::specialists](/source-reference/Namespaces/de/d04/namespacesgns_1_1neoswarm_1_1specialists/)**  |

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
| enum class uint8_t | **[Error](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-error)** { ModelLoadFailed = 1, InferenceFailed = 2, TokenizerFailed = 3, FP4DecodeFailed = 4, RoutingFailed = 5, NetworkError = 6, PeerNotFound = 7, BroadcastTimeout = 8, StorageError = 9, ReputationNotFound = 10, KnowledgeUnavailable = 11, FactValidationFailed = 12, IdentityError = 13, SignatureInvalid = 14, InvalidArgument = 15, NotImplemented = 16, InternalError = 17} |
| enum class uint8_t | **[ExecutionMode](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-executionmode)** { SingleNode = 0, Specialist = 1, Swarm = 2} |
| enum class uint8_t | **[RouteTarget](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-routetarget)** { CoreOnly = 0, CorePlusMath = 1, CorePlusGrammar = 2, CorePlusCode = 3} |
| using std::shared_ptr< spdlog::logger > | **[Logger](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#using-logger)** <br/>[Logger](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#using-logger) sgns::base::Logger convention.  |

## Functions

|                | Name           |
| -------------- | -------------- |
| [Logger](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#using-logger) | **[CreateLogger](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#function-createlogger)**(const std::string & tag)<br/>Create a named logger for a NEO SWARM component.  |

## Types Documentation

### enum Error

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| ModelLoadFailed | 1|   |
| InferenceFailed | 2|   |
| TokenizerFailed | 3|   |
| FP4DecodeFailed | 4|   |
| RoutingFailed | 5|   |
| NetworkError | 6|   |
| PeerNotFound | 7|   |
| BroadcastTimeout | 8|   |
| StorageError | 9|   |
| ReputationNotFound | 10|   |
| KnowledgeUnavailable | 11|   |
| FactValidationFailed | 12|   |
| IdentityError | 13|   |
| SignatureInvalid | 14|   |
| InvalidArgument | 15|   |
| NotImplemented | 16|   |
| InternalError | 17|   |




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




### using Logger

```cpp
using sgns::neoswarm::Logger = std::shared_ptr<spdlog::logger>;
```

[Logger](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#using-logger) sgns::base::Logger convention. 


## Functions Documentation

### function CreateLogger

```cpp
inline Logger CreateLogger(
    const std::string & tag
)
```

Create a named logger for a NEO SWARM component. 

**Parameters**: 

  * **tag** Component name shown in log output (e.g. "Router", "P2PNode"). 


**Return**: [Logger](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#using-logger) instance. 





-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700