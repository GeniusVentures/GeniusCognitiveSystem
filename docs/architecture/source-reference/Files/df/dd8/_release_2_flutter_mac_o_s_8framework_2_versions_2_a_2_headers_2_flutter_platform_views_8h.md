---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterPlatformViews.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterPlatformViews.h








## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERPLATFORMVIEWS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERPLATFORMVIEWS_H_

#import <AppKit/AppKit.h>

#import "FlutterCodecs.h"
#import "FlutterMacros.h"

@protocol FlutterPlatformViewFactory <NSObject>

- (nonnull NSView*)createWithViewIdentifier:(int64_t)viewId arguments:(nullable id)args;

@optional
- (nullable NSObject<FlutterMessageCodec>*)createArgsCodec;
@end

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_MACOS_FRAMEWORK_HEADERS_FLUTTERPLATFORMVIEWS_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
