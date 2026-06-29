---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterCodecs.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Release/FlutterMacOS.framework/Versions/A/Headers/FlutterCodecs.h





## Functions

|                | Name           |
| -------------- | -------------- |
| typedef | **[NS_ENUM](/source-reference/Files/d7/db3/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_codecs_8h/#function-ns_enum)**(NSInteger , FlutterStandardDataType ) |


## Functions Documentation

### function NS_ENUM

```cpp
typedef NS_ENUM(
    NSInteger ,
    FlutterStandardDataType 
)
```


Type of numeric data items encoded in a `FlutterStandardDataType`.



* FlutterStandardDataTypeUInt8: plain bytes
* FlutterStandardDataTypeInt32: 32-bit signed integers
* FlutterStandardDataTypeInt64: 64-bit signed integers
* FlutterStandardDataTypeFloat64: 64-bit floats 




## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERCODECS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERCODECS_H_

#import <Foundation/Foundation.h>

#import "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN

FLUTTER_DARWIN_EXPORT
@protocol FlutterMessageCodec
+ (instancetype)sharedInstance;

- (NSData* _Nullable)encode:(id _Nullable)message;

- (id _Nullable)decode:(NSData* _Nullable)message;
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterBinaryCodec : NSObject <FlutterMessageCodec>
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterStringCodec : NSObject <FlutterMessageCodec>
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterJSONMessageCodec : NSObject <FlutterMessageCodec>
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterStandardWriter : NSObject
- (instancetype)initWithData:(NSMutableData*)data;
- (void)writeByte:(UInt8)value;
- (void)writeBytes:(const void*)bytes length:(NSUInteger)length;
- (void)writeData:(NSData*)data;
- (void)writeSize:(UInt32)size;
- (void)writeAlignment:(UInt8)alignment;
- (void)writeUTF8:(NSString*)value;
- (void)writeValue:(id)value;
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterStandardReader : NSObject
- (instancetype)initWithData:(NSData*)data;
- (BOOL)hasMore;
- (UInt8)readByte;
- (void)readBytes:(void*)destination length:(NSUInteger)length;
- (NSData*)readData:(NSUInteger)length;
- (UInt32)readSize;
- (void)readAlignment:(UInt8)alignment;
- (NSString*)readUTF8;
- (nullable id)readValue;
- (nullable id)readValueOfType:(UInt8)type;
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterStandardReaderWriter : NSObject
- (FlutterStandardWriter*)writerWithData:(NSMutableData*)data;
- (FlutterStandardReader*)readerWithData:(NSData*)data;
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterStandardMessageCodec : NSObject <FlutterMessageCodec>
+ (instancetype)codecWithReaderWriter:(FlutterStandardReaderWriter*)readerWriter;
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterMethodCall : NSObject
+ (instancetype)methodCallWithMethodName:(NSString*)method arguments:(id _Nullable)arguments;

@property(readonly, nonatomic) NSString* method;

@property(readonly, nonatomic, nullable) id arguments;
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterError : NSObject
+ (instancetype)errorWithCode:(NSString*)code
                      message:(NSString* _Nullable)message
                      details:(id _Nullable)details;
@property(readonly, nonatomic) NSString* code;

@property(readonly, nonatomic, nullable) NSString* message;

@property(readonly, nonatomic, nullable) id details;
@end

typedef NS_ENUM(NSInteger, FlutterStandardDataType) {
  // NOLINTBEGIN(readability-identifier-naming)
  FlutterStandardDataTypeUInt8,
  FlutterStandardDataTypeInt32,
  FlutterStandardDataTypeInt64,
  FlutterStandardDataTypeFloat32,
  FlutterStandardDataTypeFloat64,
  // NOLINTEND(readability-identifier-naming)
};

FLUTTER_DARWIN_EXPORT
@interface FlutterStandardTypedData : NSObject
+ (instancetype)typedDataWithBytes:(NSData*)data;

+ (instancetype)typedDataWithInt32:(NSData*)data;

+ (instancetype)typedDataWithInt64:(NSData*)data;

+ (instancetype)typedDataWithFloat32:(NSData*)data;

+ (instancetype)typedDataWithFloat64:(NSData*)data;

@property(readonly, nonatomic) NSData* data;

@property(readonly, nonatomic, assign) FlutterStandardDataType type;

@property(readonly, nonatomic, assign) UInt32 elementCount;

@property(readonly, nonatomic, assign) UInt8 elementSize;
@end

FLUTTER_DARWIN_EXPORT
FLUTTER_UNAVAILABLE("Unavailable on 2018-08-31. Deprecated on 2018-01-09. "
                    "FlutterStandardBigInteger was needed because the Dart 1.0 int type had no "
                    "size limit. With Dart 2.0, the int type is a fixed-size, 64-bit signed "
                    "integer. If you need to communicate larger integers, use NSString encoding "
                    "instead.")
@interface FlutterStandardBigInteger : NSObject
@end

FLUTTER_DARWIN_EXPORT
@protocol FlutterMethodCodec
+ (instancetype)sharedInstance;

- (NSData*)encodeMethodCall:(FlutterMethodCall*)methodCall;

- (FlutterMethodCall*)decodeMethodCall:(NSData*)methodCall;

- (NSData*)encodeSuccessEnvelope:(id _Nullable)result;

- (NSData*)encodeErrorEnvelope:(FlutterError*)error;

- (id _Nullable)decodeEnvelope:(NSData*)envelope;
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterJSONMethodCodec : NSObject <FlutterMethodCodec>
@end

FLUTTER_DARWIN_EXPORT
@interface FlutterStandardMethodCodec : NSObject <FlutterMethodCodec>
+ (instancetype)codecWithReaderWriter:(FlutterStandardReaderWriter*)readerWriter;
@end

NS_ASSUME_NONNULL_END

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERCODECS_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
