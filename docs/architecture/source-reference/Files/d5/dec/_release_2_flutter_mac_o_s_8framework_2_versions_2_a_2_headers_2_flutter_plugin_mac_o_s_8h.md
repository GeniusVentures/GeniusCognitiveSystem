---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterPluginMacOS.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterPluginMacOS.h








## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERPLUGINMACOS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERPLUGINMACOS_H_

#import <Foundation/Foundation.h>

#import "FlutterAppLifecycleDelegate.h"
#import "FlutterChannels.h"
#import "FlutterCodecs.h"
#import "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN

@protocol FlutterPluginRegistrar;

FLUTTER_DARWIN_EXPORT
@protocol FlutterPlugin <NSObject, FlutterAppLifecycleDelegate>

+ (void)registerWithRegistrar:(id<FlutterPluginRegistrar>)registrar;

@optional

- (void)handleMethodCall:(FlutterMethodCall*)call result:(FlutterResult)result;

NS_ASSUME_NONNULL_END

@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERPLUGINMACOS_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
