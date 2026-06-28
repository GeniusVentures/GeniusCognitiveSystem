---
title: GNUS-NEO-SWARM/flutter_slm_bridge/src/os_defines.h
summary: Platform abstraction for flutter_slm_bridge. 

---

# GNUS-NEO-SWARM/flutter_slm_bridge/src/os_defines.h



Platform abstraction for flutter_slm_bridge.  [More...](#detailed-description)

## Defines

|                | Name           |
| -------------- | -------------- |
|  | **[FFI_PLUGIN_EXPORT](/source-reference/Files/d5/d0b/os__defines_8h/#define-ffi-plugin-export)**  |
|  | **[PLATFORM_SLEEP_MS](/source-reference/Files/d5/d0b/os__defines_8h/#define-platform-sleep-ms)**(ms)  |

## Detailed Description

Platform abstraction for flutter_slm_bridge. 

**Date**: 2026-06-18


Centralizes all OS-specific includes and macros so the main public header ([flutter_slm_bridge.h](/source-reference/Files/d7/d34/flutter__slm__bridge_8h/#file-flutter-slm-bridge.h)) contains zero #ifdef gates. 




## Macros Documentation

### define FFI_PLUGIN_EXPORT

```cpp
#define FFI_PLUGIN_EXPORT 
```


### define PLATFORM_SLEEP_MS

```cpp
#define PLATFORM_SLEEP_MS(
    ms
)
usleep( ( ms ) * 1000 )
```


## Source code

```cpp


#ifndef FLUTTER_SLM_BRIDGE_OS_DEFINES_H
#define FLUTTER_SLM_BRIDGE_OS_DEFINES_H

#if _WIN32
#include <windows.h>
#define FFI_PLUGIN_EXPORT __declspec( dllexport )
#define PLATFORM_SLEEP_MS( ms ) Sleep( ms )
#else
#include <pthread.h>
#include <unistd.h>
#define FFI_PLUGIN_EXPORT
#define PLATFORM_SLEEP_MS( ms ) usleep( ( ms ) * 1000 )
#endif

#endif // FLUTTER_SLM_BRIDGE_OS_DEFINES_H
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
