---
title: GNUS-NEO-SWARM/test/benchmark/os_memory.hpp
summary: Platform-specific peak-memory measurement for benchmarks. 

---

# GNUS-NEO-SWARM/test/benchmark/os_memory.hpp



Platform-specific peak-memory measurement for benchmarks.  [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| size_t | **[GetCurrentMemoryMB](/source-reference/Files/d2/de6/os__memory_8hpp/#function-getcurrentmemorymb)**() |

## Detailed Description

Platform-specific peak-memory measurement for benchmarks. 

**Date**: 2026-06-18


Centralizes OS-specific memory APIs so benchmark source files contain zero #ifdef gates. 


## Functions Documentation

### function GetCurrentMemoryMB

```cpp
inline size_t GetCurrentMemoryMB()
```




## Source code

```cpp


#ifndef BENCH_OS_MEMORY_HPP
#define BENCH_OS_MEMORY_HPP

#include <cstddef>

#ifdef __APPLE__
#include <mach/mach.h>

inline size_t GetCurrentMemoryMB()
{
    struct mach_task_basic_info info;
    mach_msg_type_number_t count = MACH_TASK_BASIC_INFO_COUNT;
    if ( task_info( mach_task_self(), MACH_TASK_BASIC_INFO,
                    reinterpret_cast<task_info_t>( &info ), &count ) == KERN_SUCCESS )
    {
        return info.resident_size / ( 1024 * 1024 );
    }
    return 0;
}
#else
inline size_t GetCurrentMemoryMB()
{
    return 0;
}
#endif

#endif // BENCH_OS_MEMORY_HPP
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
