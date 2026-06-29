---
title: sgns::neoswarm::network::SGClient::Impl

---

# sgns::neoswarm::network::SGClient::Impl





## Public Attributes

|                | Name           |
| -------------- | -------------- |
| [Config](/source-reference/Classes/df/dca/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_config/) | **[m_cfg](/source-reference/Classes/d9/d1e/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_impl/#variable-m_cfg)**  |
| const [security::NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) * | **[m_identity](/source-reference/Classes/d9/d1e/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_impl/#variable-m_identity)**  |
| std::unique_ptr< [SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/) > | **[m_authenticator](/source-reference/Classes/d9/d1e/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_impl/#variable-m_authenticator)**  |
| std::unique_ptr< [SGChannelManager](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/) > | **[channelMgr_](/source-reference/Classes/d9/d1e/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_impl/#variable-channelmgr_)**  |
| std::unique_ptr< [SGJobSubmitter](/source-reference/Classes/de/d51/classsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter/) > | **[jobSubmitter_](/source-reference/Classes/d9/d1e/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_impl/#variable-jobsubmitter_)**  |
| std::unique_ptr< [SGResultCollector](/source-reference/Classes/de/d02/classsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector/) > | **[resultCollector_](/source-reference/Classes/d9/d1e/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_impl/#variable-resultcollector_)**  |
| bool | **[m_connected](/source-reference/Classes/d9/d1e/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_impl/#variable-m_connected)**  |

## Public Attributes Documentation

### variable m_cfg

```cpp
Config m_cfg;
```


### variable m_identity

```cpp
const security::NodeIdentity * m_identity = nullptr;
```


### variable m_authenticator

```cpp
std::unique_ptr< SGMessageAuthenticator > m_authenticator;
```


### variable channelMgr_

```cpp
std::unique_ptr< SGChannelManager > channelMgr_;
```


### variable jobSubmitter_

```cpp
std::unique_ptr< SGJobSubmitter > jobSubmitter_;
```


### variable resultCollector_

```cpp
std::unique_ptr< SGResultCollector > resultCollector_;
```


### variable m_connected

```cpp
bool m_connected = false;
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700