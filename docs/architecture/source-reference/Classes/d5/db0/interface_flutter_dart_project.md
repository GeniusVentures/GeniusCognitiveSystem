---
title: FlutterDartProject

---

# FlutterDartProject



 [More...](#detailed-description)


`#include <FlutterDartProject.h>`

Inherits from NSObject, NSObject

## Public Functions

|                | Name           |
| -------------- | -------------- |
| virtual instancetype | **[initWithPrecompiledDartBundle:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-initwithprecompileddartbundle:)**(nullable NSBundle * NS_DESIGNATED_INITIALIZER) |
| virtual "Use -init instead." | **[FLUTTER_UNAVAILABLE](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-flutter_unavailable)**() |
| NSArray< NSString * > *dartEntrypointArguments | **[API_UNAVAILABLE](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-api_unavailable)**(ios ) |
| virtual instancetype | **[initWithPrecompiledDartBundle:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-initwithprecompileddartbundle:)**(nullable NSBundle * NS_DESIGNATED_INITIALIZER) |
| virtual "Use -init instead." | **[FLUTTER_UNAVAILABLE](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-flutter_unavailable)**() |
| NSArray< NSString * > *dartEntrypointArguments | **[API_UNAVAILABLE](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-api_unavailable)**(ios ) |
| virtual NSString * | **[defaultBundleIdentifier](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-defaultbundleidentifier)**() |
| virtual NSString * | **[lookupKeyForAsset:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:)**(NSString * asset) |
| virtual NSString * | **[lookupKeyForAsset:fromBundle:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:frombundle:)**(NSString * asset, nullable NSBundle * bundle) |
| virtual NSString * | **[lookupKeyForAsset:fromPackage:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:frompackage:)**(NSString * asset, NSString * package) |
| virtual NSString * | **[lookupKeyForAsset:fromPackage:fromBundle:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:frompackage:frombundle:)**(NSString * asset, NSString * package, nullable NSBundle * bundle) |
| virtual NSString * | **[defaultBundleIdentifier](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-defaultbundleidentifier)**() |
| virtual NSString * | **[lookupKeyForAsset:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:)**(NSString * asset) |
| virtual NSString * | **[lookupKeyForAsset:fromBundle:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:frombundle:)**(NSString * asset, nullable NSBundle * bundle) |
| virtual NSString * | **[lookupKeyForAsset:fromPackage:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:frompackage:)**(NSString * asset, NSString * package) |
| virtual NSString * | **[lookupKeyForAsset:fromPackage:fromBundle:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:frompackage:frombundle:)**(NSString * asset, NSString * package, nullable NSBundle * bundle) |

## Detailed Description

```objective-c
class FlutterDartProject;
```


A set of Flutter and Dart assets used by a [`FlutterEngine`](/source-reference/Classes/d3/dbc/interface_flutter_engine/) to initialize execution. 

## Public Functions Documentation

### function initWithPrecompiledDartBundle:

```objective-c
virtual instancetype initWithPrecompiledDartBundle:(
    nullable NSBundle * NS_DESIGNATED_INITIALIZER
)
```


**Parameters**: 

  * **bundle** The bundle containing the Flutter assets directory. If nil, the App framework created by Flutter will be used. 


Initializes a Flutter Dart project from a bundle.

The bundle must either contain a flutter_assets resource directory, or set the Info.plist key FLTAssetsPath to override that name (if you are doing a custom build using a different name).


### function FLUTTER_UNAVAILABLE

```objective-c
virtual "Use -init instead." FLUTTER_UNAVAILABLE()
```


Unavailable - use `init` instead. 


### function API_UNAVAILABLE

```objective-c
NSArray< NSString * > *dartEntrypointArguments API_UNAVAILABLE(
    ios 
)
```


An NSArray of NSStrings to be passed as command line arguments to the Dart entrypoint.

If this is not explicitly set, this will default to the contents of [NSProcessInfo arguments], without the binary name.

Set this to nil to pass no arguments to the Dart entrypoint. 


### function initWithPrecompiledDartBundle:

```objective-c
virtual instancetype initWithPrecompiledDartBundle:(
    nullable NSBundle * NS_DESIGNATED_INITIALIZER
)
```


**Parameters**: 

  * **bundle** The bundle containing the Flutter assets directory. If nil, the App framework created by Flutter will be used. 


Initializes a Flutter Dart project from a bundle.

The bundle must either contain a flutter_assets resource directory, or set the Info.plist key FLTAssetsPath to override that name (if you are doing a custom build using a different name).


### function FLUTTER_UNAVAILABLE

```objective-c
virtual "Use -init instead." FLUTTER_UNAVAILABLE()
```


Unavailable - use `init` instead. 


### function API_UNAVAILABLE

```objective-c
NSArray< NSString * > *dartEntrypointArguments API_UNAVAILABLE(
    ios 
)
```


An NSArray of NSStrings to be passed as command line arguments to the Dart entrypoint.

If this is not explicitly set, this will default to the contents of [NSProcessInfo arguments], without the binary name.

Set this to nil to pass no arguments to the Dart entrypoint. 


### function defaultBundleIdentifier

```objective-c
static virtual NSString * defaultBundleIdentifier()
```


Returns the default identifier for the bundle where we expect to find the Flutter Dart application. 


### function lookupKeyForAsset:

```objective-c
static virtual NSString * lookupKeyForAsset:(
    NSString * asset
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 


**Return**: the file name to be used for lookup in the main bundle. 

Returns the file name for the given asset. If the bundle with the identifier "io.flutter.flutter.app" exists, it will try use that bundle; otherwise, it will use the main bundle. To specify a different bundle, use `+[+ lookupKeyForAsset:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:)fromBundle`.


### function lookupKeyForAsset:fromBundle:

```objective-c
static virtual NSString * lookupKeyForAsset:fromBundle:(
    NSString * asset,
    nullable NSBundle * bundle
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 
  * **bundle** The `NSBundle` to use for looking up the asset. 


**Return**: the file name to be used for lookup in the main bundle. 

Returns the file name for the given asset. The returned file name can be used to access the asset in the supplied bundle.


### function lookupKeyForAsset:fromPackage:

```objective-c
static virtual NSString * lookupKeyForAsset:fromPackage:(
    NSString * asset,
    NSString * package
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 
  * **package** The name of the package from which the asset originates. 


**Return**: the file name to be used for lookup in the main bundle. 

Returns the file name for the given asset which originates from the specified package. The returned file name can be used to access the asset in the application's main bundle.


### function lookupKeyForAsset:fromPackage:fromBundle:

```objective-c
static virtual NSString * lookupKeyForAsset:fromPackage:fromBundle:(
    NSString * asset,
    NSString * package,
    nullable NSBundle * bundle
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 
  * **package** The name of the package from which the asset originates. 
  * **bundle** The bundle to use when doing the lookup. 


**Return**: the file name to be used for lookup in the main bundle. 

Returns the file name for the given asset which originates from the specified package. The returned file name can be used to access the asset in the specified bundle.


### function defaultBundleIdentifier

```objective-c
static virtual NSString * defaultBundleIdentifier()
```


Returns the default identifier for the bundle where we expect to find the Flutter Dart application. 


### function lookupKeyForAsset:

```objective-c
static virtual NSString * lookupKeyForAsset:(
    NSString * asset
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 


**Return**: the file name to be used for lookup in the main bundle. 

Returns the file name for the given asset. If the bundle with the identifier "io.flutter.flutter.app" exists, it will try use that bundle; otherwise, it will use the main bundle. To specify a different bundle, use `+[+ lookupKeyForAsset:](/source-reference/Classes/d5/db0/interface_flutter_dart_project/#function-lookupkeyforasset:)fromBundle`.


### function lookupKeyForAsset:fromBundle:

```objective-c
static virtual NSString * lookupKeyForAsset:fromBundle:(
    NSString * asset,
    nullable NSBundle * bundle
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 
  * **bundle** The `NSBundle` to use for looking up the asset. 


**Return**: the file name to be used for lookup in the main bundle. 

Returns the file name for the given asset. The returned file name can be used to access the asset in the supplied bundle.


### function lookupKeyForAsset:fromPackage:

```objective-c
static virtual NSString * lookupKeyForAsset:fromPackage:(
    NSString * asset,
    NSString * package
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 
  * **package** The name of the package from which the asset originates. 


**Return**: the file name to be used for lookup in the main bundle. 

Returns the file name for the given asset which originates from the specified package. The returned file name can be used to access the asset in the application's main bundle.


### function lookupKeyForAsset:fromPackage:fromBundle:

```objective-c
static virtual NSString * lookupKeyForAsset:fromPackage:fromBundle:(
    NSString * asset,
    NSString * package,
    nullable NSBundle * bundle
)
```


**Parameters**: 

  * **asset** The name of the asset. The name can be hierarchical. 
  * **package** The name of the package from which the asset originates. 
  * **bundle** The bundle to use when doing the lookup. 


**Return**: the file name to be used for lookup in the main bundle. 

Returns the file name for the given asset which originates from the specified package. The returned file name can be used to access the asset in the specified bundle.


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700