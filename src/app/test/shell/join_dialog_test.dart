/// JoinDialog widget tests (plan 04-07 Task 3, MEMB-01/T-04-07-I).
///
/// Drives the frozen [showJoinDialog] entry point against a recording
/// [GcsCommandTransport] double (no FFI): a value that does not parse as the
/// `gcs://invite/{space|room}/<id>?key=<key>` shape shows the inline error
/// and keeps the dialog open with NO publish, while a valid link publishes a
/// data-only `RedeemCommand` carrying the pasted url and closes immediately
/// (Phase 2 D-05 behavior contract). Bounded-frame pumping throughout -- never
/// a sleep.
library;

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:frontend_scaffold/components/bottom_drawer/bottom_drawer.dart';
import 'package:frontend_scaffold/components/scaffold_pressable.dart';

import 'package:flutter_app/cubits/session_cubit.dart';
import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';
import 'package:flutter_app/shell/join_dialog.dart';
import 'package:flutter_app/theme/gcs_theme.dart';

/// Transport double recording published command envelopes (D-27 seam).
class _RecordingTransport implements GcsCommandTransport {
  /// Every command handed to the seam, in order.
  final List<GcsCommand> commands = <GcsCommand>[];

  @override
  bool publishCommand(GcsCommand command) {
    commands.add(command);
    return true;
  }
}

/// Pumps the host app and opens the join dialog through the frozen entry
/// point; afterwards the dialog content is driven with `find`/`tester`.
Future<void> _openJoinDialog(
  WidgetTester tester, {
  required GcsCommandTransport transport,
}) async {
  await tester.pumpWidget(
    MaterialApp(
      theme: GcsTheme.light,
      home: const Scaffold(body: SizedBox()),
    ),
  );
  final BuildContext context = tester.state(find.byType(Scaffold)).context;
  unawaited(showJoinDialog(context: context, transport: transport));
  // Complete the entrance transition so hit testing is stable.
  await tester.pump(const Duration(milliseconds: 300));
}

/// The accent "Join" pill (the drawer title is also "Join", so match the
/// pressable that carries the label).
Finder _joinPill() => find.widgetWithText(ScaffoldPressable, kJoinConfirmLabel);

void main() {
  testWidgets('invalid link shows the inline error and stays open', (
    WidgetTester tester,
  ) async {
    final _RecordingTransport transport = _RecordingTransport();
    await _openJoinDialog(tester, transport: transport);

    await tester.enterText(find.byType(TextFormField), 'not-an-invite-link');
    await tester.pump();
    await tester.tap(_joinPill());
    await tester.pump();

    expect(transport.commands, isEmpty);
    expect(find.text(kJoinInvalidLinkMessage), findsOneWidget);
    expect(find.byType(BottomDrawer), findsOneWidget);
  });

  testWidgets('valid link publishes the redeem command and closes', (
    WidgetTester tester,
  ) async {
    final _RecordingTransport transport = _RecordingTransport();
    await _openJoinDialog(tester, transport: transport);

    const String link = 'gcs://invite/space/space-1?key=abc';
    await tester.enterText(find.byType(TextFormField), link);
    await tester.pump();
    await tester.tap(_joinPill());
    await tester.pump(const Duration(milliseconds: 300));

    expect(transport.commands, hasLength(1));
    expect(transport.commands.single.hasRedeem(), isTrue);
    expect(transport.commands.single.redeem.inviteUrl, link);
    expect(find.byType(BottomDrawer), findsNothing);
  });
}
