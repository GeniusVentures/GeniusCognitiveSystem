---
title: GNUS-NEO-SWARM/src/network/result_aggregation.cpp
summary: Swarm response aggregation implementation. 

---

# GNUS-NEO-SWARM/src/network/result_aggregation.cpp



Swarm response aggregation implementation.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |

## Detailed Description

Swarm response aggregation implementation. 

**Date**: 2026-05-06 



## Source code

```cpp


#include "result_aggregation.hpp"
#include "common/logging.hpp"

namespace sgns::neoswarm::network
{
    namespace
    {
        auto AggregationLogger()
        {
            return neoswarm::CreateLogger( "ResultAggregation" );
        }
    } // namespace

    ResultAggregation::ResultAggregation()
        : m_cfg( {} )
    {
    }
    ResultAggregation::ResultAggregation( Config cfg )
        : m_cfg( std::move( cfg ) )
    {
    }

    // -----------------------------------------------------------------------
    // Submit
    // -----------------------------------------------------------------------
    void ResultAggregation::Submit( const NodeOutput& output )
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        if ( results_.size() >= m_cfg.max_responses_ )
        {
            return;
        }
        results_.push_back( output );
        AggregationLogger()->debug( "Received from {} ({}/{})", output.m_nodeId, results_.size(), m_cfg.max_responses_ );
        if ( results_.size() >= m_cfg.min_responses_ )
        {
            done_ = true;
            cv_.notify_all();
        }
    }

    // -----------------------------------------------------------------------
    // Collect
    // -----------------------------------------------------------------------
    outcome::result<std::vector<NodeOutput>> ResultAggregation::Collect()
    {
        std::unique_lock<std::mutex> lock( m_mutex );
        bool timed_out =
            !cv_.wait_for( lock, m_cfg.m_timeout, [this] { return done_ || results_.size() >= m_cfg.max_responses_; } );

        if ( timed_out && results_.empty() )
        {
            return outcome::failure( Error::BroadcastTimeout );
        }

        AggregationLogger()->info( "Collected {} responses (timeout={})", results_.size(), timed_out ? "yes" : "no" );
        return outcome::success( results_ );
    }

    // -----------------------------------------------------------------------
    // Reset
    // -----------------------------------------------------------------------
    void ResultAggregation::Reset()
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        results_.clear();
        done_ = false;
    }

    // -----------------------------------------------------------------------
    // ResponseCount
    // -----------------------------------------------------------------------
    size_t ResultAggregation::ResponseCount() const
    {
        std::lock_guard<std::mutex> lock( m_mutex );
        return results_.size();
    }

} // namespace sgns::neoswarm::network
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
