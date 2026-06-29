---
title: sgns::neoswarm::network::SGClient::Config
summary: Configuration for SuperGenius network connectivity. 

---

# sgns::neoswarm::network::SGClient::Config



Configuration for SuperGenius network connectivity. 


`#include <super_genius_client.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[m_endpoint](/source-reference/Classes/df/dca/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_config/#variable-m_endpoint)** <br/>SuperGenius node address.  |
| std::string | **[m_tlsCaPath](/source-reference/Classes/df/dca/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_config/#variable-m_tlscapath)** <br/>TLS CA certificate bundle.  |
| std::string | **[m_tlsCertPath](/source-reference/Classes/df/dca/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_config/#variable-m_tlscertpath)** <br/>TLS client certificate.  |
| std::chrono::seconds | **[channel_m_timeout](/source-reference/Classes/df/dca/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_config/#variable-channel_m_timeout)** <br/>Channel creation timeout.  |
| std::chrono::seconds | **[result_m_timeout](/source-reference/Classes/df/dca/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_config/#variable-result_m_timeout)** <br/>Inference result timeout (5 min).  |

## Public Attributes Documentation

### variable m_endpoint

```cpp
std::string m_endpoint = "localhost:50051";
```

SuperGenius node address. 

### variable m_tlsCaPath

```cpp
std::string m_tlsCaPath;
```

TLS CA certificate bundle. 

### variable m_tlsCertPath

```cpp
std::string m_tlsCertPath;
```

TLS client certificate. 

### variable channel_m_timeout

```cpp
std::chrono::seconds channel_m_timeout { 30 };
```

Channel creation timeout. 

### variable result_m_timeout

```cpp
std::chrono::seconds result_m_timeout { 300 };
```

Inference result timeout (5 min). 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700