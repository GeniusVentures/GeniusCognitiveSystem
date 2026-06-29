---
title: GNUS-NEO-SWARM/src/core/engine/mnn_inference_engine.hpp

---

# GNUS-NEO-SWARM/src/core/engine/mnn_inference_engine.hpp





## Namespaces

| Name           |
| -------------- |
| **[MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/)**  |
| **[MNN::Transformer](/source-reference/Namespaces/d6/d2b/namespace_m_n_n_1_1_transformer/)**  |
| **[boost](/source-reference/Namespaces/d4/da9/namespaceboost/)**  |
| **[boost::asio](/source-reference/Namespaces/d2/d1e/namespaceboost_1_1asio/)**  |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |
| **[sgns::neoswarm::core](/source-reference/Namespaces/d2/db7/namespacesgns_1_1neoswarm_1_1core/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::core::MNNInferenceEngine](/source-reference/Classes/db/d49/classsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine/)** <br/>MNN-backed inference engine with composable configuration.  |
| struct | **[sgns::neoswarm::core::MNNInferenceEngine::Config](/source-reference/Classes/d7/d41/structsgns_1_1neoswarm_1_1core_1_1_m_n_n_inference_engine_1_1_config/)**  |




## Source code

```cpp


#ifndef NEOSWARM_CORE_ENGINE_MNNINFERENCEENGINE_HPP
#define NEOSWARM_CORE_ENGINE_MNNINFERENCEENGINE_HPP

#include "inference_engine.hpp"
#include "core/fp4/fp4_codec.hpp"
#include "core/sgprocessing/sg_processing_bridge.hpp"
#include "core/sgprocessing/tensor_interpreter.hpp"
#include "core/tokenizer/tokenizer.hpp"
#include <atomic>
#include <memory>
#include <string>

namespace MNN
{
    class Interpreter;
    class Session;
    namespace Transformer
    {
        class Llm;
    } // namespace Transformer
} // namespace MNN

namespace boost::asio
{
    class io_context;
} // namespace boost::asio

namespace sgns
{
    enum class InputFormat : int;
} // namespace sgns

namespace sgns::neoswarm::network
{
    class SGClient;
}

namespace sgns::neoswarm::core
{
    class MNNInferenceEngine : public InferenceEngine
    {
        public:
        struct Config
        {
            std::string m_engineMode = "sgprocessing";

            std::string m_backend = "vulkan";

            bool m_useFp4 = true;

            int m_numThreads = 4;

            static constexpr int   kDefaultMaxTokens         = 512;
            int   m_maxNewTokens     = kDefaultMaxTokens;
            static constexpr float kDefaultTemperature       = 0.7f;
            float m_temperature        = kDefaultTemperature;
            static constexpr float kDefaultTopP              = 0.9f;
            float m_topP              = kDefaultTopP;
            static constexpr int   kDefaultTopK              = 40;
            int   m_topK              = kDefaultTopK;
            static constexpr float kDefaultRepetitionPenalty = 1.1f;
            float m_repetitionPenalty = kDefaultRepetitionPenalty;

            bool m_sgNetworkMode = false;
        };

        MNNInferenceEngine();
        explicit MNNInferenceEngine( Config cfg );
        ~MNNInferenceEngine() override;

        outcome::result<void> LoadModel( const std::string& model_path ) override;
        outcome::result<InferenceResponse> Infer( const Task& task ) override;
        outcome::result<void> StreamInfer( const Task& task,
                                           std::function<void( const std::string& token )> callback ) override;

        bool IsLoaded() const override
        {
            return m_loaded.load();
        }
        std::string BackendName() const override;

        void SetTokenizer( std::shared_ptr<Tokenizer> tok )
        {
            m_tokenizer = std::move( tok );
        }

        void SetStubMode()
        {
            m_loaded.store( true );
        }

        void SetSGClient( network::SGClient* client ) noexcept;

        private:
        Config m_cfg;

        // --- MNN Interpreter path ---
        std::shared_ptr<MNN::Interpreter> m_interpreter;
        MNN::Session* m_session = nullptr;

        // --- MNN LLM path (native autoregressive) ---
        MNN::Transformer::Llm* mnn_llm_ = nullptr;

        // --- SGProcessing path ---
        std::unique_ptr<SGProcessingBridge> m_bridge;
        std::unique_ptr<TensorInterpreter> m_tensorInterpreter;
        std::shared_ptr<boost::asio::io_context> m_ioc;

        std::atomic<bool> m_loaded = false;
        std::string m_modelPath;
        std::shared_ptr<Tokenizer> m_tokenizer;
        fp4::FP4Codec m_fp4Codec;

        // Inference-path helpers (extracted from Infer for size/complexity)
        outcome::result<InferenceResponse> InferViaSGProcessing( const Task& task );
        outcome::result<InferenceResponse> InferViaMnnLlm( const Task& task );
        outcome::result<InferenceResponse> InferViaStandardInterpreter( const Task& task );

        // Interpreter-path helpers
        int SelectBackend() const;
        outcome::result<std::vector<float>> RunForward( const std::vector<int>& input_ids );
        int SampleToken( const std::vector<float>& logits, float temperature, float top_p, int top_k ) const;
        void ApplyRepetitionPenalty( std::vector<float>& logits,
                                     const std::vector<int>& generated,
                                     float penalty ) const;
    };

} // namespace sgns::neoswarm::core

#endif // NEOSWARM_CORE_ENGINE_MNNINFERENCEENGINE_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
