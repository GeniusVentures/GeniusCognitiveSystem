---
title: FlutterBasicMessageChannel

---

# FlutterBasicMessageChannel



 [More...](#detailed-description)


`#include <FlutterChannels.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[messageChannelWithName:binaryMessenger:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-messagechannelwithname:binarymessenger:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger) |
| virtual instancetype | **[messageChannelWithName:binaryMessenger:codec:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-messagechannelwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< [FlutterMessageCodec] > * codec) |
| virtual void | **[resizeChannelWithName:binaryMessenger:size:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-resizechannelwithname:binarymessenger:size:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSInteger newSize) |
| virtual void | **[setWarnsOnOverflow:forChannelWithName:binaryMessenger:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-setwarnsonoverflow:forchannelwithname:binarymessenger:)**(BOOL warns, NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger) |
| virtual instancetype | **[messageChannelWithName:binaryMessenger:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-messagechannelwithname:binarymessenger:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger) |
| virtual instancetype | **[messageChannelWithName:binaryMessenger:codec:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-messagechannelwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< [FlutterMessageCodec] > * codec) |
| virtual void | **[resizeChannelWithName:binaryMessenger:size:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-resizechannelwithname:binarymessenger:size:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSInteger newSize) |
| virtual void | **[setWarnsOnOverflow:forChannelWithName:binaryMessenger:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-setwarnsonoverflow:forchannelwithname:binarymessenger:)**(BOOL warns, NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-initwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< [FlutterMessageCodec] > * codec) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:taskQueue:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-initwithname:binarymessenger:codec:taskqueue:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< [FlutterMessageCodec] > * codec, NSObject< [FlutterTaskQueue] > *_Nullable taskQueue) |
| virtual void | **[sendMessage:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-sendmessage:)**(id _Nullable message) |
| virtual void | **[sendMessage:reply:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-sendmessage:reply:)**(id _Nullable message, [FlutterReply](/source-reference/Files/d6/d84/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#variable-flutterreply) _Nullable callback) |
| virtual void | **[setMessageHandler:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-setmessagehandler:)**([FlutterMessageHandler](/source-reference/Files/d6/d84/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-fluttermessagehandler) _Nullable handler) |
| virtual void | **[resizeChannelBuffer:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-resizechannelbuffer:)**(NSInteger newSize) |
| virtual void | **[setWarnsOnOverflow:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-setwarnsonoverflow:)**(BOOL warns) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-initwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< [FlutterMessageCodec] > * codec) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:taskQueue:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-initwithname:binarymessenger:codec:taskqueue:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< [FlutterMessageCodec] > * codec, NSObject< [FlutterTaskQueue] > *_Nullable taskQueue) |
| virtual void | **[sendMessage:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-sendmessage:)**(id _Nullable message) |
| virtual void | **[sendMessage:reply:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-sendmessage:reply:)**(id _Nullable message, [FlutterReply](/source-reference/Files/d6/d84/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#variable-flutterreply) _Nullable callback) |
| virtual void | **[setMessageHandler:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-setmessagehandler:)**([FlutterMessageHandler](/source-reference/Files/d6/d84/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-fluttermessagehandler) _Nullable handler) |
| virtual void | **[resizeChannelBuffer:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-resizechannelbuffer:)**(NSInteger newSize) |
| virtual void | **[setWarnsOnOverflow:](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/#function-setwarnsonoverflow:)**(BOOL warns) |

## Detailed Description

```objective-c
class FlutterBasicMessageChannel;
```


A channel for communicating with the Flutter side using basic, asynchronous message passing. 

## Public Functions Documentation

### function messageChannelWithName:binaryMessenger:

```objective-c
static virtual instancetype messageChannelWithName:binaryMessenger:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 


Creates a [`FlutterBasicMessageChannel`](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/) with the specified name and binary messenger.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).

The channel uses [`FlutterStandardMessageCodec`](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/) to encode and decode messages.


### function messageChannelWithName:binaryMessenger:codec:

```objective-c
static virtual instancetype messageChannelWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMessageCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The message codec. 


Creates a [`FlutterBasicMessageChannel`](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/) with the specified name, binary messenger, and message codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function resizeChannelWithName:binaryMessenger:size:

```objective-c
static virtual void resizeChannelWithName:binaryMessenger:size:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSInteger newSize
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **newSize** The number of messages that will get buffered. 


Adjusts the number of messages that will get buffered when sending messages to channels that aren't fully set up yet. For example, the engine isn't running yet or the channel's message handler isn't set up on the Dart side yet.


### function setWarnsOnOverflow:forChannelWithName:binaryMessenger:

```objective-c
static virtual void setWarnsOnOverflow:forChannelWithName:binaryMessenger:(
    BOOL warns,
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger
)
```


**Parameters**: 

  * **warns** When false, the channel is expected to overflow and warning messages will not be shown. 
  * **name** The channel name. 
  * **messenger** The binary messenger. 


Defines whether the channel should show warning messages when discarding messages due to overflow.


### function messageChannelWithName:binaryMessenger:

```objective-c
static virtual instancetype messageChannelWithName:binaryMessenger:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 


Creates a [`FlutterBasicMessageChannel`](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/) with the specified name and binary messenger.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).

The channel uses [`FlutterStandardMessageCodec`](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/) to encode and decode messages.


### function messageChannelWithName:binaryMessenger:codec:

```objective-c
static virtual instancetype messageChannelWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMessageCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The message codec. 


Creates a [`FlutterBasicMessageChannel`](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/) with the specified name, binary messenger, and message codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function resizeChannelWithName:binaryMessenger:size:

```objective-c
static virtual void resizeChannelWithName:binaryMessenger:size:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSInteger newSize
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **newSize** The number of messages that will get buffered. 


Adjusts the number of messages that will get buffered when sending messages to channels that aren't fully set up yet. For example, the engine isn't running yet or the channel's message handler isn't set up on the Dart side yet.


### function setWarnsOnOverflow:forChannelWithName:binaryMessenger:

```objective-c
static virtual void setWarnsOnOverflow:forChannelWithName:binaryMessenger:(
    BOOL warns,
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger
)
```


**Parameters**: 

  * **warns** When false, the channel is expected to overflow and warning messages will not be shown. 
  * **name** The channel name. 
  * **messenger** The binary messenger. 


Defines whether the channel should show warning messages when discarding messages due to overflow.


### function initWithName:binaryMessenger:codec:

```objective-c
virtual instancetype initWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMessageCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The message codec. 


Initializes a [`FlutterBasicMessageChannel`](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/) with the specified name, binary messenger, and message codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function initWithName:binaryMessenger:codec:taskQueue:

```objective-c
virtual instancetype initWithName:binaryMessenger:codec:taskQueue:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMessageCodec > * codec,
    NSObject< FlutterTaskQueue > *_Nullable taskQueue
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The message codec. 
  * **taskQueue** The [FlutterTaskQueue] that executes the handler (see -[[FlutterBinaryMessenger] makeBackgroundTaskQueue]). 


Initializes a [`FlutterBasicMessageChannel`](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/) with the specified name, binary messenger, and message codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function sendMessage:

```objective-c
virtual void sendMessage:(
    id _Nullable message
)
```


**Parameters**: 

  * **message** The message. Must be supported by the codec of this channel. 


Sends the specified message to the Flutter side, ignoring any reply.


### function sendMessage:reply:

```objective-c
virtual void sendMessage:reply:(
    id _Nullable message,
    FlutterReply _Nullable callback
)
```


**Parameters**: 

  * **message** The message. Must be supported by the codec of this channel. 
  * **callback** A callback to be invoked with the message reply from Flutter. 


Sends the specified message to the Flutter side, expecting an asynchronous reply.


### function setMessageHandler:

```objective-c
virtual void setMessageHandler:(
    FlutterMessageHandler _Nullable handler
)
```


**Parameters**: 

  * **handler** The message handler. 


Registers a message handler with this channel.

Replaces any existing handler. Use a `nil` handler for unregistering the existing handler.


### function resizeChannelBuffer:

```objective-c
virtual void resizeChannelBuffer:(
    NSInteger newSize
)
```


**Parameters**: 

  * **newSize** The number of messages that will get buffered. 


Adjusts the number of messages that will get buffered when sending messages to channels that aren't fully set up yet. For example, the engine isn't running yet or the channel's message handler isn't set up on the Dart side yet.


### function setWarnsOnOverflow:

```objective-c
virtual void setWarnsOnOverflow:(
    BOOL warns
)
```


**Parameters**: 

  * **warns** When false, the channel is expected to overflow and warning messages will not be shown. 


Defines whether the channel should show warning messages when discarding messages due to overflow.


### function initWithName:binaryMessenger:codec:

```objective-c
virtual instancetype initWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMessageCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The message codec. 


Initializes a [`FlutterBasicMessageChannel`](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/) with the specified name, binary messenger, and message codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function initWithName:binaryMessenger:codec:taskQueue:

```objective-c
virtual instancetype initWithName:binaryMessenger:codec:taskQueue:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMessageCodec > * codec,
    NSObject< FlutterTaskQueue > *_Nullable taskQueue
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The message codec. 
  * **taskQueue** The [FlutterTaskQueue] that executes the handler (see -[[FlutterBinaryMessenger] makeBackgroundTaskQueue]). 


Initializes a [`FlutterBasicMessageChannel`](/source-reference/Classes/de/d2f/interface_flutter_basic_message_channel/) with the specified name, binary messenger, and message codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function sendMessage:

```objective-c
virtual void sendMessage:(
    id _Nullable message
)
```


**Parameters**: 

  * **message** The message. Must be supported by the codec of this channel. 


Sends the specified message to the Flutter side, ignoring any reply.


### function sendMessage:reply:

```objective-c
virtual void sendMessage:reply:(
    id _Nullable message,
    FlutterReply _Nullable callback
)
```


**Parameters**: 

  * **message** The message. Must be supported by the codec of this channel. 
  * **callback** A callback to be invoked with the message reply from Flutter. 


Sends the specified message to the Flutter side, expecting an asynchronous reply.


### function setMessageHandler:

```objective-c
virtual void setMessageHandler:(
    FlutterMessageHandler _Nullable handler
)
```


**Parameters**: 

  * **handler** The message handler. 


Registers a message handler with this channel.

Replaces any existing handler. Use a `nil` handler for unregistering the existing handler.


### function resizeChannelBuffer:

```objective-c
virtual void resizeChannelBuffer:(
    NSInteger newSize
)
```


**Parameters**: 

  * **newSize** The number of messages that will get buffered. 


Adjusts the number of messages that will get buffered when sending messages to channels that aren't fully set up yet. For example, the engine isn't running yet or the channel's message handler isn't set up on the Dart side yet.


### function setWarnsOnOverflow:

```objective-c
virtual void setWarnsOnOverflow:(
    BOOL warns
)
```


**Parameters**: 

  * **warns** When false, the channel is expected to overflow and warning messages will not be shown. 


Defines whether the channel should show warning messages when discarding messages due to overflow.


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700