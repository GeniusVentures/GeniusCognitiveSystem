---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterChannels.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterChannels.h





## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[FlutterBasicMessageChannel](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/)**  |
| class | **[FlutterMethodChannel](/source-reference/Classes/da/d6e/interface_flutter_method_channel/)**  |

## Types

|                | Name           |
| -------------- | -------------- |
| typedef void(^)(id _Nullable message, FlutterReply callback) | **[FlutterMessageHandler](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-fluttermessagehandler)**  |
| typedef void(^)(id _Nullable result) | **[FlutterResult](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-flutterresult)**  |
| typedef void(^)(FlutterMethodCall *call, FlutterResult result) | **[FlutterMethodCallHandler](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-fluttermethodcallhandler)**  |
| typedef void(^)(id _Nullable event) | **[FlutterEventSink](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-fluttereventsink)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| [NS_ASSUME_NONNULL_BEGIN](/source-reference/Files/df/d02/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-ns_assume_nonnull_begin) typedef void(^)(id _Nullable reply) | **[FlutterReply](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#variable-flutterreply)**  |
| [FLUTTER_DARWIN_EXPORT](/source-reference/Files/df/d02/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-flutter_darwin_export) NSObject const  * | **[FlutterMethodNotImplemented](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#variable-fluttermethodnotimplemented)**  |
| [FLUTTER_DARWIN_EXPORT](/source-reference/Files/df/d02/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-flutter_darwin_export) NSObject const  * | **[FlutterEndOfEventStream](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#variable-flutterendofeventstream)**  |

## Types Documentation

### typedef FlutterMessageHandler

```cpp
typedef void(^ FlutterMessageHandler) (id _Nullable message, FlutterReply callback);
```


**Parameters**: 

  * **message** The message. 
  * **callback** A callback for submitting a reply to the sender which can be invoked from any thread. 


A strategy for handling incoming messages from Flutter and to send asynchronous replies back to Flutter.


### typedef FlutterResult

```cpp
typedef void(^ FlutterResult) (id _Nullable result);
```


**Parameters**: 

  * **result** The result. 


A method call result callback.

Used for submitting a method call result back to a Flutter caller. Also used in the dual capacity for handling a method call result received from Flutter.


### typedef FlutterMethodCallHandler

```cpp
typedef void(^ FlutterMethodCallHandler) (FlutterMethodCall *call, FlutterResult result);
```


**Parameters**: 

  * **call** The incoming method call. 
  * **result** A callback to asynchronously submit the result of the call. Invoke the callback with a [`FlutterError`](/source-reference/Classes/d0/da1/interface_flutter_error/) to indicate that the call failed. Invoke the callback with [`FlutterMethodNotImplemented`](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#variable-fluttermethodnotimplemented) to indicate that the method was unknown. Any other values, including `nil`, are interpreted as successful results. This can be invoked from any thread. 


A strategy for handling method calls.


### typedef FlutterEventSink

```cpp
typedef void(^ FlutterEventSink) (id _Nullable event);
```


**Parameters**: 

  * **event** The event. 


An event sink callback.




## Attributes Documentation

### variable FlutterReply

```cpp
NS_ASSUME_NONNULL_BEGIN typedef void(^)(id _Nullable reply) FlutterReply;
```


**Parameters**: 

  * **reply** The reply. 


A message reply callback.

Used for submitting a reply back to a Flutter message sender. Also used in the dual capacity for handling a message reply received from Flutter.


### variable FlutterMethodNotImplemented

```cpp
FLUTTER_DARWIN_EXPORT NSObject const  * FlutterMethodNotImplemented;
```


A constant used with [`FlutterMethodCallHandler`](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-fluttermethodcallhandler) to respond to the call of an unknown method. 


### variable FlutterEndOfEventStream

```cpp
FLUTTER_DARWIN_EXPORT NSObject const  * FlutterEndOfEventStream;
```


A constant used with [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/) to indicate end of stream. 



## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERCHANNELS_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERCHANNELS_H_

#import "FlutterBinaryMessenger.h"
#import "FlutterCodecs.h"

NS_ASSUME_NONNULL_BEGIN
typedef void (^FlutterReply)(id _Nullable reply);

typedef void (^FlutterMessageHandler)(id _Nullable message, FlutterReply callback);

FLUTTER_DARWIN_EXPORT
@interface FlutterBasicMessageChannel : NSObject
+ (instancetype)messageChannelWithName:(NSString*)name
                       binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger;

+ (instancetype)messageChannelWithName:(NSString*)name
                       binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                                 codec:(NSObject<FlutterMessageCodec>*)codec;

- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMessageCodec>*)codec;

- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMessageCodec>*)codec
                   taskQueue:(NSObject<FlutterTaskQueue>* _Nullable)taskQueue;

- (void)sendMessage:(id _Nullable)message;

- (void)sendMessage:(id _Nullable)message reply:(FlutterReply _Nullable)callback;

- (void)setMessageHandler:(FlutterMessageHandler _Nullable)handler;

+ (void)resizeChannelWithName:(NSString*)name
              binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                         size:(NSInteger)newSize;

- (void)resizeChannelBuffer:(NSInteger)newSize;

+ (void)setWarnsOnOverflow:(BOOL)warns
        forChannelWithName:(NSString*)name
           binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger;

- (void)setWarnsOnOverflow:(BOOL)warns;

@end

typedef void (^FlutterResult)(id _Nullable result);

typedef void (^FlutterMethodCallHandler)(FlutterMethodCall* call, FlutterResult result);

FLUTTER_DARWIN_EXPORT
extern NSObject const* FlutterMethodNotImplemented;

FLUTTER_DARWIN_EXPORT
@interface FlutterMethodChannel : NSObject
+ (instancetype)methodChannelWithName:(NSString*)name
                      binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger;

+ (instancetype)methodChannelWithName:(NSString*)name
                      binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                                codec:(NSObject<FlutterMethodCodec>*)codec;

- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMethodCodec>*)codec;

- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMethodCodec>*)codec
                   taskQueue:(NSObject<FlutterTaskQueue>* _Nullable)taskQueue;

// clang-format off
// clang-format on
- (void)invokeMethod:(NSString*)method arguments:(id _Nullable)arguments;

- (void)invokeMethod:(NSString*)method
           arguments:(id _Nullable)arguments
              result:(FlutterResult _Nullable)callback;
- (void)setMethodCallHandler:(FlutterMethodCallHandler _Nullable)handler;

- (void)resizeChannelBuffer:(NSInteger)newSize;

@end

typedef void (^FlutterEventSink)(id _Nullable event);

FLUTTER_DARWIN_EXPORT
@protocol FlutterStreamHandler
- (FlutterError* _Nullable)onListenWithArguments:(id _Nullable)arguments
                                       eventSink:(FlutterEventSink)events;

- (FlutterError* _Nullable)onCancelWithArguments:(id _Nullable)arguments;
@end

FLUTTER_DARWIN_EXPORT
extern NSObject const* FlutterEndOfEventStream;

FLUTTER_DARWIN_EXPORT
@interface FlutterEventChannel : NSObject
+ (instancetype)eventChannelWithName:(NSString*)name
                     binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger;

+ (instancetype)eventChannelWithName:(NSString*)name
                     binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                               codec:(NSObject<FlutterMethodCodec>*)codec;

- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMethodCodec>*)codec;

- (instancetype)initWithName:(NSString*)name
             binaryMessenger:(NSObject<FlutterBinaryMessenger>*)messenger
                       codec:(NSObject<FlutterMethodCodec>*)codec
                   taskQueue:(NSObject<FlutterTaskQueue>* _Nullable)taskQueue;
- (void)setStreamHandler:(NSObject<FlutterStreamHandler>* _Nullable)handler;
@end
NS_ASSUME_NONNULL_END

#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERCHANNELS_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
