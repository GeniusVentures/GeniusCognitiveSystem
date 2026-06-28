---
title: sgns::neoswarm::security::MessageSigning
summary: Signs and verifies inter-node message payloads. 

---

# sgns::neoswarm::security::MessageSigning



Signs and verifies inter-node message payloads. 


`#include <message_signing.hpp>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/#function-messagesigning)**(const [NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) & identity)<br/>Construct with a reference to the local node identity.  |
| outcome::result< std::vector< uint8_t > > | **[Sign](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/#function-sign)**(const std::string & payload) const<br/>Sign a serialised message payload.  |
| std::string | **[AttachSignature](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/#function-attachsignature)**(const std::string & payload) const<br/>Attach a signature field to a JSON payload string.  |
| bool | **[Verify](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/#function-verify)**(const std::string & payload, const std::vector< uint8_t > & signature, const std::string & m_pubKeyhex)<br/>Verify a signature against a known public key.  |
| std::string | **[GenerateNonce](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/#function-generatenonce)**()<br/>Generate a cryptographically random nonce.  |
| uint64_t | **[CurrentTimestampMs](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/#function-currenttimestampms)**()<br/>Get current Unix timestamp in milliseconds.  |
| bool | **[VerifyAndStrip](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/#function-verifyandstrip)**(std::string & payload, const std::string & m_pubKeyhex)<br/>Verify and strip the signature field from a signed JSON payload.  |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| int64_t | **[kReplayWindowSec](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/#variable-kreplaywindowsec)** <br/>Replay protection window in seconds.  |

## Public Functions Documentation

### function MessageSigning

```cpp
explicit MessageSigning(
    const NodeIdentity & identity
)
```

Construct with a reference to the local node identity. 

**Parameters**: 

  * **identity** Node identity used for signing. 


### function Sign

```cpp
outcome::result< std::vector< uint8_t > > Sign(
    const std::string & payload
) const
```

Sign a serialised message payload. 

**Parameters**: 

  * **payload** UTF-8 payload string. 


**Return**: DER-encoded signature bytes or IdentityError. 

### function AttachSignature

```cpp
std::string AttachSignature(
    const std::string & payload
) const
```

Attach a signature field to a JSON payload string. 

**Parameters**: 

  * **payload** JSON object string (must end with '}'). 


**Return**: Payload with appended "sig" field. 

### function Verify

```cpp
static bool Verify(
    const std::string & payload,
    const std::vector< uint8_t > & signature,
    const std::string & m_pubKeyhex
)
```

Verify a signature against a known public key. 

**Parameters**: 

  * **payload** Original payload string. 
  * **signature** DER-encoded signature bytes. 
  * **m_pubKeyhex** Hex-encoded compressed public key of the signer. 


**Return**: True if the signature is valid. 

### function GenerateNonce

```cpp
static std::string GenerateNonce()
```

Generate a cryptographically random nonce. 

**Return**: Hex-encoded 32-byte nonce. 

### function CurrentTimestampMs

```cpp
static uint64_t CurrentTimestampMs()
```

Get current Unix timestamp in milliseconds. 

**Return**: Milliseconds since epoch. 

### function VerifyAndStrip

```cpp
static bool VerifyAndStrip(
    std::string & payload,
    const std::string & m_pubKeyhex
)
```

Verify and strip the signature field from a signed JSON payload. 

**Parameters**: 

  * **payload** On entry: signed JSON. On exit: payload without sig. 
  * **m_pubKeyhex** Hex-encoded public key of the expected signer. 


**Return**: True if the signature is valid. 

## Public Attributes Documentation

### variable kReplayWindowSec

```cpp
static int64_t kReplayWindowSec = 30;
```

Replay protection window in seconds. 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700