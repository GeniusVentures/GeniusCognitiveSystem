# gcs_ffi — SHARED library exposing the minimal topic pub/sub C ABI to Flutter (D-01, D-02, D-27, D-29).
add_library(gcs_ffi SHARED
    gcs_core_ffi.cpp
)
set_target_properties(gcs_ffi PROPERTIES
    PUBLIC_HEADER gcs_core.h
    OUTPUT_NAME "gcs_ffi"
)
# Expose src/ so consumers (test/test_gcs_ffi.cpp) can #include "ffi/gcs_core.h".
target_include_directories(gcs_ffi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
target_link_libraries(gcs_ffi PRIVATE gcs_core gcs_proto)
target_compile_definitions(gcs_ffi PRIVATE GCS_FFI_EXPORTS)   # dllexport macro
if(ANDROID)
    target_link_options(gcs_ffi PRIVATE "-Wl,-z,max-page-size=16384")
endif()
