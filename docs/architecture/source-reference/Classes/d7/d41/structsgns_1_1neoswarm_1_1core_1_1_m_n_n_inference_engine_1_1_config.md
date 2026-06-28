---
title: sgns::neoswarm::core::MNNInferenceEngine::Config

---

# sgns::neoswarm::core::MNNInferenceEngine::Config






`#include <mnn_inference_engine.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[m_engineMode](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-enginemode)** <br/>Inference path: "sgprocessing" (primary) or "interpreter" (fallback).  |
| std::string | **[m_backend](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-backend)** <br/>GPU backend: "vulkan" (cross-platform) or "cpu".  |
| bool | **[m_useFp4](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-usefp4)** <br/>Use FP4 quantization for SGProcessing path.  |
| int | **[m_numThreads](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-numthreads)** <br/>CPU thread count (used when m_backend == "cpu").  |
| int | **[m_maxNewTokens](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-maxnewtokens)**  |
| float | **[m_temperature](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-temperature)**  |
| float | **[m_topP](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-topp)**  |
| int | **[m_topK](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-topk)**  |
| float | **[m_repetitionPenalty](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-repetitionpenalty)**  |
| bool | **[m_sgNetworkMode](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-sgnetworkmode)** <br/>SGProcessing network mode (Phase 2: dispatch via gRPC to SuperGenius).  |
| int | **[kDefaultMaxTokens](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-kdefaultmaxtokens)** <br/>Generation parameters.  |
| float | **[kDefaultTemperature](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-kdefaulttemperature)**  |
| float | **[kDefaultTopP](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-kdefaulttopp)**  |
| int | **[kDefaultTopK](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-kdefaulttopk)**  |
| float | **[kDefaultRepetitionPenalty](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-kdefaultrepetitionpenalty)**  |

## Public Attributes Documentation

### variable m_engineMode

```cpp
std::string m_engineMode = "sgprocessing";
```

Inference path: "sgprocessing" (primary) or "interpreter" (fallback). 

### variable m_backend

```cpp
std::string m_backend = "vulkan";
```

GPU backend: "vulkan" (cross-platform) or "cpu". 

### variable m_useFp4

```cpp
bool m_useFp4 = true;
```

Use FP4 quantization for SGProcessing path. 

### variable m_numThreads

```cpp
int m_numThreads = 4;
```

CPU thread count (used when m_backend == "cpu"). 

### variable m_maxNewTokens

```cpp
int m_maxNewTokens = kDefaultMaxTokens;
```


### variable m_temperature

```cpp
float m_temperature = kDefaultTemperature;
```


### variable m_topP

```cpp
float m_topP = kDefaultTopP;
```


### variable m_topK

```cpp
int m_topK = kDefaultTopK;
```


### variable m_repetitionPenalty

```cpp
float m_repetitionPenalty = kDefaultRepetitionPenalty;
```


### variable m_sgNetworkMode

```cpp
bool m_sgNetworkMode = false;
```

SGProcessing network mode (Phase 2: dispatch via gRPC to SuperGenius). 

### variable kDefaultMaxTokens

```cpp
static int kDefaultMaxTokens = 512;
```

Generation parameters. 

### variable kDefaultTemperature

```cpp
static float kDefaultTemperature = 0.7f;
```


### variable kDefaultTopP

```cpp
static float kDefaultTopP = 0.9f;
```


### variable kDefaultTopK

```cpp
static int kDefaultTopK = 40;
```


### variable kDefaultRepetitionPenalty

```cpp
static float kDefaultRepetitionPenalty = 1.1f;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700