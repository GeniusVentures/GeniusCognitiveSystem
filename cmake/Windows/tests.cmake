# Windows Dart ctest legs. Included from src/app/CMakeLists.txt's scope (see
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

    # The Dart leg loads gcs_ffi.dll from its build-output dir
    # (gcs_src/ffi/<Config>), which has no vulkan-1.dll — the vendored
    # Vulkan loader the DLL links. Without it the load fails with an
    # opaque "unknown error" before any test runs (CI run 36904228994,
    # Windows legs). Same rationale as the per-test POST_BUILD in
    # test/CMakeLists.txt; VULKAN_RUNTIME_DLL is resolved in
    # cmake/CommonBuildParameters.cmake and stays empty on system-SDK
    # hosts (which have the runtime on PATH).
    # ALL + DEPENDS (not POST_BUILD): add_custom_command(TARGET) requires
    # the target to be created in the SAME directory — gcs_ffi is created
    # in src/ffi, so a POST_BUILD here is a configure error (CI run
    # 36905881581, "TARGET 'gcs_ffi' was not created in this directory").
    # The ALL target runs during every cmake --build, after gcs_ffi links;
    # copy_if_different makes repeat builds no-ops.
    if(VULKAN_RUNTIME_DLL)
        add_custom_target(gcs_ffi_windows_runtime ALL
            COMMAND "${CMAKE_COMMAND}" -E copy_if_different
                "${VULKAN_RUNTIME_DLL}" "$<TARGET_FILE_DIR:gcs_ffi>/vulkan-1.dll"
            DEPENDS gcs_ffi
            COMMENT "Deploying vulkan-1.dll beside gcs_ffi.dll")
    endif()
endif()
