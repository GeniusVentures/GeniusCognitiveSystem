# gcs_ffi — SHARED library exposing the minimal topic pub/sub C ABI to Flutter (D-01, D-02, D-27, D-29).
add_library(gcs_ffi SHARED
    gcs_core_ffi.cpp
    dart_api_dl.c
)
set_target_properties(gcs_ffi PROPERTIES
    PUBLIC_HEADER gcs_core.h
    OUTPUT_NAME "gcs_ffi"
)
# Expose src/ so consumers (test/test_gcs_ffi.cpp) can #include "ffi/gcs_core.h".
target_include_directories(gcs_ffi PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/..>
)
# Vendored Dart API_DL set (dart_native_api.h / dart_api_dl.h / internal/) —
# resolves the Dart_PostCObject DL indirection includes from src/ffi itself.
target_include_directories(gcs_ffi PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_link_libraries(gcs_ffi PRIVATE gcs_core gcs_proto)
# ELF symbol isolation: gcs_ffi statically embeds MNN (gcs_core ->
# neoswarm_core -> MNN::MNN), and executables that link BOTH this dylib and
# the SuperGenius static chain (SGProcessors whole-archives MNN into the
# exe) end up with two MNN runtime registries in one process. On ELF's flat
# namespace the exe's symbols interpose into the dylib, so the second
# registration trips MNNInsertExtraRuntimeCreator's "duplicate type" assert
# (Debug only — MNN_ASSERT is compiled out by NDEBUG in Release; macOS never
# saw this because dyld's two-level namespace binds each image to its own
# symbols). -Bsymbolic binds the dylib's internal references locally, giving
# its embedded MNN registry self-contained state, mirroring the macOS
# behavior. -Bsymbolic-functions (not plain -Bsymbolic) so vtables/typeinfo
# data symbols keep default binding — only the function calls matter here.
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_options(gcs_ffi PRIVATE "-Wl,-Bsymbolic-functions")
endif()
# GCS_FFI_EXPORTS drives the gcs_* dllexport macro (gcs_core.h).
# DART_SHARED_LIB makes DART_EXPORT resolve to __declspec(dllexport) so
# Dart_InitializeApiDL (declared DART_EXPORT in dart_api_dl.h, defined in
# dart_api_dl.c) lands in gcs_ffi.dll's export table — without it the
# Dart-side DynamicLibrary.lookup('Dart_InitializeApiDL') fails on Windows.
target_compile_definitions(gcs_ffi PRIVATE GCS_FFI_EXPORTS DART_SHARED_LIB)
if(ANDROID)
    target_link_options(gcs_ffi PRIVATE "-Wl,-z,max-page-size=16384")
endif()
# Ad-hoc code-sign the dylib immediately after link on Apple platforms. The
# dylib is built here, outside Xcode, and lands unsigned; when Xcode embeds
# it into a macOS app bundle (Contents/Frameworks) the app-level signature
# fails on the unsigned nested code object, and Gatekeeper requires a
# signature for local execution anyway. Ad-hoc ("-") needs no identity, so
# dev machines and CI runners both satisfy it. Xcode re-signs with a real
# identity at embed time when one is configured (GeniusWallet-style CI).
# Sign at link time — never install time — because Flutter/Xcode consume the
# artifact from the build tree; install() then copies the signed file as-is.
if(APPLE)
    find_program(CODESIGN_EXECUTABLE codesign)
    if(CODESIGN_EXECUTABLE)
        add_custom_command(TARGET gcs_ffi POST_BUILD
            COMMAND ${CODESIGN_EXECUTABLE} --force --sign - $<TARGET_FILE:gcs_ffi>
            COMMENT "Ad-hoc signing gcs_ffi (Apple embed requirement)"
            VERBATIM
        )
    endif()
endif()
# Install alongside the rest of the tree so consumers (Dart test harness,
# plugin staging, packaging) take the artifact from one stable per-platform
# location — never from the CMake build dir (LIBRARY: .so/.dylib, RUNTIME:
# .dll, ARCHIVE: Windows import lib).
install(TARGETS gcs_ffi
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    ARCHIVE DESTINATION lib
    PUBLIC_HEADER DESTINATION include
)
