---
title: GNUS-NEO-SWARM/src/specialists/i_specialist.hpp
summary: Abstract interface for all specialist modules (PTDS §5.2). 

---

# GNUS-NEO-SWARM/src/specialists/i_specialist.hpp



Abstract interface for all specialist modules (PTDS §5.2).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::specialists](/source-reference/Namespaces/de/d04/namespacesgns_1_1neoswarm_1_1specialists/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::specialists::ISpecialist](/source-reference/Classes/df/df6/classsgns_1_1neoswarm_1_1specialists_1_1_i_specialist/)** <br/>Abstract interface for specialist post-processing modules.  |

## Detailed Description

Abstract interface for all specialist modules (PTDS §5.2). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_SPECIALISTS_ISPECIALIST_HPP
#define NEOSWARM_SPECIALISTS_ISPECIALIST_HPP

#include "common/error.hpp"
#include <string>

namespace sgns::neoswarm::specialists
{
    class ISpecialist
    {
        public:
        virtual ~ISpecialist() = default;

        virtual std::string GetName() const = 0;

        virtual bool IsLoaded() const = 0;

        virtual outcome::result<void> Load( const std::string& model_path ) = 0;

        virtual outcome::result<std::string> Process( const std::string& input ) = 0;

        virtual float GetConfidence() const = 0;
    };

} // namespace sgns::neoswarm::specialists

#endif // NEOSWARM_SPECIALISTS_ISPECIALIST_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
