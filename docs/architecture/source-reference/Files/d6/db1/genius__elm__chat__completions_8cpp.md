---
title: GNUS-NEO-SWARM/src/genius_elm_chat_completions.cpp

---

# GNUS-NEO-SWARM/src/genius_elm_chat_completions.cpp





## Functions

|                | Name           |
| -------------- | -------------- |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm-elm-chat-c-api) int | **[GeniusElmInit](/source-reference/Files/d6/db1/genius__elm__chat__completions_8cpp/#function-geniuselminit)**(const char * modelPath, const char * knowledgePath)<br/>Initialises the Genius ELM engine.  |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm-elm-chat-c-api) char * | **[GeniusElmChatCompletionsCreate](/source-reference/Files/d6/db1/genius__elm__chat__completions_8cpp/#function-geniuselmchatcompletionscreate)**(const char * requestJson)<br/>Creates an OpenAI v1-style chat completion response.  |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm-elm-chat-c-api) void | **[GeniusElmStringFree](/source-reference/Files/d6/db1/genius__elm__chat__completions_8cpp/#function-geniuselmstringfree)**(char * value)<br/>Releases a string buffer returned by the chat FFI API.  |
| [NEOSWARM_ELM_CHAT_C_API](/source-reference/Files/d3/deb/genius__elm__chat__completions_8h/#define-neoswarm-elm-chat-c-api) char * | **[GeniusElmGetStatus](/source-reference/Files/d6/db1/genius__elm__chat__completions_8cpp/#function-geniuselmgetstatus)**(void )<br/>Returns the current engine status as a JSON string.  |


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

#include "api/api_server.hpp"
#include "common/types.hpp"

#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <nlohmann/json.hpp>

namespace
{
    std::mutex g_mutex;
    std::unique_ptr<sgns::neoswarm::api::ApiServer> g_server;

    char* AllocCopy( const std::string& src )
    {
        const auto len = src.size();
        auto* dst = static_cast<char*>( std::malloc( len + 1 ) );
        if ( dst != nullptr )
        {
            std::memcpy( dst, src.data(), len );
            dst[ len ] = '\0';
        }
        return dst;
    }

    constexpr std::string_view kStubChatJson = R"({
  "id": "chatcmpl-stub",
  "object": "chat.completion",
  "created": 0,
  "model": "neoswarm-elm-stub",
  "choices": [
    {
      "index": 0,
      "message": {
        "role": "assistant",
        "content": "NeoSwarm ELM is running in stub mode."
      },
      "finish_reason": "stop"
    }
  ],
  "usage": {
    "prompt_tokens": 0,
    "completion_tokens": 0,
    "total_tokens": 0
  }
})";

    constexpr std::string_view kStatusJsonStub = R"({
  "model_loaded": false,
  "mode": "stub",
  "backend": "none",
  "node_id": "stub",
  "supergenius_connected": false,
  "fallback_active": true
})";

    std::string BuildChatResponseJson( const sgns::neoswarm::InferenceResponse& resp )
    {
        nlohmann::json j;
        j[ "id" ]       = "chatcmpl-" + resp.m_taskId;
        j[ "object" ]   = "chat.completion";
        j[ "created" ]  = 0;
        j[ "model" ]    = "genius-elm-v1";
        j[ "choices" ]  = nlohmann::json::array( { { { "index", 0 },
                                                    { "message",
                                                      { { "role", "assistant" },
                                                        { "content", resp.m_output } } },
                                                    { "finish_reason", resp.m_success ? "stop" : "error" } } } );
        j[ "usage" ] = { { "prompt_tokens", 0 }, { "completion_tokens", 0 }, { "total_tokens", 0 } };
        return j.dump();
    }

    std::string BuildStatusJson()
    {
        std::lock_guard<std::mutex> lock( g_mutex );
        if ( !g_server )
        {
            return kStatusJsonStub;
        }

        nlohmann::json j;
        j[ "model_loaded" ]         = false; // ApiServer doesn't expose this directly
        j[ "mode" ]                 = g_server->IsRunning() ? "active" : "idle";
        j[ "backend" ]              = "cpu";
        j[ "node_id" ]              = "local";
        j[ "supergenius_connected" ] = g_server->IsSuperGeniusConnected();
        j[ "fallback_active" ]      = false;
        return j.dump();
    }

    std::string ExtractPrompt( const std::string& requestJson )
    {
        if ( requestJson.empty() )
        {
            return "";
        }

        try
        {
            auto j = nlohmann::json::parse( requestJson );
            if ( j.contains( "messages" ) && j[ "messages" ].is_array() )
            {
                for ( auto it = j[ "messages" ].rbegin(); it != j[ "messages" ].rend(); ++it )
                {
                    if ( it->contains( "role" ) && ( *it )[ "role" ] == "user" && it->contains( "content" ) )
                    {
                        return ( *it )[ "content" ].get<std::string>();
                    }
                }
            }
            if ( j.contains( "prompt" ) )
            {
                return j[ "prompt" ].get<std::string>();
            }
        }
        catch ( const nlohmann::json::exception& )
        {
            // Fall through — return empty prompt
        }

        return "";
    }

} // anonymous namespace

extern "C"
{
    NEOSWARM_ELM_CHAT_C_API int GeniusElmInit( const char* modelPath,
                                              const char* knowledgePath ) NEOSWARM_ELM_CHAT_C_NOEXCEPT
    {
        std::lock_guard<std::mutex> lock( g_mutex );

        if ( g_server )
        {
            return 0; // already initialized
        }

        sgns::neoswarm::api::ApiServer::Config cfg;
        if ( modelPath != nullptr && modelPath[ 0 ] != '\0' )
        {
            cfg.m_modelPath = modelPath;
        }
        if ( knowledgePath != nullptr && knowledgePath[ 0 ] != '\0' )
        {
            cfg.m_knowledgeFacts = knowledgePath;
        }
        cfg.m_enableNetwork = false;

        auto server = std::make_unique<sgns::neoswarm::api::ApiServer>( std::move( cfg ) );
        auto result = server->Initialize();
        if ( !result.has_value() )
        {
            return -1;
        }

        g_server = std::move( server );
        return 0;
    }

    NEOSWARM_ELM_CHAT_C_API char*
        GeniusElmChatCompletionsCreate( const char* requestJson ) NEOSWARM_ELM_CHAT_C_NOEXCEPT
    {
        std::lock_guard<std::mutex> lock( g_mutex );

        if ( !g_server )
        {
            return AllocCopy( kStubChatJson );
        }

        std::string prompt;
        if ( requestJson != nullptr )
        {
            prompt = ExtractPrompt( requestJson );
        }

        if ( prompt.empty() )
        {
            return AllocCopy( kStubChatJson );
        }

        sgns::neoswarm::Task task;
        task.m_id     = "ffi-" + std::to_string( std::hash<std::string>{}( prompt ) );
        task.m_prompt = std::move( prompt );
        task.m_mode   = sgns::neoswarm::ExecutionMode::SingleNode;

        auto result = g_server->Process( task );
        if ( !result.has_value() )
        {
            return AllocCopy( kStubChatJson );
        }

        return AllocCopy( BuildChatResponseJson( result.value() ) );
    }

    NEOSWARM_ELM_CHAT_C_API void GeniusElmStringFree( char* value ) NEOSWARM_ELM_CHAT_C_NOEXCEPT
    {
        std::free( value );
    }

    NEOSWARM_ELM_CHAT_C_API char* GeniusElmGetStatus( void ) NEOSWARM_ELM_CHAT_C_NOEXCEPT
    {
        return AllocCopy( BuildStatusJson() );
    }
} // extern "C"
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700
