---
title: GNUS-NEO-SWARM/src/security/message_signing.hpp
summary: secp256k1 sign/verify for inter-node messages (PTDS §4.3) 

---

# GNUS-NEO-SWARM/src/security/message_signing.hpp



secp256k1 sign/verify for inter-node messages (PTDS §4.3)  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::security](/source-reference/Namespaces/d7/d75/namespacesgns_1_1neoswarm_1_1security/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::security::MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/)** <br/>Signs and verifies inter-node message payloads.  |

## Detailed Description

secp256k1 sign/verify for inter-node messages (PTDS §4.3) 

**Date**: 2026-05-08 



## Source code

```cpp


#ifndef NEOSWARM_SECURITY_MESSAGESIGNING_HPP
#define NEOSWARM_SECURITY_MESSAGESIGNING_HPP

#include "node_identity.hpp"
#include "common/error.hpp"
#include <cstdint>
#include <string>
#include <vector>

namespace sgns::neoswarm::security
{
    class MessageSigning
    {
        public:
        explicit MessageSigning( const NodeIdentity& identity );

        outcome::result<std::vector<uint8_t>> Sign( const std::string& payload ) const;

        static bool Verify( const std::string& payload,
                            const std::vector<uint8_t>& signature,
                            const std::string& m_pubKeyhex );

        static constexpr int64_t kReplayWindowSec = 30;

        static std::string GenerateNonce();

        static uint64_t CurrentTimestampMs();

        std::string AttachSignature( const std::string& payload ) const;

        static bool VerifyAndStrip( std::string& payload, const std::string& m_pubKeyhex );

        private:
        const NodeIdentity& m_identity;
    };

} // namespace sgns::neoswarm::security

#endif // NEOSWARM_SECURITY_MESSAGESIGNING_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
