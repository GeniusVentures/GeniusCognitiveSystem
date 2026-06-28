---
title: sgns::neoswarm::knowledge::ContextInjection
summary: Prepends retrieved Grokipedia facts to a prompt before inference. 

---

# sgns::neoswarm::knowledge::ContextInjection



Prepends retrieved Grokipedia facts to a prompt before inference. 


`#include <context_injection.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/d7/dee/structsgns_1_1neoswarm_1_1knowledge_1_1_context_injection_1_1_config/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[ContextInjection](/source-reference/Classes/d9/da2/classsgns_1_1neoswarm_1_1knowledge_1_1_context_injection/#function-contextinjection)**() |
| | **[ContextInjection](/source-reference/Classes/d9/da2/classsgns_1_1neoswarm_1_1knowledge_1_1_context_injection/#function-contextinjection)**([Config](/source-reference/Classes/d7/dee/structsgns_1_1neoswarm_1_1knowledge_1_1_context_injection_1_1_config/) cfg) |
| std::string | **[Inject](/source-reference/Classes/d9/da2/classsgns_1_1neoswarm_1_1knowledge_1_1_context_injection/#function-inject)**(const std::string & prompt, const std::vector< [KnowledgeFact](/source-reference/Classes/d5/d9b/structsgns_1_1neoswarm_1_1_knowledge_fact/) > & facts) const<br/>Inject facts into a prompt before inference.  |

## Public Functions Documentation

### function ContextInjection

```cpp
ContextInjection()
```


### function ContextInjection

```cpp
explicit ContextInjection(
    Config cfg
)
```


### function Inject

```cpp
std::string Inject(
    const std::string & prompt,
    const std::vector< KnowledgeFact > & facts
) const
```

Inject facts into a prompt before inference. 

**Parameters**: 

  * **prompt** Original user prompt. 
  * **facts** Retrieved knowledge facts. 


**Return**: Augmented prompt string. 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700