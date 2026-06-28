---
title: sgns::neoswarm::Task

---

# sgns::neoswarm::Task






`#include <types.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[m_id](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/#variable-m-id)**  |
| std::string | **[m_prompt](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/#variable-m-prompt)**  |
| [ExecutionMode](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-executionmode) | **[m_mode](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/#variable-m-mode)**  |
| uint32_t | **[m_maxTokens](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/#variable-m-maxtokens)**  |
| float | **[m_temperature](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/#variable-m-temperature)**  |
| std::string | **[m_nodeId](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/#variable-m-nodeid)** <br/>originating node  |

## Public Attributes Documentation

### variable m_id

```cpp
std::string m_id;
```


### variable m_prompt

```cpp
std::string m_prompt;
```


### variable m_mode

```cpp
ExecutionMode m_mode = ExecutionMode::SingleNode;
```


### variable m_maxTokens

```cpp
uint32_t m_maxTokens = 512;
```


### variable m_temperature

```cpp
float m_temperature = 0.7f;
```


### variable m_nodeId

```cpp
std::string m_nodeId;
```

originating node 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700