---
title: GNUS-NEO-SWARM/ui/macos/Pods/Target Support Files/Pods-RunnerTests/Pods-RunnerTests-umbrella.h

---

# GNUS-NEO-SWARM/ui/macos/Pods/Target Support Files/Pods-RunnerTests/Pods-RunnerTests-umbrella.h





## Attributes

|                | Name           |
| -------------- | -------------- |
| [FOUNDATION_EXPORT](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#define-foundation_export) double | **[Pods_RunnerTestsVersionNumber](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#variable-pods_runnertestsversionnumber)**  |
| [FOUNDATION_EXPORT](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#define-foundation_export) const unsigned char[] | **[Pods_RunnerTestsVersionString](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#variable-pods_runnertestsversionstring)**  |

## Defines

|                | Name           |
| -------------- | -------------- |
|  | **[FOUNDATION_EXPORT](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#define-foundation_export)**  |



## Attributes Documentation

### variable Pods_RunnerTestsVersionNumber

```cpp
FOUNDATION_EXPORT double Pods_RunnerTestsVersionNumber;
```


### variable Pods_RunnerTestsVersionString

```cpp
FOUNDATION_EXPORT const unsigned char[] Pods_RunnerTestsVersionString;
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


FOUNDATION_EXPORT double Pods_RunnerTestsVersionNumber;
FOUNDATION_EXPORT const unsigned char Pods_RunnerTestsVersionString[];
```


-------------------------------

Updated on 2026-06-28 at 23:28:43 -0700
