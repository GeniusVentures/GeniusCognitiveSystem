# iOS bare-app build — app_build_ios target + .app install.
# Included from src/app/CMakeLists.txt's scope (see OSX/appBuild.cmake):
# GCS_FLUTTER_BUILD_MODE/_app_extra_deps are set by the includer, and
# CMAKE_CURRENT_SOURCE_DIR is src/app (include() does not change it).
#
# `flutter build ios` (no --simulator) emits the device .app at
# build/ios/iphoneos/Runner.app regardless of debug/release — the Xcode
# Runner target names the bundle, not the pubspec. --no-codesign keeps the
# dev loop signable-later; the .so embeds into Runner.app/Frameworks/ (the
# layout the bundled exe's @rpath resolves). Device runs are the
# integration_test flavor; this target only produces the .app artifact.

set(GCS_FLUTTER_APP_BUNDLE
    "${CMAKE_CURRENT_SOURCE_DIR}/build/ios/iphoneos/Runner.app")

add_custom_target(app_build_ios
    COMMAND ${FLUTTER_EXECUTABLE} build ios --no-codesign ${GCS_FLUTTER_BUILD_MODE}
    # Embed the FFI dylib as an app framework — same Contents/Frameworks
    # shape as the macOS bundle (self-contained, all-static GeniusSDK
    # chain: a plain copy is sufficient).
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "${GCS_FLUTTER_APP_BUNDLE}/Frameworks"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:gcs_ffi>" "${GCS_FLUTTER_APP_BUNDLE}/Frameworks"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${_app_extra_deps}
    COMMENT "Building bare Flutter app for iOS (${GCS_FLUTTER_BUILD_MODE})"
)

# `ninja install` copies the unsigned .app into <prefix>/bin/ (iOS builds
# run on a mac, so ditto exists). Skipped gracefully if the app hasn't been
# built yet (ninja app_build_ios), so C++-only installs still work.
install(CODE "
    if(EXISTS \"${GCS_FLUTTER_APP_BUNDLE}\")
        execute_process(COMMAND ditto \"${GCS_FLUTTER_APP_BUNDLE}\"
                \"\${CMAKE_INSTALL_PREFIX}/bin/Runner.app\")
        message(STATUS \"Installing: \${CMAKE_INSTALL_PREFIX}/bin/Runner.app\")
    else()
        message(STATUS \"Flutter app not built (ninja app_build_ios) — skipping app install\")
    endif()")
