---
title: GNUS-NEO-SWARM/src/genius_elm_chat_completions.h

---

# GNUS-NEO-SWARM/src/genius_elm_chat_completions.h





## Functions

|                | Name           |
| -------------- | -------------- |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_api) int | **[GeniusElmInit](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#function-geniuselminit)**(const char * modelPath, const char * knowledgePath)<br/>Initialises the Genius ELM engine.  |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_api) char * | **[GeniusElmChatCompletionsCreate](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#function-geniuselmchatcompletionscreate)**(const char * requestJson)<br/>Creates an OpenAI v1-style chat completion response.  |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_api) void | **[GeniusElmStringFree](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#function-geniuselmstringfree)**(char * value)<br/>Releases a string buffer returned by the chat FFI API.  |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_api) char * | **[GeniusElmGetStatus](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#function-geniuselmgetstatus)**(void )<br/>Returns the current engine status as a JSON string.  |

## Defines

|                | Name           |
| -------------- | -------------- |
|  | **[NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_api)**  |
|  | **[NEOSWARM_ELM_CHAT_C_NOEXCEPT](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_noexcept)**  |


## Functions Documentation

### function GeniusElmInit

```cpp
NEOSWARM_ELM_CHAT_C_API int GeniusElmInit(
    const char * modelPath,
    const char * knowledgePath
)
```

Initialises the Genius ELM engine. 

**Parameters**: 

  * **modelPath** Path to the [MNN](/source-reference/Namespaces/d1/d90/namespace_m_n_n/) model file, or NULL for stub mode. 
  * **knowledgePath** Path to a Grokipedia facts CSV, or NULL to disable. 


**Return**: 0 on success, -1 if ApiServer initialization fails. 

Creates and initialises an ApiServer instance with the given model and knowledge paths. Must be called before `GeniusElmChatCompletionsCreate` for real inference; falls back to stub mode if not called.

Thread-safe: may be called multiple times. Subsequent calls are no-ops.


### function GeniusElmChatCompletionsCreate

```cpp
NEOSWARM_ELM_CHAT_C_API char * GeniusElmChatCompletionsCreate(
    const char * requestJson
)
```

Creates an OpenAI v1-style chat completion response. 

**Parameters**: 

  * **requestJson** UTF-8 JSON request in OpenAI v1 format, or NULL. 


**Return**: Heap-allocated UTF-8 JSON string. Caller must release with `GeniusElmStringFree`. Returns NULL only on allocation failure. 

Parses the last user message from `requestJson` via nlohmann::json, dispatches through the ApiServer pipeline (router → inference → optional specialist), and returns a JSON chat completion.

Falls back to a stub response if GeniusElmInit has not been called or if the ApiServer fails to process the request.

Thread-safe via global mutex.


### function GeniusElmStringFree

```cpp
NEOSWARM_ELM_CHAT_C_API void GeniusElmStringFree(
    char * value
)
```

Releases a string buffer returned by the chat FFI API. 

**Parameters**: 

  * **value** Heap-allocated string returned by `GeniusElmChatCompletionsCreate`. NULL is allowed. 


### function GeniusElmGetStatus

```cpp
NEOSWARM_ELM_CHAT_C_API char * GeniusElmGetStatus(
    void 
)
```

Returns the current engine status as a JSON string. 

**Return**: Heap-allocated UTF-8 JSON string. Caller must release with `GeniusElmStringFree`. Returns NULL only on allocation failure. 

The returned JSON contains:

* "model_loaded": bool
* "mode": string — "active", "idle", or "stub"
* "backend": string — "cpu", "vulkan", or "none"
* "node_id": string — local node identifier
* "supergenius_connected": bool
* "fallback_active": bool

Thread-safe via global mutex.




## Macros Documentation

### define NEOSWARM_ELM_CHAT_C_API

```cpp
#define NEOSWARM_ELM_CHAT_C_API 
```


### define NEOSWARM_ELM_CHAT_C_NOEXCEPT

```cpp
#define NEOSWARM_ELM_CHAT_C_NOEXCEPT 
```


## Source code

```cpp
#ifndef GNUS_NEO_SWARM_GENIUS_ELM_CHAT_C_H
#define GNUS_NEO_SWARM_GENIUS_ELM_CHAT_C_H

#include <stddef.h>

#if defined( _WIN32 )
#if defined( NEOSWARM_CHAT_C_EXPORTS )
#define NEOSWARM_ELM_CHAT_C_API __declspec( dllexport )
#else
#define NEOSWARM_ELM_CHAT_C_API __declspec( dllimport )
#endif
#else
#define NEOSWARM_ELM_CHAT_C_API
#endif

#if defined( __cplusplus )
#define NEOSWARM_ELM_CHAT_C_NOEXCEPT noexcept
extern "C"
{
#else
#define NEOSWARM_ELM_CHAT_C_NOEXCEPT
#endif

    NEOSWARM_ELM_CHAT_C_API int GeniusElmInit( const char* modelPath,
                                             const char* knowledgePath ) NEOSWARM_ELM_CHAT_C_NOEXCEPT;

    NEOSWARM_ELM_CHAT_C_API char* GeniusElmChatCompletionsCreate( const char* requestJson ) NEOSWARM_ELM_CHAT_C_NOEXCEPT;

    NEOSWARM_ELM_CHAT_C_API void GeniusElmStringFree( char* value ) NEOSWARM_ELM_CHAT_C_NOEXCEPT;

    NEOSWARM_ELM_CHAT_C_API char* GeniusElmGetStatus( void ) NEOSWARM_ELM_CHAT_C_NOEXCEPT;

#if defined( __cplusplus )
}
#endif

#endif // GNUS_NEO_SWARM_GENIUS_ELM_CHAT_C_H
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
