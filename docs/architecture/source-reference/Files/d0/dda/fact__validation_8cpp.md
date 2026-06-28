---
title: GNUS-NEO-SWARM/src/knowledge/fact_validation.cpp
summary: Post-generation fact checking implementation. 

---

# GNUS-NEO-SWARM/src/knowledge/fact_validation.cpp



Post-generation fact checking implementation.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::knowledge](/source-reference/Namespaces/d8/da0/namespacesgns_1_1neoswarm_1_1knowledge/)**  |

## Detailed Description

Post-generation fact checking implementation. 

**Date**: 2026-05-06 



## Source code

```cpp


#include "fact_validation.hpp"
#include "common/logging.hpp"

#include <algorithm>
#include <cmath>
#include <regex>
#include <sstream>

namespace sgns::neoswarm::knowledge
{
    namespace
    {
        auto ValidationLogger()
        {
            return neoswarm::CreateLogger( "FactValidation" );
        }
    } // namespace

    FactValidation::FactValidation( std::shared_ptr<KnowledgeRetrieval> retrieval )
        : retrieval_( std::move( retrieval ) )
    {
    }

    bool FactValidation::IsAvailable() const
    {
        return retrieval_ && retrieval_->IsLoaded();
    }

    // -----------------------------------------------------------------------
    // ExtractNumericClaims
    // -----------------------------------------------------------------------
    std::vector<std::pair<std::string, double>> FactValidation::ExtractNumericClaims( const std::string& text ) const
    {
        std::vector<std::pair<std::string, double>> claims;
        static const std::regex kNumPattern( R"((?:is|=|equals?|approximately|about|around)\s+([\d,]+(?:\.\d+)?))" );

        std::sregex_iterator it( text.begin(), text.end(), kNumPattern );
        std::sregex_iterator end;
        for ( ; it != end; ++it )
        {
            std::string num_str = ( *it )[1].str();
            num_str.erase( std::remove( num_str.begin(), num_str.end(), ',' ), num_str.end() );
            try
            {
                double val = std::stod( num_str );
                claims.push_back( { ( *it )[0].str(), val } );
            }
            catch ( ... )
            {
            }
        }
        return claims;
    }

    // -----------------------------------------------------------------------
    // Contradicts
    // -----------------------------------------------------------------------
    bool FactValidation::Contradicts( double claim, const std::string& fact_content ) const
    {
        static const std::regex kNumPattern( R"([\d,]+(?:\.\d+)?)" );
        std::sregex_iterator it( fact_content.begin(), fact_content.end(), kNumPattern );
        std::sregex_iterator end;
        for ( ; it != end; ++it )
        {
            std::string num_str = it->str();
            num_str.erase( std::remove( num_str.begin(), num_str.end(), ',' ), num_str.end() );
            try
            {
                double fact_val = std::stod( num_str );
                if ( fact_val == 0.0 )
                {
                    continue;
                }
                double rel_diff = std::abs( claim - fact_val ) / std::abs( fact_val );
                if ( rel_diff > 0.01 )
                {
                    return true; // >1% difference = contradiction
                }
            }
            catch ( ... )
            {
            }
        }
        return false;
    }

    // -----------------------------------------------------------------------
    // Validate
    // -----------------------------------------------------------------------
    FactValidation::ValidationResult FactValidation::Validate( const std::string& output,
                                                               const std::vector<KnowledgeFact>& grounding_facts ) const
    {
        ValidationResult result;

        if ( !IsAvailable() || grounding_facts.empty() )
        {
            ValidationLogger()->debug( "FactValidation: skipping (unavailable or no grounding facts)" );
            return result;
        }

        auto claims = ExtractNumericClaims( output );
        if ( claims.empty() )
        {
            return result;
        }

        int contradiction_count = 0;
        for ( const auto& [claim_text, claim_val] : claims )
        {
            for ( const auto& fact : grounding_facts )
            {
                if ( Contradicts( claim_val, fact.m_content ) )
                {
                    result.m_contradictions.push_back( "Claim '" + claim_text + "' may contradict: " + fact.m_content );
                    ++contradiction_count;
                }
            }
        }

        if ( contradiction_count > 0 )
        {
            result.passed_ = false;
            result.m_contradictionScore =
                std::min( static_cast<float>( contradiction_count ) / static_cast<float>( claims.size() ), 1.0f );
            result.suggestion_ = "Output contains " + std::to_string( contradiction_count ) +
                                 " potential contradiction(s) with Grokipedia facts.";
            ValidationLogger()->warn( "FactValidation: {} contradiction(s) detected", contradiction_count );
        }

        return result;
    }

} // namespace sgns::neoswarm::knowledge
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
