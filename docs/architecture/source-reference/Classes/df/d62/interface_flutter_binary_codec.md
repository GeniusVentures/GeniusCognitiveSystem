---
title: FlutterBinaryCodec

---

# FlutterBinaryCodec



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, <FlutterMessageCodec>, NSObject, <FlutterMessageCodec>

## Detailed Description

```objective-c
class FlutterBinaryCodec;
```


A [`FlutterMessageCodec`] using unencoded binary messages, represented as `NSData` instances.

This codec is guaranteed to be compatible with the corresponding [BinaryCodec](https://api.flutter.dev/flutter/services/BinaryCodec-class.html) on the Dart side. These parts of the Flutter SDK are evolved synchronously.

On the Dart side, messages are represented using `ByteData`. 

-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700