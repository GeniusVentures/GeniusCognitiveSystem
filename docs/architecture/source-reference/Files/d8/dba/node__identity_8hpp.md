---
title: GNUS-NEO-SWARM/src/security/node_identity.hpp
summary: secp256k1 keypair and PeerId derivation (PTDS §4.3) 

---

# GNUS-NEO-SWARM/src/security/node_identity.hpp



secp256k1 keypair and PeerId derivation (PTDS §4.3)  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::security](/source-reference/Namespaces/d7/d75/namespacesgns_1_1neoswarm_1_1security/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::security::NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/)** <br/>Manages a secp256k1 keypair and derives the node's PeerId.  |

## Detailed Description

secp256k1 keypair and PeerId derivation (PTDS §4.3) 

**Date**: 2026-05-08 



## Source code

```cpp


#ifndef NEOSWARM_SECURITY_NODEIDENTITY_HPP
#define NEOSWARM_SECURITY_NODEIDENTITY_HPP

#include "common/error.hpp"
#include <array>
#include <memory>
#include <string>
#include <vector>

namespace sgns::neoswarm::security
{
    class NodeIdentity
    {
        public:
        static constexpr size_t kPrivKeySize = 32;
        static constexpr size_t kPubKeySize = 33; 
        static constexpr size_t kPeerIdSize = 32;

        using PrivKey = std::array<uint8_t, kPrivKeySize>;
        using PubKey = std::array<uint8_t, kPubKeySize>;

        NodeIdentity();
        ~NodeIdentity();

        outcome::result<void> Generate();

        outcome::result<void> LoadFromFile( const std::string& path );

        outcome::result<void> SaveToFile( const std::string& path ) const;

        outcome::result<void> SaveEncrypted( const std::string& path, const std::string& passphrase ) const;

        outcome::result<void> LoadEncrypted( const std::string& path, const std::string& passphrase );

        std::string GetPeerId() const;

        const PubKey& GetPublicKey() const
        {
            return m_pubKey;
        }

        bool IsLoaded() const
        {
            return m_loaded;
        }

        outcome::result<std::vector<uint8_t>> Sign( const std::vector<uint8_t>& message ) const;

        bool Verify( const std::vector<uint8_t>& message, const std::vector<uint8_t>& signature ) const;

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
        PubKey m_pubKey{};
        bool m_loaded = false;
    };

} // namespace sgns::neoswarm::security

#endif // NEOSWARM_SECURITY_NODEIDENTITY_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
