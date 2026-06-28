---
title: GNUS-NEO-SWARM/src/knowledge/fact_validation.hpp
summary: Post-generation fact checking against Grokipedia (PTDS §8.3). 

---

# GNUS-NEO-SWARM/src/knowledge/fact_validation.hpp



Post-generation fact checking against Grokipedia (PTDS §8.3).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::knowledge](/source-reference/Namespaces/d8/da0/namespacesgns_1_1neoswarm_1_1knowledge/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::knowledge::FactValidation](/source-reference/Classes/d3/dce/classsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation/)** <br/>Checks factual claims in generated output against Grokipedia.  |
| struct | **[sgns::neoswarm::knowledge::FactValidation::ValidationResult](/source-reference/Classes/df/dd5/structsgns_1_1neoswarm_1_1knowledge_1_1_fact_validation_1_1_validation_result/)**  |

## Detailed Description

Post-generation fact checking against Grokipedia (PTDS §8.3). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_KNOWLEDGE_FACTVALIDATION_HPP
#define NEOSWARM_KNOWLEDGE_FACTVALIDATION_HPP

#include "knowledge_retrieval.hpp"
#include "common/types.hpp"
#include <memory>
#include <string>
#include <vector>

namespace sgns::neoswarm::knowledge
{
    class FactValidation
    {
        public:
        struct ValidationResult
        {
            bool passed_ = true;
            float m_contradictionScore = 0.0f;
            std::vector<std::string> m_contradictions;
            std::string suggestion_;
        };

        explicit FactValidation( std::shared_ptr<KnowledgeRetrieval> retrieval );

        ValidationResult Validate( const std::string& output, const std::vector<KnowledgeFact>& grounding_facts ) const;

        bool IsAvailable() const;

        private:
        std::shared_ptr<KnowledgeRetrieval> retrieval_;

        std::vector<std::pair<std::string, double>> ExtractNumericClaims( const std::string& text ) const;

        bool Contradicts( double claim, const std::string& fact_content ) const;
    };

} // namespace sgns::neoswarm::knowledge

#endif // NEOSWARM_KNOWLEDGE_FACTVALIDATION_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
