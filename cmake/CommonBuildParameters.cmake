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
# No gRPC: protobuf headers resolve from protobuf's own include tree.
if(NOT DEFINED Protobuf_INCLUDE_DIR)
    set(Protobuf_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/protobuf/include")
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
# Per-component DIR hints (needed by GeniusSDK's own component list below —
# ported from GeniusNetwork/GeniusSDK/cmake/CommonBuildParameters.cmake).
set(boost_atomic_DIR "${Boost_LIB_DIR}/cmake/boost_atomic-${BOOST_VERSION}")
set(boost_chrono_DIR "${Boost_LIB_DIR}/cmake/boost_chrono-${BOOST_VERSION}")
set(boost_container_DIR "${Boost_LIB_DIR}/cmake/boost_container-${BOOST_VERSION}")
set(boost_context_DIR "${Boost_LIB_DIR}/cmake/boost_context-${BOOST_VERSION}")
set(boost_date_time_DIR "${Boost_LIB_DIR}/cmake/boost_date_time-${BOOST_VERSION}")
set(boost_filesystem_DIR "${Boost_LIB_DIR}/cmake/boost_filesystem-${BOOST_VERSION}")
set(boost_headers_DIR "${Boost_LIB_DIR}/cmake/boost_headers-${BOOST_VERSION}")
set(boost_json_DIR "${Boost_LIB_DIR}/cmake/boost_json-${BOOST_VERSION}")
set(boost_log_DIR "${Boost_LIB_DIR}/cmake/boost_log-${BOOST_VERSION}")
set(boost_log_setup_DIR "${Boost_LIB_DIR}/cmake/boost_log_setup-${BOOST_VERSION}")
set(boost_program_options_DIR "${Boost_LIB_DIR}/cmake/boost_program_options-${BOOST_VERSION}")
set(boost_random_DIR "${Boost_LIB_DIR}/cmake/boost_random-${BOOST_VERSION}")
set(boost_regex_DIR "${Boost_LIB_DIR}/cmake/boost_regex-${BOOST_VERSION}")
set(boost_system_DIR "${Boost_LIB_DIR}/cmake/boost_system-${BOOST_VERSION}")
set(boost_thread_DIR "${Boost_LIB_DIR}/cmake/boost_thread-${BOOST_VERSION}")
set(boost_coroutine_DIR "${Boost_LIB_DIR}/cmake/boost_coroutine-${BOOST_VERSION}")
set(boost_unit_test_framework_DIR "${Boost_LIB_DIR}/cmake/boost_unit_test_framework-${BOOST_VERSION}")
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_NO_SYSTEM_PATHS ON)
option(Boost_USE_STATIC_RUNTIME "Use static runtimes" ON)

if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD)
endif()

option(SGNS_STACKTRACE_BACKTRACE "Use BOOST_STACKTRACE_USE_BACKTRACE in stacktraces, for POSIX" OFF)
if(SGNS_STACKTRACE_BACKTRACE)
    add_definitions(-DSGNS_STACKTRACE_BACKTRACE=1)
    if(BACKTRACE_INCLUDE)
        add_definitions(-DBOOST_STACKTRACE_BACKTRACE_INCLUDE_FILE=${BACKTRACE_INCLUDE})
    endif()
endif()

# Component list is the union of GCS's own needs and GeniusSDK's genius_node
# needs (container, unit_test_framework, coroutine added per GeniusSDK's file).
find_package(Boost REQUIRED COMPONENTS container date_time filesystem random regex system thread log log_setup program_options json unit_test_framework coroutine)
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
# Remaining GeniusSDK transitive dependencies. GeniusCognitiveSystem now
# links GeniusSDK directly (sgns::GeniusSDK -> sgns::genius_node), and
# genius_node is GeniusSDK's own internal aggregate library — its exported
# config does not re-chain further find_package() calls for its own PUBLIC
# dependencies, so this file (the consumer) must resolve every one of them
# itself, exactly as GeniusNetwork/GeniusSDK/cmake/CommonBuildParameters.cmake
# does for its own build. Ported verbatim from that file; only entries GCS
# already had (GTest, protobuf, OpenSSL, Microsoft.GSL, zlib, fmt, spdlog,
# libsecp256k1, nlohmann_json) are skipped here to avoid duplication.

# MNN
set(MNN_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/MNN/include")
set(MNN_DIR "${THIRDPARTY_BUILD_DIR}/MNN/lib/cmake/MNN")
find_package(MNN CONFIG REQUIRED)
include_directories(${MNN_INCLUDE_DIR})

# vk-bootstrap
set(vk-bootstrap_DIR "${THIRDPARTY_BUILD_DIR}/vk-bootstrap/lib/cmake/vk-bootstrap")
find_package(vk-bootstrap CONFIG REQUIRED)

# soralog
set(soralog_DIR "${THIRDPARTY_BUILD_DIR}/soralog/lib/cmake/soralog")
set(soralog_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/soralog/include")
find_package(soralog CONFIG REQUIRED)

# yaml-cpp
set(yaml-cpp_DIR "${THIRDPARTY_BUILD_DIR}/yaml-cpp/lib/cmake/yaml-cpp")
set(yaml-cpp_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/yaml-cpp/include")
find_package(yaml-cpp CONFIG REQUIRED)

# snappy
set(Snappy_DIR "${THIRDPARTY_BUILD_DIR}/snappy/lib/cmake/Snappy")
find_package(Snappy CONFIG REQUIRED)

# rocksdb
set(RocksDB_DIR "${THIRDPARTY_BUILD_DIR}/rocksdb/lib/cmake/rocksdb")
set(RocksDB_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/rocksdb/include")
find_package(RocksDB CONFIG REQUIRED)

# stb
include_directories(${THIRDPARTY_BUILD_DIR}/stb/include)

#  tsl_hat_trie
set(tsl_hat_trie_DIR "${THIRDPARTY_BUILD_DIR}/tsl_hat_trie/lib/cmake/tsl_hat_trie")
set(tsl_hat_trie_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/tsl_hat_trie/include")
find_package(tsl_hat_trie CONFIG REQUIRED)

# Boost.DI
set(Boost.DI_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/Boost.DI/include")
set(Boost.DI_DIR "${THIRDPARTY_BUILD_DIR}/Boost.DI/lib/cmake/Boost.DI")
find_package(Boost.DI CONFIG REQUIRED)

# SQLiteModernCpp
set(SQLiteModernCpp_ROOT_DIR "${THIRDPARTY_BUILD_DIR}/SQLiteModernCpp")
set(SQLiteModernCpp_DIR "${SQLiteModernCpp_ROOT_DIR}/lib/cmake/SQLiteModernCpp")
set(SQLiteModernCpp_LIB_DIR "${SQLiteModernCpp_ROOT_DIR}/lib")
set(SQLiteModernCpp_INCLUDE_DIR "${SQLiteModernCpp_ROOT_DIR}/include")

# sqlite3
set(sqlite3_ROOT_DIR "${THIRDPARTY_BUILD_DIR}/sqlite3")
set(sqlite3_DIR "${sqlite3_ROOT_DIR}/lib/cmake/sqlite3")
set(sqlite3_LIB_DIR "${sqlite3_ROOT_DIR}/lib")
set(sqlite3_INCLUDE_DIR "${sqlite3_ROOT_DIR}/include")

# cares
set(c-ares_DIR "${THIRDPARTY_BUILD_DIR}/cares/lib/cmake/c-ares" CACHE PATH "Path to c-ares install folder")
set(c-ares_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/cares/include" CACHE PATH "Path to c-ares include folder")

# libp2p
set(libp2p_DIR "${THIRDPARTY_BUILD_DIR}/libp2p/lib/cmake/libp2p")
set(libp2p_LIBRARY_DIR "${THIRDPARTY_BUILD_DIR}/libp2p/lib")
set(libp2p_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/libp2p/include")
find_package(libp2p CONFIG REQUIRED)

# Find and include cares if libp2p have not included it
if(NOT TARGET c-ares::cares_static)
    find_package(c-ares CONFIG REQUIRED)
endif()

# ipfs-lite-cpp
set(ipfs-lite-cpp_DIR "${THIRDPARTY_BUILD_DIR}/ipfs-lite-cpp/lib/cmake/ipfs-lite-cpp")
set(ipfs-lite-cpp_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/ipfs-lite-cpp/include")
set(ipfs-lite-cpp_LIB_DIR "${THIRDPARTY_BUILD_DIR}/ipfs-lite-cpp/lib")
set(CBOR_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/ipfs-lite-cpp/include/deps/tinycbor/src")
find_package(ipfs-lite-cpp CONFIG REQUIRED)

# ipfs-pubsub
set(ipfs-pubsub_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/ipfs-pubsub/include")
set(ipfs-pubsub_DIR "${THIRDPARTY_BUILD_DIR}/ipfs-pubsub/lib/cmake/ipfs-pubsub")
find_package(ipfs-pubsub CONFIG REQUIRED)

# ipfs-bitswap-cpp
set(ipfs-bitswap-cpp_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/ipfs-bitswap-cpp/include")
set(ipfs-bitswap-cpp_DIR "${THIRDPARTY_BUILD_DIR}/ipfs-bitswap-cpp/lib/cmake/ipfs-bitswap-cpp")
find_package(ipfs-bitswap-cpp CONFIG REQUIRED)

# ed25519
set(ed25519_DIR "${THIRDPARTY_BUILD_DIR}/ed25519/lib/cmake/ed25519")
set(ed25519_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/ed25519/include")
find_package(ed25519 CONFIG REQUIRED)

# RapidJSON
set(RapidJSON_DIR "${THIRDPARTY_BUILD_DIR}/rapidjson/lib/cmake/RapidJSON")
set(RapidJSON_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/rapidjson/include")
find_package(RapidJSON CONFIG REQUIRED)
include_directories(${RapidJSON_INCLUDE_DIR})

# jsonrpc-lean
set(jsonrpc_lean_INCLUDE_DIR "${THIRDPARTY_DIR}/jsonrpc-lean/include")
include_directories(${jsonrpc_lean_INCLUDE_DIR})

# xxhash
set(xxHash_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/xxhash/include")
set(xxHash_LIBRARY_DIR "${THIRDPARTY_BUILD_DIR}/xxhash/lib")
set(xxHash_DIR "${THIRDPARTY_BUILD_DIR}/xxhash/lib/cmake/xxHash")
find_package(xxHash CONFIG REQUIRED)

# Prefer package config files while loading Libssh2's dependencies.
# Libssh2 config calls `find_dependency(ZLIB)` without `CONFIG`, which can
# otherwise resolve to CMake's FindZLIB module on Windows CI.
set(_SGNS_CMAKE_FIND_PACKAGE_PREFER_CONFIG_WAS_DEFINED FALSE)
if(DEFINED CMAKE_FIND_PACKAGE_PREFER_CONFIG)
    set(_SGNS_CMAKE_FIND_PACKAGE_PREFER_CONFIG_WAS_DEFINED TRUE)
    set(_SGNS_CMAKE_FIND_PACKAGE_PREFER_CONFIG_PREV "${CMAKE_FIND_PACKAGE_PREFER_CONFIG}")
endif()
set(CMAKE_FIND_PACKAGE_PREFER_CONFIG ON)

# libssh2
set(Libssh2_DIR "${THIRDPARTY_BUILD_DIR}/libssh2/lib/cmake/libssh2")
find_package(Libssh2 CONFIG REQUIRED)

if(_SGNS_CMAKE_FIND_PACKAGE_PREFER_CONFIG_WAS_DEFINED)
    set(CMAKE_FIND_PACKAGE_PREFER_CONFIG "${_SGNS_CMAKE_FIND_PACKAGE_PREFER_CONFIG_PREV}")
else()
    unset(CMAKE_FIND_PACKAGE_PREFER_CONFIG)
endif()
unset(_SGNS_CMAKE_FIND_PACKAGE_PREFER_CONFIG_PREV)
unset(_SGNS_CMAKE_FIND_PACKAGE_PREFER_CONFIG_WAS_DEFINED)

# AsyncIOManager
set(AsyncIOManager_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/AsyncIOManager/include")
set(AsyncIOManager_LIBRARY_DIR "${THIRDPARTY_BUILD_DIR}/AsyncIOManager/lib")
set(AsyncIOManager_DIR "${THIRDPARTY_BUILD_DIR}/AsyncIOManager/lib/cmake/AsyncIOManager")
find_package(AsyncIOManager CONFIG REQUIRED)

# gnus_upnp
set(gnus_upnp_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/gnus_upnp/include")
set(gnus_upnp_LIBRARY_DIR "${THIRDPARTY_BUILD_DIR}/gnus_upnp/lib")
set(gnus_upnp_DIR "${THIRDPARTY_BUILD_DIR}/gnus_upnp/lib/cmake/gnus_upnp")
find_package(gnus_upnp CONFIG REQUIRED)

# wallet-core
set(TrustWalletCore_LIBRARY_DIR "${THIRDPARTY_BUILD_DIR}/wallet-core/lib")
set(TrustWalletCore_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/wallet-core/include")

find_library(TrezorCrypto_PATH TrezorCrypto PATHS ${TrustWalletCore_LIBRARY_DIR} REQUIRED)
find_library(wallet_core_rs_PATH wallet_core_rs PATHS ${TrustWalletCore_LIBRARY_DIR} REQUIRED)
find_library(TrustWalletCore_PATH TrustWalletCore PATHS ${TrustWalletCore_LIBRARY_DIR} REQUIRED)

add_library(TrezorCrypto STATIC IMPORTED)
add_library(wallet_core_rs STATIC IMPORTED)
add_library(TrustWalletCore STATIC IMPORTED)

set_target_properties(TrezorCrypto PROPERTIES IMPORTED_LOCATION "${TrezorCrypto_PATH}")
set_target_properties(wallet_core_rs PROPERTIES IMPORTED_LOCATION "${wallet_core_rs_PATH}")
set_target_properties(TrustWalletCore PROPERTIES IMPORTED_LOCATION "${TrustWalletCore_PATH}")

target_include_directories(TrustWalletCore INTERFACE "${TrustWalletCore_INCLUDE_DIR}")

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    find_package(PkgConfig)
    pkg_check_modules(LIBSECRET REQUIRED IMPORTED_TARGET libsecret-1>=0.18.4)
endif()

# --------------------------------------------------------
# zkLLVM / crypto3 (GCS's own CommonCompilerOptions.cmake already resolves
# ZKLLVM_BUILD_DIR — this only adds the IMPORTED targets GeniusSDK's own
# crypto3-consuming code links against, plus LLVM itself).
add_library(crypto3::algebra INTERFACE IMPORTED)
add_library(crypto3::block INTERFACE IMPORTED)
add_library(crypto3::blueprint INTERFACE IMPORTED)
add_library(crypto3::codec INTERFACE IMPORTED)
add_library(crypto3::math INTERFACE IMPORTED)
add_library(crypto3::multiprecision INTERFACE IMPORTED)
add_library(crypto3::pkpad INTERFACE IMPORTED)
add_library(crypto3::pubkey INTERFACE IMPORTED)
add_library(crypto3::random INTERFACE IMPORTED)
add_library(crypto3::zk INTERFACE IMPORTED)
add_library(marshalling::core INTERFACE IMPORTED)
add_library(marshalling::crypto3_algebra INTERFACE IMPORTED)
add_library(marshalling::crypto3_multiprecision INTERFACE IMPORTED)
add_library(marshalling::crypto3_zk INTERFACE IMPORTED)

foreach(_crypto3_tgt crypto3::algebra crypto3::block crypto3::blueprint crypto3::codec
                     crypto3::math crypto3::multiprecision crypto3::pkpad crypto3::pubkey
                     crypto3::random crypto3::zk marshalling::core marshalling::crypto3_algebra
                     marshalling::crypto3_multiprecision marshalling::crypto3_zk)
    set_target_properties(${_crypto3_tgt} PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ZKLLVM_BUILD_DIR}/zkLLVM/include"
    )
endforeach()

set(zkLLVM_INCLUDE_DIR "${ZKLLVM_BUILD_DIR}/zkLLVM/include")

# llvm
set(LLVM_DIR "${ZKLLVM_BUILD_DIR}/zkLLVM/lib/cmake/llvm")
find_package(LLVM CONFIG REQUIRED)

# --------------------------------------------------------
# Vulkan (GPU acceleration — needed by MNN/SGProcessingManager via
# GNUS-NEO-SWARM). Established here, before either add_subdirectory() call
# below, so the resulting Vulkan::Vulkan target is visible to both the
# GNUS-NEO-SWARM subtree and the GCS-level src/ subtree (sibling
# add_subdirectory scopes don't share targets with each other, only with
# their common parent). Ported from
# GeniusNetwork/GeniusSDK/cmake/CommonBuildParameters.cmake.
if(APPLE)
    if(IOS)
        # Settings specifically for iOS
        set(Vulkan_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/moltenvk/build/include")
        set(Vulkan_LIBRARY "${THIRDPARTY_BUILD_DIR}/moltenvk/build/lib/MoltenVK.xcframework")
    else()
        # Settings for macOS
        set(Vulkan_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/moltenvk/build/include")
        set(Vulkan_LIBRARY "${THIRDPARTY_BUILD_DIR}/moltenvk/build/lib/MoltenVK.xcframework")
    endif()
endif()

set(VulkanHeaders_DIR "${THIRDPARTY_BUILD_DIR}/Vulkan-Headers/share/cmake/VulkanHeaders" CACHE PATH "Path to Vulkan-Headers install folder")
find_package(VulkanHeaders CONFIG REQUIRED)
find_package(Vulkan)

if(NOT TARGET Vulkan::Vulkan)
    set(Vulkan_INCLUDE_DIR "${THIRDPARTY_BUILD_DIR}/Vulkan-Headers/include")
    if(NOT DEFINED $ENV{VULKAN_SDK})
        set(ENV{VULKAN_SDK} "${THIRDPARTY_BUILD_DIR}/Vulkan-Loader")
    endif()

    find_package(Vulkan REQUIRED)
endif()

# Force Vulkan::Vulkan to use the vendored Vulkan-Headers on every platform,
# even when find_package(Vulkan) resolves against a system-installed SDK.
# vk-bootstrap/MNN here are built against the vendored headers, so mixing in
# a different system header version causes unknown-type errors downstream.
set_target_properties(Vulkan::Vulkan PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${THIRDPARTY_BUILD_DIR}/Vulkan-Headers/include"
)

# --------------------------------------------------------
# SuperGenius (sibling repo under GeniusNetwork). Must be found BEFORE
# GeniusSDK below — GeniusSDK's exported sgns::GeniusSDK target links
# sgns::genius_node, which itself links SuperGenius's exported targets, so
# those packages need to already exist by the time find_package(GeniusSDK)
# resolves its own target graph. Ported verbatim from
# GeniusNetwork/GeniusSDK/cmake/CommonBuildParameters.cmake, which locates
# SuperGenius the same way for the exact same reason.
if(NOT DEFINED SUPERGENIUS_BUILD_DIR)
    if(NOT DEFINED SUPERGENIUS_DIR)
        if(EXISTS "${PROJECT_SUPER_ROOT}/SuperGenius")
            print("Setting default SuperGenius directory")
            set(SUPERGENIUS_DIR "${PROJECT_SUPER_ROOT}/SuperGenius" CACHE STRING "Default SuperGenius Library")

            # get absolute path
            cmake_path(SET SUPERGENIUS_DIR NORMALIZE "${SUPERGENIUS_DIR}")
        else()
            message(FATAL_ERROR "Cannot find SuperGenius directory required to build")
        endif()
    endif()
    print("Setting SuperGenius build directory default")
    get_filename_component(BUILD_PLATFORM_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(SUPERGENIUS_BUILD_DIR "${SUPERGENIUS_DIR}/build/${BUILD_PLATFORM_NAME}/${CMAKE_BUILD_TYPE}${ABI_SUBFOLDER_NAME}" CACHE STRING "Default Super Genius Build Directory")
endif()

# SuperGenius project
set(evmrelay_DIR "${SUPERGENIUS_BUILD_DIR}/SuperGenius/lib/cmake/evmrelay/")
set(SuperGenius_DIR "${SUPERGENIUS_BUILD_DIR}/SuperGenius/lib/cmake/SuperGenius/")
set(ProofSystem_DIR "${SUPERGENIUS_BUILD_DIR}/SuperGenius/lib/cmake/ProofSystem/")
set(SGProcessingManager_DIR "${SUPERGENIUS_BUILD_DIR}/SuperGenius/lib/cmake/SGProcessingManager/")

print("SuperGenius_DIR: ${SuperGenius_DIR}")

# shaderc installs no CMake package config, so SuperGenius hand-rolls this
# IMPORTED target rather than exporting one; SGProcessingManagerTargets.cmake's
# SGShaderCompiler link interface references shaderc::shaderc directly, so
# consumers of that export (like this file) must define the same target
# themselves before find_package(SGProcessingManager) resolves it below.
if(NOT TARGET shaderc::shaderc)
    add_library(shaderc::shaderc STATIC IMPORTED GLOBAL)
    set_target_properties(shaderc::shaderc PROPERTIES
        IMPORTED_LOCATION "${THIRDPARTY_BUILD_DIR}/shaderc/lib/${CMAKE_STATIC_LIBRARY_PREFIX}shaderc_combined${CMAKE_STATIC_LIBRARY_SUFFIX}"
        INTERFACE_INCLUDE_DIRECTORIES "${THIRDPARTY_BUILD_DIR}/shaderc/include"
    )
endif()

find_package(evmrelay CONFIG REQUIRED)
find_package(ProofSystem CONFIG REQUIRED)
find_package(SGProcessingManager CONFIG REQUIRED)
find_package(SuperGenius CONFIG REQUIRED)
include_directories(${SuperGenius_INCLUDE_DIR})

# --------------------------------------------------------
# GeniusSDK (sibling repo under GeniusNetwork — GeniusCognitiveSystem links
# directly against it). Located and found the same way GeniusSDK's own
# CommonBuildParameters.cmake locates SuperGenius: a sibling of
# PROJECT_SUPER_ROOT, with a per-platform/build-type install tree at
# <repo>/build/<Platform>/<BuildType>/<repo>/lib/cmake/<Package>/.
if(NOT DEFINED GENIUSSDK_BUILD_DIR)
    if(NOT DEFINED GENIUSSDK_DIR)
        if(EXISTS "${PROJECT_SUPER_ROOT}/GeniusSDK")
            print("Setting default GeniusSDK directory")
            set(GENIUSSDK_DIR "${PROJECT_SUPER_ROOT}/GeniusSDK" CACHE STRING "Default GeniusSDK Library")

            # get absolute path
            cmake_path(SET GENIUSSDK_DIR NORMALIZE "${GENIUSSDK_DIR}")
        else()
            message(FATAL_ERROR "Cannot find GeniusSDK directory required to build")
        endif()
    endif()
    print("Setting GeniusSDK build directory default")
    get_filename_component(BUILD_PLATFORM_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
    set(GENIUSSDK_BUILD_DIR "${GENIUSSDK_DIR}/build/${BUILD_PLATFORM_NAME}/${CMAKE_BUILD_TYPE}${ABI_SUBFOLDER_NAME}" CACHE STRING "Default GeniusSDK Build Directory")
endif()

# GeniusSDK project
set(GeniusSDK_DIR "${GENIUSSDK_BUILD_DIR}/GeniusSDK/lib/cmake/GeniusSDK/")
print("GeniusSDK_DIR: ${GeniusSDK_DIR}")
find_package(GeniusSDK CONFIG REQUIRED)

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

