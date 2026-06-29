---
title: GNUS-NEO-SWARM/src/specialists/symbolic_fallback.hpp
summary: Expression parser and evaluator for math validation (PTDS §5.2). 

---

# GNUS-NEO-SWARM/src/specialists/symbolic_fallback.hpp



Expression parser and evaluator for math validation (PTDS §5.2).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::specialists](/source-reference/Namespaces/de/d04/namespacesgns_1_1neoswarm_1_1specialists/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::specialists::SymbolicFallback](/source-reference/Classes/d1/d06/classsgns_1_1neoswarm_1_1specialists_1_1_symbolic_fallback/)** <br/>Evaluates mathematical expressions symbolically.  |

## Detailed Description

Expression parser and evaluator for math validation (PTDS §5.2). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_SPECIALISTS_SYMBOLICFALLBACK_HPP
#define NEOSWARM_SPECIALISTS_SYMBOLICFALLBACK_HPP

#include "common/error.hpp"
#include <optional>
#include <string>

namespace sgns::neoswarm::specialists
{
    class SymbolicFallback
    {
        public:
        static constexpr float kConfidenceThreshold = 0.6f;

        static std::optional<double> Evaluate( const std::string& expr );

        static std::optional<double> ExtractAndEvaluate( const std::string& text );

        static std::string FormatResult( double value );

        private:
        struct Parser
        {
            const std::string& input_;
            size_t pos_ = 0;

            double ParseExpr();
            double ParseTerm();
            double ParseFactor();
            double ParsePrimary();
            void SkipWhitespace();
            char Peek() const;
            char Consume();
        };
    };

} // namespace sgns::neoswarm::specialists

#endif // NEOSWARM_SPECIALISTS_SYMBOLICFALLBACK_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
