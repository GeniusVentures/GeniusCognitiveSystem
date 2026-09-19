# GCS-level source tree: storage, api server, FFI.
# Built as part of the parent GeniusCogntiveSystem build (root CMakeLists add_subdirectory(src)).

add_library(gcs_core
        lib/gcs_core.cpp
        lib/gcs_entity_store.cpp
)

target_include_directories(gcs_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(gcs_core PUBLIC
    spdlog::spdlog
    fmt::fmt
)

# gcs_proto — protobuf wire contract (D-26/D-29); leaf target, must precede ffi.
add_subdirectory(proto)

add_subdirectory(lib/gcs_storage)

# gcs_storage — hard required (project Conditional Compilation rule: missing required
# libraries fail at configure time, never degrade to a stub).
if(TARGET gcs_storage)
    target_link_libraries(gcs_core PUBLIC gcs_storage)
else()
    message(FATAL_ERROR "gcs_storage target not found — src/lib/gcs_storage must be configured before gcs_core")
endif()

# gcs_proto — hard required (02-01): gcs_entity_store's record types come from
# the generated gcs_chat.pb.h; PUBLIC because the entity-store headers expose
# SpaceRecord/RoomRecord in public signatures (test consumers link gcs_core only).
if(TARGET gcs_proto)
    target_link_libraries(gcs_core PUBLIC gcs_proto)
else()
    message(FATAL_ERROR "gcs_proto target not found — src/proto must be configured before gcs_core links entity records")
endif()

# gcs_ffi — SHARED library exposing the topic pub/sub C ABI (D-02); after proto
# (gcs_ffi links gcs_proto) and gcs_core.
add_subdirectory(ffi)

# Flutter/Dart app — gated behind FRONTEND_BUILD_ENABLED (default OFF) so
# non-frontend CI paths and platforms without dart/flutter configure cleanly.
add_subdirectory(app)
