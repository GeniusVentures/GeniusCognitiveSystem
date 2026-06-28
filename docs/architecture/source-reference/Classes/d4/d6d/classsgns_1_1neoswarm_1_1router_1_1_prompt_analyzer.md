---
title: sgns::neoswarm::router::PromptAnalyzer
summary: Analyses a prompt string and returns a feature vector used by the router. 

---

# sgns::neoswarm::router::PromptAnalyzer



Analyses a prompt string and returns a feature vector used by the router. 


`#include <prompt_analyzer.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| [PromptFeatures](/source-reference/Classes/d4/dc5/structsgns_1_1neoswarm_1_1_prompt_features/) | **[Analyze](/source-reference/Classes/d4/d6d/classsgns_1_1neoswarm_1_1router_1_1_prompt_analyzer/#function-analyze)**(const std::string & prompt) const<br/>Analyse a prompt and return its feature vector.  |

## Public Functions Documentation

### function Analyze

```cpp
PromptFeatures Analyze(
    const std::string & prompt
) const
```

Analyse a prompt and return its feature vector. 

**Parameters**: 

  * **prompt** Raw user prompt string. 


**Return**: [PromptFeatures](/source-reference/Classes/d4/dc5/structsgns_1_1neoswarm_1_1_prompt_features/) struct populated with extracted features. 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700