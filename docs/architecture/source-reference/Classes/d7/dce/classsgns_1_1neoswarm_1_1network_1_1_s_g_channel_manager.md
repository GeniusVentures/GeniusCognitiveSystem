---
title: sgns::neoswarm::network::SGChannelManager
summary: Manages a persistent gRPC channel to a SuperGenius node. 

---

# sgns::neoswarm::network::SGChannelManager



Manages a persistent gRPC channel to a SuperGenius node.  [More...](#detailed-description)


`#include <sg_channel_manager.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/d8/d2b/structsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager_1_1_config/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[SGChannelManager](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/#function-sgchannelmanager)**([Config](/source-reference/Classes/d8/d2b/structsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager_1_1_config/) cfg) |
| | **[~SGChannelManager](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/#function-~sgchannelmanager)**() =default |
| outcome::result< void > | **[CreateChannel](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/#function-createchannel)**() |
| outcome::result< bool > | **[HealthCheck](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/#function-healthcheck)**() const |
| outcome::result< void > | **[Reconnect](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/#function-reconnect)**() |
| std::shared_ptr< grpc::Channel > | **[GetChannel](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/#function-getchannel)**() const |
| bool | **[IsConnected](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/#function-isconnected)**() const |

## Detailed Description

```cpp
class sgns::neoswarm::network::SGChannelManager;
```

Manages a persistent gRPC channel to a SuperGenius node. 

Handles channel creation with optional TLS, keepalive configuration, health checking, and exponential backoff reconnection. 

## Public Functions Documentation

### function SGChannelManager

```cpp
explicit SGChannelManager(
    Config cfg
)
```


### function ~SGChannelManager

```cpp
~SGChannelManager() =default
```


### function CreateChannel

```cpp
outcome::result< void > CreateChannel()
```


### function HealthCheck

```cpp
outcome::result< bool > HealthCheck() const
```


### function Reconnect

```cpp
outcome::result< void > Reconnect()
```


### function GetChannel

```cpp
std::shared_ptr< grpc::Channel > GetChannel() const
```


### function IsConnected

```cpp
bool IsConnected() const
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700