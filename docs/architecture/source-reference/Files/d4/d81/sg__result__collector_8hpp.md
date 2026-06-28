---
title: GNUS-NEO-SWARM/src/network/sg_client/sg_result_collector.hpp
summary: Subscribes to per-job result channels and collects TaskResult messages. 

---

# GNUS-NEO-SWARM/src/network/sg_client/sg_result_collector.hpp



Subscribes to per-job result channels and collects TaskResult messages.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[grpc](/source-reference/Namespaces/d4/d4f/namespacegrpc/)**  |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[sgns::neoswarm::network::SGResultCollectorConfig](/source-reference/Classes/d1/dd3/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_config/)**  |
| class | **[sgns::neoswarm::network::SGResultCollector](/source-reference/Classes/de/d02/classsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector/)** <br/>Collects inference results from SuperGenius PubSub result channels.  |

## Detailed Description

Subscribes to per-job result channels and collects TaskResult messages. 

**Date**: 2026-05-28 



## Source code

```cpp


#ifndef NEOSWARM_NETWORK_SG_CLIENT_SGRESULTCOLLECTOR_HPP
#define NEOSWARM_NETWORK_SG_CLIENT_SGRESULTCOLLECTOR_HPP

#include "common/error.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace grpc
{
    class Channel;
}

namespace sgns::neoswarm::network
{
    class SGMessageAuthenticator;

    struct SGResultCollectorConfig
    {
        std::chrono::seconds result_m_timeout{ 300 };
    };

    class SGResultCollector
    {
        public:
        SGResultCollector( std::shared_ptr<grpc::Channel> channel,
                           SGMessageAuthenticator& authenticator,
                           SGResultCollectorConfig cfg = {} );
        ~SGResultCollector();

        outcome::result<std::vector<uint8_t>> WaitForResult( const std::string& taskId, std::chrono::seconds timeout );

        outcome::result<std::vector<uint8_t>> WaitForResult( const std::string& taskId );

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace sgns::neoswarm::network

#endif // NEOSWARM_NETWORK_SG_CLIENT_SGRESULTCOLLECTOR_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
