---
title: GNUS-NEO-SWARM/flutter_slm_bridge/example/windows/runner/utils.h

---

# GNUS-NEO-SWARM/flutter_slm_bridge/example/windows/runner/utils.h





## Functions

|                | Name           |
| -------------- | -------------- |
| void | **[CreateAndAttachConsole](/source-reference/Files/d1/da0/flutter__slm__bridge_2example_2windows_2runner_2utils_8h/#function-createandattachconsole)**() |
| std::string | **[Utf8FromUtf16](/source-reference/Files/d1/da0/flutter__slm__bridge_2example_2windows_2runner_2utils_8h/#function-utf8fromutf16)**(const wchar_t * utf16_string) |
| std::vector< std::string > | **[GetCommandLineArguments](/source-reference/Files/d1/da0/flutter__slm__bridge_2example_2windows_2runner_2utils_8h/#function-getcommandlinearguments)**() |


## Functions Documentation

### function CreateAndAttachConsole

```cpp
void CreateAndAttachConsole()
```


### function Utf8FromUtf16

```cpp
std::string Utf8FromUtf16(
    const wchar_t * utf16_string
)
```


### function GetCommandLineArguments

```cpp
std::vector< std::string > GetCommandLineArguments()
```




## Source code

```cpp
#ifndef RUNNER_UTILS_H_
#define RUNNER_UTILS_H_

#include <string>
#include <vector>

// Creates a console for the process, and redirects stdout and stderr to
// it for both the runner and the Flutter library.
void CreateAndAttachConsole();

// Takes a null-terminated wchar_t* encoded in UTF-16 and returns a std::string
// encoded in UTF-8. Returns an empty std::string on failure.
std::string Utf8FromUtf16(const wchar_t* utf16_string);

// Gets the command line arguments passed in as a std::vector<std::string>,
// encoded in UTF-8. Returns an empty std::vector<std::string> on failure.
std::vector<std::string> GetCommandLineArguments();

#endif  // RUNNER_UTILS_H_
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
