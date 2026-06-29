---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterAppDelegate.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterAppDelegate.h








## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERAPPDELEGATE_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERAPPDELEGATE_H_

#import <Cocoa/Cocoa.h>

#import "FlutterAppLifecycleDelegate.h"
#import "FlutterMacros.h"

FLUTTER_DARWIN_EXPORT
@protocol FlutterAppLifecycleProvider <NSObject>

- (void)addApplicationLifecycleDelegate:(nonnull NSObject<FlutterAppLifecycleDelegate>*)delegate;

- (void)removeApplicationLifecycleDelegate:(nonnull NSObject<FlutterAppLifecycleDelegate>*)delegate;

@end

FLUTTER_DARWIN_EXPORT
@interface FlutterAppDelegate : NSObject <NSApplicationDelegate, FlutterAppLifecycleProvider>

@property(weak, nonatomic, nullable) IBOutlet NSMenu* applicationMenu;

@property(weak, nonatomic, nullable) IBOutlet NSWindow* mainFlutterWindow;

@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERAPPDELEGATE_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
