---
title: GNUS-NEO-SWARM/src/network/sg_client/sg_result_collector.cpp
summary: Timeout-bounded result collection from SuperGenius PubSub result channels. 

---

# GNUS-NEO-SWARM/src/network/sg_client/sg_result_collector.cpp



Timeout-bounded result collection from SuperGenius PubSub result channels.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[sgns::neoswarm::network::SGResultCollector::Impl](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/)**  |

## Detailed Description

Timeout-bounded result collection from SuperGenius PubSub result channels. 

**Date**: 2026-05-28 



## Source code

```cpp


#include "sg_result_collector.hpp"
#include "sg_message_authenticator.hpp"
#include "common/logging.hpp"
#include <condition_variable>
#include <mutex>

namespace sgns::neoswarm::network
{
    namespace
    {
        auto CollectLogger()
        {
            return CreateLogger( "NeoSwarm/SGCollect" );
        }
    } // namespace

    struct SGResultCollector::Impl
    {
        std::shared_ptr<grpc::Channel> m_channel;
        SGMessageAuthenticator& m_authenticator;
        SGResultCollectorConfig m_cfg;

        // Timeout-bounded collection using condition_variable
        // (matches existing ResultAggregation pattern)
        std::mutex m_mutex;
        std::condition_variable cv_;
        bool resultReady_ = false;
        std::vector<uint8_t> resultData_;

        Impl( std::shared_ptr<grpc::Channel> channel,
              SGMessageAuthenticator& authenticator,
              SGResultCollectorConfig cfg )
            : m_channel( std::move( channel ) )
            , m_authenticator( authenticator )
            , m_cfg( std::move( cfg ) )
        {
        }
    };

    SGResultCollector::SGResultCollector( std::shared_ptr<grpc::Channel> channel,
                                          SGMessageAuthenticator& authenticator,
                                          SGResultCollectorConfig cfg )
        : m_impl( std::make_unique<Impl>( std::move( channel ), authenticator, std::move( cfg ) ) )
    {
    }

    outcome::result<std::vector<uint8_t>> SGResultCollector::WaitForResult( const std::string& taskId,
                                                                            std::chrono::seconds timeout )
    {
        CollectLogger()->info( "Waiting for result on results/{} (timeout={}s)", taskId, timeout.count() );

        // Subscribe to results/<taskId> channel
        // TODO(Phase 2): implement actual gRPC PubSub subscribe when service stubs linked

        // Block until result arrives or timeout expires
        // Pattern matches ResultAggregation::Collect() in src/network/result_aggregation.cpp
        std::unique_lock<std::mutex> lock( m_impl->m_mutex );

        bool gotResult = m_impl->cv_.wait_for( lock, timeout, [this] { return m_impl->resultReady_; } );

        if ( !gotResult )
        {
            CollectLogger()->warn( "Result collection timed out for task {}", taskId );
            return outcome::failure( Error::BroadcastTimeout );
        }

        if ( m_impl->resultData_.empty() )
        {
            CollectLogger()->warn( "Empty result received for task {}", taskId );
            return outcome::failure( Error::InferenceFailed );
        }

        CollectLogger()->info( "Result collected for task {} ({} bytes)", taskId, m_impl->resultData_.size() );

        std::vector<uint8_t> result = std::move( m_impl->resultData_ );
        m_impl->resultReady_ = false;

        return outcome::success( result );
    }

    outcome::result<std::vector<uint8_t>> SGResultCollector::WaitForResult( const std::string& taskId )
    {
        return WaitForResult( taskId, m_impl->m_cfg.result_m_timeout );
    }

    SGResultCollector::~SGResultCollector() = default;

} // namespace sgns::neoswarm::network
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
