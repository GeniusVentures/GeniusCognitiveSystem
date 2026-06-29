---
title: sgns::neoswarm::InferenceResponse

---

# sgns::neoswarm::InferenceResponse






`#include <types.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[m_output](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_output)**  |
| std::string | **[m_taskId](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_taskid)**  |
| [ExecutionMode](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-executionmode) | **[m_modeUsed](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_modeused)**  |
| [RouteTarget](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#enum-routetarget) | **[m_routeUsed](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_routeused)**  |
| double | **[m_totalLatencyMs](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_totallatencyms)**  |
| float | **[m_perplexity](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_perplexity)**  |
| double | **[m_latencyMs](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_latencyms)**  |
| std::string | **[m_nodeId](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_nodeid)**  |
| bool | **[m_success](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_success)**  |
| std::string | **[m_errorMessage](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/#variable-m_errormessage)**  |

## Public Attributes Documentation

### variable m_output

```cpp
std::string m_output;
```


### variable m_taskId

```cpp
std::string m_taskId;
```


### variable m_modeUsed

```cpp
ExecutionMode m_modeUsed = ExecutionMode::SingleNode;
```


### variable m_routeUsed

```cpp
RouteTarget m_routeUsed = RouteTarget::CoreOnly;
```


### variable m_totalLatencyMs

```cpp
double m_totalLatencyMs = 0.0;
```


### variable m_perplexity

```cpp
float m_perplexity = 1.0f;
```


### variable m_latencyMs

```cpp
double m_latencyMs = 0.0;
```


### variable m_nodeId

```cpp
std::string m_nodeId;
```


### variable m_success

```cpp
bool m_success = true;
```


### variable m_errorMessage

```cpp
std::string m_errorMessage;
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700