---
title: GNUS-NEO-SWARM/src/knowledge/context_injection.hpp
summary: Augments prompts with Grokipedia facts (PTDS §8.2). 

---

# GNUS-NEO-SWARM/src/knowledge/context_injection.hpp



Augments prompts with Grokipedia facts (PTDS §8.2).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::knowledge](/source-reference/Namespaces/d8/da0/namespacesgns_1_1neoswarm_1_1knowledge/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::knowledge::ContextInjection](/source-reference/Classes/d9/da2/classsgns_1_1neoswarm_1_1knowledge_1_1_context_injection/)** <br/>Prepends retrieved Grokipedia facts to a prompt before inference.  |
| struct | **[sgns::neoswarm::knowledge::ContextInjection::Config](/source-reference/Classes/d7/dee/structsgns_1_1neoswarm_1_1knowledge_1_1_context_injection_1_1_config/)**  |

## Detailed Description

Augments prompts with Grokipedia facts (PTDS §8.2). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_KNOWLEDGE_CONTEXTINJECTION_HPP
#define NEOSWARM_KNOWLEDGE_CONTEXTINJECTION_HPP

#include "common/types.hpp"
#include <string>
#include <vector>

namespace sgns::neoswarm::knowledge
{
    class ContextInjection
    {
        public:
        struct Config
        {
            size_t max_token_budget_ = 256; 
            bool add_source_tags_ = true;   
        };

        ContextInjection();
        explicit ContextInjection( Config cfg );

        std::string Inject( const std::string& prompt, const std::vector<KnowledgeFact>& facts ) const;

        private:
        Config m_cfg;

        static size_t EstimateTokens( const std::string& text );
    };

} // namespace sgns::neoswarm::knowledge

#endif // NEOSWARM_KNOWLEDGE_CONTEXTINJECTION_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
