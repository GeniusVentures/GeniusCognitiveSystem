---
title: sgns::neoswarm::network::SGMessageAuthenticator::Impl

---

# sgns::neoswarm::network::SGMessageAuthenticator::Impl





## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[Impl](/source-reference/Classes/d8/d21/structsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator_1_1_impl/#function-impl)**(const [security::NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) & identity) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| const [security::NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) & | **[m_identity](/source-reference/Classes/d8/d21/structsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator_1_1_impl/#variable-m-identity)**  |
| std::unique_ptr< [security::MessageSigning](/source-reference/Classes/dd/d22/classsgns_1_1neoswarm_1_1security_1_1_message_signing/) > | **[signer_](/source-reference/Classes/d8/d21/structsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator_1_1_impl/#variable-signer-)**  |

## Public Functions Documentation

### function Impl

```cpp
inline explicit Impl(
    const security::NodeIdentity & identity
)
```


## Public Attributes Documentation

### variable m_identity

```cpp
const security::NodeIdentity & m_identity;
```


### variable signer_

```cpp
std::unique_ptr< security::MessageSigning > signer_;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700