# Android Dart ctest legs — none: the host Dart VM cannot load the
# device-arch gcs_ffi artifact, so a host `flutter test` leg can never pass;
# device coverage is the integration_test flavor. FRONTEND_TESTS_ENABLED
# therefore defaults OFF here (ON only ever makes sense on the desktop
# fragments).
option(FRONTEND_TESTS_ENABLED "Register Dart/flutter tests in ctest" OFF)
