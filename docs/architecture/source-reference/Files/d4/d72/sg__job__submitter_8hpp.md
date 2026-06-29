---
title: GNUS-NEO-SWARM/src/network/sg_client/sg_job_submitter.hpp
summary: Publishes signed Task messages to the SuperGenius grid channel via PubSub. 

---

# GNUS-NEO-SWARM/src/network/sg_client/sg_job_submitter.hpp



Publishes signed Task messages to the SuperGenius grid channel via PubSub.  [More...](#detailed-description)

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
| class | **[sgns::neoswarm::network::SGJobSubmitter](/source-reference/Classes/de/d51/classsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter/)** <br/>Signs and publishes [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) messages to the SuperGenius grid channel.  |

## Detailed Description

Publishes signed Task messages to the SuperGenius grid channel via PubSub. 

**Date**: 2026-05-28 



## Source code

```cpp


#ifndef NEOSWARM_NETWORK_SG_CLIENT_SGJOBSUBMITTER_HPP
#define NEOSWARM_NETWORK_SG_CLIENT_SGJOBSUBMITTER_HPP

#include "common/error.hpp"
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

    class SGJobSubmitter
    {
        public:
        SGJobSubmitter( std::shared_ptr<grpc::Channel> channel, SGMessageAuthenticator& authenticator );
        ~SGJobSubmitter();

        outcome::result<std::string> PublishJob( const std::string& gnusSchemaJson );

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace sgns::neoswarm::network

#endif // NEOSWARM_NETWORK_SG_CLIENT_SGJOBSUBMITTER_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
