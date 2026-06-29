---
title: FlutterMethodCall

---

# FlutterMethodCall



 [More...](#detailed-description)


`#include <FlutterCodecs.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[methodCallWithMethodName:arguments:](/source-reference/Classes/d4/d81/interface_flutter_method_call/#function-methodcallwithmethodname:arguments:)**(NSString * method, id _Nullable arguments) |
| virtual instancetype | **[methodCallWithMethodName:arguments:](/source-reference/Classes/d4/d81/interface_flutter_method_call/#function-methodcallwithmethodname:arguments:)**(NSString * method, id _Nullable arguments) |

## Public Properties

|                | Name           |
| -------------- | -------------- |
| NSString * | **[method](/source-reference/Classes/d4/d81/interface_flutter_method_call/#property-method)**  |
| id | **[arguments](/source-reference/Classes/d4/d81/interface_flutter_method_call/#property-arguments)**  |

## Detailed Description

```objective-c
class FlutterMethodCall;
```


Command object representing a method call on a [`FlutterMethodChannel`](/source-reference/Classes/da/d6e/interface_flutter_method_channel/). 

## Public Functions Documentation

### function methodCallWithMethodName:arguments:

```objective-c
static virtual instancetype methodCallWithMethodName:arguments:(
    NSString * method,
    id _Nullable arguments
)
```


**Parameters**: 

  * **[method](/source-reference/Classes/d4/d81/interface_flutter_method_call/#property-method)** the name of the method to call. 
  * **[arguments](/source-reference/Classes/d4/d81/interface_flutter_method_call/#property-arguments)** the arguments value. 


Creates a method call for invoking the specified named method with the specified arguments.


### function methodCallWithMethodName:arguments:

```objective-c
static virtual instancetype methodCallWithMethodName:arguments:(
    NSString * method,
    id _Nullable arguments
)
```


**Parameters**: 

  * **[method](/source-reference/Classes/d4/d81/interface_flutter_method_call/#property-method)** the name of the method to call. 
  * **[arguments](/source-reference/Classes/d4/d81/interface_flutter_method_call/#property-arguments)** the arguments value. 


Creates a method call for invoking the specified named method with the specified arguments.


## Public Property Documentation

### property method

```objective-c
NSString * method;
```


The method name. 


### property arguments

```objective-c
id arguments;
```


The arguments. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700