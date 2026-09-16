/// Chat shell smoke test (plan 01-11 Task 4, D-11/D-21/D-23 — RESEARCH
/// success criteria row 865).
///
/// Pumps the real [GCSChat] three-region shell with cubits injected (no FFI:
/// the session cubit is inert, readiness toggled through a test subclass).
/// Proves: rail + flow + composer render; the pushed room list renders rail
/// rows; rail selection projects onto the composer (active-room hint); the
/// composer stays disabled until session readiness; appended flow items
/// render; the send affordance publishes a send_text envelope for the active
/// room through the transport seam.
///
/// Cubits are constructed INSIDE each test body (never `setUp`): cubit
/// streams are async broadcast controllers, and only cubits created in the
/// test's own zone have their delivery microtasks flushed by
/// `tester.pump`. Async propagation uses the wait-condition template
/// (`_pumpUntil` bounded frame pumping), never sleeps.
library;

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:flutter_app/cubits/composer_cubit.dart';
import 'package:flutter_app/cubits/message_flow_cubit.dart';
import 'package:flutter_app/cubits/rail_cubit.dart';
import 'package:flutter_app/cubits/session_cubit.dart';
import 'package:flutter_app/generated/chat/chat_message_flow.dart';
import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';
import 'package:flutter_app/shell/gcs_shell.dart';
import 'package:flutter_app/shell/room_rail.dart';
import 'package:flutter_app/theme/gcs_theme.dart';

/// Inert [SessionCubit] the test can flip to ready (no bindings = no FFI).
class _ReadyableSessionCubit extends SessionCubit {
  /// Creates an inert session cubit with dispatch targets wired.
  _ReadyableSessionCubit({super.railCubit, super.messageFlowCubit});

  /// Forces the readiness flag the C++ side normally pushes.
  void setReady(bool ready) {
    emit(state.copyWith(isReady: ready));
  }
}

/// Transport double capturing published command envelopes (D-27 seam).
class _RecordingTransport implements GcsCommandTransport {
  final List<GcsCommand> commands = <GcsCommand>[];

  @override
  bool publishCommand(GcsCommand command) {
    commands.add(command);
    return true;
  }
}

/// Wait-condition template (never a sleep): pumps 16ms fake-clock frames
/// until [condition] holds, bounded by [maxFrames]. Covers the chained
/// async stream hops (cubits emit through an async broadcast controller, so
/// rail -> composer -> widget rebuild can span multiple event-loop turns).
Future<void> _pumpUntil(
  WidgetTester tester,
  bool Function() condition, {
  int maxFrames = 20,
}) async {
  for (int i = 0; i < maxFrames && !condition(); i++) {
    await tester.pump(const Duration(milliseconds: 16));
  }
}

void main() {
  testWidgets('shell renders rail + flow + composer', (
    WidgetTester tester,
  ) async {
    final RailCubit rail = RailCubit();
    final MessageFlowCubit flow = MessageFlowCubit();
    final _ReadyableSessionCubit session = _ReadyableSessionCubit(
      railCubit: rail,
      messageFlowCubit: flow,
    );
    final _RecordingTransport transport = _RecordingTransport();
    final ComposerCubit composer = ComposerCubit(
      transport: transport,
      railCubit: rail,
    );
    addTearDown(composer.close);
    addTearDown(session.close);
    addTearDown(flow.close);
    addTearDown(rail.close);

    await tester.pumpWidget(
      MaterialApp(
        theme: GcsTheme.light,
        home: GCSChat(
          sessionCubit: session,
          railCubit: rail,
          messageFlowCubit: flow,
          composerCubit: composer,
        ),
      ),
    );

    expect(find.byType(RoomRail), findsOneWidget);
    expect(find.byType(ChatMessageFlow), findsOneWidget);
    expect(find.byType(GCSChat), findsOneWidget);
    // Empty rail renders the scaffold empty state, not a hardcoded list.
    expect(find.text('No rooms yet'), findsOneWidget);
  });

  testWidgets('pushed room list renders rail rows (D-21)', (
    WidgetTester tester,
  ) async {
    final RailCubit rail = RailCubit();
    final MessageFlowCubit flow = MessageFlowCubit();
    final _ReadyableSessionCubit session = _ReadyableSessionCubit(
      railCubit: rail,
      messageFlowCubit: flow,
    );
    final ComposerCubit composer = ComposerCubit(
      transport: _RecordingTransport(),
      railCubit: rail,
    );
    addTearDown(composer.close);
    addTearDown(session.close);
    addTearDown(flow.close);
    addTearDown(rail.close);

    await tester.pumpWidget(
      MaterialApp(
        theme: GcsTheme.light,
        home: GCSChat(
          sessionCubit: session,
          railCubit: rail,
          messageFlowCubit: flow,
          composerCubit: composer,
        ),
      ),
    );

    rail.setRooms(<String>['gcs/chat/smoke-test', 'gcs/chat/smoke-test-2']);
    await tester.pump();

    expect(find.text('smoke-test'), findsOneWidget);
    expect(find.text('smoke-test-2'), findsOneWidget);
  });

  testWidgets('rail selection projects onto the composer active room', (
    WidgetTester tester,
  ) async {
    final RailCubit rail = RailCubit();
    final MessageFlowCubit flow = MessageFlowCubit();
    final _ReadyableSessionCubit session = _ReadyableSessionCubit(
      railCubit: rail,
      messageFlowCubit: flow,
    );
    final _RecordingTransport transport = _RecordingTransport();
    final ComposerCubit composer = ComposerCubit(
      transport: transport,
      railCubit: rail,
    );
    addTearDown(composer.close);
    addTearDown(session.close);
    addTearDown(flow.close);
    addTearDown(rail.close);

    await tester.pumpWidget(
      MaterialApp(
        theme: GcsTheme.light,
        home: GCSChat(
          sessionCubit: session,
          railCubit: rail,
          messageFlowCubit: flow,
          composerCubit: composer,
        ),
      ),
    );

    rail.setRooms(<String>['gcs/chat/smoke-test', 'gcs/chat/smoke-test-2']);
    await tester.pump();

    expect(find.text('Select a room to start messaging'), findsOneWidget);
    await tester.tap(find.text('smoke-test-2'));
    await _pumpUntil(tester, () => composer.state.activeRoom != null);

    expect(rail.state.activeRoom, 'gcs/chat/smoke-test-2');
    expect(composer.state.activeRoom, 'gcs/chat/smoke-test-2');
    await _pumpUntil(
      tester,
      () => find.text('Message #smoke-test-2').evaluate().isNotEmpty,
    );
    expect(find.text('Message #smoke-test-2'), findsOneWidget);
  });

  testWidgets('composer is disabled until the session readiness flag', (
    WidgetTester tester,
  ) async {
    final RailCubit rail = RailCubit();
    final MessageFlowCubit flow = MessageFlowCubit();
    final _ReadyableSessionCubit session = _ReadyableSessionCubit(
      railCubit: rail,
      messageFlowCubit: flow,
    );
    final ComposerCubit composer = ComposerCubit(
      transport: _RecordingTransport(),
      railCubit: rail,
    );
    addTearDown(composer.close);
    addTearDown(session.close);
    addTearDown(flow.close);
    addTearDown(rail.close);

    await tester.pumpWidget(
      MaterialApp(
        theme: GcsTheme.light,
        home: GCSChat(
          sessionCubit: session,
          railCubit: rail,
          messageFlowCubit: flow,
          composerCubit: composer,
        ),
      ),
    );

    rail.setRooms(<String>['gcs/chat/smoke-test']);
    await tester.pump();
    await tester.tap(find.text('smoke-test'));
    await tester.pump();

    TextField textField() => tester.widget<TextField>(find.byType(TextField));
    expect(textField().enabled, isFalse, reason: 'not ready -> disabled');

    session.setReady(true);
    await _pumpUntil(tester, () => textField().enabled == true);
    expect(textField().enabled, isTrue);
  });

  testWidgets('appended flow items render in the center pane', (
    WidgetTester tester,
  ) async {
    final RailCubit rail = RailCubit();
    final MessageFlowCubit flow = MessageFlowCubit();
    final _ReadyableSessionCubit session = _ReadyableSessionCubit(
      railCubit: rail,
      messageFlowCubit: flow,
    );
    final ComposerCubit composer = ComposerCubit(
      transport: _RecordingTransport(),
      railCubit: rail,
    );
    addTearDown(composer.close);
    addTearDown(session.close);
    addTearDown(flow.close);
    addTearDown(rail.close);

    await tester.pumpWidget(
      MaterialApp(
        theme: GcsTheme.light,
        home: GCSChat(
          sessionCubit: session,
          railCubit: rail,
          messageFlowCubit: flow,
          composerCubit: composer,
        ),
      ),
    );

    flow.append(
      const ChatFlowItemTextBubble(
        instanceId: 'm1',
        role: 'user_self',
        text: 'pushed echo renders',
      ),
    );
    await _pumpUntil(
      tester,
      () => find.text('pushed echo renders').evaluate().isNotEmpty,
    );

    expect(find.text('pushed echo renders'), findsOneWidget);
  });

  testWidgets('send affordance publishes send_text for the active room', (
    WidgetTester tester,
  ) async {
    final RailCubit rail = RailCubit();
    final MessageFlowCubit flow = MessageFlowCubit();
    final _ReadyableSessionCubit session = _ReadyableSessionCubit(
      railCubit: rail,
      messageFlowCubit: flow,
    );
    final _RecordingTransport transport = _RecordingTransport();
    final ComposerCubit composer = ComposerCubit(
      transport: transport,
      railCubit: rail,
    );
    addTearDown(composer.close);
    addTearDown(session.close);
    addTearDown(flow.close);
    addTearDown(rail.close);

    await tester.pumpWidget(
      MaterialApp(
        theme: GcsTheme.light,
        home: GCSChat(
          sessionCubit: session,
          railCubit: rail,
          messageFlowCubit: flow,
          composerCubit: composer,
        ),
      ),
    );

    rail.setRooms(<String>['gcs/chat/smoke-test', 'gcs/chat/smoke-test-2']);
    await tester.pump();
    await tester.tap(find.text('smoke-test-2'));
    await _pumpUntil(tester, () => composer.state.activeRoom != null);
    session.setReady(true);
    composer.updateDraft('hello from the shell');
    await _pumpUntil(
      tester,
      () => tester.widget<TextField>(find.byType(TextField)).enabled == true,
    );

    await tester.tap(find.byIcon(Icons.send));
    await tester.pump();

    expect(transport.commands, hasLength(1));
    expect(
      transport.commands.single.whichPayload(),
      GcsCommand_Payload.sendText,
    );
    expect(
      transport.commands.single.sendText.roomTopic,
      'gcs/chat/smoke-test-2',
    );
    expect(transport.commands.single.sendText.text, 'hello from the shell');
  });
}
