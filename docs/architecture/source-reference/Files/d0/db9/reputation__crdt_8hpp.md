---
title: GNUS-NEO-SWARM/src/reputation/reputation_crdt.hpp
summary: Last-Write-Wins CRDT for reputation synchronisation (PTDS §4.2). 

---

# GNUS-NEO-SWARM/src/reputation/reputation_crdt.hpp



Last-Write-Wins CRDT for reputation synchronisation (PTDS §4.2).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::reputation](/source-reference/Namespaces/d7/d2c/namespacesgns_1_1neoswarm_1_1reputation/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::reputation::ReputationCRDT](/source-reference/Classes/d2/d29/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_c_r_d_t/)** <br/>Last-Write-Wins Register per node (PTDS §4.2).  |

## Detailed Description

Last-Write-Wins CRDT for reputation synchronisation (PTDS §4.2). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_REPUTATION_REPUTATIONCRDT_HPP
#define NEOSWARM_REPUTATION_REPUTATIONCRDT_HPP

#include "node_reputation.hpp"
#include <mutex>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sgns::neoswarm::reputation
{
    class ReputationCRDT
    {
        public:
        void Merge( const NodeReputation& remote );

        std::optional<NodeReputation> Get( const std::string& identity_key ) const;

        std::vector<NodeReputation> GetAll() const;

        std::string Serialize() const;

        void DeserializeAndMerge( const std::string& data );

        private:
        mutable std::mutex m_mutex;
        std::unordered_map<std::string, NodeReputation> state_; 
    };

} // namespace sgns::neoswarm::reputation

#endif // NEOSWARM_REPUTATION_REPUTATIONCRDT_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
