---
title: GNUS-NEO-SWARM/src/knowledge/context_injection.cpp
summary: Prompt augmentation with Grokipedia facts. 

---

# GNUS-NEO-SWARM/src/knowledge/context_injection.cpp



Prompt augmentation with Grokipedia facts.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::knowledge](/source-reference/Namespaces/d8/da0/namespacesgns_1_1neoswarm_1_1knowledge/)**  |

## Detailed Description

Prompt augmentation with Grokipedia facts. 

**Date**: 2026-05-06 



## Source code

```cpp


#include "context_injection.hpp"

namespace sgns::neoswarm::knowledge
{
    ContextInjection::ContextInjection()
        : m_cfg( {} )
    {
    }
    ContextInjection::ContextInjection( Config cfg )
        : m_cfg( std::move( cfg ) )
    {
    }

    size_t ContextInjection::EstimateTokens( const std::string& text )
    {
        return text.size() / 4;
    }

    std::string ContextInjection::Inject( const std::string& prompt, const std::vector<KnowledgeFact>& facts ) const
    {
        if ( facts.empty() )
        {
            return prompt;
        }

        std::string context;
        size_t used_tokens = 0;

        for ( const auto& fact : facts )
        {
            std::string entry;
            if ( m_cfg.add_source_tags_ )
            {
                entry = "[GROKIPEDIA: " + fact.m_source + "] " + fact.m_content + "\n";
            }
            else
            {
                entry = fact.m_content + "\n";
            }

            size_t entry_tokens = EstimateTokens( entry );
            if ( used_tokens + entry_tokens > m_cfg.max_token_budget_ )
            {
                break;
            }

            context += entry;
            used_tokens += entry_tokens;
        }

        if ( context.empty() )
        {
            return prompt;
        }

        return "Context from Grokipedia:\n" + context + "\n" + prompt;
    }

} // namespace sgns::neoswarm::knowledge
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
