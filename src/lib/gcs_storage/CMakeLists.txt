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

# GeniusSDK — hard required: GcsGlobalDb::Initialize() calls GeniusSDKGetNode() (D-16a).
#
# The CMake-imported sgns::GeniusSDK_shared target's IMPORTED_LOCATION points at the
# install prefix (${GENIUS_SDK_BUILD_DIR}/GeniusSDK/lib/libGeniusSDK_shared.dylib),
# which lags the build tree output. GeniusSDKGetNode() was added in d550800 (Aug 3)
# and is present in the build-tree dylib at ${GENIUS_SDK_BUILD_DIR}/src/... but not
# yet in the install-prefix copy. Redirect the imported location at the newer dylib
# when both exist; otherwise leave the imported default (which will fail at link
# time with a clear "Undefined symbols: GeniusSDKGetNode" — the failure signal to
# re-run `ninja install` in the GeniusSDK build).
if(TARGET sgns::GeniusSDK_shared)
    set(_GENIUSSDK_BUILD_DYLIB "${GENIUS_SDK_BUILD_DIR}/src/libGeniusSDK_shared.dylib")
    if(EXISTS "${_GENIUSSDK_BUILD_DYLIB}")
        set_target_properties(sgns::GeniusSDK_shared PROPERTIES
            IMPORTED_LOCATION_DEBUG   "${_GENIUSSDK_BUILD_DYLIB}"
            IMPORTED_LOCATION_RELEASE "${_GENIUSSDK_BUILD_DYLIB}"
            IMPORTED_LOCATION         "${_GENIUSSDK_BUILD_DYLIB}"
        )
        message(STATUS "sgns::GeniusSDK_shared redirected to build-tree dylib (has GeniusSDKGetNode): ${_GENIUSSDK_BUILD_DYLIB}")
    endif()
    target_link_libraries(gcs_storage PUBLIC sgns::GeniusSDK_shared)
    message(STATUS "GeniusSDK linked to gcs_storage")
elseif(TARGET sgns::GeniusSDK)
    target_link_libraries(gcs_storage PUBLIC sgns::GeniusSDK)
    message(STATUS "GeniusSDK (static) linked to gcs_storage")
else()
    message(FATAL_ERROR "sgns::GeniusSDK_shared / sgns::GeniusSDK target not found — GeniusSDK package must be configured before gcs_storage")
endif()

# SuperGenius account headers (mirrors GNUS-NEO-SWARM/src/storage/CMakeLists.txt)
target_include_directories(gcs_storage PUBLIC
    "${SUPERGENIUS_BUILD_DIR}/SuperGenius/include"
)

# Headers for direct deps of gcs_global_db.cpp that sgns::crdt_globaldb does not
# re-export via INTERFACE_INCLUDE_DIRECTORIES (its interface carries only the
# SuperGenius install include dir). The ipfs-pubsub imported target is a stub
# (cmake/CommonBuildParameters.cmake), so include its headers directly.
target_include_directories(gcs_storage PUBLIC
    "${THIRDPARTY_BUILD_DIR}/ipfs-pubsub/include"
    "${THIRDPARTY_BUILD_DIR}/ipfs-lite-cpp/include"
    "${THIRDPARTY_BUILD_DIR}/libp2p/include"
    "${THIRDPARTY_BUILD_DIR}/boost/build/include"
)

# GeniusSDK.hpp (C++ API exposing GeniusSDKGetNode, added in GeniusSDK d550800) is not
# yet installed into ${GENIUS_SDK_BUILD_DIR}/GeniusSDK/include (the build only exports
# the C header). Consume it directly from the GeniusSDK source tree.
target_include_directories(gcs_storage PUBLIC
    "${GENIUS_SDK_DIR}/src"
)

# GeniusSDK.hpp -> account/GeniusNode.hpp -> WalletCore/PrivateKey.h. TrustWalletCore
# is a stub-imported target in cmake/CommonBuildParameters.cmake, so expose its
# headers explicitly here.
target_include_directories(gcs_storage PUBLIC
    "${THIRDPARTY_BUILD_DIR}/wallet-core/include"
)

# GeniusNode.hpp transitively pulls ProofSystem/EthereumKeyPairParams.hpp which
# requires crypto3 (zkLLVM). ZKLLVM_BUILD_DIR is set by CommonCompilerOptions.cmake.
if(DEFINED ZKLLVM_BUILD_DIR AND NOT ZKLLVM_BUILD_DIR STREQUAL "")
    target_include_directories(gcs_storage PUBLIC
        "${ZKLLVM_BUILD_DIR}/zkLLVM/include"
    )
endif()

# GeniusNode.hpp -> account/BridgeRelayer.hpp -> eth/eth_watch_service.hpp. evmrelay's
# headers live under the SuperGenius install include dir at /evmrelay, not at the top.
target_include_directories(gcs_storage PUBLIC
    "${SUPERGENIUS_BUILD_DIR}/SuperGenius/include/evmrelay"
)

# GeniusNode.hpp -> processing_processor_mnn_image.hpp -> MNN/ImageProcess.hpp.
# MNN_INCLUDE_DIR is set in cmake/CommonBuildParameters.cmake.
target_include_directories(gcs_storage PUBLIC
    "${MNN_INCLUDE_DIR}"
)
