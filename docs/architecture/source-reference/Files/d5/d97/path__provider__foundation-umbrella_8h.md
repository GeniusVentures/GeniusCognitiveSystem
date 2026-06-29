---
title: GNUS-NEO-SWARM/ui/macos/Pods/Target Support Files/path_provider_foundation/path_provider_foundation-umbrella.h

---

# GNUS-NEO-SWARM/ui/macos/Pods/Target Support Files/path_provider_foundation/path_provider_foundation-umbrella.h





## Attributes

|                | Name           |
| -------------- | -------------- |
| [FOUNDATION_EXPORT](/source-reference/Files/d5/d97/path__provider__foundation-umbrella_8h/#define-foundation_export) double | **[path_provider_foundationVersionNumber](/source-reference/Files/d5/d97/path__provider__foundation-umbrella_8h/#variable-path_provider_foundationversionnumber)**  |
| [FOUNDATION_EXPORT](/source-reference/Files/d5/d97/path__provider__foundation-umbrella_8h/#define-foundation_export) const unsigned char[] | **[path_provider_foundationVersionString](/source-reference/Files/d5/d97/path__provider__foundation-umbrella_8h/#variable-path_provider_foundationversionstring)**  |

## Defines

|                | Name           |
| -------------- | -------------- |
|  | **[FOUNDATION_EXPORT](/source-reference/Files/d5/d97/path__provider__foundation-umbrella_8h/#define-foundation_export)**  |



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

Updated on 2026-06-28 at 23:28:42 -0700
