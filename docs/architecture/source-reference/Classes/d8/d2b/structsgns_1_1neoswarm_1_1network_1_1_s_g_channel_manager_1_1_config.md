---
title: sgns::neoswarm::network::SGChannelManager::Config

---

# sgns::neoswarm::network::SGChannelManager::Config






`#include <sg_channel_manager.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[m_endpoint](/source-reference/Classes/d8/d2b/structsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager_1_1_config/#variable-m_endpoint)**  |
| std::string | **[m_tlsCaPath](/source-reference/Classes/d8/d2b/structsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager_1_1_config/#variable-m_tlscapath)**  |
| std::string | **[m_tlsCertPath](/source-reference/Classes/d8/d2b/structsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager_1_1_config/#variable-m_tlscertpath)**  |
| std::chrono::seconds | **[m_timeout](/source-reference/Classes/d8/d2b/structsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager_1_1_config/#variable-m_timeout)**  |

## Public Attributes Documentation

### variable m_endpoint

```cpp
std::string m_endpoint = "localhost:50051";
```


### variable m_tlsCaPath

```cpp
std::string m_tlsCaPath;
```


### variable m_tlsCertPath

```cpp
std::string m_tlsCertPath;
```


### variable m_timeout

```cpp
std::chrono::seconds m_timeout { 30 };
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700