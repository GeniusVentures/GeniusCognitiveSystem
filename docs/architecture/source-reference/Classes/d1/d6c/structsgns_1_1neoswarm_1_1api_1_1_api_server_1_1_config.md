---
title: sgns::neoswarm::api::ApiServer::Config

---

# sgns::neoswarm::api::ApiServer::Config






`#include <api_server.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[m_modelPath](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-modelpath)**  |
| std::string | **[m_grammarModelPath](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-grammarmodelpath)**  |
| std::string | **[m_mathModelPath](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-mathmodelpath)**  |
| std::string | **[m_reputationDbPath](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-reputationdbpath)**  |
| std::string | **[m_knowledgeFacts](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-knowledgefacts)**  |
| bool | **[m_enableNetwork](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-enablenetwork)**  |
| bool | **[m_enableKnowledge](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-enableknowledge)**  |
| int | **[m_grpcPort](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-grpcport)**  |
| std::string | **[m_nodeKeyFile](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-nodekeyfile)**  |
| std::string | **[m_nodeKeyPassphrase](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-nodekeypassphrase)**  |
| bool | **[m_enableSgProcessing](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-enablesgprocessing)**  |
| bool | **[m_sgProcessingNetworkMode](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-sgprocessingnetworkmode)**  |
| std::string | **[m_sgEndpoint](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-sgendpoint)**  |
| std::string | **[m_sgTlsCa](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-sgtlsca)**  |
| std::string | **[m_sgTlsCert](/source-reference/Classes/d1/d6c/structsgns_1_1neoswarm_1_1api_1_1_api_server_1_1_config/#variable-m-sgtlscert)**  |

## Public Attributes Documentation

### variable m_modelPath

```cpp
std::string m_modelPath;
```


### variable m_grammarModelPath

```cpp
std::string m_grammarModelPath;
```


### variable m_mathModelPath

```cpp
std::string m_mathModelPath;
```


### variable m_reputationDbPath

```cpp
std::string m_reputationDbPath = "./reputation.db";
```


### variable m_knowledgeFacts

```cpp
std::string m_knowledgeFacts = "";
```


### variable m_enableNetwork

```cpp
bool m_enableNetwork = false;
```


### variable m_enableKnowledge

```cpp
bool m_enableKnowledge = true;
```


### variable m_grpcPort

```cpp
int m_grpcPort = 50051;
```


### variable m_nodeKeyFile

```cpp
std::string m_nodeKeyFile = "./node.key";
```


### variable m_nodeKeyPassphrase

```cpp
std::string m_nodeKeyPassphrase = "gnus-neo-swarm-default";
```


### variable m_enableSgProcessing

```cpp
bool m_enableSgProcessing = false;
```


### variable m_sgProcessingNetworkMode

```cpp
bool m_sgProcessingNetworkMode = false;
```


### variable m_sgEndpoint

```cpp
std::string m_sgEndpoint = "localhost:50051";
```


### variable m_sgTlsCa

```cpp
std::string m_sgTlsCa;
```


### variable m_sgTlsCert

```cpp
std::string m_sgTlsCert;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700