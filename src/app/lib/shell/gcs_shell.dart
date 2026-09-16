/// GCSChat -- the GCS chat app shell (D-11/D-23).
///
/// The shell is a three-region layout: left [RoomRail] (D-21), center
/// [ChatMessageFlow] bound to [MessageFlowCubit]'s pushed
/// `List<ChatFlowItem>`, and a bottom composer row inside the center column
/// (the composer never sits under the rail). Phase 1 uses a plain
/// `Row` + `Expanded` center region -- no hand-rolled responsive
/// breakpoints. The shell synthesizes no message content (D-04): everything
/// rendered arrives as pushed FFI events routed through the cubits below.
library;

import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:frontend_scaffold/components/scaffold_composer.dart';
import 'package:frontend_scaffold/components/scaffold_pressable.dart';
import 'package:frontend_scaffold/components/scaffold_surface.dart';
import 'package:frontend_scaffold/theme/scaffold_colors.dart';
import 'package:frontend_scaffold/theme/scaffold_dimens.dart';
import 'package:frontend_scaffold/theme/scaffold_palette.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import '../cubits/composer_cubit.dart';
import '../cubits/message_flow_cubit.dart';
import '../cubits/rail_cubit.dart';
import '../cubits/session_cubit.dart';
import '../generated/chat/chat_message_flow.dart';
import 'room_rail.dart';

/// The app root shell (D-23): rail + flow + composer, themed by the host
/// `MaterialApp` (D-12/D-22 -- see `lib/theme/gcs_theme.dart`).
///
/// Cubits may be injected (tests drive the shell with fakes, no FFI); every
/// null cubit is created and owned by the shell and disposed with it. The
/// default [SessionCubit] opens the real FFI session when the environment
/// provides the shared library, otherwise the shell renders inert (composer
/// disabled until readiness is pushed).
class GCSChat extends StatefulWidget {
  /// Creates a [GCSChat].
  const GCSChat({
    super.key,
    this.sessionCubit,
    this.railCubit,
    this.messageFlowCubit,
    this.composerCubit,
  });

  /// Session cubit owning the FFI handle lifecycle (D-05/D-26); null -> the
  /// shell creates one via [SessionCubit.openDefault].
  final SessionCubit? sessionCubit;

  /// Room rail cubit; null -> the shell owns one.
  final RailCubit? railCubit;

  /// Message flow cubit; null -> the shell owns one.
  final MessageFlowCubit? messageFlowCubit;

  /// Composer cubit; null -> the shell owns one wired to the session
  /// transport and the rail selection.
  final ComposerCubit? composerCubit;

  @override
  State<GCSChat> createState() => _GCSChatState();
}

class _GCSChatState extends State<GCSChat> {
  late final SessionCubit _sessionCubit;
  late final RailCubit _railCubit;
  late final MessageFlowCubit _messageFlowCubit;
  late final ComposerCubit _composerCubit;
  late final bool _ownsSessionCubit;
  late final bool _ownsRailCubit;
  late final bool _ownsMessageFlowCubit;
  late final bool _ownsComposerCubit;

  @override
  void initState() {
    super.initState();
    _ownsSessionCubit = widget.sessionCubit == null;
    _ownsRailCubit = widget.railCubit == null;
    _ownsMessageFlowCubit = widget.messageFlowCubit == null;
    _ownsComposerCubit = widget.composerCubit == null;
    _railCubit = widget.railCubit ?? RailCubit();
    _messageFlowCubit = widget.messageFlowCubit ?? MessageFlowCubit();
    _sessionCubit =
        widget.sessionCubit ??
        SessionCubit.openDefault(
          railCubit: _railCubit,
          messageFlowCubit: _messageFlowCubit,
        );
    _composerCubit =
        widget.composerCubit ??
        ComposerCubit(transport: _sessionCubit, railCubit: _railCubit);
    _sessionCubit.start();
  }

  @override
  void dispose() {
    // Tear down only what the shell created; injected cubits (tests) outlive
    // the shell. Order: composer (uses the session transport) -> session
    // (closes the ReceivePort BEFORE the native shutdown) -> flow -> rail.
    if (_ownsComposerCubit) {
      _composerCubit.close();
    }
    if (_ownsSessionCubit) {
      _sessionCubit.close();
    }
    if (_ownsMessageFlowCubit) {
      _messageFlowCubit.close();
    }
    if (_ownsRailCubit) {
      _railCubit.close();
    }
    super.dispose();
  }

  @override
  Widget build(BuildContext context) {
    return MultiBlocProvider(
      providers: [
        BlocProvider<SessionCubit>.value(value: _sessionCubit),
        BlocProvider<RailCubit>.value(value: _railCubit),
        BlocProvider<MessageFlowCubit>.value(value: _messageFlowCubit),
        BlocProvider<ComposerCubit>.value(value: _composerCubit),
      ],
      child: Scaffold(
        body: Row(
          crossAxisAlignment: CrossAxisAlignment.stretch,
          children: <Widget>[
            const SizedBox(width: RoomRail.kWidth, child: RoomRail()),
            VerticalDivider(
              width: 1,
              thickness: 1,
              color: context.palette.borderSubtle,
            ),
            Expanded(
              child: Column(
                children: <Widget>[
                  Expanded(
                    child: BlocBuilder<MessageFlowCubit, List<ChatFlowItem>>(
                      builder:
                          (BuildContext context, List<ChatFlowItem> items) {
                            return ChatMessageFlow(items: items);
                          },
                    ),
                  ),
                  const _ComposerBar(),
                ],
              ),
            ),
          ],
        ),
      ),
    );
  }
}

/// The bottom composer row of the center column: the scaffold composer
/// surface with its send affordance (UI-SPEC composer contract lines).
///
/// The composer is disabled until the session readiness flag is pushed
/// (D-05); submission forwards the raw string into [ComposerCubit], which
/// publishes it as a codec-tagged `GcsCommand` (D-27/D-29).
class _ComposerBar extends StatelessWidget {
  /// Creates the composer bar.
  const _ComposerBar();

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return BlocBuilder<SessionCubit, SessionState>(
      builder: (BuildContext context, SessionState session) {
        return BlocBuilder<ComposerCubit, ComposerState>(
          builder: (BuildContext context, ComposerState composer) {
            final String? activeRoom = composer.activeRoom;
            return Padding(
              padding: EdgeInsets.fromLTRB(
                dimens.space8,
                dimens.space6,
                dimens.space8,
                dimens.space6,
              ),
              child: ScaffoldComposer(
                hintText: activeRoom == null
                    ? 'Select a room to start messaging'
                    : 'Message #${roomDisplayName(activeRoom)}',
                disabled: !session.isReady,
                onSubmit: (String value) {
                  final ComposerCubit cubit = context.read<ComposerCubit>();
                  cubit.updateDraft(value);
                  cubit.send();
                },
                actionRow: <Widget>[_SendButton(disabled: !session.isReady)],
              ),
            );
          },
        );
      },
    );
  }
}

/// The circular send affordance of the composer action row (UI-SPEC: fill
/// `lightGreenPrimary`, `Icons.send` tinted `ScaffoldColors.btnText`).
///
/// Bound to [ComposerCubit.send]: it submits the cubit-held draft. The
/// scaffold composer atom owns its `TextEditingController` internally
/// (D-07), so live keystrokes surface to the cubit on each composer submit.
class _SendButton extends StatelessWidget {
  /// Creates the send button.
  const _SendButton({required this.disabled});

  /// Mirrors the composer's disabled state (session not ready).
  final bool disabled;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    // 40px circular chat action (UI-SPEC circular-button size: minimum touch
    // target minus one space4 step); ScaffoldPressable lifts it to 48px hit
    // area via its internal touch target.
    final double size = dimens.minTouchTarget - dimens.space4;
    return ScaffoldPressable(
      disabled: disabled,
      semanticLabel: 'Send message',
      onPressed: () => context.read<ComposerCubit>().send(),
      child: ScaffoldSurface(
        shape: BoxShape.circle,
        color: context.palette.lightGreenPrimary,
        child: SizedBox(
          width: size,
          height: size,
          child: const Center(
            child: Icon(Icons.send, color: ScaffoldColors.btnText),
          ),
        ),
      ),
    );
  }
}
