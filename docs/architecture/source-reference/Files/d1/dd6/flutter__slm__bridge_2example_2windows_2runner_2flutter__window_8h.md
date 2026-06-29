---
title: GNUS-NEO-SWARM/flutter_slm_bridge/example/windows/runner/flutter_window.h

---

# GNUS-NEO-SWARM/flutter_slm_bridge/example/windows/runner/flutter_window.h





## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[FlutterWindow](/source-reference/Classes/d0/df0/class_flutter_window/)**  |




## Source code

```cpp
#ifndef RUNNER_FLUTTER_WINDOW_H_
#define RUNNER_FLUTTER_WINDOW_H_

#include <flutter/dart_project.h>
#include <flutter/flutter_view_controller.h>

#include <memory>

#include "win32_window.h"

// A window that does nothing but host a Flutter view.
class FlutterWindow : public Win32Window {
 public:
  // Creates a new FlutterWindow hosting a Flutter view running |project|.
  explicit FlutterWindow(const flutter::DartProject& project);
  virtual ~FlutterWindow();

 protected:
  // Win32Window:
  bool OnCreate() override;
  void OnDestroy() override;
  LRESULT MessageHandler(HWND window, UINT const message, WPARAM const wparam,
                         LPARAM const lparam) noexcept override;

 private:
  // The project to run.
  flutter::DartProject project_;

  // The Flutter instance hosted by this window.
  std::unique_ptr<flutter::FlutterViewController> flutter_controller_;
};

#endif  // RUNNER_FLUTTER_WINDOW_H_
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
