/// Generated chat composites smoke test (plan 01-11 Task 4, D-17/D-18/D-19/
/// D-20 — RESEARCH success criteria row 866).
///
/// Drives the generated (never hand-edited) composites straight from the
/// generator fixture: renders every role × state bubble variant from
/// `templates/components/chat_message_bubble_vars.json` (the same vars axis
/// the codegen consumed), then renders an interleaved `ChatMessageFlow`
/// (text/code/media/text) through the sealed `ChatFlowItem` subclasses.
/// Render + find assertions only (goldens optional per plan).
library;

import 'dart:convert' show jsonDecode;
import 'dart:io' show File;

import 'package:flutter/material.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:flutter_app/generated/chat/chat_message_bubble.dart';
import 'package:flutter_app/generated/chat/chat_message_bubble_cubit.dart';
import 'package:flutter_app/generated/chat/chat_message_bubble_state.dart';
import 'package:flutter_app/generated/chat/chat_message_code_block.dart';
import 'package:flutter_app/generated/chat/chat_message_flow.dart';
import 'package:flutter_app/generated/chat/chat_message_media.dart';
import 'package:flutter_app/theme/gcs_theme.dart';

/// Role axis -> generated bubble widget class (mirrors the flow's D-18
/// registry; keeps this test table-driven off the fixture).
const Map<String, Widget Function({ChatMessageBubbleCubit? cubit})>
_roleWidgets = <String, Widget Function({ChatMessageBubbleCubit? cubit})>{
  'user_self': ChatMessageBubbleUserSelf.new,
  'user_peer': ChatMessageBubbleUserPeer.new,
  'assistant': ChatMessageBubbleAssistant.new,
  'system': ChatMessageBubbleSystem.new,
};

void main() {
  final Map<String, dynamic> fixture =
      jsonDecode(
            File(
              'templates/components/chat_message_bubble_vars.json',
            ).readAsStringSync(),
          )
          as Map<String, dynamic>;
  final List<String> fixtureRoles = (fixture['roles'] as List<dynamic>)
      .cast<String>();
  final List<String> fixtureStates = (fixture['states'] as List<dynamic>)
      .cast<String>();

  test('generated role/state axes match the generator fixture verbatim', () {
    expect(ChatMessageBubbleState.kValidRoles, fixtureRoles);
    expect(ChatMessageBubbleState.kValidStates, fixtureStates);
  });

  testWidgets('every role x state bubble variant renders (D-18 x D-19)', (
    WidgetTester tester,
  ) async {
    const String sampleText = 'fixture-driven bubble payload';
    for (final String role in fixtureRoles) {
      for (final String state in fixtureStates) {
        final ChatMessageBubbleCubit cubit = ChatMessageBubbleCubit(
          role: role,
          initialMessageState: state,
          initialText: sampleText,
        );
        addTearDown(cubit.close);

        await tester.pumpWidget(
          MaterialApp(
            theme: GcsTheme.light,
            home: Scaffold(
              body: Center(child: _roleWidgets[role]!(cubit: cubit)),
            ),
          ),
        );

        expect(
          find.byWidgetPredicate(
            (Widget widget) =>
                widget.runtimeType.toString() ==
                'ChatMessageBubble${_pascalRole(role)}',
          ),
          findsOneWidget,
          reason: '$role/$state variant must render',
        );
        if (state != 'thinking') {
          // Thinking chrome replaces the text with a dots indicator; every
          // other state keeps the payload visible.
          expect(find.text(sampleText), findsOneWidget);
        }
      }
    }
  });

  testWidgets(
    'interleaved flow renders text, code and media items (D-17/D-20)',
    (WidgetTester tester) async {
      tester.view.physicalSize = const Size(1200, 2400);
      tester.view.devicePixelRatio = 1.0;
      addTearDown(tester.view.reset);

      const List<ChatFlowItem> items = <ChatFlowItem>[
        ChatFlowItemTextBubble(
          instanceId: 't1',
          role: 'user_self',
          text: 'first pushed line',
        ),
        ChatFlowItemCodeBlock(
          instanceId: 'c1',
          code: 'final answer = 42;',
          language: 'dart',
          filename: 'answer.dart',
        ),
        ChatFlowItemMedia(
          instanceId: 'm1',
          mediaRef: 'gcs://media/diagram',
          title: 'architecture diagram',
        ),
        ChatFlowItemTextBubble(
          instanceId: 't2',
          role: 'assistant',
          text: 'second pushed line',
        ),
      ];

      await tester.pumpWidget(
        MaterialApp(
          theme: GcsTheme.light,
          home: const Scaffold(body: ChatMessageFlow(items: items)),
        ),
      );

      expect(find.byType(ChatMessageCodeBlock), findsOneWidget);
      expect(find.byType(ChatMessageMedia), findsOneWidget);
      expect(find.byType(ChatMessageBubbleUserSelf), findsOneWidget);
      expect(find.byType(ChatMessageBubbleAssistant), findsOneWidget);
      expect(find.text('first pushed line'), findsOneWidget);
      expect(find.text('second pushed line'), findsOneWidget);
      expect(find.text('final answer = 42;'), findsOneWidget);
      expect(find.text('architecture diagram'), findsOneWidget);
    },
  );

  testWidgets(
    'replacement snapshot with a reused instanceId reconciles post-build (IN-06)',
    (WidgetTester tester) async {
      tester.view.physicalSize = const Size(1200, 2400);
      tester.view.devicePixelRatio = 1.0;
      addTearDown(tester.view.reset);

      Widget flowFor(String text) => MaterialApp(
        theme: GcsTheme.light,
        home: Scaffold(
          body: ChatMessageFlow(
            items: <ChatFlowItem>[
              ChatFlowItemTextBubble(
                instanceId: 'same-id',
                role: 'user_self',
                text: text,
              ),
            ],
          ),
        ),
      );

      await tester.pumpWidget(flowFor('snapshot-v1'));
      expect(find.text('snapshot-v1'), findsOneWidget);

      // A replacement snapshot (same instanceId, newer text) must reconcile
      // the flow-owned cached cubit — via didUpdateWidget, never inside the
      // sliver builder (emit-during-build hazard, review IN-06).
      await tester.pumpWidget(flowFor('snapshot-v2'));
      await tester.pump();
      expect(find.text('snapshot-v2'), findsOneWidget);
      expect(find.text('snapshot-v1'), findsNothing);
    },
  );
}

/// 'user_self' -> 'UserSelf' (for the generated class-name assertion).
String _pascalRole(String role) {
  return role
      .split('_')
      .map(
        (String part) => part.isEmpty
            ? part
            : '${part[0].toUpperCase()}${part.substring(1)}',
      )
      .join();
}
