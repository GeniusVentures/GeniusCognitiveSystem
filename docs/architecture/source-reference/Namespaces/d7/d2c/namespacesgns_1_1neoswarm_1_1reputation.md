---
title: sgns::neoswarm::reputation

---

# sgns::neoswarm::reputation





## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::reputation::ReputationCRDT](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/)** <br/>Last-Write-Wins Register per node (PTDS §4.2).  |
| class | **[sgns::neoswarm::reputation::ReputationScoring](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/)** <br/>Implements the PTDS §7.2 reputation update formulas.  |
| class | **[sgns::neoswarm::reputation::ReputationStorage](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/)** <br/>Persists [NodeReputation](/source-reference/Classes/df/d86/structsgns_1_1neoswarm_1_1reputation_1_1_node_reputation/) records to RocksDB. Falls back to an in-memory store when RocksDB is not compiled in.  |
| class | **[sgns::neoswarm::reputation::WeightedConsensus](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/)** <br/>Selects the winning output from a set of node responses.  |
| struct | **[sgns::neoswarm::reputation::NodeReputation](/source-reference/Classes/df/d86/structsgns_1_1neoswarm_1_1reputation_1_1_node_reputation/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| double | **[ClampScore](/source-reference/Namespaces/d7/d2c/namespacesgns_1_1neoswarm_1_1reputation/#function-clampscore)**(double score)<br/>Clamp a reputation score to [0.0, 1.0].  |
| bool | **[IsHighTrust](/source-reference/Namespaces/d7/d2c/namespacesgns_1_1neoswarm_1_1reputation/#function-ishightrust)**(const [NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/) & rep)<br/>Check whether a node has enough history to be considered high-trust.  |


## Functions Documentation

### function ClampScore

```cpp
inline double ClampScore(
    double score
)
```

Clamp a reputation score to [0.0, 1.0]. 

**Parameters**: 

  * **score** Raw score value. 


**Return**: Clamped score. 

### function IsHighTrust

```cpp
inline bool IsHighTrust(
    const NodeReputation & rep
)
```

Check whether a node has enough history to be considered high-trust. 

**Parameters**: 

  * **rep** Node reputation record. 


**Return**: True if the node meets the high-trust threshold. 





-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700