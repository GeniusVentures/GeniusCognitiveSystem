---
title: sgns::neoswarm::RouteDecision

---

# sgns::neoswarm::RouteDecision






`#include <types.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| [RouteTarget](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-routetarget) | **[m_target](/source-reference/Classes/db/d13/structsgns_1_1neoswarm_1_1_route_decision/#variable-m-target)**  |
| float | **[confidence_](/source-reference/Classes/db/d13/structsgns_1_1neoswarm_1_1_route_decision/#variable-confidence-)**  |
| std::string | **[m_reasoning](/source-reference/Classes/db/d13/structsgns_1_1neoswarm_1_1_route_decision/#variable-m-reasoning)**  |
| [ExecutionMode](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-executionmode) | **[m_mode](/source-reference/Classes/db/d13/structsgns_1_1neoswarm_1_1_route_decision/#variable-m-mode)**  |

## Public Attributes Documentation

### variable m_target

```cpp
RouteTarget m_target = RouteTarget::CoreOnly;
```


### variable confidence_

```cpp
float confidence_ = 1.0f;
```


### variable m_reasoning

```cpp
std::string m_reasoning;
```


### variable m_mode

```cpp
ExecutionMode m_mode = ExecutionMode::SingleNode;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700