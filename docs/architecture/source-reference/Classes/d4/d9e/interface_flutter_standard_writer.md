---
title: FlutterStandardWriter

---

# FlutterStandardWriter



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[initWithData:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-initwithdata:)**(NSMutableData * data) |
| virtual void | **[writeByte:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writebyte:)**(UInt8 value) |
| virtual void | **[writeBytes:length:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writebytes:length:)**(const void * bytes, NSUInteger length) |
| virtual void | **[writeData:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writedata:)**(NSData * data) |
| virtual void | **[writeSize:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writesize:)**(UInt32 size) |
| virtual void | **[writeAlignment:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writealignment:)**(UInt8 alignment) |
| virtual void | **[writeUTF8:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writeutf8:)**(NSString * value) |
| virtual void | **[writeValue:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writevalue:)**(id value) |
| virtual instancetype | **[initWithData:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-initwithdata:)**(NSMutableData * data) |
| virtual void | **[writeByte:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writebyte:)**(UInt8 value) |
| virtual void | **[writeBytes:length:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writebytes:length:)**(const void * bytes, NSUInteger length) |
| virtual void | **[writeData:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writedata:)**(NSData * data) |
| virtual void | **[writeSize:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writesize:)**(UInt32 size) |
| virtual void | **[writeAlignment:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writealignment:)**(UInt8 alignment) |
| virtual void | **[writeUTF8:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writeutf8:)**(NSString * value) |
| virtual void | **[writeValue:](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/#function-writevalue:)**(id value) |

## Detailed Description

```objective-c
class FlutterStandardWriter;
```


A writer of the Flutter standard binary encoding.

See [`FlutterStandardMessageCodec`](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/) for details on the encoding.

The encoding is extensible via subclasses overriding `writeValue`. 

## Public Functions Documentation

### function initWithData:

```objective-c
virtual instancetype initWithData:(
    NSMutableData * data
)
```


Create a [`FlutterStandardWriter`](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) who will write to `data`. 


### function writeByte:

```objective-c
virtual void writeByte:(
    UInt8 value
)
```


Write a 8-bit byte. 


### function writeBytes:length:

```objective-c
virtual void writeBytes:length:(
    const void * bytes,
    NSUInteger length
)
```


Write an array of `bytes` of size `length`. 


### function writeData:

```objective-c
virtual void writeData:(
    NSData * data
)
```


Write an array of bytes contained in `data`. 


### function writeSize:

```objective-c
virtual void writeSize:(
    UInt32 size
)
```


Write 32-bit unsigned integer that represents a `size` of a collection. 


### function writeAlignment:

```objective-c
virtual void writeAlignment:(
    UInt8 alignment
)
```


Write zero padding until data is aligned with `alignment`. 


### function writeUTF8:

```objective-c
virtual void writeUTF8:(
    NSString * value
)
```


Write a string with UTF-8 encoding. 


### function writeValue:

```objective-c
virtual void writeValue:(
    id value
)
```


Introspects into an object and writes its representation.

Supported Data Types:

* NSNull
* NSNumber
* NSString (as UTF-8)
* [FlutterStandardTypedData](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/)
* NSArray of supported types
* NSDictionary of supporte types

NSAsserts on failure. 


### function initWithData:

```objective-c
virtual instancetype initWithData:(
    NSMutableData * data
)
```


Create a [`FlutterStandardWriter`](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) who will write to `data`. 


### function writeByte:

```objective-c
virtual void writeByte:(
    UInt8 value
)
```


Write a 8-bit byte. 


### function writeBytes:length:

```objective-c
virtual void writeBytes:length:(
    const void * bytes,
    NSUInteger length
)
```


Write an array of `bytes` of size `length`. 


### function writeData:

```objective-c
virtual void writeData:(
    NSData * data
)
```


Write an array of bytes contained in `data`. 


### function writeSize:

```objective-c
virtual void writeSize:(
    UInt32 size
)
```


Write 32-bit unsigned integer that represents a `size` of a collection. 


### function writeAlignment:

```objective-c
virtual void writeAlignment:(
    UInt8 alignment
)
```


Write zero padding until data is aligned with `alignment`. 


### function writeUTF8:

```objective-c
virtual void writeUTF8:(
    NSString * value
)
```


Write a string with UTF-8 encoding. 


### function writeValue:

```objective-c
virtual void writeValue:(
    id value
)
```


Introspects into an object and writes its representation.

Supported Data Types:

* NSNull
* NSNumber
* NSString (as UTF-8)
* [FlutterStandardTypedData](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/)
* NSArray of supported types
* NSDictionary of supporte types

NSAsserts on failure. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700