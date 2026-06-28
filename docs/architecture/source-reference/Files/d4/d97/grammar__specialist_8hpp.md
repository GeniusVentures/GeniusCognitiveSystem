---
title: GNUS-NEO-SWARM/src/specialists/grammar_specialist.hpp
summary: Grammar correction specialist model (PTDS §5.2). 

---

# GNUS-NEO-SWARM/src/specialists/grammar_specialist.hpp



Grammar correction specialist model (PTDS §5.2).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::specialists](/source-reference/Namespaces/de/d04/namespacesgns_1_1neoswarm_1_1specialists/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::specialists::GrammarSpecialist](/source-reference/Classes/d2/df3/classsgns_1_1neoswarm_1_1specialists_1_1_grammar_specialist/)** <br/>200M–500M parameter grammar correction model (PTDS §5.2).  |

## Detailed Description

Grammar correction specialist model (PTDS §5.2). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_SPECIALISTS_GRAMMARSPECIALIST_HPP
#define NEOSWARM_SPECIALISTS_GRAMMARSPECIALIST_HPP

#include "i_specialist.hpp"
#include "core/engine/inference_engine.hpp"
#include <memory>

namespace sgns::neoswarm::specialists
{
    class GrammarSpecialist : public ISpecialist
    {
        public:
        explicit GrammarSpecialist( std::shared_ptr<core::InferenceEngine> engine = nullptr );

        std::string GetName() const override
        {
            return "GrammarSpecialist";
        }
        bool IsLoaded() const override
        {
            return m_loaded;
        }

        outcome::result<void> Load( const std::string& model_path ) override;
        outcome::result<std::string> Process( const std::string& input ) override;
        float GetConfidence() const override
        {
            return last_confidence_;
        }

        private:
        std::shared_ptr<core::InferenceEngine> m_engine;
        bool m_loaded = false;
        float last_confidence_ = 0.0f;

        std::string BuildPrompt( const std::string& input ) const;
    };

} // namespace sgns::neoswarm::specialists

#endif // NEOSWARM_SPECIALISTS_GRAMMARSPECIALIST_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
