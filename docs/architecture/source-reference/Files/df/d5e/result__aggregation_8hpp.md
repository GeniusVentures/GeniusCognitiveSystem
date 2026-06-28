---
title: GNUS-NEO-SWARM/src/network/result_aggregation.hpp
summary: Timeout-bounded collection of swarm node responses (PTDS §4.2). 

---

# GNUS-NEO-SWARM/src/network/result_aggregation.hpp



Timeout-bounded collection of swarm node responses (PTDS §4.2).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::network::ResultAggregation](/source-reference/Classes/d8/d65/classsgns_1_1neoswarm_1_1network_1_1_result_aggregation/)** <br/>Collects [NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/) responses from swarm peers with a timeout.  |
| struct | **[sgns::neoswarm::network::ResultAggregation::Config](/source-reference/Classes/d4/daf/structsgns_1_1neoswarm_1_1network_1_1_result_aggregation_1_1_config/)**  |

## Detailed Description

Timeout-bounded collection of swarm node responses (PTDS §4.2). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_NETWORK_RESULTAGGREGATION_HPP
#define NEOSWARM_NETWORK_RESULTAGGREGATION_HPP

#include "common/error.hpp"
#include "common/types.hpp"
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <vector>

namespace sgns::neoswarm::network
{
    class ResultAggregation
    {
        public:
        struct Config
        {
            std::chrono::milliseconds m_timeout{ 5000 }; 
            size_t min_responses_ = 1;                  
            size_t max_responses_ = 10;                 
        };

        ResultAggregation();
        explicit ResultAggregation( Config cfg );

        void Submit( const NodeOutput& output );

        outcome::result<std::vector<NodeOutput>> Collect();

        void Reset();

        size_t ResponseCount() const;

        private:
        Config m_cfg;
        std::vector<NodeOutput> results_;
        mutable std::mutex m_mutex;
        std::condition_variable cv_;
        bool done_ = false;
    };

} // namespace sgns::neoswarm::network

#endif // NEOSWARM_NETWORK_RESULTAGGREGATION_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
