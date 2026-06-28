---
title: GNUS-NEO-SWARM/src/specialists/grammar_specialist.cpp
summary: Grammar specialist implementation. 

---

# GNUS-NEO-SWARM/src/specialists/grammar_specialist.cpp



Grammar specialist implementation.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::specialists](/source-reference/Namespaces/de/d04/namespacesgns_1_1neoswarm_1_1specialists/)**  |

## Detailed Description

Grammar specialist implementation. 

**Date**: 2026-05-06 



## Source code

```cpp


#include "grammar_specialist.hpp"
#include "common/logging.hpp"

#include <functional>

namespace sgns::neoswarm::specialists
{
    namespace
    {
        auto GrammarLogger()
        {
            return neoswarm::CreateLogger( "GrammarSpecialist" );
        }
    } // namespace

    GrammarSpecialist::GrammarSpecialist( std::shared_ptr<core::InferenceEngine> engine )
        : m_engine( std::move( engine ) )
    {
    }

    // -----------------------------------------------------------------------
    // Load
    // -----------------------------------------------------------------------
    outcome::result<void> GrammarSpecialist::Load( const std::string& model_path )
    {
        if ( !m_engine )
        {
            return outcome::failure( Error::ModelLoadFailed );
        }
        BOOST_OUTCOME_TRY( m_engine->LoadModel( model_path ) );
        m_loaded = true;
        GrammarLogger()->info( "GrammarSpecialist loaded: {}", model_path );
        return outcome::success();
    }

    // -----------------------------------------------------------------------
    // BuildPrompt
    // -----------------------------------------------------------------------
    std::string GrammarSpecialist::BuildPrompt( const std::string& input ) const
    {
        return "[INST] Correct the grammar, spelling, and fluency of the following text. "
               "Return only the corrected text without explanation.\n\n"
               "Text: " +
               input + "\n\nCorrected: [/INST]";
    }

    // -----------------------------------------------------------------------
    // Process
    // -----------------------------------------------------------------------
    outcome::result<std::string> GrammarSpecialist::Process( const std::string& input )
    {
        if ( !m_loaded || !m_engine )
        {
            GrammarLogger()->warn( "GrammarSpecialist not loaded — returning input unchanged" );
            last_confidence_ = 0.0f;
            return outcome::success( input );
        }

        Task task;
        task.m_id = "grammar-" + std::to_string( std::hash<std::string>{}( input ) );
        task.m_prompt = BuildPrompt( input );
        task.m_maxTokens = static_cast<uint32_t>( input.size() + 64 );
        task.m_temperature = 0.1f;

        auto res = m_engine->Infer( task );
        if ( !res.has_value() )
        {
            GrammarLogger()->warn( "GrammarSpecialist inference failed — returning input unchanged" );
            last_confidence_ = 0.0f;
            return outcome::success( input );
        }

        last_confidence_ = 1.0f - std::min( res.value().m_perplexity / 10.0f, 1.0f );
        return outcome::success( res.value().m_output );
    }

} // namespace sgns::neoswarm::specialists
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
