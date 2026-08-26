/// ChatMessageBubbleState -- immutable state for the ChatMessageBubble variants.
///
/// Generated from chat_message_bubble_state.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_bubble_state.dart.jinja2
/// Generator version: 0.4.0
/// Plain Dart state class consumed by ChatMessageBubbleCubit. Mirrors
/// the variant axes (D-18 roles: user_self, user_peer, assistant, system; D-19 states:
/// pending, streaming, thinking, complete, error) and the protobuf ChatMessageState field
/// taxonomy from src/proto/gcs_chat.proto (D-24 both-halves: role/state
/// share the MessageRole/MessageState enum names). In-memory only.
library;

// ---------------------------------------------------------------------------
// State class
// ---------------------------------------------------------------------------

/// Immutable state for the [ChatMessageBubble] role variants.
///
/// [role] and [state] carry axis values (append-only, mirroring the
/// MessageRole/MessageState enums in src/proto/gcs_chat.proto); [text] is
/// the text payload (D-20). New instances are produced exclusively
/// via [copyWith]; the cubit emits them to drive widget rebuilds.
class ChatMessageBubbleState {
  /// Creates a [ChatMessageBubbleState] with the given values.
  const ChatMessageBubbleState({
    required this.role,
    this.state = 'complete',
    this.text = '',
  });

  /// Structural role axis value (D-18), mirroring MessageRole in
  /// src/proto/gcs_chat.proto. Append-only.
  final String role;

  /// Runtime state axis value (D-19), mirroring MessageState in
  /// src/proto/gcs_chat.proto. Append-only.
  final String state;

  /// The text payload (D-20).
  final String text;

  /// Roles the generator produced variant classes for (D-18 axis snapshot).
  static const List<String> kValidRoles = <String>[
    'user_self',
    'user_peer',
    'assistant',
    'system',
  ];

  /// States the generator produced chrome for (D-19 axis snapshot).
  static const List<String> kValidStates = <String>[
    'pending',
    'streaming',
    'thinking',
    'complete',
    'error',
  ];

  /// Returns a copy of this state with the given fields replaced.
  ChatMessageBubbleState copyWith({
    String? role,
    String? state,
    String? text,
  }) {
    return ChatMessageBubbleState(
      role: role ?? this.role,
      state: state ?? this.state,
      text: text ?? this.text,
    );
  }
}
