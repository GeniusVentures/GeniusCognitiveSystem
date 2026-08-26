/// ChatMessageCodeBlockCubit -- Cubit for the ChatMessageCodeBlock item.
///
/// Generated from chat_message_code_block_cubit.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_code_block_cubit.dart.jinja2
/// Generator version: 0.4.0
/// Thin FFI-event-driven Cubit (D-04/D-27): applies pushed state/payload
/// values, never synthesizes content. C++ owns application state; Dart
/// renders. State axis (D-19): pending, streaming, thinking, complete, error. No role axis --
/// the code block is a top-level flow item without bubble chrome (D-20).
/// In-memory only -- no hydration.
library;

import 'package:flutter_bloc/flutter_bloc.dart';

import 'chat_message_code_block_state.dart';

// ---------------------------------------------------------------------------
// Cubit
// ---------------------------------------------------------------------------

/// Cubit for the [ChatMessageCodeBlock] top-level flow item.
///
/// Owns the item's runtime state-axis value and code payload.
/// Values arrive from pushed FFI events; the mutation methods below are thin
/// appliers, not synthesizers (D-04).
class ChatMessageCodeBlockCubit extends Cubit<ChatMessageCodeBlockState> {
  /// Creates a [ChatMessageCodeBlockCubit].
  ///
  /// [initialMessageState] seeds the state axis (one of
  /// [ChatMessageCodeBlockState.kValidStates]); [initialCode] seeds the
  /// code payload; [initialLanguage]/[initialFilename] seed the
  /// optional atom-header tags.
  ChatMessageCodeBlockCubit({
    this.instanceId = '',
    this.initialMessageState = 'complete',
    String initialCode = '',
    String initialLanguage = '',
    String initialFilename = '',
  })  : assert(
          ChatMessageCodeBlockState.kValidStates.contains(initialMessageState),
          'initialMessageState must be one of the generated states axis values',
        ),
        super(
          ChatMessageCodeBlockState(
            state: initialMessageState,
            code: initialCode,
            language: initialLanguage,
            filename: initialFilename,
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
      ChatMessageCodeBlockState.kValidStates.contains(value),
      'value must be one of the generated states axis values',
    );
    emit(state.copyWith(state: value));
  }

  /// Replaces the code payload with a pushed value. Streaming
  /// replaces the full code: C++ pushes authoritative snapshots, Dart never
  /// assembles content locally (D-04).
  void updateCode(String value) {
    emit(state.copyWith(code: value));
  }

  /// Replaces the optional language tag with a pushed value.
  void updateLanguage(String value) {
    emit(state.copyWith(language: value));
  }

  /// Replaces the optional filename tag with a pushed value.
  void updateFilename(String value) {
    emit(state.copyWith(filename: value));
  }

  /// Resets to the seeded state-axis value and empty payload fields.
  void reset() {
    emit(
      ChatMessageCodeBlockState(
        state: initialMessageState,
        code: '',
        language: '',
        filename: '',
      ),
    );
  }
}
