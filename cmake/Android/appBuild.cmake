# Android bare-app build — app_build_android target + APK install.
# Included from src/app/CMakeLists.txt's scope (see OSX/appBuild.cmake):
# GCS_FLUTTER_CONFIG_DIR/GCS_FLUTTER_BUILD_MODE/_app_extra_deps are set by
# the includer, and CMAKE_CURRENT_SOURCE_DIR is src/app (include() does not
# change it).
#
# An APK has no "beside the exe" — the FFI .so must be inside the package.
# Android gradle packaging sweeps android/app/src/main/jniLibs/<abi>/, so
# the .so is copied there BEFORE `flutter build apk` (ANDROID_ABI comes from
# build/Android's wrapper, default arm64-v8a). Output APK:
# build/app/outputs/flutter-apk/app-<config>.apk (config lowercase, like
# Linux — unlike macOS/Windows).

string(TOLOWER "${GCS_FLUTTER_CONFIG_DIR}" _gcs_android_config)
set(GCS_FLUTTER_JNILIBS_DIR
    "${CMAKE_CURRENT_SOURCE_DIR}/android/app/src/main/jniLibs/${ANDROID_ABI}")
set(GCS_FLUTTER_APK
    "${CMAKE_CURRENT_SOURCE_DIR}/build/app/outputs/flutter-apk/app-${_gcs_android_config}.apk")

add_custom_target(app_build_android
    COMMAND ${CMAKE_COMMAND} -E make_directory "${GCS_FLUTTER_JNILIBS_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:gcs_ffi>" "${GCS_FLUTTER_JNILIBS_DIR}/libgcs_ffi.so"
    COMMAND ${FLUTTER_EXECUTABLE} build apk ${GCS_FLUTTER_BUILD_MODE}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    DEPENDS ${_app_extra_deps}
    COMMENT "Building bare Flutter app for Android (${GCS_FLUTTER_BUILD_MODE}, ${ANDROID_ABI})"
)

# `ninja install` copies the APK into <prefix>/bin/ as
# flutter_app-<config>.apk (config in the name keeps debug/release installs
# distinct). Skipped gracefully if the app hasn't been built yet
# (ninja app_build_android), so C++-only installs still work.
install(CODE "
    if(EXISTS \"${GCS_FLUTTER_APK}\")
        execute_process(COMMAND \"${CMAKE_COMMAND}\" -E copy
                \"${GCS_FLUTTER_APK}\"
                \"\${CMAKE_INSTALL_PREFIX}/bin/flutter_app-${_gcs_android_config}.apk\")
        message(STATUS \"Installing: \${CMAKE_INSTALL_PREFIX}/bin/flutter_app-${_gcs_android_config}.apk\")
    else()
        message(STATUS \"Flutter app not built (ninja app_build_android) — skipping app install\")
    endif()")
