---
title: FlutterStandardReaderWriter

---

# FlutterStandardReaderWriter



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual [FlutterStandardWriter](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) * | **[writerWithData:](/source-reference/Classes/da/d07/interface_flutter_standard_reader_writer/#function-writerwithdata:)**(NSMutableData * data) |
| virtual [FlutterStandardReader](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/) * | **[readerWithData:](/source-reference/Classes/da/d07/interface_flutter_standard_reader_writer/#function-readerwithdata:)**(NSData * data) |
| virtual [FlutterStandardWriter](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) * | **[writerWithData:](/source-reference/Classes/da/d07/interface_flutter_standard_reader_writer/#function-writerwithdata:)**(NSMutableData * data) |
| virtual [FlutterStandardReader](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/) * | **[readerWithData:](/source-reference/Classes/da/d07/interface_flutter_standard_reader_writer/#function-readerwithdata:)**(NSData * data) |

## Detailed Description

```objective-c
class FlutterStandardReaderWriter;
```


A factory of compatible reader/writer instances using the Flutter standard binary encoding or extensions thereof. 

## Public Functions Documentation

### function writerWithData:

```objective-c
virtual FlutterStandardWriter * writerWithData:(
    NSMutableData * data
)
```


Create a new [`FlutterStandardWriter`](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) for writing to `data`. 


### function readerWithData:

```objective-c
virtual FlutterStandardReader * readerWithData:(
    NSData * data
)
```


Create a new [`FlutterStandardReader`](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/) for reading from `data`. 


### function writerWithData:

```objective-c
virtual FlutterStandardWriter * writerWithData:(
    NSMutableData * data
)
```


Create a new [`FlutterStandardWriter`](/source-reference/Classes/d4/d9e/interface_flutter_standard_writer/) for writing to `data`. 


### function readerWithData:

```objective-c
virtual FlutterStandardReader * readerWithData:(
    NSData * data
)
```


Create a new [`FlutterStandardReader`](/source-reference/Classes/d6/d72/interface_flutter_standard_reader/) for reading from `data`. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700