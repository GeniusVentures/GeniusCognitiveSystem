# macOS Dart ctest legs. Included from src/app/CMakeLists.txt's scope (see
# OSX/appBuild.cmake): FLUTTER_EXECUTABLE is set by the includer and
# CMAKE_CURRENT_SOURCE_DIR is src/app (include() does not change it).
#
# Dart-side FFI smoke test (01-05 Task 4) — registered in the SAME ctest
# harness as the gtests, so `ninja && ctest` is the single entry point.
# CMake owns all path/suffix knowledge: $<TARGET_FILE:gcs_ffi> expands to
# libgcs_ffi.dylib/.so/gcs_ffi.dll per platform and config, injected via
# `cmake -E env` (COMMAND args support generator expressions on every CMake
# version we target; the test ENVIRONMENT property does not).
# FRONTEND_TESTS_ENABLED (default ON on desktop) only removes this ctest leg
# — the app itself always builds. OFF is for focused C++ test runs
# (-DFRONTEND_TESTS_ENABLED=OFF).
option(FRONTEND_TESTS_ENABLED "Register Dart/flutter tests in ctest" ON)
if(BUILD_TESTS AND FRONTEND_TESTS_ENABLED AND TARGET gcs_ffi)
    add_test(NAME test_gcs_ffi_dart
        COMMAND ${CMAKE_COMMAND} -E env
            "GCS_FFI_LIBRARY=$<TARGET_FILE:gcs_ffi>"
            ${FLUTTER_EXECUTABLE} test test/gcs_native_port_smoke_test.dart
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
endif()
