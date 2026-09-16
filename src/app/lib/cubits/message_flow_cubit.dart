/// MessageFlowCubit -- center-pane flow state for the GCS chat shell
/// (D-11/D-17).
///
/// Thin state holder (D-04): the state IS the pushed `List<ChatFlowItem>` --
/// items arrive as decoded `ChatMessageState` events dispatched by
/// [SessionCubit] and are mapped here onto the generated D-18/D-19 variant
/// axes. The retained list is capped, dropping the OLDEST entries past
/// [ChatMessageFlowState.kMaxFlowItems] (T-01-11-03, sharing the cap constant
/// the generated flow composite established as T-01-10-02). No local message
/// synthesis beyond wiring the pushed echo.
library;

import 'package:flutter_bloc/flutter_bloc.dart';

import 'package:flutter_app/generated/chat/chat_message_flow.dart';
import 'package:flutter_app/generated/chat/chat_message_flow_state.dart';
import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';

/// MessageRole (proto) -> D-18 role-axis variant string. Unspecified roles
/// degrade to the system variant (the generated flow registry's fallback).
const Map<MessageRole, String> kRoleVariants = <MessageRole, String>{
  MessageRole.MESSAGE_ROLE_USER_SELF: 'user_self',
  MessageRole.MESSAGE_ROLE_USER_PEER: 'user_peer',
  MessageRole.MESSAGE_ROLE_ASSISTANT: 'assistant',
  MessageRole.MESSAGE_ROLE_SYSTEM: 'system',
};

/// MessageState (proto) -> D-19 state-axis chrome string. Unspecified states
/// render with identity chrome ('complete' -- the generated registry's
/// fallback for unknown values).
const Map<MessageState, String> kStateVariants = <MessageState, String>{
  MessageState.MESSAGE_STATE_PENDING: 'pending',
  MessageState.MESSAGE_STATE_STREAMING: 'streaming',
  MessageState.MESSAGE_STATE_THINKING: 'thinking',
  MessageState.MESSAGE_STATE_COMPLETE: 'complete',
  MessageState.MESSAGE_STATE_ERROR: 'error',
};

/// Cubit holding the chat flow items for the active shell (D-11).
class MessageFlowCubit extends Cubit<List<ChatFlowItem>> {
  /// Creates a [MessageFlowCubit] with an empty flow.
  MessageFlowCubit({List<ChatFlowItem> initialItems = const <ChatFlowItem>[]})
    : super(_capped(initialItems));

  /// Appends a pushed item, never mutating existing entries, dropping the
  /// oldest entries past the cap (T-01-11-03).
  void append(ChatFlowItem item) {
    emit(_capped(<ChatFlowItem>[...state, item]));
  }

  /// Builds a [ChatFlowItemTextBubble] from a decoded [ChatMessageState]:
  /// maps MessageRole -> user_self/user_peer/assistant/system, MessageState
  /// -> state chrome, and carries the C++-stamped id and text (D-04: nothing
  /// is synthesized -- every field is the pushed snapshot).
  ChatFlowItemTextBubble buildChatFlowItemTextBubble(ChatMessageState message) {
    return ChatFlowItemTextBubble(
      instanceId: message.id,
      role: kRoleVariants[message.role] ?? 'system',
      state: kStateVariants[message.state] ?? 'complete',
      text: message.text,
    );
  }

  /// Returns [items] capped to [ChatMessageFlowState.kMaxFlowItems], dropping
  /// the OLDEST entries, as an unmodifiable list (immutable-state contract).
  static List<ChatFlowItem> _capped(List<ChatFlowItem> items) {
    const int maxItems = ChatMessageFlowState.kMaxFlowItems;
    final List<ChatFlowItem> capped = <ChatFlowItem>[
      if (items.length > maxItems)
        ...items.sublist(items.length - maxItems)
      else
        ...items,
    ];
    return List<ChatFlowItem>.unmodifiable(capped);
  }
}
