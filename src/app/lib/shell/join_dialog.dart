/// JoinDialog -- the Phase 4 paste-invite redeem flow (MEMB-01/T-04-07-I).
///
/// One field, one shape validation, one data-only command. The pasted link is
/// checked against the minted `gcs://invite/{space|room}/<id>?key=<key>` shape
/// before the redeem command publishes, and C++ re-validates the same shape on
/// receipt (defense in depth -- the C++ guard is the enforcement, this is the
/// UX gate). A refused publish toasts and leaves the dialog open; success
/// closes immediately and the rail/member-list update arrives as a pushed
/// event -- NO local optimism, NO success toast (UI-SPEC join flow).
///
/// View-local state (the paste field and the one-shot submit guard) lives in a
/// private [_JoinForm] ChangeNotifier -- never in Cubits (Phase 2 D-05
/// discipline).
library;

import 'package:flutter/material.dart';

import 'package:frontend_scaffold/components/bottom_drawer/responsive_drawer.dart';
import 'package:frontend_scaffold/components/scaffold_pressable.dart';
import 'package:frontend_scaffold/components/scaffold_surface.dart';
import 'package:frontend_scaffold/components/text_entry_field_widget.dart';
import 'package:frontend_scaffold/components/text_form_field_logic.dart';
import 'package:frontend_scaffold/components/toast/toast_manager.dart';
import 'package:frontend_scaffold/theme/scaffold_colors.dart';
import 'package:frontend_scaffold/theme/scaffold_dimens.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import '../cubits/session_cubit.dart';
import '../generated/proto/gcs_chat.pb.dart';
import 'space_room_dialog.dart' show kConfirmButtonHeight, kPublishFailedMessage;

// ---------------------------------------------------------------------------
// Copy (verbatim 04-UI-SPEC)
// ---------------------------------------------------------------------------

/// Join dialog title.
const String kJoinDialogTitle = 'Join';

/// Paste field hint.
const String kJoinFieldHint = 'Paste invite link';

/// Inline shape-validation message (dialog stays open).
const String kJoinInvalidLinkMessage = 'Enter a valid invite link.';

/// Join confirm label (UI-SPEC: accent reserved list, item 11).
const String kJoinConfirmLabel = 'Join';

/// Error-toast title when the transport refuses the redeem publish.
const String kErrorCouldNotJoin = "Couldn't join";

/// The minted invite-link shape (D-05, 04-RESEARCH P8): scheme `gcs`, the
/// `invite` authority, a `space`/`room` scope segment, an opaque id, and the
/// bearer key as the single query parameter. Dart's job is the UX gate; the
/// C++ redeem path re-parses and re-validates the exact shape.
final RegExp kInviteLinkPattern = RegExp(
  r'^gcs://invite/(?:space|room)/[^/?#]+[?]key=[^&\s]+$',
);

// ---------------------------------------------------------------------------
// Entry points
// ---------------------------------------------------------------------------

/// The Phase 4 paste-invite redeem flow.
class JoinDialog {
  JoinDialog._();

  /// Opens the join (redeem) dialog.
  static Future<void> show({
    required BuildContext context,
    required GcsCommandTransport transport,
  }) {
    final _JoinForm form = _JoinForm(transport: transport);
    return ResponsiveDrawer.show<void>(
      context: context,
      title: kJoinDialogTitle,
      children: <Widget>[_JoinFields(form: form)],
      footer: _JoinFooter(form: form),
      onClose: form.dispose,
    );
  }
}

/// Opens the join (redeem) dialog (the plan 04-07 entry point; delegates to
/// [JoinDialog.show]).
Future<void> showJoinDialog({
  required BuildContext context,
  required GcsCommandTransport transport,
}) {
  return JoinDialog.show(context: context, transport: transport);
}

// ---------------------------------------------------------------------------
// Form
// ---------------------------------------------------------------------------

/// View-local join-form state: the paste field, the inline error, and the
/// one-shot submit guard (Phase 2 D-05 discipline).
class _JoinForm extends ChangeNotifier {
  _JoinForm({required this.transport}) {
    controller.addListener(_handleChanged);
  }

  /// Publish seam for the redeem command.
  final GcsCommandTransport transport;

  /// Owns the pasted link.
  final TextEditingController controller = TextEditingController();

  bool _showError = false;

  /// One-shot confirm guard (Phase 2 D-05): a held Enter key or a fast
  /// Enter-then-tap must publish at most one redeem command.
  bool _submitHandled = false;

  /// Whether the inline shape error is showing.
  bool get showError => _showError;

  /// Whether confirm is interactive (an empty field is a disabled pill).
  bool get canSubmit => controller.text.isNotEmpty;

  /// Confirm (UI-SPEC join flow): a value that does not parse as the invite
  /// shape shows the inline error and keeps the dialog open; a valid value
  /// publishes the redeem command and closes immediately; a refused publish
  /// toasts and stays open for a retry. Exactly one publish per confirm.
  void submit(BuildContext context) {
    if (_submitHandled) {
      return;
    }
    final String value = controller.text.trim();
    if (!kInviteLinkPattern.hasMatch(value)) {
      _showError = true;
      notifyListeners();
      return;
    }
    final bool accepted = transport.publishCommand(
      GcsCommand()..redeem = (RedeemCommand()..inviteUrl = value),
    );
    if (!accepted) {
      showToast(
        context,
        kPublishFailedMessage,
        title: kErrorCouldNotJoin,
        type: ToastType.error,
      );
      return;
    }
    _submitHandled = true;
    // Close immediately -- the rail/member-list update is the confirmation.
    Navigator.of(context).pop();
  }

  /// Notifies the subtrees on keystrokes and clears a stale inline error once
  /// the value matches the invite shape.
  void _handleChanged() {
    if (_showError && kInviteLinkPattern.hasMatch(controller.text.trim())) {
      _showError = false;
    }
    notifyListeners();
  }

  /// Releases the paste controller; runs from the drawer's close callback.
  @override
  void dispose() {
    controller.removeListener(_handleChanged);
    controller.dispose();
    super.dispose();
  }
}

/// The paste field plus the inline validation line (the `children` subtree).
class _JoinFields extends StatelessWidget {
  const _JoinFields({required this.form});

  /// Shared view-local form state.
  final _JoinForm form;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return ListenableBuilder(
      listenable: form,
      builder: (BuildContext context, Widget? child) {
        return Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            TextEntryFieldWidget(
              logic: TextFormFieldLogic(
                context,
                controller: form.controller,
                hintText: kJoinFieldHint,
                autofocus: true,
                // Enter submits (UI-SPEC field composition).
                onFieldSubmitted: (String _) => form.submit(context),
              ),
            ),
            if (form.showError)
              Padding(
                padding: EdgeInsets.only(top: dimens.space2),
                child: Text(
                  kJoinInvalidLinkMessage,
                  style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                    color: context.palette.statusError,
                  ),
                ),
              ),
          ],
        );
      },
    );
  }
}

/// The join footer: Cancel then the accent "Join" pill (the Phase 2
/// confirm-pill composition).
class _JoinFooter extends StatelessWidget {
  const _JoinFooter({required this.form});

  /// Shared view-local form state.
  final _JoinForm form;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return ListenableBuilder(
      listenable: form,
      builder: (BuildContext context, Widget? child) {
        return Row(
          mainAxisAlignment: MainAxisAlignment.end,
          children: <Widget>[
            ScaffoldPressable(
              onPressed: () => Navigator.of(context).pop(),
              child: Padding(
                padding: EdgeInsets.symmetric(
                  horizontal: dimens.space4,
                  vertical: dimens.space2,
                ),
                child: Text(
                  'Cancel',
                  style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                    color: context.palette.textSecondary,
                  ),
                ),
              ),
            ),
            SizedBox(width: dimens.space4),
            ScaffoldPressable(
              disabled: !form.canSubmit,
              onPressed: () => form.submit(context),
              child: ScaffoldSurface(
                color: context.palette.lightGreenPrimary,
                borderRadius: BorderRadius.circular(dimens.borderRadiusButton),
                padding: EdgeInsets.symmetric(horizontal: dimens.space8),
                child: SizedBox(
                  height: kConfirmButtonHeight,
                  child: Center(
                    child: Text(
                      kJoinConfirmLabel,
                      style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                        color: ScaffoldColors.btnText,
                        fontWeight: FontWeight.w500,
                      ),
                    ),
                  ),
                ),
              ),
            ),
          ],
        );
      },
    );
  }
}
