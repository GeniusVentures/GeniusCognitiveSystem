---
title: sgns::neoswarm::knowledge::KnowledgeRetrieval::Config

---

# sgns::neoswarm::knowledge::KnowledgeRetrieval::Config






`#include <knowledge_retrieval.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[index_path_](/source-reference/Classes/d0/d0c/structsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval_1_1_config/#variable-index-path-)** <br/>path to HNSW index file (future)  |
| std::string | **[m_factsPath](/source-reference/Classes/d0/d0c/structsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval_1_1_config/#variable-m-factspath)** <br/>path to facts CSV  |
| int | **[top_k_](/source-reference/Classes/d0/d0c/structsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval_1_1_config/#variable-top-k-)** <br/>number of facts to retrieve  |
| float | **[min_score_](/source-reference/Classes/d0/d0c/structsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval_1_1_config/#variable-min-score-)** <br/>minimum relevance score  |
| bool | **[enabled_](/source-reference/Classes/d0/d0c/structsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval_1_1_config/#variable-enabled-)**  |

## Public Attributes Documentation

### variable index_path_

```cpp
std::string index_path_ = "";
```

path to HNSW index file (future) 

### variable m_factsPath

```cpp
std::string m_factsPath = "";
```

path to facts CSV 

### variable top_k_

```cpp
int top_k_ = 3;
```

number of facts to retrieve 

### variable min_score_

```cpp
float min_score_ = 0.5f;
```

minimum relevance score 

### variable enabled_

```cpp
bool enabled_ = true;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700