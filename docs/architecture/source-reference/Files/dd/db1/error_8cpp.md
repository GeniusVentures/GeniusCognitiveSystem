---
title: GNUS-NEO-SWARM/src/common/error.cpp
summary: Boost.Outcome error category registration for GNUS NEO SWARM. 

---

# GNUS-NEO-SWARM/src/common/error.cpp



Boost.Outcome error category registration for GNUS NEO SWARM. 

## Functions

|                | Name           |
| -------------- | -------------- |
| | **[OUTCOME_CPP_DEFINE_CATEGORY_3](/source-reference/Files/dd/db1/error_8cpp/#function-outcome_cpp_define_category_3)**([sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/) , Error , e ) |


## Functions Documentation

### function OUTCOME_CPP_DEFINE_CATEGORY_3

```cpp
OUTCOME_CPP_DEFINE_CATEGORY_3(
    sgns::neoswarm ,
    Error ,
    e 
)
```




## Source code

```cpp


#include "error.hpp"

OUTCOME_CPP_DEFINE_CATEGORY_3( sgns::neoswarm, Error, e )
{
    using E = sgns::neoswarm::Error;
    switch ( e )
    {
        case E::ModelLoadFailed:
            return "Model load failed";
        case E::InferenceFailed:
            return "Inference failed";
        case E::TokenizerFailed:
            return "Tokenizer failed";
        case E::FP4DecodeFailed:
            return "FP4 decode failed";
        case E::RoutingFailed:
            return "Routing failed";
        case E::NetworkError:
            return "Network error";
        case E::PeerNotFound:
            return "Peer not found";
        case E::BroadcastTimeout:
            return "Broadcast timeout";
        case E::StorageError:
            return "Storage error";
        case E::ReputationNotFound:
            return "Reputation not found";
        case E::KnowledgeUnavailable:
            return "Knowledge unavailable";
        case E::FactValidationFailed:
            return "Fact validation failed";
        case E::IdentityError:
            return "Identity error";
        case E::SignatureInvalid:
            return "Signature invalid";
        case E::InvalidArgument:
            return "Invalid argument";
        case E::NotImplemented:
            return "Not implemented";
        case E::InternalError:
            return "Internal error";
    }
    return "Unknown error";
}
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
