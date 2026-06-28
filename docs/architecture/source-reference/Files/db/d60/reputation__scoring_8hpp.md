---
title: GNUS-NEO-SWARM/src/reputation/reputation_scoring.hpp
summary: Reputation update formulas (PTDS §7.2). 

---

# GNUS-NEO-SWARM/src/reputation/reputation_scoring.hpp



Reputation update formulas (PTDS §7.2).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::reputation](/source-reference/Namespaces/d7/d2c/namespacesgns_1_1neoswarm_1_1reputation/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::reputation::ReputationScoring](/source-reference/Classes/d1/dd6/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring/)** <br/>Implements the PTDS §7.2 reputation update formulas.  |
| struct | **[sgns::neoswarm::reputation::ReputationScoring::Config](/source-reference/Classes/db/dae/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_scoring_1_1_config/)**  |

## Detailed Description

Reputation update formulas (PTDS §7.2). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_REPUTATION_REPUTATIONSCORING_HPP
#define NEOSWARM_REPUTATION_REPUTATIONSCORING_HPP

#include "node_reputation.hpp"
#include "common/types.hpp"
#include <optional>

namespace sgns::neoswarm::reputation
{
    class ReputationScoring
    {
        public:
        struct Config
        {
            double alpha_ = 0.10;   
            double beta_ = 0.05;    
            double gamma_ = 0.02;   
            double delta_ = 0.03;   
            double epsilon_ = 1e-6; 
            double baseline_accuracy_ = 0.5;
        };

        ReputationScoring();
        explicit ReputationScoring( Config cfg );

        NodeReputation Update( const NodeReputation& old,
                               const InferenceResponse& response,
                               double median_latency_ms,
                               std::optional<std::string> ground_truth,
                               const std::string& m_consensusoutput ) const;

        double DeltaAccuracy( bool has_ground_truth, double accuracy ) const;

        double DeltaLatency( double latency_ms, double median_latency_ms ) const;

        double DeltaConsistency( float perplexity ) const;

        private:
        Config m_cfg;
    };

} // namespace sgns::neoswarm::reputation

#endif // NEOSWARM_REPUTATION_REPUTATIONSCORING_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
