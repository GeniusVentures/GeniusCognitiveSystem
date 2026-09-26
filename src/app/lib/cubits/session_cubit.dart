/// SessionCubit -- owner of the GCS FFI session handle and the pushed-event
/// decode/dispatch pipeline (D-04/D-05/D-26/D-29).
///
/// Lifecycle: [start] serializes a `GcsConfig` (codec-tagged protobuf bytes,
/// D-29) and calls `gcs_init`, registers a `ReceivePort` via `gcs_subscribe`
/// on the event topic, and then ONLY receives: every `GcsEvent` arrives
/// pushed on the port (D-05 -- no pull, no polling) and is decoded with
/// `GcsEvent.fromBuffer` and dispatched per the plan interfaces contract:
///
///   spaceTree -> RailCubit.setTree            (D-02 pushed catalog tree)
///   roomList -> RailCubit.setRooms            (D-21 pushed room list)
///   readiness -> session ready flag
///   message  -> MessageFlowCubit.upsert(buildChatFlowItemTextBubble(...))
///                -- gated by the rail's active room (CR-01: the C++ side
///                   live-subscribes every joined room, so ungated dispatch
///                   would render one room's traffic in another)
///   messageHistory -> MessageFlowCubit.replaceAll(mapped batch) (D-06, same
///                active-room gate -- a replay for a just-joined room must
///                not wipe the room the user is reading)
///   error    -> session error surface (raw string per D-29)
///
/// Rail selection is watched too (review P1): the active-room gate above
/// discards a replay pushed while the room is not selected, so when the rail
/// selection CHANGES the session clears the flow and re-publishes an
/// idempotent join_topic for the newly active room -- the C++ side replays
/// that room's MessageHistory on re-join, and the batch then passes the gate.
///
/// [close] cancels the rail subscription, then closes the ReceivePort BEFORE
/// the native `gcs_shutdown` (pitfall ordering) exactly once; a null handle
/// is a no-op (T-01-11-04: the handle never leaks to widgets).
library;

import 'dart:async';
import 'dart:ffi' as ffi;
import 'dart:io' show File, Platform;
import 'dart:isolate' show ReceivePort;
import 'dart:typed_data' show Uint8List;

import 'package:ffi/ffi.dart';
import 'package:flutter_bloc/flutter_bloc.dart';

import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';
import 'package:flutter_app/gcs_bindings_generated.dart';

import 'message_flow_cubit.dart';
import 'rail_cubit.dart';

/// Topic carrying every pushed `GcsEvent` (D-26 event stream).
const String kGcsEventTopic = 'gcs/event';

/// Topic receiving every published `GcsCommand` (D-27 command publishes).
const String kGcsCommandTopic = 'gcs/command';

/// Environment variable carrying the gcs_ffi shared-library path (ctest
/// injects `$<TARGET_FILE:gcs_ffi>`; see src/app/CMakeLists.txt).
const String kFfiLibraryEnvVar = 'GCS_FFI_LIBRARY';

/// File names probed for the packaged gcs_ffi library, per-platform file
/// naming (macOS dylib / Windows dll / Linux so).
const List<String> kPackagedFfiLibraryFileNames = <String>[
  'libgcs_ffi.dylib',
  'gcs_ffi.dll',
  'libgcs_ffi.so',
];

/// Environment flag set by the Flutter test harness; the default session
/// never opens the real library under it.
const String kTestHarnessEnvVar = 'FLUTTER_TEST';

/// Typed seam for outgoing commands (D-27): implementers serialize the
/// envelope to codec-tagged bytes and publish it to [kGcsCommandTopic] via
/// `gcs_publish`. Implemented by [SessionCubit]; fakes drive tests.
abstract interface class GcsCommandTransport {
  /// Publishes [command]; returns whether the native publish accepted it.
  bool publishCommand(GcsCommand command);
}

/// Immutable session state: handle-open + readiness flags and the raw error
/// surface (D-29 -- errors are raw strings, never typed exceptions).
class SessionState {
  /// Creates a [SessionState].
  const SessionState({
    this.isHandleOpen = false,
    this.isReady = false,
    this.error,
  });

  /// Whether a native session handle is currently open.
  final bool isHandleOpen;

  /// Readiness flag pushed by the C++ side (true once init + smoke-topic
  /// pre-join complete). Gates the composer.
  final bool isReady;

  /// Last surfaced raw error string, if any.
  final String? error;

  /// Returns a copy of this state with the given fields replaced. Passing
  /// [clearError] clears the error (nullable-field copyWith idiom).
  SessionState copyWith({
    bool? isHandleOpen,
    bool? isReady,
    String? error,
    bool clearError = false,
  }) {
    return SessionState(
      isHandleOpen: isHandleOpen ?? this.isHandleOpen,
      isReady: isReady ?? this.isReady,
      error: clearError ? null : (error ?? this.error),
    );
  }
}

/// Cubit owning the FFI handle lifecycle and the pushed-event dispatch
/// (D-01/D-04/D-05). Construct with [bindings] null for an inert session
/// (shell-only rendering, tests); [openDefault] resolves the real library.
class SessionCubit extends Cubit<SessionState> implements GcsCommandTransport {
  /// Creates a [SessionCubit].
  ///
  /// [railCubit]/[messageFlowCubit] are the dispatch targets for pushed
  /// room-list and message events; [initialError] seeds the error surface
  /// (e.g. a failed library open).
  SessionCubit({
    GcsBindings? bindings,
    String? dbPath,
    RailCubit? railCubit,
    MessageFlowCubit? messageFlowCubit,
    String? initialError,
  }) : _bindings = bindings,
       _dbPath = dbPath,
       _railCubit = railCubit,
       _messageFlowCubit = messageFlowCubit,
       super(SessionState(error: initialError)) {
    if (railCubit != null) {
      _lastSeenActiveRoom = railCubit.state.activeRoom;
      _railSubscription = railCubit.stream.listen(_onRailStateChanged);
    }
  }

  final GcsBindings? _bindings;
  final RailCubit? _railCubit;
  final MessageFlowCubit? _messageFlowCubit;
  final String? _dbPath;

  ffi.Pointer<GcsSession> _handle = ffi.Pointer<GcsSession>.fromAddress(0);
  ReceivePort? _receivePort;
  bool _nativeShutdownDone = false;
  StreamSubscription<RailState>? _railSubscription;
  String? _lastSeenActiveRoom;

  /// Creates the production session, resolving the gcs_ffi shared library
  /// from [kFfiLibraryEnvVar], falling back to the packaged location next to
  /// the executable (macOS bundle `Contents/Frameworks`, or the executable's
  /// own directory — the Windows/Linux package layout).
  ///
  /// Inert (no bindings) when the harness is a Flutter test, no candidate
  /// resolves, or the library file is absent -- the shell then renders with
  /// the composer disabled until readiness would have been pushed. Opens the
  /// library, initializes Dart API_DL (required before `gcs_subscribe`; the
  /// C++ side never self-initializes), and hands the bindings to a normal
  /// [SessionCubit].
  static SessionCubit openDefault({
    String? dbPath,
    RailCubit? railCubit,
    MessageFlowCubit? messageFlowCubit,
  }) {
    if (Platform.environment[kTestHarnessEnvVar] == 'true') {
      return SessionCubit(
        railCubit: railCubit,
        messageFlowCubit: messageFlowCubit,
      );
    }
    final String? fromEnv = Platform.environment[kFfiLibraryEnvVar];
    final String? libraryPath = _resolveLibraryPath();
    if (libraryPath == null) {
      // Distinguish a stale configured path from an unset variable (IN-09):
      // resolution returned null while the env value is non-empty, so the
      // configured file does not exist (a live env file would have been
      // returned) and no packaged candidate covered for it.
      final bool envPathSet = fromEnv != null && fromEnv.isNotEmpty;
      return SessionCubit(
        railCubit: railCubit,
        messageFlowCubit: messageFlowCubit,
        initialError: envPathSet
            ? 'gcs_ffi library not found at $fromEnv (check $kFfiLibraryEnvVar)'
            : 'gcs_ffi library not found (set $kFfiLibraryEnvVar)',
      );
    }
    try {
      final ffi.DynamicLibrary library = ffi.DynamicLibrary.open(libraryPath);
      // API_DL init is Dart-side (01-05 contract) and must precede
      // gcs_subscribe, or no events arrive.
      final int Function(ffi.Pointer<ffi.Void>) initializeApiDl = library
          .lookupFunction<
            ffi.Int32 Function(ffi.Pointer<ffi.Void>),
            int Function(ffi.Pointer<ffi.Void>)
          >('Dart_InitializeApiDL');
      final int result = initializeApiDl(ffi.NativeApi.initializeApiDLData);
      if (result != 0) {
        return SessionCubit(
          railCubit: railCubit,
          messageFlowCubit: messageFlowCubit,
          initialError: 'Dart_InitializeApiDL version mismatch ($result)',
        );
      }
      return SessionCubit(
        bindings: GcsBindings(library),
        dbPath: dbPath,
        railCubit: railCubit,
        messageFlowCubit: messageFlowCubit,
      );
    } catch (error) {
      return SessionCubit(
        railCubit: railCubit,
        messageFlowCubit: messageFlowCubit,
        initialError: 'failed to open gcs_ffi library: $error',
      );
    }
  }

  /// Resolves the gcs_ffi library path: [kFfiLibraryEnvVar] when set and
  /// present, else the packaged location relative to the running executable
  /// (macOS bundle `../Frameworks/`, then the executable's own directory —
  /// the Windows/Linux package layout). Returns null when no candidate file
  /// exists, leaving the session inert.
  static String? _resolveLibraryPath() {
    final String? fromEnv = Platform.environment[kFfiLibraryEnvVar];
    if (fromEnv != null && fromEnv.isNotEmpty && File(fromEnv).existsSync()) {
      return fromEnv;
    }
    final String exeDir = File(Platform.resolvedExecutable).parent.path;
    final List<String> candidates = <String>[
      for (final String fileName in kPackagedFfiLibraryFileNames) ...<String>[
        '$exeDir/../Frameworks/$fileName',
        '$exeDir/$fileName',
      ],
    ];
    for (final String candidate in candidates) {
      if (File(candidate).existsSync()) {
        return candidate;
      }
    }
    return null;
  }

  /// The opaque native session handle (address 0 = no open session). Owned
  /// exclusively here (T-01-11-04); never exposed to widgets.
  ffi.Pointer<GcsSession> get handle => _handle;

  /// Initializes the native session: serializes the `GcsConfig`
  /// (codec-tagged protobuf bytes, D-29), calls `gcs_init`
  /// (allocate/copy/call/free), registers the ReceivePort on
  /// [kGcsEventTopic] via `gcs_subscribe`, and starts listening. No-op when
  /// inert, already torn down, or already started (re-entry guard WR-02: a
  /// second call would leak the first ReceivePort and re-subscribe over it).
  ///
  /// Requires [dbPath] (or the [openDefault] argument) to be set to a
  /// NON-EMPTY path: the app's `main()` derives it from the per-user
  /// application-support directory (`data/KEY` — see main.dart); a missing
  /// or empty path surfaces a raw error instead of silently defaulting to a
  /// system temp directory (IN-03: an empty string reaches SessionBasePath's
  /// temp-dir fallback, the exact default this guard exists to eliminate).
  void start() {
    final GcsBindings? bindings = _bindings;
    if (bindings == null || _nativeShutdownDone || _receivePort != null) {
      return; // inert / torn down / already started
    }
    final String? dbPath = _dbPath;
    if (dbPath == null || dbPath.isEmpty) {
      emit(
        state.copyWith(
          error:
              'session db path not configured (main() derives it from '
              'the per-user data directory)',
        ),
      );
      return;
    }
    final Uint8List configBytes =
        (GcsConfig()
              ..dbPath = dbPath
              ..codec = Codec.CODEC_PROTOBUF)
            .writeToBuffer();
    final ffi.Pointer<ffi.Uint8> configPtr = calloc<ffi.Uint8>(
      configBytes.length,
    );
    configPtr.asTypedList(configBytes.length).setAll(0, configBytes);
    final ffi.Pointer<GcsSession> handle = bindings.gcs_init(
      configPtr,
      configBytes.length,
    );
    calloc.free(configPtr);
    if (handle.address == 0) {
      emit(
        state.copyWith(
          error: 'gcs_init failed (config rejected or node unavailable)',
        ),
      );
      return;
    }
    _handle = handle;

    final ReceivePort receivePort = ReceivePort();
    _receivePort = receivePort;
    receivePort.listen((dynamic message) {
      if (message is Uint8List) {
        handlePushedBytes(message);
      }
    });

    final ffi.Pointer<ffi.Char> topic = kGcsEventTopic
        .toNativeUtf8()
        .cast<ffi.Char>();
    final int status = bindings.gcs_subscribe(
      handle,
      topic,
      receivePort.sendPort.nativePort,
    );
    calloc.free(topic);
    if (status != GcsStatus.GCS_OK) {
      _closeNativeOnce();
      emit(state.copyWith(error: 'gcs_subscribe failed with status $status'));
      return;
    }
    emit(state.copyWith(isHandleOpen: true));
  }

  /// Decodes pushed `GcsEvent` bytes and dispatches them per the interfaces
  /// contract. Malformed bytes surface as a raw error string, never a crash
  /// (T-01-11-01). Public so the port listener path is testable end to end.
  void handlePushedBytes(Uint8List bytes) {
    final GcsEvent event;
    try {
      event = GcsEvent.fromBuffer(bytes);
    } catch (error) {
      emit(state.copyWith(error: 'malformed pushed event: $error'));
      return;
    }
    _dispatchEvent(event);
  }

  /// Publishes a serialized `GcsCommand` envelope to [kGcsCommandTopic]
  /// (D-27: codec-tagged bytes, allocate/copy/call/free). Returns false when
  /// no session is open.
  @override
  bool publishCommand(GcsCommand command) {
    final GcsBindings? bindings = _bindings;
    final ffi.Pointer<GcsSession> handle = _handle;
    if (bindings == null || handle.address == 0) {
      return false;
    }
    final Uint8List payload = command.writeToBuffer();
    final ffi.Pointer<ffi.Uint8> payloadPtr = calloc<ffi.Uint8>(payload.length);
    payloadPtr.asTypedList(payload.length).setAll(0, payload);
    final ffi.Pointer<ffi.Char> topic = kGcsCommandTopic
        .toNativeUtf8()
        .cast<ffi.Char>();
    final int status = bindings.gcs_publish(
      handle,
      topic,
      payloadPtr,
      payload.length,
    );
    calloc.free(topic);
    calloc.free(payloadPtr);
    return status == GcsStatus.GCS_OK;
  }

  /// Requests the newly active room's history when the rail selection
  /// changes (review P1): a MessageHistory batch pushed at join time is
  /// discarded by the active-room gate while the room is not selected, so
  /// the flow would otherwise keep showing the previously active room with
  /// no way to refill. The join_topic publish is idempotent on the C++ side
  /// (RoomList refresh + MessageHistory replay; the live subscribe is not
  /// re-armed), and the replayed batch then passes the gate. The flow is
  /// cleared first so the previous room's items never linger while the
  /// replay is in flight. Redundant rail emissions (same active room) and
  /// null selections publish nothing.
  void _onRailStateChanged(RailState railState) {
    final String? activeRoom = railState.activeRoom;
    if (activeRoom == null) {
      _lastSeenActiveRoom = null;
      return;
    }
    if (activeRoom == _lastSeenActiveRoom) {
      return;
    }
    _lastSeenActiveRoom = activeRoom;
    _messageFlowCubit?.clear();
    publishCommand(
      GcsCommand()..joinTopic = (JoinTopicCommand()..roomTopic = activeRoom),
    );
  }

  /// Cancels the rail subscription, then closes the ReceivePort BEFORE the
  /// native `gcs_shutdown`, exactly once (idempotent guard; null handle
  /// no-op) -- pitfall ordering so the native side can never post into a
  /// disposed port and no selection-driven publish races the teardown.
  @override
  Future<void> close() async {
    await _railSubscription?.cancel();
    _railSubscription = null;
    _closeNativeOnce();
    await super.close();
  }

  /// Single-shot native teardown shared by [close] and the subscribe-failure
  /// path: port first, then handle, then forget the handle.
  void _closeNativeOnce() {
    if (_nativeShutdownDone) {
      return;
    }
    _nativeShutdownDone = true;
    _receivePort?.close();
    _receivePort = null;
    final ffi.Pointer<GcsSession> handle = _handle;
    _handle = ffi.Pointer<GcsSession>.fromAddress(0);
    if (handle.address != 0) {
      _bindings?.gcs_shutdown(handle);
    }
  }

  /// Routes one decoded event to its dispatch target (interfaces contract).
  /// The SpaceTree arm runs FIRST -- the catalog renders before membership
  /// (D-02 push ordering).
  void _dispatchEvent(GcsEvent event) {
    if (event.hasSpaceTree()) {
      _railCubit?.setTree(event.spaceTree.space, event.spaceTree.room);
      return;
    }
    if (event.hasRoomList()) {
      _railCubit?.setRooms(event.roomList.roomTopic);
      return;
    }
    if (event.hasReadiness()) {
      emit(state.copyWith(isReady: event.readiness.ready));
      return;
    }
    if (event.hasMessage()) {
      // CR-01: the C++ side live-subscribes EVERY joined room, so a pushed
      // message is rendered only when it belongs to the rail's active room --
      // live traffic from another room must never land in the open one.
      final MessageFlowCubit? flow = _messageFlowCubit;
      if (flow != null &&
          _railCubit?.state.activeRoom == event.message.roomTopic) {
        flow.upsert(flow.buildChatFlowItemTextBubble(event.message));
      }
      return;
    }
    if (event.hasMessageHistory()) {
      // CR-01: same active-room gate for the D-06 replay batch -- a history
      // replay pushed for a room the user just joined must not replaceAll-
      // wipe the flow of the room the user is currently reading.
      final MessageFlowCubit? flow = _messageFlowCubit;
      if (flow != null &&
          _railCubit?.state.activeRoom == event.messageHistory.roomTopic) {
        flow.replaceAll([
          for (final ChatMessageState m in event.messageHistory.message)
            flow.buildChatFlowItemTextBubble(m),
        ]);
      }
      return;
    }
    if (event.hasError()) {
      emit(state.copyWith(error: event.error.message));
    }
  }
}
