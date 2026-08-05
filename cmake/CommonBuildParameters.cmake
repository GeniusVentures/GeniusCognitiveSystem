# CommonBuildParameters.cmake — GeniusCognitiveSystem
# Called from build/<Platform>/CMakeLists.txt via build/CommonBuildParameters.cmake.
# Sets up all thirdparty dependencies and adds the project source/test trees.
#
# At this point, CommonCompilerOptions.cmake has already set:
#   - PROJECT_ROOT (via get_default_root)
#   - _THIRDPARTY_BUILD_DIR
#   - C++17, GNUInstallDirs, CompilationFlags, etc.

# ---------------------------------------------------------------------------
# Convenience alias
# ---------------------------------------------------------------------------
set(THIRDPARTY_BUILD_DIR "${_THIRDPARTY_BUILD_DIR}" CACHE PATH "" FORCE)

# BOOST VERSION TO USE
set(BOOST_MAJOR_VERSION "1" CACHE STRING "Boost Major Version")
set(BOOST_MINOR_VERSION "85" CACHE STRING "Boost Minor Version")
set(BOOST_PATCH_VERSION "0" CACHE STRING "Boost Patch Version")

# convenience settings
set(BOOST_VERSION "${BOOST_MAJOR_VERSION}.${BOOST_MINOR_VERSION}.${BOOST_PATCH_VERSION}")
set(BOOST_VERSION_2U "${BOOST_MAJOR_VERSION}_${BOOST_MINOR_VERSION}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --------------------------------------------------------
# Set config of GTest
set(GTest_DIR "${THIRDPARTY_BUILD_DIR}/GTest/lib/cmake/GTest")
set(GTest_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/GTest/include")
find_package(GTest CONFIG REQUIRED)
include_directories(${GTest_INCLUDE_DIR})

# --------------------------------------------------------
# protobuf (+ absl / utf8_range) — needed by NEO-SWARM src/proto add_proto_library
if(NOT DEFINED absl_DIR)
    set(absl_DIR "${THIRDPARTY_BUILD_DIR}/protobuf/lib/cmake/absl")
endif()
if(NOT DEFINED utf8_range_DIR)
    set(utf8_range_DIR "${THIRDPARTY_BUILD_DIR}/protobuf/lib/cmake/utf8_range")
endif()
if(NOT DEFINED Protobuf_DIR)
    set(Protobuf_DIR "${THIRDPARTY_BUILD_DIR}/protobuf/lib/cmake/protobuf")
endif()
if(NOT DEFINED grpc_INCLUDE_DIR)
    set(grpc_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/grpc/include")
endif()
if(NOT DEFINED Protobuf_INCLUDE_DIR)
    set(Protobuf_INCLUDE_DIR "${grpc_INCLUDE_DIR}/google/protobuf")
endif()
find_package(Protobuf CONFIG REQUIRED)

if(NOT DEFINED PROTOC_EXECUTABLE)
    set(PROTOC_EXECUTABLE "${THIRDPARTY_BUILD_DIR}/protobuf/bin/protoc${CMAKE_EXECUTABLE_SUFFIX}")
endif()
set(Protobuf_PROTOC_EXECUTABLE ${PROTOC_EXECUTABLE} CACHE PATH "Initial cache" FORCE)
if(NOT TARGET protobuf::protoc)
    add_executable(protobuf::protoc IMPORTED)
endif()
if(EXISTS "${Protobuf_PROTOC_EXECUTABLE}")
    set_target_properties(protobuf::protoc PROPERTIES
        IMPORTED_LOCATION ${Protobuf_PROTOC_EXECUTABLE})
endif()

# --------------------------------------------------------
# Set config of OpenSSL
set(OPENSSL_DIR "${THIRDPARTY_BUILD_DIR}/openssl/build" CACHE PATH "Path to OpenSSL install folder")
set(OPENSSL_USE_STATIC_LIBS ON CACHE BOOL "OpenSSL use static libs")
set(OPENSSL_MSVC_STATIC_RT ON CACHE BOOL "OpenSSL use static RT")
set(OPENSSL_ROOT_DIR "${OPENSSL_DIR}" CACHE PATH "Path to OpenSSL install root folder")
set(OPENSSL_INCLUDE_DIR "${OPENSSL_DIR}/include" CACHE PATH "Path to OpenSSL include folder")
find_package(OpenSSL REQUIRED)

# --------------------------------------------------------
# Set config of Microsoft GSL (header-only library)
set(GSL_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/Microsoft.GSL/include")
include_directories(${GSL_INCLUDE_DIR})
add_library(Microsoft.GSL INTERFACE IMPORTED)
set_target_properties(Microsoft.GSL PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GSL_INCLUDE_DIR}"
)
add_library(Microsoft.GSL::GSL ALIAS Microsoft.GSL)

# --------------------------------------------------------
# Set config of Boost project
set(_BOOST_ROOT "${THIRDPARTY_BUILD_DIR}/boost/build")
set(Boost_LIB_DIR "${_BOOST_ROOT}/lib")
set(Boost_INCLUDE_DIR "${_BOOST_ROOT}/include/boost-${BOOST_VERSION_2U}")
set(Boost_DIR "${Boost_LIB_DIR}/cmake/Boost-${BOOST_VERSION}")
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_NO_SYSTEM_PATHS ON)
option(Boost_USE_STATIC_RUNTIME "Use static runtimes" ON)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

find_package(Boost REQUIRED COMPONENTS date_time filesystem random regex system thread log log_setup program_options json)
include_directories(${Boost_INCLUDE_DIRS})

# zlib
set(ZLIB_ROOT "${THIRDPARTY_BUILD_DIR}/zlib")
set(ZLIB_DIR "${THIRDPARTY_BUILD_DIR}/zlib/lib/cmake/zlib")
find_package(ZLIB CONFIG REQUIRED)

# fmt
set(fmt_DIR "${THIRDPARTY_BUILD_DIR}/fmt/lib/cmake/fmt")
find_package(fmt CONFIG REQUIRED)

# spdlog
set(spdlog_DIR "${THIRDPARTY_BUILD_DIR}/spdlog/lib/cmake/spdlog")
find_package(spdlog CONFIG REQUIRED)
add_compile_definitions("SPDLOG_FMT_EXTERNAL")

# libsecp256k1
set(libsecp256k1_DIR "${THIRDPARTY_BUILD_DIR}/libsecp256k1/lib/cmake/libsecp256k1")
find_package(libsecp256k1 CONFIG REQUIRED)

# nlohmann/json
set(nlohmann_json_DIR "${THIRDPARTY_BUILD_DIR}/json/share/cmake/nlohmann_json")
find_package(nlohmann_json CONFIG REQUIRED)

# --------------------------------------------------------
# Project options
option(BUILD_TESTS "Build tests" FALSE)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(BUILD_EXAMPLES "Enable demonstration targets." FALSE)

# --------------------------------------------------------
# Project include root
include_directories(${PROJECT_ROOT}/src)

# --------------------------------------------------------
# Submodule + source-tree wiring (this file is the entry point the
# build/<Platform>/CMakeLists.txt chain runs — add_subdirectory lives here).
# --------------------------------------------------------

# GNUS-NEO-SWARM: C++ inference engine library (neoswarm_* targets)
add_subdirectory(${PROJECT_ROOT}/GNUS-NEO-SWARM ${CMAKE_BINARY_DIR}/GNUS-NEO-SWARM)

# GCS-level source tree (storage, api, FFI)
# Binary dir is gcs_src (not "src") — GNUS-NEO-SWARM's own CommonBuildParameters
# already claims ${CMAKE_BINARY_DIR}/src for its own source tree.
add_subdirectory(${PROJECT_ROOT}/src ${CMAKE_BINARY_DIR}/gcs_src)

if(BUILD_TESTS)
    enable_testing()
    if(IS_DIRECTORY "${PROJECT_ROOT}/test")
        add_subdirectory(${PROJECT_ROOT}/test ${CMAKE_BINARY_DIR}/test)
    endif()
endif()

