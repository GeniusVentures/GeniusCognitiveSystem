/// MessageFlowCubit -- center-pane flow state for the GCS chat shell
/// (D-11/D-17).
///
/// Thin state holder (D-04): the state IS the pushed `List<ChatFlowItem>` --
/// items arrive as decoded `ChatMessageState` events dispatched by
/// [SessionCubit] and are mapped here onto the generated D-18/D-19 variant
/// axes. The retained list is capped by the generated flow cubit's shared
/// [ChatMessageFlowCubit.cappedItems] (T-01-11-03 / review IN-05: the cap
/// invariant lives in exactly one place). No local message synthesis beyond
/// wiring the pushed echo.
library;

import 'package:flutter_bloc/flutter_bloc.dart';

import 'package:flutter_app/generated/chat/chat_message_flow.dart';
import 'package:flutter_app/generated/chat/chat_message_flow_cubit.dart';
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

/// Sender short-form truncation widths (D-04): a C++-stamped wallet address
/// longer than prefix + suffix + the '0x' marker + one separator is rendered
/// as '0x' + prefix + '…' + suffix; shorter addresses pass through verbatim.
const int kSenderShortPrefixLength = 8;
const int kSenderShortSuffixLength = 8;

/// Truncates a wallet-address sender to a display short-form (D-04): the
/// leading '0x' plus the first [kSenderShortPrefixLength] and last
/// [kSenderShortSuffixLength] hex characters, joined by '…'. Addresses short
/// enough to read whole are returned unchanged.
String _truncateSender(String address) {
  if (address.length >
      kSenderShortPrefixLength + kSenderShortSuffixLength + 4) {
    return '0x${address.substring(2, 2 + kSenderShortPrefixLength)}…'
        '${address.substring(address.length - kSenderShortSuffixLength)}';
  }
  return address;
}

/// Cubit holding the chat flow items for the active shell (D-11).
class MessageFlowCubit extends Cubit<List<ChatFlowItem>> {
  /// Creates a [MessageFlowCubit] with an empty flow.
  MessageFlowCubit({List<ChatFlowItem> initialItems = const <ChatFlowItem>[]})
    : super(ChatMessageFlowCubit.cappedItems(initialItems));

  /// Appends a pushed item, never mutating existing entries, dropping the
  /// oldest entries past the cap (T-01-11-03).
  void append(ChatFlowItem item) {
    emit(ChatMessageFlowCubit.cappedItems(<ChatFlowItem>[...state, item]));
  }

  /// Replaces an existing entry sharing [item]'s instanceId with [item] in
  /// place (D-07): a pushed pending -> complete/error echo for one message id
  /// must replace, never duplicate. Non-matching entries keep their order;
  /// the result is capped by the generated invariant.
  void upsert(ChatFlowItem item) {
    final List<ChatFlowItem> next = <ChatFlowItem>[
      for (final ChatFlowItem existing in state)
        if (existing.instanceId != item.instanceId) existing,
      item,
    ];
    emit(ChatMessageFlowCubit.cappedItems(next));
  }

  /// Replaces the retained flow with [items] in full (D-06): a pushed
  /// MessageHistory batch is the complete sorted room history, so the list is
  /// replaced, never appended. Capped by the generated invariant.
  void replaceAll(List<ChatFlowItem> items) {
    emit(ChatMessageFlowCubit.cappedItems(items));
  }

  /// Builds a [ChatFlowItemTextBubble] from a decoded [ChatMessageState]:
  /// maps MessageRole -> user_self/user_peer/assistant/system, MessageState
  /// -> state chrome, and carries the C++-stamped id, text, and truncated
  /// sender label (D-04: nothing is synthesized -- every field is the pushed
  /// snapshot).
  ChatFlowItemTextBubble buildChatFlowItemTextBubble(ChatMessageState message) {
    return ChatFlowItemTextBubble(
      instanceId: message.id,
      role: kRoleVariants[message.role] ?? 'system',
      state: kStateVariants[message.state] ?? 'complete',
      text: message.text,
      senderName: message.sender.isEmpty ? null : _truncateSender(message.sender),
    );
  }
}
