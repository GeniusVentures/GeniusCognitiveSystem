---
title: GNUS-NEO-SWARM/src/knowledge/knowledge_retrieval.hpp

---

# GNUS-NEO-SWARM/src/knowledge/knowledge_retrieval.hpp





## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::knowledge](/source-reference/Namespaces/d8/da0/namespacesgns_1_1neoswarm_1_1knowledge/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::knowledge::KnowledgeRetrieval](/source-reference/Classes/d3/dd7/classsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval/)** <br/>Retrieves top-k structured facts from a Grokipedia index.  |
| struct | **[sgns::neoswarm::knowledge::KnowledgeRetrieval::Config](/source-reference/Classes/d0/d0c/structsgns_1_1neoswarm_1_1knowledge_1_1_knowledge_retrieval_1_1_config/)**  |




## Source code

```cpp


#ifndef NEOSWARM_KNOWLEDGE_KNOWLEDGERETRIEVAL_HPP
#define NEOSWARM_KNOWLEDGE_KNOWLEDGERETRIEVAL_HPP

#include "common/error.hpp"
#include "common/types.hpp"
#include <memory>
#include <string>
#include <vector>

namespace sgns::neoswarm::knowledge
{
    class KnowledgeRetrieval
    {
        public:
        struct Config
        {
            std::string index_path_ = ""; 
            std::string m_factsPath = ""; 
            int top_k_ = 3;               
            float min_score_ = 0.5f;      
            bool enabled_ = true;
        };

        KnowledgeRetrieval();
        explicit KnowledgeRetrieval( Config cfg );
        ~KnowledgeRetrieval();

        outcome::result<void> Load();

        bool IsLoaded() const
        {
            return m_loaded;
        }

        outcome::result<std::vector<KnowledgeFact>> Retrieve( const std::string& query ) const;

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
        Config m_cfg;
        bool m_loaded = false;

        std::vector<float> Embed( const std::string& text ) const;

        static float CosineSimilarity( const std::vector<float>& a, const std::vector<float>& b );
    };

} // namespace sgns::neoswarm::knowledge

#endif // NEOSWARM_KNOWLEDGE_KNOWLEDGERETRIEVAL_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
