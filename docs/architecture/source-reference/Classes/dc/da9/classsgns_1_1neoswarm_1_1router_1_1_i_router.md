---
title: sgns::neoswarm::router::IRouter
summary: Abstract interface for prompt routing strategies. 

---

# sgns::neoswarm::router::IRouter



Abstract interface for prompt routing strategies. 


`#include <i_router.hpp>`

Inherited by [sgns::neoswarm::router::RuleBasedRouter](/source-reference/Classes/d6/def/classsgns_1_1neoswarm_1_1router_1_1_rule_based_router/)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual | **[~IRouter](/source-reference/Classes/dc/da9/classsgns_1_1neoswarm_1_1router_1_1_i_router/#function-~irouter)**() =default |
| virtual outcome::result< [RouteDecision](/source-reference/Classes/db/d13/structsgns_1_1neoswarm_1_1_route_decision/) > | **[Route](/source-reference/Classes/dc/da9/classsgns_1_1neoswarm_1_1router_1_1_i_router/#function-route)**(const [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) & task) =0<br/>Route a task to the appropriate execution mode and specialist.  |

## Public Functions Documentation

### function ~IRouter

```cpp
virtual ~IRouter() =default
```


### function Route

```cpp
virtual outcome::result< RouteDecision > Route(
    const Task & task
) =0
```

Route a task to the appropriate execution mode and specialist. 

**Parameters**: 

  * **task** Incoming task to route. 


**Return**: [RouteDecision](/source-reference/Classes/db/d13/structsgns_1_1neoswarm_1_1_route_decision/) on success, [Error](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-error) on failure. 

**Reimplemented by**: [sgns::neoswarm::router::RuleBasedRouter::Route](/source-reference/Classes/d6/def/classsgns_1_1neoswarm_1_1router_1_1_rule_based_router/#function-route)


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700