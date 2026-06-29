---
title: sgns::neoswarm::network::SGJobSubmitter::Impl

---

# sgns::neoswarm::network::SGJobSubmitter::Impl





## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[Impl](/source-reference/Classes/da/d6d/structsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter_1_1_impl/#function-impl)**(std::shared_ptr< grpc::Channel > channel, [SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/) & authenticator) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::shared_ptr< grpc::Channel > | **[m_channel](/source-reference/Classes/da/d6d/structsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter_1_1_impl/#variable-m_channel)**  |
| [SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/) & | **[m_authenticator](/source-reference/Classes/da/d6d/structsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter_1_1_impl/#variable-m_authenticator)**  |
| std::string | **[gridChannel_](/source-reference/Classes/da/d6d/structsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter_1_1_impl/#variable-gridchannel_)**  |

## Public Functions Documentation

### function Impl

```cpp
inline Impl(
    std::shared_ptr< grpc::Channel > channel,
    SGMessageAuthenticator & authenticator
)
```


## Public Attributes Documentation

### variable m_channel

```cpp
std::shared_ptr< grpc::Channel > m_channel;
```


### variable m_authenticator

```cpp
SGMessageAuthenticator & m_authenticator;
```


### variable gridChannel_

```cpp
std::string gridChannel_ = "gnus.processing.grid";
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700