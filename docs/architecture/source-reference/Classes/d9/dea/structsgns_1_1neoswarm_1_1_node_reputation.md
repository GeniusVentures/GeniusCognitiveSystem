---
title: sgns::neoswarm::NodeReputation

---

# sgns::neoswarm::NodeReputation






`#include <types.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[m_identityKey](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m-identitykey)**  |
| double | **[m_globalScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m-globalscore)**  |
| double | **[m_mathScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m-mathscore)**  |
| double | **[m_grammarScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m-grammarscore)**  |
| double | **[m_latencyScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m-latencyscore)**  |
| double | **[m_consistencyScore](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m-consistencyscore)**  |
| uint64_t | **[m_taskCount](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m-taskcount)**  |
| uint64_t | **[m_lastUpdatedMs](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/#variable-m-lastupdatedms)** <br/>Unix epoch ms.  |
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

Updated on 2026-06-28 at 13:58:22 -0700