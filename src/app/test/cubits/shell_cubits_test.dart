/// Shell cubit tests (plan 01-11 Task 2, TDD RED-first).
///
/// Pure Dart: no widgets, no live FFI. [SessionCubit]'s native lifecycle is
/// exercised against a recording [GcsBindings] fake; pushed-event decode is
/// driven through `handlePushedBytes` with serialized `GcsEvent` payloads --
/// the same bytes the NativePort delivers in production (D-05 push, never
/// polling). Async propagation uses the wait-condition pattern
/// (`pumpEventQueue`), never sleeps.
library;

// The gcs_* override names mirror the C symbols (ffigen naming), same as
// gcs_bindings_generated.dart.
// ignore_for_file: non_constant_identifier_names

import 'dart:ffi' as ffi;
import 'dart:typed_data';

import 'package:ffi/ffi.dart' as pkg_ffi;
import 'package:flutter_test/flutter_test.dart';

import 'package:flutter_app/cubits/composer_cubit.dart';
import 'package:flutter_app/cubits/message_flow_cubit.dart';
import 'package:flutter_app/cubits/rail_cubit.dart';
import 'package:flutter_app/cubits/session_cubit.dart';
import 'package:flutter_app/gcs_bindings_generated.dart';
import 'package:flutter_app/generated/chat/chat_message_flow.dart';
import 'package:flutter_app/generated/chat/chat_message_flow_state.dart';
import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';

/// Records every gcs_* call and captures the exact bytes crossing the ABI
/// (config on init, envelope on publish, topic strings on subscribe/publish).
class _RecordingBindings extends GcsBindings {
  /// Creates a fake whose gcs_init returns [initResult] (default: a null
  /// handle, i.e. init failure).
  _RecordingBindings({ffi.Pointer<GcsSession>? initResult})
    : initResult = initResult ?? ffi.Pointer<GcsSession>.fromAddress(0),
      super.fromLookup(_noSymbols);

  static ffi.Pointer<T> _noSymbols<T extends ffi.NativeType>(
    String symbolName,
  ) => throw UnsupportedError('fake bindings resolve no symbols');

  /// Handle returned by gcs_init.
  final ffi.Pointer<GcsSession> initResult;

  /// Number of gcs_init/gcs_subscribe/gcs_publish/gcs_shutdown calls.
  int initCalls = 0;
  int subscribeCalls = 0;
  int publishCalls = 0;
  int shutdownCalls = 0;

  /// Decoded GcsConfig captured from the last gcs_init call.
  GcsConfig? lastConfig;

  /// Topic string captured from the last gcs_subscribe call.
  String? lastSubscribeTopic;

  /// NativePort id captured from the last gcs_subscribe call.
  int lastDartPort = 0;

  /// Topic string captured from the last gcs_publish call.
  String? lastPublishTopic;

  /// Decoded envelope captured from the last gcs_publish call.
  GcsCommand? lastPublishedCommand;

  @override
  ffi.Pointer<GcsSession> gcs_init(
    ffi.Pointer<ffi.Uint8> configBytes,
    int configLength,
  ) {
    initCalls++;
    lastConfig = GcsConfig.fromBuffer(configBytes.asTypedList(configLength));
    return initResult;
  }

  @override
  int gcs_subscribe(
    ffi.Pointer<GcsSession> session,
    ffi.Pointer<ffi.Char> topic,
    int dartPort,
  ) {
    subscribeCalls++;
    lastSubscribeTopic = topic.cast<pkg_ffi.Utf8>().toDartString();
    lastDartPort = dartPort;
    return GcsStatus.GCS_OK;
  }

  @override
  int gcs_publish(
    ffi.Pointer<GcsSession> session,
    ffi.Pointer<ffi.Char> topic,
    ffi.Pointer<ffi.Uint8> payloadBytes,
    int payloadLength,
  ) {
    publishCalls++;
    lastPublishTopic = topic.cast<pkg_ffi.Utf8>().toDartString();
    lastPublishedCommand = GcsCommand.fromBuffer(
      payloadBytes.asTypedList(payloadLength),
    );
    return GcsStatus.GCS_OK;
  }

  @override
  void gcs_shutdown(ffi.Pointer<GcsSession> session) {
    shutdownCalls++;
  }
}

/// Transport double recording published command envelopes (D-27 seam).
class _RecordingTransport implements GcsCommandTransport {
  final List<GcsCommand> commands = <GcsCommand>[];

  @override
  bool publishCommand(GcsCommand command) {
    commands.add(command);
    return true;
  }
}

void main() {
  group('RailCubit', () {
    test('starts empty with no selection and no hardcoded rooms', () {
      final RailCubit cubit = RailCubit();
      addTearDown(cubit.close);
      expect(cubit.state.rooms, isEmpty);
      expect(cubit.state.activeRoom, isNull);
    });

    test('setRooms REPLACES the list (pushed RoomList is the truth)', () {
      final RailCubit cubit = RailCubit();
      addTearDown(cubit.close);
      cubit.setRooms(<String>['gcs/chat/a', 'gcs/chat/b']);
      expect(cubit.state.rooms, <String>['gcs/chat/a', 'gcs/chat/b']);
      cubit.setRooms(<String>['gcs/chat/c']);
      expect(cubit.state.rooms, <String>['gcs/chat/c']);
    });

    test('selectRoom sets the active room; stale selection clears', () {
      final RailCubit cubit = RailCubit();
      addTearDown(cubit.close);
      cubit.setRooms(<String>['gcs/chat/a', 'gcs/chat/b']);
      cubit.selectRoom('gcs/chat/a');
      expect(cubit.state.activeRoom, 'gcs/chat/a');
      cubit.setRooms(<String>['gcs/chat/b']);
      expect(
        cubit.state.activeRoom,
        isNull,
        reason: 'selection must not dangle',
      );
      cubit.selectRoom('gcs/chat/b');
      expect(cubit.state.activeRoom, 'gcs/chat/b');
    });
  });

  group('MessageFlowCubit', () {
    test('append adds items and never mutates existing entries', () {
      final MessageFlowCubit cubit = MessageFlowCubit();
      addTearDown(cubit.close);
      const ChatFlowItemTextBubble first = ChatFlowItemTextBubble(
        instanceId: 'm1',
        role: 'user_self',
        text: 'one',
      );
      const ChatFlowItemTextBubble second = ChatFlowItemTextBubble(
        instanceId: 'm2',
        role: 'assistant',
        text: 'two',
      );
      cubit.append(first);
      cubit.append(second);
      expect(cubit.state, hasLength(2));
      expect(cubit.state.first, same(first));
      expect(cubit.state[1], same(second));
    });

    test('caps the flow list dropping the oldest (T-01-11-03)', () {
      final MessageFlowCubit cubit = MessageFlowCubit();
      addTearDown(cubit.close);
      const int cap = ChatMessageFlowState.kMaxFlowItems;
      for (int i = 0; i < cap + 2; i++) {
        cubit.append(
          ChatFlowItemTextBubble(
            instanceId: 'm$i',
            role: 'user_self',
            text: 't$i',
          ),
        );
      }
      expect(cubit.state, hasLength(cap));
      expect(
        cubit.state.first.instanceId,
        'm2',
        reason: 'oldest dropped first',
      );
      expect(cubit.state.last.instanceId, 'm${cap + 1}');
    });

    test('buildChatFlowItemTextBubble maps the proto taxonomy to axes', () {
      final MessageFlowCubit cubit = MessageFlowCubit();
      addTearDown(cubit.close);
      final ChatMessageState message = ChatMessageState()
        ..id = 'id-1'
        ..role = MessageRole.MESSAGE_ROLE_USER_PEER
        ..state = MessageState.MESSAGE_STATE_PENDING
        ..text = 'hello';
      final ChatFlowItemTextBubble item = cubit.buildChatFlowItemTextBubble(
        message,
      );
      expect(item.instanceId, 'id-1');
      expect(item.role, 'user_peer');
      expect(item.state, 'pending');
      expect(item.text, 'hello');
    });
  });

  group('ComposerCubit', () {
    test(
      'send publishes send_text for the active room and clears the draft',
      () async {
        final _RecordingTransport transport = _RecordingTransport();
        final RailCubit rail = RailCubit();
        final ComposerCubit cubit = ComposerCubit(
          transport: transport,
          railCubit: rail,
        );
        addTearDown(cubit.close);
        addTearDown(rail.close);
        rail.setRooms(<String>['gcs/chat/room-one']);
        rail.selectRoom('gcs/chat/room-one');
        await pumpEventQueue();
        expect(cubit.state.activeRoom, 'gcs/chat/room-one');
        cubit.updateDraft('hello world');
        cubit.send();
        expect(transport.commands, hasLength(1));
        expect(
          transport.commands.single.whichPayload(),
          GcsCommand_Payload.sendText,
        );
        expect(
          transport.commands.single.sendText.roomTopic,
          'gcs/chat/room-one',
        );
        expect(transport.commands.single.sendText.text, 'hello world');
        expect(cubit.state.draft, isEmpty);
      },
    );

    test('send is a no-op without a sendable draft or room selection', () {
      final _RecordingTransport transport = _RecordingTransport();
      final ComposerCubit cubit = ComposerCubit(transport: transport);
      addTearDown(cubit.close);
      cubit.updateDraft('   ');
      cubit.send();
      expect(transport.commands, isEmpty);
      expect(cubit.state.draft, '   ');
    });
  });

  group('SessionCubit', () {
    test(
      'inert mode (no bindings): start is a no-op, publish refuses',
      () async {
        final RailCubit rail = RailCubit();
        final MessageFlowCubit flow = MessageFlowCubit();
        final SessionCubit cubit = SessionCubit(
          railCubit: rail,
          messageFlowCubit: flow,
        );
        addTearDown(rail.close);
        addTearDown(flow.close);
        cubit.start();
        expect(cubit.state.isHandleOpen, isFalse);
        expect(cubit.state.isReady, isFalse);
        expect(cubit.publishCommand(GcsCommand()), isFalse);
        await cubit.close();
      },
    );

    test(
      'start: protobuf config bytes init, event-topic port subscribe',
      () async {
        final ffi.Pointer<GcsSession> fakeHandle =
            ffi.Pointer<GcsSession>.fromAddress(64);
        final _RecordingBindings bindings = _RecordingBindings(
          initResult: fakeHandle,
        );
        final SessionCubit cubit = SessionCubit(
          bindings: bindings,
          dbPath: '/tmp/gcs-cubit-test-db',
        );
        addTearDown(cubit.close);
        cubit.start();
        expect(bindings.initCalls, 1);
        expect(bindings.lastConfig!.codec, Codec.CODEC_PROTOBUF);
        expect(bindings.lastConfig!.dbPath, '/tmp/gcs-cubit-test-db');
        expect(bindings.subscribeCalls, 1);
        expect(bindings.lastSubscribeTopic, 'gcs/event');
        expect(bindings.lastDartPort, greaterThan(0));
        expect(cubit.state.isHandleOpen, isTrue);
        expect(cubit.handle.address, 64);
      },
    );

    test('init failure surfaces a raw error and never subscribes', () {
      final _RecordingBindings bindings = _RecordingBindings();
      final SessionCubit cubit = SessionCubit(bindings: bindings);
      addTearDown(cubit.close);
      cubit.start();
      expect(cubit.state.isHandleOpen, isFalse);
      expect(cubit.state.error, isNotNull);
      expect(bindings.subscribeCalls, 0);
    });

    test(
      'close tears the native session down exactly once (idempotent)',
      () async {
        final _RecordingBindings bindings = _RecordingBindings(
          initResult: ffi.Pointer<GcsSession>.fromAddress(64),
        );
        final SessionCubit cubit = SessionCubit(bindings: bindings);
        cubit.start();
        await cubit.close();
        await cubit.close();
        expect(bindings.shutdownCalls, 1);
      },
    );

    test('handlePushedBytes decodes and dispatches every GcsEvent variant', () {
      final RailCubit rail = RailCubit();
      final MessageFlowCubit flow = MessageFlowCubit();
      final SessionCubit cubit = SessionCubit(
        railCubit: rail,
        messageFlowCubit: flow,
      );
      addTearDown(cubit.close);
      addTearDown(rail.close);
      addTearDown(flow.close);

      cubit.handlePushedBytes(
        (GcsEvent()
              ..roomList = (RoomList()
                ..roomTopic.addAll(<String>['gcs/chat/a', 'gcs/chat/b'])))
            .writeToBuffer(),
      );
      expect(rail.state.rooms, <String>['gcs/chat/a', 'gcs/chat/b']);

      cubit.handlePushedBytes(
        (GcsEvent()..readiness = (Readiness()..ready = true)).writeToBuffer(),
      );
      expect(cubit.state.isReady, isTrue);

      cubit.handlePushedBytes(
        (GcsEvent()
              ..message = (ChatMessageState()
                ..id = 'm1'
                ..role = MessageRole.MESSAGE_ROLE_USER_SELF
                ..state = MessageState.MESSAGE_STATE_COMPLETE
                ..text = 'hi'))
            .writeToBuffer(),
      );
      expect(flow.state, hasLength(1));
      expect((flow.state.single as ChatFlowItemTextBubble).text, 'hi');

      cubit.handlePushedBytes(
        (GcsEvent()..error = (ErrorNotice()..message = 'boom')).writeToBuffer(),
      );
      expect(cubit.state.error, 'boom');
    });

    test(
      'pushed SpaceTree populates the rail tree with grouped rooms (D-02)',
      () {
        final RailCubit rail = RailCubit();
        final SessionCubit cubit = SessionCubit(railCubit: rail);
        addTearDown(cubit.close);
        addTearDown(rail.close);

        cubit.handlePushedBytes(
          (GcsEvent()
                ..spaceTree = (SpaceTree()
                  ..space.add(
                    SpaceRecord()
                      ..id = 'space-1'
                      ..name = 'ops'
                      ..isPublic = true
                      ..autoJoinRooms = true,
                  )
                  ..room.add(
                    RoomRecord()
                      ..id = 'room-1'
                      ..name = 'general'
                      ..parentSpaceId = 'space-1',
                  )))
              .writeToBuffer(),
        );
        expect(rail.state.treeReceived, isTrue);
        expect(rail.state.spaces, hasLength(1));
        expect(rail.state.spaces.single.id, 'space-1');
        expect(rail.state.spaces.single.name, 'ops');
        expect(rail.state.spaces.single.autoJoinRooms, isTrue);
        expect(rail.state.spaces.single.rooms, hasLength(1));
        expect(rail.state.spaces.single.rooms.single.id, 'room-1');
        expect(rail.state.spaces.single.rooms.single.name, 'general');
        expect(
          rail.state.spaces.single.rooms.single.topic,
          'gcs/chat/room-1',
        );
        expect(rail.state.standaloneRooms, isEmpty);
      },
    );

    test(
      'pushed SpaceTree: empty parentSpaceId room is standalone (D-01)',
      () {
        final RailCubit rail = RailCubit();
        final SessionCubit cubit = SessionCubit(railCubit: rail);
        addTearDown(cubit.close);
        addTearDown(rail.close);

        cubit.handlePushedBytes(
          (GcsEvent()
                ..spaceTree = (SpaceTree()
                  ..space.add(
                    SpaceRecord()
                      ..id = 'space-1'
                      ..name = 'ops',
                  )
                  ..room.add(
                    RoomRecord()
                      ..id = 'room-2'
                      ..name = 'lounge',
                  )))
              .writeToBuffer(),
        );
        expect(rail.state.treeReceived, isTrue);
        expect(rail.state.standaloneRooms, hasLength(1));
        expect(rail.state.standaloneRooms.single.id, 'room-2');
        expect(rail.state.standaloneRooms.single.name, 'lounge');
        expect(rail.state.spaces.single.rooms, isEmpty);
      },
    );

    test(
      'RoomList after SpaceTree replaces the joined view, tree fields intact',
      () {
        final RailCubit rail = RailCubit();
        final SessionCubit cubit = SessionCubit(railCubit: rail);
        addTearDown(cubit.close);
        addTearDown(rail.close);

        cubit.handlePushedBytes(
          (GcsEvent()
                ..spaceTree = (SpaceTree()
                  ..space.add(
                    SpaceRecord()
                      ..id = 'space-1'
                      ..name = 'ops',
                  )
                  ..room.add(
                    RoomRecord()
                      ..id = 'room-1'
                      ..name = 'general'
                      ..parentSpaceId = 'space-1',
                  )))
              .writeToBuffer(),
        );
        cubit.handlePushedBytes(
          (GcsEvent()
                ..roomList = (RoomList()
                  ..roomTopic.addAll(<String>['gcs/chat/room-1'])))
              .writeToBuffer(),
        );
        expect(rail.state.rooms, <String>['gcs/chat/room-1']);
        expect(rail.state.treeReceived, isTrue);
        expect(rail.state.spaces, hasLength(1));
        expect(rail.state.spaces.single.rooms.single.id, 'room-1');
      },
    );

    test(
      'malformed pushed bytes surface as an error, never a crash (T-01-11-01)',
      () {
        final SessionCubit cubit = SessionCubit();
        addTearDown(cubit.close);
        cubit.handlePushedBytes(Uint8List.fromList(<int>[0xFF, 0xFF, 0xFF]));
        expect(cubit.state.error, isNotNull);
        expect(cubit.state.isReady, isFalse);
      },
    );

    test(
      'publishCommand serializes the envelope to the command topic (D-27)',
      () async {
        final _RecordingBindings bindings = _RecordingBindings(
          initResult: ffi.Pointer<GcsSession>.fromAddress(64),
        );
        final SessionCubit cubit = SessionCubit(bindings: bindings);
        addTearDown(cubit.close);
        cubit.start();
        expect(
          cubit.publishCommand(
            GcsCommand()
              ..sendText = (SendTextCommand()
                ..roomTopic = 'gcs/chat/a'
                ..text = 'x'),
          ),
          isTrue,
        );
        expect(bindings.publishCalls, 1);
        expect(bindings.lastPublishTopic, 'gcs/command');
        expect(bindings.lastPublishedCommand!.sendText.text, 'x');
      },
    );

    test(
      'openDefault stays inert under the test harness (no FFI in tests)',
      () {
        final SessionCubit cubit = SessionCubit.openDefault();
        addTearDown(cubit.close);
        cubit.start();
        expect(cubit.state.isHandleOpen, isFalse);
      },
    );
  });
}
