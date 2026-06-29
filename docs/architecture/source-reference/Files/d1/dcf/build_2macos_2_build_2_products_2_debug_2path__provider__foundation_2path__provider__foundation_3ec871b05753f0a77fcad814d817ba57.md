---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/path_provider_foundation/path_provider_foundation.framework/Versions/A/Headers/path_provider_foundation-umbrella.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/path_provider_foundation/path_provider_foundation.framework/Versions/A/Headers/path_provider_foundation-umbrella.h





## Attributes

|                | Name           |
| -------------- | -------------- |
| [FOUNDATION_EXPORT](/source-reference/Files/d1/dcf/build_2macos_2_build_2_products_2_debug_2path__provider__foundation_2path__provider__foundation_3ec871b05753f0a77fcad814d817ba57/#define-foundation_export) double | **[path_provider_foundationVersionNumber](/source-reference/Files/d1/dcf/build_2macos_2_build_2_products_2_debug_2path__provider__foundation_2path__provider__foundation_3ec871b05753f0a77fcad814d817ba57/#variable-path_provider_foundationversionnumber)**  |
| [FOUNDATION_EXPORT](/source-reference/Files/d1/dcf/build_2macos_2_build_2_products_2_debug_2path__provider__foundation_2path__provider__foundation_3ec871b05753f0a77fcad814d817ba57/#define-foundation_export) const unsigned char[] | **[path_provider_foundationVersionString](/source-reference/Files/d1/dcf/build_2macos_2_build_2_products_2_debug_2path__provider__foundation_2path__provider__foundation_3ec871b05753f0a77fcad814d817ba57/#variable-path_provider_foundationversionstring)**  |

## Defines

|                | Name           |
| -------------- | -------------- |
|  | **[FOUNDATION_EXPORT](/source-reference/Files/d1/dcf/build_2macos_2_build_2_products_2_debug_2path__provider__foundation_2path__provider__foundation_3ec871b05753f0a77fcad814d817ba57/#define-foundation_export)**  |



## Attributes Documentation

### variable path_provider_foundationVersionNumber

```cpp
FOUNDATION_EXPORT double path_provider_foundationVersionNumber;
```


### variable path_provider_foundationVersionString

```cpp
FOUNDATION_EXPORT const unsigned char[] path_provider_foundationVersionString;
```



## Macros Documentation

### define FOUNDATION_EXPORT

```cpp
#define FOUNDATION_EXPORT extern
```


## Source code

```cpp
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
#ifndef FOUNDATION_EXPORT
#if defined(__cplusplus)
#define FOUNDATION_EXPORT extern "C"
#else
#define FOUNDATION_EXPORT extern
#endif
#endif
#endif


FOUNDATION_EXPORT double path_provider_foundationVersionNumber;
FOUNDATION_EXPORT const unsigned char path_provider_foundationVersionString[];
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
