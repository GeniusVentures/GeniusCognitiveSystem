---
title: FlutterEventChannel

---

# FlutterEventChannel



 [More...](#detailed-description)


`#include <FlutterChannels.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[eventChannelWithName:binaryMessenger:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-eventchannelwithname:binarymessenger:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger) |
| virtual instancetype | **[eventChannelWithName:binaryMessenger:codec:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-eventchannelwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec) |
| virtual instancetype | **[eventChannelWithName:binaryMessenger:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-eventchannelwithname:binarymessenger:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger) |
| virtual instancetype | **[eventChannelWithName:binaryMessenger:codec:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-eventchannelwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-initwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:taskQueue:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-initwithname:binarymessenger:codec:taskqueue:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec, NSObject< [FlutterTaskQueue] > *_Nullable taskQueue) |
| virtual void | **[setStreamHandler:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-setstreamhandler:)**(NSObject< [FlutterStreamHandler] > *_Nullable handler) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-initwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:taskQueue:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-initwithname:binarymessenger:codec:taskqueue:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec, NSObject< [FlutterTaskQueue] > *_Nullable taskQueue) |
| virtual void | **[setStreamHandler:](/source-reference/Classes/dd/dda/interface_flutter_event_channel/#function-setstreamhandler:)**(NSObject< [FlutterStreamHandler] > *_Nullable handler) |

## Detailed Description

```objective-c
class FlutterEventChannel;
```


A channel for communicating with the Flutter side using event streams. 

## Public Functions Documentation

### function eventChannelWithName:binaryMessenger:

```objective-c
static virtual instancetype eventChannelWithName:binaryMessenger:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 


Creates a [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/) with the specified name and binary messenger.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).

The channel uses [`FlutterStandardMethodCodec`](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/) to decode stream setup and teardown requests, and to encode event envelopes.


### function eventChannelWithName:binaryMessenger:codec:

```objective-c
static virtual instancetype eventChannelWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMethodCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The method codec. 


Creates a [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/) with the specified name, binary messenger, and method codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function eventChannelWithName:binaryMessenger:

```objective-c
static virtual instancetype eventChannelWithName:binaryMessenger:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 


Creates a [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/) with the specified name and binary messenger.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).

The channel uses [`FlutterStandardMethodCodec`](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/) to decode stream setup and teardown requests, and to encode event envelopes.


### function eventChannelWithName:binaryMessenger:codec:

```objective-c
static virtual instancetype eventChannelWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMethodCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The method codec. 


Creates a [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/) with the specified name, binary messenger, and method codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function initWithName:binaryMessenger:codec:

```objective-c
virtual instancetype initWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMethodCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The method codec. 


Initializes a [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/) with the specified name, binary messenger, and method codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function initWithName:binaryMessenger:codec:taskQueue:

```objective-c
virtual instancetype initWithName:binaryMessenger:codec:taskQueue:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMethodCodec > * codec,
    NSObject< FlutterTaskQueue > *_Nullable taskQueue
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The method codec. 
  * **taskQueue** The [FlutterTaskQueue] that executes the handler (see -[[FlutterBinaryMessenger] makeBackgroundTaskQueue]). 


Initializes a [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/) with the specified name, binary messenger, method codec and task queue.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function setStreamHandler:

```objective-c
virtual void setStreamHandler:(
    NSObject< FlutterStreamHandler > *_Nullable handler
)
```


**Parameters**: 

  * **handler** The stream handler. 


Registers a handler for stream setup requests from the Flutter side.

Replaces any existing handler. Use a `nil` handler for unregistering the existing handler.


### function initWithName:binaryMessenger:codec:

```objective-c
virtual instancetype initWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMethodCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The method codec. 


Initializes a [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/) with the specified name, binary messenger, and method codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function initWithName:binaryMessenger:codec:taskQueue:

```objective-c
virtual instancetype initWithName:binaryMessenger:codec:taskQueue:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMethodCodec > * codec,
    NSObject< FlutterTaskQueue > *_Nullable taskQueue
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The method codec. 
  * **taskQueue** The [FlutterTaskQueue] that executes the handler (see -[[FlutterBinaryMessenger] makeBackgroundTaskQueue]). 


Initializes a [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/) with the specified name, binary messenger, method codec and task queue.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function setStreamHandler:

```objective-c
virtual void setStreamHandler:(
    NSObject< FlutterStreamHandler > *_Nullable handler
)
```


**Parameters**: 

  * **handler** The stream handler. 


Registers a handler for stream setup requests from the Flutter side.

Replaces any existing handler. Use a `nil` handler for unregistering the existing handler.


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700