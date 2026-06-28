---
title: GNUS-NEO-SWARM/src/core/tokenizer/tokenizer.hpp
summary: Abstract tokenizer interface and SentencePiece implementation. 

---

# GNUS-NEO-SWARM/src/core/tokenizer/tokenizer.hpp



Abstract tokenizer interface and SentencePiece implementation.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::core](/source-reference/Namespaces/d2/db7/namespacesgns_1_1neoswarm_1_1core/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::core::Tokenizer](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/)** <br/>Abstract tokenizer interface.  |
| class | **[sgns::neoswarm::core::SentencePieceTokenizer](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/)** <br/>SentencePiece tokenizer.  |

## Detailed Description

Abstract tokenizer interface and SentencePiece implementation. 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_CORE_TOKENIZER_TOKENIZER_HPP
#define NEOSWARM_CORE_TOKENIZER_TOKENIZER_HPP

#include "common/error.hpp"
#include <memory>
#include <string>
#include <vector>

namespace sgns::neoswarm::core
{
    class Tokenizer
    {
        public:
        virtual ~Tokenizer() = default;

        virtual outcome::result<std::vector<int>> Encode( const std::string& text ) const = 0;

        virtual outcome::result<std::string> Decode( const std::vector<int>& ids ) const = 0;

        virtual bool IsEOS( int token_id ) const = 0;

        virtual int EosTokenId() const = 0;

        virtual int BosTokenId() const = 0;

        virtual size_t VocabSize() const = 0;
    };

    class SentencePieceTokenizer : public Tokenizer
    {
        public:
        explicit SentencePieceTokenizer( int eos_id = 2, int bos_id = 1 );
        ~SentencePieceTokenizer() override;

        outcome::result<void> Load( const std::string& model_path );

        outcome::result<std::vector<int>> Encode( const std::string& text ) const override;
        outcome::result<std::string> Decode( const std::vector<int>& ids ) const override;
        bool IsEOS( int token_id ) const override
        {
            return token_id == m_eosId;
        }
        int EosTokenId() const override
        {
            return m_eosId;
        }
        int BosTokenId() const override
        {
            return m_bosId;
        }
        size_t VocabSize() const override;

        private:
        struct Impl;
        std::unique_ptr<Impl> impl_;
        int m_eosId;
        int m_bosId;
    };

} // namespace sgns::neoswarm::core

#endif // NEOSWARM_CORE_TOKENIZER_TOKENIZER_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
