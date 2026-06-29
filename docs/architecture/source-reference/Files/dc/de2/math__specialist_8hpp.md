---
title: GNUS-NEO-SWARM/src/specialists/math_specialist.hpp
summary: GSM8K-tuned math specialist model (PTDS §5.2). 

---

# GNUS-NEO-SWARM/src/specialists/math_specialist.hpp



GSM8K-tuned math specialist model (PTDS §5.2).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::specialists](/source-reference/Namespaces/de/d04/namespacesgns_1_1neoswarm_1_1specialists/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::specialists::MathSpecialist](/source-reference/Classes/df/d42/classsgns_1_1neoswarm_1_1specialists_1_1_math_specialist/)** <br/>1–3B parameter GSM8K-tuned math model (PTDS §5.2).  |

## Detailed Description

GSM8K-tuned math specialist model (PTDS §5.2). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_SPECIALISTS_MATHSPECIALIST_HPP
#define NEOSWARM_SPECIALISTS_MATHSPECIALIST_HPP

#include "i_specialist.hpp"
#include "symbolic_fallback.hpp"
#include "core/engine/inference_engine.hpp"
#include <memory>
#include <optional>

namespace sgns::neoswarm::specialists
{
    class MathSpecialist : public ISpecialist
    {
        public:
        explicit MathSpecialist( std::shared_ptr<core::InferenceEngine> engine = nullptr );

        std::string GetName() const override
        {
            return "MathSpecialist";
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
        std::optional<std::string> TrySymbolicFallback( const std::string& input ) const;
    };

} // namespace sgns::neoswarm::specialists

#endif // NEOSWARM_SPECIALISTS_MATHSPECIALIST_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
