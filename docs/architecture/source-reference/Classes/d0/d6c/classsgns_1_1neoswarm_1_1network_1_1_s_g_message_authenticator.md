---
title: sgns::neoswarm::network::SGMessageAuthenticator
summary: Wraps NodeIdentity and MessageSigning for SuperGenius dispatch. 

---

# sgns::neoswarm::network::SGMessageAuthenticator



Wraps NodeIdentity and MessageSigning for SuperGenius dispatch.  [More...](#detailed-description)


`#include <sg_message_authenticator.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Impl](/source-reference/Classes/d8/d21/structsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator_1_1_impl/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/#function-sgmessageauthenticator)**(const [security::NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) & identity)<br/>Construct with the node's cryptographic identity.  |
| | **[~SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/#function-~sgmessageauthenticator)**() |
| outcome::result< std::string > | **[SignPayload](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/#function-signpayload)**(const std::string & payload) const<br/>Sign a JSON payload with nonce + timestamp replays protection.  |
| outcome::result< bool > | **[VerifyResult](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/#function-verifyresult)**(std::string & payload, const std::string & pubKeyHex) const<br/>Verify a signed result and strip authentication fields.  |

## Detailed Description

```cpp
class sgns::neoswarm::network::SGMessageAuthenticator;
```

Wraps NodeIdentity and MessageSigning for SuperGenius dispatch. 

Signs every outgoing [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) payload with the node's secp256k1 identity (including nonce + timestamp for replay protection) and verifies incoming result signatures before accepting them. 

## Public Functions Documentation

### function SGMessageAuthenticator

```cpp
explicit SGMessageAuthenticator(
    const security::NodeIdentity & identity
)
```

Construct with the node's cryptographic identity. 

**Parameters**: 

  * **identity** The node's secp256k1 identity (from Phase 1). 


### function ~SGMessageAuthenticator

```cpp
~SGMessageAuthenticator()
```


### function SignPayload

```cpp
outcome::result< std::string > SignPayload(
    const std::string & payload
) const
```

Sign a JSON payload with nonce + timestamp replays protection. 

**Parameters**: 

  * **payload** The raw JSON payload to sign. 


**Return**: The signed payload (JSON with attached signature fields). 

### function VerifyResult

```cpp
outcome::result< bool > VerifyResult(
    std::string & payload,
    const std::string & pubKeyHex
) const
```

Verify a signed result and strip authentication fields. 

**Parameters**: 

  * **payload** The signed payload (modified in-place). 
  * **pubKeyHex** The expected signer's public key as hex. 


**Return**: true if signature is valid and replay-check passes. 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700