/// GCS chat app entry point (plan 01-11 Task 3, D-10/D-11/D-23).
///
/// The root registers the scaffold token themes (D-12/D-22 -- see
/// `lib/theme/gcs_theme.dart`) and mounts the [GCSChat] shell. No legacy
/// chat-kit imports, no seed color scheme, no direct SLM/FFI calls here:
/// the shell's [SessionCubit] owns the GCS session lifecycle.
///
/// The per-instance directory lives under the per-user
/// application-support directory at `data/KEY` — [kDefaultInstanceKey] for
/// a normal launch, or the key passed via `--instance=KEY` so a second app
/// instance gets its own node without touching the first. The instance
/// directory is the embedded node's base path (wallet/identity, network
/// config, logs — SuperGenius derives the libp2p port from the wallet
/// address), and the session database sits in its `db` subdirectory.
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

/// Shape every instance key must fit (IN-04): the key is spliced into a
/// filesystem path (`data/KEY/db`), so anything outside [A-Za-z0-9_-] —
/// path separators, '..', NUL-adjacent oddities — would break the
/// per-instance isolation contract (each key must boot its own wallet and
/// identity under the application-support directory, never escape it).
final RegExp kInstanceKeyPattern = RegExp(r'^[A-Za-z0-9_-]{1,64}$');

/// Extracts the instance key from [args]: the value of the first
/// [kInstanceFlag] argument, or [kDefaultInstanceKey] when absent/empty.
/// A key that fails [kInstanceKeyPattern] falls back to
/// [kDefaultInstanceKey] (IN-04: adversarial keys never splice into the
/// data path).
String instanceKeyFromArgs(List<String> args) {
  for (final String arg in args) {
    if (arg.startsWith(kInstanceFlag) && arg.length > kInstanceFlag.length) {
      final String key = arg.substring(kInstanceFlag.length);
      if (kInstanceKeyPattern.hasMatch(key)) {
        return key;
      }
      return kDefaultInstanceKey;
    }
  }
  return kDefaultInstanceKey;
}

/// Resolves the session database path for [instanceKey]:
/// `<application-support>/data/KEY/db`, created if missing. The parent
/// `data/KEY` directory doubles as the embedded node's base path — each
/// instance key therefore boots with its own wallet (own peer identity and
/// sender address) and its own address-derived libp2p port, so multiple
/// instances coexist as genuine separate nodes.
Future<String> resolveDbPath(String instanceKey) async {
  final Directory support = await getApplicationSupportDirectory();
  final String dbPath =
      '${support.path}/$kDataSubdirectory/$instanceKey/db';
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
