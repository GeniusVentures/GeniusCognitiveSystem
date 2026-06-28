---
title: GNUS-NEO-SWARM/src/core/sgprocessing/tensor_interpreter.hpp
summary: Converts raw SGProcessingManager output bytes to text. 

---

# GNUS-NEO-SWARM/src/core/sgprocessing/tensor_interpreter.hpp



Converts raw SGProcessingManager output bytes to text.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::core](/source-reference/Namespaces/d2/db7/namespacesgns_1_1neoswarm_1_1core/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::core::TensorInterpreter](/source-reference/Classes/d2/d9a/classsgns_1_1neoswarm_1_1core_1_1_tensor_interpreter/)** <br/>Converts raw [MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) tensor output bytes to a human-readable string.  |

## Detailed Description

Converts raw SGProcessingManager output bytes to text. 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_CORE_SGPROCESSING_TENSORINTERPRETER_HPP
#define NEOSWARM_CORE_SGPROCESSING_TENSORINTERPRETER_HPP

#include "common/error.hpp"
#include "core/tokenizer/tokenizer.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace sgns
{
    enum class InputFormat : int;
} // namespace sgns

namespace sgns::neoswarm::core
{
    class TensorInterpreter
    {
        public:
        TensorInterpreter() = default;
        ~TensorInterpreter() = default;

        void SetTokenizer( std::shared_ptr<Tokenizer> tok );

        outcome::result<std::string> Interpret( const std::vector<uint8_t>& bytes, sgns::InputFormat format ) const;

        private:
        std::shared_ptr<Tokenizer> m_tokenizer;

        outcome::result<std::string> InterpretFloat32( const std::vector<uint8_t>& bytes ) const;
        outcome::result<std::string> InterpretFloat16( const std::vector<uint8_t>& bytes ) const;
        outcome::result<std::string> InterpretInt32( const std::vector<uint8_t>& bytes ) const;
        outcome::result<std::string> InterpretInt8( const std::vector<uint8_t>& bytes ) const;
        outcome::result<std::string> DecodeLogits( const std::vector<float>& logits ) const;
    };

} // namespace sgns::neoswarm::core

#endif // NEOSWARM_CORE_SGPROCESSING_TENSORINTERPRETER_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
