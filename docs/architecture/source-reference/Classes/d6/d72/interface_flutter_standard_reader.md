---
title: FlutterStandardReader

---

# FlutterStandardReader



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[initWithData:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-initwithdata:)**(NSData * data) |
| virtual BOOL | **[hasMore](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-hasmore)**() |
| virtual UInt8 | **[readByte](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readbyte)**() |
| virtual void | **[readBytes:length:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readbytes:length:)**(void * destination, NSUInteger length) |
| virtual NSData * | **[readData:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readdata:)**(NSUInteger length) |
| virtual UInt32 | **[readSize](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readsize)**() |
| virtual void | **[readAlignment:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readalignment:)**(UInt8 alignment) |
| virtual NSString * | **[readUTF8](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readutf8)**() |
| virtual nullable id | **[readValue](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readvalue)**() |
| virtual nullable id | **[readValueOfType:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readvalueoftype:)**(UInt8 type) |
| virtual instancetype | **[initWithData:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-initwithdata:)**(NSData * data) |
| virtual BOOL | **[hasMore](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-hasmore)**() |
| virtual UInt8 | **[readByte](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readbyte)**() |
| virtual void | **[readBytes:length:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readbytes:length:)**(void * destination, NSUInteger length) |
| virtual NSData * | **[readData:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readdata:)**(NSUInteger length) |
| virtual UInt32 | **[readSize](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readsize)**() |
| virtual void | **[readAlignment:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readalignment:)**(UInt8 alignment) |
| virtual NSString * | **[readUTF8](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readutf8)**() |
| virtual nullable id | **[readValue](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readvalue)**() |
| virtual nullable id | **[readValueOfType:](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/#function-readvalueoftype:)**(UInt8 type) |

## Detailed Description

```objective-c
class FlutterStandardReader;
```


A reader of the Flutter standard binary encoding.

See [`FlutterStandardMessageCodec`](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/) for details on the encoding.

The encoding is extensible via subclasses overriding `readValueOfType`. 

## Public Functions Documentation

### function initWithData:

```objective-c
virtual instancetype initWithData:(
    NSData * data
)
```


Create a new [`FlutterStandardReader`](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/) who reads from `data`. 


### function hasMore

```objective-c
virtual BOOL hasMore()
```


Returns YES when the reader hasn't reached the end of its data. 


### function readByte

```objective-c
virtual UInt8 readByte()
```


Reads a byte value and increments the position. 


### function readBytes:length:

```objective-c
virtual void readBytes:length:(
    void * destination,
    NSUInteger length
)
```


Reads a sequence of byte values of `length` and increments the position. 


### function readData:

```objective-c
virtual NSData * readData:(
    NSUInteger length
)
```


Reads a sequence of byte values of `length` and increments the position. 


### function readSize

```objective-c
virtual UInt32 readSize()
```


Reads a 32-bit unsigned integer representing a collection size and increments the position. 


### function readAlignment:

```objective-c
virtual void readAlignment:(
    UInt8 alignment
)
```


Advances the read position until it is aligned with `alignment`. 


### function readUTF8

```objective-c
virtual NSString * readUTF8()
```


Read a null terminated string encoded with UTF-8/ 


### function readValue

```objective-c
virtual nullable id readValue()
```


Reads a byte for `FlutterStandardField` the decodes a value matching that type.

See also: -[[FlutterStandardWriter](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) writeValue] 


### function readValueOfType:

```objective-c
virtual nullable id readValueOfType:(
    UInt8 type
)
```


Decodes a value matching the `type` specified.

See also:

* `FlutterStandardField`
* `-[[FlutterStandardWriter](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) writeValue]`


### function initWithData:

```objective-c
virtual instancetype initWithData:(
    NSData * data
)
```


Create a new [`FlutterStandardReader`](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/) who reads from `data`. 


### function hasMore

```objective-c
virtual BOOL hasMore()
```


Returns YES when the reader hasn't reached the end of its data. 


### function readByte

```objective-c
virtual UInt8 readByte()
```


Reads a byte value and increments the position. 


### function readBytes:length:

```objective-c
virtual void readBytes:length:(
    void * destination,
    NSUInteger length
)
```


Reads a sequence of byte values of `length` and increments the position. 


### function readData:

```objective-c
virtual NSData * readData:(
    NSUInteger length
)
```


Reads a sequence of byte values of `length` and increments the position. 


### function readSize

```objective-c
virtual UInt32 readSize()
```


Reads a 32-bit unsigned integer representing a collection size and increments the position. 


### function readAlignment:

```objective-c
virtual void readAlignment:(
    UInt8 alignment
)
```


Advances the read position until it is aligned with `alignment`. 


### function readUTF8

```objective-c
virtual NSString * readUTF8()
```


Read a null terminated string encoded with UTF-8/ 


### function readValue

```objective-c
virtual nullable id readValue()
```


Reads a byte for `FlutterStandardField` the decodes a value matching that type.

See also: -[[FlutterStandardWriter](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) writeValue] 


### function readValueOfType:

```objective-c
virtual nullable id readValueOfType:(
    UInt8 type
)
```


Decodes a value matching the `type` specified.

See also:

* `FlutterStandardField`
* `-[[FlutterStandardWriter](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) writeValue]`


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700