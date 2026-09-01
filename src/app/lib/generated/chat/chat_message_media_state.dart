/// ChatMessageMediaState -- immutable state for the
/// ChatMessageMedia item.
///
/// Generated from chat_message_media_state.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_media_state.dart.jinja2
/// Generator version: 0.4.0
/// Plain Dart state class consumed by ChatMessageMediaCubit. Mirrors
/// the variant axis (D-19 states: pending, streaming, thinking, complete, error) and the protobuf
/// ChatMessageState taxonomy from src/proto/gcs_chat.proto (D-24
/// both-halves: the state axis shares the MessageState enum names; the
/// media payload fields are this composite's own, append-only). No
/// role axis -- top-level flow item, no bubble chrome (D-20). In-memory only.
library;

// ---------------------------------------------------------------------------
// State class
// ---------------------------------------------------------------------------

/// Immutable state for the [ChatMessageMedia] top-level flow item.
///
/// [state] carries the axis value (append-only, mirroring the MessageState
/// enum in src/proto/gcs_chat.proto); [mediaRef] is the media
/// payload (D-20), an opaque reference pushed from C++ (D-04) and resolved
/// to an image provider at the widget boundary, never here. New instances
/// are produced exclusively via [copyWith]; the cubit emits them to drive
/// widget rebuilds.
class ChatMessageMediaState {
  /// Creates a [ChatMessageMediaState] with the given values.
  const ChatMessageMediaState({
    this.state = 'complete',
    this.mediaRef = '',
    this.title = '',
  });

  /// Runtime state axis value (D-19), mirroring MessageState in
  /// src/proto/gcs_chat.proto. Append-only.
  final String state;

  /// The media payload (D-20): opaque media reference pushed from
  /// C++ (D-04).
  final String mediaRef;

  /// Display label rendered in the card's metadata row.
  final String title;

  /// States the generator produced chrome for (D-19 axis snapshot).
  static const List<String> kValidStates = <String>[
    'pending',
    'streaming',
    'thinking',
    'complete',
    'error',
  ];

  /// Returns a copy of this state with the given fields replaced.
  ChatMessageMediaState copyWith({
    String? state,
    String? mediaRef,
    String? title,
  }) {
    return ChatMessageMediaState(
      state: state ?? this.state,
      mediaRef: mediaRef ?? this.mediaRef,
      title: title ?? this.title,
    );
  }
}
