# GCS per-platform CMake fragment loader. Resolves BUILD_TARGET_OS (the
# predefined platform set: OSX/Windows/Linux/iOS/Android) and includes every
# CMake file under cmake/${BUILD_TARGET_OS}/ — platform-specific app build
# targets, install rules, and Dart ctest legs live there
# (OSX/appBuild.cmake, Windows/appBuild.cmake, */tests.cmake), never behind
# scattered CMAKE_SYSTEM_NAME gates. Fragments are optional per platform: a
# platform with no directory (or an empty one) simply has no platform-
# specific wiring. Adding a platform is a new directory + fragment; this
# file never changes.
#
# BUILD_TARGET_OS is resolved here via the build submodule's
# define_build_target_os() (functions.cmake) — the wrapper chain does NOT
# invoke it, so this include+call is the single resolution point for every
# configure, wrapper-driven or standalone; the Darwin→OSX mapping has
# exactly one definition.

if(NOT DEFINED BUILD_TARGET_OS)
    include("${CMAKE_CURRENT_LIST_DIR}/../build/cmake/functions.cmake")
    define_build_target_os()
endif()

file(GLOB _gcs_platform_fragments
    "${CMAKE_CURRENT_LIST_DIR}/${BUILD_TARGET_OS}/*.cmake")

foreach(_gcs_platform_fragment IN LISTS _gcs_platform_fragments)
    message(STATUS "Including platform fragment: ${_gcs_platform_fragment}")
    include("${_gcs_platform_fragment}")
endforeach()
