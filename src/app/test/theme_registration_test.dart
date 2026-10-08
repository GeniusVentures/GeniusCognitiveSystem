/// Theme registration smoke test (plan 01-11 Task 4, D-12/D-22 — RESEARCH
/// success criteria row 864).
///
/// Proves the app registers the scaffold token system as ThemeExtensions per
/// brightness (light = `ScaffoldPalette.lightPalette`, dark = the scaffold
/// default/dark palette; both carry `ScaffoldDimens.defaultDimens`), that
/// `context.palette` / `context.dimens` resolve to those instances, and that
/// no hand-rolled `ColorScheme.fromSeed` remains in the theme sources.
library;

import 'dart:io' show File;

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';
import 'package:frontend_scaffold/theme/scaffold_dimens.dart';
import 'package:frontend_scaffold/theme/scaffold_palette.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import 'package:flutter_app/main.dart';
import 'package:flutter_app/theme/gcs_theme.dart';

/// Pumps a [MaterialApp] under [theme] and returns the palette/dimens the
/// scaffold extensions resolve for the child context.
Future<(ScaffoldPalette, ScaffoldDimens)> _resolveExtensions(
  WidgetTester tester,
  ThemeData theme,
) async {
  late (ScaffoldPalette, ScaffoldDimens) resolved;
  await tester.pumpWidget(
    MaterialApp(
      theme: theme,
      home: Builder(
        builder: (BuildContext context) {
          resolved = (context.palette, context.dimens);
          return const SizedBox.shrink();
        },
      ),
    ),
  );
  return resolved;
}

void main() {
  testWidgets('GCSChatApp mounts GcsTheme.light/dark in system mode', (
    WidgetTester tester,
  ) async {
    await tester.pumpWidget(const GCSChatApp());
    final MaterialApp app = tester.widget<MaterialApp>(
      find.byType(MaterialApp),
    );
    expect(app.theme, same(GcsTheme.light));
    expect(app.darkTheme, same(GcsTheme.dark));
    expect(app.themeMode, ThemeMode.system);
  });

  testWidgets('light theme resolves the light scaffold palette + dimens', (
    WidgetTester tester,
  ) async {
    final ThemeData theme = GcsTheme.light;
    expect(theme.useMaterial3, isTrue);
    expect(theme.brightness, Brightness.light);

    final (ScaffoldPalette palette, ScaffoldDimens dimens) =
        await _resolveExtensions(tester, theme);
    expect(palette, same(ScaffoldPalette.lightPalette));
    expect(dimens, same(ScaffoldDimens.defaultDimens));
  });

  testWidgets('dark theme resolves the default (dark) scaffold palette', (
    WidgetTester tester,
  ) async {
    final ThemeData theme = GcsTheme.dark;
    expect(theme.useMaterial3, isTrue);
    expect(theme.brightness, Brightness.dark);

    final (ScaffoldPalette palette, ScaffoldDimens dimens) =
        await _resolveExtensions(tester, theme);
    expect(palette, same(ScaffoldPalette.defaultPalette));
    expect(dimens, same(ScaffoldDimens.defaultDimens));
  });

  test('no ColorScheme.fromSeed seed color remains in the theme sources', () {
    const List<String> sources = <String>[
      'lib/main.dart',
      'lib/theme/gcs_theme.dart',
    ];
    for (final String source in sources) {
      expect(
        File(source).readAsStringSync(),
        isNot(contains('ColorScheme.fromSeed')),
        reason:
            '$source must theme via the scaffold palette, never a seed '
            'color (D-12)',
      );
    }
  });
}
