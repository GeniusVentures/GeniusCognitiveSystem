---
title: FlutterAppLifecycleRegistrar

---

# FlutterAppLifecycleRegistrar



 [More...](#detailed-description)


`#include <FlutterAppLifecycleDelegate.h>`

Inherits from NSObject, <FlutterAppLifecycleDelegate>, NSObject, <FlutterAppLifecycleDelegate>

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual void | **[addDelegate:](/source-reference/Classes/d0/d54/interface_flutter_app_lifecycle_registrar/#function-adddelegate:)**(NSObject< [FlutterAppLifecycleDelegate] > * delegate) |
| virtual void | **[removeDelegate:](/source-reference/Classes/d0/d54/interface_flutter_app_lifecycle_registrar/#function-removedelegate:)**(NSObject< [FlutterAppLifecycleDelegate] > * delegate) |
| virtual void | **[addDelegate:](/source-reference/Classes/d0/d54/interface_flutter_app_lifecycle_registrar/#function-adddelegate:)**(NSObject< [FlutterAppLifecycleDelegate] > * delegate) |
| virtual void | **[removeDelegate:](/source-reference/Classes/d0/d54/interface_flutter_app_lifecycle_registrar/#function-removedelegate:)**(NSObject< [FlutterAppLifecycleDelegate] > * delegate) |

## Detailed Description

```objective-c
class FlutterAppLifecycleRegistrar;
```


Propagates `NSAppDelegate` callbacks to registered delegates. 

## Public Functions Documentation

### function addDelegate:

```objective-c
virtual void addDelegate:(
    NSObject< FlutterAppLifecycleDelegate > * delegate
)
```


Registers `delegate` to receive lifecycle callbacks via this [FlutterAppLifecycleDelegate] as long as it is alive.

`delegate` will only be referenced weakly. 


### function removeDelegate:

```objective-c
virtual void removeDelegate:(
    NSObject< FlutterAppLifecycleDelegate > * delegate
)
```


Unregisters `delegate` so that it will no longer receive life cycle callbacks via this [FlutterAppLifecycleDelegate].

`delegate` will only be referenced weakly. 


### function addDelegate:

```objective-c
virtual void addDelegate:(
    NSObject< FlutterAppLifecycleDelegate > * delegate
)
```


Registers `delegate` to receive lifecycle callbacks via this [FlutterAppLifecycleDelegate] as long as it is alive.

`delegate` will only be referenced weakly. 


### function removeDelegate:

```objective-c
virtual void removeDelegate:(
    NSObject< FlutterAppLifecycleDelegate > * delegate
)
```


Unregisters `delegate` so that it will no longer receive life cycle callbacks via this [FlutterAppLifecycleDelegate].

`delegate` will only be referenced weakly. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700