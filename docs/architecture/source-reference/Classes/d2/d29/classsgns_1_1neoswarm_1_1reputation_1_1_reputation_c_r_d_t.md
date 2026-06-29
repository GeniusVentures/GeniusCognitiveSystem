---
title: sgns::neoswarm::reputation::ReputationCRDT
summary: Last-Write-Wins Register per node (PTDS §4.2). 

---

# sgns::neoswarm::reputation::ReputationCRDT



Last-Write-Wins Register per node (PTDS §4.2).  [More...](#detailed-description)


`#include <reputation_crdt.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| void | **[Merge](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/#function-merge)**(const [NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/) & remote)<br/>Apply a remote reputation update (merge).  |
| std::optional< [NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/) > | **[Get](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/#function-get)**(const std::string & identity_key) const<br/>Get the current merged state for a node.  |
| std::vector< [NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/) > | **[GetAll](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/#function-getall)**() const<br/>Get all merged reputation records.  |
| std::string | **[Serialize](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/#function-serialize)**() const<br/>Serialise the full CRDT state for network transmission.  |
| void | **[DeserializeAndMerge](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/#function-deserializeandmerge)**(const std::string & data)<br/>Deserialise and merge a received CRDT state.  |

## Detailed Description

```cpp
class sgns::neoswarm::reputation::ReputationCRDT;
```

Last-Write-Wins Register per node (PTDS §4.2). 

Merge rule: keep the entry with the highest m_lastUpdatedMs timestamp. Designed to be replicated across nodes via libp2p GossipSub. 

## Public Functions Documentation

### function Merge

```cpp
void Merge(
    const NodeReputation & remote
)
```

Apply a remote reputation update (merge). 

**Parameters**: 

  * **remote** Reputation record received from a peer. 


### function Get

```cpp
std::optional< NodeReputation > Get(
    const std::string & identity_key
) const
```

Get the current merged state for a node. 

**Parameters**: 

  * **identity_key** Node identity key. 


**Return**: [NodeReputation](/source-reference/Classes/df/d86/structsgns_1_1neoswarm_1_1reputation_1_1_node_reputation/) if known, std::nullopt otherwise. 

### function GetAll

```cpp
std::vector< NodeReputation > GetAll() const
```

Get all merged reputation records. 

**Return**: Vector of all known records. 

### function Serialize

```cpp
std::string Serialize() const
```

Serialise the full CRDT state for network transmission. 

**Return**: CSV-encoded state string. 

### function DeserializeAndMerge

```cpp
void DeserializeAndMerge(
    const std::string & data
)
```

Deserialise and merge a received CRDT state. 

**Parameters**: 

  * **data** CSV-encoded state string from a peer. 


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700