---
title: sgns::neoswarm::knowledge::FactValidation
summary: Checks factual claims in generated output against Grokipedia. 

---

# sgns::neoswarm::knowledge::FactValidation



Checks factual claims in generated output against Grokipedia.  [More...](#detailed-description)


`#include <fact_validation.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[ValidationResult](/source-reference/Classes/df/dd5/structsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation_1_1_validation_result/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[FactValidation](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/#function-factvalidation)**(std::shared_ptr< [KnowledgeRetrieval](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/) > retrieval)<br/>Construct with a shared knowledge retrieval instance.  |
| [ValidationResult](/source-reference/Classes/df/dd5/structsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation_1_1_validation_result/) | **[Validate](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/#function-validate)**(const std::string & output, const std::vector< [KnowledgeFact](/source-reference/Classes/d5/d9b/structsgns_1_1neoswarm_1_1_knowledge_fact/) > & grounding_facts) const<br/>Validate generated output against retrieved grounding facts.  |
| bool | **[IsAvailable](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/#function-isavailable)**() const |

## Detailed Description

```cpp
class sgns::neoswarm::knowledge::FactValidation;
```

Checks factual claims in generated output against Grokipedia. 

A contradiction lowers the node's consistency_score and may trigger regeneration. 

## Public Functions Documentation

### function FactValidation

```cpp
explicit FactValidation(
    std::shared_ptr< KnowledgeRetrieval > retrieval
)
```

Construct with a shared knowledge retrieval instance. 

**Parameters**: 

  * **retrieval** Loaded [KnowledgeRetrieval](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/) to check against. 


### function Validate

```cpp
ValidationResult Validate(
    const std::string & output,
    const std::vector< KnowledgeFact > & grounding_facts
) const
```

Validate generated output against retrieved grounding facts. 

**Parameters**: 

  * **output** Generated text to validate. 
  * **grounding_facts** Facts that were injected into the prompt. 


**Return**: [ValidationResult](/source-reference/Classes/df/dd5/structsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation_1_1_validation_result/) with contradiction details. 

### function IsAvailable

```cpp
bool IsAvailable() const
```


**Return**: True if the retrieval index is loaded and validation is possible. 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700