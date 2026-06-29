---
title: GNUS-NEO-SWARM/src/network/sg_client/super_genius_client.hpp
summary: Client for SuperGenius blockchain compute network dispatch via PubSub gRPC. 

---

# GNUS-NEO-SWARM/src/network/sg_client/super_genius_client.hpp



Client for SuperGenius blockchain compute network dispatch via PubSub gRPC.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::security](/source-reference/Namespaces/d7/d75/namespacesgns_1_1neoswarm_1_1security/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::network::SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/)** <br/>Client that bridges GNUS NEO SWARM to the SuperGenius blockchain compute network via PubSub-based gRPC dispatch.  |
| struct | **[sgns::neoswarm::network::SGClient::Config](/source-reference/Classes/df/dca/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_config/)** <br/>Configuration for SuperGenius network connectivity.  |

## Detailed Description

Client for SuperGenius blockchain compute network dispatch via PubSub gRPC. 

**Date**: 2026-05-28


Encapsulates all communication with the SuperGenius processing network. Manages a persistent gRPC channel, publishes signed Task messages to the grid channel via PubSub, and collects results from per-job result channels. 




## Source code

```cpp


#ifndef NEOSWARM_NETWORK_SG_CLIENT_SUPERGENIUSCLIENT_HPP
#define NEOSWARM_NETWORK_SG_CLIENT_SUPERGENIUSCLIENT_HPP

#include "common/error.hpp"
#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace sgns::neoswarm::security
{
    class NodeIdentity;
}

namespace sgns::neoswarm::network
{
    class SGClient
    {
        public:
        struct Config
        {
            std::string m_endpoint = "localhost:50051";   
            std::string m_tlsCaPath;                    
            std::string m_tlsCertPath;                  
            std::chrono::seconds channel_m_timeout{ 30 }; 
            std::chrono::seconds result_m_timeout{ 300 }; 
        };

        explicit SGClient( Config cfg );

        ~SGClient();

        // Non-copyable, movable
        SGClient( const SGClient& ) = delete;
        SGClient& operator=( const SGClient& ) = delete;
        SGClient( SGClient&& ) noexcept;
        SGClient& operator=( SGClient&& ) noexcept;

        outcome::result<void> Initialize( const security::NodeIdentity& identity );

        outcome::result<void> Connect();

        outcome::result<std::vector<uint8_t>> SubmitJob( const std::string& gnusSchemaJson );

        void Disconnect();

        bool IsConnected() const noexcept;

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace sgns::neoswarm::network

#endif // NEOSWARM_NETWORK_SG_CLIENT_SUPERGENIUSCLIENT_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
