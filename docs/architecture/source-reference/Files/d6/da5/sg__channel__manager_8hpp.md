---
title: GNUS-NEO-SWARM/src/network/sg_client/sg_channel_manager.hpp
summary: Manages gRPC channel lifecycle — create, keepalive, reconnect, health check. 

---

# GNUS-NEO-SWARM/src/network/sg_client/sg_channel_manager.hpp



Manages gRPC channel lifecycle — create, keepalive, reconnect, health check.  [More...](#detailed-description)

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
| class | **[sgns::neoswarm::network::SGChannelManager](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/)** <br/>Manages a persistent gRPC channel to a SuperGenius node.  |
| struct | **[sgns::neoswarm::network::SGChannelManager::Config](/source-reference/Classes/d8/d2b/structsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager_1_1_config/)**  |

## Detailed Description

Manages gRPC channel lifecycle — create, keepalive, reconnect, health check. 

**Date**: 2026-05-28 



## Source code

```cpp


#ifndef NEOSWARM_NETWORK_SG_CLIENT_SGCHANNELMANAGER_HPP
#define NEOSWARM_NETWORK_SG_CLIENT_SGCHANNELMANAGER_HPP

#include "common/error.hpp"
#include <chrono>
#include <memory>
#include <string>

namespace grpc
{
    class Channel;
}

namespace sgns::neoswarm::network
{
    class SGChannelManager
    {
        public:
        struct Config
        {
            std::string m_endpoint = "localhost:50051";
            std::string m_tlsCaPath;
            std::string m_tlsCertPath;
            std::chrono::seconds m_timeout{ 30 };
        };

        explicit SGChannelManager( Config cfg );
        ~SGChannelManager() = default;

        outcome::result<void> CreateChannel();
        outcome::result<bool> HealthCheck() const;
        outcome::result<void> Reconnect();
        std::shared_ptr<grpc::Channel> GetChannel() const;

        bool IsConnected() const noexcept;

        private:
        Config m_cfg;
        std::shared_ptr<grpc::Channel> m_channel;
    };

} // namespace sgns::neoswarm::network

#endif // NEOSWARM_NETWORK_SG_CLIENT_SGCHANNELMANAGER_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
