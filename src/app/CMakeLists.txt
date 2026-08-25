# GCS Flutter app — gated behind dart/flutter detection (D-16, D-08).
# Included from src/CMakeLists.txt (owned by plan 01-03) via add_subdirectory(app).
# Default OFF so the C++-only CI path configures cleanly with no dart/flutter.
option(FRONTEND_BUILD_ENABLED "Enable Flutter/Dart app targets" OFF)
if(NOT FRONTEND_BUILD_ENABLED)
    return()
endif()

find_program(DART_EXECUTABLE dart)
find_program(FLUTTER_EXECUTABLE flutter)
find_program(PYTHON3_EXECUTABLE python3)
if(NOT DART_EXECUTABLE OR NOT FLUTTER_EXECUTABLE)
    message(WARNING "FRONTEND_BUILD_ENABLED=ON but dart/flutter not found — skipping app targets.")
    return()
endif()

# Cache vars MUST be set BEFORE add_subdirectory(scaffold) (Pitfall U1:
# set after, they silently don't apply — scaffold has already used its defaults).
set(TEMPLATES_DIR "${CMAKE_CURRENT_SOURCE_DIR}/templates" CACHE STRING "GCS chat composite templates")
set(GENERATED_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/generated" CACHE STRING "GCS generated composite output (committed)")
set(FRONTEND_TARGET "flutter" CACHE STRING "Scaffold codegen target")

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/scaffold/CMakeLists.txt")
    add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/scaffold scaffold)
endif()

# Stage the FFI dylib into the neoswarm_ffi plugin package (macos/lib/) so the
# podspec can reference a stable relative path instead of hardcoding the CMake
# build directory/config. Runs on every build of the FFI target — whichever
# config was built last is what gets staged and embedded.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin" AND TARGET Genius-MOS-ELM-FFI)
    set(GCS_FFI_PLUGIN_LIB_DIR
        "${CMAKE_CURRENT_SOURCE_DIR}/../../GNUS-NEO-SWARM/neoswarm_ffi/macos/lib")
    add_custom_target(app_stage_ffi_dylib
        COMMAND ${CMAKE_COMMAND} -E make_directory "${GCS_FFI_PLUGIN_LIB_DIR}"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:Genius-MOS-ELM-FFI>" "${GCS_FFI_PLUGIN_LIB_DIR}"
        DEPENDS Genius-MOS-ELM-FFI
        COMMENT "Staging Genius-MOS-ELM-FFI dylib into neoswarm_ffi plugin"
    )
endif()

# App-level dev-loop targets. app_analyze/app_test DEPEND only on app_pub_get
# for now; the DEPENDS app_generate_chat_components edge is added in 01-08 when
# the composite codegen loop lands (avoids a dangling target reference).
# Never depend on the scaffold's OpenAPI aggregate/api targets — GCS has no
# api-specs (Pitfall U4).
add_custom_target(app_pub_get
    COMMAND ${DART_EXECUTABLE} pub get
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Resolving app Dart dependencies"
)
add_custom_target(app_analyze
    COMMAND ${DART_EXECUTABLE} analyze --fatal-infos
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS app_pub_get
)
add_custom_target(app_test
    COMMAND ${FLUTTER_EXECUTABLE} test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS app_pub_get
)

# Bare-app desktop build. Host-desktop only (macOS here) — the equivalent
# windows/linux targets can be added the same way when those hosts are needed.
# Same Flutter invocation `flutter run -d macos` uses under the hood.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(_app_extra_deps app_pub_get)
    if(TARGET app_stage_ffi_dylib)
        list(APPEND _app_extra_deps app_stage_ffi_dylib)
    endif()
    add_custom_target(app_build_macos
        COMMAND ${FLUTTER_EXECUTABLE} build macos --debug
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_app_extra_deps}
        COMMENT "Building bare Flutter app for macOS desktop"
    )
endif()
