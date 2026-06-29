---
title: FlutterViewController

---

# FlutterViewController



 [More...](#detailed-description)


`#include <FlutterViewController.h>`

Inherits from NSViewController, <FlutterPluginRegistry>, NSViewController, <FlutterPluginRegistry>

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual nonnull instancetype | **[initWithProject:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-initwithproject:)**(nullable [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) * NS_DESIGNATED_INITIALIZER) |
| virtual nonnull instancetype | **[initWithNibName:bundle:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-initwithnibname:bundle:)**(nullable NSString * nibNameOrNil, nullable NSBundle * NS_DESIGNATED_INITIALIZER) |
| virtual nonnull instancetype | **[initWithCoder:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-initwithcoder:)**(nonnull NSCoder * NS_DESIGNATED_INITIALIZER) |
| virtual nonnull instancetype | **[initWithEngine:nibName:bundle:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-initwithengine:nibname:bundle:)**(nonnull [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) * engine, nullable NSString * nibName, nullable NSBundle * NS_DESIGNATED_INITIALIZER) |
| virtual BOOL | **[attached](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-attached)**() |
| virtual void | **[onPreEngineRestart](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-onpreenginerestart)**() |
| virtual nonnull NSString * | **[lookupKeyForAsset:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-lookupkeyforasset:)**(nonnull NSString * asset) |
| virtual nonnull NSString * | **[lookupKeyForAsset:fromPackage:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-lookupkeyforasset:frompackage:)**(nonnull NSString * asset, nonnull NSString * package) |
| virtual nonnull instancetype | **[initWithProject:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-initwithproject:)**(nullable [FlutterDartProject](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) * NS_DESIGNATED_INITIALIZER) |
| virtual nonnull instancetype | **[initWithNibName:bundle:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-initwithnibname:bundle:)**(nullable NSString * nibNameOrNil, nullable NSBundle * NS_DESIGNATED_INITIALIZER) |
| virtual nonnull instancetype | **[initWithCoder:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-initwithcoder:)**(nonnull NSCoder * NS_DESIGNATED_INITIALIZER) |
| virtual nonnull instancetype | **[initWithEngine:nibName:bundle:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-initwithengine:nibname:bundle:)**(nonnull [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) * engine, nullable NSString * nibName, nullable NSBundle * NS_DESIGNATED_INITIALIZER) |
| virtual BOOL | **[attached](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-attached)**() |
| virtual void | **[onPreEngineRestart](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-onpreenginerestart)**() |
| virtual nonnull NSString * | **[lookupKeyForAsset:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-lookupkeyforasset:)**(nonnull NSString * asset) |
| virtual nonnull NSString * | **[lookupKeyForAsset:fromPackage:](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-lookupkeyforasset:frompackage:)**(nonnull NSString * asset, nonnull NSString * package) |

## Public Properties

|                | Name           |
| -------------- | -------------- |
| [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) * | **[engine](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#property-engine)**  |
| FlutterMouseTrackingMode | **[mouseTrackingMode](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#property-mousetrackingmode)**  |
| NSColor * | **[backgroundColor](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#property-backgroundcolor)**  |
| [FlutterViewIdentifier](/source-reference/Files/d7/d79/_release_2_flutter_mac_o_s_8framework_2_versions_2_a_2_headers_2_flutter_view_controller_8h/#typedef-flutterviewidentifier) | **[viewIdentifier](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#property-viewidentifier)**  |

## Detailed Description

```objective-c
class FlutterViewController;
```


Controls a view that displays Flutter content and manages input.

A [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) works with a [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/). Upon creation, the view controller is always added to an engine, either a given engine, or it implicitly creates an engine and add itself to that engine.

The [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) assigns each view controller attached to it a unique ID. Each view controller corresponds to a view, and the ID is used by the framework to specify which view to operate.

A [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) can also be unattached to an engine after it is manually unset from the engine, or transiently during the initialization process. An unattached view controller is invalid. Whether the view controller is attached can be queried using [attached (FlutterViewController)](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-attached).

The [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) strongly references the [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/), while the engine weakly the view controller. When a [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) is deallocated, it automatically removes itself from its attached engine. When a [FlutterEngine](/source-reference/Classes/d3/dbc/interface_flutter_engine/) has no FlutterViewControllers attached, it might shut down itself or not depending on its configuration. 

## Public Functions Documentation

### function initWithProject:

```objective-c
virtual nonnull instancetype initWithProject:(
    nullable FlutterDartProject * NS_DESIGNATED_INITIALIZER
)
```


**Parameters**: 

  * **project** The project to run in this view controller. If nil, a default [`FlutterDartProject`](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) will be used. 


Initializes a controller that will run the given project.

In this initializer, this controller creates an engine, and is attached to that engine as the default controller. In this way, this controller can not be set to other engines. This initializer is suitable for the first Flutter view controller of the app. To use the controller with an existing engine, use initWithEngine:nibName:bundle: instead.


### function initWithNibName:bundle:

```objective-c
virtual nonnull instancetype initWithNibName:bundle:(
    nullable NSString * nibNameOrNil,
    nullable NSBundle * NS_DESIGNATED_INITIALIZER
)
```


### function initWithCoder:

```objective-c
virtual nonnull instancetype initWithCoder:(
    nonnull NSCoder * NS_DESIGNATED_INITIALIZER
)
```


### function initWithEngine:nibName:bundle:

```objective-c
virtual nonnull instancetype initWithEngine:nibName:bundle:(
    nonnull FlutterEngine * engine,
    nullable NSString * nibName,
    nullable NSBundle * NS_DESIGNATED_INITIALIZER
)
```


**Parameters**: 

  * **[engine](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#property-engine)** The [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) instance to attach to. Cannot be nil. 
  * **nibName** The NIB name to initialize this controller with. 
  * **nibBundle** The NIB bundle. 


Initializes this [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) with an existing [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/).

The initialized view controller will add itself to the engine as part of this process.

This initializer is suitable for both the first Flutter view controller and the following ones of the app.


### function attached

```objective-c
virtual BOOL attached()
```


Return YES if the view controller is attached to an engine. 


### function onPreEngineRestart

```objective-c
virtual void onPreEngineRestart()
```


Invoked by the engine right before the engine is restarted.

This should reset states to as if the application has just started. It usually indicates a hot restart (Shift-R in Flutter CLI.) 


### function lookupKeyForAsset:

```objective-c
virtual nonnull NSString * lookupKeyForAsset:(
    nonnull NSString * asset
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 


**Return**: The file name to be used for lookup in the main bundle. 

Returns the file name for the given asset. The returned file name can be used to access the asset in the application's main bundle.


### function lookupKeyForAsset:fromPackage:

```objective-c
virtual nonnull NSString * lookupKeyForAsset:fromPackage:(
    nonnull NSString * asset,
    nonnull NSString * package
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 
  * **package** The name of the package from which the asset originates. 


**Return**: The file name to be used for lookup in the main bundle. 

Returns the file name for the given asset which originates from the specified package. The returned file name can be used to access the asset in the application's main bundle.


### function initWithProject:

```objective-c
virtual nonnull instancetype initWithProject:(
    nullable FlutterDartProject * NS_DESIGNATED_INITIALIZER
)
```


**Parameters**: 

  * **project** The project to run in this view controller. If nil, a default [`FlutterDartProject`](/source-reference/Classes/d5/db0/interface_flutter_dart_project/) will be used. 


Initializes a controller that will run the given project.

In this initializer, this controller creates an engine, and is attached to that engine as the default controller. In this way, this controller can not be set to other engines. This initializer is suitable for the first Flutter view controller of the app. To use the controller with an existing engine, use initWithEngine:nibName:bundle: instead.


### function initWithNibName:bundle:

```objective-c
virtual nonnull instancetype initWithNibName:bundle:(
    nullable NSString * nibNameOrNil,
    nullable NSBundle * NS_DESIGNATED_INITIALIZER
)
```


### function initWithCoder:

```objective-c
virtual nonnull instancetype initWithCoder:(
    nonnull NSCoder * NS_DESIGNATED_INITIALIZER
)
```


### function initWithEngine:nibName:bundle:

```objective-c
virtual nonnull instancetype initWithEngine:nibName:bundle:(
    nonnull FlutterEngine * engine,
    nullable NSString * nibName,
    nullable NSBundle * NS_DESIGNATED_INITIALIZER
)
```


**Parameters**: 

  * **[engine](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#property-engine)** The [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) instance to attach to. Cannot be nil. 
  * **nibName** The NIB name to initialize this controller with. 
  * **nibBundle** The NIB bundle. 


Initializes this [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) with an existing [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/).

The initialized view controller will add itself to the engine as part of this process.

This initializer is suitable for both the first Flutter view controller and the following ones of the app.


### function attached

```objective-c
virtual BOOL attached()
```


Return YES if the view controller is attached to an engine. 


### function onPreEngineRestart

```objective-c
virtual void onPreEngineRestart()
```


Invoked by the engine right before the engine is restarted.

This should reset states to as if the application has just started. It usually indicates a hot restart (Shift-R in Flutter CLI.) 


### function lookupKeyForAsset:

```objective-c
virtual nonnull NSString * lookupKeyForAsset:(
    nonnull NSString * asset
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 


**Return**: The file name to be used for lookup in the main bundle. 

Returns the file name for the given asset. The returned file name can be used to access the asset in the application's main bundle.


### function lookupKeyForAsset:fromPackage:

```objective-c
virtual nonnull NSString * lookupKeyForAsset:fromPackage:(
    nonnull NSString * asset,
    nonnull NSString * package
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 
  * **package** The name of the package from which the asset originates. 


**Return**: The file name to be used for lookup in the main bundle. 

Returns the file name for the given asset which originates from the specified package. The returned file name can be used to access the asset in the application's main bundle.


## Public Property Documentation

### property engine

```objective-c
FlutterEngine * engine;
```


The Flutter engine associated with this view controller. 


### property mouseTrackingMode

```objective-c
FlutterMouseTrackingMode mouseTrackingMode;
```


The style of mouse tracking to use for the view. Defaults to FlutterMouseTrackingModeInKeyWindow. 


### property backgroundColor

```objective-c
NSColor * backgroundColor;
```


The contentView (FlutterView)'s background color is set to black during its instantiation.

The containing layer's color can be set to the NSColor provided to this method.

For example, the background may be set after the [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/) is instantiated in MainFlutterWindow.swift in the Flutter project. ```swift

import Cocoa
import FlutterMacOS

class MainFlutterWindow: NSWindow {
  override func awakeFromNib() {
    let flutterViewController = FlutterViewController()

    // The background color of the window and `FlutterViewController`
    // are retained separately.
    //
    // In this example, both the MainFlutterWindow and FlutterViewController's
    // FlutterView's backgroundColor are set to clear to achieve a fully
    // transparent effect.
    //
    // If the window's background color is not set, it will use the system
    // default.
    //
    // If the `FlutterView`'s color is not set via `FlutterViewController.setBackgroundColor`
    // it's default will be black.
    self.backgroundColor = NSColor.clear
    flutterViewController.backgroundColor = NSColor.clear

    let windowFrame = self.frame
    self.contentViewController = flutterViewController
    self.setFrame(windowFrame, display: true)

    RegisterGeneratedPlugins(registry: flutterViewController)

    super.awakeFromNib()
  }
}
```


### property viewIdentifier

```objective-c
FlutterViewIdentifier viewIdentifier;
```


The identifier for this view controller, if it is attached.

The identifier is assigned when the view controller is attached to a [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/).

If the view controller is detached (see `[FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/)#[- attached](/source-reference/Classes/d1/d53/interface_flutter_view_controller/#function-attached)`), reading this property throws an assertion. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700