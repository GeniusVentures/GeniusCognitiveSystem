---
title: FlutterJSONMethodCodec

---

# FlutterJSONMethodCodec



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, <FlutterMethodCodec>, NSObject, <FlutterMethodCodec>

## Detailed Description

```objective-c
class FlutterJSONMethodCodec;
```


**Parameters**: 

  * **methodCall** The method call. The arguments value must be supported by this codec. 
  * **methodCall** The method call to decode. 
  * **result** The result. Must be a value supported by this codec. 
  * **error** The error object. The error details value must be supported by this codec. 
  * **envelope** The error object. 


**Return**: 

  * The shared instance. Encodes the specified method call into binary.
  * The binary encoding. Decodes the specified method call from binary.
  * The decoded method call. Encodes the specified successful result into binary.
  * The binary encoding. Encodes the specified error result into binary.
  * The binary encoding. Deccodes the specified result envelope from binary.
  * The result value, if the envelope represented a successful result, or a [`FlutterError`](/source-reference/Classes/d0/da1/interface_flutter_error/) instance, if not. A [`FlutterMethodCodec`] using UTF-8 encoded JSON method calls and result envelopes.


An arbitrarily large integer value, used with [`FlutterStandardMessageCodec`](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/) and [`FlutterStandardMethodCodec`](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/). A codec for method calls and enveloped results.

Method calls are encoded as binary messages with enough structure that the codec can extract a method name `NSString` and an arguments `NSObject`, possibly `nil`. These data items are used to populate a [`FlutterMethodCall`](/source-reference/Classes/d4/d81/interface_flutter_method_call/).

Result envelopes are encoded as binary messages with enough structure that the codec can determine whether the result was successful or an error. In the former case, the codec can extract the result `NSObject`, possibly `nil`. In the latter case, the codec can extract an error code `NSString`, a human-readable `NSString` error message (possibly `nil`), and a custom error details `NSObject`, possibly `nil`. These data items are used to populate a [`FlutterError`](/source-reference/Classes/d0/da1/interface_flutter_error/). Provides access to a shared instance this codec.


This codec is guaranteed to be compatible with the corresponding [JSONMethodCodec](https://api.flutter.dev/flutter/services/JSONMethodCodec-class.html) on the Dart side. These parts of the Flutter SDK are evolved synchronously.

Values supported as methods arguments and result payloads are those supported as top-level or leaf values by [`FlutterJSONMessageCodec`](/source-reference/Classes/d6/daa/interface_flutter_j_s_o_n_message_codec/). 

-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700