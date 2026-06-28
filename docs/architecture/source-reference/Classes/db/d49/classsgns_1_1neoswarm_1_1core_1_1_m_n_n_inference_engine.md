---
title: sgns::neoswarm::core::MNNInferenceEngine
summary: MNN-backed inference engine with composable configuration. 

---

# sgns::neoswarm::core::MNNInferenceEngine



MNN-backed inference engine with composable configuration.  [More...](#detailed-description)


`#include <mnn_inference_engine.hpp>`

Inherits from [sgns::neoswarm::core::InferenceEngine](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/)

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[MNNInferenceEngine](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-mnninferenceengine)**() |
| | **[MNNInferenceEngine](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-mnninferenceengine)**([Config](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/) cfg) |
| | **[~MNNInferenceEngine](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-~mnninferenceengine)**() override |
| virtual outcome::result< void > | **[LoadModel](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-loadmodel)**(const std::string & model_path) override<br/>Load a model from disk ([MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) .mnn file or similar).  |
| virtual outcome::result< [InferenceResponse](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/) > | **[Infer](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-infer)**(const [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) & task) override<br/>Synchronous inference — returns the full generated output.  |
| virtual outcome::result< void > | **[StreamInfer](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-streaminfer)**(const [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) & task, std::function< void(const std::string &token)> callback) override<br/>Streaming inference — calls callback for each generated token.  |
| virtual bool | **[IsLoaded](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-isloaded)**() const override |
| virtual std::string | **[BackendName](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-backendname)**() const override |
| void | **[SetTokenizer](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-settokenizer)**(std::shared_ptr< [Tokenizer](/source-reference/Classes/d8/d0c/classsgns_1_1neoswarm_1_1core_1_1_tokenizer/) > tok)<br/>Attach a tokenizer (required for "interpreter" mode).  |
| void | **[SetStubMode](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-setstubmode)**()<br/>Mark engine as loaded in stub/test mode (no real model file needed).  |
| void | **[SetSGClient](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-setsgclient)**([network::SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/) * client)<br/>Set the SGClient for Phase 2 network dispatch.  |

## Additional inherited members

**Public Functions inherited from [sgns::neoswarm::core::InferenceEngine](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/)**

|                | Name           |
| -------------- | -------------- |
| virtual | **[~InferenceEngine](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-~inferenceengine)**() =default |


## Detailed Description

```cpp
class sgns::neoswarm::core::MNNInferenceEngine;
```

MNN-backed inference engine with composable configuration. 

Inference paths (selected at runtime via [Config::m_engineMode](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-enginemode)):

"sgprocessing" — Primary path. Routes through SGProcessingManager which handles model loading, chunking, and execution. Cross-platform. Network-ready (Phase 2).

"interpreter" — Fallback. Uses MNN::Interpreter directly for standard single-file .mnn models. Requires the external [SentencePieceTokenizer](/source-reference/Classes/d6/d8a/classsgns_1_1neoswarm_1_1core_1_1_sentence_piece_tokenizer/) to be attached.

GPU backend (selected at runtime via [Config::m_backend](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/#variable-m-backend)):

"vulkan" — Vulkan (cross-platform). MoltenVK translates to Metal on Apple. "cpu" — CPU-only fallback. 

## Public Functions Documentation

### function MNNInferenceEngine

```cpp
MNNInferenceEngine()
```


### function MNNInferenceEngine

```cpp
explicit MNNInferenceEngine(
    Config cfg
)
```


### function ~MNNInferenceEngine

```cpp
~MNNInferenceEngine() override
```


### function LoadModel

```cpp
virtual outcome::result< void > LoadModel(
    const std::string & model_path
) override
```

Load a model from disk ([MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) .mnn file or similar). 

**Parameters**: 

  * **model_path** Path to the model file. 


**Return**: outcome::success or ModelLoadFailed. 

**Reimplements**: [sgns::neoswarm::core::InferenceEngine::LoadModel](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-loadmodel)


### function Infer

```cpp
virtual outcome::result< InferenceResponse > Infer(
    const Task & task
) override
```

Synchronous inference — returns the full generated output. 

**Parameters**: 

  * **task** Inference task with prompt and generation parameters. 


**Return**: [InferenceResponse](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/) or InferenceFailed. 

**Reimplements**: [sgns::neoswarm::core::InferenceEngine::Infer](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-infer)


### function StreamInfer

```cpp
virtual outcome::result< void > StreamInfer(
    const Task & task,
    std::function< void(const std::string &token)> callback
) override
```

Streaming inference — calls callback for each generated token. 

**Parameters**: 

  * **task** Inference task. 
  * **callback** Called with each token string as it is generated. 


**Return**: outcome::success or InferenceFailed. 

**Reimplements**: [sgns::neoswarm::core::InferenceEngine::StreamInfer](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-streaminfer)


### function IsLoaded

```cpp
inline virtual bool IsLoaded() const override
```


**Return**: True if a model has been loaded. 

**Reimplements**: [sgns::neoswarm::core::InferenceEngine::IsLoaded](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-isloaded)


### function BackendName

```cpp
virtual std::string BackendName() const override
```


**Return**: Human-readable backend name (e.g. "MNN/Vulkan", "MNN/CPU"). 

**Reimplements**: [sgns::neoswarm::core::InferenceEngine::BackendName](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-backendname)


### function SetTokenizer

```cpp
inline void SetTokenizer(
    std::shared_ptr< Tokenizer > tok
)
```

Attach a tokenizer (required for "interpreter" mode). 

### function SetStubMode

```cpp
inline void SetStubMode()
```

Mark engine as loaded in stub/test mode (no real model file needed). 

### function SetSGClient

```cpp
void SetSGClient(
    network::SGClient * client
)
```

Set the SGClient for Phase 2 network dispatch. 

**Parameters**: 

  * **client** The SGClient instance (owned by ApiServer). 


Call once during initialization after both the engine and the SGClient are created. The client pointer is passed through to the internal [SGProcessingBridge](/source-reference/Classes/d2/de4/classsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge/).


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700