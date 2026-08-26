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

# --- GCS chat composite codegen (D-17): bubble-only loop, Dart triple only.
# Mirrors scaffold CMakeLists.txt §6 (one stamp per rendered file, per-stamp
# DEPENDS on template + tokens + engine + vars for the incremental
# re-render invariant). The C++ half of this composite is the protobuf
# ChatMessageState generated from src/proto/gcs_chat.proto by plan 01-03
# (D-26/D-29) — there is deliberately no C++ template rendering here.
# 01-09 (code_block/media) and 01-10 (flow) extend the aggregate target.
set(_chat_templates_dir "${CMAKE_CURRENT_SOURCE_DIR}/templates/components")
set(_bubble_vars "${_chat_templates_dir}/chat_message_bubble_vars.json")

set(_bubble_stamps "")
set(_bubble_outputs "")
if(PYTHON3_EXECUTABLE AND FRONTEND_TARGET STREQUAL "flutter")
    # List form so each flag expands to TWO argv tokens (Pitfall 1762a45: a
    # single space-embedded string reaches argparse as one token and fails).
    # OUR templates dir FIRST — FileSystemLoader is first-match-wins.
    set(_bubble_template_dirs
        --template-dir "${CMAKE_CURRENT_SOURCE_DIR}/templates"
        --template-dir "${CMAKE_CURRENT_SOURCE_DIR}/scaffold/templates"
    )

    foreach(_kind "" "_cubit" "_state")
        set(_bubble_template
            "${_chat_templates_dir}/chat_message_bubble${_kind}.dart.jinja2")
        set(_bubble_stamp
            "${CMAKE_CURRENT_BINARY_DIR}/template_gen/chat_message_bubble${_kind}.stamp")
        set(_bubble_output
            "${GENERATED_DIR}/chat/chat_message_bubble${_kind}.dart")

        add_custom_command(
            OUTPUT ${_bubble_stamp} ${_bubble_output}
            COMMAND ${CMAKE_COMMAND} -E make_directory
                "${CMAKE_CURRENT_BINARY_DIR}/template_gen"
            COMMAND ${PYTHON3_EXECUTABLE} ${ENGINE_SCRIPT}
                --template "components/chat_message_bubble${_kind}.dart.jinja2"
                --tokens ${DESIGN_TOKENS}
                --output ${_bubble_output}
                ${_bubble_template_dirs}
                --vars ${_bubble_vars}
            COMMAND ${CMAKE_COMMAND} -E touch ${_bubble_stamp}
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Rendering chat composite: chat_message_bubble${_kind}"
            DEPENDS ${_bubble_template} ${DESIGN_TOKENS} ${ENGINE_SCRIPT} ${_bubble_vars}
        )
        list(APPEND _bubble_stamps ${_bubble_stamp})
        list(APPEND _bubble_outputs ${_bubble_output})
    endforeach()
endif()

# Declared unconditionally (empty DEPENDS when python3 is absent) so
# app_analyze/app_test below never reference a dangling target.
add_custom_target(generate_composite_chat_message_bubble
    COMMENT "Generate chat_message_bubble widget/cubit/state triple"
    DEPENDS ${_bubble_stamps} ${_bubble_outputs}
)
# Aggregate for ALL chat composites (D-17) — bubble-only for now.
add_custom_target(app_generate_chat_components
    COMMENT "Generate all chat composite widget/cubit/state triples"
    DEPENDS ${_bubble_stamps} ${_bubble_outputs}
)

# App-level dev-loop targets. app_analyze/app_test run after dependency
# resolution AND chat composite codegen so analyze/tests always see freshly
# generated Dart. Never depend on the scaffold's OpenAPI aggregate/api
# targets — GCS has no api-specs (Pitfall U4).
add_custom_target(app_pub_get
    COMMAND ${DART_EXECUTABLE} pub get
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Resolving app Dart dependencies"
)
add_custom_target(app_analyze
    COMMAND ${DART_EXECUTABLE} analyze --fatal-infos
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS app_pub_get app_generate_chat_components
)
add_custom_target(app_test
    COMMAND ${FLUTTER_EXECUTABLE} test
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS app_pub_get app_generate_chat_components
)

# Bare-app desktop build. Host-desktop only (macOS here) — the equivalent
# windows/linux targets can be added the same way when those hosts are needed.
# Same Flutter invocation `flutter run -d macos` uses under the hood.
if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # Map the CMake config to Flutter's build mode/product dir so a Release
    # CMake tree produces the Release bundle the install block looks for
    # (Codex PR #11 P1: hardcoded --debug made non-Debug installs silently
    # skip the app bundle).
    string(TOUPPER "${CMAKE_BUILD_TYPE}" _gcs_build_type_upper)
    if(_gcs_build_type_upper STREQUAL "DEBUG")
        set(GCS_FLUTTER_CONFIG_DIR "Debug")
        set(GCS_FLUTTER_BUILD_MODE "--debug")
    elseif(_gcs_build_type_upper STREQUAL "RELEASE")
        set(GCS_FLUTTER_CONFIG_DIR "Release")
        set(GCS_FLUTTER_BUILD_MODE "--release")
    else()
        set(GCS_FLUTTER_CONFIG_DIR "RelWithDebInfo")
        # Flutter has no RelWithDebInfo; --release is the closest (opt build).
        set(GCS_FLUTTER_BUILD_MODE "--release")
    endif()

    set(_app_extra_deps app_pub_get)
    if(TARGET app_stage_ffi_dylib)
        list(APPEND _app_extra_deps app_stage_ffi_dylib)
    endif()
    add_custom_target(app_build_macos
        COMMAND ${FLUTTER_EXECUTABLE} build macos ${GCS_FLUTTER_BUILD_MODE}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS ${_app_extra_deps}
        COMMENT "Building bare Flutter app for macOS (${GCS_FLUTTER_BUILD_MODE})"
    )

    # `ninja install` copies the built .app bundle into <prefix>/bin/
    # (i.e. build/OSX/Debug/GeniusCogntiveSystem/bin/, next to neo-swarm).
    # Flutter always builds under src/app/build/... (not configurable), so
    # this bridges it into the CMake install tree. ditto preserves the
    # framework symlink structure. Skipped gracefully if the app hasn't been
    # built yet (ninja app_build_macos), so C++-only installs still work.
    set(GCS_FLUTTER_APP_BUNDLE
        "${CMAKE_CURRENT_SOURCE_DIR}/build/macos/Build/Products/${GCS_FLUTTER_CONFIG_DIR}/flutter_app.app")
    install(CODE "
        if(EXISTS \"${GCS_FLUTTER_APP_BUNDLE}\")
            execute_process(COMMAND ditto \"${GCS_FLUTTER_APP_BUNDLE}\"
                    \"\${CMAKE_INSTALL_PREFIX}/bin/flutter_app.app\")
            message(STATUS \"Installing: \${CMAKE_INSTALL_PREFIX}/bin/flutter_app.app\")
        else()
            message(STATUS \"Flutter app not built (ninja app_build_macos) — skipping app install\")
        endif()")
endif()
