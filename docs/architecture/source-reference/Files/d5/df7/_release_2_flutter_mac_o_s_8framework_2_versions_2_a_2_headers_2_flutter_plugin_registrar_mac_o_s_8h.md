---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterPluginRegistrarMacOS.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterPluginRegistrarMacOS.h








## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERPLUGINREGISTRARMACOS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERPLUGINREGISTRARMACOS_H_

#import <Cocoa/Cocoa.h>

#import "FlutterBinaryMessenger.h"
#import "FlutterChannels.h"
#import "FlutterMacros.h"
#import "FlutterPlatformViews.h"
#import "FlutterPluginMacOS.h"
#import "FlutterTexture.h"

// TODO(stuartmorgan): Merge this file and FlutterPluginMacOS.h with the iOS FlutterPlugin.h,
// sharing all but the platform-specific methods.

FLUTTER_DARWIN_EXPORT
@protocol FlutterPluginRegistrar <NSObject>

@property(nonnull, readonly) id<FlutterBinaryMessenger> messenger;

@property(nonnull, readonly) id<FlutterTextureRegistry> textures;

@property(nullable, readonly) NSView* view;

@property(nullable, readonly) NSViewController* viewController;

- (void)addMethodCallDelegate:(nonnull id<FlutterPlugin>)delegate
                      channel:(nonnull FlutterMethodChannel*)channel;

- (void)addApplicationDelegate:(nonnull NSObject<FlutterAppLifecycleDelegate>*)delegate;

- (void)registerViewFactory:(nonnull NSObject<FlutterPlatformViewFactory>*)factory
                     withId:(nonnull NSString*)factoryId;

- (void)publish:(nonnull NSObject*)value;

- (nonnull NSString*)lookupKeyForAsset:(nonnull NSString*)asset;

- (nonnull NSString*)lookupKeyForAsset:(nonnull NSString*)asset
                           fromPackage:(nonnull NSString*)package;

@end

@protocol FlutterPluginRegistry <NSObject>

- (nonnull id<FlutterPluginRegistrar>)registrarForPlugin:(nonnull NSString*)pluginKey;

- (nullable NSObject*)valuePublishedByPlugin:(nonnull NSString*)pluginKey;

@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERPLUGINREGISTRARMACOS_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
