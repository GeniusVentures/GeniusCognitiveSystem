---
title: GNUS-NEO-SWARM/src/core/tokenizer/sentence_piece_tokenizer.cpp
summary: SentencePiece tokenizer implementation. 

---

# GNUS-NEO-SWARM/src/core/tokenizer/sentence_piece_tokenizer.cpp



SentencePiece tokenizer implementation.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::core](/source-reference/Namespaces/d2/db7/namespacesgns_1_1neoswarm_1_1core/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[sgns::neoswarm::core::SentencePieceTokenizer::Impl](/source-reference/Classes/d1/d1d/structsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer_1_1_impl/)**  |

## Detailed Description

SentencePiece tokenizer implementation. 

**Date**: 2026-05-06 



## Source code

```cpp


#include "tokenizer.hpp"
#include "common/logging.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

#include <sentencepiece_processor.h>

namespace sgns::neoswarm::core
{
    namespace
    {
        auto TokenizerLogger()
        {
            return neoswarm::CreateLogger( "Tokenizer" );
        }
    } // namespace

    struct SentencePieceTokenizer::Impl
    {
        sentencepiece::SentencePieceProcessor m_processor;
        bool m_loaded = false;
    };

    SentencePieceTokenizer::SentencePieceTokenizer( int eos_id, int bos_id )
        : impl_( std::make_unique<Impl>() )
        , m_eosId( eos_id )
        , m_bosId( bos_id )
    {
    }

    SentencePieceTokenizer::~SentencePieceTokenizer() = default;

    // -----------------------------------------------------------------------
    // Load
    // -----------------------------------------------------------------------
    outcome::result<void> SentencePieceTokenizer::Load( const std::string& model_path )
    {
        auto status = impl_->m_processor.Load( model_path );
        if ( !status.ok() )
        {
            return outcome::failure( Error::TokenizerFailed );
        }
        impl_->m_loaded = true;
        TokenizerLogger()->info( "Tokenizer loaded: {} (vocab={})", model_path, VocabSize() );
        return outcome::success();

    }

    // -----------------------------------------------------------------------
    // Encode
    // -----------------------------------------------------------------------
    outcome::result<std::vector<int>> SentencePieceTokenizer::Encode( const std::string& text ) const
    {
        if ( !impl_->m_loaded )
        {
            return outcome::failure( Error::TokenizerFailed );
        }
        std::vector<int> ids;
        auto status = impl_->m_processor.Encode( text, &ids );
        if ( !status.ok() )
        {
            return outcome::failure( Error::TokenizerFailed );
        }
        return outcome::success( std::move( ids ) );

    }

    // -----------------------------------------------------------------------
    // Decode
    // -----------------------------------------------------------------------
    outcome::result<std::string> SentencePieceTokenizer::Decode( const std::vector<int>& ids ) const
    {
        if ( !impl_->m_loaded )
        {
            return outcome::failure( Error::TokenizerFailed );
        }
        std::string text;
        auto status = impl_->m_processor.Decode( ids, &text );
        if ( !status.ok() )
        {
            return outcome::failure( Error::TokenizerFailed );
        }
        return outcome::success( std::move( text ) );

    }

    // -----------------------------------------------------------------------
    // VocabSize
    // -----------------------------------------------------------------------
    size_t SentencePieceTokenizer::VocabSize() const
    {
        if ( impl_->m_loaded )
        {
            return static_cast<size_t>( impl_->m_processor.GetPieceSize() );
        }
        return 0; // unknown until model is loaded
    }

} // namespace sgns::neoswarm::core
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
