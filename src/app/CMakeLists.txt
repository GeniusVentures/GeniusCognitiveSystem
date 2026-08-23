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
