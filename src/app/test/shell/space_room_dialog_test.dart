/// SpaceRoomDialog widget tests (plan 02-04 Task 2, D-05).
///
/// Drives the frozen [showSpaceRoomDialog] entry point against a recording
/// [GcsCommandTransport] double (the shell_cubits_test fake pattern): each
/// mode publishes the correct GcsCommand oneof arm, the empty-name inline
/// validation blocks the publish, and a refused publish toasts while the
/// dialog stays open. The default 800x600 test surface takes
/// [ResponsiveDrawer] down its desktop dialog branch. Bounded-frame
/// `_pumpUntil` pumping throughout -- never a sleep.
library;

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:frontend_scaffold/components/bottom_drawer/bottom_drawer.dart';
import 'package:frontend_scaffold/components/scaffold_selection_indicator_radio.dart';
import 'package:frontend_scaffold/components/scaffold_selection_indicator_toggle.dart';

import 'package:flutter_app/cubits/rail_cubit.dart';
import 'package:flutter_app/cubits/session_cubit.dart';
import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';
import 'package:flutter_app/shell/space_room_dialog.dart';
import 'package:flutter_app/theme/gcs_theme.dart';

/// Transport double recording published command envelopes (D-27 seam);
/// [publishResult] drives the publish-false path.
class _RecordingTransport implements GcsCommandTransport {
  /// Creates a transport returning [publishResult] from every publish.
  _RecordingTransport({this.publishResult = true});

  /// Whether publishCommand accepts the command (false = transport down).
  final bool publishResult;

  /// Every command handed to the seam, in order.
  final List<GcsCommand> commands = <GcsCommand>[];

  @override
  bool publishCommand(GcsCommand command) {
    commands.add(command);
    return publishResult;
  }
}

/// Wait-condition template (never a sleep): pumps 16ms fake-clock frames
/// until [condition] holds, bounded by [maxFrames].
Future<void> _pumpUntil(
  WidgetTester tester,
  bool Function() condition, {
  int maxFrames = 20,
}) async {
  for (int i = 0; i < maxFrames && !condition(); i++) {
    await tester.pump(const Duration(milliseconds: 16));
  }
}

/// Pumps the host app and opens the dialog through the frozen entry
/// point; afterwards the dialog content is driven with `find`/`tester`.
Future<void> _openDialog(
  WidgetTester tester, {
  required GcsCommandTransport transport,
  required SpaceRoomDialogMode mode,
  RailSpace? space,
  RailSpace? parent,
}) async {
  await tester.pumpWidget(
    MaterialApp(
      theme: GcsTheme.light,
      home: const Scaffold(body: SizedBox()),
    ),
  );
  final BuildContext context = tester.state(find.byType(Scaffold)).context;
  unawaited(
    showSpaceRoomDialog(
      context: context,
      transport: transport,
      mode: mode,
      space: space,
      parent: parent,
    ),
  );
  // Complete the entrance transition so hit testing is stable.
  await tester.pump(const Duration(milliseconds: 300));
}

void main() {
  testWidgets('CreateSpacePublishesCreateSpace', (WidgetTester tester) async {
    final _RecordingTransport transport = _RecordingTransport();
    await _openDialog(
      tester,
      transport: transport,
      mode: SpaceRoomDialogMode.createFromHeader,
    );

    await tester.enterText(find.byType(TextFormField), 'Design');
    await tester.pump();
    await tester.tap(find.text('Private'));
    await tester.pump();
    await tester.tap(find.byType(ScaffoldSelectionIndicatorToggle));
    await tester.pump();
    await tester.tap(find.text('Create space'));

    expect(transport.commands, hasLength(1));
    final GcsCommand command = transport.commands.single;
    expect(command.hasCreateSpace(), isTrue);
    expect(command.createSpace.name, 'Design');
    expect(command.createSpace.isPublic, isFalse);
    expect(command.createSpace.autoJoinRooms, isTrue);
    // Confirm closes the drawer immediately (no local optimism).
    await tester.pump(const Duration(milliseconds: 300));
    expect(find.byType(BottomDrawer), findsNothing);
  });

  testWidgets('CreateStandaloneRoomPublishesEmptyParent', (
    WidgetTester tester,
  ) async {
    final _RecordingTransport transport = _RecordingTransport();
    await _openDialog(
      tester,
      transport: transport,
      mode: SpaceRoomDialogMode.createFromHeader,
    );

    await tester.tap(find.text('Standalone room'));
    await tester.pump();
    // Type-neutral title (IN-03): stays 'New' after the selector flips --
    // no stale 'New space' header over the room form.
    expect(find.text('New'), findsOneWidget);
    // Room type hides the space-config controls.
    expect(find.byType(ScaffoldSelectionIndicatorToggle), findsNothing);
    expect(find.text('Public'), findsNothing);
    await tester.enterText(find.byType(TextFormField), 'lounge');
    await tester.pump();
    await tester.tap(find.text('Create room'));

    expect(transport.commands, hasLength(1));
    final GcsCommand command = transport.commands.single;
    expect(command.hasCreateRoom(), isTrue);
    expect(command.createRoom.name, 'lounge');
    expect(command.createRoom.parentSpaceId, isEmpty);
  });

  testWidgets('CreateRoomInSpacePublishesParentId', (
    WidgetTester tester,
  ) async {
    final _RecordingTransport transport = _RecordingTransport();
    await _openDialog(
      tester,
      transport: transport,
      mode: SpaceRoomDialogMode.createRoomInSpace,
      parent: const RailSpace(
        id: 'space-abc',
        name: 'ops',
        isPublic: true,
        autoJoinRooms: false,
      ),
    );

    // Fixed room-in-space mode: no type selector, no space-config controls.
    expect(find.text('New room in ops'), findsOneWidget);
    expect(find.text('Space'), findsNothing);
    expect(find.text('Standalone room'), findsNothing);
    expect(find.byType(ScaffoldSelectionIndicatorToggle), findsNothing);
    await tester.enterText(find.byType(TextFormField), 'general');
    await tester.pump();
    await tester.tap(find.text('Create room'));

    expect(transport.commands, hasLength(1));
    final GcsCommand command = transport.commands.single;
    expect(command.hasCreateRoom(), isTrue);
    expect(command.createRoom.name, 'general');
    expect(command.createRoom.parentSpaceId, 'space-abc');
  });

  testWidgets('EditSpacePublishesUpdateSpace', (WidgetTester tester) async {
    final _RecordingTransport transport = _RecordingTransport();
    await _openDialog(
      tester,
      transport: transport,
      mode: SpaceRoomDialogMode.editSpace,
      space: const RailSpace(
        id: 'space-xyz',
        name: 'ops',
        isPublic: false,
        autoJoinRooms: true,
      ),
    );

    expect(find.text('Edit space'), findsOneWidget);
    expect(find.text('Space'), findsNothing); // no type selector in edit mode
    // Prefills: Private radio checked, autoJoin toggle on.
    final ScaffoldSelectionIndicatorRadio privateRadio = tester
        .widget<ScaffoldSelectionIndicatorRadio>(
          find.byType(ScaffoldSelectionIndicatorRadio).at(1),
        );
    expect(privateRadio.value, isTrue);
    final ScaffoldSelectionIndicatorToggle toggle = tester
        .widget<ScaffoldSelectionIndicatorToggle>(
          find.byType(ScaffoldSelectionIndicatorToggle),
        );
    expect(toggle.value, isTrue);
    // Prefilled name enables confirm without typing.
    await tester.tap(find.text('Save changes'));

    expect(transport.commands, hasLength(1));
    final GcsCommand command = transport.commands.single;
    expect(command.hasUpdateSpace(), isTrue);
    expect(command.updateSpace.spaceId, 'space-xyz');
    expect(command.updateSpace.name, 'ops');
    expect(command.updateSpace.isPublic, isFalse);
    expect(command.updateSpace.autoJoinRooms, isTrue);
  });

  testWidgets('EmptyNameDoesNotPublish', (WidgetTester tester) async {
    final _RecordingTransport transport = _RecordingTransport();
    await _openDialog(
      tester,
      transport: transport,
      mode: SpaceRoomDialogMode.createFromHeader,
    );

    // Whitespace-only name: the confirm pill stays active (any input
    // enables it) but the trimmed name is empty, so confirm hits the
    // inline validation instead of the publish.
    await tester.enterText(find.byType(TextFormField), '   ');
    await tester.pump();
    await tester.tap(find.text('Create space'));
    await tester.pump();

    expect(transport.commands, isEmpty);
    expect(find.text('Enter a name.'), findsOneWidget);
    expect(find.byType(BottomDrawer), findsOneWidget); // dialog stays open
  });

  testWidgets('PublishFalseShowsToast', (WidgetTester tester) async {
    final _RecordingTransport transport = _RecordingTransport(
      publishResult: false,
    );
    await _openDialog(
      tester,
      transport: transport,
      mode: SpaceRoomDialogMode.createFromHeader,
    );

    await tester.enterText(find.byType(TextFormField), 'Design');
    await tester.pump();
    await tester.tap(find.text('Create space'));
    await _pumpUntil(
      tester,
      () => find.text(kPublishFailedMessage).evaluate().isNotEmpty,
    );

    expect(find.text(kPublishFailedMessage), findsOneWidget);
    expect(find.text("Couldn't create space"), findsOneWidget);
    expect(find.byType(BottomDrawer), findsOneWidget); // dialog stays open
    // Burn the toast card's auto-dismiss so no timer outlives the tree.
    await tester.pump(const Duration(seconds: 5));
    await tester.pump(const Duration(milliseconds: 300));
  });

  testWidgets('DoubleConfirmPublishesExactlyOnce', (WidgetTester tester) async {
    final _RecordingTransport transport = _RecordingTransport();
    await _openDialog(
      tester,
      transport: transport,
      mode: SpaceRoomDialogMode.createFromHeader,
    );

    await tester.enterText(find.byType(TextFormField), 'Design');
    await tester.pump();
    // Re-entrancy (WR-04): a fast Enter-then-tap fires submit twice before
    // the route finishes popping -- exactly one command may publish.
    await tester.tap(find.text('Create space'));
    await tester.tap(find.text('Create space'));
    await tester.pump(const Duration(milliseconds: 300));

    expect(transport.commands, hasLength(1));
    expect(find.byType(BottomDrawer), findsNothing);
  });

  testWidgets('ModeParamsAssertOnMisuse', (WidgetTester tester) async {
    // IN-04: omitting the mode-required param fails loudly at the call site
    // (debug assert) instead of crashing later in dialogTitle/_buildCommand.
    await tester.pumpWidget(
      MaterialApp(
        theme: GcsTheme.light,
        home: const Scaffold(body: SizedBox()),
      ),
    );
    final BuildContext context = tester.state(find.byType(Scaffold)).context;
    expect(
      () => showSpaceRoomDialog(
        context: context,
        transport: _RecordingTransport(),
        mode: SpaceRoomDialogMode.createRoomInSpace,
      ),
      throwsA(isA<AssertionError>()),
    );
    expect(
      () => showSpaceRoomDialog(
        context: context,
        transport: _RecordingTransport(),
        mode: SpaceRoomDialogMode.editSpace,
      ),
      throwsA(isA<AssertionError>()),
    );
  });
}
