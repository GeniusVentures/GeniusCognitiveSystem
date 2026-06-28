---
title: sgns::neoswarm::knowledge::KnowledgeRetrieval
summary: Retrieves top-k structured facts from a Grokipedia index. 

---

# sgns::neoswarm::knowledge::KnowledgeRetrieval



Retrieves top-k structured facts from a Grokipedia index.  [More...](#detailed-description)


`#include <knowledge_retrieval.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/d0/d0c/structsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval_1_1_config/)**  |
| struct | **[Impl](/source-reference/Classes/db/d6e/structsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval_1_1_impl/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[KnowledgeRetrieval](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/#function-knowledgeretrieval)**() |
| | **[KnowledgeRetrieval](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/#function-knowledgeretrieval)**([Config](/source-reference/Classes/d0/d0c/structsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval_1_1_config/) cfg) |
| | **[~KnowledgeRetrieval](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/#function-~knowledgeretrieval)**() |
| outcome::result< void > | **[Load](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/#function-load)**()<br/>Load the knowledge index from disk.  |
| bool | **[IsLoaded](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/#function-isloaded)**() const |
| outcome::result< std::vector< [KnowledgeFact](/source-reference/Classes/d5/d9b/structsgns_1_1neoswarm_1_1_knowledge_fact/) > > | **[Retrieve](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/#function-retrieve)**(const std::string & query) const<br/>Retrieve top-k facts relevant to the query.  |

## Detailed Description

```cpp
class sgns::neoswarm::knowledge::KnowledgeRetrieval;
```

Retrieves top-k structured facts from a Grokipedia index. 

Uses a simple TF-IDF bag-of-words embedding with cosine similarity. Degrades gracefully when the index is unavailable. 

## Public Functions Documentation

### function KnowledgeRetrieval

```cpp
KnowledgeRetrieval()
```


### function KnowledgeRetrieval

```cpp
explicit KnowledgeRetrieval(
    Config cfg
)
```


### function ~KnowledgeRetrieval

```cpp
~KnowledgeRetrieval()
```


### function Load

```cpp
outcome::result< void > Load()
```

Load the knowledge index from disk. 

**Return**: outcome::success or KnowledgeUnavailable. 

### function IsLoaded

```cpp
inline bool IsLoaded() const
```


**Return**: True if the index has been loaded. 

### function Retrieve

```cpp
outcome::result< std::vector< KnowledgeFact > > Retrieve(
    const std::string & query
) const
```

Retrieve top-k facts relevant to the query. 

**Parameters**: 

  * **query** User prompt or search string. 


**Return**: Vector of [KnowledgeFact](/source-reference/Classes/d5/d9b/structsgns_1_1neoswarm_1_1_knowledge_fact/) or KnowledgeUnavailable. 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700