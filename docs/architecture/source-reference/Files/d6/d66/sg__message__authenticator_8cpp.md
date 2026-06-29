---
title: GNUS-NEO-SWARM/src/network/sg_client/sg_message_authenticator.cpp
summary: Signs and verifies messages via hardened NodeIdentity + MessageSigning. 

---

# GNUS-NEO-SWARM/src/network/sg_client/sg_message_authenticator.cpp



Signs and verifies messages via hardened NodeIdentity + MessageSigning.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[sgns::neoswarm::network::SGMessageAuthenticator::Impl](/source-reference/Classes/d8/d21/structsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator_1_1_impl/)**  |

## Detailed Description

Signs and verifies messages via hardened NodeIdentity + MessageSigning. 

**Date**: 2026-05-28 



## Source code

```cpp


#include "sg_message_authenticator.hpp"
#include "common/logging.hpp"
#include "security/message_signing.hpp"
#include "security/node_identity.hpp"

namespace sgns::neoswarm::network
{
    namespace
    {
        auto AuthLogger()
        {
            return CreateLogger( "NeoSwarm/SGAuth" );
        }
    } // namespace

    struct SGMessageAuthenticator::Impl
    {
        const security::NodeIdentity& m_identity;
        std::unique_ptr<security::MessageSigning> signer_;

        explicit Impl( const security::NodeIdentity& identity )
            : m_identity( identity )
            , signer_( std::make_unique<security::MessageSigning>( identity ) )
        {
        }
    };

    SGMessageAuthenticator::SGMessageAuthenticator( const security::NodeIdentity& identity )
        : m_impl( std::make_unique<Impl>( identity ) )
    {
        AuthLogger()->debug( "SGMessageAuthenticator created" );
    }

    outcome::result<std::string> SGMessageAuthenticator::SignPayload( const std::string& payload ) const
    {
        if ( !m_impl->m_identity.IsLoaded() )
        {
            AuthLogger()->error( "Cannot sign — NodeIdentity not loaded" );
            return outcome::failure( Error::IdentityError );
        }

        std::string signedPayload = m_impl->signer_->AttachSignature( payload );

        AuthLogger()->debug( "Payload signed ({} bytes → {} bytes)", payload.size(), signedPayload.size() );
        return signedPayload;
    }

    outcome::result<bool> SGMessageAuthenticator::VerifyResult( std::string& payload,
                                                                const std::string& pubKeyHex ) const
    {
        bool valid = security::MessageSigning::VerifyAndStrip( payload, pubKeyHex );

        if ( !valid )
        {
            AuthLogger()->warn( "Result verification FAILED for key {}", pubKeyHex.substr( 0, 16 ) );
            return false;
        }

        AuthLogger()->debug( "Result verified successfully" );
        return true;
    }

    SGMessageAuthenticator::~SGMessageAuthenticator() = default;

} // namespace sgns::neoswarm::network
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
