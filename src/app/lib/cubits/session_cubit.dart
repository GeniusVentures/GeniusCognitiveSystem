/// SessionCubit -- owner of the GCS FFI session handle and the pushed-event
/// decode/dispatch pipeline (D-04/D-05/D-26/D-29).
///
/// Lifecycle: [start] serializes a `GcsConfig` (codec-tagged protobuf bytes,
/// D-29) and calls `gcs_init`, registers a `ReceivePort` via `gcs_subscribe`
/// on the event topic, and then ONLY receives: every `GcsEvent` arrives
/// pushed on the port (D-05 -- no pull, no polling) and is decoded with
/// `GcsEvent.fromBuffer` and dispatched per the plan interfaces contract:
///
///   roomList -> RailCubit.setRooms            (D-21 pushed room list)
///   readiness -> session ready flag
///   message  -> MessageFlowCubit.append(buildChatFlowItemTextBubble(...))
///   error    -> session error surface (raw string per D-29)
///
/// [close] closes the ReceivePort BEFORE the native `gcs_shutdown` (pitfall
/// ordering) exactly once; a null handle is a no-op (T-01-11-04: the handle
/// never leaks to widgets).
library;

import 'dart:async';
import 'dart:ffi' as ffi;
import 'dart:io' show Directory, File, Platform;
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

/// Environment flag set by the Flutter test harness; the default session
/// never opens the real library under it.
const String kTestHarnessEnvVar = 'FLUTTER_TEST';

/// Default session directory under the system temp directory.
const String kDefaultSessionDirectoryName = 'gcs_chat_session';

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
       super(SessionState(error: initialError));

  final GcsBindings? _bindings;
  final RailCubit? _railCubit;
  final MessageFlowCubit? _messageFlowCubit;
  String? _dbPath;

  ffi.Pointer<GcsSession> _handle = ffi.Pointer<GcsSession>.fromAddress(0);
  ReceivePort? _receivePort;
  bool _nativeShutdownDone = false;

  /// Creates the production session, resolving the gcs_ffi shared library
  /// from [kFfiLibraryEnvVar].
  ///
  /// Inert (no bindings) when the harness is a Flutter test, the variable is
  /// unset, or the library file is absent -- the shell then renders with the
  /// composer disabled until readiness would have been pushed. Opens the
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
    final String? libraryPath = Platform.environment[kFfiLibraryEnvVar];
    if (libraryPath == null ||
        libraryPath.isEmpty ||
        !File(libraryPath).existsSync()) {
      return SessionCubit(
        railCubit: railCubit,
        messageFlowCubit: messageFlowCubit,
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

  /// The opaque native session handle (address 0 = no open session). Owned
  /// exclusively here (T-01-11-04); never exposed to widgets.
  ffi.Pointer<GcsSession> get handle => _handle;

  /// Initializes the native session: serializes the `GcsConfig`
  /// (codec-tagged protobuf bytes, D-29), calls `gcs_init`
  /// (allocate/copy/call/free), registers the ReceivePort on
  /// [kGcsEventTopic] via `gcs_subscribe`, and starts listening. No-op when
  /// inert or already torn down.
  void start() {
    final GcsBindings? bindings = _bindings;
    if (bindings == null || _nativeShutdownDone) {
      return;
    }
    _dbPath ??= '${Directory.systemTemp.path}/$kDefaultSessionDirectoryName';
    final Uint8List configBytes =
        (GcsConfig()
              ..dbPath = _dbPath!
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

  /// Closes the ReceivePort BEFORE the native `gcs_shutdown`, exactly once
  /// (idempotent guard; null handle no-op) -- pitfall ordering so the native
  /// side can never post into a disposed port.
  @override
  Future<void> close() async {
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
  void _dispatchEvent(GcsEvent event) {
    if (event.hasRoomList()) {
      _railCubit?.setRooms(event.roomList.roomTopic);
      return;
    }
    if (event.hasReadiness()) {
      emit(state.copyWith(isReady: event.readiness.ready));
      return;
    }
    if (event.hasMessage()) {
      final MessageFlowCubit? flow = _messageFlowCubit;
      if (flow != null) {
        flow.append(flow.buildChatFlowItemTextBubble(event.message));
      }
      return;
    }
    if (event.hasError()) {
      emit(state.copyWith(error: event.error.message));
    }
  }
}
