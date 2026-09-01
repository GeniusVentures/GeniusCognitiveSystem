/// ChatMessageMediaCubit -- Cubit for the ChatMessageMedia item.
///
/// Generated from chat_message_media_cubit.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_media_cubit.dart.jinja2
/// Generator version: 0.4.0
/// Thin FFI-event-driven Cubit (D-04/D-27): applies pushed state/payload
/// values, never fetches or decodes media content. C++ owns application
/// state; Dart renders. State axis (D-19): pending, streaming, thinking, complete, error. No
/// role axis -- the media card is a top-level flow item without bubble
/// chrome (D-20). In-memory only -- no hydration.
library;

import 'package:flutter_bloc/flutter_bloc.dart';

import 'chat_message_media_state.dart';

// ---------------------------------------------------------------------------
// Cubit
// ---------------------------------------------------------------------------

/// Cubit for the [ChatMessageMedia] top-level flow item.
///
/// Owns the item's runtime state-axis value and media payload.
/// Values arrive from pushed FFI events; the mutation methods below are thin
/// appliers, not synthesizers (D-04).
class ChatMessageMediaCubit extends Cubit<ChatMessageMediaState> {
  /// Creates a [ChatMessageMediaCubit].
  ///
  /// [initialMessageState] seeds the state axis (one of
  /// [ChatMessageMediaState.kValidStates]); [initialMediaRef] seeds
  /// the media payload reference; [initialTitle] seeds the display
  /// label.
  ChatMessageMediaCubit({
    this.instanceId = '',
    this.initialMessageState = 'complete',
    String initialMediaRef = '',
    String initialTitle = '',
  })  : assert(
          ChatMessageMediaState.kValidStates.contains(initialMessageState),
          'initialMessageState must be one of the generated states axis values',
        ),
        super(
          ChatMessageMediaState(
            state: initialMessageState,
            mediaRef: initialMediaRef,
            title: initialTitle,
          ),
        );

  /// Optional instance discriminator (kept for API uniformity with the
  /// scaffold component cubits; unused by the in-memory cubit).
  final String instanceId;

  /// The state-axis value used when no pushed value has been applied yet.
  final String initialMessageState;

  /// Applies a pushed state-axis value (D-19 registry in the widget file).
  void applyState(String value) {
    assert(
      ChatMessageMediaState.kValidStates.contains(value),
      'value must be one of the generated states axis values',
    );
    emit(state.copyWith(state: value));
  }

  /// Replaces the media payload reference with a pushed value.
  /// C++ pushes authoritative snapshots; Dart never resolves or fetches the
  /// reference itself (D-04).
  void updateMediaRef(String value) {
    emit(state.copyWith(mediaRef: value));
  }

  /// Replaces the display label with a pushed value.
  void updateTitle(String value) {
    emit(state.copyWith(title: value));
  }

  /// Resets to the seeded state-axis value and empty payload fields.
  void reset() {
    emit(
      ChatMessageMediaState(
        state: initialMessageState,
        mediaRef: '',
        title: '',
      ),
    );
  }
}
