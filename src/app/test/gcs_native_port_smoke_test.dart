// ignore_for_file: avoid_print
//
// GCS Dart NativePort smoke test (plan 01-05 Task 4 — human-verify gate).
//
// Proves the D-27/D-29 FFI data plane end to end from Dart:
//   gcs_init(serialized GcsConfig bytes, codec=PROTOBUF)
//     -> gcs_subscribe(event topic, NativePort)
//     -> pushed RoomList (>=2 smoke topics) + Readiness(ready=true)
//     -> GcsCommand join_topic publish -> pushed updated RoomList
//     -> GcsCommand send_text publish -> pushed ChatMessageState echo with
//        C++-stamped authority fields (role/state/id — Dart sent a thin
//        chat-shaped struct only, per D-04).
//
// API_DL contract (discovered during 01-05 execution): the DART side must call
// Dart_InitializeApiDL(NativeApi.initializeApiDLData) BEFORE gcs_subscribe, or
// no events arrive. The C++ half never self-initializes (nullptr segfaults in
// the vendored SDK source).
//
// Option C: if gcs_init returns null (GeniusSDK node unavailable in this
// process), the test skips with a note — acceptable per the phase contract.

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
const int kGcsOk = 0;
const Duration kWaitLimit = Duration(seconds: 5);

/// Event-driven queue over the ReceivePort — waits are bounded futures, never
/// polling loops (D-05: push, don't poll).
class _EventQueue {
  final List<GcsEvent> _pending = <GcsEvent>[];
  Completer<GcsEvent>? _waiter;

  void add(GcsEvent event)
  {
    final Completer<GcsEvent>? waiter = _waiter;
    if (waiter != null && !waiter.isCompleted)
    {
      _waiter = null;
      waiter.complete(event);
    }
    else
    {
      _pending.add(event);
    }
  }

  Future<GcsEvent> next()
  {
    if (_pending.isNotEmpty)
    {
      return Future<GcsEvent>.value(_pending.removeAt(0));
    }
    final Completer<GcsEvent> waiter = Completer<GcsEvent>();
    _waiter = waiter;
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
    final GcsConfig config = GcsConfig()
      ..dbPath = '${tempDir.path}/db'
      ..codec = Codec.CODEC_PROTOBUF;
    final Uint8List configBytes = config.writeToBuffer();
    final Pointer<Uint8> configPtr = calloc<Uint8>(configBytes.length);
    configPtr.asTypedList(configBytes.length).setAll(0, configBytes);
    final Pointer<GcsSession> handle = bindings.gcs_init(configPtr, configBytes.length);
    calloc.free(configPtr);
    if (handle.address == 0)
    {
      markTestSkipped('gcs_init returned null (GeniusSDK node unavailable) — acceptable per option C; wiring proven through init');
      return;
    }
    addTearDown(() => bindings.gcs_shutdown(handle));

    final ReceivePort receivePort = ReceivePort();
    addTearDown(receivePort.close);
    final _EventQueue events = _EventQueue();
    receivePort.listen((dynamic message) => events.add(GcsEvent.fromBuffer(message as Uint8List)));

    final Pointer<Char> eventTopic = _topicPtr(kEventTopic);
    final int subStatus = bindings.gcs_subscribe(handle, eventTopic, receivePort.sendPort.nativePort);
    calloc.free(eventTopic);
    expect(subStatus, kGcsOk, reason: 'gcs_subscribe must accept the port registration');

    // 1) First push: RoomList with both pre-joined smoke topics.
    final GcsEvent roomEvent = await events.next().timeout(kWaitLimit);
    expect(roomEvent.hasRoomList(), isTrue, reason: 'first pushed event is the room list');
    expect(roomEvent.roomList.roomTopic, containsAll(<String>[kSmokeTopicA, kSmokeTopicB]));

    // 2) Second push: Readiness(ready=true).
    final GcsEvent readyEvent = await events.next().timeout(kWaitLimit);
    expect(readyEvent.hasReadiness(), isTrue);
    expect(readyEvent.readiness.ready, isTrue);

    // 3) join_topic command publish -> updated RoomList (oneof dispatch #1).
    final GcsCommand joinCommand = GcsCommand()
      ..joinTopic = (JoinTopicCommand()..roomTopic = kJoinedTopic);
    expect(_publishCommand(bindings, handle, joinCommand), kGcsOk);
    final GcsEvent joinedEvent = await events.next().timeout(kWaitLimit);
    expect(joinedEvent.hasRoomList(), isTrue);
    expect(joinedEvent.roomList.roomTopic, contains(kJoinedTopic));

    // 4) send_text command publish -> ChatMessageState echo with C++-stamped
    //    authority fields (D-04: Dart sent room_topic + text only).
    final GcsCommand sendCommand = GcsCommand()
      ..sendText = (SendTextCommand()
        ..roomTopic = kSmokeTopicA
        ..text = kSendText);
    expect(_publishCommand(bindings, handle, sendCommand), kGcsOk);
    final GcsEvent echo = await events.next().timeout(kWaitLimit);
    expect(echo.hasMessage(), isTrue);
    expect(echo.message.role, MessageRole.MESSAGE_ROLE_USER_SELF);
    expect(echo.message.state, MessageState.MESSAGE_STATE_COMPLETE);
    expect(echo.message.text, kSendText);
    expect(echo.message.id, isNotEmpty, reason: 'C++ stamps the message id');
  });
}
