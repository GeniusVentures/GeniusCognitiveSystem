---
title: FlutterAppDelegate

---

# FlutterAppDelegate



 [More...](#detailed-description)


`#include <FlutterAppDelegate.h>`

Inherits from NSObject, <NSApplicationDelegate>, <FlutterAppLifecycleProvider>, NSObject, <NSApplicationDelegate>, <FlutterAppLifecycleProvider>

## Public Properties

|                | Name           |
| -------------- | -------------- |
| IBOutlet NSMenu * | **[applicationMenu](/source-reference/Classes/dc/d1a/interface_flutter_app_delegate/#property-applicationmenu)**  |
| IBOutlet NSWindow * | **[mainFlutterWindow](/source-reference/Classes/dc/d1a/interface_flutter_app_delegate/#property-mainflutterwindow)**  |

## Detailed Description

```objective-c
class FlutterAppDelegate;
```


|NSApplicationDelegate| subclass for simple apps that want default behavior.

This class implements the following behaviors:

* Updates the application name of items in the application menu to match the name in the app's Info.plist, assuming it is set to APP_NAME initially. |applicationMenu| must be set before the application finishes launching for this to take effect.
* Updates the main Flutter window's title to match the name in the app's Info.plist. |mainFlutterWindow| must be set before the application finishes launching for this to take effect.
* Forwards [`NSApplicationDelegate`] callbacks to plugins that register for them.

App delegates for Flutter applications are _not_ required to inherit from this class. Developers of custom app delegate classes should copy and paste code as necessary from FlutterAppDelegate.mm. 

## Public Property Documentation

### property applicationMenu

```objective-c
IBOutlet NSMenu * applicationMenu;
```


The application menu in the menu bar. 


### property mainFlutterWindow

```objective-c
IBOutlet NSWindow * mainFlutterWindow;
```


The primary application window containing a [FlutterViewController](/source-reference/Classes/d1/d53/interface_flutter_view_controller/). This is primarily intended for use in single-window applications. 


-------------------------------

Updated on 2026-06-28 at 21:45:33 -0700