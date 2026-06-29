---
title: sgns::neoswarm::reputation::ReputationStorage
summary: Persists NodeReputation records to RocksDB. Falls back to an in-memory store when RocksDB is not compiled in. 

---

# sgns::neoswarm::reputation::ReputationStorage



Persists [NodeReputation](/source-reference/Classes/df/d86/structsgns_1_1neoswarm_1_1reputation_1_1_node_reputation/) records to RocksDB. Falls back to an in-memory store when RocksDB is not compiled in. 


`#include <reputation_storage.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Impl](/source-reference/Classes/d5/dee/structsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage_1_1_impl/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[ReputationStorage](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/#function-reputationstorage)**(const std::string & db_path)<br/>Construct storage pointing at the given database path.  |
| | **[~ReputationStorage](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/#function-~reputationstorage)**() |
| outcome::result< void > | **[Open](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/#function-open)**()<br/>Open (or create) the database.  |
| void | **[Close](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/#function-close)**()<br/>Close the database.  |
| outcome::result< void > | **[Put](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/#function-put)**(const [NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/) & rep)<br/>Persist a reputation record.  |
| outcome::result< [NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/) > | **[Get](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/#function-get)**(const std::string & identity_key) const<br/>Retrieve a reputation record by identity key.  |
| outcome::result< void > | **[Remove](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/#function-remove)**(const std::string & identity_key)<br/>Delete a reputation record.  |
| outcome::result< std::vector< [NodeReputation](/source-reference/Classes/d9/dea/structsgns_1_1neoswarm_1_1_node_reputation/) > > | **[GetAll](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/#function-getall)**() const<br/>Retrieve all stored reputation records.  |
| bool | **[IsOpen](/source-reference/Classes/dd/d0a/classsgns_1_1neoswarm_1_1reputation_1_1_reputation_storage/#function-isopen)**() const |

## Public Functions Documentation

### function ReputationStorage

```cpp
explicit ReputationStorage(
    const std::string & db_path
)
```

Construct storage pointing at the given database path. 

**Parameters**: 

  * **db_path** Filesystem path for the RocksDB database directory. 


### function ~ReputationStorage

```cpp
~ReputationStorage()
```


### function Open

```cpp
outcome::result< void > Open()
```

Open (or create) the database. 

**Return**: outcome::success or StorageError. 

### function Close

```cpp
void Close()
```

Close the database. 

### function Put

```cpp
outcome::result< void > Put(
    const NodeReputation & rep
)
```

Persist a reputation record. 

**Parameters**: 

  * **rep** Record to store. 


**Return**: outcome::success or StorageError. 

### function Get

```cpp
outcome::result< NodeReputation > Get(
    const std::string & identity_key
) const
```

Retrieve a reputation record by identity key. 

**Parameters**: 

  * **identity_key** Node identity key. 


**Return**: [NodeReputation](/source-reference/Classes/df/d86/structsgns_1_1neoswarm_1_1reputation_1_1_node_reputation/) or ReputationNotFound / StorageError. 

### function Remove

```cpp
outcome::result< void > Remove(
    const std::string & identity_key
)
```

Delete a reputation record. 

**Parameters**: 

  * **identity_key** Node identity key. 


**Return**: outcome::success or StorageError. 

### function GetAll

```cpp
outcome::result< std::vector< NodeReputation > > GetAll() const
```

Retrieve all stored reputation records. 

**Return**: Vector of all records or StorageError. 

### function IsOpen

```cpp
inline bool IsOpen() const
```


**Return**: True if the database is currently open. 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700