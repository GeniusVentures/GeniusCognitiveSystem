---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterTexture.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterTexture.h








## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERTEXTURE_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERTEXTURE_H_

#import <CoreMedia/CoreMedia.h>
#import <Foundation/Foundation.h>

#import "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN

FLUTTER_DARWIN_EXPORT
@protocol FlutterTexture <NSObject>
- (CVPixelBufferRef _Nullable)copyPixelBuffer;

@optional
- (void)onTextureUnregistered:(NSObject<FlutterTexture>*)texture;
@end

FLUTTER_DARWIN_EXPORT
@protocol FlutterTextureRegistry <NSObject>
- (int64_t)registerTexture:(NSObject<FlutterTexture>*)texture;
- (void)textureFrameAvailable:(int64_t)textureId;
- (void)unregisterTexture:(int64_t)textureId;
@end

NS_ASSUME_NONNULL_END

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERTEXTURE_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
