/// ComposerCubit -- bottom-composer state for the GCS chat shell (D-11).
///
/// Thin state holder (D-04): holds the draft text plus the active room topic
/// (projected from the [RailCubit] selection). [send] builds a codec-tagged
/// `GcsCommand` envelope carrying a [SendTextCommand] and publishes it to the
/// command topic via the transport seam -- the envelope is serialized and
/// crosses the ABI through `gcs_publish` inside [SessionCubit] (D-27:
/// commands are topic publishes of codec-tagged bytes, never raw text), then
/// the draft clears. The pushed echo later arrives on the NativePort and is
/// appended by [MessageFlowCubit] -- no local echo synthesis.
library;

import 'dart:async';

import 'package:flutter_bloc/flutter_bloc.dart';

import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';

import 'rail_cubit.dart';
import 'session_cubit.dart';

/// Immutable composer state: the current draft and the active room topic.
class ComposerState {
  /// Creates a [ComposerState].
  const ComposerState({this.draft = '', this.activeRoom});

  /// Current draft text (last value handed to the composer surface).
  final String draft;

  /// Active room topic projected from the rail selection; null when no room
  /// is selected.
  final String? activeRoom;

  /// Whether a send is currently possible: a non-blank draft and a selected
  /// room.
  bool get isSendable => draft.trim().isNotEmpty && activeRoom != null;

  /// Returns a copy of this state with the given fields replaced. Passing
  /// [clearActiveRoom] clears the room (nullable-field copyWith idiom).
  ComposerState copyWith({
    String? draft,
    String? activeRoom,
    bool clearActiveRoom = false,
  }) {
    return ComposerState(
      draft: draft ?? this.draft,
      activeRoom: clearActiveRoom ? null : (activeRoom ?? this.activeRoom),
    );
  }
}

/// Cubit holding the composer draft and send path (D-11).
class ComposerCubit extends Cubit<ComposerState> {
  /// Creates a [ComposerCubit] publishing commands through [transport].
  ///
  /// When [railCubit] is given, the active-room projection is seeded from its
  /// current state and kept in sync with its stream (subscription canceled on
  /// close).
  ComposerCubit({required GcsCommandTransport transport, RailCubit? railCubit})
    : _transport = transport,
      super(const ComposerState()) {
    if (railCubit != null) {
      _onRailStateChanged(railCubit.state);
      _railSubscription = railCubit.stream.listen(_onRailStateChanged);
    }
  }

  final GcsCommandTransport _transport;
  StreamSubscription<RailState>? _railSubscription;

  /// Stores the latest draft value handed to the composer surface.
  void updateDraft(String text) {
    if (state.draft == text) {
      return;
    }
    emit(state.copyWith(draft: text));
  }

  /// Publishes the held draft for the active room as a `GcsCommand`
  /// `send_text` envelope (D-27), then clears the draft. Returns true when
  /// the draft is published (cleared) or there is nothing sendable (no-op);
  /// false only when a sendable draft's publish was refused by the transport
  /// (D-07 transport failure -- the draft is retained for a manual re-send).
  bool send() {
    final ComposerState current = state;
    final String? roomTopic = current.activeRoom;
    if (!current.isSendable || roomTopic == null) {
      return true;
    }
    final GcsCommand command = GcsCommand()
      ..sendText = (SendTextCommand()
        ..roomTopic = roomTopic
        ..text = current.draft);
    if (!_transport.publishCommand(command)) {
      return false;
    }
    emit(current.copyWith(draft: ''));
    return true;
  }

  /// Projects the rail state onto the composer (active-room follow).
  void _onRailStateChanged(RailState railState) {
    final String? activeRoom = railState.activeRoom;
    if (activeRoom == null) {
      if (state.activeRoom != null) {
        emit(state.copyWith(clearActiveRoom: true));
      }
      return;
    }
    if (state.activeRoom != activeRoom) {
      emit(state.copyWith(activeRoom: activeRoom));
    }
  }

  @override
  Future<void> close() async {
    await _railSubscription?.cancel();
    _railSubscription = null;
    await super.close();
  }
}
