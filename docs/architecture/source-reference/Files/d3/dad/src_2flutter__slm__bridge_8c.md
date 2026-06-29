---
title: GNUS-NEO-SWARM/flutter_slm_bridge/src/flutter_slm_bridge.c

---

# GNUS-NEO-SWARM/flutter_slm_bridge/src/flutter_slm_bridge.c





## Functions

|                | Name           |
| -------------- | -------------- |
| [FFI_PLUGIN_EXPORT](/source-reference/Files/d5/d0b/os__defines_8h/#define-ffi_plugin_export) int | **[sum](/source-reference/Files/d3/dad/src_2flutter__slm__bridge_8c/#function-sum)**(int a, int b) |
| [FFI_PLUGIN_EXPORT](/source-reference/Files/d5/d0b/os__defines_8h/#define-ffi_plugin_export) int | **[sum_long_running](/source-reference/Files/d3/dad/src_2flutter__slm__bridge_8c/#function-sum_long_running)**(int a, int b) |


## Functions Documentation

### function sum

```cpp
FFI_PLUGIN_EXPORT int sum(
    int a,
    int b
)
```


### function sum_long_running

```cpp
FFI_PLUGIN_EXPORT int sum_long_running(
    int a,
    int b
)
```




## Source code

```cpp
#include "flutter_slm_bridge.h"

// A very short-lived native function.
//
// For very short-lived functions, it is fine to call them on the main isolate.
// They will block the Dart execution while running the native function, so
// only do this for native functions which are guaranteed to be short-lived.
FFI_PLUGIN_EXPORT int sum(int a, int b) { return a + b; }

// A longer-lived native function, which occupies the thread calling it.
//
// Do not call these kind of native functions in the main isolate. They will
// block Dart execution. This will cause dropped frames in Flutter applications.
// Instead, call these native functions on a separate isolate.
FFI_PLUGIN_EXPORT int sum_long_running(int a, int b) {
  // Simulate work.
PLATFORM_SLEEP_MS( 5000 );
  return a + b;
}
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
