# macOS bare-app build (moved verbatim from src/app/CMakeLists.txt — the
# platform gates there are gone; this fragment is glob-included by
# cmake/platform.cmake). Included from src/app/CMakeLists.txt's scope:
# GCS_FLUTTER_CONFIG_DIR/GCS_FLUTTER_BUILD_MODE/_app_extra_deps are set by
# the includer, and CMAKE_CURRENT_SOURCE_DIR is src/app (include() does not
# change it).

set(GCS_FLUTTER_APP_BUNDLE
    "${CMAKE_CURRENT_SOURCE_DIR}/build/macos/Build/Products/${GCS_FLUTTER_CONFIG_DIR}/flutter_app.app")

add_custom_target(app_build_macos
    COMMAND ${FLUTTER_EXECUTABLE} build macos ${GCS_FLUTTER_BUILD_MODE}
    # Package the FFI dylib next to the executable (Contents/Frameworks) —
    # the packaged layout SessionCubit's exe-relative fallback resolves,
    # and the layout a Windows/Linux package dir uses. Self-contained
    # dylib (all-static GeniusSDK chain): a plain copy is sufficient.
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${GCS_FLUTTER_APP_BUNDLE}/Contents/Frameworks"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:gcs_ffi>" "${GCS_FLUTTER_APP_BUNDLE}/Contents/Frameworks"
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
install(CODE "
    if(EXISTS \"${GCS_FLUTTER_APP_BUNDLE}\")
        execute_process(COMMAND ditto \"${GCS_FLUTTER_APP_BUNDLE}\"
                \"\${CMAKE_INSTALL_PREFIX}/bin/flutter_app.app\")
        message(STATUS \"Installing: \${CMAKE_INSTALL_PREFIX}/bin/flutter_app.app\")
    else()
        message(STATUS \"Flutter app not built (ninja app_build_macos) — skipping app install\")
    endif()")
