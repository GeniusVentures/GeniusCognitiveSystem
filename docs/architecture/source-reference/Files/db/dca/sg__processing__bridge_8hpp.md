---
title: GNUS-NEO-SWARM/src/core/sgprocessing/sg_processing_bridge.hpp
summary: Bridge to SuperGenius SGProcessingManager for GNUS network dispatch. 

---

# GNUS-NEO-SWARM/src/core/sgprocessing/sg_processing_bridge.hpp



Bridge to SuperGenius SGProcessingManager for GNUS network dispatch.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[boost](/source-reference/Namespaces/d4/da9/namespaceboost/)**  |
| **[boost::asio](/source-reference/Namespaces/d2/d1e/namespaceboost_1_1asio/)**  |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |
| **[sgns::neoswarm::core](/source-reference/Namespaces/d2/db7/namespacesgns_1_1neoswarm_1_1core/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::core::SGProcessingBridge](/source-reference/Classes/d2/de4/classsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge/)** <br/>Constructs GNUS_Schema-compliant JSON and submits inference jobs to SGProcessingManager (Phase 1 direct) or the GNUS network (Phase 2).  |
| struct | **[sgns::neoswarm::core::SGProcessingBridge::Config](/source-reference/Classes/dd/d29/structsgns_1_1neoswarm_1_1core_1_1_s_g_processing_bridge_1_1_config/)**  |

## Detailed Description

Bridge to SuperGenius SGProcessingManager for GNUS network dispatch. 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_CORE_SGPROCESSING_SGPROCESSINGBRIDGE_HPP
#define NEOSWARM_CORE_SGPROCESSING_SGPROCESSINGBRIDGE_HPP

#include "common/error.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

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
    class SGProcessingBridge
    {
        public:
        struct Config
        {
            bool m_networkMode = false; 
        };

        SGProcessingBridge();
        explicit SGProcessingBridge( Config cfg );
        ~SGProcessingBridge() = default;

        void SetClient( network::SGClient* client ) noexcept;

        outcome::result<std::string> BuildSchemaJson( const std::string& model_uri,
                                                      const std::string& input_uri,
                                                      sgns::InputFormat input_format,
                                                      const std::vector<int64_t>& shape ) const;

        outcome::result<std::vector<uint8_t>> SubmitJob( const std::string& model_uri,
                                                         const std::string& input_uri,
                                                         sgns::InputFormat input_format,
                                                         const std::vector<int64_t>& shape,
                                                         std::shared_ptr<boost::asio::io_context> ioc );

        private:
        Config m_cfg;
        network::SGClient* m_client = nullptr;

        outcome::result<std::vector<uint8_t>> SubmitDirect( const std::string& jsondata,
                                                            std::shared_ptr<boost::asio::io_context> ioc ) const;

        outcome::result<std::vector<uint8_t>> SubmitNetwork( const std::string& jsondata ) const;
    };

} // namespace sgns::neoswarm::core

#endif // NEOSWARM_CORE_SGPROCESSING_SGPROCESSINGBRIDGE_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
