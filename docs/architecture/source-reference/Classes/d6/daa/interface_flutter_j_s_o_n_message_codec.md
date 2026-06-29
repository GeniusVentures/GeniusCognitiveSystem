---
title: FlutterJSONMessageCodec

---

# FlutterJSONMessageCodec



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, <FlutterMessageCodec>, NSObject, <FlutterMessageCodec>

## Detailed Description

```objective-c
class FlutterJSONMessageCodec;
```


A [`FlutterMessageCodec`] using UTF-8 encoded JSON messages.

This codec is guaranteed to be compatible with the corresponding [JSONMessageCodec](https://api.flutter.dev/flutter/services/JSONMessageCodec-class.html) on the Dart side. These parts of the Flutter SDK are evolved synchronously.

Supports values accepted by `NSJSONSerialization` plus top-level `nil`, `NSNumber`, and `NSString`.

On the Dart side, JSON messages are handled by the JSON facilities of the [`dart:convert`](https://api.dartlang.org/stable/dart-convert/JSON-constant.html) package. 

-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700