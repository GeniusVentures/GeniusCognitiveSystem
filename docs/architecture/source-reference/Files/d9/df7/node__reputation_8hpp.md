---
title: GNUS-NEO-SWARM/src/reputation/node_reputation.hpp
summary: Reputation helpers for GNUS NEO SWARM nodes. 

---

# GNUS-NEO-SWARM/src/reputation/node_reputation.hpp



Reputation helpers for GNUS NEO SWARM nodes.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::reputation](/source-reference/Namespaces/d7/d2c/namespacesgns_1_1neoswarm_1_1reputation/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[sgns::neoswarm::reputation::NodeReputation](/source-reference/Classes/df/d86/structsgns_1_1neoswarm_1_1reputation_1_1_node_reputation/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| double | **[ClampScore](/source-reference/Files/d9/df7/node__reputation_8hpp/#function-clampscore)**(double score)<br/>Clamp a reputation score to [0.0, 1.0].  |
| bool | **[IsHighTrust](/source-reference/Files/d9/df7/node__reputation_8hpp/#function-ishightrust)**(const [NodeReputation](/source-reference/Classes/df/d86/structsgns_1_1neoswarm_1_1reputation_1_1_node_reputation/) & rep)<br/>Check whether a node has enough history to be considered high-trust.  |

## Detailed Description

Reputation helpers for GNUS NEO SWARM nodes. 

**Date**: 2026-05-06 

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



## Source code

```cpp


#ifndef NEOSWARM_REPUTATION_NODEREPUTATION_HPP
#define NEOSWARM_REPUTATION_NODEREPUTATION_HPP

#include "common/types.hpp"
#include <algorithm>

namespace sgns::neoswarm::reputation
{
    using neoswarm::NodeReputation;

    inline double ClampScore( double score )
    {
        return std::max( 0.0, std::min( 1.0, score ) );
    }

    inline bool IsHighTrust( const NodeReputation& rep )
    {
        return rep.m_taskCount >= NodeReputation::kMinTasksForHighTrust && rep.m_globalScore >= 0.7;
    }

} // namespace sgns::neoswarm::reputation

#endif // NEOSWARM_REPUTATION_NODEREPUTATION_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
