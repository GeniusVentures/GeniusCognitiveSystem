---
title: sgns::neoswarm::network::SGResultCollector
summary: Collects inference results from SuperGenius PubSub result channels. 

---

# sgns::neoswarm::network::SGResultCollector



Collects inference results from SuperGenius PubSub result channels. 


`#include <sg_result_collector.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Impl](/source-reference/Classes/d6/d08/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_1_1_impl/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[SGResultCollector](/source-reference/Classes/de/d02/classsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector/#function-sgresultcollector)**(std::shared_ptr< grpc::Channel > channel, [SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/) & authenticator, [SGResultCollectorConfig](/source-reference/Classes/d1/dd3/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_config/) cfg ={}) |
| | **[~SGResultCollector](/source-reference/Classes/de/d02/classsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector/#function-~sgresultcollector)**() |
| outcome::result< std::vector< uint8_t > > | **[WaitForResult](/source-reference/Classes/de/d02/classsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector/#function-waitforresult)**(const std::string & taskId, std::chrono::seconds timeout)<br/>Block until a result arrives or timeout expires.  |
| outcome::result< std::vector< uint8_t > > | **[WaitForResult](/source-reference/Classes/de/d02/classsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector/#function-waitforresult)**(const std::string & taskId)<br/>Wait for result using the configured default timeout.  |

## Public Functions Documentation

### function SGResultCollector

```cpp
SGResultCollector(
    std::shared_ptr< grpc::Channel > channel,
    SGMessageAuthenticator & authenticator,
    SGResultCollectorConfig cfg ={}
)
```


### function ~SGResultCollector

```cpp
~SGResultCollector()
```


### function WaitForResult

```cpp
outcome::result< std::vector< uint8_t > > WaitForResult(
    const std::string & taskId,
    std::chrono::seconds timeout
)
```

Block until a result arrives or timeout expires. 

**Parameters**: 

  * **taskId** The task ID to collect results for. 
  * **timeout** Maximum time to wait. 


**Return**: Raw output bytes or timeout/network error. 

### function WaitForResult

```cpp
outcome::result< std::vector< uint8_t > > WaitForResult(
    const std::string & taskId
)
```

Wait for result using the configured default timeout. 

**Parameters**: 

  * **taskId** The task ID to collect results for. 


**Return**: Raw output bytes or error. 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700