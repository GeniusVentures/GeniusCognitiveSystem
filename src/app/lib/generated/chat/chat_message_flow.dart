/// ChatMessageFlow -- M3 chat message-flow envelope (D-20 interleave).
///
/// Generated from chat_message_flow.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_flow.dart.jinja2
/// Generator version: 0.4.0
/// Item-type dispatch axis (D-20): text_bubble, code_block, media. The envelope
/// is a Dart 3 sealed hierarchy -- [ChatFlowItem] with one final subclass per
/// item type -- so per-item rendering is an exhaustive compile-time switch
/// over the sealed subclasses: a new item type without a render case is a
/// compile error, never a silent miss (T-01-10-01). Append-only: item types
/// are added to, never removed/reordered/renamed (D-17). The flow renders
/// PUSHED state (D-04): items are immutable snapshots mapped from FFI events
/// upstream; this widget never synthesizes message content. Anchored to the
/// bottom per the UI-SPEC message-flow contract (reverse-axis CustomScrollView:
/// the latest item stays visible without scrolling).
/// The C++ half of this composite is the protobuf ChatMessageState generated
/// from src/proto/gcs_chat.proto (D-24/D-26) -- this file is the Dart half.
library;

import 'package:flutter/material.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import 'chat_message_bubble.dart';
import 'chat_message_bubble_cubit.dart';
import 'chat_message_bubble_state.dart';
import 'chat_message_code_block.dart';
import 'chat_message_code_block_cubit.dart';
import 'chat_message_media.dart';
import 'chat_message_media_cubit.dart';

// ---------------------------------------------------------------------------
// Sealed item hierarchy (D-20 item-type dispatch axis)
// ---------------------------------------------------------------------------

/// A single item in the chat message flow -- the sealed base of the D-20
/// item-type axis.
///
/// One `final class` subclass is generated per item type in the vars axis;
/// Dart 3 sealedness makes every downstream switch over [ChatFlowItem]
/// exhaustive at compile time. [instanceId] discriminates the rendered
/// composite instance; [state] carries the D-19 state-axis snapshot seeding
/// that composite's chrome (mirroring MessageState in
/// src/proto/gcs_chat.proto, append-only).
sealed class ChatFlowItem {
  /// Creates a [ChatFlowItem].
  const ChatFlowItem({this.instanceId = '', this.state = 'complete'});

  /// Optional instance discriminator forwarded to the rendered composite.
  final String instanceId;

  /// Runtime state-axis value (D-19) seeding the rendered composite's chrome.
  final String state;
}

/// A text-bubble flow item -- the D-20 text-only payload (role + state + text).
///
/// [role] is the structural D-18 axis value selecting the generated bubble
/// variant class; it mirrors [ChatMessageBubbleState.kValidRoles] and is
/// validated against it by the render-time coverage check below.
final class ChatFlowItemTextBubble extends ChatFlowItem {
  /// Creates a [ChatFlowItemTextBubble].
  const ChatFlowItemTextBubble({
    super.instanceId,
    super.state,
    required this.role,
    this.text = '',
    this.senderName,
  });

  /// Structural role-axis value (D-18) selecting the bubble variant class.
  final String role;

  /// Text payload (D-20: text-only bubble).
  final String text;

  /// Optional sender label rendered above the bubble by the peer variants.
  final String? senderName;
}

/// A code-block flow item -- the D-20 code payload, never bubble chrome.
///
/// The fields are pushed snapshots (D-04): C++ owns the authoritative code
/// text; Dart renders it via the code-block composite.
final class ChatFlowItemCodeBlock extends ChatFlowItem {
  /// Creates a [ChatFlowItemCodeBlock].
  const ChatFlowItemCodeBlock({
    super.instanceId,
    super.state,
    this.code = '',
    this.language = '',
    this.filename = '',
  });

  /// Code payload snapshot (D-20).
  final String code;

  /// Optional language tag for the code atom header (empty when unknown).
  final String language;

  /// Optional filename for the code atom header (empty when unknown).
  final String filename;
}

/// A media flow item -- the D-20 media payload (opaque reference + label).
///
/// Content resolution is a caller-supplied seam (01-09): the reference and
/// display strings are pushed data (D-04); the widget layer never fetches or
/// decodes media content.
final class ChatFlowItemMedia extends ChatFlowItem {
  /// Creates a [ChatFlowItemMedia].
  const ChatFlowItemMedia({
    super.instanceId,
    super.state,
    this.mediaRef = '',
    this.title = '',
  });

  /// Opaque media reference pushed from C++ (D-04).
  final String mediaRef;

  /// Display label rendered in the media card metadata row.
  final String title;
}

// ---------------------------------------------------------------------------
// Bubble variant registry (D-18 roles axis -> generated variant classes)
// ---------------------------------------------------------------------------

/// Signature of a bubble-variant builder generated from the roles axis.
typedef _ChatFlowBubbleVariantBuilder = Widget Function(
  BuildContext context,
  ChatFlowItemTextBubble item,
);

/// Role -> bubble-variant registry, generated from the D-18 roles axis.
///
/// Selection is a data-driven map lookup keyed by the item's pushed role
/// string -- the same registry pattern as the composites' state-chrome maps,
/// never a hand-written switch. A role outside the registry (e.g. a newer
/// C++ push than this generated build) degrades to the system notice variant
/// in release builds; debug builds surface it via the cubit's axis assert.
const Map<String, _ChatFlowBubbleVariantBuilder> _bubbleVariants =
    <String, _ChatFlowBubbleVariantBuilder>{
      'assistant': _bubbleAssistant,
      'system': _bubbleSystem,
      'user_peer': _bubbleUserPeer,
      'user_self': _bubbleUserSelf,
    };

/// Seeds a bubble cubit from a pushed item snapshot (D-04): the state axis
/// and text payload ARE the pushed data; this never synthesizes content.
ChatMessageBubbleCubit _bubbleCubit(ChatFlowItemTextBubble item) {
  return ChatMessageBubbleCubit(
    instanceId: item.instanceId,
    role: item.role,
    initialMessageState: item.state,
    initialText: item.text,
  );
}

/// Builds the [ChatMessageBubbleAssistant] variant for a pushed [ChatFlowItemTextBubble].
Widget _bubbleAssistant(
  BuildContext context,
  ChatFlowItemTextBubble item,
) {
  return ChatMessageBubbleAssistant(
    instanceId: item.instanceId,
    text: item.text,
    senderName: item.senderName,
    cubit: _bubbleCubit(item),
  );
}

/// Builds the [ChatMessageBubbleSystem] variant for a pushed [ChatFlowItemTextBubble].
Widget _bubbleSystem(
  BuildContext context,
  ChatFlowItemTextBubble item,
) {
  return ChatMessageBubbleSystem(
    instanceId: item.instanceId,
    text: item.text,
    senderName: item.senderName,
    cubit: _bubbleCubit(item),
  );
}

/// Builds the [ChatMessageBubbleUserPeer] variant for a pushed [ChatFlowItemTextBubble].
Widget _bubbleUserPeer(
  BuildContext context,
  ChatFlowItemTextBubble item,
) {
  return ChatMessageBubbleUserPeer(
    instanceId: item.instanceId,
    text: item.text,
    senderName: item.senderName,
    cubit: _bubbleCubit(item),
  );
}

/// Builds the [ChatMessageBubbleUserSelf] variant for a pushed [ChatFlowItemTextBubble].
Widget _bubbleUserSelf(
  BuildContext context,
  ChatFlowItemTextBubble item,
) {
  return ChatMessageBubbleUserSelf(
    instanceId: item.instanceId,
    text: item.text,
    senderName: item.senderName,
    cubit: _bubbleCubit(item),
  );
}

/// Asserts the registry covers every role on the generated bubble's axis
/// snapshot (T-01-10-01 for the role axis: a bubble roles-axis addition
/// without a matching flow registry entry is an error, not a silent miss).
void _debugCheckBubbleVariantCoverage() {
  assert(
    ChatMessageBubbleState.kValidRoles.every(_bubbleVariants.containsKey),
    'chat_message_flow bubble-variant registry is missing a role from '
    'ChatMessageBubbleState.kValidRoles (D-18 append-only: extend '
    '_bubbleVariants and regenerate)',
  );
}

// ---------------------------------------------------------------------------
// Item rendering (exhaustive sealed dispatch, T-01-10-01)
// ---------------------------------------------------------------------------

/// Renders one flow item via the exhaustive sealed-subclass switch.
///
/// The switch carries one case per generated [ChatFlowItem] subclass and NO
/// default: adding an item type to the vars axis without a render case below
/// is a compile error (non-exhaustive switch), never a silent miss. Payload
/// seeds are passed alongside a parent-owned cubit seeded from the same
/// snapshot so the item's D-19 state value drives the composite's chrome
/// registry (the composites' internal-cubit path is for standalone use).
Widget _buildFlowItem(BuildContext context, ChatFlowItem item) {
  return switch (item) {
    ChatFlowItemTextBubble() =>
        (_bubbleVariants[item.role] ?? _bubbleSystem)(context, item),
    ChatFlowItemCodeBlock() => ChatMessageCodeBlock(
        instanceId: item.instanceId,
        code: item.code,
        language: item.language.isEmpty ? null : item.language,
        filename: item.filename.isEmpty ? null : item.filename,
        cubit: ChatMessageCodeBlockCubit(
          instanceId: item.instanceId,
          initialMessageState: item.state,
          initialCode: item.code,
          initialLanguage: item.language,
          initialFilename: item.filename,
        ),
      ),
    ChatFlowItemMedia() => ChatMessageMedia(
        instanceId: item.instanceId,
        mediaRef: item.mediaRef,
        title: item.title,
        cubit: ChatMessageMediaCubit(
          instanceId: item.instanceId,
          initialMessageState: item.state,
          initialMediaRef: item.mediaRef,
          initialTitle: item.title,
        ),
      ),
  };
}

// ---------------------------------------------------------------------------
// Widget
// ---------------------------------------------------------------------------

/// The chat message flow -- an interleave-capable list of heterogeneous D-20
/// items rendered through the exhaustive sealed dispatch above.
///
/// The flow is a reverse-axis [CustomScrollView] anchored to the bottom per
/// the UI-SPEC message-flow contract: growth offset 0 is the visual bottom,
/// so the latest item is visible without scrolling and appended items arrive
/// bottom-anchored (the scroll-to-bottom affordance is a shell contract line,
/// not this composite's). [items] are pushed snapshots, oldest first (D-04):
/// this widget renders what it is given and holds no message state of its
/// own. Horizontal inset uses the phone-baseline token; the desktop-wide
/// inset refinement is a shell/template iteration.
class ChatMessageFlow extends StatefulWidget {
  /// Creates a [ChatMessageFlow].
  const ChatMessageFlow({required this.items, super.key});

  /// The pushed flow items (oldest first); rendered newest at the bottom.
  final List<ChatFlowItem> items;

  @override
  State<ChatMessageFlow> createState() => _ChatMessageFlowState();
}

class _ChatMessageFlowState extends State<ChatMessageFlow> {
  @override
  Widget build(BuildContext context) {
    _debugCheckBubbleVariantCoverage();
    final dimens = context.dimens;
    final int itemCount = widget.items.length;
    return CustomScrollView(
      // Reverse axis anchors the flow to the bottom (UI-SPEC message-flow
      // contract): the latest item stays visible without scrolling.
      reverse: true,
      slivers: <Widget>[
        SliverPadding(
          padding: EdgeInsets.symmetric(horizontal: dimens.space8),
          sliver: SliverList(
            delegate: SliverChildBuilderDelegate(
              (BuildContext context, int index) {
                // Reversed axis: index 0 renders at the visual bottom, so
                // walk the pushed list newest-first.
                final ChatFlowItem item =
                    widget.items[itemCount - 1 - index];
                return KeyedSubtree(
                  key: ValueKey<ChatFlowItem>(item),
                  child: Padding(
                    // Uniform inter-item gap (UI-SPEC: space8 between runs).
                    padding: EdgeInsets.only(top: dimens.space8),
                    child: _buildFlowItem(context, item),
                  ),
                );
              },
              childCount: itemCount,
            ),
          ),
        ),
      ],
    );
  }
}
