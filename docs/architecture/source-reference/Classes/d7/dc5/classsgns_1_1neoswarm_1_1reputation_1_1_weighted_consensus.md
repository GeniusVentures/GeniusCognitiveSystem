---
title: sgns::neoswarm::reputation::WeightedConsensus
summary: Selects the winning output from a set of node responses. 

---

# sgns::neoswarm::reputation::WeightedConsensus



Selects the winning output from a set of node responses.  [More...](#detailed-description)


`#include <weighted_consensus.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/dd/dc7/structsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus_1_1_config/)**  |

## Public Types

|                | Name           |
| -------------- | -------------- |
| enum class uint8_t | **[Strategy](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/#enum-strategy)** { WeightedVoting = 0, BestWeightedScore = 1} |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[WeightedConsensus](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/#function-weightedconsensus)**() |
| | **[WeightedConsensus](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/#function-weightedconsensus)**([Config](/source-reference/Classes/dd/dc7/structsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus_1_1_config/) cfg) |
| [NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/) | **[SelectWinner](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/#function-selectwinner)**(const std::vector< [NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/) > & outputs) const<br/>Select the winning output from a set of node outputs.  |

## Detailed Description

```cpp
class sgns::neoswarm::reputation::WeightedConsensus;
```

Selects the winning output from a set of node responses. 

weight_i = reputation_i / (perplexity_i + ε)

[Strategy](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/#enum-strategy) A (WeightedVoting): select O_k maximising Σ weight_i × (O_i == O_k) [Strategy](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/#enum-strategy) B (BestWeightedScore): select O_i maximising weight_i 

## Public Types Documentation

### enum Strategy

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| WeightedVoting | 0|   |
| BestWeightedScore | 1|   |




## Public Functions Documentation

### function WeightedConsensus

```cpp
WeightedConsensus()
```


### function WeightedConsensus

```cpp
explicit WeightedConsensus(
    Config cfg
)
```


### function SelectWinner

```cpp
NodeOutput SelectWinner(
    const std::vector< NodeOutput > & outputs
) const
```

Select the winning output from a set of node outputs. 

**Parameters**: 

  * **outputs** Responses from all participating nodes. 


**Return**: The winning [NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/) (or the first if empty). 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700