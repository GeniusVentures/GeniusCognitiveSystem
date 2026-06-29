---
title: Win32Window

---

# Win32Window






`#include <win32_window.h>`

Inherited by [FlutterWindow](/source-reference/Classes/d0/df0/class_flutter_window/), [FlutterWindow](/source-reference/Classes/d0/df0/class_flutter_window/), [FlutterWindow](/source-reference/Classes/d0/df0/class_flutter_window/)

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Point](/source-reference/Classes/d4/d78/struct_win32_window_1_1_point/)**  |
| struct | **[Size](/source-reference/Classes/d1/db6/struct_win32_window_1_1_size/)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[Win32Window](/source-reference/Classes/df/d4e/class_win32_window/#function-win32window)**() |
| virtual | **[~Win32Window](/source-reference/Classes/df/d4e/class_win32_window/#function-~win32window)**() |
| bool | **[Create](/source-reference/Classes/df/d4e/class_win32_window/#function-create)**(const std::wstring & title, const [Point](/source-reference/Classes/d4/d78/struct_win32_window_1_1_point/) & origin, const [Size](/source-reference/Classes/d1/db6/struct_win32_window_1_1_size/) & size) |
| bool | **[Show](/source-reference/Classes/df/d4e/class_win32_window/#function-show)**() |
| void | **[Destroy](/source-reference/Classes/df/d4e/class_win32_window/#function-destroy)**() |
| void | **[SetChildContent](/source-reference/Classes/df/d4e/class_win32_window/#function-setchildcontent)**(HWND content) |
| HWND | **[GetHandle](/source-reference/Classes/df/d4e/class_win32_window/#function-gethandle)**() |
| void | **[SetQuitOnClose](/source-reference/Classes/df/d4e/class_win32_window/#function-setquitonclose)**(bool quit_on_close) |
| RECT | **[GetClientArea](/source-reference/Classes/df/d4e/class_win32_window/#function-getclientarea)**() |
| | **[Win32Window](/source-reference/Classes/df/d4e/class_win32_window/#function-win32window)**() |
| virtual | **[~Win32Window](/source-reference/Classes/df/d4e/class_win32_window/#function-~win32window)**() |
| bool | **[Create](/source-reference/Classes/df/d4e/class_win32_window/#function-create)**(const std::wstring & title, const [Point](/source-reference/Classes/d4/d78/struct_win32_window_1_1_point/) & origin, const [Size](/source-reference/Classes/d1/db6/struct_win32_window_1_1_size/) & size) |
| bool | **[Show](/source-reference/Classes/df/d4e/class_win32_window/#function-show)**() |
| void | **[Destroy](/source-reference/Classes/df/d4e/class_win32_window/#function-destroy)**() |
| void | **[SetChildContent](/source-reference/Classes/df/d4e/class_win32_window/#function-setchildcontent)**(HWND content) |
| HWND | **[GetHandle](/source-reference/Classes/df/d4e/class_win32_window/#function-gethandle)**() |
| void | **[SetQuitOnClose](/source-reference/Classes/df/d4e/class_win32_window/#function-setquitonclose)**(bool quit_on_close) |
| RECT | **[GetClientArea](/source-reference/Classes/df/d4e/class_win32_window/#function-getclientarea)**() |
| | **[Win32Window](/source-reference/Classes/df/d4e/class_win32_window/#function-win32window)**() |
| virtual | **[~Win32Window](/source-reference/Classes/df/d4e/class_win32_window/#function-~win32window)**() |
| bool | **[Create](/source-reference/Classes/df/d4e/class_win32_window/#function-create)**(const std::wstring & title, const [Point](/source-reference/Classes/d4/d78/struct_win32_window_1_1_point/) & origin, const [Size](/source-reference/Classes/d1/db6/struct_win32_window_1_1_size/) & size) |
| bool | **[Show](/source-reference/Classes/df/d4e/class_win32_window/#function-show)**() |
| void | **[Destroy](/source-reference/Classes/df/d4e/class_win32_window/#function-destroy)**() |
| void | **[SetChildContent](/source-reference/Classes/df/d4e/class_win32_window/#function-setchildcontent)**(HWND content) |
| HWND | **[GetHandle](/source-reference/Classes/df/d4e/class_win32_window/#function-gethandle)**() |
| void | **[SetQuitOnClose](/source-reference/Classes/df/d4e/class_win32_window/#function-setquitonclose)**(bool quit_on_close) |
| RECT | **[GetClientArea](/source-reference/Classes/df/d4e/class_win32_window/#function-getclientarea)**() |

## Protected Functions

|                | Name           |
| -------------- | -------------- |
| virtual LRESULT | **[MessageHandler](/source-reference/Classes/df/d4e/class_win32_window/#function-messagehandler)**(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam) |
| virtual bool | **[OnCreate](/source-reference/Classes/df/d4e/class_win32_window/#function-oncreate)**() |
| virtual void | **[OnDestroy](/source-reference/Classes/df/d4e/class_win32_window/#function-ondestroy)**() |
| virtual LRESULT | **[MessageHandler](/source-reference/Classes/df/d4e/class_win32_window/#function-messagehandler)**(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam) |
| virtual bool | **[OnCreate](/source-reference/Classes/df/d4e/class_win32_window/#function-oncreate)**() |
| virtual void | **[OnDestroy](/source-reference/Classes/df/d4e/class_win32_window/#function-ondestroy)**() |
| virtual LRESULT | **[MessageHandler](/source-reference/Classes/df/d4e/class_win32_window/#function-messagehandler)**(HWND window, UINT const message, WPARAM const wparam, LPARAM const lparam) |
| virtual bool | **[OnCreate](/source-reference/Classes/df/d4e/class_win32_window/#function-oncreate)**() |
| virtual void | **[OnDestroy](/source-reference/Classes/df/d4e/class_win32_window/#function-ondestroy)**() |

## Friends

|                | Name           |
| -------------- | -------------- |
| class | **[WindowClassRegistrar](/source-reference/Classes/df/d4e/class_win32_window/#friend-windowclassregistrar)**  |

## Public Functions Documentation

### function Win32Window

```cpp
Win32Window()
```


### function ~Win32Window

```cpp
virtual ~Win32Window()
```


### function Create

```cpp
bool Create(
    const std::wstring & title,
    const Point & origin,
    const Size & size
)
```


### function Show

```cpp
bool Show()
```


### function Destroy

```cpp
void Destroy()
```


### function SetChildContent

```cpp
void SetChildContent(
    HWND content
)
```


### function GetHandle

```cpp
HWND GetHandle()
```


### function SetQuitOnClose

```cpp
void SetQuitOnClose(
    bool quit_on_close
)
```


### function GetClientArea

```cpp
RECT GetClientArea()
```


### function Win32Window

```cpp
Win32Window()
```


### function ~Win32Window

```cpp
virtual ~Win32Window()
```


### function Create

```cpp
bool Create(
    const std::wstring & title,
    const Point & origin,
    const Size & size
)
```


### function Show

```cpp
bool Show()
```


### function Destroy

```cpp
void Destroy()
```


### function SetChildContent

```cpp
void SetChildContent(
    HWND content
)
```


### function GetHandle

```cpp
HWND GetHandle()
```


### function SetQuitOnClose

```cpp
void SetQuitOnClose(
    bool quit_on_close
)
```


### function GetClientArea

```cpp
RECT GetClientArea()
```


### function Win32Window

```cpp
Win32Window()
```


### function ~Win32Window

```cpp
virtual ~Win32Window()
```


### function Create

```cpp
bool Create(
    const std::wstring & title,
    const Point & origin,
    const Size & size
)
```


### function Show

```cpp
bool Show()
```


### function Destroy

```cpp
void Destroy()
```


### function SetChildContent

```cpp
void SetChildContent(
    HWND content
)
```


### function GetHandle

```cpp
HWND GetHandle()
```


### function SetQuitOnClose

```cpp
void SetQuitOnClose(
    bool quit_on_close
)
```


### function GetClientArea

```cpp
RECT GetClientArea()
```


## Protected Functions Documentation

### function MessageHandler

```cpp
virtual LRESULT MessageHandler(
    HWND window,
    UINT const message,
    WPARAM const wparam,
    LPARAM const lparam
)
```


**Reimplemented by**: [FlutterWindow::MessageHandler](/source-reference/Classes/d0/df0/class_flutter_window/#function-messagehandler), [FlutterWindow::MessageHandler](/source-reference/Classes/d0/df0/class_flutter_window/#function-messagehandler), [FlutterWindow::MessageHandler](/source-reference/Classes/d0/df0/class_flutter_window/#function-messagehandler)


### function OnCreate

```cpp
virtual bool OnCreate()
```


**Reimplemented by**: [FlutterWindow::OnCreate](/source-reference/Classes/d0/df0/class_flutter_window/#function-oncreate), [FlutterWindow::OnCreate](/source-reference/Classes/d0/df0/class_flutter_window/#function-oncreate), [FlutterWindow::OnCreate](/source-reference/Classes/d0/df0/class_flutter_window/#function-oncreate)


### function OnDestroy

```cpp
virtual void OnDestroy()
```


**Reimplemented by**: [FlutterWindow::OnDestroy](/source-reference/Classes/d0/df0/class_flutter_window/#function-ondestroy), [FlutterWindow::OnDestroy](/source-reference/Classes/d0/df0/class_flutter_window/#function-ondestroy), [FlutterWindow::OnDestroy](/source-reference/Classes/d0/df0/class_flutter_window/#function-ondestroy)


### function MessageHandler

```cpp
virtual LRESULT MessageHandler(
    HWND window,
    UINT const message,
    WPARAM const wparam,
    LPARAM const lparam
)
```


**Reimplemented by**: [FlutterWindow::MessageHandler](/source-reference/Classes/d0/df0/class_flutter_window/#function-messagehandler), [FlutterWindow::MessageHandler](/source-reference/Classes/d0/df0/class_flutter_window/#function-messagehandler), [FlutterWindow::MessageHandler](/source-reference/Classes/d0/df0/class_flutter_window/#function-messagehandler)


### function OnCreate

```cpp
virtual bool OnCreate()
```


**Reimplemented by**: [FlutterWindow::OnCreate](/source-reference/Classes/d0/df0/class_flutter_window/#function-oncreate), [FlutterWindow::OnCreate](/source-reference/Classes/d0/df0/class_flutter_window/#function-oncreate), [FlutterWindow::OnCreate](/source-reference/Classes/d0/df0/class_flutter_window/#function-oncreate)


### function OnDestroy

```cpp
virtual void OnDestroy()
```


**Reimplemented by**: [FlutterWindow::OnDestroy](/source-reference/Classes/d0/df0/class_flutter_window/#function-ondestroy), [FlutterWindow::OnDestroy](/source-reference/Classes/d0/df0/class_flutter_window/#function-ondestroy), [FlutterWindow::OnDestroy](/source-reference/Classes/d0/df0/class_flutter_window/#function-ondestroy)


### function MessageHandler

```cpp
virtual LRESULT MessageHandler(
    HWND window,
    UINT const message,
    WPARAM const wparam,
    LPARAM const lparam
)
```


**Reimplemented by**: [FlutterWindow::MessageHandler](/source-reference/Classes/d0/df0/class_flutter_window/#function-messagehandler), [FlutterWindow::MessageHandler](/source-reference/Classes/d0/df0/class_flutter_window/#function-messagehandler), [FlutterWindow::MessageHandler](/source-reference/Classes/d0/df0/class_flutter_window/#function-messagehandler)


### function OnCreate

```cpp
virtual bool OnCreate()
```


**Reimplemented by**: [FlutterWindow::OnCreate](/source-reference/Classes/d0/df0/class_flutter_window/#function-oncreate), [FlutterWindow::OnCreate](/source-reference/Classes/d0/df0/class_flutter_window/#function-oncreate), [FlutterWindow::OnCreate](/source-reference/Classes/d0/df0/class_flutter_window/#function-oncreate)


### function OnDestroy

```cpp
virtual void OnDestroy()
```


**Reimplemented by**: [FlutterWindow::OnDestroy](/source-reference/Classes/d0/df0/class_flutter_window/#function-ondestroy), [FlutterWindow::OnDestroy](/source-reference/Classes/d0/df0/class_flutter_window/#function-ondestroy), [FlutterWindow::OnDestroy](/source-reference/Classes/d0/df0/class_flutter_window/#function-ondestroy)


## Friends

### friend WindowClassRegistrar

```cpp
friend class WindowClassRegistrar(
    WindowClassRegistrar 
);
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700