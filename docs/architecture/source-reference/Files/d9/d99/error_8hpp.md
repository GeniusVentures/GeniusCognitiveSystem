---
title: GNUS-NEO-SWARM/src/common/error.hpp
summary: Error codes and outcome::result alias for GNUS NEO SWARM. 

---

# GNUS-NEO-SWARM/src/common/error.hpp



Error codes and outcome::result alias for GNUS NEO SWARM. 

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |

## Types

|                | Name           |
| -------------- | -------------- |
| enum class uint8_t | **[Error](/source-reference/Files/d9/d99/error_8hpp/#enum-error)** { ModelLoadFailed = 1, InferenceFailed = 2, TokenizerFailed = 3, FP4DecodeFailed = 4, RoutingFailed = 5, NetworkError = 6, PeerNotFound = 7, BroadcastTimeout = 8, StorageError = 9, ReputationNotFound = 10, KnowledgeUnavailable = 11, FactValidationFailed = 12, IdentityError = 13, SignatureInvalid = 14, InvalidArgument = 15, NotImplemented = 16, InternalError = 17} |

## Types Documentation

### enum Error

| Enumerator | Value | Description |
| ---------- | ----- | ----------- |
| ModelLoadFailed | 1|   |
| InferenceFailed | 2|   |
| TokenizerFailed | 3|   |
| FP4DecodeFailed | 4|   |
| RoutingFailed | 5|   |
| NetworkError | 6|   |
| PeerNotFound | 7|   |
| BroadcastTimeout | 8|   |
| StorageError | 9|   |
| ReputationNotFound | 10|   |
| KnowledgeUnavailable | 11|   |
| FactValidationFailed | 12|   |
| IdentityError | 13|   |
| SignatureInvalid | 14|   |
| InvalidArgument | 15|   |
| NotImplemented | 16|   |
| InternalError | 17|   |







## Source code

```cpp


#ifndef NEOSWARM_COMMON_ERROR_HPP
#define NEOSWARM_COMMON_ERROR_HPP

#include <libp2p/outcome/outcome.hpp>

namespace sgns::neoswarm
{
    namespace outcome = libp2p::outcome;

    // -----------------------------------------------------------------------
    // Error codes
    // -----------------------------------------------------------------------
    enum class Error : uint8_t
    {
        // Core engine
        ModelLoadFailed = 1,
        InferenceFailed = 2,
        TokenizerFailed = 3,
        FP4DecodeFailed = 4,
        // Router
        RoutingFailed = 5,
        // Network
        NetworkError = 6,
        PeerNotFound = 7,
        BroadcastTimeout = 8,
        // Reputation
        StorageError = 9,
        ReputationNotFound = 10,
        // Knowledge
        KnowledgeUnavailable = 11,
        FactValidationFailed = 12,
        // Security
        IdentityError = 13,
        SignatureInvalid = 14,
        // General
        InvalidArgument = 15,
        NotImplemented = 16,
        InternalError = 17,
    };

} // namespace sgns::neoswarm

// Register the error enum with Boost.Outcome so it can be used in outcome::result<>
OUTCOME_HPP_DECLARE_ERROR_2( sgns::neoswarm, Error )

#endif // NEOSWARM_COMMON_ERROR_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
