---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterAppLifecycleDelegate.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterAppLifecycleDelegate.h








## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERAPPLIFECYCLEDELEGATE_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERAPPLIFECYCLEDELEGATE_H_

#import <Cocoa/Cocoa.h>
#include <Foundation/Foundation.h>

#import "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN

#pragma mark -
FLUTTER_DARWIN_EXPORT
@protocol FlutterAppLifecycleDelegate <NSObject>

@optional
- (void)handleWillFinishLaunching:(NSNotification*)notification;

- (void)handleDidFinishLaunching:(NSNotification*)notification;

- (void)handleWillBecomeActive:(NSNotification*)notification;

- (void)handleDidBecomeActive:(NSNotification*)notification;

- (void)handleWillResignActive:(NSNotification*)notification;

- (void)handleDidResignActive:(NSNotification*)notification;

- (void)handleWillHide:(NSNotification*)notification;

- (void)handleDidHide:(NSNotification*)notification;

- (void)handleWillUnhide:(NSNotification*)notification;

- (void)handleDidUnhide:(NSNotification*)notification;

- (void)handleDidChangeScreenParameters:(NSNotification*)notification;

- (void)handleDidChangeOcclusionState:(NSNotification*)notification;

- (BOOL)handleOpenURLs:(NSArray<NSURL*>*)urls;

- (void)handleWillTerminate:(NSNotification*)notification;
@end

#pragma mark -

FLUTTER_DARWIN_EXPORT
@interface FlutterAppLifecycleRegistrar : NSObject <FlutterAppLifecycleDelegate>

- (void)addDelegate:(NSObject<FlutterAppLifecycleDelegate>*)delegate;

- (void)removeDelegate:(NSObject<FlutterAppLifecycleDelegate>*)delegate;
@end

NS_ASSUME_NONNULL_END

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERAPPLIFECYCLEDELEGATE_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
