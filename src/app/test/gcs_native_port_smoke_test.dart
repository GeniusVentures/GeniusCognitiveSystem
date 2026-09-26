// ignore_for_file: avoid_print
//
// GCS Dart NativePort smoke test (plan 01-05 Task 4 — human-verify gate;
// extended 2026-09-19 for the embedded-node boot fix + Phase 2 entities).
//
// Proves the D-27/D-29 FFI data plane end to end from Dart:
//   gcs_init(serialized GcsConfig bytes, codec=PROTOBUF) — boots the embedded
//     GeniusSDK node itself when none exists in the process (empty mnemonic =
//     child-wallet contract; 2026-09-19 live-app fix), never returns null here
//     -> gcs_subscribe(event topic, NativePort)
//     -> pushed SpaceTree (empty catalog) + RoomList (>=2 smoke topics)
//        + Readiness(ready=true)
//     -> GcsCommand join_topic publish -> pushed updated RoomList
//     -> GcsCommand send_text publish -> pushed ChatMessageState echo with
//        C++-stamped authority fields (role/state/id — Dart sent a thin
//        chat-shaped struct only, per D-04).
//     -> GcsCommand create_space / create_room publishes -> pushed SpaceTree
//        carrying the C++-minted entities (the exact flow the live app's
//        "can't reach the chat core" UAT failure walked).
//
// API_DL contract (discovered during 01-05 execution): the DART side must call
// Dart_InitializeApiDL(NativeApi.initializeApiDLData) BEFORE gcs_subscribe, or
// no events arrive. The C++ half never self-initializes (nullptr segfaults in
// the vendored SDK source).

import 'dart:async';
import 'dart:ffi';
import 'dart:io';
import 'dart:isolate';
import 'dart:typed_data';

import 'package:ffi/ffi.dart';
import 'package:flutter_test/flutter_test.dart';

import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';
import 'package:flutter_app/gcs_bindings_generated.dart';

const String kCommandTopic = 'gcs/command';
const String kEventTopic = 'gcs/event';
const String kSmokeTopicA = 'gcs/chat/smoke-test';
const String kSmokeTopicB = 'gcs/chat/smoke-test-2';
const String kJoinedTopic = 'gcs/chat/dart-joined';
const String kSendText = 'hello-from-dart';
const String kSpaceName = 'ops';
const String kRoomName = 'general';
const int kGcsOk = 0;
const Duration kWaitLimit = Duration(seconds: 5);

/// Pinned pubsub listen port for this test's embedded node (continues the C++
/// FFI binaries' 41500-41502 pins): written to network_config.json under the
/// temp dir before gcs_init. GeniusNode derives ports as 40001 + hash%301
/// with no availability probe, so parallel node processes can collide on a
/// derived port; the pin sits OUTSIDE the derived range and matches no other
/// test binary's pin.
const int kPinnedPubsubPort = 41503;

/// Event-driven queue over the ReceivePort — waits are bounded futures, never
/// polling loops (D-05: push, don't poll). Waiters are queued FIFO: a second
/// [next] before an event arrives no longer replaces (and orphans) the first.
class _EventQueue {
  final List<GcsEvent> _pending = <GcsEvent>[];
  final List<Completer<GcsEvent>> _waiters = <Completer<GcsEvent>>[];

  void add(GcsEvent event)
  {
    if (_waiters.isNotEmpty)
    {
      _waiters.removeAt(0).complete(event);
      return;
    }
    _pending.add(event);
  }

  Future<GcsEvent> next()
  {
    if (_pending.isNotEmpty)
    {
      return Future<GcsEvent>.value(_pending.removeAt(0));
    }
    final Completer<GcsEvent> waiter = Completer<GcsEvent>();
    _waiters.add(waiter);
    return waiter.future;
  }
}

Pointer<Char> _topicPtr(String topic) => topic.toNativeUtf8().cast<Char>();

/// Publishes a serialized GcsCommand to the command topic. The caller owns the
/// byte buffer for the call duration only: allocate -> copy -> call -> free.
int _publishCommand(GcsBindings bindings, Pointer<GcsSession> handle, GcsCommand command)
{
  final Uint8List bytes = command.writeToBuffer();
  final Pointer<Uint8> payload = calloc<Uint8>(bytes.length);
  payload.asTypedList(bytes.length).setAll(0, bytes);
  final Pointer<Char> topic = _topicPtr(kCommandTopic);
  final int status = bindings.gcs_publish(handle, topic, payload, bytes.length);
  calloc.free(topic);
  calloc.free(payload);
  return status;
}

// The library path is owned by CMake, never by this file: ctest injects it
// via GCS_FFI_LIBRARY (see src/app/CMakeLists.txt). Unset (bare IDE runs) →
// skip with instructions instead of guessing per-platform paths.
const String kFfiLibraryEnvVar = 'GCS_FFI_LIBRARY';

void main()
{
  TestWidgetsFlutterBinding.ensureInitialized();

  test('NativePort round-trip: config-bytes init, subscribe, command publishes, pushed events', () async
  {
    final String? libPath = Platform.environment[kFfiLibraryEnvVar];
    if (libPath == null || libPath.isEmpty)
    {
      markTestSkipped('$kFfiLibraryEnvVar not set — run via ctest (CMake injects \$<TARGET_FILE:gcs_ffi>); see src/app/CMakeLists.txt');
      return;
    }
    if (!File(libPath).existsSync())
    {
      markTestSkipped('library at $libPath does not exist — build gcs_ffi first (ninja)');
      return;
    }
    final DynamicLibrary dl = DynamicLibrary.open(libPath);
    final GcsBindings bindings = GcsBindings(dl);

    // API_DL init is Dart-side (see file header) and must precede gcs_subscribe.
    final int Function(Pointer<Void>) initApiDl = dl
        .lookupFunction<Int32 Function(Pointer<Void>), int Function(Pointer<Void>)>(
          'Dart_InitializeApiDL',
        );
    expect(initApiDl(NativeApi.initializeApiDLData), isZero, reason: 'Dart_InitializeApiDL version mismatch with vendored API_DL');

    // D-29: codec-tagged config bytes; buffer freed right after the call.
    final Directory tempDir = await Directory.systemTemp.createTemp('gcs_dart_smoke');
    addTearDown(() => tempDir.delete(recursive: true));
    // Pin the embedded node's pubsub port (child_registration.cpp pattern):
    // the node reads this file at InitNetwork; the temp dir is the node base
    // path (the db sits at <tmp>/db).
    File('${tempDir.path}/network_config.json').writeAsStringSync(
      '{ "port_seed": $kPinnedPubsubPort, "auto_dht": false'
      ', "upnp_enabled": false'
      ', "pubsub_port": "$kPinnedPubsubPort" }',
    );
    final GcsConfig config = GcsConfig()
      ..dbPath = '${tempDir.path}/db'
      ..codec = Codec.CODEC_PROTOBUF;
    final Uint8List configBytes = config.writeToBuffer();
    final Pointer<Uint8> configPtr = calloc<Uint8>(configBytes.length);
    configPtr.asTypedList(configBytes.length).setAll(0, configBytes);
    final Pointer<GcsSession> handle = bindings.gcs_init(configPtr, configBytes.length);
    calloc.free(configPtr);
    // 2026-09-19 fix: gcs_init boots the embedded GeniusSDK node itself when
    // none exists (empty mnemonic = child wallet created under the db path's
    // parent). A null handle here is the live-app "can't reach the chat core"
    // regression, not a skip condition.
    expect(handle.address, isNonZero, reason: 'gcs_init must boot the embedded node (child wallet) when none exists');
    addTearDown(() => bindings.gcs_shutdown(handle));

    final ReceivePort receivePort = ReceivePort();
    addTearDown(receivePort.close);
    final _EventQueue events = _EventQueue();
    receivePort.listen((dynamic message) => events.add(GcsEvent.fromBuffer(message as Uint8List)));

    final Pointer<Char> eventTopic = _topicPtr(kEventTopic);
    final int subStatus = bindings.gcs_subscribe(handle, eventTopic, receivePort.sendPort.nativePort);
    calloc.free(eventTopic);
    expect(subStatus, kGcsOk, reason: 'gcs_subscribe must accept the port registration');

    // 1) First push: SpaceTree — empty catalog on the fresh temp store (D-02:
    //    tree before membership before readiness).
    final GcsEvent treeEvent = await events.next().timeout(kWaitLimit);
    expect(treeEvent.hasSpaceTree(), isTrue, reason: 'first pushed event is the (empty) space tree');
    expect(treeEvent.spaceTree.space, isEmpty);
    expect(treeEvent.spaceTree.room, isEmpty);

    // 2) Second push: RoomList with both pre-joined smoke topics.
    final GcsEvent roomEvent = await events.next().timeout(kWaitLimit);
    expect(roomEvent.hasRoomList(), isTrue, reason: 'second pushed event is the room list');
    expect(roomEvent.roomList.roomTopic, containsAll(<String>[kSmokeTopicA, kSmokeTopicB]));

    // 3) Third push: Readiness(ready=true).
    final GcsEvent readyEvent = await events.next().timeout(kWaitLimit);
    expect(readyEvent.hasReadiness(), isTrue);
    expect(readyEvent.readiness.ready, isTrue);

    // 4) join_topic command publish -> updated RoomList, then a MessageHistory
    //    replay batch (D-06 — empty for a freshly joined room).
    final GcsCommand joinCommand = GcsCommand()
      ..joinTopic = (JoinTopicCommand()..roomTopic = kJoinedTopic);
    expect(_publishCommand(bindings, handle, joinCommand), kGcsOk);
    final GcsEvent joinedEvent = await events.next().timeout(kWaitLimit);
    expect(joinedEvent.hasRoomList(), isTrue);
    expect(joinedEvent.roomList.roomTopic, contains(kJoinedTopic));
    final GcsEvent joinHistory = await events.next().timeout(kWaitLimit);
    expect(joinHistory.hasMessageHistory(), isTrue,
        reason: 'join_topic replays the room history (D-06)');
    expect(joinHistory.messageHistory.roomTopic, kJoinedTopic);
    expect(joinHistory.messageHistory.message, isEmpty,
        reason: 'freshly joined room has no history yet');

    // 5) send_text command publish -> pending echo then complete echo, both
    //    carrying the C++-stamped id (D-07) and role USER_SELF (D-04 — Dart
    //    sent room_topic + text only).
    final GcsCommand sendCommand = GcsCommand()
      ..sendText = (SendTextCommand()
        ..roomTopic = kSmokeTopicA
        ..text = kSendText);
    expect(_publishCommand(bindings, handle, sendCommand), kGcsOk);
    final GcsEvent pendingEcho = await events.next().timeout(kWaitLimit);
    expect(pendingEcho.hasMessage(), isTrue);
    expect(pendingEcho.message.role, MessageRole.MESSAGE_ROLE_USER_SELF);
    expect(pendingEcho.message.state, MessageState.MESSAGE_STATE_PENDING);
    expect(pendingEcho.message.text, kSendText);
    expect(pendingEcho.message.id, isNotEmpty, reason: 'C++ stamps the message id');
    final GcsEvent completeEcho = await events.next().timeout(kWaitLimit);
    expect(completeEcho.hasMessage(), isTrue);
    expect(completeEcho.message.role, MessageRole.MESSAGE_ROLE_USER_SELF);
    expect(completeEcho.message.state, MessageState.MESSAGE_STATE_COMPLETE);
    expect(completeEcho.message.text, kSendText);
    expect(completeEcho.message.id, pendingEcho.message.id,
        reason: 'pending -> complete upsert shares one id (D-07)');

    // 6) create_space command publish -> SpaceTree with the C++-minted space
    //    (D-27: data-only command; the id never comes from Dart).
    final GcsCommand createSpace = GcsCommand()
      ..createSpace = (CreateSpaceCommand()
        ..name = kSpaceName
        ..isPublic = true
        ..autoJoinRooms = true);
    expect(_publishCommand(bindings, handle, createSpace), kGcsOk);
    final GcsEvent spaceEvent = await events.next().timeout(kWaitLimit);
    expect(spaceEvent.hasSpaceTree(), isTrue);
    expect(spaceEvent.spaceTree.space, hasLength(1));
    expect(spaceEvent.spaceTree.space.first.name, kSpaceName);
    final String spaceId = spaceEvent.spaceTree.space.first.id;
    expect(spaceId, isNotEmpty, reason: 'C++ stamps the space id');

    // 7) create_room command publish -> MessageHistory replay for the derived
    //    join, then SpaceTree carrying the room nested under the space, then a
    //    RoomList that derives the join (autoJoin).
    final GcsCommand createRoom = GcsCommand()
      ..createRoom = (CreateRoomCommand()
        ..name = kRoomName
        ..parentSpaceId = spaceId);
    expect(_publishCommand(bindings, handle, createRoom), kGcsOk);
    final GcsEvent derivedHistory = await events.next().timeout(kWaitLimit);
    expect(derivedHistory.hasMessageHistory(), isTrue,
        reason: 'derived join replays the room history (D-06)');
    expect(derivedHistory.messageHistory.roomTopic, isNotEmpty);
    expect(derivedHistory.messageHistory.message, isEmpty,
        reason: 'freshly derived room has no history yet');
    final GcsEvent roomTreeEvent = await events.next().timeout(kWaitLimit);
    expect(roomTreeEvent.hasSpaceTree(), isTrue);
    expect(roomTreeEvent.spaceTree.space, hasLength(1));
    expect(roomTreeEvent.spaceTree.room, hasLength(1));
    expect(roomTreeEvent.spaceTree.room.first.name, kRoomName);
    expect(roomTreeEvent.spaceTree.room.first.parentSpaceId, spaceId);
    final String roomId = roomTreeEvent.spaceTree.room.first.id;
    final GcsEvent derivedJoinEvent = await events.next().timeout(kWaitLimit);
    expect(derivedJoinEvent.hasRoomList(), isTrue);
    expect(derivedJoinEvent.roomList.roomTopic, contains('gcs/chat/$roomId'));
  });
}
