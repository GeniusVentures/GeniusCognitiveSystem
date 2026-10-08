# gcs_storage: GCS GlobalDB component (moved from GNUS-NEO-SWARM/src/storage per D-25).
# Built as part of the parent GCS build via src/CMakeLists.txt add_subdirectory.

add_library(gcs_storage STATIC
    gcs_global_db.cpp
    common/error.cpp
)

# Self-include resolves "gcs_storage/gcs_global_db.hpp" (parent dir = src/lib/);
# current dir resolves "common/error.hpp" + "common/logging.hpp" (vendored copies —
# gcs_storage does not depend on the GNUS-NEO-SWARM include tree).
target_include_directories(gcs_storage PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)

# SuperGenius CRDT GlobalDB — hard required (project Conditional Compilation rule:
# missing required libraries fail at configure time, never degrade to a stub).
if(TARGET sgns::crdt_globaldb)
    target_link_libraries(gcs_storage PUBLIC sgns::crdt_globaldb)
else()
    message(FATAL_ERROR "sgns::crdt_globaldb target not found — SuperGenius package must be configured before gcs_storage")
endif()

# GeniusSDK — hard required: GcsGlobalDb::Initialize() calls GeniusSDKGetNode().
# Static target first (no runtime shared-lib search path); shared is the fallback.
if(TARGET sgns::GeniusSDK)
    target_link_libraries(gcs_storage PUBLIC sgns::GeniusSDK)
    message(STATUS "GeniusSDK (static) linked to gcs_storage")
elseif(TARGET sgns::GeniusSDK_shared)
    target_link_libraries(gcs_storage PUBLIC sgns::GeniusSDK_shared)
    message(STATUS "GeniusSDK (shared) linked to gcs_storage")
else()
    message(FATAL_ERROR "sgns::GeniusSDK / sgns::GeniusSDK_shared target not found — GeniusSDK package must be configured before gcs_storage")
endif()

# SuperGenius account headers (mirrors GNUS-NEO-SWARM/src/storage/CMakeLists.txt)
target_include_directories(gcs_storage PUBLIC
    "${SUPERGENIUS_BUILD_DIR}/SuperGenius/include"
)

# Direct deps of gcs_global_db.cpp not re-exported by sgns::crdt_globaldb's interface.
target_include_directories(gcs_storage PUBLIC
    "${THIRDPARTY_BUILD_DIR}/ipfs-pubsub/include"
    "${THIRDPARTY_BUILD_DIR}/ipfs-lite-cpp/include"
    "${THIRDPARTY_BUILD_DIR}/libp2p/include"
    "${THIRDPARTY_BUILD_DIR}/boost/build/include"
)

# GeniusSDK.hpp is not installed by the GeniusSDK build; consume it from the source tree.
target_include_directories(gcs_storage PUBLIC
    "${GENIUSSDK_DIR}/src"
)

# Transitive headers pulled in by GeniusSDK.hpp's account/GeniusNode.hpp chain.
target_include_directories(gcs_storage PUBLIC
    "${THIRDPARTY_BUILD_DIR}/wallet-core/include"
    "${SUPERGENIUS_BUILD_DIR}/SuperGenius/include/evmrelay"
    "${MNN_INCLUDE_DIR}"
)
if(DEFINED ZKLLVM_BUILD_DIR AND NOT ZKLLVM_BUILD_DIR STREQUAL "")
    target_include_directories(gcs_storage PUBLIC
        "${ZKLLVM_BUILD_DIR}/zkLLVM/include"
    )
endif()
