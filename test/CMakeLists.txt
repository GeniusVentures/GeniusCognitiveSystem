# GCS-root test tree (01-04): GTest discovery + gcs_test macro + smoke-test
# registration. Mirrors the GNUS-NEO-SWARM test CMakeLists (GTest discovery
# block copied verbatim; the helper macro carries the gcs_ name here; no
# external test-data-dir define is needed for these smoke tests).
#
# Wired by cmake/CommonBuildParameters.cmake's add_subdirectory(${PROJECT_ROOT}/test
# ...) hook, gated on BUILD_TESTS (binary dir gcs_test — NEO-SWARM's own
# CommonBuildParameters already claims ${CMAKE_BINARY_DIR}/test).

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
    # GTest::Main must precede ${libs}: SuperGenius static libs PUBLIC-export
    # Boost::unit_test_framework, whose archive defines main(). If it precedes
    # gtest_main, ld pulls unit_test_main.o and init_unit_test_suite goes undefined.
    target_link_libraries(${name} PRIVATE
        GTest::GTest
        GTest::Main
        ${libs}
    )
    add_test(NAME ${name} COMMAND ${name})
endmacro()

# Moved GlobalDB lifecycle test (01-01) — exercises the GcsGlobalDb component
# directly; links the root-repo gcs_storage target (D-25 moved the component
# out of the submodule). Target name is test_gcs_storage (not test_gcs_global_db):
# the submodule's own test tree still registers a test_gcs_global_db executable,
# and duplicate target names are a CMake configure error (CMP0002) whenever both
# test trees configure in one build. Same source file, same gcs_storage linkage.
gcs_test(test_gcs_storage    test_gcs_global_db.cpp  "gcs_storage;neoswarm_common;sgns::crdt_globaldb")
# SDK-wiring regression (2026-08-27 handler-clobber bug): separate binary —
# GeniusSDKInit/Shutdown toggle a process-global node, which must not leak into
# the option-C tests above. Links gcs_storage (PUBLIC: GeniusSDK_shared +
# SuperGenius/GeniusSDK include dirs); skips when the node cannot boot.
gcs_test(test_gcs_global_db_sdk test_gcs_global_db_sdk.cpp "gcs_storage;neoswarm_common;sgns::crdt_globaldb")
# CORE-05 substrate smoke (01-04 Task 2): CoreSession lifecycle + topic + Put/Get
# round-trip over an injected port-0 GossipPubSub.
gcs_test(test_gcs_core_smoke test_gcs_core_smoke.cpp "gcs_core;gcs_storage;neoswarm_common")
# Phase 2 entity catalog (02-01 Task 3): EntityStore create/update/derived-join/
# tombstone/restart over the same injected-pubsub seam (same libs as the smoke
# test — EntityStore lives in gcs_core).
gcs_test(test_gcs_entities test_gcs_entities.cpp "gcs_core;gcs_storage;neoswarm_common")
# Phase 3 crypto adapter (03-04 Task 1/2): pure unit tests over the stateless
# OpenSSL adapter — no pubsub/DB fixture, links gcs_core (which carries the
# OpenSSL::Crypto link line after Task 2).
gcs_test(test_gcs_crypto test_gcs_crypto.cpp "gcs_core;gcs_storage;neoswarm_common")
# Phase 3 Messaging component (03-04 Task 1/3): injected-pubsub send/receive/
# history/dedupe + encrypted/plaintext both-path coverage over gcs::Messaging.
gcs_test(test_gcs_messaging test_gcs_messaging.cpp "gcs_core;gcs_storage;neoswarm_common")
# FFI ABI smoke (01-04 Task 3): config-bytes gcs_init failure classes, arg
# validation, shutdown-null. Also links gcs_proto so the test can serialize a
# gcs::chat::GcsConfig via gcs_chat.pb.h (D-29 codec-tagged init bytes).
gcs_test(test_gcs_ffi        test_gcs_ffi.cpp        "gcs_ffi;gcs_proto")
# CR-01 regression (01 code review): two init/send/shutdown cycles against the
# SAME db_path — session A's persisted records must survive session B. Own
# binary for the same reason as test_gcs_global_db_sdk (GeniusSDKInit toggles
# a process-global node). gcs_ffi is listed FIRST: the node this test boots via
# the dylib's exported GeniusSDKInit must be the node gcs_init (inside the
# dylib) sees — the dylib must win symbol resolution over the SDK archives
# dragged in transitively by gcs_storage (used only for the verification-side
# injected-pubsub store). src/ffi is added for the vendored Dart API_DL
# headers the fake post hook installs through the exported Dart_InitializeApiDL.
gcs_test(test_gcs_ffi_sdk    test_gcs_ffi_sdk.cpp    "gcs_ffi;gcs_storage;neoswarm_common;gcs_proto")
target_include_directories(test_gcs_ffi_sdk PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/ffi"
)
# Live-app regression (2026-09-19 UAT bug): a lone Flutter app reaches gcs_init
# with NO GeniusSDK node in the process — gcs_init must boot the embedded node
# itself (empty-mnemonic child-wallet path). This binary never boots the SDK
# directly; the cold-boot path IS the test. Same gcs_ffi-first linkage and
# process-global-node isolation as test_gcs_ffi_sdk; skips only when a direct
# SDK probe proves the environment cannot boot a node at all.
gcs_test(test_gcs_ffi_coldboot test_gcs_ffi_coldboot.cpp "gcs_ffi;gcs_storage;neoswarm_common;gcs_proto")
target_include_directories(test_gcs_ffi_coldboot PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/../src/ffi"
)
