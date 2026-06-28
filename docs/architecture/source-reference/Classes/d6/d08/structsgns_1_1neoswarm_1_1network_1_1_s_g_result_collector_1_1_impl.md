---
title: sgns::neoswarm::network::SGResultCollector::Impl

---

# sgns::neoswarm::network::SGResultCollector::Impl





## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[Impl](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/#function-impl)**(std::shared_ptr< grpc::Channel > channel, [SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/) & authenticator, [SGResultCollectorConfig](/source-reference/Classes/d1/dd3/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_config/) cfg) |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::shared_ptr< grpc::Channel > | **[m_channel](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/#variable-m-channel)**  |
| [SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/) & | **[m_authenticator](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/#variable-m-authenticator)**  |
| [SGResultCollectorConfig](/source-reference/Classes/d1/dd3/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_config/) | **[m_cfg](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/#variable-m-cfg)**  |
| std::mutex | **[m_mutex](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/#variable-m-mutex)**  |
| std::condition_variable | **[cv_](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/#variable-cv-)**  |
| bool | **[resultReady_](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/#variable-resultready-)**  |
| std::vector< uint8_t > | **[resultData_](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/#variable-resultdata-)**  |

## Public Functions Documentation

### function Impl

```cpp
inline Impl(
    std::shared_ptr< grpc::Channel > channel,
    SGMessageAuthenticator & authenticator,
    SGResultCollectorConfig cfg
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


### variable m_cfg

```cpp
SGResultCollectorConfig m_cfg;
```


### variable m_mutex

```cpp
std::mutex m_mutex;
```


### variable cv_

```cpp
std::condition_variable cv_;
```


### variable resultReady_

```cpp
bool resultReady_ = false;
```


### variable resultData_

```cpp
std::vector< uint8_t > resultData_;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700