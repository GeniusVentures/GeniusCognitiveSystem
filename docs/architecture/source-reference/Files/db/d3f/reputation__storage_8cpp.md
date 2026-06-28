---
title: GNUS-NEO-SWARM/src/reputation/reputation_storage.cpp
summary: RocksDB-backed reputation persistence implementation. 

---

# GNUS-NEO-SWARM/src/reputation/reputation_storage.cpp



RocksDB-backed reputation persistence implementation.  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::reputation](/source-reference/Namespaces/d7/d2c/namespacesgns_1_1neoswarm_1_1reputation/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[sgns::neoswarm::reputation::ReputationStorage::Impl](/source-reference/Classes/d5/dee/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage_1_1_impl/)**  |

## Detailed Description

RocksDB-backed reputation persistence implementation. 

**Date**: 2026-05-06 



## Source code

```cpp


#include "reputation_storage.hpp"
#include "common/logging.hpp"

#include <nlohmann/json.hpp>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

#include <rocksdb/db.h>
#include <rocksdb/options.h>
#include <rocksdb/slice.h>

namespace sgns::neoswarm::reputation
{
    namespace
    {
        auto StorageLogger()
        {
            return neoswarm::CreateLogger( "ReputationStorage" );
        }
    } // namespace

    // -----------------------------------------------------------------------
    // Serialization — JSON (nlohmann/json)
    // -----------------------------------------------------------------------
    std::string ReputationStorage::Serialize( const NodeReputation& r )
    {
        nlohmann::json j;
        j["identity_key"] = r.m_identityKey;
        j["global_score"] = r.m_globalScore;
        j["math_score"] = r.m_mathScore;
        j["grammar_score"] = r.m_grammarScore;
        j["latency_score"] = r.m_latencyScore;
        j["consistency_score"] = r.m_consistencyScore;
        j["task_count"] = r.m_taskCount;
        j["last_updated_ms"] = r.m_lastUpdatedMs;
        return j.dump();
    }

    NodeReputation ReputationStorage::Deserialize( const std::string& data )
    {
        NodeReputation r;
        try
        {
            nlohmann::json j = nlohmann::json::parse( data );
            r.m_identityKey = j.value( "identity_key", "" );
            r.m_globalScore = j.value( "global_score", 0.0 );
            r.m_mathScore = j.value( "math_score", 0.0 );
            r.m_grammarScore = j.value( "grammar_score", 0.0 );
            r.m_latencyScore = j.value( "latency_score", 0.0 );
            r.m_consistencyScore = j.value( "consistency_score", 0.0 );
            r.m_taskCount = j.value( "task_count", 0ULL );
            r.m_lastUpdatedMs = j.value( "last_updated_ms", 0ULL );
        }
        catch ( const std::exception& e )
        {
            StorageLogger()->error( "Corrupt reputation record, skipping: {}", e.what() );
            r.m_identityKey = "";
        }
        return r;
    }

    // -----------------------------------------------------------------------
    // Impl
    // -----------------------------------------------------------------------
    struct ReputationStorage::Impl
    {
        rocksdb::DB* m_db = nullptr;
        rocksdb::Options options_;

    };

    ReputationStorage::ReputationStorage( const std::string& db_path )
        : m_impl( std::make_unique<Impl>() )
        , db_path_( db_path )
    {
    }

    ReputationStorage::~ReputationStorage()
    {
        Close();
    }

    // -----------------------------------------------------------------------
    // Open
    // -----------------------------------------------------------------------
    outcome::result<void> ReputationStorage::Open()
    {
        m_impl->options_.create_if_missing = true;
        rocksdb::Status status = rocksdb::DB::Open( m_impl->options_, db_path_, &m_impl->m_db );
        if ( !status.ok() )
        {
            return outcome::failure( Error::StorageError );
        }
        StorageLogger()->info( "ReputationStorage opened: {}", db_path_ );

        open_ = true;
        return outcome::success();
    }

    // -----------------------------------------------------------------------
    // Close
    // -----------------------------------------------------------------------
    void ReputationStorage::Close()
    {
        if ( m_impl && m_impl->m_db )
        {
            delete m_impl->m_db;
            m_impl->m_db = nullptr;
        }
        open_ = false;
    }

    // -----------------------------------------------------------------------
    // Put
    // -----------------------------------------------------------------------
    outcome::result<void> ReputationStorage::Put( const NodeReputation& rep )
    {
        if ( !open_ )
        {
            return outcome::failure( Error::StorageError );
        }
        std::string val = Serialize( rep );
        auto status = m_impl->m_db->Put( rocksdb::WriteOptions(), rep.m_identityKey, val );
        if ( !status.ok() )
        {
            return outcome::failure( Error::StorageError );
        }

        return outcome::success();
    }

    // -----------------------------------------------------------------------
    // Get
    // -----------------------------------------------------------------------
    outcome::result<NodeReputation> ReputationStorage::Get( const std::string& identity_key ) const
    {
        if ( !open_ )
        {
            return outcome::failure( Error::StorageError );
        }
        std::string val;
        rocksdb::Status status = m_impl->m_db->Get( rocksdb::ReadOptions(), identity_key, &val );
        if ( status.IsNotFound() )
        {
            return outcome::failure( Error::ReputationNotFound );
        }
        if ( !status.ok() )
        {
            return outcome::failure( Error::StorageError );
        }
        return outcome::success( Deserialize( val ) );

    }

    // -----------------------------------------------------------------------
    // Remove
    // -----------------------------------------------------------------------
    outcome::result<void> ReputationStorage::Remove( const std::string& identity_key )
    {
        if ( !open_ )
        {
            return outcome::failure( Error::StorageError );
        }
        auto status = m_impl->m_db->Delete( rocksdb::WriteOptions(), identity_key );
        if ( !status.ok() )
        {
            return outcome::failure( Error::StorageError );
        }

        return outcome::success();
    }

    // -----------------------------------------------------------------------
    // GetAll
    // -----------------------------------------------------------------------
    outcome::result<std::vector<NodeReputation>> ReputationStorage::GetAll() const
    {
        if ( !open_ )
        {
            return outcome::failure( Error::StorageError );
        }
        std::vector<NodeReputation> result;
        auto* it = m_impl->m_db->NewIterator( rocksdb::ReadOptions() );
        for ( it->SeekToFirst(); it->Valid(); it->Next() )
        {
            result.push_back( Deserialize( it->value().ToString() ) );
        }
        delete it;

        return outcome::success( std::move( result ) );
    }

} // namespace sgns::neoswarm::reputation
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
