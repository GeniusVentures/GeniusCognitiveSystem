---
title: GNUS-NEO-SWARM/src/core/engine/inference_engine.hpp
summary: Abstract inference engine interface. 

---

# GNUS-NEO-SWARM/src/core/engine/inference_engine.hpp



Abstract inference engine interface.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::core](/source-reference/Namespaces/d2/db7/namespacesgns_1_1neoswarm_1_1core/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::core::InferenceEngine](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/)** <br/>Abstract interface for all inference backends.  |

## Detailed Description

Abstract inference engine interface. 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_CORE_ENGINE_INFERENCEENGINE_HPP
#define NEOSWARM_CORE_ENGINE_INFERENCEENGINE_HPP

#include "common/error.hpp"
#include "common/types.hpp"
#include <functional>
#include <string>

namespace sgns::neoswarm::core
{
    class InferenceEngine
    {
        public:
        virtual ~InferenceEngine() = default;

        virtual outcome::result<void> LoadModel( const std::string& model_path ) = 0;

        virtual outcome::result<InferenceResponse> Infer( const Task& task ) = 0;

        virtual outcome::result<void> StreamInfer( const Task& task,
                                                   std::function<void( const std::string& token )> callback ) = 0;

        virtual bool IsLoaded() const = 0;

        virtual std::string BackendName() const = 0;
    };

} // namespace sgns::neoswarm::core

#endif // NEOSWARM_CORE_ENGINE_INFERENCEENGINE_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
