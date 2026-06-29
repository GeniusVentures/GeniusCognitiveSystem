---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterMacros.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterMacros.h





## Defines

|                | Name           |
| -------------- | -------------- |
|  | **[FLUTTER_DARWIN_EXPORT](/source-reference/Files/d7/d06/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-flutter_darwin_export)**  |
|  | **[NS_ASSUME_NONNULL_BEGIN](/source-reference/Files/d7/d06/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-ns_assume_nonnull_begin)**  |
|  | **[NS_ASSUME_NONNULL_END](/source-reference/Files/d7/d06/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-ns_assume_nonnull_end)**  |
|  | **[FLUTTER_DEPRECATED](/source-reference/Files/d7/d06/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-flutter_deprecated)**(msg)  |
|  | **[FLUTTER_UNAVAILABLE](/source-reference/Files/d7/d06/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-flutter_unavailable)**(msg)  |
|  | **[FLUTTER_ASSERT_ARC](/source-reference/Files/d7/d06/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-flutter_assert_arc)**  |
|  | **[FLUTTER_ASSERT_NOT_ARC](/source-reference/Files/d7/d06/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-flutter_assert_not_arc)**  |




## Macros Documentation

### define FLUTTER_DARWIN_EXPORT

```cpp
#define FLUTTER_DARWIN_EXPORT 
```


### define NS_ASSUME_NONNULL_BEGIN

```cpp
#define NS_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
```


### define NS_ASSUME_NONNULL_END

```cpp
#define NS_ASSUME_NONNULL_END _Pragma("clang assume_nonnull end")
```


### define FLUTTER_DEPRECATED

```cpp
#define FLUTTER_DEPRECATED(
    msg
)
__attribute__((__deprecated__(msg)))
```


Indicates that the API has been deprecated for the specified reason. Code that uses the deprecated API will continue to work as before. However, the API will soon become unavailable and users are encouraged to immediately take the appropriate action mentioned in the deprecation message and the BREAKING CHANGES section present in the Flutter.h umbrella header. 


### define FLUTTER_UNAVAILABLE

```cpp
#define FLUTTER_UNAVAILABLE(
    msg
)
__attribute__((__unavailable__(msg)))
```


Indicates that the previously deprecated API is now unavailable. Code that uses the API will not work and the declaration of the API is only a stub meant to display the given message detailing the actions for the user to take immediately. 


### define FLUTTER_ASSERT_ARC

```cpp
#define FLUTTER_ASSERT_ARC #error ARC must be enabled !
```


### define FLUTTER_ASSERT_NOT_ARC

```cpp
#define FLUTTER_ASSERT_NOT_ARC 
```


## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERMACROS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERMACROS_H_

#if defined(FLUTTER_FRAMEWORK)

#define FLUTTER_DARWIN_EXPORT __attribute__((visibility("default")))

#else  // defined(FLUTTER_SDK)

#define FLUTTER_DARWIN_EXPORT

#endif  // defined(FLUTTER_SDK)

#ifndef NS_ASSUME_NONNULL_BEGIN
#define NS_ASSUME_NONNULL_BEGIN _Pragma("clang assume_nonnull begin")
#define NS_ASSUME_NONNULL_END _Pragma("clang assume_nonnull end")
#endif  // defined(NS_ASSUME_NONNULL_BEGIN)

#define FLUTTER_DEPRECATED(msg) __attribute__((__deprecated__(msg)))

#define FLUTTER_UNAVAILABLE(msg) __attribute__((__unavailable__(msg)))

#if __has_feature(objc_arc)
#define FLUTTER_ASSERT_ARC
#define FLUTTER_ASSERT_NOT_ARC #error ARC must be disabled !
#else
#define FLUTTER_ASSERT_ARC #error ARC must be enabled !
#define FLUTTER_ASSERT_NOT_ARC
#endif

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERMACROS_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
