---
title: sgns::neoswarm::reputation::WeightedConsensus::Config

---

# sgns::neoswarm::reputation::WeightedConsensus::Config






`#include <weighted_consensus.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| [Strategy](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/#enum-strategy) | **[strategy_](/source-reference/Classes/dd/dc7/structsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus_1_1_config/#variable-strategy_)**  |
| double | **[epsilon_](/source-reference/Classes/dd/dc7/structsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus_1_1_config/#variable-epsilon_)**  |
| double | **[min_weight_](/source-reference/Classes/dd/dc7/structsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus_1_1_config/#variable-min_weight_)** <br/>ignore nodes below this weight  |

## Public Attributes Documentation

### variable strategy_

```cpp
Strategy strategy_ = Strategy::WeightedVoting;
```


### variable epsilon_

```cpp
double epsilon_ = 1e-6;
```


### variable min_weight_

```cpp
double min_weight_ = 0.0;
```

ignore nodes below this weight 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700