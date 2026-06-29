---
title: sgns::neoswarm::network::SGJobSubmitter
summary: Signs and publishes Task messages to the SuperGenius grid channel. 

---

# sgns::neoswarm::network::SGJobSubmitter



Signs and publishes [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) messages to the SuperGenius grid channel.  [More...](#detailed-description)


`#include <sg_job_submitter.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Impl](/source-reference/Classes/da/d6d/structsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter_1_1_impl/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[SGJobSubmitter](/source-reference/Classes/de/d51/classsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter/#function-sgjobsubmitter)**(std::shared_ptr< grpc::Channel > channel, [SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/) & authenticator) |
| | **[~SGJobSubmitter](/source-reference/Classes/de/d51/classsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter/#function-~sgjobsubmitter)**() |
| outcome::result< std::string > | **[PublishJob](/source-reference/Classes/de/d51/classsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter/#function-publishjob)**(const std::string & gnusSchemaJson)<br/>Sign and publish a GNUS schema JSON job to the grid channel.  |

## Detailed Description

```cpp
class sgns::neoswarm::network::SGJobSubmitter;
```

Signs and publishes [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) messages to the SuperGenius grid channel. 

Converts GNUS schema JSON into signed PubSub [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) messages, publishes them to the processing grid channel, and returns a taskId for result collection. 

## Public Functions Documentation

### function SGJobSubmitter

```cpp
SGJobSubmitter(
    std::shared_ptr< grpc::Channel > channel,
    SGMessageAuthenticator & authenticator
)
```


### function ~SGJobSubmitter

```cpp
~SGJobSubmitter()
```


### function PublishJob

```cpp
outcome::result< std::string > PublishJob(
    const std::string & gnusSchemaJson
)
```

Sign and publish a GNUS schema JSON job to the grid channel. 

**Parameters**: 

  * **gnusSchemaJson** The GNUS_Schema JSON from BuildSchemaJson(). 


**Return**: The generated taskId for result collection. 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700