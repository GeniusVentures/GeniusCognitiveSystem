/// ChatMessageMedia -- M3 chat media composite (top-level flow item).
///
/// Generated from chat_message_media.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_media.dart.jinja2
/// Generator version: 0.4.0
/// Top-level flow item (D-20): renders a media payload (image or
/// attachment card) with NO bubble chrome -- the card surface belongs to the
/// scaffold MediaCard atom, never to the bubble composite. The states axis
/// (D-19: pending, streaming, thinking, complete, error) drives item-level chrome via generated
/// per-state helpers selected from a data-driven registry -- never a runtime
/// state switch (D-17). Media metadata is data-only (D-04): C++ pushes the
/// opaque reference and display strings; the retrieval seam ([thumbnail])
/// resolves the reference to an image provider and is deliberately
/// caller-supplied until the media pipeline lands.
/// The C++ half of this composite is the protobuf ChatMessageState generated
/// from src/proto/gcs_chat.proto (D-24/D-26) -- this file is the Dart half.
library;

import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:frontend_scaffold/components/media_card.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import 'chat_message_media_cubit.dart';
import 'chat_message_media_state.dart';

/// Error-tint alpha painted over the card in the error state
/// (UI-SPEC color contract; chat-domain constant, not a scaffold token).
const double kErrorOverlayAlpha = 0.12;

/// Signature of a per-state chrome builder generated from the states axis.
///
/// [item] is the media payload widget (the media card); a state
/// treatment dims it, tints it, or passes it through unchanged.
typedef _ChatMessageMediaStateChromeBuilder = Widget Function(
  BuildContext context,
  Widget item,
);

// ---------------------------------------------------------------------------
// State-chrome registry (D-19 states axis)
// ---------------------------------------------------------------------------

/// State-chrome registry generated from the states axis (D-19).
///
/// Selection is a data-driven map lookup keyed by the pushed state string --
/// never a hand-written switch. Append-only: adding a state to the vars axis
/// regenerates a matching entry (identity chrome until a treatment is
/// authored); existing entries are never renamed or removed.
const Map<String, _ChatMessageMediaStateChromeBuilder> _stateChrome =
    <String, _ChatMessageMediaStateChromeBuilder>{
      'pending': _chromePending,
      'streaming': _chromeStreaming,
      'thinking': _chromeThinking,
      'complete': _chromeComplete,
      'error': _chromeError,
    };

/// Identity chrome: the item unchanged. Also the fallback for state values
/// outside the generated registry (e.g. the proto's MESSAGE_STATE_UNSPECIFIED).
Widget _chromeIdentity(BuildContext context, Widget item) {
  return item;
}

// ---------------------------------------------------------------------------
// Per-state chrome builders (generated from the states axis)
// ---------------------------------------------------------------------------

/// Pending chrome: the whole item at [ScaffoldDimens.disabledOverlayOpacity]
/// (UI-SPEC: the scaffold disabled/dim pattern -- upload/load in flight).
Widget _chromePending(BuildContext context, Widget item) {
  return Opacity(
    opacity: context.dimens.disabledOverlayOpacity,
    child: item,
  );
}

/// Streaming chrome: no additional state treatment (identity).
Widget _chromeStreaming(BuildContext context, Widget item) {
  return _chromeIdentity(context, item);
}

/// Thinking chrome: no additional state treatment (identity).
Widget _chromeThinking(BuildContext context, Widget item) {
  return _chromeIdentity(context, item);
}

/// Complete chrome: no additional state treatment (identity).
Widget _chromeComplete(BuildContext context, Widget item) {
  return _chromeIdentity(context, item);
}

/// Error chrome: status-error tint painted over the card (12% alpha per the
/// UI-SPEC color contract -- failed load/transfer).
Widget _chromeError(BuildContext context, Widget item) {
  return _ErrorOverlay(child: item);
}

// ---------------------------------------------------------------------------
// State helper widgets
// ---------------------------------------------------------------------------

/// Error tint painted over the item (D-19 error state).
class _ErrorOverlay extends StatelessWidget {
  /// Creates the overlay wrapping [child].
  const _ErrorOverlay({required this.child});

  /// The item this tint is painted over.
  final Widget child;

  @override
  Widget build(BuildContext context) {
    final palette = context.palette;
    final dimens = context.dimens;
    return Stack(
      children: <Widget>[
        child,
        Positioned.fill(
          child: DecoratedBox(
            decoration: BoxDecoration(
              color: palette.statusError.withValues(alpha: kErrorOverlayAlpha),
              borderRadius: BorderRadius.circular(dimens.radiusMd),
            ),
          ),
        ),
      ],
    );
  }
}

// ---------------------------------------------------------------------------
// Widget
// ---------------------------------------------------------------------------

/// A chat media card -- a top-level flow item (D-20), never bubble chrome.
///
/// Variant structure is generated from the states axis (D-17/D-19); the
/// state arrives at runtime from pushed FFI events and selects chrome from
/// the generated registry (never a state switch). The media payload
/// is data-only -- this widget renders pushed reference/title snapshots, it
/// never fetches or decodes media content (D-04). [thumbnail] is the
/// retrieval seam: the shell resolves the pushed [mediaRef] to an image
/// provider when the media pipeline lands; while null the atom renders its
/// plain surface placeholder.
class ChatMessageMedia extends StatefulWidget {
  /// Creates a [ChatMessageMedia].
  const ChatMessageMedia({
    this.instanceId = '',
    this.mediaRef = '',
    this.title = '',
    this.thumbnail,
    this.cubit,
    super.key,
  });

  /// Optional instance discriminator forwarded to the Cubit.
  final String instanceId;

  /// Seed media reference (opaque media identifier); runtime updates
  /// flow through [cubit] (pushed FFI events).
  final String mediaRef;

  /// Seed display label rendered in the card's metadata row.
  final String title;

  /// Caller-supplied thumbnail provider -- the retrieval seam resolving
  /// [mediaRef] once the media pipeline exists (D-04: content stays C++'s
  /// concern; Dart renders).
  final ImageProvider? thumbnail;

  /// Optional parent-owned cubit driving this item (an FFI event
  /// subscriber); when null the widget owns an internal cubit seeded from
  /// [mediaRef]/[title].
  final ChatMessageMediaCubit? cubit;

  @override
  State<ChatMessageMedia> createState() =>
      _ChatMessageMediaState();
}

class _ChatMessageMediaState extends State<ChatMessageMedia> {
  late ChatMessageMediaCubit _cubit;
  late bool _ownsCubit;

  @override
  void initState() {
    super.initState();
    _ownsCubit = widget.cubit == null;
    _cubit = widget.cubit ??
        ChatMessageMediaCubit(
          instanceId: widget.instanceId,
          initialMediaRef: widget.mediaRef,
          initialTitle: widget.title,
        );
  }

  @override
  void didUpdateWidget(ChatMessageMedia oldWidget) {
    super.didUpdateWidget(oldWidget);
    // Only the discriminator re-seeds the internal cubit. The payload seeds
    // ([mediaRef]/[title]) deliberately do NOT: runtime metadata arrives via
    // pushed FFI events through the cubit (D-04), and re-seeding on every
    // change would reset the state axis mid-transfer.
    final bool seedChanged = widget.instanceId != oldWidget.instanceId;
    if (widget.cubit != oldWidget.cubit || (_ownsCubit && seedChanged)) {
      if (_ownsCubit) {
        _cubit.close();
      }
      _ownsCubit = widget.cubit == null;
      _cubit = widget.cubit ??
          ChatMessageMediaCubit(
            instanceId: widget.instanceId,
            initialMediaRef: widget.mediaRef,
            initialTitle: widget.title,
          );
    }
  }

  @override
  void dispose() {
    if (_ownsCubit) {
      _cubit.close();
    }
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return BlocProvider<ChatMessageMediaCubit>.value(
      value: _cubit,
      child: BlocBuilder<ChatMessageMediaCubit, ChatMessageMediaState>(
        builder: (context, state) {
          final palette = context.palette;
          final Widget card = MediaCard(
            thumbnail: widget.thumbnail,
            metadataRow: <Widget>[
              if (state.title.isNotEmpty)
                Text(
                  state.title,
                  maxLines: 1,
                  overflow: TextOverflow.ellipsis,
                  style: Theme.of(context)
                      .textTheme
                      .labelMedium
                      ?.copyWith(color: palette.textSecondary),
                ),
            ],
          );
          return (_stateChrome[state.state] ?? _chromeIdentity)(
            context,
            card,
          );
        },
      ),
    );
  }
}
