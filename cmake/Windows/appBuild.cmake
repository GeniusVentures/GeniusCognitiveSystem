# Windows bare-app build — app_build_windows target + runner-dir install.
# Included from src/app/CMakeLists.txt's scope (see OSX/appBuild.cmake):
# GCS_FLUTTER_CONFIG_DIR/GCS_FLUTTER_BUILD_MODE/_app_extra_deps are set by
# the includer, and CMAKE_CURRENT_SOURCE_DIR is src/app (include() does not
# change it).
#
# Flutter emits the runner EXE at build/windows/x64/runner/<Config>/ — the
# dir IS the app on Windows (no bundle). The DLL is placed beside the EXE:
# the $exeDir/gcs_ffi.dll packaged layout SessionCubit's exe-relative
# fallback resolves. gcs_ffi.dll needs the vendored vulkan-1.dll beside it
# on loader-linked hosts (see cmake/CommonBuildParameters.cmake VULKAN_RUNTIME_DLL).

set(GCS_FLUTTER_APP_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/build/windows/x64/runner/${GCS_FLUTTER_CONFIG_DIR}")

add_custom_target(app_build_windows
    COMMAND ${FLUTTER_EXECUTABLE} build windows ${GCS_FLUTTER_BUILD_MODE}
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GCS_FLUTTER_APP_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:gcs_ffi>" "${GCS_FLUTTER_APP_DIR}"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${_app_extra_deps}
    COMMENT "Building bare Flutter app for Windows (${GCS_FLUTTER_BUILD_MODE})"
)

# `ninja install` copies the runner dir (EXE + DLLs + data/) into
# <prefix>/bin/flutter_app/. Skipped gracefully if the app hasn't been
# built yet (ninja app_build_windows), so C++-only installs still work.
install(CODE "
    if(EXISTS \"${GCS_FLUTTER_APP_DIR}/flutter_app.exe\")
        execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy_directory
                \"${GCS_FLUTTER_APP_DIR}\" \"\${CMAKE_INSTALL_PREFIX}/bin/flutter_app\")
        message(STATUS \"Installing: \${CMAKE_INSTALL_PREFIX}/bin/flutter_app\")
    else()
        message(STATUS \"Flutter app not built (ninja app_build_windows) — skipping app install\")
    endif()")
