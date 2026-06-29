---
title: sgns::neoswarm::network::ResultAggregation
summary: Collects NodeOutput responses from swarm peers with a timeout. 

---

# sgns::neoswarm::network::ResultAggregation



Collects [NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/) responses from swarm peers with a timeout.  [More...](#detailed-description)


`#include <result_aggregation.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/d4/daf/structsgns_1_1neoswarm_1_1network_1_1_result_aggregation_1_1_config/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[ResultAggregation](/source-reference/Classes/d8/d65/classsgns_1_1neoswarm_1_1network_1_1_result_aggregation/#function-resultaggregation)**() |
| | **[ResultAggregation](/source-reference/Classes/d8/d65/classsgns_1_1neoswarm_1_1network_1_1_result_aggregation/#function-resultaggregation)**([Config](/source-reference/Classes/d4/daf/structsgns_1_1neoswarm_1_1network_1_1_result_aggregation_1_1_config/) cfg) |
| void | **[Submit](/source-reference/Classes/d8/d65/classsgns_1_1neoswarm_1_1network_1_1_result_aggregation/#function-submit)**(const [NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/) & output)<br/>Submit a response from a node (thread-safe).  |
| outcome::result< std::vector< [NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/) > > | **[Collect](/source-reference/Classes/d8/d65/classsgns_1_1neoswarm_1_1network_1_1_result_aggregation/#function-collect)**()<br/>Wait for responses and return collected results.  |
| void | **[Reset](/source-reference/Classes/d8/d65/classsgns_1_1neoswarm_1_1network_1_1_result_aggregation/#function-reset)**()<br/>Reset for a new collection round.  |
| size_t | **[ResponseCount](/source-reference/Classes/d8/d65/classsgns_1_1neoswarm_1_1network_1_1_result_aggregation/#function-responsecount)**() const |

## Detailed Description

```cpp
class sgns::neoswarm::network::ResultAggregation;
```

Collects [NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/) responses from swarm peers with a timeout. 

Returns as soon as min_responses_ are received or the timeout expires. 

## Public Functions Documentation

### function ResultAggregation

```cpp
ResultAggregation()
```


### function ResultAggregation

```cpp
explicit ResultAggregation(
    Config cfg
)
```


### function Submit

```cpp
void Submit(
    const NodeOutput & output
)
```

Submit a response from a node (thread-safe). 

**Parameters**: 

  * **output** Node output to add to the collection. 


### function Collect

```cpp
outcome::result< std::vector< NodeOutput > > Collect()
```

Wait for responses and return collected results. 

**Return**: Vector of collected NodeOutputs or BroadcastTimeout. 

Blocks until min_responses_ received or timeout expires. 


### function Reset

```cpp
void Reset()
```

Reset for a new collection round. 

### function ResponseCount

```cpp
size_t ResponseCount() const
```


**Return**: Number of responses received so far. 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700