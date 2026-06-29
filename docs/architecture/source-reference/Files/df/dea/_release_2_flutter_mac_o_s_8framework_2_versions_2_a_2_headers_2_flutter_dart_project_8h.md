---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterDartProject.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterDartProject.h





## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/)**  |




## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERDARTPROJECT_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERDARTPROJECT_H_

#import <Foundation/Foundation.h>

#import "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN

FLUTTER_DARWIN_EXPORT
@interface FlutterDartProject : NSObject

- (instancetype)initWithPrecompiledDartBundle:(nullable NSBundle*)bundle NS_DESIGNATED_INITIALIZER;
- (instancetype)initFromDefaultSourceForConfiguration API_UNAVAILABLE(macos)
    FLUTTER_UNAVAILABLE("Use -init instead.");

+ (NSString*)defaultBundleIdentifier;

@property(nonatomic, nullable, copy)
    NSArray<NSString*>* dartEntrypointArguments API_UNAVAILABLE(ios);

+ (NSString*)lookupKeyForAsset:(NSString*)asset;

+ (NSString*)lookupKeyForAsset:(NSString*)asset fromBundle:(nullable NSBundle*)bundle;

+ (NSString*)lookupKeyForAsset:(NSString*)asset fromPackage:(NSString*)package;

+ (NSString*)lookupKeyForAsset:(NSString*)asset
                   fromPackage:(NSString*)package
                    fromBundle:(nullable NSBundle*)bundle;

@end

NS_ASSUME_NONNULL_END

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERDARTPROJECT_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
