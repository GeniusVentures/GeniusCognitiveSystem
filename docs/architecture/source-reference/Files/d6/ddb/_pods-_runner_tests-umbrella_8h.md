---
title: GNUS-NEO-SWARM/ui/macos/Pods/Target Support Files/Pods-RunnerTests/Pods-RunnerTests-umbrella.h

---

# GNUS-NEO-SWARM/ui/macos/Pods/Target Support Files/Pods-RunnerTests/Pods-RunnerTests-umbrella.h





## Attributes

|                | Name           |
| -------------- | -------------- |
| [FOUNDATION_EXPORT](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#define-foundation-export) double | **[Pods_RunnerTestsVersionNumber](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#variable-pods-runnertestsversionnumber)**  |
| [FOUNDATION_EXPORT](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#define-foundation-export) const unsigned char[] | **[Pods_RunnerTestsVersionString](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#variable-pods-runnertestsversionstring)**  |

## Defines

|                | Name           |
| -------------- | -------------- |
|  | **[FOUNDATION_EXPORT](/source-reference/Files/d6/ddb/_pods-_runner_tests-umbrella_8h/#define-foundation-export)**  |



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

Updated on 2026-06-28 at 13:58:22 -0700
