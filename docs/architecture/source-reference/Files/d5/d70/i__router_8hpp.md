---
title: GNUS-NEO-SWARM/src/router/i_router.hpp
summary: Abstract router interface for GNUS NEO SWARM. 

---

# GNUS-NEO-SWARM/src/router/i_router.hpp



Abstract router interface for GNUS NEO SWARM.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::router](/source-reference/Namespaces/df/d79/namespacesgns_1_1neoswarm_1_1router/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::router::IRouter](/source-reference/Classes/dc/da9/classsgns_1_1neoswarm_1_1router_1_1_i_router/)** <br/>Abstract interface for prompt routing strategies.  |

## Detailed Description

Abstract router interface for GNUS NEO SWARM. 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_ROUTER_IROUTER_HPP
#define NEOSWARM_ROUTER_IROUTER_HPP

#include "common/error.hpp"
#include "common/types.hpp"

namespace sgns::neoswarm::router
{
    class IRouter
    {
        public:
        virtual ~IRouter() = default;

        virtual outcome::result<RouteDecision> Route( const Task& task ) = 0;
    };

} // namespace sgns::neoswarm::router

#endif // NEOSWARM_ROUTER_IROUTER_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
