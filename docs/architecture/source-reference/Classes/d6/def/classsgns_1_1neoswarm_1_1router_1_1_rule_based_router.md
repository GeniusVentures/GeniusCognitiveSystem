---
title: sgns::neoswarm::router::RuleBasedRouter
summary: MVP rule-based routing (PTDS §6.1). 

---

# sgns::neoswarm::router::RuleBasedRouter



MVP rule-based routing (PTDS §6.1).  [More...](#detailed-description)


`#include <rule_based_router.hpp>`

Inherits from [sgns::neoswarm::router::IRouter](/source-reference/Classes/dc/da9/classsgns_1_1neoswarm_1_1router_1_1_i_router/)

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/df/d02/structsgns_1_1neoswarm_1_1router_1_1_rule_based_router_1_1_config/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[RuleBasedRouter](/source-reference/Classes/d6/def/classsgns_1_1neoswarm_1_1router_1_1_rule_based_router/#function-rulebasedrouter)**() |
| | **[RuleBasedRouter](/source-reference/Classes/d6/def/classsgns_1_1neoswarm_1_1router_1_1_rule_based_router/#function-rulebasedrouter)**([Config](/source-reference/Classes/df/d02/structsgns_1_1neoswarm_1_1router_1_1_rule_based_router_1_1_config/) cfg) |
| virtual outcome::result< [RouteDecision](/source-reference/Classes/db/d13/structsgns_1_1neoswarm_1_1_route_decision/) > | **[Route](/source-reference/Classes/d6/def/classsgns_1_1neoswarm_1_1router_1_1_rule_based_router/#function-route)**(const [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) & task) override<br/>Route a task to the appropriate execution mode and specialist.  |

## Additional inherited members

**Public Functions inherited from [sgns::neoswarm::router::IRouter](/source-reference/Classes/dc/da9/classsgns_1_1neoswarm_1_1router_1_1_i_router/)**

|                | Name           |
| -------------- | -------------- |
| virtual | **[~IRouter](/source-reference/Classes/dc/da9/classsgns_1_1neoswarm_1_1router_1_1_i_router/#function-~irouter)**() =default |


## Detailed Description

```cpp
class sgns::neoswarm::router::RuleBasedRouter;
```

MVP rule-based routing (PTDS §6.1). 

Decision tree: numeric_density > threshold OR has_math_keywords → CorePlusMath has_grammar_request → CorePlusGrammar has_code_syntax → CoreOnly (future: CorePlusCode) else → CoreOnly 

## Public Functions Documentation

### function RuleBasedRouter

```cpp
RuleBasedRouter()
```


### function RuleBasedRouter

```cpp
explicit RuleBasedRouter(
    Config cfg
)
```


### function Route

```cpp
virtual outcome::result< RouteDecision > Route(
    const Task & task
) override
```

Route a task to the appropriate execution mode and specialist. 

**Parameters**: 

  * **task** Incoming task. 


**Return**: [RouteDecision](/source-reference/Classes/db/d13/structsgns_1_1neoswarm_1_1_route_decision/) on success, [Error](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-error) on failure. 

**Reimplements**: [sgns::neoswarm::router::IRouter::Route](/source-reference/Classes/dc/da9/classsgns_1_1neoswarm_1_1router_1_1_i_router/#function-route)


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700