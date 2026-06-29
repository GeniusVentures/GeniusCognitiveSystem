---
title: GNUS-NEO-SWARM/src/genius_elm_chat_c.cpp
summary: C FFI entry point stub for GeniusElmInit / GeniusElmChatCompletionsCreate / GeniusElmStringFree / GeniusElmGetStatus. 

---

# GNUS-NEO-SWARM/src/genius_elm_chat_c.cpp



C FFI entry point stub for GeniusElmInit / GeniusElmChatCompletionsCreate / GeniusElmStringFree / GeniusElmGetStatus.  [More...](#detailed-description)

## Functions

|                | Name           |
| -------------- | -------------- |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_api) int | **[GeniusElmInit](/source-reference/Files/d8/d0f/genius__elm__chat__c_8cpp/#function-geniuselminit)**(const char * modelPath, const char * knowledgePath)<br/>Initialises the Genius ELM engine.  |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_api) char * | **[GeniusElmChatCompletionsCreate](/source-reference/Files/d8/d0f/genius__elm__chat__c_8cpp/#function-geniuselmchatcompletionscreate)**(const char * requestJson)<br/>Creates an OpenAI v1-style chat completion response.  |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_api) void | **[GeniusElmStringFree](/source-reference/Files/d8/d0f/genius__elm__chat__c_8cpp/#function-geniuselmstringfree)**(char * value)<br/>Releases a string buffer returned by the chat FFI API.  |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm_elm_chat_c_api) char * | **[GeniusElmGetStatus](/source-reference/Files/d8/d0f/genius__elm__chat__c_8cpp/#function-geniuselmgetstatus)**(void )<br/>Returns the current engine status as a JSON string.  |

## Detailed Description

C FFI entry point stub for GeniusElmInit / GeniusElmChatCompletionsCreate / GeniusElmStringFree / GeniusElmGetStatus. 

**Date**: 

  * 2026-06-10
  * 2026-06-15


C FFI entry points for Genius ELM chat completions API.


Returns canned responses until the real ApiServer pipeline is wired. The shared library is consumed by the Flutter bridge.


Wires the FFI surface to the real ApiServer pipeline. Thread-safe: all FFI calls are protected by a global mutex. Degrades gracefully: returns stub responses when no model is loaded. 


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




## Source code

```cpp


#include "genius_elm_chat_completions.h"

#include <cstdlib>
#include <cstring>
#include <string>

namespace
{
    char* DuplicateString( const std::string& value ) noexcept
    {
        const size_t size = value.size() + 1U;
        char*        buf  = static_cast<char*>( std::malloc( size ) );

        if ( buf == nullptr )
        {
            return nullptr;
        }

        std::memcpy( buf, value.data(), size );
        return buf;
    }

    static const char kStubChatJson[] =
        "{\"id\":\"chatcmpl-stub\",\"object\":\"chat.completion\","
        "\"created\":0,\"model\":\"elm-v1\","
        "\"choices\":[{\"index\":0,\"message\":{"
        "\"role\":\"assistant\",\"content\":\"[ELM stub - engine not wired]\"},"
        "\"finish_reason\":\"stop\"}],"
        "\"usage\":{\"prompt_tokens\":0,\"completion_tokens\":0,\"total_tokens\":0}}";

    static const char kStubStatusJson[] =
        "{\"model_loaded\":false,\"mode\":\"stub\",\"backend\":\"none\",\"node_id\":\"stub\"}";

    static bool g_initialized = false;
} // namespace

NEOSWARM_ELM_CHAT_C_API int GeniusElmInit( const char* /* modelPath */, const char* /* knowledgePath */ )
    NEOSWARM_ELM_CHAT_C_NOEXCEPT
{
    g_initialized = true;
    return 0;
}

NEOSWARM_ELM_CHAT_C_API char* GeniusElmChatCompletionsCreate( const char* /* requestJson */ )
    NEOSWARM_ELM_CHAT_C_NOEXCEPT
{
    return DuplicateString( kStubChatJson );
}

NEOSWARM_ELM_CHAT_C_API void GeniusElmStringFree( char* value ) NEOSWARM_ELM_CHAT_C_NOEXCEPT
{
    std::free( value );
}

NEOSWARM_ELM_CHAT_C_API char* GeniusElmGetStatus( void ) NEOSWARM_ELM_CHAT_C_NOEXCEPT
{
    return DuplicateString( kStubStatusJson );
}
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
