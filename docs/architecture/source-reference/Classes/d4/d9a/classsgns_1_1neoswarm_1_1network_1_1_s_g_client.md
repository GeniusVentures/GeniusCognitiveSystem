---
title: sgns::neoswarm::network::SGClient
summary: Client that bridges GNUS NEO SWARM to the SuperGenius blockchain compute network via PubSub-based gRPC dispatch. 

---

# sgns::neoswarm::network::SGClient



Client that bridges GNUS NEO SWARM to the SuperGenius blockchain compute network via PubSub-based gRPC dispatch.  [More...](#detailed-description)


`#include <super_genius_client.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/df/dca/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_config/)** <br/>Configuration for SuperGenius network connectivity.  |
| struct | **[Impl](/source-reference/Classes/d9/d1e/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_impl/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-sgclient)**([Config](/source-reference/Classes/df/dca/structsgns_1_1neoswarm_1_1network_1_1_s_g_client_1_1_config/) cfg)<br/>Construct with configuration.  |
| | **[~SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-~sgclient)**() |
| | **[SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-sgclient)**(const SGClient & ) =delete |
| [SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-sgclient) & | **[operator=](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-operator=)**(const [SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-sgclient) & ) =delete |
| | **[SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-sgclient)**(SGClient && ) |
| [SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-sgclient) & | **[operator=](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-operator=)**([SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-sgclient) && ) |
| outcome::result< void > | **[Initialize](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-initialize)**(const [security::NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) & identity)<br/>Initialize with the node's cryptographic identity.  |
| outcome::result< void > | **[Connect](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-connect)**()<br/>Establish connection to the SuperGenius node.  |
| outcome::result< std::vector< uint8_t > > | **[SubmitJob](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-submitjob)**(const std::string & gnusSchemaJson)<br/>Submit a GNUS schema JSON job and wait for the result.  |
| void | **[Disconnect](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-disconnect)**()<br/>Disconnect from the SuperGenius node.  |
| bool | **[IsConnected](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-isconnected)**() const<br/>Check whether the client is currently connected.  |

## Detailed Description

```cpp
class sgns::neoswarm::network::SGClient;
```

Client that bridges GNUS NEO SWARM to the SuperGenius blockchain compute network via PubSub-based gRPC dispatch. 

Methodology:

* Open a persistent gRPC channel with keepalive
* Sign every [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) with the node's secp256k1 identity
* Publish to the grid channel, subscribe to per-job result channels
* Timeout-bounded result collection via condition_variable

Designed as a separate component under src/network/sg_client/ with four internal sub-components: channel manager, job submitter, result collector, and message authenticator. 

## Public Functions Documentation

### function SGClient

```cpp
explicit SGClient(
    Config cfg
)
```

Construct with configuration. 

**Parameters**: 

  * **cfg** Network and timeout settings. 


### function ~SGClient

```cpp
~SGClient()
```


### function SGClient

```cpp
SGClient(
    const SGClient & 
) =delete
```


### function operator=

```cpp
SGClient & operator=(
    const SGClient & 
) =delete
```


### function SGClient

```cpp
SGClient(
    SGClient && 
)
```


### function operator=

```cpp
SGClient & operator=(
    SGClient && 
)
```


### function Initialize

```cpp
outcome::result< void > Initialize(
    const security::NodeIdentity & identity
)
```

Initialize with the node's cryptographic identity. 

**Parameters**: 

  * **identity** The node's secp256k1 identity. 


**Return**: outcome::success or IdentityError. 

Must be called before [Connect()](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-connect). The NodeIdentity is used for signing all [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) messages dispatched to SuperGenius.


### function Connect

```cpp
outcome::result< void > Connect()
```

Establish connection to the SuperGenius node. 

**Return**: outcome::success or NetworkError. 

Creates a persistent gRPC channel with TLS, keepalive, and health checking. For localhost endpoints without TLS certs, an insecure channel is used with a WARN log.


### function SubmitJob

```cpp
outcome::result< std::vector< uint8_t > > SubmitJob(
    const std::string & gnusSchemaJson
)
```

Submit a GNUS schema JSON job and wait for the result. 

**Parameters**: 

  * **gnusSchemaJson** The GNUS_Schema JSON from BuildSchemaJson(). 


**Return**: Raw output bytes or error. 

Signs the payload, publishes to the grid channel, subscribes to the per-job result channel, and blocks until the result arrives or the timeout expires.

Blocking synchronous call — uses condition_variable internally for timeout-bounded collection.


### function Disconnect

```cpp
void Disconnect()
```

Disconnect from the SuperGenius node. 

Closes the gRPC channel and resets internal state. Safe to call [Connect()](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-connect) again after [Disconnect()](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/#function-disconnect). 


### function IsConnected

```cpp
bool IsConnected() const
```

Check whether the client is currently connected. 

**Return**: true if the gRPC channel is alive. 

-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700