/// ChatMessageCodeBlockState -- immutable state for the
/// ChatMessageCodeBlock item.
///
/// Generated from chat_message_code_block_state.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_code_block_state.dart.jinja2
/// Generator version: 0.4.0
/// Plain Dart state class consumed by ChatMessageCodeBlockCubit. Mirrors
/// the variant axis (D-19 states: pending, streaming, thinking, complete, error) and the protobuf
/// ChatMessageState taxonomy from src/proto/gcs_chat.proto (D-24
/// both-halves: the state axis shares the MessageState enum names; the
/// code payload fields are this composite's own, append-only). No
/// role axis -- top-level flow item, no bubble chrome (D-20). In-memory only.
library;

// ---------------------------------------------------------------------------
// State class
// ---------------------------------------------------------------------------

/// Immutable state for the [ChatMessageCodeBlock] top-level flow item.
///
/// [state] carries the axis value (append-only, mirroring the MessageState
/// enum in src/proto/gcs_chat.proto); [code] is the code payload
/// (D-20), an authoritative snapshot pushed from C++ (D-04). New instances
/// are produced exclusively via [copyWith]; the cubit emits them to drive
/// widget rebuilds.
class ChatMessageCodeBlockState {
  /// Creates a [ChatMessageCodeBlockState] with the given values.
  const ChatMessageCodeBlockState({
    this.state = 'complete',
    this.code = '',
    this.language = '',
    this.filename = '',
  });

  /// Runtime state axis value (D-19), mirroring MessageState in
  /// src/proto/gcs_chat.proto. Append-only.
  final String state;

  /// The code payload (D-20): authoritative code snapshot pushed
  /// from C++ (D-04).
  final String code;

  /// Optional language tag for the atom header (empty when unknown).
  final String language;

  /// Optional filename for the atom header (empty when unknown).
  final String filename;

  /// States the generator produced chrome for (D-19 axis snapshot).
  static const List<String> kValidStates = <String>[
    'pending',
    'streaming',
    'thinking',
    'complete',
    'error',
  ];

  /// Returns a copy of this state with the given fields replaced.
  ChatMessageCodeBlockState copyWith({
    String? state,
    String? code,
    String? language,
    String? filename,
  }) {
    return ChatMessageCodeBlockState(
      state: state ?? this.state,
      code: code ?? this.code,
      language: language ?? this.language,
      filename: filename ?? this.filename,
    );
  }
}
