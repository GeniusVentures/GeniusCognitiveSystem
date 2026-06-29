---
title: sgns::neoswarm::NodeReputation

---

# sgns::neoswarm::NodeReputation






`#include <types.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[m_identityKey](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m_identitykey)**  |
| double | **[m_globalScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m_globalscore)**  |
| double | **[m_mathScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m_mathscore)**  |
| double | **[m_grammarScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m_grammarscore)**  |
| double | **[m_latencyScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m_latencyscore)**  |
| double | **[m_consistencyScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m_consistencyscore)**  |
| uint64_t | **[m_taskCount](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m_taskcount)**  |
| uint64_t | **[m_lastUpdatedMs](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m_lastupdatedms)** <br/>Unix epoch ms.  |
| uint64_t | **[kMinTasksForHighTrust](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-kmintasksforhightrust)** <br/>Minimum tasks before high-trust (anti-gaming).  |

## Public Attributes Documentation

### variable m_identityKey

```cpp
std::string m_identityKey;
```


### variable m_globalScore

```cpp
double m_globalScore = 0.5;
```


### variable m_mathScore

```cpp
double m_mathScore = 0.5;
```


### variable m_grammarScore

```cpp
double m_grammarScore = 0.5;
```


### variable m_latencyScore

```cpp
double m_latencyScore = 0.5;
```


### variable m_consistencyScore

```cpp
double m_consistencyScore = 0.5;
```


### variable m_taskCount

```cpp
uint64_t m_taskCount = 0;
```


### variable m_lastUpdatedMs

```cpp
uint64_t m_lastUpdatedMs = 0;
```

Unix epoch ms. 

### variable kMinTasksForHighTrust

```cpp
static uint64_t kMinTasksForHighTrust = 10;
```

Minimum tasks before high-trust (anti-gaming). 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700