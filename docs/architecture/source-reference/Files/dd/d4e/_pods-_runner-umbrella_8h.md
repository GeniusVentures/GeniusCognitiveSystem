---
title: GNUS-NEO-SWARM/ui/macos/Pods/Target Support Files/Pods-Runner/Pods-Runner-umbrella.h

---

# GNUS-NEO-SWARM/ui/macos/Pods/Target Support Files/Pods-Runner/Pods-Runner-umbrella.h





## Attributes

|                | Name           |
| -------------- | -------------- |
| [FOUNDATION_EXPORT](/source-reference/Files/dd/d4e/_pods-_runner-umbrella_8h/#define-foundation_export) double | **[Pods_RunnerVersionNumber](/source-reference/Files/dd/d4e/_pods-_runner-umbrella_8h/#variable-pods_runnerversionnumber)**  |
| [FOUNDATION_EXPORT](/source-reference/Files/dd/d4e/_pods-_runner-umbrella_8h/#define-foundation_export) const unsigned char[] | **[Pods_RunnerVersionString](/source-reference/Files/dd/d4e/_pods-_runner-umbrella_8h/#variable-pods_runnerversionstring)**  |

## Defines

|                | Name           |
| -------------- | -------------- |
|  | **[FOUNDATION_EXPORT](/source-reference/Files/dd/d4e/_pods-_runner-umbrella_8h/#define-foundation_export)**  |



## Attributes Documentation

### variable Pods_RunnerVersionNumber

```cpp
FOUNDATION_EXPORT double Pods_RunnerVersionNumber;
```


### variable Pods_RunnerVersionString

```cpp
FOUNDATION_EXPORT const unsigned char[] Pods_RunnerVersionString;
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


FOUNDATION_EXPORT double Pods_RunnerVersionNumber;
FOUNDATION_EXPORT const unsigned char Pods_RunnerVersionString[];
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
