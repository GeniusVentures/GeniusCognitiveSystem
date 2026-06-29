---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/url_launcher_macos/url_launcher_macos.framework/Versions/A/Headers/url_launcher_macos-umbrella.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/url_launcher_macos/url_launcher_macos.framework/Versions/A/Headers/url_launcher_macos-umbrella.h





## Attributes

|                | Name           |
| -------------- | -------------- |
| [FOUNDATION_EXPORT](/source-reference/Files/d2/d45/url__launcher__macos-umbrella_8h/#define-foundation_export) double | **[url_launcher_macosVersionNumber](/source-reference/Files/d2/d45/url__launcher__macos-umbrella_8h/#variable-url_launcher_macosversionnumber)**  |
| [FOUNDATION_EXPORT](/source-reference/Files/d2/d45/url__launcher__macos-umbrella_8h/#define-foundation_export) const unsigned char[] | **[url_launcher_macosVersionString](/source-reference/Files/d2/d45/url__launcher__macos-umbrella_8h/#variable-url_launcher_macosversionstring)**  |

## Defines

|                | Name           |
| -------------- | -------------- |
|  | **[FOUNDATION_EXPORT](/source-reference/Files/d2/d45/url__launcher__macos-umbrella_8h/#define-foundation_export)**  |



## Attributes Documentation

### variable url_launcher_macosVersionNumber

```cpp
FOUNDATION_EXPORT double url_launcher_macosVersionNumber;
```


### variable url_launcher_macosVersionString

```cpp
FOUNDATION_EXPORT const unsigned char[] url_launcher_macosVersionString;
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


FOUNDATION_EXPORT double url_launcher_macosVersionNumber;
FOUNDATION_EXPORT const unsigned char url_launcher_macosVersionString[];
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
