---
title: FlutterStandardTypedData

---

# FlutterStandardTypedData



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[typedDataWithBytes:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithbytes:)**(NSData * data) |
| virtual instancetype | **[typedDataWithInt32:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithint32:)**(NSData * data) |
| virtual instancetype | **[typedDataWithInt64:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithint64:)**(NSData * data) |
| virtual instancetype | **[typedDataWithFloat32:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithfloat32:)**(NSData * data) |
| virtual instancetype | **[typedDataWithFloat64:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithfloat64:)**(NSData * data) |
| virtual instancetype | **[typedDataWithBytes:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithbytes:)**(NSData * data) |
| virtual instancetype | **[typedDataWithInt32:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithint32:)**(NSData * data) |
| virtual instancetype | **[typedDataWithInt64:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithint64:)**(NSData * data) |
| virtual instancetype | **[typedDataWithFloat32:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithfloat32:)**(NSData * data) |
| virtual instancetype | **[typedDataWithFloat64:](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#function-typeddatawithfloat64:)**(NSData * data) |

## Public Properties

|                | Name           |
| -------------- | -------------- |
| NSData * | **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)**  |
| FlutterStandardDataType | **[type](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-type)**  |
| UInt32 | **[elementCount](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-elementcount)**  |
| UInt8 | **[elementSize](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-elementsize)**  |

## Detailed Description

```objective-c
class FlutterStandardTypedData;
```


A byte buffer holding `UInt8`, `SInt32`, `SInt64`, or `Float64` values, used with [`FlutterStandardMessageCodec`](/source-reference/Classes/da/d21/interface_flutter_standard_message_codec/) and [`FlutterStandardMethodCodec`](/source-reference/Classes/d2/d0e/interface_flutter_standard_method_codec/).

Two's complement encoding is used for signed integers. IEEE754 double-precision representation is used for floats. The platform's native endianness is assumed. 

## Public Functions Documentation

### function typedDataWithBytes:

```objective-c
static virtual instancetype typedDataWithBytes:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as plain bytes.


### function typedDataWithInt32:

```objective-c
static virtual instancetype typedDataWithInt32:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. The length must be divisible by 4. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as 32-bit signed integers.


### function typedDataWithInt64:

```objective-c
static virtual instancetype typedDataWithInt64:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. The length must be divisible by 8. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as 64-bit signed integers.


### function typedDataWithFloat32:

```objective-c
static virtual instancetype typedDataWithFloat32:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. The length must be divisible by 8. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as 32-bit floats.


### function typedDataWithFloat64:

```objective-c
static virtual instancetype typedDataWithFloat64:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. The length must be divisible by 8. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as 64-bit floats.


### function typedDataWithBytes:

```objective-c
static virtual instancetype typedDataWithBytes:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as plain bytes.


### function typedDataWithInt32:

```objective-c
static virtual instancetype typedDataWithInt32:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. The length must be divisible by 4. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as 32-bit signed integers.


### function typedDataWithInt64:

```objective-c
static virtual instancetype typedDataWithInt64:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. The length must be divisible by 8. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as 64-bit signed integers.


### function typedDataWithFloat32:

```objective-c
static virtual instancetype typedDataWithFloat32:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. The length must be divisible by 8. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as 32-bit floats.


### function typedDataWithFloat64:

```objective-c
static virtual instancetype typedDataWithFloat64:(
    NSData * data
)
```


**Parameters**: 

  * **[data](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/#property-data)** the byte data. The length must be divisible by 8. 


Creates a [`FlutterStandardTypedData`](/source-reference/Classes/df/d85/interface_flutter_standard_typed_data/) which interprets the specified data as 64-bit floats.


## Public Property Documentation

### property data

```objective-c
NSData * data;
```


The raw underlying data buffer. 


### property type

```objective-c
FlutterStandardDataType type;
```


The type of the encoded values. 


### property elementCount

```objective-c
UInt32 elementCount;
```


The number of value items encoded. 


### property elementSize

```objective-c
UInt8 elementSize;
```


The number of bytes used by the encoding of a single value item. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700