cmake_minimum_required(VERSION 3.16)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

set(
        CMAKE_TOOLCHAIN_FILE
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain/cxx17.cmake"
        CACHE
        FILEPATH
        "Default toolchain"
)
add_definitions(-D_WIN32_WINNT=0x0601)
add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS)

project(GeniusCogntiveSystem VERSION 0.1.0
        DESCRIPTION "GCS cognitive system — shared GlobalDB storage, API server, and FFI over the NEO-SWARM inference engine"
        LANGUAGES CXX)

include(GNUInstallDirs)

if(BUILD_TESTING)
    enable_testing()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})
include_directories(${GSL_INCLUDE_DIR})
find_package(ZLIB CONFIG REQUIRED)
find_package(libsecp256k1 CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)
find_package(Boost REQUIRED COMPONENTS date_time filesystem random regex system thread log log_setup program_options json)
find_package(nlohmann_json CONFIG REQUIRED)
include_directories(${Boost_INCLUDE_DIRS})

if(BUILD_TESTING)
    find_package(GTest CONFIG REQUIRED)
endif()

set(CMAKE_INSTALL_INCLUDEDIR "${CMAKE_INSTALL_INCLUDEDIR}/GeniusCogntiveSystem")

# NEO-SWARM inference engine submodule (neoswarm_* targets, neo-swarm exe, FFI lib)
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/GNUS-NEO-SWARM GNUS-NEO-SWARM)

# GCS-level source tree (storage, api, FFI) — parent repo
add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/src src)
