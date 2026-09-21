/// ChatMessageFlowState -- immutable state for the
/// ChatMessageFlow envelope.
///
/// Generated from chat_message_flow_state.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_flow_state.dart.jinja2
/// Generator version: 0.4.0
/// Plain Dart state class consumed by ChatMessageFlowCubit. The state
/// IS the pushed item list (D-04: no local message synthesis, no derived
/// fields). Mirrors the variant axis (D-20 item types:
/// text_bubble, code_block, media) whose Dart half -- the sealed ChatFlowItem
/// hierarchy -- lives in the widget file; the protobuf ChatMessageState
/// taxonomy from src/proto/gcs_chat.proto is the C++ half (D-24). In-memory
/// only.
library;

import 'chat_message_flow.dart';

// ---------------------------------------------------------------------------
// State class
// ---------------------------------------------------------------------------

/// Immutable state for the [ChatMessageFlow] envelope: the pushed
/// flow items, oldest first.
///
/// New instances are produced exclusively via [copyWith]; the cubit emits
/// them to drive widget rebuilds. Lists handed to the constructor should be
/// treated as owned by the state (the cubit's appliers pass unmodifiable,
/// capped lists).
class ChatMessageFlowState {
  /// Creates a [ChatMessageFlowState] with the given items.
  const ChatMessageFlowState({
    this.items = const <ChatFlowItem>[],
  });

  /// The pushed flow items, oldest first (the widget renders newest at the
  /// bottom of the bottom-anchored flow).
  final List<ChatFlowItem> items;

  /// Item types the generator produced subclasses for (D-20 axis snapshot).
  static const List<String> kValidItemTypes = <String>[
    'text_bubble',
    'code_block',
    'media',
  ];

  /// Maximum retained flow items (T-01-10-02): appending past the cap drops
  /// the OLDEST items, bounding the rendered list against unbounded growth.
  /// Chat-domain constant (not a scaffold token); Phase 1 default value.
  static const int kMaxFlowItems = 500;

  /// Returns a copy of this state with the given fields replaced.
  ChatMessageFlowState copyWith({List<ChatFlowItem>? items}) {
    return ChatMessageFlowState(items: items ?? this.items);
  }
}
