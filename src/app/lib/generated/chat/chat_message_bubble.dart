/// ChatMessageBubble -- M3 chat message bubble composite (roles x states).
///
/// Generated from chat_message_bubble.dart.jinja2 -- do not edit by hand.
/// Source schema: templates/components/chat_message_bubble.dart.jinja2
/// Generator version: 0.4.0
/// One variant widget class per role (D-18 roles axis: user_self, user_peer, assistant, system).
/// The states axis (D-19: pending, streaming, thinking, complete, error) drives within-bubble
/// chrome via generated per-state helpers selected from a data-driven
/// registry -- never a runtime role/state enum switch (D-17).
/// text-only payload (D-20); code and media are separate composites
/// in the flow envelope, never wrapped in bubble chrome.
/// The C++ half of this composite is the protobuf ChatMessageState generated
/// from src/proto/gcs_chat.proto (D-24/D-26) -- this file is the Dart half.
library;

import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:frontend_scaffold/components/scaffold_surface.dart';
import 'package:frontend_scaffold/theme/scaffold_colors.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import 'chat_message_bubble_cubit.dart';
import 'chat_message_bubble_state.dart';

/// Error-tint alpha painted over the role fill in the error state
/// (UI-SPEC color contract; chat-domain constant, not a scaffold token).
const double kErrorOverlayAlpha = 0.12;

/// Signature of a per-state chrome builder generated from the states axis.
///
/// [content] is the text payload widget; [wrapSurface] builds the
/// role's bubble surface (fill/border/radius) around a given content widget,
/// so a state can restyle the content, replace it, or wrap the whole surface.
typedef _ChatMessageBubbleStateChromeBuilder = Widget Function(
  BuildContext context,
  Widget content,
  Widget Function(Widget content) wrapSurface,
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
const Map<String, _ChatMessageBubbleStateChromeBuilder> _stateChrome =
    <String, _ChatMessageBubbleStateChromeBuilder>{
      'pending': _chromePending,
      'streaming': _chromeStreaming,
      'thinking': _chromeThinking,
      'complete': _chromeComplete,
      'error': _chromeError,
    };

/// Identity chrome: the raw surface around the content, with no state
/// treatment. Also the fallback for state values outside the generated
/// registry (e.g. the proto's MESSAGE_STATE_UNSPECIFIED).
Widget _chromeIdentity(
  BuildContext context,
  Widget content,
  Widget Function(Widget content) wrapSurface,
) {
  return wrapSurface(content);
}

// ---------------------------------------------------------------------------
// Per-state chrome builders (generated from the states axis)
// ---------------------------------------------------------------------------

/// Pending chrome: the whole bubble at [ScaffoldDimens.disabledOverlayOpacity]
/// (UI-SPEC: role fill at 40% -- the scaffold disabled/dim pattern).
Widget _chromePending(
  BuildContext context,
  Widget content,
  Widget Function(Widget content) wrapSurface,
) {
  return Opacity(
    opacity: context.dimens.disabledOverlayOpacity,
    child: wrapSurface(content),
  );
}

/// Streaming chrome: no additional state treatment (identity).
Widget _chromeStreaming(
  BuildContext context,
  Widget content,
  Widget Function(Widget content) wrapSurface,
) {
  return _chromeIdentity(context, content, wrapSurface);
}

/// Thinking chrome: the pre-generation typing indicator replaces the
/// text content (distinct from streaming -- no tokens yet).
Widget _chromeThinking(
  BuildContext context,
  Widget content,
  Widget Function(Widget content) wrapSurface,
) {
  return wrapSurface(const _ThinkingDots());
}

/// Complete chrome: no additional state treatment (identity).
Widget _chromeComplete(
  BuildContext context,
  Widget content,
  Widget Function(Widget content) wrapSurface,
) {
  return _chromeIdentity(context, content, wrapSurface);
}

/// Error chrome: status-error tint painted over the role fill (12% alpha
/// per the UI-SPEC color contract).
Widget _chromeError(
  BuildContext context,
  Widget content,
  Widget Function(Widget content) wrapSurface,
) {
  return _ErrorOverlay(child: wrapSurface(content));
}

// ---------------------------------------------------------------------------
// State helper widgets
// ---------------------------------------------------------------------------

/// Three-dot typing indicator shown while the assistant is thinking
/// (UI-SPEC: leading dot lightGreenPrimary, then blue500 and
/// lightGreenSecondary; no scaffold typing-indicator atom exists yet).
class _ThinkingDots extends StatelessWidget {
  /// Creates the typing indicator.
  const _ThinkingDots();

  @override
  Widget build(BuildContext context) {
    final palette = context.palette;
    final dimens = context.dimens;
    return Padding(
      padding: EdgeInsets.symmetric(
        horizontal: dimens.space6,
        vertical: dimens.space4,
      ),
      child: Row(
        mainAxisSize: MainAxisSize.min,
        children: <Widget>[
          _BubbleDot(color: palette.lightGreenPrimary),
          SizedBox(width: dimens.space2),
          _BubbleDot(color: palette.blue500),
          SizedBox(width: dimens.space2),
          _BubbleDot(color: palette.lightGreenSecondary),
        ],
      ),
    );
  }
}

/// Single dot of the [_ThinkingDots] indicator (diameter = `space4`).
class _BubbleDot extends StatelessWidget {
  /// Creates a dot with the given [color].
  const _BubbleDot({required this.color});

  /// Fill color of this dot.
  final Color color;

  @override
  Widget build(BuildContext context) {
    final dimens = context.dimens;
    return Container(
      width: dimens.space4,
      height: dimens.space4,
      decoration: BoxDecoration(color: color, shape: BoxShape.circle),
    );
  }
}

/// Error tint painted over the bubble's role fill (D-19 error state).
class _ErrorOverlay extends StatelessWidget {
  /// Creates the overlay wrapping [child].
  const _ErrorOverlay({required this.child});

  /// The bubble surface this tint is painted over.
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
// ChatMessageBubbleUserSelf (user_self)
// ---------------------------------------------------------------------------

/// A chat message bubble variant for user_self -- self-authored messages (trailing alignment, accent fill).
///
/// Variant structure is generated from the roles axis (D-17/D-18); the state
/// axis arrives at runtime from pushed FFI events and selects chrome from the
/// generated registry (never a role/state switch). The text payload
/// is data-only -- this widget renders, it never synthesizes message content
/// (D-04).
class ChatMessageBubbleUserSelf extends StatefulWidget {
  /// Creates a [ChatMessageBubbleUserSelf].
  const ChatMessageBubbleUserSelf({
    this.instanceId = '',
    this.text = '',
    this.senderName,
    this.cubit,
    super.key,
  });

  /// Optional instance discriminator forwarded to the Cubit.
  final String instanceId;

  /// Seed text payload; runtime updates flow through [cubit]
  /// (pushed FFI events).
  final String text;

  /// Optional sender label rendered above the bubble (reserved
  /// for API uniformity across the role variants; this role renders no label
  /// -- alignment and fill already own the bubble visually).
  final String? senderName;

  /// Optional parent-owned cubit driving this bubble (an FFI event
  /// subscriber); when null the widget owns an internal cubit seeded from
  /// [text].
  final ChatMessageBubbleCubit? cubit;

  @override
  State<ChatMessageBubbleUserSelf> createState() =>
      _ChatMessageBubbleUserSelfState();
}

class _ChatMessageBubbleUserSelfState
    extends State<ChatMessageBubbleUserSelf> {
  late ChatMessageBubbleCubit _cubit;
  late bool _ownsCubit;

  @override
  void initState() {
    super.initState();
    _ownsCubit = widget.cubit == null;
    _cubit = widget.cubit ??
        ChatMessageBubbleCubit(
          instanceId: widget.instanceId,
          role: 'user_self',
          initialText: widget.text,
        );
  }

  @override
  void didUpdateWidget(ChatMessageBubbleUserSelf oldWidget) {
    super.didUpdateWidget(oldWidget);
    // Only the discriminator re-seeds the internal cubit. The payload seed
    // ([text]) deliberately does NOT: runtime text arrives via pushed FFI
    // events through the cubit (D-04), and re-seeding on every text change
    // would reset the state axis mid-stream.
    final bool seedChanged = widget.instanceId != oldWidget.instanceId;
    if (widget.cubit != oldWidget.cubit || (_ownsCubit && seedChanged)) {
      if (_ownsCubit) {
        _cubit.close();
      }
      _ownsCubit = widget.cubit == null;
      _cubit = widget.cubit ??
          ChatMessageBubbleCubit(
            instanceId: widget.instanceId,
            role: 'user_self',
            initialText: widget.text,
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
    return BlocProvider<ChatMessageBubbleCubit>.value(
      value: _cubit,
      child: BlocBuilder<ChatMessageBubbleCubit, ChatMessageBubbleState>(
        builder: (context, state) {
          final palette = context.palette;
          final dimens = context.dimens;

          Widget wrapSurface(Widget content) {
            return ScaffoldSurface(
              color: palette.lightGreenPrimary,
              border: null,
              elevation: 0,
              borderRadius: BorderRadius.circular(dimens.radiusMd),
              child: content,
            );
          }

          final Widget textContent = Padding(
            padding: EdgeInsets.symmetric(
              horizontal: dimens.space6,
              vertical: dimens.space4,
            ),
            child: Text(
              state.text,
              textAlign: null,
              style: Theme.of(context)
                  .textTheme
                  .bodyMedium
                  ?.copyWith(color: ScaffoldColors.btnText),
            ),
          );

          final Widget bubble = (_stateChrome[state.state] ?? _chromeIdentity)(
            context,
            textContent,
            wrapSurface,
          );

          return Align(
            alignment: AlignmentDirectional.centerEnd,
            child: bubble,
          );
        },
      ),
    );
  }
}

// ---------------------------------------------------------------------------
// ChatMessageBubbleUserPeer (user_peer)
// ---------------------------------------------------------------------------

/// A chat message bubble variant for user_peer -- peer user messages (leading alignment, secondary fill).
///
/// Variant structure is generated from the roles axis (D-17/D-18); the state
/// axis arrives at runtime from pushed FFI events and selects chrome from the
/// generated registry (never a role/state switch). The text payload
/// is data-only -- this widget renders, it never synthesizes message content
/// (D-04).
class ChatMessageBubbleUserPeer extends StatefulWidget {
  /// Creates a [ChatMessageBubbleUserPeer].
  const ChatMessageBubbleUserPeer({
    this.instanceId = '',
    this.text = '',
    this.senderName,
    this.cubit,
    super.key,
  });

  /// Optional instance discriminator forwarded to the Cubit.
  final String instanceId;

  /// Seed text payload; runtime updates flow through [cubit]
  /// (pushed FFI events).
  final String text;

  /// Optional sender label rendered above the bubble.
  final String? senderName;

  /// Optional parent-owned cubit driving this bubble (an FFI event
  /// subscriber); when null the widget owns an internal cubit seeded from
  /// [text].
  final ChatMessageBubbleCubit? cubit;

  @override
  State<ChatMessageBubbleUserPeer> createState() =>
      _ChatMessageBubbleUserPeerState();
}

class _ChatMessageBubbleUserPeerState
    extends State<ChatMessageBubbleUserPeer> {
  late ChatMessageBubbleCubit _cubit;
  late bool _ownsCubit;

  @override
  void initState() {
    super.initState();
    _ownsCubit = widget.cubit == null;
    _cubit = widget.cubit ??
        ChatMessageBubbleCubit(
          instanceId: widget.instanceId,
          role: 'user_peer',
          initialText: widget.text,
        );
  }

  @override
  void didUpdateWidget(ChatMessageBubbleUserPeer oldWidget) {
    super.didUpdateWidget(oldWidget);
    // Only the discriminator re-seeds the internal cubit. The payload seed
    // ([text]) deliberately does NOT: runtime text arrives via pushed FFI
    // events through the cubit (D-04), and re-seeding on every text change
    // would reset the state axis mid-stream.
    final bool seedChanged = widget.instanceId != oldWidget.instanceId;
    if (widget.cubit != oldWidget.cubit || (_ownsCubit && seedChanged)) {
      if (_ownsCubit) {
        _cubit.close();
      }
      _ownsCubit = widget.cubit == null;
      _cubit = widget.cubit ??
          ChatMessageBubbleCubit(
            instanceId: widget.instanceId,
            role: 'user_peer',
            initialText: widget.text,
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
    return BlocProvider<ChatMessageBubbleCubit>.value(
      value: _cubit,
      child: BlocBuilder<ChatMessageBubbleCubit, ChatMessageBubbleState>(
        builder: (context, state) {
          final palette = context.palette;
          final dimens = context.dimens;

          Widget wrapSurface(Widget content) {
            return ScaffoldSurface(
              color: palette.deepBlueTertiary,
              border: null,
              elevation: 0,
              borderRadius: BorderRadius.circular(dimens.radiusMd),
              child: content,
            );
          }

          final Widget textContent = Padding(
            padding: EdgeInsets.symmetric(
              horizontal: dimens.space6,
              vertical: dimens.space4,
            ),
            child: Text(
              state.text,
              textAlign: null,
              style: Theme.of(context)
                  .textTheme
                  .bodyMedium
                  ?.copyWith(color: palette.textPrimary),
            ),
          );

          final Widget bubble = (_stateChrome[state.state] ?? _chromeIdentity)(
            context,
            textContent,
            wrapSurface,
          );

          final Widget labeledBubble = Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              if (widget.senderName != null)
                Padding(
                  padding: EdgeInsets.only(bottom: dimens.space2),
                  child: Text(
                    widget.senderName!,
                    style: Theme.of(context).textTheme.labelSmall?.copyWith(
                          color: palette.textSecondary,
                        ),
                  ),
                ),
              bubble,
            ],
          );

          return Align(
            alignment: AlignmentDirectional.centerStart,
            child: labeledBubble,
          );
        },
      ),
    );
  }
}

// ---------------------------------------------------------------------------
// ChatMessageBubbleAssistant (assistant)
// ---------------------------------------------------------------------------

/// A chat message bubble variant for assistant -- GCS bot responses (leading alignment, secondary fill).
///
/// Variant structure is generated from the roles axis (D-17/D-18); the state
/// axis arrives at runtime from pushed FFI events and selects chrome from the
/// generated registry (never a role/state switch). The text payload
/// is data-only -- this widget renders, it never synthesizes message content
/// (D-04).
class ChatMessageBubbleAssistant extends StatefulWidget {
  /// Creates a [ChatMessageBubbleAssistant].
  const ChatMessageBubbleAssistant({
    this.instanceId = '',
    this.text = '',
    this.senderName,
    this.cubit,
    super.key,
  });

  /// Optional instance discriminator forwarded to the Cubit.
  final String instanceId;

  /// Seed text payload; runtime updates flow through [cubit]
  /// (pushed FFI events).
  final String text;

  /// Optional sender label rendered above the bubble.
  final String? senderName;

  /// Optional parent-owned cubit driving this bubble (an FFI event
  /// subscriber); when null the widget owns an internal cubit seeded from
  /// [text].
  final ChatMessageBubbleCubit? cubit;

  @override
  State<ChatMessageBubbleAssistant> createState() =>
      _ChatMessageBubbleAssistantState();
}

class _ChatMessageBubbleAssistantState
    extends State<ChatMessageBubbleAssistant> {
  late ChatMessageBubbleCubit _cubit;
  late bool _ownsCubit;

  @override
  void initState() {
    super.initState();
    _ownsCubit = widget.cubit == null;
    _cubit = widget.cubit ??
        ChatMessageBubbleCubit(
          instanceId: widget.instanceId,
          role: 'assistant',
          initialText: widget.text,
        );
  }

  @override
  void didUpdateWidget(ChatMessageBubbleAssistant oldWidget) {
    super.didUpdateWidget(oldWidget);
    // Only the discriminator re-seeds the internal cubit. The payload seed
    // ([text]) deliberately does NOT: runtime text arrives via pushed FFI
    // events through the cubit (D-04), and re-seeding on every text change
    // would reset the state axis mid-stream.
    final bool seedChanged = widget.instanceId != oldWidget.instanceId;
    if (widget.cubit != oldWidget.cubit || (_ownsCubit && seedChanged)) {
      if (_ownsCubit) {
        _cubit.close();
      }
      _ownsCubit = widget.cubit == null;
      _cubit = widget.cubit ??
          ChatMessageBubbleCubit(
            instanceId: widget.instanceId,
            role: 'assistant',
            initialText: widget.text,
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
    return BlocProvider<ChatMessageBubbleCubit>.value(
      value: _cubit,
      child: BlocBuilder<ChatMessageBubbleCubit, ChatMessageBubbleState>(
        builder: (context, state) {
          final palette = context.palette;
          final dimens = context.dimens;

          Widget wrapSurface(Widget content) {
            return ScaffoldSurface(
              color: palette.deepBlueTertiary,
              border: null,
              elevation: 0,
              borderRadius: BorderRadius.circular(dimens.radiusMd),
              child: content,
            );
          }

          final Widget textContent = Padding(
            padding: EdgeInsets.symmetric(
              horizontal: dimens.space6,
              vertical: dimens.space4,
            ),
            child: Text(
              state.text,
              textAlign: null,
              style: Theme.of(context)
                  .textTheme
                  .bodyMedium
                  ?.copyWith(color: palette.textPrimary),
            ),
          );

          final Widget bubble = (_stateChrome[state.state] ?? _chromeIdentity)(
            context,
            textContent,
            wrapSurface,
          );

          final Widget labeledBubble = Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              if (widget.senderName != null)
                Padding(
                  padding: EdgeInsets.only(bottom: dimens.space2),
                  child: Text(
                    widget.senderName!,
                    style: Theme.of(context).textTheme.labelSmall?.copyWith(
                          color: palette.textSecondary,
                        ),
                  ),
                ),
              bubble,
            ],
          );

          return Align(
            alignment: AlignmentDirectional.centerStart,
            child: labeledBubble,
          );
        },
      ),
    );
  }
}

// ---------------------------------------------------------------------------
// ChatMessageBubbleSystem (system)
// ---------------------------------------------------------------------------

/// A chat message bubble variant for system -- join/leave/moderation notices (centered, transparent fill with a subtle outline).
///
/// Variant structure is generated from the roles axis (D-17/D-18); the state
/// axis arrives at runtime from pushed FFI events and selects chrome from the
/// generated registry (never a role/state switch). The text payload
/// is data-only -- this widget renders, it never synthesizes message content
/// (D-04).
class ChatMessageBubbleSystem extends StatefulWidget {
  /// Creates a [ChatMessageBubbleSystem].
  const ChatMessageBubbleSystem({
    this.instanceId = '',
    this.text = '',
    this.senderName,
    this.cubit,
    super.key,
  });

  /// Optional instance discriminator forwarded to the Cubit.
  final String instanceId;

  /// Seed text payload; runtime updates flow through [cubit]
  /// (pushed FFI events).
  final String text;

  /// Optional sender label rendered above the bubble (reserved
  /// for API uniformity across the role variants; this role renders no label
  /// -- alignment and fill already own the bubble visually).
  final String? senderName;

  /// Optional parent-owned cubit driving this bubble (an FFI event
  /// subscriber); when null the widget owns an internal cubit seeded from
  /// [text].
  final ChatMessageBubbleCubit? cubit;

  @override
  State<ChatMessageBubbleSystem> createState() =>
      _ChatMessageBubbleSystemState();
}

class _ChatMessageBubbleSystemState
    extends State<ChatMessageBubbleSystem> {
  late ChatMessageBubbleCubit _cubit;
  late bool _ownsCubit;

  @override
  void initState() {
    super.initState();
    _ownsCubit = widget.cubit == null;
    _cubit = widget.cubit ??
        ChatMessageBubbleCubit(
          instanceId: widget.instanceId,
          role: 'system',
          initialText: widget.text,
        );
  }

  @override
  void didUpdateWidget(ChatMessageBubbleSystem oldWidget) {
    super.didUpdateWidget(oldWidget);
    // Only the discriminator re-seeds the internal cubit. The payload seed
    // ([text]) deliberately does NOT: runtime text arrives via pushed FFI
    // events through the cubit (D-04), and re-seeding on every text change
    // would reset the state axis mid-stream.
    final bool seedChanged = widget.instanceId != oldWidget.instanceId;
    if (widget.cubit != oldWidget.cubit || (_ownsCubit && seedChanged)) {
      if (_ownsCubit) {
        _cubit.close();
      }
      _ownsCubit = widget.cubit == null;
      _cubit = widget.cubit ??
          ChatMessageBubbleCubit(
            instanceId: widget.instanceId,
            role: 'system',
            initialText: widget.text,
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
    return BlocProvider<ChatMessageBubbleCubit>.value(
      value: _cubit,
      child: BlocBuilder<ChatMessageBubbleCubit, ChatMessageBubbleState>(
        builder: (context, state) {
          final palette = context.palette;
          final dimens = context.dimens;

          Widget wrapSurface(Widget content) {
            return ScaffoldSurface(
              color: Colors.transparent,
              border: Border.all(color: palette.borderSubtle, width: 1),
              elevation: 0,
              borderRadius: BorderRadius.circular(dimens.radiusMd),
              child: content,
            );
          }

          final Widget textContent = Padding(
            padding: EdgeInsets.symmetric(
              horizontal: dimens.space6,
              vertical: dimens.space4,
            ),
            child: Text(
              state.text,
              textAlign: TextAlign.center,
              style: Theme.of(context)
                  .textTheme
                  .bodyMedium
                  ?.copyWith(color: palette.textSecondary),
            ),
          );

          final Widget bubble = (_stateChrome[state.state] ?? _chromeIdentity)(
            context,
            textContent,
            wrapSurface,
          );

          return Align(
            alignment: AlignmentDirectional.center,
            child: bubble,
          );
        },
      ),
    );
  }
}
