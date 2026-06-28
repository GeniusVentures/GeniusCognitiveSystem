---
title: GNUS-NEO-SWARM/flutter_slm_bridge/example/linux/runner/my_application.h

---

# GNUS-NEO-SWARM/flutter_slm_bridge/example/linux/runner/my_application.h





## Functions

|                | Name           |
| -------------- | -------------- |
| | **[G_DECLARE_FINAL_TYPE](/source-reference/Files/d4/d8b/flutter__slm__bridge_2example_2linux_2runner_2my__application_8h/#function-g-declare-final-type)**(MyApplication , my_application , MY , APPLICATION , GtkApplication ) |


## Functions Documentation

### function G_DECLARE_FINAL_TYPE

```cpp
G_DECLARE_FINAL_TYPE(
    MyApplication ,
    my_application ,
    MY ,
    APPLICATION ,
    GtkApplication 
)
```


my_application_new:

Creates a new Flutter-based application.

Returns: a new #MyApplication. 




## Source code

```cpp
#ifndef FLUTTER_MY_APPLICATION_H_
#define FLUTTER_MY_APPLICATION_H_

#include <gtk/gtk.h>

G_DECLARE_FINAL_TYPE(MyApplication, my_application, MY, APPLICATION,
                     GtkApplication)


MyApplication* my_application_new();

#endif  // FLUTTER_MY_APPLICATION_H_
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
