---
title: FlutterError

---

# FlutterError



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[errorWithCode:message:details:](/source-reference/Classes/d0/da1/interface_flutter_error/#function-errorwithcode:message:details:)**(NSString * code, NSString *_Nullable message, id _Nullable details) |
| virtual instancetype | **[errorWithCode:message:details:](/source-reference/Classes/d0/da1/interface_flutter_error/#function-errorwithcode:message:details:)**(NSString * code, NSString *_Nullable message, id _Nullable details) |

## Public Properties

|                | Name           |
| -------------- | -------------- |
| NSString * | **[code](/source-reference/Classes/d0/da1/interface_flutter_error/#property-code)**  |
| NSString * | **[message](/source-reference/Classes/d0/da1/interface_flutter_error/#property-message)**  |
| id | **[details](/source-reference/Classes/d0/da1/interface_flutter_error/#property-details)**  |

## Detailed Description

```objective-c
class FlutterError;
```


Error object representing an unsuccessful outcome of invoking a method on a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/), or an error event on a [`FlutterEventChannel`](/source-reference/Classes/dd/dda/interface_flutter_event_channel/). 

## Public Functions Documentation

### function errorWithCode:message:details:

```objective-c
static virtual instancetype errorWithCode:message:details:(
    NSString * code,
    NSString *_Nullable message,
    id _Nullable details
)
```


**Parameters**: 

  * **[code](/source-reference/Classes/d0/da1/interface_flutter_error/#property-code)** An error code string for programmatic use. 
  * **[message](/source-reference/Classes/d0/da1/interface_flutter_error/#property-message)** A human-readable error message. 
  * **[details](/source-reference/Classes/d0/da1/interface_flutter_error/#property-details)** Custom error details. 


Creates a [`FlutterError`](/source-reference/Classes/d0/da1/interface_flutter_error/) with the specified error code, message, and details.


### function errorWithCode:message:details:

```objective-c
static virtual instancetype errorWithCode:message:details:(
    NSString * code,
    NSString *_Nullable message,
    id _Nullable details
)
```


**Parameters**: 

  * **[code](/source-reference/Classes/d0/da1/interface_flutter_error/#property-code)** An error code string for programmatic use. 
  * **[message](/source-reference/Classes/d0/da1/interface_flutter_error/#property-message)** A human-readable error message. 
  * **[details](/source-reference/Classes/d0/da1/interface_flutter_error/#property-details)** Custom error details. 


Creates a [`FlutterError`](/source-reference/Classes/d0/da1/interface_flutter_error/) with the specified error code, message, and details.


## Public Property Documentation

### property code

```objective-c
NSString * code;
```


The error code. 


### property message

```objective-c
NSString * message;
```


The error message. 


### property details

```objective-c
id details;
```


The error details. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700