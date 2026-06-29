---
title: GNUS-NEO-SWARM/src/common/logging.hpp
summary: Logging facade — wraps spdlog directly. 

---

# GNUS-NEO-SWARM/src/common/logging.hpp



Logging facade — wraps spdlog directly. 

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |

## Types

|                | Name           |
| -------------- | -------------- |
| using std::shared_ptr< spdlog::logger > | **[Logger](/source-reference/Files/d0/da9/logging_8hpp/#using-logger)** <br/>[Logger](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#using-logger) sgns::base::Logger convention.  |

## Functions

|                | Name           |
| -------------- | -------------- |
| Logger | **[CreateLogger](/source-reference/Files/d0/da9/logging_8hpp/#function-createlogger)**(const std::string & tag)<br/>Create a named logger for a NEO SWARM component.  |

## Types Documentation

### using Logger

```cpp
using sgns::neoswarm::Logger = std::shared_ptr<spdlog::logger>;
```

[Logger](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#using-logger) sgns::base::Logger convention. 


## Functions Documentation

### function CreateLogger

```cpp
inline Logger CreateLogger(
    const std::string & tag
)
```

Create a named logger for a NEO SWARM component. 

**Parameters**: 

  * **tag** Component name shown in log output (e.g. "Router", "P2PNode"). 


**Return**: [Logger](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/#using-logger) instance. 



## Source code

```cpp


#ifndef NEOSWARM_COMMON_LOGGING_HPP
#define NEOSWARM_COMMON_LOGGING_HPP

#include <memory>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>
#include <string>

namespace sgns::neoswarm
{
    using Logger = std::shared_ptr<spdlog::logger>;

    inline Logger CreateLogger( const std::string& tag )
    {
        const std::string name = "NeoSwarm/" + tag;
        auto existing = spdlog::get( name );
        if ( existing )
        {
            return existing;
        }
        auto logger = spdlog::stdout_color_mt( name );
        logger->set_pattern( "[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] [%n] %v" );
        return logger;
    }

} // namespace sgns::neoswarm

#endif // NEOSWARM_COMMON_LOGGING_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
