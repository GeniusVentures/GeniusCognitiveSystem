---
title: FlutterEngine

---

# FlutterEngine



 [More...](#detailed-description)


`#include <FlutterEngine.h>`

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual nonnull instancetype | **[initWithName:project:](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-initwithname:project:)**(nonnull NSString * labelPrefix, nullable [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) * project) |
| virtual nonnull instancetype | **[initWithName:project:allowHeadlessExecution:](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-initwithname:project:allowheadlessexecution:)**(nonnull NSString * labelPrefix, nullable [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) * project, BOOL NS_DESIGNATED_INITIALIZER) |
| virtual nonnull instancetype | **[NS_UNAVAILABLE](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-ns_unavailable)**() |
| virtual BOOL | **[runWithEntrypoint:](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-runwithentrypoint:)**(nullable NSString * entrypoint) |
| virtual void | **[shutDownEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-shutdownengine)**() |
| virtual nonnull instancetype | **[initWithName:project:](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-initwithname:project:)**(nonnull NSString * labelPrefix, nullable [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) * project) |
| virtual nonnull instancetype | **[initWithName:project:allowHeadlessExecution:](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-initwithname:project:allowheadlessexecution:)**(nonnull NSString * labelPrefix, nullable [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) * project, BOOL NS_DESIGNATED_INITIALIZER) |
| virtual nonnull instancetype | **[NS_UNAVAILABLE](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-ns_unavailable)**() |
| virtual BOOL | **[runWithEntrypoint:](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-runwithentrypoint:)**(nullable NSString * entrypoint) |
| virtual void | **[shutDownEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/#function-shutdownengine)**() |

## Public Properties

|                | Name           |
| -------------- | -------------- |
| [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) * | **[viewController](/source-reference/Classes/d3/dbc/interface_flutter_engine/#property-viewcontroller)**  |
| id< [FlutterBinaryMessenger] > | **[binaryMessenger](/source-reference/Classes/d3/dbc/interface_flutter_engine/#property-binarymessenger)**  |

## Protected Attributes

|                | Name           |
| -------------- | -------------- |
| | **[__pad0__](/source-reference/Classes/d3/dbc/interface_flutter_engine/#variable-__pad0__)**  |
| | **[FlutterPluginRegistry](/source-reference/Classes/d3/dbc/interface_flutter_engine/#variable-flutterpluginregistry)**  |

## Detailed Description

```objective-c
class FlutterEngine;
```


Coordinates a single instance of execution of a Flutter engine.

A [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) can only be attached with one controller from the native code. 

## Public Functions Documentation

### function initWithName:project:

```objective-c
virtual nonnull instancetype initWithName:project:(
    nonnull NSString * labelPrefix,
    nullable FlutterDartProject * project
)
```


**Parameters**: 

  * **labelPrefix** Currently unused; in the future, may be used for labelling threads as with the iOS [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/). 
  * **project** The project configuration. If nil, a default [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) will be used. 


Initializes an engine with the given project.


### function initWithName:project:allowHeadlessExecution:

```objective-c
virtual nonnull instancetype initWithName:project:allowHeadlessExecution:(
    nonnull NSString * labelPrefix,
    nullable FlutterDartProject * project,
    BOOL NS_DESIGNATED_INITIALIZER
)
```


**Parameters**: 

  * **labelPrefix** Currently unused; in the future, may be used for labelling threads as with the iOS [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/). 
  * **project** The project configuration. If nil, a default [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) will be used. 


Initializes an engine that can run headlessly with the given project.


### function NS_UNAVAILABLE

```objective-c
virtual nonnull instancetype NS_UNAVAILABLE()
```


### function runWithEntrypoint:

```objective-c
virtual BOOL runWithEntrypoint:(
    nullable NSString * entrypoint
)
```


**Parameters**: 

  * **entrypoint** The name of a top-level function from the same Dart library that contains the app's [main()](/source-reference/Files/d1/d3a/_c_make_c_compiler_id_8c/#function-main) function. If this is nil, it will default to [`main()`](/source-reference/Files/d1/d3a/_c_make_c_compiler_id_8c/#function-main). If it is not the app's [main()](/source-reference/Files/d1/d3a/_c_make_c_compiler_id_8c/#function-main) function, that function must be decorated with `@pragma(vm:entry-point)` to ensure the method is not tree-shaken by the Dart compiler. 


**Return**: YES if the call succeeds in creating and running a Flutter Engine instance; NO otherwise. 

Runs a Dart program on an Isolate from the main Dart library (i.e. the library that contains [`main()`](/source-reference/Files/d1/d3a/_c_make_c_compiler_id_8c/#function-main)).

The first call to this method will create a new Isolate. Subsequent calls will return immediately.


### function shutDownEngine

```objective-c
virtual void shutDownEngine()
```


Shuts the Flutter engine if it is running. The [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) instance must always be shutdown before it may be collected. Not shutting down the [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) instance before releasing it will result in the leak of that engine instance. 


### function initWithName:project:

```objective-c
virtual nonnull instancetype initWithName:project:(
    nonnull NSString * labelPrefix,
    nullable FlutterDartProject * project
)
```


**Parameters**: 

  * **labelPrefix** Currently unused; in the future, may be used for labelling threads as with the iOS [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/). 
  * **project** The project configuration. If nil, a default [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) will be used. 


Initializes an engine with the given project.


### function initWithName:project:allowHeadlessExecution:

```objective-c
virtual nonnull instancetype initWithName:project:allowHeadlessExecution:(
    nonnull NSString * labelPrefix,
    nullable FlutterDartProject * project,
    BOOL NS_DESIGNATED_INITIALIZER
)
```


**Parameters**: 

  * **labelPrefix** Currently unused; in the future, may be used for labelling threads as with the iOS [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/). 
  * **project** The project configuration. If nil, a default [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) will be used. 


Initializes an engine that can run headlessly with the given project.


### function NS_UNAVAILABLE

```objective-c
virtual nonnull instancetype NS_UNAVAILABLE()
```


### function runWithEntrypoint:

```objective-c
virtual BOOL runWithEntrypoint:(
    nullable NSString * entrypoint
)
```


**Parameters**: 

  * **entrypoint** The name of a top-level function from the same Dart library that contains the app's [main()](/source-reference/Files/d1/d3a/_c_make_c_compiler_id_8c/#function-main) function. If this is nil, it will default to [`main()`](/source-reference/Files/d1/d3a/_c_make_c_compiler_id_8c/#function-main). If it is not the app's [main()](/source-reference/Files/d1/d3a/_c_make_c_compiler_id_8c/#function-main) function, that function must be decorated with `@pragma(vm:entry-point)` to ensure the method is not tree-shaken by the Dart compiler. 


**Return**: YES if the call succeeds in creating and running a Flutter Engine instance; NO otherwise. 

Runs a Dart program on an Isolate from the main Dart library (i.e. the library that contains [`main()`](/source-reference/Files/d1/d3a/_c_make_c_compiler_id_8c/#function-main)).

The first call to this method will create a new Isolate. Subsequent calls will return immediately.


### function shutDownEngine

```objective-c
virtual void shutDownEngine()
```


Shuts the Flutter engine if it is running. The [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) instance must always be shutdown before it may be collected. Not shutting down the [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) instance before releasing it will result in the leak of that engine instance. 


## Public Property Documentation

### property viewController

```objective-c
FlutterViewController * viewController;
```


The [`FlutterViewController`](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) of this engine, if any.

This view is used by legacy APIs that assume a single view.

Setting this field from nil to a non-nil view controller also updates the view controller's engine and ID.

Setting this field from non-nil to nil will terminate the engine if allowHeadlessExecution is NO.

Setting this field from non-nil to a different non-nil [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) is prohibited and will throw an assertion error. 


### property binaryMessenger

```objective-c
id< FlutterBinaryMessenger > binaryMessenger;
```


The [`FlutterBinaryMessenger`] for communicating with this engine. 


## Protected Attributes Documentation

### variable __pad0__

```objective-c
__pad0__;
```


### variable FlutterPluginRegistry

```objective-c
FlutterPluginRegistry;
```


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700