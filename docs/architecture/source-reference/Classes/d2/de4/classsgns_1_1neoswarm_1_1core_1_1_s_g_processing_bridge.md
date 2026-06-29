---
title: sgns::neoswarm::core::SGProcessingBridge
summary: Constructs GNUS_Schema-compliant JSON and submits inference jobs to SGProcessingManager (Phase 1 direct) or the GNUS network (Phase 2). 

---

# sgns::neoswarm::core::SGProcessingBridge



Constructs GNUS_Schema-compliant JSON and submits inference jobs to SGProcessingManager (Phase 1 direct) or the GNUS network (Phase 2). 


`#include <sg_processing_bridge.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/dd/d29/structsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge_1_1_config/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[SGProcessingBridge](/source-reference/Classes/d2/de4/classsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge/#function-sgprocessingbridge)**() |
| | **[SGProcessingBridge](/source-reference/Classes/d2/de4/classsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge/#function-sgprocessingbridge)**([Config](/source-reference/Classes/dd/d29/structsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge_1_1_config/) cfg) |
| | **[~SGProcessingBridge](/source-reference/Classes/d2/de4/classsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge/#function-~sgprocessingbridge)**() =default |
| void | **[SetClient](/source-reference/Classes/d2/de4/classsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge/#function-setclient)**([network::SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/) * client)<br/>Set the SGClient for Phase 2 network dispatch.  |
| outcome::result< std::string > | **[BuildSchemaJson](/source-reference/Classes/d2/de4/classsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge/#function-buildschemajson)**(const std::string & model_uri, const std::string & input_uri, sgns::InputFormat input_format, const std::vector< int64_t > & shape) const<br/>Build a GNUS_Schema JSON string from the supplied parameters.  |
| outcome::result< std::vector< uint8_t > > | **[SubmitJob](/source-reference/Classes/d2/de4/classsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge/#function-submitjob)**(const std::string & model_uri, const std::string & input_uri, sgns::InputFormat input_format, const std::vector< int64_t > & shape, std::shared_ptr< boost::asio::io_context > ioc)<br/>Submit a job and return raw tensor output bytes.  |

## Public Functions Documentation

### function SGProcessingBridge

```cpp
SGProcessingBridge()
```


### function SGProcessingBridge

```cpp
explicit SGProcessingBridge(
    Config cfg
)
```


### function ~SGProcessingBridge

```cpp
~SGProcessingBridge() =default
```


### function SetClient

```cpp
void SetClient(
    network::SGClient * client
)
```

Set the SGClient for Phase 2 network dispatch. 

**Parameters**: 

  * **client** The SGClient instance (owned by ApiServer). 


### function BuildSchemaJson

```cpp
outcome::result< std::string > BuildSchemaJson(
    const std::string & model_uri,
    const std::string & input_uri,
    sgns::InputFormat input_format,
    const std::vector< int64_t > & shape
) const
```

Build a GNUS_Schema JSON string from the supplied parameters. 

**Parameters**: 

  * **model_uri** IPFS URI or path to the [MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) model. 
  * **input_uri** IPFS URI or path to the input data. 
  * **input_format** Tensor element format. 
  * **shape** Tensor shape dimensions. 


**Return**: JSON string or InvalidArgument. 

### function SubmitJob

```cpp
outcome::result< std::vector< uint8_t > > SubmitJob(
    const std::string & model_uri,
    const std::string & input_uri,
    sgns::InputFormat input_format,
    const std::vector< int64_t > & shape,
    std::shared_ptr< boost::asio::io_context > ioc
)
```

Submit a job and return raw tensor output bytes. 

**Parameters**: 

  * **model_uri** IPFS URI or path to the [MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) model. 
  * **input_uri** IPFS URI or path to the input data. 
  * **input_format** Tensor element format. 
  * **shape** Tensor shape dimensions. 
  * **ioc** Boost ASIO io_context for async operations. 


**Return**: Raw output bytes or InferenceFailed / NotImplemented. 

Phase 1 (m_networkMode=false): calls ProcessingManager::Create + Process. Phase 2 (m_networkMode=true): dispatches via gRPCForSuperGenius (stub).


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700