# GCS-level source tree: storage, api server, FFI.
# Built as part of the parent GeniusCogntiveSystem build (root CMakeLists add_subdirectory(src)).

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
