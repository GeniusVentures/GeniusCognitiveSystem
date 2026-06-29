---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterViewController.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterViewController.h





## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/)**  |

## Functions

|                | Name           |
| -------------- | -------------- |
| typedef | **[NS_ENUM](/source-reference/Files/dd/d4e/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_view_controller_8h/#function-ns_enum)**(NSInteger , FlutterMouseTrackingMode ) |


## Functions Documentation

### function NS_ENUM

```cpp
typedef NS_ENUM(
    NSInteger ,
    FlutterMouseTrackingMode 
)
```


Values for the `mouseTrackingMode` property. 




## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERVIEWCONTROLLER_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERVIEWCONTROLLER_H_

#import <Cocoa/Cocoa.h>

#import "FlutterEngine.h"
#import "FlutterMacros.h"
#import "FlutterPlatformViews.h"
#import "FlutterPluginRegistrarMacOS.h"

typedef NS_ENUM(NSInteger, FlutterMouseTrackingMode) {
  // Hover events will never be sent to Flutter.
  kFlutterMouseTrackingModeNone = 0,
  // NOLINTNEXTLINE(readability-identifier-naming)
  FlutterMouseTrackingModeNone __attribute__((deprecated)) = kFlutterMouseTrackingModeNone,

  // Hover events will be sent to Flutter when the view is in the key window.
  kFlutterMouseTrackingModeInKeyWindow = 1,
  // NOLINTNEXTLINE(readability-identifier-naming)
  FlutterMouseTrackingModeInKeyWindow
  __attribute__((deprecated)) = kFlutterMouseTrackingModeInKeyWindow,

  // Hover events will be sent to Flutter when the view is in the active app.
  kFlutterMouseTrackingModeInActiveApp = 2,
  // NOLINTNEXTLINE(readability-identifier-naming)
  FlutterMouseTrackingModeInActiveApp
  __attribute__((deprecated)) = kFlutterMouseTrackingModeInActiveApp,

  // Hover events will be sent to Flutter regardless of window and app focus.
  kFlutterMouseTrackingModeAlways = 3,
  // NOLINTNEXTLINE(readability-identifier-naming)
  FlutterMouseTrackingModeAlways __attribute__((deprecated)) = kFlutterMouseTrackingModeAlways,
};

FLUTTER_DARWIN_EXPORT
@interface FlutterViewController : NSViewController <FlutterPluginRegistry>

@property(nonatomic, nonnull, readonly) FlutterEngine* engine;

@property(nonatomic) FlutterMouseTrackingMode mouseTrackingMode;

- (nonnull instancetype)initWithProject:(nullable FlutterDartProject*)project
    NS_DESIGNATED_INITIALIZER;

- (nonnull instancetype)initWithNibName:(nullable NSString*)nibNameOrNil
                                 bundle:(nullable NSBundle*)nibBundleOrNil
    NS_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithCoder:(nonnull NSCoder*)nibNameOrNil NS_DESIGNATED_INITIALIZER;
- (nonnull instancetype)initWithEngine:(nonnull FlutterEngine*)engine
                               nibName:(nullable NSString*)nibName
                                bundle:(nullable NSBundle*)nibBundle NS_DESIGNATED_INITIALIZER;

- (BOOL)attached;

- (void)onPreEngineRestart;

- (nonnull NSString*)lookupKeyForAsset:(nonnull NSString*)asset;

- (nonnull NSString*)lookupKeyForAsset:(nonnull NSString*)asset
                           fromPackage:(nonnull NSString*)package;

@property(readwrite, nonatomic, nullable, copy) NSColor* backgroundColor;

@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERVIEWCONTROLLER_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
