---
title: GNUS-NEO-SWARM/src/reputation/reputation_storage.hpp
summary: RocksDB-backed reputation persistence (PTDS §4.2). 

---

# GNUS-NEO-SWARM/src/reputation/reputation_storage.hpp



RocksDB-backed reputation persistence (PTDS §4.2).  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::reputation](/source-reference/Namespaces/d7/d2c/namespacesgns_1_1neoswarm_1_1reputation/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::reputation::ReputationStorage](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/)** <br/>Persists [NodeReputation](/source-reference/Classes/df/d86/structsgns_1_1neoswarm_1_1reputation_1_1_node_reputation/) records to RocksDB. Falls back to an in-memory store when RocksDB is not compiled in.  |

## Detailed Description

RocksDB-backed reputation persistence (PTDS §4.2). 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_REPUTATION_REPUTATIONSTORAGE_HPP
#define NEOSWARM_REPUTATION_REPUTATIONSTORAGE_HPP

#include "node_reputation.hpp"
#include "common/error.hpp"
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace sgns::neoswarm::reputation
{
    class ReputationStorage
    {
        public:
        explicit ReputationStorage( const std::string& db_path );
        ~ReputationStorage();

        outcome::result<void> Open();

        void Close();

        outcome::result<void> Put( const NodeReputation& rep );

        outcome::result<NodeReputation> Get( const std::string& identity_key ) const;

        outcome::result<void> Remove( const std::string& identity_key );

        outcome::result<std::vector<NodeReputation>> GetAll() const;

        bool IsOpen() const
        {
            return open_;
        }

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
        std::string db_path_;
        bool open_ = false;

        static std::string Serialize( const NodeReputation& rep );
        static NodeReputation Deserialize( const std::string& data );
    };

} // namespace sgns::neoswarm::reputation

#endif // NEOSWARM_REPUTATION_REPUTATIONSTORAGE_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
