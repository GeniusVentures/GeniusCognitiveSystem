---
title: FlutterStandardMethodCodec

---

# FlutterStandardMethodCodec



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, <FlutterMethodCodec>, NSObject, <FlutterMethodCodec>

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[codecWithReaderWriter:](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/#function-codecwithreaderwriter:)**([FlutterStandardReaderWriter](/source-reference/Classes/da/d07/interface_flutter_standard_reader_writer/) * readerWriter) |
| virtual instancetype | **[codecWithReaderWriter:](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/#function-codecwithreaderwriter:)**([FlutterStandardReaderWriter](/source-reference/Classes/da/d07/interface_flutter_standard_reader_writer/) * readerWriter) |

## Detailed Description

```objective-c
class FlutterStandardMethodCodec;
```


A [`FlutterMethodCodec`] using the Flutter standard binary encoding.

This codec is guaranteed to be compatible with the corresponding [StandardMethodCodec](https://api.flutter.dev/flutter/services/StandardMethodCodec-class.html) on the Dart side. These parts of the Flutter SDK are evolved synchronously.

Values supported as method arguments and result payloads are those supported by [`FlutterStandardMessageCodec`](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/). 

## Public Functions Documentation

### function codecWithReaderWriter:

```objective-c
static virtual instancetype codecWithReaderWriter:(
    FlutterStandardReaderWriter * readerWriter
)
```


Create a [`FlutterStandardMethodCodec`](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/) who will read and write to `readerWriter`. 


### function codecWithReaderWriter:

```objective-c
static virtual instancetype codecWithReaderWriter:(
    FlutterStandardReaderWriter * readerWriter
)
```


Create a [`FlutterStandardMethodCodec`](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/) who will read and write to `readerWriter`. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700