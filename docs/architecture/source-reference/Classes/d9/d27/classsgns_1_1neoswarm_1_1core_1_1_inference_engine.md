---
title: sgns::neoswarm::core::InferenceEngine
summary: Abstract interface for all inference backends. 

---

# sgns::neoswarm::core::InferenceEngine



Abstract interface for all inference backends. 


`#include <inference_engine.hpp>`

Inherited by [sgns::neoswarm::core::MNNInferenceEngine](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/)

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual | **[~InferenceEngine](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-~inferenceengine)**() =default |
| virtual outcome::result< void > | **[LoadModel](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-loadmodel)**(const std::string & model_path) =0<br/>Load a model from disk ([MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) .mnn file or similar).  |
| virtual outcome::result< [InferenceResponse](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/) > | **[Infer](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-infer)**(const [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) & task) =0<br/>Synchronous inference — returns the full generated output.  |
| virtual outcome::result< void > | **[StreamInfer](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-streaminfer)**(const [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) & task, std::function< void(const std::string &token)> callback) =0<br/>Streaming inference — calls callback for each generated token.  |
| virtual bool | **[IsLoaded](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-isloaded)**() const =0 |
| virtual std::string | **[BackendName](/source-reference/Classes/d9/d27/classsgns_1_1neoswarm_1_1core_1_1_inference_engine/#function-backendname)**() const =0 |

## Public Functions Documentation

### function ~InferenceEngine

```cpp
virtual ~InferenceEngine() =default
```


### function LoadModel

```cpp
virtual outcome::result< void > LoadModel(
    const std::string & model_path
) =0
```

Load a model from disk ([MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) .mnn file or similar). 

**Parameters**: 

  * **model_path** Path to the model file. 


**Return**: outcome::success or ModelLoadFailed. 

**Reimplemented by**: [sgns::neoswarm::core::MNNInferenceEngine::LoadModel](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-loadmodel)


### function Infer

```cpp
virtual outcome::result< InferenceResponse > Infer(
    const Task & task
) =0
```

Synchronous inference — returns the full generated output. 

**Parameters**: 

  * **task** Inference task with prompt and generation parameters. 


**Return**: [InferenceResponse](/source-reference/Classes/d4/d14/structsgns_1_1neoswarm_1_1_inference_response/) or InferenceFailed. 

**Reimplemented by**: [sgns::neoswarm::core::MNNInferenceEngine::Infer](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-infer)


### function StreamInfer

```cpp
virtual outcome::result< void > StreamInfer(
    const Task & task,
    std::function< void(const std::string &token)> callback
) =0
```

Streaming inference — calls callback for each generated token. 

**Parameters**: 

  * **task** Inference task. 
  * **callback** Called with each token string as it is generated. 


**Return**: outcome::success or InferenceFailed. 

**Reimplemented by**: [sgns::neoswarm::core::MNNInferenceEngine::StreamInfer](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-streaminfer)


### function IsLoaded

```cpp
virtual bool IsLoaded() const =0
```


**Return**: True if a model has been loaded. 

**Reimplemented by**: [sgns::neoswarm::core::MNNInferenceEngine::IsLoaded](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-isloaded)


### function BackendName

```cpp
virtual std::string BackendName() const =0
```


**Return**: Human-readable backend name (e.g. "MNN/Vulkan", "MNN/CPU"). 

**Reimplemented by**: [sgns::neoswarm::core::MNNInferenceEngine::BackendName](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/#function-backendname)


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700