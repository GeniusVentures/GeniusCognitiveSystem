# Linux bare-app build — app_build_linux target + bundle-dir install.
# Included from src/app/CMakeLists.txt's scope (see OSX/appBuild.cmake):
# GCS_FLUTTER_CONFIG_DIR/GCS_FLUTTER_BUILD_MODE/_app_extra_deps are set by
# the includer, and CMAKE_CURRENT_SOURCE_DIR is src/app (include() does not
# change it).
#
# Flutter emits the app as a self-contained bundle directory at
# build/linux/<arch>/<config>/bundle/ — the exe (flutter_app, named by the
# pubspec), lib/, and data/ live there. Unlike macOS/Windows, the config dir
# component is lowercase (debug/release). The .so goes beside the flutter
# engine libs in bundle/lib/ (the runner's $ORIGIN/lib rpath).

string(TOLOWER "${GCS_FLUTTER_CONFIG_DIR}" _gcs_linux_config)
if(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
    set(_gcs_linux_arch "x64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm64")
    set(_gcs_linux_arch "arm64")
else()
    # Unknown processor: pass it through — flutter errors loudly if the arch
    # dir does not match what it built.
    set(_gcs_linux_arch "${CMAKE_SYSTEM_PROCESSOR}")
endif()

set(GCS_FLUTTER_APP_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/build/linux/${_gcs_linux_arch}/${_gcs_linux_config}/bundle")

add_custom_target(app_build_linux
    COMMAND ${FLUTTER_EXECUTABLE} build linux ${GCS_FLUTTER_BUILD_MODE}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GCS_FLUTTER_APP_DIR}/lib"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:gcs_ffi>" "${GCS_FLUTTER_APP_DIR}/lib"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${_app_extra_deps}
    COMMENT "Building bare Flutter app for Linux (${GCS_FLUTTER_BUILD_MODE})"
)

# `ninja install` copies the bundle dir (exe + lib/ + data/) into
# <prefix>/bin/flutter_app/. Skipped gracefully if the app hasn't been
# built yet (ninja app_build_linux), so C++-only installs still work.
install(CODE "
    if(EXISTS \"${GCS_FLUTTER_APP_DIR}/flutter_app\")
        execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_directory
                \"${GCS_FLUTTER_APP_DIR}\" \"\${CMAKE_INSTALL_PREFIX}/bin/flutter_app\")
        message(STATUS \"Installing: \${CMAKE_INSTALL_PREFIX}/bin/flutter_app\")
    else()
        message(STATUS \"Flutter app not built (ninja app_build_linux) — skipping app install\")
    endif()")
