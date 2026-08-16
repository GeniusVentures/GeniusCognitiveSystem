# Phase 1: Foundation - Pattern Map

**Mapped:** 2026-08-15
**Files analyzed:** 11 (9 new + 2 modified in GCS root; 1 submodule addition pending user approval)
**Analogs found:** 10 / 11

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `src/CMakeLists.txt` (modify) | config (CMake target) | build-graph | `GNUS-NEO-SWARM/src/storage/CMakeLists.txt` | exact (hard-required link idiom) |
| `src/lib/gcs_core.cpp` (modify) | service (core session owner) | request-response (sync wrapper around async CRDT) | `GNUS-NEO-SWARM/src/storage/gcs_global_db.{hpp,cpp}` | exact (component lifecycle) |
| `src/ffi/CMakeLists.txt` (new) | config (CMake SHARED lib) | build-graph | `GNUS-NEO-SWARM/neoswarm_ffi/src/CMakeLists.txt` + `src/storage/CMakeLists.txt` | exact (SHARED target) + role-match (export define) |
| `src/ffi/gcs_core.h` (new) | FFI C header (opaque handle) | request-response | `GNUS-NEO-SWARM/src/genius_elm_chat_completions.h` | exact |
| `src/ffi/gcs_core_ffi.cpp` (new) | FFI thunk | request-response | `GNUS-NEO-SWARM/src/genius_elm_chat_completions.cpp` | exact |
| `test/CMakeLists.txt` (new) | config (test registration) | build-graph | `GNUS-NEO-SWARM/test/CMakeLists.txt` | exact |
| `test/test_wait_condition.hpp` (new) | test utility | request-response | `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp:72-90` | exact (copy verbatim) |
| `test/test_gcs_core_smoke.cpp` (new) | test | event-driven (CRDT pub/sub round-trip) | `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp` | exact |
| `test/test_gcs_ffi.cpp` (new) | test | request-response (FFI echo) | `GNUS-NEO-SWARM/test/ffi/test_genius_elm_ffi.cpp` | exact |
| `.github/workflows/cmake.yml` (new) | config (CI) | batch (matrix build/test/release) | `../GeniusSDK/.github/workflows/cmake.yml` | exact (consumer-repo variant) |
| `GNUS-NEO-SWARM/src/storage/gcs_global_db.{hpp,cpp}` (submodule — add Put/Get/AddBroadcastTopic accessors) | service (CRDT pass-through) | CRUD | SuperGenius `src/crdt/globaldb/globaldb.hpp` (underlying API surface) | role-match |

## Pattern Assignments

### `src/CMakeLists.txt` (modify — extend existing target)

**Analog:** `GNUS-NEO-SWARM/src/storage/CMakeLists.txt`

**Current state** (src/CMakeLists.txt lines 1-16 — what we extend):
```cmake
add_library(gcs_core
        lib/gcs_core.cpp
)

target_include_directories(gcs_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gcs_core PUBLIC
    spdlog::spdlog
    fmt::fmt
)
```

**Hard-required linkage idiom to insert** (lines 14-18 of analog):
```cmake
# neoswarm_storage — hard required (project Conditional Compilation rule:
# missing required libraries fail at configure time, never degrade to a stub).
if(TARGET neoswarm_storage)
    target_link_libraries(gcs_core PUBLIC neoswarm_storage)
else()
    message(FATAL_ERROR "neoswarm_storage target not found — GNUS-NEO-SWARM must be configured before gcs_core")
endif()
```

**Pattern notes:**
- `neoswarm_storage` brings `sgns::crdt_globaldb` + `sgns::GeniusSDK_shared` transitively as `PUBLIC` links (verified: analog lines 14-47), so `gcs_core` needs no additional include dirs.
- Add `add_subdirectory(ffi)` after the `gcs_core` block to wire the new FFI target.

---

### `src/ffi/CMakeLists.txt` (new — SHARED lib target)

**Analog A (SHARED idiom):** `GNUS-NEO-SWARM/neoswarm_ffi/src/CMakeLists.txt` (lines 8-22):
```cmake
add_library(flutter_slm_bridge SHARED
  "flutter_slm_bridge.c"
)

set_target_properties(flutter_slm_bridge PROPERTIES
  PUBLIC_HEADER flutter_slm_bridge.h
  OUTPUT_NAME "flutter_slm_bridge"
)

target_compile_definitions(flutter_slm_bridge PUBLIC DART_SHARED_LIB)

if (ANDROID)
  # Support Android 15 16k page size
  target_link_options(flutter_slm_bridge PRIVATE "-Wl,-z,max-page-size=16384")
endif()
```

**Analog B (export-define + linkage):** pattern synthesized from `src/storage/CMakeLists.txt` + `genius_elm_chat_completions.h`:
```cmake
add_library(gcs_ffi SHARED
    gcs_core_ffi.cpp
)

set_target_properties(gcs_ffi PROPERTIES
    PUBLIC_HEADER gcs_core.h
    OUTPUT_NAME "gcs_ffi"
)

target_link_libraries(gcs_ffi PRIVATE gcs_core)
target_compile_definitions(gcs_ffi PRIVATE GCS_FFI_EXPORTS)   # dllexport macro

if(ANDROID)
    # Support Android 15 16k page size (mirrors neoswarm_ffi)
    target_link_options(gcs_ffi PRIVATE "-Wl,-z,max-page-size=16384")
endif()
```

**Pattern notes:**
- `PRIVATE` linkage on `gcs_core` (not `PUBLIC`) — `gcs_ffi` is a leaf; nothing links to it from C++.
- Export define uses `PRIVATE` scope so consumers see `dllimport`.
- Unlike the neoswarm plugin template (which sets `cmake_minimum_required(VERSION 3.10)` for Flutter plugin customers), GCS's `src/ffi/CMakeLists.txt` inherits the parent build's cmake version — do **not** re-declare `cmake_minimum_required` or `project()` here.

---

### `src/ffi/gcs_core.h` (new — single opaque-handle C API)

**Analog:** `GNUS-NEO-SWARM/src/genius_elm_chat_completions.h` (full file — 103 lines)

**Export-macro + extern "C" guard** (lines 6-22):
```c
#if defined( _WIN32 )
#if defined( GCS_FFI_EXPORTS )
#define GCS_FFI_API __declspec( dllexport )
#else
#define GCS_FFI_API __declspec( dllimport )
#endif
#else
#define GCS_FFI_API
#endif

#if defined( __cplusplus )
#define GCS_FFI_NOEXCEPT noexcept
extern "C"
{
#else
#define GCS_FFI_NOEXCEPT
#endif
```

**Header guard + stdint** (lines 1-4):
```c
#ifndef GENIUS_COGNITIVE_SYSTEM_GCS_CORE_H
#define GENIUS_COGNITIVE_SYSTEM_GCS_CORE_H

#include <stddef.h>
#include <stdint.h>   /* int64_t for Dart NativePort */
```

**Opaque-handle session API skeleton** (style modeled on lines 37, 56, 64, 96 — note Doxygen `\brief`, `\param`, `\return`, and trailing `NOEXCEPT`):
```c
    /**
     * \brief Initialises a GCS core session.
     *
     * Creates the C++-side session object that owns GcsGlobalDb. Thread-safe:
     * may be called multiple times; subsequent calls are no-ops returning the
     * existing handle.
     *
     * \param dbPath  Path for the CRDT store, or NULL for the default.
     * \return Opaque session handle on success, NULL on failure.
     */
    GCS_FFI_API GcsSession* gcs_init( const char* dbPath ) GCS_FFI_NOEXCEPT;

    GCS_FFI_API int  gcs_join_topic( GcsSession* session, const char* topic ) GCS_FFI_NOEXCEPT;
    GCS_FFI_API int  gcs_publish( GcsSession* session, const char* topic,
                                  const char* utf8Payload ) GCS_FFI_NOEXCEPT;
    GCS_FFI_API int  gcs_on_message( GcsSession* session, int64_t dartPort ) GCS_FFI_NOEXCEPT;
    GCS_FFI_API void gcs_shutdown( GcsSession* session ) GCS_FFI_NOEXCEPT;

    /* Paired free for any heap strings returned by the API (mirrors GeniusElmStringFree) */
    GCS_FFI_API void gcs_string_free( char* value ) GCS_FFI_NOEXCEPT;
```

**Closing guard** (lines 98-102):
```c
#if defined( __cplusplus )
}
#endif

#endif // GENIUS_COGNITIVE_SYSTEM_GCS_CORE_H
```

**Pattern notes:**
- Opaque type: `typedef struct GcsSession GcsSession;` — declared only, defined in the .cpp.
- This is the **only** permitted `_WIN32` ifdef — the export-macro block. Project rule forbids OS ifdefs in logic; this is the established carve-out (confirmed in RESEARCH § Build System Wiring and CONTEXT § Established Patterns).
- The paired `gcs_string_free` export prevents Windows cross-allocator heap mismatch (per "Don't Hand-Roll" in RESEARCH).

---

### `src/ffi/gcs_core_ffi.cpp` (new — FFI thunk)

**Analog:** `GNUS-NEO-SWARM/src/genius_elm_chat_completions.cpp` (227 lines)

**Includes + global state pattern** (lines 10-26):
```cpp
#include "gcs_core.h"

#include "lib/gcs_core.hpp"   // C++ session implementation

#include <cstdlib>
#include <cstring>
#include <memory>
#include <mutex>

namespace
{
    std::mutex g_mutex;
    // Opaque handle = pointer into this map. Session state owned here, not in Dart.
    std::unique_ptr<gcs::CoreSession> g_session;   // Phase 1: single global session (mirrors g_server)
}
```

**Heap-string AllocCopy helper** (lines 28-38 — copy verbatim, rename only):
```cpp
char* AllocCopy( const std::string_view src )
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
```

**extern "C" thunk with mutex + noexcept + null-tolerant** (lines 141-173, 210-225):
```cpp
extern "C"
{
    GCS_FFI_API GcsSession* gcs_init( const char* dbPath ) GCS_FFI_NOEXCEPT
    {
        std::lock_guard<std::mutex> lock( g_mutex );

        if ( g_session )
        {
            return reinterpret_cast<GcsSession*>( g_session.get() );  // idempotent (mirrors line 148-151)
        }

        gcs::CoreSession::Config cfg{};
        if ( dbPath != nullptr && dbPath[ 0 ] != '\0' )
        {
            cfg.m_dbPath = dbPath;
        }

        auto session = std::make_unique<gcs::CoreSession>( std::move( cfg ) );
        auto result  = session->Initialize();
        if ( !result.has_value() )
        {
            return nullptr;   // int → pointer return is the only semantic change vs analog
        }

        g_session = std::move( session );
        return reinterpret_cast<GcsSession*>( g_session.get() );
    }

    GCS_FFI_API void gcs_string_free( char* value ) GCS_FFI_NOEXCEPT
    {
        std::free( value );   // copy of GeniusElmStringFree (line 210-213)
    }

    GCS_FFI_API void gcs_shutdown( GcsSession* session ) GCS_FFI_NOEXCEPT
    {
        std::lock_guard<std::mutex> lock( g_mutex );
        // Unregister dart_port BEFORE teardown (Pitfall 6 in RESEARCH).
        // Shutdown ordering: clear port → Shutdown() joins io thread → free handle.
        if ( g_session && reinterpret_cast<gcs::CoreSession*>( session ) == g_session.get() )
        {
            g_session->UnregisterMessagePort();
            g_session->Shutdown();
            g_session.reset();
        }
    }
}
```

**Pattern notes:**
- `g_session` is `unique_ptr` (not `shared_ptr`) per project ownership rule.
- Every FFI function: `std::lock_guard<std::mutex>` first statement, `noexcept`, no exceptions escape.
- Null `const char*` from Dart is valid input (treated as "use default") — mirrors `GeniusElmInit(nullptr, nullptr)` semantics (line 154-161).

---

### `test/CMakeLists.txt` (new — GCS root test registration)

**Analog:** `GNUS-NEO-SWARM/test/CMakeLists.txt` (89 lines)

**GTest discovery + helper macro pattern** (lines 1-53):
```cmake
find_package(GTest QUIET)

if(NOT GTest_FOUND)
    find_path(GTEST_INCLUDE_DIR gtest/gtest.h
        PATHS ${THIRDPARTY_BUILD_DIR}/GTest/include
              ${CMAKE_SOURCE_DIR}/../thirdparty/GTest/googletest/include
    )
    find_library(GTEST_LIB gtest
        PATHS ${THIRDPARTY_BUILD_DIR}/GTest/lib
    )
    find_library(GTEST_MAIN_LIB gtest_main
        PATHS ${THIRDPARTY_BUILD_DIR}/GTest/lib
    )
    if(GTEST_INCLUDE_DIR AND GTEST_LIB)
        add_library(GTest::GTest UNKNOWN IMPORTED)
        set_target_properties(GTest::GTest PROPERTIES
            IMPORTED_LOCATION ${GTEST_LIB}
            INTERFACE_INCLUDE_DIRECTORIES ${GTEST_INCLUDE_DIR}
        )
        add_library(GTest::Main UNKNOWN IMPORTED)
        set_target_properties(GTest::Main PROPERTIES
            IMPORTED_LOCATION ${GTEST_MAIN_LIB}
        )
        set(GTest_FOUND TRUE)
    endif()
endif()

if(NOT GTest_FOUND)
    message(WARNING "GTest not found — skipping tests")
    return()
endif()

enable_testing()

# Helper macro
macro(gcs_test name sources libs)
    add_executable(${name} ${sources})
    target_link_libraries(${name} PRIVATE
        ${libs}
        GTest::GTest
        GTest::Main
    )
    add_test(NAME ${name} COMMAND ${name})
endmacro()

gcs_test(test_gcs_core_smoke  test_gcs_core_smoke.cpp  "gcs_core;neoswarm_storage;sgns::crdt_globaldb")
gcs_test(test_gcs_ffi         test_gcs_ffi.cpp         "gcs_ffi;gcs_core")
```

**Pattern notes:**
- Rename macro `neoswarm_test` → `gcs_test`; drop the `SUPERGENIUS_TEST_DATA_DIR` define (not needed for smoke tests).
- No benchmark subdirectory this phase.
- Hook is already in place: `build/CommonBuildParameters.cmake:146-151` does `add_subdirectory(${PROJECT_ROOT}/test ...)` when `BUILD_TESTING` is on (RESEARCH § Validation Architecture).

---

### `test/test_wait_condition.hpp` (new — copy verbatim)

**Analog:** `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp:33-91`

**Complete file** (extract + turn into header):
```cpp
/**
 * @file       test_wait_condition.hpp
 * @brief      Wait-condition template (condition_variable polling, no sleep_for).
 *             Copied verbatim from GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp
 *             per project testing rule (no std::this_thread::sleep_for in tests).
 * @date       2026-08-15
 */

#ifndef GCS_TEST_WAIT_CONDITION_HPP
#define GCS_TEST_WAIT_CONDITION_HPP

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace gcs::test
{
    /// Upper bound for a single wait-condition call.
    constexpr std::chrono::milliseconds kWaitTimeout{ 25000 };
    /// Re-check interval for pure polling predicates inside WaitForCondition.
    constexpr std::chrono::milliseconds kPollInterval{ 10 };

    /**
     * @brief Poll `predicate` via condition_variable::wait_for until it returns true
     *        or `timeout` elapses.
     *
     * @param[in] predicate Nullary callable returning bool.
     * @param[in] timeout   Maximum time to wait.
     * @return true if the predicate became true before the deadline; false otherwise.
     */
    inline bool WaitForCondition( const std::function<bool()> &predicate,
                                  std::chrono::milliseconds timeout )
    {
        std::mutex              mtx;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock( mtx );
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while ( std::chrono::steady_clock::now() < deadline )
        {
            if ( predicate() )
            {
                return true;
            }
            const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - std::chrono::steady_clock::now() );
            const auto slice     = std::min( kPollInterval, remaining );
            cv.wait_for( lock, slice );
        }
        return predicate();
    }
} // namespace gcs::test

#endif // GCS_TEST_WAIT_CONDITION_HPP
```

**Pattern notes:**
- Moved out of anonymous namespace into `gcs::test` so both smoke tests can include it.
- `inline` on the function so multiple TUs can include the header without ODR violations.

---

### `test/test_gcs_core_smoke.cpp` (new — GlobalDB lifecycle + CRDT round-trip)

**Analog:** `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp` (full 249 lines — copy structural skeleton verbatim, rename namespaces)

**File-header + includes block** (lines 1-31):
```cpp
/**
 * @file       test_gcs_core_smoke.cpp
 * @brief      Phase 1 smoke tests for the gcs_core session: GlobalDB lifecycle via the
 *             injected-pubsub seam, AddBroadcastTopic+AddListenTopic on a gcs/chat/*
 *             topic, and a Put→Get round-trip over a real GossipPubSub on port 0.
 *             Uses the GCS wait-condition template (no sleep_for).
 * @date       2026-08-15
 */

#include "lib/gcs_core.hpp"
#include "test_wait_condition.hpp"

#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <string>

#include <gtest/gtest.h>

#include "crdt/globaldb/keypair_file_storage.hpp"
#include "ipfs_pubsub/gossip_pubsub.hpp"

#include <libp2p/log/configurator.hpp>
#include <libp2p/log/logger.hpp>

#include <soralog/impl/configurator_from_yaml.hpp>
#include <soralog/logging_system.hpp>
```

**Soralog one-time setup** (lines 46-58 + 108-117 — REQUIRED, otherwise "Logging system is not ready" crash):
```cpp
namespace
{
    /// GossipPubSub bind address used by every test.
    constexpr const char *kListenIp = "0.0.0.0";

    /// Minimal soralog YAML — console sink only, error level.
    constexpr const char *kLoggingYaml = R"(
     sinks:
       - name: console
         type: console
         color: false
     groups:
       - name: gcs_core_smoke_test
         sink: console
         level: error
         children:
           - name: libp2p
           - name: Gossip
    )";
}

// Inside fixture:
static void SetUpTestSuite()
{
    auto loggerConfigurator = std::make_shared<libp2p::log::Configurator>();
    auto configFromYaml     = std::make_shared<soralog::ConfiguratorFromYAML>(
        loggerConfigurator, std::string{ kLoggingYaml } );
    auto loggingSystem      = std::make_shared<soralog::LoggingSystem>( configFromYaml );
    const auto confResult   = loggingSystem->configure();
    ASSERT_FALSE( confResult.has_error ) << "Could not configure test logging system";
    libp2p::log::setLoggingSystem( loggingSystem );
}
```

**Per-test temp dir + TearDown** (lines 119-134):
```cpp
void SetUp() override
{
    const auto *info       = ::testing::UnitTest::GetInstance()->current_test_info();
    const auto  uniqueSalt = std::chrono::steady_clock::now().time_since_epoch().count();
    m_tempPath = ( std::filesystem::temp_directory_path()
                   / ( std::string{ "gcs_core_smoke_" } + info->name() + "_" +
                       std::to_string( uniqueSalt ) ) )
                     .string();
    std::filesystem::create_directories( m_tempPath );
}

void TearDown() override
{
    std::error_code ec;
    std::filesystem::remove_all( m_tempPath, ec );
}
```

**GossipPubSub bring-up helper** (lines 140-158 — port 0 ephemeral):
```cpp
std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> MakeStartedPubSub( const std::string &keyDir )
{
    sgns::crdt::KeyPairFileStorage keyStore( keyDir );
    auto                           keyPairResult = keyStore.GetKeyPair();
    EXPECT_FALSE( keyPairResult.has_error() );
    if ( keyPairResult.has_error() )
    {
        return nullptr;
    }
    auto pubsub      = std::make_shared<sgns::ipfs_pubsub::GossipPubSub>( keyPairResult.value() );
    auto startFuture = pubsub->Start( 0, {}, kListenIp, {} );   // port 0 = ephemeral
    auto startError  = startFuture.get();
    EXPECT_FALSE( startError ) << "Could not start GossipPubSub: " << startError.message();
    if ( startError )
    {
        return nullptr;
    }
    return pubsub;
}
```

**Lifecycle test shape** (lines 200-222 — LifecycleWithInjectedPubSub):
```cpp
TEST_F( GcsCoreSmokeTest, LifecycleWithInjectedPubSub )
{
    auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
    ASSERT_NE( pubsub, nullptr );

    gcs::CoreSession::Config cfg{};
    cfg.m_dbPath = m_tempPath + "/db";
    gcs::CoreSession session( cfg );

    auto res = session.Initialize( pubsub );
    ASSERT_TRUE( res.has_value() );
    EXPECT_TRUE( session.IsRunning() );

    // The db directory may be created lazily — wait via the wait-condition template.
    const auto dbPath = cfg.m_dbPath;
    EXPECT_TRUE( gcs::test::WaitForCondition(
        [&dbPath]() { return std::filesystem::exists( dbPath ); },
        gcs::test::kWaitTimeout ) );

    session.Shutdown();
    EXPECT_FALSE( session.IsRunning() );

    pubsub->Stop();
}
```

**CRDT round-trip test shape** (extends LifecycleWithInjectedPubSub with topic + Put/Get):
```cpp
TEST_F( GcsCoreSmokeTest, CrdtPutGetRoundTripsOnGcsChatTopic )
{
    auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
    ASSERT_NE( pubsub, nullptr );

    gcs::CoreSession::Config cfg{};
    cfg.m_dbPath = m_tempPath + "/db";
    gcs::CoreSession session( cfg );
    ASSERT_TRUE( session.Initialize( pubsub ).has_value() );

    // Topic name follows architecture convention gcs/chat/<roomname> (CONTEXT.md
    // canonical_refs → gcs-chat-architecture.md).
    constexpr const char *kSmokeTopic = "gcs/chat/smoke-test";
    ASSERT_TRUE( session.AddBroadcastTopic( kSmokeTopic ).has_value() );
    ASSERT_TRUE( session.AddListenTopic( kSmokeTopic ).has_value() );

    // Put→Get round-trip via the GcsGlobalDb pass-through accessors added in the
    // submodule task (gcs_global_db.hpp/cpp — see "GNUS-NEO-SWARM submodule" below).
    const std::string key   = "smoke-key";
    const std::string value = "smoke-value";
    ASSERT_TRUE( session.Put( key, value ).has_value() );

    auto getResult = session.Get( key );
    ASSERT_TRUE( getResult.has_value() );
    EXPECT_EQ( getResult.value(), value );

    session.Shutdown();
    pubsub->Stop();
}
```

**Pattern notes:**
- Tests use the injected-pubsub seam — never bring GeniusNode online in CI (RESEARCH § Alternatives Considered).
- The `SetUpTestSuite` soralog block is **mandatory** — omitting it crashes the first GossipPubSub construction (RESEARCH Pitfall 2).
- No `std::this_thread::sleep_for` anywhere — always `WaitForCondition`.

---

### `test/test_gcs_ffi.cpp` (new — FFI init/echo/shutdown)

**Analog:** `GNUS-NEO-SWARM/test/ffi/test_genius_elm_ffi.cpp` (100+ lines)

**Fixture with shutdown-on-teardown** (lines 10-22):
```cpp
/**
 * @file       test_gcs_ffi.cpp
 * @brief      Unit tests for the gcs_ffi C ABI — init, echo, string_free, shutdown.
 * @date       2026-08-15
 */

#include "ffi/gcs_core.h"
#include <gtest/gtest.h>

class GcsFFI : public ::testing::Test
{
protected:
    void TearDown() override
    {
        // Destroy the C++ session before GTest global teardown — the GcsGlobalDb
        // destructor touches asio::io_context internals and the libp2p stack; if left
        // to static destruction after __cxa_finalize, pthread primitives are gone
        // and a "pthread lock: Invalid argument" abort fires at process exit.
        // (Same reasoning as GeniusElmFFI::TearDown in the analog.)
        gcs_shutdown( /* handle from test state */ );
    }
};
```

**Test shapes** (mirroring lines 24-94 of analog):
```cpp
TEST_F( GcsFFI, InitWithNullptrUsesDefaults )
{
    GcsSession* h = gcs_init( nullptr );
    EXPECT_NE( h, nullptr );
    // handle stored in fixture for TearDown
}

TEST_F( GcsFFI, StringFreeNullptrDoesNotCrash )
{
    gcs_string_free( nullptr );
    SUCCEED();
}

TEST_F( GcsFFI, MultipleInitCallsReturnSameHandle )
{
    GcsSession* h1 = gcs_init( nullptr );
    GcsSession* h2 = gcs_init( nullptr );
    GcsSession* h3 = gcs_init( nullptr );
    EXPECT_EQ( h1, h2 );
    EXPECT_EQ( h2, h3 );
}

TEST_F( GcsFFI, ShutdownNullptrIsSafeNoOp )
{
    gcs_shutdown( nullptr );
    SUCCEED();
}
```

**Pattern notes:**
- Shutdown in `TearDown` is mandatory (prevents pthread abort at process exit — analog's `FIX-01` comment).
- FFI tests link `gcs_ffi` only (not `gcs_core` directly); they exercise the C ABI exactly as Dart will.

---

### `.github/workflows/cmake.yml` (new — CI)

**Analog:** `../GeniusSDK/.github/workflows/cmake.yml` (594 lines — copy near-verbatim with documented deltas)

**Trigger + permissions block** (lines 1-26 — copy verbatim):
```yaml
name: Release Build CI

on:
  push:
    branches:
      - develop
      - main
    paths-ignore:
      - ".github/**"
      - "Readme.md"
  pull_request:
    branches:
      - develop
      - main
    paths-ignore:
      - ".github/**"
      - "Readme.md"
  workflow_dispatch:
    inputs:
      tag:
        description: "Release tag"
        required: false
        type: string
permissions:
  contents: write
  packages: read
```

**resolve-runners job** (lines 28-125 — copy verbatim, no changes):
The full `pick_runner()` bash function with `sg-ubuntu-linux` / `sg-arm-linux` / `SG-WIN11` / `gv-OSX-Large` selectors and `ubuntu-latest` / `ubuntu-24.04-arm` / `windows-2022` / `macos-latest` fallbacks. Uses `secrets.GNUS_TOKEN_1`. ~100 lines.

**Matrix block** (lines 138-191 — copy verbatim):
5 targets × Debug/Release with ABI includes for Linux (x86_64, aarch64) and Android (arm64-v8a, armeabi-v7a), excludes for empty-ABI Linux/Android. Container `ghcr.io/geniusventures/debian-bullseye:latest` on Linux/Android.

**Self-hosted cleanup step** (lines 214-267 — copy verbatim, change one path):
```yaml
      - name: Clean workspace (self-hosted runners)
        if: ${{ runner.environment == 'self-hosted' }}
        shell: bash
        run: |
          # ... (git clean / reset / rm -rf) ...
          for dir in thirdparty zkLLVM SuperGenius GeniusSDK; do
            if [ -d "$dir" ]; then
              echo "Removing $dir directory..."
              $SUDO rm -rf "$dir"
            fi
          done
```
Keep the `zkLLVM` cleanup line even though GCS skips the explicit zkLLVM download step — stale dirs from prior runs could exist on the self-hosted runner.

**Checkout step** (lines 269-274 — modify path):
```yaml
      - name: Checkout GCS
        uses: actions/checkout@de0fac2e4500dabe0009e67214ff5f5447ce83dd
        with:
          # No `path:` override — workspace IS the GCS repo root.
          submodules: "recursive"   # GNUS-NEO-SWARM, build (cmaketemplate), gendoc-template, src/app/scaffold
          persist-credentials: true
```

**thirdparty release download** (lines 286-353 — copy verbatim, no changes):
Tag format `<Target>[-<ABI>]-<branch>-<BuildType>`, asset `<Target>[-<ABI>]-<BuildType>.tar.gz`, sets `THIRDPARTY_BUILD_DIR`.

**zkLLVM release download** (lines 355-409 — DELETE entire step per D-09):
Per RESEARCH § CI/CD item 3, cmaketemplate will still auto-download zkLLVM headers at configure time. "Skip zkLLVM" means no explicit step, no assigner binaries. Confirm with user if "zero zkLLVM bytes" was intended.

**SuperGenius release download** (lines 411-450 — copy verbatim, no changes):
Sets `SUPERGENIUS_DIR`.

**NEW: GeniusSDK release download** (insert after SuperGenius step — GCS consumes GeniusSDK prebuilt, unlike GeniusSDK repo which builds itself):
```yaml
      - name: Download GeniusSDK
        shell: bash
        run: |
          if [ "$IS_TAG" == "true" ]; then
            sdk_branch="$CURRENT_BRANCH"
          elif [ "$CURRENT_BRANCH" == "main" ]; then
            sdk_branch="main"
          else
            sdk_branch="develop"
          fi

          mkdir GeniusSDK
          cd GeniusSDK

          if [ "$IS_TAG" == "true" ]; then
            tag_name="${sdk_branch}"
          else
            if [ '${{matrix.abi}}' ]; then
              tag_name="${{matrix.target}}-${{matrix.abi}}-${sdk_branch}-${{matrix.build-type}}"
            else
              tag_name="${{matrix.target}}-${sdk_branch}-${{matrix.build-type}}"
            fi
          fi

          gh release download ${tag_name} --repo GeniusVentures/GeniusSDK -p "${FILE_NAME}"
          tar -xzf "${FILE_NAME}"

          if [ "${{ matrix.target }}" = "Windows" ]; then
            echo "GENIUS_SDK_DIR=$(pwd -W)" >> $GITHUB_ENV
          else
            echo "GENIUS_SDK_DIR=$(pwd)" >> $GITHUB_ENV
          fi
```

**Per-platform host setup steps** (lines 452-502 — copy verbatim):
- Linux: `update-alternatives` for clang + `apt install ccache ninja-build libvulkan-dev libzstd-dev libsecret-1-dev -y`
- Windows: `choco install ccache -A`
- macOS: `brew install ninja bash gnu-tar` + GNU tar PATH fixup
- Android: NDK r27b download + rustup targets (drop the `rustup` lines if GCS has no Rust — **confirm with user**)

**CMake configure steps** (lines 504-527 — copy verbatim, drop `-DZKLLVM_BUILD_DIR`, drop `-DSUPERGENIUS_DIR` since GCS uses sibling auto-detect, **add `-DGENIUSSDK_BUILD_DIR` if needed**; working directory becomes `${{github.workspace}}` since workspace IS GCS root):
```yaml
      - name: Configure CMake for Mac
        if: ${{ matrix.target == 'OSX'}}
        working-directory: ${{github.workspace}}
        run: cmake -S build/${{matrix.target}} -B $BUILD_DIRECTORY -DCMAKE_BUILD_TYPE=${{matrix.build-type}} -DTHIRDPARTY_BUILD_DIR=${{env.THIRDPARTY_BUILD_DIR}}
```
Same shape for Linux/Windows/Android/iOS. The `PROJECT_SUPER_ROOT` auto-detect resolves SuperGenius and GeniusSDK as siblings of the workspace (RESEARCH § CI/CD item 4).

**Build + Install** (lines 529-536 — copy verbatim, working-directory becomes `${{github.workspace}}/${{env.BUILD_DIRECTORY}}`).

**ctest steps** (copy from `../SuperGenius/.github/workflows/cmake.yml:703-740` — NOT in the GeniusSDK workflow):
```yaml
      - name: Run tests (Linux)
        working-directory: ${{ github.workspace }}/${{ env.BUILD_DIRECTORY }}
        if: ${{ matrix.target == 'Linux' }}
        shell: bash
        run: |
          dbus-run-session -- bash -c '\
            mkdir -p ~/.cache ~/.local/share/keyrings && \
            echo -n "login" > ~/.local/share/keyrings/default && \
            echo "[keyring]
            display-name=login
            lock-on-idle=false
            lock-after=false" > ~/.local/share/keyrings/login.keyring && \
            eval $(printf "\n" | gnome-keyring-daemon --unlock --components=secrets) && \
            ctest . -j -C ${{ matrix.build-type }} --output-on-failure'

      - name: Run tests (OSX)
        working-directory: ${{ github.workspace }}/${{ env.BUILD_DIRECTORY }}
        if: ${{ matrix.target == 'OSX' }}
        run: ctest . -j -C ${{ matrix.build-type }} --output-on-failure

      - name: Run tests (Windows)
        working-directory: ${{ github.workspace }}/${{ env.BUILD_DIRECTORY }}
        if: ${{ matrix.build-type == 'Release' && matrix.target == 'Windows' }}
        run: ctest . -j -C ${{ matrix.build-type }} --output-on-failure
```
**No ctest on Android/iOS** (cross-compiles). The gnome-keyring wrapper is harmless for GCS (per RESEARCH § A4) and matches SuperGenius precedent — copy it verbatim.

**Release upload steps** (lines 538-593 — copy verbatim, replace `GeniusSDK` paths with GCS workspace; the `--transform 's|^|${{env.BUILD_DIRECTORY}}/|'` tar argument needs the right top-level dir for GCS — likely `src/` + `GNUS-NEO-SWARM/` or whatever the planner decides to ship. **Planner's discretion**; recommend including this phase to smoke-prove artifact packaging per RESEARCH § CI/CD item 8).

**Pattern notes:**
- Total ~600 lines, 95% copy from GeniusSDK template.
- Deltas are: drop zkLLVM step, add GeniusSDK download step, change `working-directory` from `${{github.workspace}}/GeniusSDK` to `${{github.workspace}}`, add ctest steps from SuperGenius workflow.
- zkLLVM cleanup line stays in self-hosted cleanup step.

---

### `GNUS-NEO-SWARM/src/storage/gcs_global_db.{hpp,cpp}` (submodule — pass-through accessors)

**Analog:** SuperGenius `src/crdt/globaldb/globaldb.hpp:89-225` (the underlying API the accessors wrap)

**Addition shape** (in `GcsGlobalDb` class — placed after `IsRunning()`):
```cpp
        /**
         * @brief Add a broadcast topic to the underlying GlobalDB.
         *
         * Pass-through to sgns::crdt::GlobalDB::AddBroadcastTopic. Required for
         * Phase 1 CORE-05 smoke test (gcs/chat/<roomname> topic wiring).
         *
         * @param[in] topicName Topic name (e.g. "gcs/chat/smoke-test").
         * @return outcome::success or Error::GcsDbError if not running.
         */
        outcome::result<void> AddBroadcastTopic( const std::string &topicName );

        /**
         * @brief Add a listen topic to the underlying GlobalDB.
         */
        outcome::result<void> AddListenTopic( const std::string &topicName );

        /**
         * @brief Put a key/value pair into the CRDT store.
         *
         * @param[in] key    HierarchicalKey path.
         * @param[in] value  Buffer contents.
         */
        outcome::result<void> Put( const std::string &key, const std::string &value );

        /**
         * @brief Get a value from the CRDT store.
         */
        outcome::result<std::string> Get( const std::string &key );
```

**Implementation shape** (thin wrappers over `m_db`, in the .cpp):
```cpp
outcome::result<void> GcsGlobalDb::AddBroadcastTopic( const std::string &topicName )
{
    if ( !m_running.load() )
    {
        return outcome::failure( Error::GcsDbError );
    }
    return m_db->AddBroadcastTopic( topicName );
}
// ... same shape for AddListenTopic / Put / Get
```

**Pattern notes:**
- This is a submodule change — must be committed on a GNUS-NEO-SWARM branch, NOT in the GCS repo directly.
- Per RESEARCH Open Question 2, planner should sequence this as its own task with its own neoswarm-branch commit before the GCS smoke test task.
- Confirm scope with the user before adding — STATE.md flags "Phase 1 depends on GlobalDB CRDT integration from GNUS-NEO-SWARM Phase 3" as a concern.
- Match `GcsGlobalDb`'s existing error mapping (D-14): use `Error::GcsDbError` for all failures (not running + underlying errors).

---

## Shared Patterns

### Hard-Required Dependency Linkage (no stubs)

**Source:** `GNUS-NEO-SWARM/src/storage/CMakeLists.txt:14-18, 30-47`
**Apply to:** `src/CMakeLists.txt` (gcs_core → neoswarm_storage), `src/ffi/CMakeLists.txt` (gcs_ffi → gcs_core)

```cmake
if(TARGET <required>)
    target_link_libraries(<consumer> PUBLIC <required>)
else()
    message(FATAL_ERROR "<required> target not found — <owner> must be configured first")
endif()
```

Project Conditional Compilation rule: missing required libraries fail at CMake configure time, never degrade to stub. No `#ifdef` feature gates in source.

---

### Export Macro Idiom (FFI shared lib)

**Source:** `GNUS-NEO-SWARM/src/genius_elm_chat_completions.h:6-22`
**Apply to:** `src/ffi/gcs_core.h`

```c
#if defined( _WIN32 )
#if defined( GCS_FFI_EXPORTS )
#define GCS_FFI_API __declspec( dllexport )
#else
#define GCS_FFI_API __declspec( dllimport )
#endif
#else
#define GCS_FFI_API
#endif

#if defined( __cplusplus )
#define GCS_FFI_NOEXCEPT noexcept
extern "C"
{
#else
#define GCS_FFI_NOEXCEPT
#endif
```

The `_WIN32` ifdef here is the **only** permitted OS preprocessor guard in the GCS C++ source. It is the established in-repo carve-out for FFI export headers (RESEARCH § Build System Wiring).

---

### Heap-String Allocation + Paired Free

**Source:** `GNUS-NEO-SWARM/src/genius_elm_chat_completions.cpp:28-38, 210-213`
**Apply to:** `src/ffi/gcs_core_ffi.cpp`

```cpp
char* AllocCopy( const std::string_view src )
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
// ...
GCS_FFI_API void gcs_string_free( char* value ) GCS_FFI_NOEXCEPT
{
    std::free( value );
}
```

Prevents Windows cross-allocator heap mismatch (RESEARCH § Don't Hand-Roll).

---

### FFI Thread-Safety + Idempotent Init

**Source:** `GNUS-NEO-SWARM/src/genius_elm_chat_completions.cpp:23-26, 146-152, 220-225`
**Apply to:** `src/ffi/gcs_core_ffi.cpp`

```cpp
namespace
{
    std::mutex g_mutex;
    std::unique_ptr<gcs::CoreSession> g_session;
}

// Every FFI function:
std::lock_guard<std::mutex> lock( g_mutex );
if ( g_session ) { return /* existing */; }   // idempotent
```

Global mutex + `unique_ptr` global. Subsequent init calls are no-ops. All functions `noexcept` (project rule: no exceptions across FFI).

---

### Wait-Condition Test Template (no sleep_for)

**Source:** `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp:72-90`
**Apply to:** All GCS test files (extracted into `test/test_wait_condition.hpp`)

```cpp
bool WaitForCondition( const std::function<bool()> &predicate, std::chrono::milliseconds timeout )
{
    std::mutex              mtx;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock( mtx );
    const auto deadline = std::chrono::steady_clock::now() + timeout;
    while ( std::chrono::steady_clock::now() < deadline )
    {
        if ( predicate() ) { return true; }
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
            deadline - std::chrono::steady_clock::now() );
        const auto slice     = std::min( kPollInterval, remaining );
        cv.wait_for( lock, slice );
    }
    return predicate();
}
```

Project rule: NEVER `std::this_thread::sleep_for` in tests. Always `WaitForCondition`.

---

### Soralog One-Time Setup in Tests

**Source:** `GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp:46-58, 108-117`
**Apply to:** All GCS test fixtures that construct `GossipPubSub` or touch SuperGenius loggers

```cpp
static void SetUpTestSuite()
{
    auto loggerConfigurator = std::make_shared<libp2p::log::Configurator>();
    auto configFromYaml     = std::make_shared<soralog::ConfiguratorFromYAML>(
        loggerConfigurator, std::string{ kLoggingYaml } );
    auto loggingSystem      = std::make_shared<soralog::LoggingSystem>( configFromYaml );
    ASSERT_FALSE( loggingSystem->configure().has_error );
    libp2p::log::setLoggingSystem( loggingSystem );
}
```

Omitting this crashes `GossipPubSub` construction with "Logging system is not ready" (RESEARCH Pitfall 2).

---

### Injected-Pubsub Test Seam

**Source:** `GNUS-NEO-SWARM/src/storage/gcs_global_db.hpp:113-123` + `test_gcs_global_db.cpp:140-158`
**Apply to:** `test/test_gcs_core_smoke.cpp`, `src/lib/gcs_core.cpp` (CoreSession must expose the same overload)

```cpp
// Production path:
outcome::result<void> Initialize();   // uses GeniusSDKGetNode()

// Test seam (Tier 2 fixture pattern):
outcome::result<void> Initialize( std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> pubsub );
```

Tests stand up `GossipPubSub` on port 0 (ephemeral) — never bring GeniusNode online in CI (RESEARCH § Alternatives Considered).

---

### Outcome-Based Error Handling

**Source:** `GNUS-NEO-SWARM/src/storage/gcs_global_db.hpp:110, 123` + `gcs_global_db.cpp`
**Apply to:** `src/lib/gcs_core.cpp`, `src/ffi/gcs_core_ffi.cpp` (translate `outcome::result` → C return codes at FFI boundary)

Every fallible C++ function returns `outcome::result<T>`. Error codes from `GNUS-NEO-SWARM/src/common/error.hpp` (`Error::SdkNotInitialized`, `Error::GcsDbError`, etc.). At the FFI boundary, translate `outcome::failure(...)` → `int` status code or `nullptr` handle; never let exceptions escape.

---

### C++ Style (SuperGenius Naming)

**Source:** `GNUS-NEO-SWARM/CLAUDE.md` (overrides general code style)
**Apply to:** All new GCS C++ files

| Element | Convention | Example |
|---------|-----------|---------|
| Member variables | `m_` prefix + camelCase | `m_dbPath`, `m_logger`, `m_ioThread` |
| Function arguments | camelCase | `topicName`, `dbPath` |
| File names | `snake_case` | `gcs_core.cpp`, `gcs_core_ffi.cpp` |
| Constants (compile-time) | `k` prefix + PascalCase | `kWaitTimeout`, `kListenIp`, `kDefaultDbPath` |
| Accessors | `Get`/`Set`/`Is` prefix | `IsRunning()`, `GetSession()` |
| Braces | Allman (own line) | see all analog excerpts above |
| Spacing | `if ( condition )` (space inside parens) | see all analog excerpts above |
| Line length | 120 chars max | — |
| Doxygen | `\brief` / `\param[in]` / `\return` on every public API | see `genius_elm_chat_completions.h` |

C++17 ceiling — no coroutines, no `std::format`, no concepts.

---

## No Analog Found

Files with no close match in the codebase (planner should use RESEARCH.md patterns instead):

| File | Role | Data Flow | Reason |
|------|------|-----------|--------|
| `src/ffi/dart_native_api.h` + `dart_api_dl.c` (vendored Dart SDK headers — only if the spike confirms this include strategy) | FFI C header | event-driven (native→Dart callback) | No `Dart_PostCObject`/`Dart_Port`/`NativePort`/`ReceivePort` usage anywhere in GNUS-NEO-SWARM, SuperGenius, or GeniusSDK (RESEARCH § FFI Callback Mechanism — repo-wide grep returned zero hits). The standard Dart API_DL pattern is documented in RESEARCH § Pattern 1 with example code; planner should budget a small spike task to validate the include/link strategy. |

This is the only genuine "no in-repo precedent" gap in the phase. RESEARCH § FFI Callback Mechanism documents the standard pattern with example C and Dart code; CONTEXT D-05 locks the ReceivePort/NativePort approach.

---

## Metadata

**Analog search scope:**
- `GNUS-NEO-SWARM/src/storage/` (GcsGlobalDb component + CMakeLists)
- `GNUS-NEO-SWARM/src/genius_elm_chat_completions.{h,cpp}` (FFI export idiom)
- `GNUS-NEO-SWARM/neoswarm_ffi/` (ffigen plugin structure)
- `GNUS-NEO-SWARM/test/{CMakeLists.txt,storage/,ffi/}` (test patterns)
- `../GeniusSDK/.github/workflows/cmake.yml` (consumer-repo CI template)
- `../SuperGenius/.github/workflows/cmake.yml` (ctest steps reference)
- `src/{CMakeLists.txt,lib/gcs_core.cpp}` (current GCS state)

**Files scanned:** 12 (read in full or targeted sections)
**Pattern extraction date:** 2026-08-15
**Cross-references:** RESEARCH.md (all cited line numbers verified), CONTEXT.md (D-01 through D-09 traced to patterns)
