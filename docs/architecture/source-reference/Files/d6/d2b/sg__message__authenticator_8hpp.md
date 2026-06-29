---
title: GNUS-NEO-SWARM/src/network/sg_client/sg_message_authenticator.hpp
summary: Signs and verifies messages using the node's secp256k1 identity. 

---

# GNUS-NEO-SWARM/src/network/sg_client/sg_message_authenticator.hpp



Signs and verifies messages using the node's secp256k1 identity.  [More...](#detailed-description)

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
| class | **[sgns::neoswarm::network::SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/)** <br/>Wraps NodeIdentity and MessageSigning for SuperGenius dispatch.  |

## Detailed Description

Signs and verifies messages using the node's secp256k1 identity. 

**Date**: 2026-05-28 



## Source code

```cpp


#ifndef NEOSWARM_NETWORK_SG_CLIENT_SGMESSAGEAUTHENTICATOR_HPP
#define NEOSWARM_NETWORK_SG_CLIENT_SGMESSAGEAUTHENTICATOR_HPP

#include "common/error.hpp"
#include <memory>
#include <string>
#include <vector>

namespace sgns::neoswarm::security
{
    class NodeIdentity;
}

namespace sgns::neoswarm::network
{
    class SGMessageAuthenticator
    {
        public:
        explicit SGMessageAuthenticator( const security::NodeIdentity& identity );

        ~SGMessageAuthenticator();

        outcome::result<std::string> SignPayload( const std::string& payload ) const;

        outcome::result<bool> VerifyResult( std::string& payload, const std::string& pubKeyHex ) const;

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
    };

} // namespace sgns::neoswarm::network

#endif // NEOSWARM_NETWORK_SG_CLIENT_SGMESSAGEAUTHENTICATOR_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
