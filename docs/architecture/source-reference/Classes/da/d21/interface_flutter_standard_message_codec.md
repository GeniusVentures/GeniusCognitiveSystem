---
title: FlutterStandardMessageCodec

---

# FlutterStandardMessageCodec



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, <FlutterMessageCodec>, NSObject, <FlutterMessageCodec>

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[codecWithReaderWriter:](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/#function-codecwithreaderwriter:)**([FlutterStandardReaderWriter](/source-reference/Classes/da/d07/interface_flutter_standard_reader_writer/) * readerWriter) |
| virtual instancetype | **[codecWithReaderWriter:](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/#function-codecwithreaderwriter:)**([FlutterStandardReaderWriter](/source-reference/Classes/da/d07/interface_flutter_standard_reader_writer/) * readerWriter) |

## Detailed Description

```objective-c
class FlutterStandardMessageCodec;
```


A [`FlutterMessageCodec`] using the Flutter standard binary encoding.

This codec is guaranteed to be compatible with the corresponding [StandardMessageCodec](https://api.flutter.dev/flutter/services/StandardMessageCodec-class.html) on the Dart side. These parts of the Flutter SDK are evolved synchronously.

Supported messages are acyclic values of these forms:



* `nil` or `NSNull`
* `NSNumber` (including their representation of Boolean values)
* `NSString`
* [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/)
* `NSArray` of supported values
* `NSDictionary` with supported keys and values

On the Dart side, these values are represented as follows:



* `nil` or `NSNull`: null
* `NSNumber`: `bool`, `int`, or `double`, depending on the contained value.
* `NSString`: `String`
* [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/): `Uint8List`, `Int32List`, `Int64List`, or `Float64List`
* `NSArray`: `List`
* `NSDictionary`: `Map`

## Public Functions Documentation

### function codecWithReaderWriter:

```objective-c
static virtual instancetype codecWithReaderWriter:(
    FlutterStandardReaderWriter * readerWriter
)
```


Create a [`FlutterStandardMessageCodec`](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/) who will read and write to `readerWriter`. 


### function codecWithReaderWriter:

```objective-c
static virtual instancetype codecWithReaderWriter:(
    FlutterStandardReaderWriter * readerWriter
)
```


Create a [`FlutterStandardMessageCodec`](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/) who will read and write to `readerWriter`. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700