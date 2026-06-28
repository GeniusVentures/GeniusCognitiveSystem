---
title: GNUS-NEO-SWARM/src/router/rule_based_router.hpp
summary: Rule-based prompt router (PTDS §6.1). 

---

# GNUS-NEO-SWARM/src/router/rule_based_router.hpp



Rule-based prompt router (PTDS §6.1).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::router](/source-reference/Namespaces/df/d79/namespacesgns_1_1neoswarm_1_1router/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::router::RuleBasedRouter](/source-reference/Classes/d6/def/classsgns_1_1neoswarm_1_1router_1_1_rule_based_router/)** <br/>MVP rule-based routing (PTDS §6.1).  |
| struct | **[sgns::neoswarm::router::RuleBasedRouter::Config](/source-reference/Classes/df/d02/structsgns_1_1neoswarm_1_1router_1_1_rule_based_router_1_1_config/)**  |

## Detailed Description

Rule-based prompt router (PTDS §6.1). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_ROUTER_RULEBASEDROUTER_HPP
#define NEOSWARM_ROUTER_RULEBASEDROUTER_HPP

#include "i_router.hpp"
#include "prompt_analyzer.hpp"

namespace sgns::neoswarm::router
{
    class RuleBasedRouter : public IRouter
    {
        public:
        struct Config
        {
            float numeric_density_threshold_ = 0.30f;
            float complexity_swarm_threshold_ = 5.0f;
            bool enable_swarm_m_mode = true;
        };

        RuleBasedRouter();
        explicit RuleBasedRouter( Config cfg );

        outcome::result<RouteDecision> Route( const Task& task ) override;

        private:
        Config m_cfg;
        PromptAnalyzer m_analyzer;

        ExecutionMode SelectMode( const PromptFeatures& features, ExecutionMode requested ) const;
    };

} // namespace sgns::neoswarm::router

#endif // NEOSWARM_ROUTER_RULEBASEDROUTER_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
