/// GCS chat app entry point (plan 01-11 Task 3, D-10/D-11/D-23).
///
/// The root registers the scaffold token themes (D-12/D-22 -- see
/// `lib/theme/gcs_theme.dart`) and mounts the [GCSChat] shell. No legacy
/// chat-kit imports, no seed color scheme, no direct SLM/FFI calls here:
/// the shell's [SessionCubit] owns the GCS session lifecycle.
library;

import 'package:flutter/material.dart';

import 'package:flutter_app/shell/gcs_shell.dart';
import 'package:flutter_app/theme/gcs_theme.dart';

void main() {
  runApp(const GCSChatApp());
}

/// Root widget: themed Material 3 host for the GCS chat shell.
class GCSChatApp extends StatelessWidget {
  /// Creates the app root.
  const GCSChatApp({super.key});

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'GCS Chat',
      theme: GcsTheme.light,
      darkTheme: GcsTheme.dark,
      themeMode: ThemeMode.system,
      home: const GCSChat(),
    );
  }
}
