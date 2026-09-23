# GCS Chat — Flutter App

Flutter frontend for the Genius Cognitive System chat app. Consumes the
`frontend_scaffold` widget library (git submodule at `scaffold/` — derive
from it, never edit it) and the C++ core through `gcs_ffi` (dart FFI).

## Build

The app builds through CMake, not bare `flutter build`, so codegen and the
native dylib are produced in the right order:

```sh
cmake -B build/OSX/Debug -DFRONTEND_BUILD_ENABLED=ON <platform args>
ninja -C build/OSX/Debug app_build_macos
```

Bundle lands at `build/macos/Build/Products/Debug/flutter_app.app` with
`libgcs_ffi.dylib` packaged under `Contents/Frameworks`.

## Generated code — never hand-edit

- `lib/generated/chat/` — chat composite widget/cubit/state triples, rendered
  from `templates/components/*.jinja2` by the scaffold engine.
  **Gitignored.** Regenerated automatically by
  `ninja app_generate_chat_components` (a dependency of `app_analyze`,
  `app_test`, and `app_build_macos`). Per-composite dev loop:
  `ninja generate_composite_chat_message_bubble` (etc.).
- `lib/generated/proto/` — Dart protobuf stubs from the shared
  `src/proto/gcs_chat.proto` (D-24 single source of truth). **Committed** —
  there is no CMake generator. Regenerate manually when the proto changes:

  ```sh
  cd src/app
  dart pub global activate protoc_plugin 22.5.0   # pin must match committed output
  protoc -I ../proto --dart_out=lib/generated/proto \
      --plugin=protoc-gen-dart=$(dart pub global run protoc_plugin:bin/protoc.dart) \
      ../proto/gcs_chat.proto
  ```

- `lib/gcs_bindings_generated.dart` — ffigen output; regenerate with `ffigen`
  per `ffigen.yaml` when `src/ffi/gcs_core.h` changes.
