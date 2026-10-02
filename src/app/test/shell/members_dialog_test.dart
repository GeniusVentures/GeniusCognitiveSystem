/// MembersDialog widget tests (plan 04-07 Task 3, D-04/D-07/D-08).
///
/// Drives the frozen [showMembersDialog] entry point against a recording
/// [GcsCommandTransport] double with an injected [MembersCubit] (no FFI): the
/// pushed roster renders its role badges and the "You" marker, an empty push
/// renders the UI-SPEC empty state, the "Invite members" CTA publishes an
/// invite for the scope, and a dormant member op renders its "Pending
/// removal" badge plus the Super Admin's approve affordance, whose tap
/// publishes the data-only approve command. Bounded-frame pumping throughout
/// -- never a sleep.
library;

import 'dart:async';

import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:frontend_scaffold/components/bottom_drawer/bottom_drawer.dart';
import 'package:frontend_scaffold/components/scaffold_pressable.dart';

import 'package:flutter_app/cubits/members_cubit.dart';
import 'package:flutter_app/cubits/session_cubit.dart';
import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';
import 'package:flutter_app/shell/members_dialog.dart';
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

/// Pumps the host app with the injected [MembersCubit] provider ABOVE the
/// `MaterialApp` so the modal drawer route (pushed onto the app's Navigator
/// overlay) still resolves it, then opens the member list through the frozen
/// entry point.
Future<void> _openMembersDialog(
  WidgetTester tester, {
  required MembersCubit members,
  required GcsCommandTransport transport,
  String scopeId = 'space-1',
  String scopeName = 'ops',
}) async {
  await tester.pumpWidget(
    BlocProvider<MembersCubit>.value(
      value: members,
      child: MaterialApp(
        theme: GcsTheme.light,
        home: const Scaffold(body: SizedBox()),
      ),
    ),
  );
  final BuildContext context = tester.state(find.byType(Scaffold)).context;
  unawaited(
    showMembersDialog(
      context: context,
      transport: transport,
      scopeId: scopeId,
      scopeName: scopeName,
    ),
  );
  // Complete the entrance transition so hit testing is stable.
  await tester.pump(const Duration(milliseconds: 300));
}

void main() {
  testWidgets('roster renders role badges and the You marker', (
    WidgetTester tester,
  ) async {
    final MembersCubit members = MembersCubit();
    addTearDown(members.close);
    members.setMembers('space-1', <MemberRecord>[
      MemberRecord()
        ..address = '0xcreator'
        ..role = Role.ROLE_SUPER_ADMIN,
      MemberRecord()
        ..address = '0xguest'
        ..role = Role.ROLE_GUEST,
    ], '0xguest');

    await _openMembersDialog(
      tester,
      members: members,
      transport: _RecordingTransport(),
    );

    expect(find.text('Super Admin'), findsOneWidget);
    expect(find.text('Guest'), findsOneWidget);
    expect(find.text('You'), findsOneWidget);
  });

  testWidgets('empty roster renders the no-members state', (
    WidgetTester tester,
  ) async {
    final MembersCubit members = MembersCubit();
    addTearDown(members.close);
    members.setMembers('space-1', <MemberRecord>[], '0xself');

    await _openMembersDialog(
      tester,
      members: members,
      transport: _RecordingTransport(),
    );

    expect(find.text('No members yet'), findsOneWidget);
    expect(
      find.text('Invite someone to this space to get started.'),
      findsOneWidget,
    );
  });

  testWidgets('Invite members publishes an invite for the scope', (
    WidgetTester tester,
  ) async {
    final MembersCubit members = MembersCubit();
    addTearDown(members.close);
    final _RecordingTransport transport = _RecordingTransport();
    members.setMembers('space-1', <MemberRecord>[
      MemberRecord()
        ..address = '0xself'
        ..role = Role.ROLE_SUPER_ADMIN,
    ], '0xself');

    await _openMembersDialog(tester, members: members, transport: transport);

    await tester.tap(find.text('Invite members'));
    await tester.pump();

    expect(transport.commands, hasLength(1));
    expect(transport.commands.single.hasInvite(), isTrue);
    expect(transport.commands.single.invite.scopeId, 'space-1');

    // Settle the invite-view drawer opened by the accepted publish so no
    // route transition outlives the test.
    await tester.pump(const Duration(milliseconds: 300));
  });

  testWidgets('pending row renders the Pending removal badge and approve', (
    WidgetTester tester,
  ) async {
    final MembersCubit members = MembersCubit();
    addTearDown(members.close);
    members.setMembers('space-1', <MemberRecord>[
      MemberRecord()
        ..address = '0xself'
        ..role = Role.ROLE_SUPER_ADMIN,
      MemberRecord()
        ..address = '0xbeef'
        ..role = Role.ROLE_MEMBER
        ..pendingAction = PendingAction.PENDING_ACTION_REMOVE,
    ], '0xself');

    await _openMembersDialog(
      tester,
      members: members,
      transport: _RecordingTransport(),
    );

    expect(find.text('Pending removal'), findsOneWidget);
    // The drawer's roster scroll node merges its descendants' semantics into
    // one label, so an exact `bySemanticsLabel` finder cannot isolate the
    // affordance; assert the pressable's accessible name (WCAG 4.1.2) directly.
    expect(
      find.byWidgetPredicate(
        (Widget widget) =>
            widget is ScaffoldPressable &&
            widget.semanticLabel == 'Approve removal of 0xbeef',
      ),
      findsOneWidget,
    );
  });

  testWidgets('approve publishes the approve command for the target', (
    WidgetTester tester,
  ) async {
    final MembersCubit members = MembersCubit();
    addTearDown(members.close);
    final _RecordingTransport transport = _RecordingTransport();
    members.setMembers('space-1', <MemberRecord>[
      MemberRecord()
        ..address = '0xself'
        ..role = Role.ROLE_SUPER_ADMIN,
      MemberRecord()
        ..address = '0xbeef'
        ..role = Role.ROLE_MEMBER
        ..pendingAction = PendingAction.PENDING_ACTION_REMOVE,
    ], '0xself');

    await _openMembersDialog(tester, members: members, transport: transport);

    await tester.tap(find.byIcon(Icons.check_circle_outline));
    await tester.pump();

    expect(transport.commands, hasLength(1));
    final GcsCommand command = transport.commands.single;
    expect(command.hasApprove(), isTrue);
    expect(command.approve.scopeId, 'space-1');
    expect(command.approve.target, '0xbeef');
    expect(find.byType(BottomDrawer), findsOneWidget);
  });
}
