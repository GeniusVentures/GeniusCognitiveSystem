/// ChatMessageFlowCubit -- Cubit for the ChatMessageFlow envelope.
///
/// Generated from chat_message_flow_cubit.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_flow_cubit.dart.jinja2
/// Generator version: 0.4.0
/// Thin FFI-event-driven Cubit (D-04/D-27): the state IS the pushed
/// `List<ChatFlowItem>` -- the mutation methods below are thin appliers that
/// store and cap pushed snapshots, never synthesizers of message content
/// (the shell maps FFI payloads to items upstream). The retained list is
/// capped at kMaxFlowItems, dropping the oldest (T-01-10-02). Item-type axis
/// (D-20): text_bubble, code_block, media -- append-only. In-memory only, no
/// hydration.
library;

import 'package:flutter_bloc/flutter_bloc.dart';

import 'chat_message_flow.dart';
import 'chat_message_flow_state.dart';

/// Returns [items] capped to [ChatMessageFlowState.kMaxFlowItems],
/// dropping the OLDEST entries (T-01-10-02: unbounded growth of a pushed
/// flow list must never exhaust memory). The result is unmodifiable so the
/// immutable-state contract cannot be broken by callers mutating the list
/// after emit.
List<ChatFlowItem> _cappedItems(List<ChatFlowItem> items) {
  final int maxItems = ChatMessageFlowState.kMaxFlowItems;
  final List<ChatFlowItem> capped = <ChatFlowItem>[
    if (items.length > maxItems) ...items.sublist(items.length - maxItems)
    else ...items,
  ];
  return List<ChatFlowItem>.unmodifiable(capped);
}

// ---------------------------------------------------------------------------
// Cubit
// ---------------------------------------------------------------------------

/// Cubit for the [ChatMessageFlow] envelope.
///
/// Holds the pushed flow items (oldest first). Values arrive from pushed FFI
/// events via the shell's mapping layer; the mutation methods below only
/// store, cap, and clear them (D-04).
class ChatMessageFlowCubit extends Cubit<ChatMessageFlowState> {
  /// Creates a [ChatMessageFlowCubit].
  ///
  /// [initialItems] seeds the list (capped the same way as [append]).
  ChatMessageFlowCubit({
    this.instanceId = '',
    List<ChatFlowItem> initialItems = const <ChatFlowItem>[],
  }) : super(ChatMessageFlowState(items: _cappedItems(initialItems)));

  /// Optional instance discriminator (kept for API uniformity with the
  /// scaffold component cubits; unused by the in-memory cubit).
  final String instanceId;

  /// Appends a pushed item, dropping the oldest entries past the cap
  /// (T-01-10-02). Never mutates existing items.
  void append(ChatFlowItem item) {
    emit(
      state.copyWith(
        items: _cappedItems(<ChatFlowItem>[...state.items, item]),
      ),
    );
  }

  /// Applies a pushed full-list snapshot (capped oldest-out, T-01-10-02).
  void replaceAll(List<ChatFlowItem> items) {
    emit(state.copyWith(items: _cappedItems(items)));
  }

  /// Clears the flow to the empty snapshot.
  void clear() {
    emit(const ChatMessageFlowState());
  }
}
