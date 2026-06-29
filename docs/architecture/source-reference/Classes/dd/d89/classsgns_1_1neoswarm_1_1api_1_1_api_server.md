---
title: sgns::neoswarm::api::ApiServer
summary: Orchestrates the full inference pipeline. 

---

# sgns::neoswarm::api::ApiServer



Orchestrates the full inference pipeline.  [More...](#detailed-description)


`#include <api_server.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[ApiServer](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-apiserver)**([Config](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/) cfg) |
| | **[~ApiServer](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-~apiserver)**() |
| outcome::result< void > | **[Initialize](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-initialize)**()<br/>Initialise all subsystems.  |
| outcome::result< [InferenceResponse](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/) > | **[Process](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-process)**(const [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) & task)<br/>Process a single inference request (all modes).  |
| outcome::result< void > | **[Serve](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-serve)**()<br/>Start the gRPC server (blocks until [Stop()](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-stop) is called).  |
| void | **[Stop](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-stop)**()<br/>Stop the server and release all resources.  |
| bool | **[IsRunning](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-isrunning)**() const |
| bool | **[IsSuperGeniusConnected](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-issupergeniusconnected)**() const |

## Detailed Description

```cpp
class sgns::neoswarm::api::ApiServer;
```

Orchestrates the full inference pipeline. 

Mode 1 (SingleNode): API → Router → Core LLM → Response Mode 2 (Specialist): API → Router → Core → Specialist → Response Mode 3 (Swarm): API → Router → Broadcast → [Nodes] → Consensus → Grokipedia Validation → Response 

## Public Functions Documentation

### function ApiServer

```cpp
explicit ApiServer(
    Config cfg
)
```


### function ~ApiServer

```cpp
~ApiServer()
```


### function Initialize

```cpp
outcome::result< void > Initialize()
```

Initialise all subsystems. 

**Return**: outcome::success or the first error encountered. 

### function Process

```cpp
outcome::result< InferenceResponse > Process(
    const Task & task
)
```

Process a single inference request (all modes). 

**Parameters**: 

  * **task** Incoming task. 


**Return**: [InferenceResponse](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/) or InferenceFailed. 

### function Serve

```cpp
outcome::result< void > Serve()
```

Start the gRPC server (blocks until [Stop()](/source-reference/Classes/dd/d89/classsgns_1_1neoswarm_1_1api_1_1_api_server/#function-stop) is called). 

**Return**: outcome::success or NetworkError. 

### function Stop

```cpp
void Stop()
```

Stop the server and release all resources. 

### function IsRunning

```cpp
inline bool IsRunning() const
```


**Return**: True if the server is currently running. 

### function IsSuperGeniusConnected

```cpp
bool IsSuperGeniusConnected() const
```


**Return**: True if connected to SuperGenius network. 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700