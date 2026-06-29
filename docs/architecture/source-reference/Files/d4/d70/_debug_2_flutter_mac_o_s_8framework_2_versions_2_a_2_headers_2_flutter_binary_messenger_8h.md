---
title: GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterBinaryMessenger.h

---

# GNUS-NEO-SWARM/ui/build/macos/Build/Products/Debug/FlutterMacOS.framework/Versions/A/Headers/FlutterBinaryMessenger.h





## Types

|                | Name           |
| -------------- | -------------- |
| typedef void(^)(NSData *_Nullable message, FlutterBinaryReply reply) | **[FlutterBinaryMessageHandler](/source-reference/Files/d4/d70/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_binary_messenger_8h/#typedef-flutterbinarymessagehandler)**  |
| typedef int64_t | **[FlutterBinaryMessengerConnection](/source-reference/Files/d4/d70/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_binary_messenger_8h/#typedef-flutterbinarymessengerconnection)**  |

## Attributes

|                | Name           |
| -------------- | -------------- |
| [NS_ASSUME_NONNULL_BEGIN](/source-reference/Files/df/d02/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_macros_8h/#define-ns_assume_nonnull_begin) typedef void(^)(NSData *_Nullable reply) | **[FlutterBinaryReply](/source-reference/Files/d4/d70/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_binary_messenger_8h/#variable-flutterbinaryreply)**  |

## Types Documentation

### typedef FlutterBinaryMessageHandler

```cpp
typedef void(^ FlutterBinaryMessageHandler) (NSData *_Nullable message, FlutterBinaryReply reply);
```


**Parameters**: 

  * **message** The message. 
  * **reply** A callback for submitting an asynchronous reply to the sender. 


A strategy for handling incoming binary messages from Flutter and to send asynchronous replies back to Flutter.


### typedef FlutterBinaryMessengerConnection

```cpp
typedef int64_t FlutterBinaryMessengerConnection;
```




## Attributes Documentation

### variable FlutterBinaryReply

```cpp
NS_ASSUME_NONNULL_BEGIN typedef void(^)(NSData *_Nullable reply) FlutterBinaryReply;
```


**Parameters**: 

  * **reply** The reply. 


A message reply callback.

Used for submitting a binary reply back to a Flutter message sender. Also used in for handling a binary message reply received from Flutter.



## Source code

```cpp
// Copyright 2013 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERBINARYMESSENGER_H_
#define FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERBINARYMESSENGER_H_

#import <Foundation/Foundation.h>

#import "FlutterMacros.h"

NS_ASSUME_NONNULL_BEGIN
typedef void (^FlutterBinaryReply)(NSData* _Nullable reply);

typedef void (^FlutterBinaryMessageHandler)(NSData* _Nullable message, FlutterBinaryReply reply);

typedef int64_t FlutterBinaryMessengerConnection;

@protocol FlutterTaskQueue <NSObject>
@end

FLUTTER_DARWIN_EXPORT
@protocol FlutterBinaryMessenger <NSObject>
@optional
- (NSObject<FlutterTaskQueue>*)makeBackgroundTaskQueue;

- (FlutterBinaryMessengerConnection)
    setMessageHandlerOnChannel:(NSString*)channel
          binaryMessageHandler:(FlutterBinaryMessageHandler _Nullable)handler
                     taskQueue:(NSObject<FlutterTaskQueue>* _Nullable)taskQueue;

@required
- (void)sendOnChannel:(NSString*)channel message:(NSData* _Nullable)message;

- (void)sendOnChannel:(NSString*)channel
              message:(NSData* _Nullable)message
          binaryReply:(FlutterBinaryReply _Nullable)callback;

- (FlutterBinaryMessengerConnection)setMessageHandlerOnChannel:(NSString*)channel
                                          binaryMessageHandler:
                                              (FlutterBinaryMessageHandler _Nullable)handler;

- (void)cleanUpConnection:(FlutterBinaryMessengerConnection)connection;
@end
NS_ASSUME_NONNULL_END
#endif  // FLUTTER_SHELL_PLATFORM_DARWIN_COMMON_FRAMEWORK_HEADERS_FLUTTERBINARYMESSENGER_H_
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700
