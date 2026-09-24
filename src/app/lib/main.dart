/// GCS chat app entry point (plan 01-11 Task 3, D-10/D-11/D-23).
///
/// The root registers the scaffold token themes (D-12/D-22 -- see
/// `lib/theme/gcs_theme.dart`) and mounts the [GCSChat] shell. No legacy
/// chat-kit imports, no seed color scheme, no direct SLM/FFI calls here:
/// the shell's [SessionCubit] owns the GCS session lifecycle.
///
/// The session database lives under the per-user application-support
/// directory at `data/KEY` — [kDefaultInstanceKey] for a normal launch, or
/// the key passed via `--instance=KEY` so a second app instance gets its
/// own database (multi-instance testing/support) without touching the
/// first.
library;

import 'dart:io';

import 'package:flutter/material.dart';
import 'package:flutter_app/shell/gcs_shell.dart';
import 'package:flutter_app/theme/gcs_theme.dart';
import 'package:path_provider/path_provider.dart';

/// Subdirectory under the application-support directory that holds the
/// per-instance session databases.
const String kDataSubdirectory = 'data';

/// Instance key used when the app launches without [kInstanceFlag]: the
/// default single-instance database (`data/default`).
const String kDefaultInstanceKey = 'default';

/// Command-line flag selecting the instance key. A launch such as
/// `flutter_app --instance=second` stores its database under `data/second`,
/// isolated from the default instance.
const String kInstanceFlag = '--instance=';

/// Extracts the instance key from [args]: the value of the first
/// [kInstanceFlag] argument, or [kDefaultInstanceKey] when absent/empty.
String instanceKeyFromArgs(List<String> args) {
  for (final String arg in args) {
    if (arg.startsWith(kInstanceFlag) && arg.length > kInstanceFlag.length) {
      return arg.substring(kInstanceFlag.length);
    }
  }
  return kDefaultInstanceKey;
}

/// Resolves the session database path for [instanceKey]:
/// `<application-support>/data/KEY`, created if missing.
Future<String> resolveDbPath(String instanceKey) async {
  final Directory support = await getApplicationSupportDirectory();
  final String dbPath =
      '${support.path}/$kDataSubdirectory/$instanceKey';
  Directory(dbPath).createSync(recursive: true);
  return dbPath;
}

Future<void> main(List<String> args) async {
  WidgetsFlutterBinding.ensureInitialized();
  final String dbPath = await resolveDbPath(instanceKeyFromArgs(args));
  runApp(GCSChatApp(dbPath: dbPath));
}

/// Root widget: themed Material 3 host for the GCS chat shell.
class GCSChatApp extends StatelessWidget {
  /// Creates the app root.
  const GCSChatApp({super.key, this.dbPath});

  /// Session database path derived in [main] (per-user `data/KEY`); handed
  /// to the shell's [SessionCubit.openDefault] call.
  final String? dbPath;

  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      title: 'GCS Chat',
      theme: GcsTheme.light,
      darkTheme: GcsTheme.dark,
      themeMode: ThemeMode.system,
      home: GCSChat(dbPath: dbPath),
    );
  }
}
