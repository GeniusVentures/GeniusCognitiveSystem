---
title: GNUS-NEO-SWARM/src/reputation/weighted_consensus.hpp
summary: Weighted consensus selection (PTDS §7.3). 

---

# GNUS-NEO-SWARM/src/reputation/weighted_consensus.hpp



Weighted consensus selection (PTDS §7.3).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::reputation](/source-reference/Namespaces/d7/d2c/namespacesgns_1_1neoswarm_1_1reputation/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::reputation::WeightedConsensus](/source-reference/Classes/d7/dc5/classsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus/)** <br/>Selects the winning output from a set of node responses.  |
| struct | **[sgns::neoswarm::reputation::WeightedConsensus::Config](/source-reference/Classes/dd/dc7/structsgns_1_1neoswarm_1_1reputation_1_1_weighted_consensus_1_1_config/)**  |

## Detailed Description

Weighted consensus selection (PTDS §7.3). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_REPUTATION_WEIGHTEDCONSENSUS_HPP
#define NEOSWARM_REPUTATION_WEIGHTEDCONSENSUS_HPP

#include "common/types.hpp"
#include <vector>

namespace sgns::neoswarm::reputation
{
    class WeightedConsensus
    {
        public:
        enum class Strategy : uint8_t
        {
            WeightedVoting = 0,
            BestWeightedScore = 1
        };

        struct Config
        {
            Strategy strategy_ = Strategy::WeightedVoting;
            double epsilon_ = 1e-6;
            double min_weight_ = 0.0; 
        };

        WeightedConsensus();
        explicit WeightedConsensus( Config cfg );

        NodeOutput SelectWinner( const std::vector<NodeOutput>& outputs ) const;

        private:
        Config m_cfg;

        std::vector<double> ComputeWeights( const std::vector<NodeOutput>& outputs ) const;

        NodeOutput WeightedVoting( const std::vector<NodeOutput>& outputs, const std::vector<double>& weights ) const;

        NodeOutput BestWeightedScore( const std::vector<NodeOutput>& outputs,
                                      const std::vector<double>& weights ) const;
    };

} // namespace sgns::neoswarm::reputation

#endif // NEOSWARM_REPUTATION_WEIGHTEDCONSENSUS_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
