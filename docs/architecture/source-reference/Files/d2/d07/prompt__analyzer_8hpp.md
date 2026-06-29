---
title: GNUS-NEO-SWARM/src/router/prompt_analyzer.hpp
summary: Extracts routing features from a raw prompt string (PTDS §6.1). 

---

# GNUS-NEO-SWARM/src/router/prompt_analyzer.hpp



Extracts routing features from a raw prompt string (PTDS §6.1).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::router](/source-reference/Namespaces/df/d79/namespacesgns_1_1neoswarm_1_1router/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::router::PromptAnalyzer](/source-reference/Classes/d4/d6d/classsgns_1_1neoswarm_1_1router_1_1_prompt_analyzer/)** <br/>Analyses a prompt string and returns a feature vector used by the router.  |

## Detailed Description

Extracts routing features from a raw prompt string (PTDS §6.1). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_ROUTER_PROMPTANALYZER_HPP
#define NEOSWARM_ROUTER_PROMPTANALYZER_HPP

#include "common/types.hpp"
#include <string>

namespace sgns::neoswarm::router
{
    class PromptAnalyzer
    {
        public:
        PromptFeatures Analyze( const std::string& prompt ) const;

        private:
        float ComputeNumericDensity( const std::string& prompt ) const;

        bool DetectCodeSyntax( const std::string& prompt ) const;

        float EstimateComplexity( const std::string& prompt ) const;

        bool HasMathKeywords( const std::string& prompt ) const;

        bool HasGrammarRequest( const std::string& prompt ) const;

        size_t CountTokens( const std::string& text ) const;
    };

} // namespace sgns::neoswarm::router

#endif // NEOSWARM_ROUTER_PROMPTANALYZER_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
