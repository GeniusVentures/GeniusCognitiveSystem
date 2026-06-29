---
title: sgns::neoswarm::security::NodeIdentity
summary: Manages a secp256k1 keypair and derives the node's PeerId. 

---

# sgns::neoswarm::security::NodeIdentity



Manages a secp256k1 keypair and derives the node's PeerId.  [More...](#detailed-description)


`#include <node_identity.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Impl](/source-reference/Classes/d6/dc0/structsgns_1_1neoswarm_1_1security_1_1_node_identity_1_1_impl/)**  |

## Public Types

|                | Name           |
| -------------- | -------------- |
| using std::array< uint8_t, [kPrivKeySize](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#variable-kprivkeysize) > | **[PrivKey](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#using-privkey)**  |
| using std::array< uint8_t, [kPubKeySize](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#variable-kpubkeysize) > | **[PubKey](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#using-pubkey)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-nodeidentity)**() |
| | **[~NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-~nodeidentity)**() |
| outcome::result< void > | **[Generate](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-generate)**()<br/>Generate a new random secp256k1 keypair.  |
| outcome::result< void > | **[LoadFromFile](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-loadfromfile)**(const std::string & path)<br/>Load a keypair from a hex file.  |
| outcome::result< void > | **[SaveToFile](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-savetofile)**(const std::string & path) const<br/>Save the current keypair to a hex file.  |
| outcome::result< void > | **[SaveEncrypted](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-saveencrypted)**(const std::string & path, const std::string & passphrase) const<br/>Save the current keypair encrypted with AES-256-GCM.  |
| outcome::result< void > | **[LoadEncrypted](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-loadencrypted)**(const std::string & path, const std::string & passphrase)<br/>Load an encrypted keypair and decrypt it.  |
| std::string | **[GetPeerId](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-getpeerid)**() const<br/>Derive the PeerId string from the public key.  |
| const [PubKey](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#using-pubkey) & | **[GetPublicKey](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-getpublickey)**() const |
| bool | **[IsLoaded](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-isloaded)**() const |
| outcome::result< std::vector< uint8_t > > | **[Sign](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-sign)**(const std::vector< uint8_t > & message) const<br/>Sign a message with the node's private key.  |
| bool | **[Verify](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#function-verify)**(const std::vector< uint8_t > & message, const std::vector< uint8_t > & signature) const<br/>Verify a signature against this node's public key.  |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| size_t | **[kPrivKeySize](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#variable-kprivkeysize)**  |
| size_t | **[kPubKeySize](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#variable-kpubkeysize)** <br/>compressed  |
| size_t | **[kPeerIdSize](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/#variable-kpeeridsize)**  |

## Detailed Description

```cpp
class sgns::neoswarm::security::NodeIdentity;
```

Manages a secp256k1 keypair and derives the node's PeerId. 

PeerId = hex( SHA-256( compressed_public_key ) ) 

## Public Types Documentation

### using PrivKey

```cpp
using sgns::neoswarm::security::NodeIdentity::PrivKey = std::array<uint8_t, kPrivKeySize>;
```


### using PubKey

```cpp
using sgns::neoswarm::security::NodeIdentity::PubKey = std::array<uint8_t, kPubKeySize>;
```


## Public Functions Documentation

### function NodeIdentity

```cpp
NodeIdentity()
```


### function ~NodeIdentity

```cpp
~NodeIdentity()
```


### function Generate

```cpp
outcome::result< void > Generate()
```

Generate a new random secp256k1 keypair. 

**Return**: outcome::success or IdentityError. 

### function LoadFromFile

```cpp
outcome::result< void > LoadFromFile(
    const std::string & path
)
```

Load a keypair from a hex file. 

**Parameters**: 

  * **path** Path to the key file. 


**Return**: outcome::success or IdentityError. 

### function SaveToFile

```cpp
outcome::result< void > SaveToFile(
    const std::string & path
) const
```

Save the current keypair to a hex file. 

**Parameters**: 

  * **path** Destination file path. 


**Return**: outcome::success or IdentityError. 

### function SaveEncrypted

```cpp
outcome::result< void > SaveEncrypted(
    const std::string & path,
    const std::string & passphrase
) const
```

Save the current keypair encrypted with AES-256-GCM. 

**Parameters**: 

  * **path** Destination file path (typically "node.key"). 
  * **passphrase** User-supplied encryption passphrase. 


**Return**: outcome::success or IdentityError. 

Derives a 256-bit encryption key from `passphrase` using PBKDF2-HMAC-SHA256 (600,000 iterations) with a random salt. The key is encrypted and written in a self-describing binary format: [4-byte salt length][salt][12-byte IV][ciphertext][16-byte GCM tag].


### function LoadEncrypted

```cpp
outcome::result< void > LoadEncrypted(
    const std::string & path,
    const std::string & passphrase
)
```

Load an encrypted keypair and decrypt it. 

**Parameters**: 

  * **path** Path to the encrypted key file. 
  * **passphrase** Decryption passphrase. 


**Return**: outcome::success or IdentityError. 

Reads the binary format written by SaveEncrypted, derives the decryption key from `passphrase`, decrypts, and verifies the GCM authentication tag. If the tag does not match (wrong passphrase or tampered file), returns IdentityError.

On success, the public key is derived and PeerId is available.


### function GetPeerId

```cpp
std::string GetPeerId() const
```

Derive the PeerId string from the public key. 

**Return**: Hex-encoded SHA-256 of the compressed public key. 

### function GetPublicKey

```cpp
inline const PubKey & GetPublicKey() const
```


**Return**: The compressed public key bytes. 

### function IsLoaded

```cpp
inline bool IsLoaded() const
```


**Return**: True if a keypair has been loaded or generated. 

### function Sign

```cpp
outcome::result< std::vector< uint8_t > > Sign(
    const std::vector< uint8_t > & message
) const
```

Sign a message with the node's private key. 

**Parameters**: 

  * **message** Raw bytes to sign. 


**Return**: DER-encoded signature or IdentityError. 

### function Verify

```cpp
bool Verify(
    const std::vector< uint8_t > & message,
    const std::vector< uint8_t > & signature
) const
```

Verify a signature against this node's public key. 

**Parameters**: 

  * **message** Original message bytes. 
  * **signature** DER-encoded signature to verify. 


**Return**: True if the signature is valid. 

## Public Attributes Documentation

### variable kPrivKeySize

```cpp
static size_t kPrivKeySize = 32;
```


### variable kPubKeySize

```cpp
static size_t kPubKeySize = 33;
```

compressed 

### variable kPeerIdSize

```cpp
static size_t kPeerIdSize = 32;
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700