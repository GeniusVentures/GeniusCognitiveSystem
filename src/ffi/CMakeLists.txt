# gcs_ffi — SHARED library exposing the minimal topic pub/sub C ABI to Flutter (D-01, D-02, D-27, D-29).
add_library(gcs_ffi SHARED
    gcs_core_ffi.cpp
    dart_api_dl.c
)
set_target_properties(gcs_ffi PROPERTIES
    PUBLIC_HEADER gcs_core.h
    OUTPUT_NAME "gcs_ffi"
)
# Expose src/ so consumers (test/test_gcs_ffi.cpp) can #include "ffi/gcs_core.h".
target_include_directories(gcs_ffi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
# Vendored Dart API_DL set (dart_native_api.h / dart_api_dl.h / internal/) —
# resolves the Dart_PostCObject DL indirection includes from src/ffi itself.
target_include_directories(gcs_ffi PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(gcs_ffi PRIVATE gcs_core gcs_proto)
# GCS_FFI_EXPORTS drives the gcs_* dllexport macro (gcs_core.h).
# DART_SHARED_LIB makes DART_EXPORT resolve to __declspec(dllexport) so
# Dart_InitializeApiDL (declared DART_EXPORT in dart_api_dl.h, defined in
# dart_api_dl.c) lands in gcs_ffi.dll's export table — without it the
# Dart-side DynamicLibrary.lookup('Dart_InitializeApiDL') fails on Windows.
target_compile_definitions(gcs_ffi PRIVATE GCS_FFI_EXPORTS DART_SHARED_LIB)
if(ANDROID)
    target_link_options(gcs_ffi PRIVATE "-Wl,-z,max-page-size=16384")
endif()
# Install alongside the rest of the tree so consumers (Dart test harness,
# plugin staging, packaging) take the artifact from one stable per-platform
# location — never from the CMake build dir (LIBRARY: .so/.dylib, RUNTIME:
# .dll, ARCHIVE: Windows import lib).
install(TARGETS gcs_ffi
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
