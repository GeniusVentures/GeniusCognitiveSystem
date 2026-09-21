/// GCS theme registration (D-12/D-22).
///
/// The app never hand-rolls a color scheme: light and dark `ThemeData` are
/// built on the scaffold design tokens, registered as `ThemeExtension`s so
/// every atom/composite resolves `context.palette` / `context.dimens`
/// (`frontend_scaffold/theme/scaffold_theme.dart`). Light mode registers
/// [ScaffoldPalette.lightPalette]; dark mode registers the scaffold's default
/// (dark) palette. Material 3 only -- the app never derives its colors from
/// a hand-rolled seed color scheme (D-12).
library;

import 'package:flutter/material.dart';
import 'package:frontend_scaffold/theme/scaffold_dimens.dart';
import 'package:frontend_scaffold/theme/scaffold_palette.dart';

/// Light and dark app themes wired to the scaffold token system.
abstract final class GcsTheme {
  GcsTheme._();

  /// Light theme: scaffold light palette + default dimens (D-22).
  static final ThemeData light = ThemeData(
    brightness: Brightness.light,
    useMaterial3: true,
    extensions: const <ThemeExtension<dynamic>>[
      ScaffoldPalette.lightPalette,
      ScaffoldDimens.defaultDimens,
    ],
  );

  /// Dark theme: scaffold default (dark) palette + default dimens.
  static final ThemeData dark = ThemeData(
    brightness: Brightness.dark,
    useMaterial3: true,
    extensions: const <ThemeExtension<dynamic>>[
      ScaffoldPalette.defaultPalette,
      ScaffoldDimens.defaultDimens,
    ],
  );
}
