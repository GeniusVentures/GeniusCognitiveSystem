---
title: FlutterMethodChannel

---

# FlutterMethodChannel



 [More...](#detailed-description)


`#include <FlutterChannels.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[methodChannelWithName:binaryMessenger:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-methodchannelwithname:binarymessenger:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger) |
| virtual instancetype | **[methodChannelWithName:binaryMessenger:codec:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-methodchannelwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec) |
| virtual instancetype | **[methodChannelWithName:binaryMessenger:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-methodchannelwithname:binarymessenger:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger) |
| virtual instancetype | **[methodChannelWithName:binaryMessenger:codec:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-methodchannelwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-initwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:taskQueue:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-initwithname:binarymessenger:codec:taskqueue:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec, NSObject< [FlutterTaskQueue] > *_Nullable taskQueue) |
| virtual void | **[invokeMethod:arguments:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-invokemethod:arguments:)**(NSString * method, id _Nullable arguments) |
| virtual void | **[invokeMethod:arguments:result:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-invokemethod:arguments:result:)**(NSString * method, id _Nullable arguments, [FlutterResult](/source-reference/Files/d6/d84/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-flutterresult) _Nullable callback) |
| virtual void | **[setMethodCallHandler:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-setmethodcallhandler:)**([FlutterMethodCallHandler](/source-reference/Files/d6/d84/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-fluttermethodcallhandler) _Nullable handler) |
| virtual void | **[resizeChannelBuffer:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-resizechannelbuffer:)**(NSInteger newSize) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-initwithname:binarymessenger:codec:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec) |
| virtual instancetype | **[initWithName:binaryMessenger:codec:taskQueue:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-initwithname:binarymessenger:codec:taskqueue:)**(NSString * name, NSObject< [FlutterBinaryMessenger] > * messenger, NSObject< FlutterMethodCodec > * codec, NSObject< [FlutterTaskQueue] > *_Nullable taskQueue) |
| virtual void | **[invokeMethod:arguments:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-invokemethod:arguments:)**(NSString * method, id _Nullable arguments) |
| virtual void | **[invokeMethod:arguments:result:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-invokemethod:arguments:result:)**(NSString * method, id _Nullable arguments, [FlutterResult](/source-reference/Files/d6/d84/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-flutterresult) _Nullable callback) |
| virtual void | **[setMethodCallHandler:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-setmethodcallhandler:)**([FlutterMethodCallHandler](/source-reference/Files/d6/d84/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#typedef-fluttermethodcallhandler) _Nullable handler) |
| virtual void | **[resizeChannelBuffer:](/source-reference/Classes/da/d6e/interface_flutter_method_channel/#function-resizechannelbuffer:)**(NSInteger newSize) |

## Detailed Description

```objective-c
class FlutterMethodChannel;
```


A channel for communicating with the Flutter side using invocation of asynchronous methods. 

## Public Functions Documentation

### function methodChannelWithName:binaryMessenger:

```objective-c
static virtual instancetype methodChannelWithName:binaryMessenger:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 


Creates a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/) with the specified name and binary messenger.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).

The channel uses [`FlutterStandardMethodCodec`](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/) to encode and decode method calls and result envelopes.


### function methodChannelWithName:binaryMessenger:codec:

```objective-c
static virtual instancetype methodChannelWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMethodCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The method codec. 


Creates a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/) with the specified name, binary messenger, and method codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function methodChannelWithName:binaryMessenger:

```objective-c
static virtual instancetype methodChannelWithName:binaryMessenger:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 


Creates a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/) with the specified name and binary messenger.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).

The channel uses [`FlutterStandardMethodCodec`](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/) to encode and decode method calls and result envelopes.


### function methodChannelWithName:binaryMessenger:codec:

```objective-c
static virtual instancetype methodChannelWithName:binaryMessenger:codec:(
    NSString * name,
    NSObject< FlutterBinaryMessenger > * messenger,
    NSObject< FlutterMethodCodec > * codec
)
```


**Parameters**: 

  * **name** The channel name. 
  * **messenger** The binary messenger. 
  * **codec** The method codec. 


Creates a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/) with the specified name, binary messenger, and method codec.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


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


Initializes a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/) with the specified name, binary messenger, and method codec.

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


Initializes a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/) with the specified name, binary messenger, method codec, and task queue.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function invokeMethod:arguments:

```objective-c
virtual void invokeMethod:arguments:(
    NSString * method,
    id _Nullable arguments
)
```


**Parameters**: 

  * **method** The name of the method to invoke. 
  * **arguments** The arguments. Must be a value supported by the codec of this channel. 


**See**: [MethodChannel.setMethodCallHandler](https://api.flutter.dev/flutter/services/MethodChannel/setMethodCallHandler.html)

Invokes the specified Flutter method with the specified arguments, expecting no results.


### function invokeMethod:arguments:result:

```objective-c
virtual void invokeMethod:arguments:result:(
    NSString * method,
    id _Nullable arguments,
    FlutterResult _Nullable callback
)
```


**Parameters**: 

  * **method** The name of the method to invoke. 
  * **arguments** The arguments. Must be a value supported by the codec of this channel. 
  * **callback** A callback that will be invoked with the asynchronous result. The result will be a [`FlutterError`](/source-reference/Classes/d0/da1/interface_flutter_error/) instance, if the method call resulted in an error on the Flutter side. Will be [`FlutterMethodNotImplemented`](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#variable-fluttermethodnotimplemented), if the method called was not implemented on the Flutter side. Any other value, including `nil`, should be interpreted as successful results. 


Invokes the specified Flutter method with the specified arguments, expecting an asynchronous result.


### function setMethodCallHandler:

```objective-c
virtual void setMethodCallHandler:(
    FlutterMethodCallHandler _Nullable handler
)
```


**Parameters**: 

  * **handler** The method call handler. 


Registers a handler for method calls from the Flutter side.

Replaces any existing handler. Use a `nil` handler for unregistering the existing handler.


### function resizeChannelBuffer:

```objective-c
virtual void resizeChannelBuffer:(
    NSInteger newSize
)
```


Adjusts the number of messages that will get buffered when sending messages to channels that aren't fully set up yet. For example, the engine isn't running yet or the channel's message handler isn't set up on the Dart side yet. 


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


Initializes a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/) with the specified name, binary messenger, and method codec.

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


Initializes a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/) with the specified name, binary messenger, method codec, and task queue.

The channel name logically identifies the channel; identically named channels interfere with each other's communication.

The binary messenger is a facility for sending raw, binary messages to the Flutter side. This protocol is implemented by [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) and [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/).


### function invokeMethod:arguments:

```objective-c
virtual void invokeMethod:arguments:(
    NSString * method,
    id _Nullable arguments
)
```


**Parameters**: 

  * **method** The name of the method to invoke. 
  * **arguments** The arguments. Must be a value supported by the codec of this channel. 


**See**: [MethodChannel.setMethodCallHandler](https://api.flutter.dev/flutter/services/MethodChannel/setMethodCallHandler.html)

Invokes the specified Flutter method with the specified arguments, expecting no results.


### function invokeMethod:arguments:result:

```objective-c
virtual void invokeMethod:arguments:result:(
    NSString * method,
    id _Nullable arguments,
    FlutterResult _Nullable callback
)
```


**Parameters**: 

  * **method** The name of the method to invoke. 
  * **arguments** The arguments. Must be a value supported by the codec of this channel. 
  * **callback** A callback that will be invoked with the asynchronous result. The result will be a [`FlutterError`](/source-reference/Classes/d0/da1/interface_flutter_error/) instance, if the method call resulted in an error on the Flutter side. Will be [`FlutterMethodNotImplemented`](/source-reference/Files/d2/d37/_debug_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_channels_8h/#variable-fluttermethodnotimplemented), if the method called was not implemented on the Flutter side. Any other value, including `nil`, should be interpreted as successful results. 


Invokes the specified Flutter method with the specified arguments, expecting an asynchronous result.


### function setMethodCallHandler:

```objective-c
virtual void setMethodCallHandler:(
    FlutterMethodCallHandler _Nullable handler
)
```


**Parameters**: 

  * **handler** The method call handler. 


Registers a handler for method calls from the Flutter side.

Replaces any existing handler. Use a `nil` handler for unregistering the existing handler.


### function resizeChannelBuffer:

```objective-c
virtual void resizeChannelBuffer:(
    NSInteger newSize
)
```


Adjusts the number of messages that will get buffered when sending messages to channels that aren't fully set up yet. For example, the engine isn't running yet or the channel's message handler isn't set up on the Dart side yet. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700