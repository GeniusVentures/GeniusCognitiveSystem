---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterEngine.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterEngine.h





## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/)**  |




## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERENGINE_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERENGINE_H_

#import <Foundation/Foundation.h>

#include <stdint.h>

#import "FlutterAppLifecycleDelegate.h"
#import "FlutterBinaryMessenger.h"
#import "FlutterDartProject.h"
#import "FlutterMacros.h"
#import "FlutterPluginRegistrarMacOS.h"
#import "FlutterTexture.h"

// TODO(stuartmorgan): Merge this file with the iOS FlutterEngine.h.

@class FlutterViewController;

FLUTTER_DARWIN_EXPORT
@interface FlutterEngine
    : NSObject <FlutterTextureRegistry, FlutterPluginRegistry, FlutterAppLifecycleDelegate>

- (nonnull instancetype)initWithName:(nonnull NSString*)labelPrefix
                             project:(nullable FlutterDartProject*)project;

- (nonnull instancetype)initWithName:(nonnull NSString*)labelPrefix
                             project:(nullable FlutterDartProject*)project
              allowHeadlessExecution:(BOOL)allowHeadlessExecution NS_DESIGNATED_INITIALIZER;

- (nonnull instancetype)init NS_UNAVAILABLE;

- (BOOL)runWithEntrypoint:(nullable NSString*)entrypoint;

@property(nonatomic, nullable, weak) FlutterViewController* viewController;

@property(nonatomic, nonnull, readonly) id<FlutterBinaryMessenger> binaryMessenger;

- (void)shutDownEngine;

@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERENGINE_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
