/// SpaceRoomDialog -- the one reusable create/edit dialog (D-05).
///
/// A dumb form composed entirely from scaffold atoms: shown through
/// [ResponsiveDrawer.show] (dialog on wide screens, bottom sheet on
/// narrow), the name field is a [TextEntryFieldWidget], the type and
/// visibility selectors are coordinated [ScaffoldSelectionIndicatorRadio]
/// pairs, autoJoinRooms is a [ScaffoldSelectionIndicatorToggle], and a
/// refused publish surfaces via [showToast]. Confirm publishes the
/// matching `GcsCommand` arm through the injected [GcsCommandTransport]
/// and closes immediately -- NO local optimism (D-05): the rail renders
/// nothing new until C++ pushes the updated SpaceTree, and no success
/// toast fires (the rail updating IS the confirmation). Pushed
/// ErrorNotice rejections surface through the existing SessionState.error
/// path, not a toast (accepted deviation, plan 02-04).
///
/// View-local form state lives in a private [_DialogForm] ChangeNotifier
/// rather than a single `State` because [ResponsiveDrawer.show] mounts the
/// fields (`children`) and the action row (`footer`) as two separate
/// subtrees -- one State cannot rebuild both. The form is disposed by the
/// drawer's close callback, after both subtrees have unmounted.
library;

import 'package:flutter/material.dart';

import 'package:frontend_scaffold/components/bottom_drawer/responsive_drawer.dart';
import 'package:frontend_scaffold/components/scaffold_pressable.dart';
import 'package:frontend_scaffold/components/scaffold_selection_indicator_radio.dart';
import 'package:frontend_scaffold/components/scaffold_selection_indicator_toggle.dart';
import 'package:frontend_scaffold/components/scaffold_surface.dart';
import 'package:frontend_scaffold/components/text_entry_field_widget.dart';
import 'package:frontend_scaffold/components/text_form_field_logic.dart';
import 'package:frontend_scaffold/components/toast/toast_manager.dart';
import 'package:frontend_scaffold/theme/scaffold_colors.dart';
import 'package:frontend_scaffold/theme/scaffold_dimens.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import '../cubits/rail_cubit.dart';
import '../cubits/session_cubit.dart';
import '../generated/proto/gcs_chat.pb.dart';

/// Which surface launched the dialog; selects the field set per the D-05
/// mode table (UI-SPEC dialog modes).
enum SpaceRoomDialogMode {
  /// Rail-header "+": type selector visible ("Space" default /
  /// "Standalone room"); visibility + autoJoin shown while Space is
  /// selected.
  createFromHeader,

  /// Space-node "+": fixed room-in-that-space; name field only.
  createRoomInSpace,

  /// Space-node edit: name/visibility/autoJoinRooms prefilled from the
  /// passed space.
  editSpace,
}

/// Maximum name length (T-02-09: client-side cap; C++ re-validates as
/// defense in depth).
const int kMaxNameLength = 64;

/// Confirm pill height in logical pixels (UI-SPEC footer: 40px pill lifted
/// to a 48px hit area by ScaffoldPressable's internal touch target).
const double kConfirmButtonHeight = 40.0;

/// Error-toast body when the transport refuses the publish (UI-SPEC copy,
/// verbatim).
const String kPublishFailedMessage =
    "The command didn't reach the chat core. Try again.";

/// Shows the one create/edit dialog (D-05).
///
/// FROZEN entry-point contract -- Plan 02-05 rail affordances call exactly
/// this signature; do not rename or re-sign. [space] is required for
/// [SpaceRoomDialogMode.editSpace] (prefills id/name/isPublic/
/// autoJoinRooms); [parent] is required for
/// [SpaceRoomDialogMode.createRoomInSpace] (dialog title + parent id).
/// Dart stays data-only (D-01): the published command carries no id --
/// C++ mints it at creation.
Future<void> showSpaceRoomDialog({
  required BuildContext context,
  required GcsCommandTransport transport,
  required SpaceRoomDialogMode mode,
  RailSpace? space,
  RailSpace? parent,
}) {
  final _DialogForm form = _DialogForm(
    mode: mode,
    space: space,
    parent: parent,
    transport: transport,
  );
  return ResponsiveDrawer.show<void>(
    context: context,
    // The atom takes a static title per open; createFromHeader opens on
    // the default Space type. The hint and confirm label (inside the
    // form's subtrees) DO flip with the type selector.
    title: form.dialogTitle,
    children: <Widget>[_DialogFields(form: form)],
    footer: _DialogFooter(form: form),
    onClose: form.dispose,
  );
}

/// View-local dialog form state (never enters Cubits -- UI-SPEC state
/// contracts). Holds the name controller and the config booleans, derives
/// every per-mode label/hint, and owns the confirm behavior contract:
/// validate -> publish -> close immediately (or toast and stay open).
class _DialogForm extends ChangeNotifier {
  /// Creates the form; [space] prefills editSpace, [parent] carries the
  /// containing space for createRoomInSpace.
  _DialogForm({
    required this.mode,
    required this.transport,
    this.space,
    this.parent,
  }) : _isPublic = mode == SpaceRoomDialogMode.editSpace
           ? (space?.isPublic ?? true)
           : true,
       _autoJoinRooms = mode == SpaceRoomDialogMode.editSpace
           ? (space?.autoJoinRooms ?? false)
           : false,
       nameController = TextEditingController(
         text: mode == SpaceRoomDialogMode.editSpace ? (space?.name ?? '') : '',
       ) {
    nameController.addListener(_handleNameChanged);
  }

  /// Launch mode (selects the field set).
  final SpaceRoomDialogMode mode;

  /// Publish seam for the confirm action (D-27).
  final GcsCommandTransport transport;

  /// The space being edited (editSpace only).
  final RailSpace? space;

  /// The containing space (createRoomInSpace only).
  final RailSpace? parent;

  /// Owns the name text (prefilled for editSpace).
  final TextEditingController nameController;

  /// Whether the header-launch type selector reads "Space" (the default).
  bool _isSpaceType = true;

  /// Selected visibility (Public default; editSpace prefilled).
  bool _isPublic;

  /// D-04 derived-join config (editSpace prefilled).
  bool _autoJoinRooms;

  /// Whether the inline empty-name error is showing.
  bool _showNameError = false;

  /// One-shot confirm guard: a held Enter key (key-repeat on
  /// [TextFormField.onFieldSubmitted]) or a fast Enter-then-tap can invoke
  /// [submit] twice before the route finishes popping -- without the guard
  /// each invocation publishes another command (WR-04).
  bool _submitHandled = false;

  /// Dialog title per mode (UI-SPEC copy table).
  String get dialogTitle {
    switch (mode) {
      case SpaceRoomDialogMode.createFromHeader:
        return 'New space';
      case SpaceRoomDialogMode.createRoomInSpace:
        return 'New room in ${parent!.name}';
      case SpaceRoomDialogMode.editSpace:
        return 'Edit space';
    }
  }

  /// Name-field hint per mode and selected type (UI-SPEC copy table).
  String get nameHint {
    switch (mode) {
      case SpaceRoomDialogMode.createFromHeader:
        return _isSpaceType ? 'Space name' : 'Room name';
      case SpaceRoomDialogMode.createRoomInSpace:
        return 'Room name';
      case SpaceRoomDialogMode.editSpace:
        return 'Space name';
    }
  }

  /// Confirm label per mode and selected type (UI-SPEC copy table).
  String get confirmLabel {
    switch (mode) {
      case SpaceRoomDialogMode.createFromHeader:
        return _isSpaceType ? 'Create space' : 'Create room';
      case SpaceRoomDialogMode.createRoomInSpace:
        return 'Create room';
      case SpaceRoomDialogMode.editSpace:
        return 'Save changes';
    }
  }

  /// Error-toast title for the command this mode publishes (UI-SPEC copy).
  String get _errorTitle {
    switch (mode) {
      case SpaceRoomDialogMode.createFromHeader:
        return _isSpaceType ? "Couldn't create space" : "Couldn't create room";
      case SpaceRoomDialogMode.createRoomInSpace:
        return "Couldn't create room";
      case SpaceRoomDialogMode.editSpace:
        return "Couldn't update space";
    }
  }

  /// Whether the type selector renders (header-launch create only).
  bool get showsTypeSelector => mode == SpaceRoomDialogMode.createFromHeader;

  /// Whether the visibility + autoJoin controls render (space-config
  /// modes: header-create with Space selected, or editSpace).
  bool get showsSpaceConfig =>
      mode == SpaceRoomDialogMode.editSpace ||
      (mode == SpaceRoomDialogMode.createFromHeader && _isSpaceType);

  /// Current type-selector value ("Space" when true).
  bool get isSpaceType => _isSpaceType;

  /// Current visibility selection.
  bool get isPublic => _isPublic;

  /// Current autoJoinRooms selection.
  bool get autoJoinRooms => _autoJoinRooms;

  /// Whether the inline empty-name error is showing.
  bool get showNameError => _showNameError;

  /// Whether confirm is interactive: any characters typed (whitespace
  /// counts, so the inline validation can explain a whitespace-only name
  /// -- a disabled tap teaches nothing). A truly empty field disables the
  /// confirm pill (UI-SPEC footer contract).
  bool get canSubmit => nameController.text.isNotEmpty;

  /// Coordinated type-selector write (bool radio pair, D-05).
  void selectSpaceType(bool isSpace) {
    if (_isSpaceType == isSpace) {
      return;
    }
    _isSpaceType = isSpace;
    notifyListeners();
  }

  /// Coordinated visibility write (bool radio pair, D-05).
  void selectVisibility(bool isPublic) {
    if (_isPublic == isPublic) {
      return;
    }
    _isPublic = isPublic;
    notifyListeners();
  }

  /// autoJoinRooms write (D-04 config toggle).
  void setAutoJoinRooms(bool value) {
    if (_autoJoinRooms == value) {
      return;
    }
    _autoJoinRooms = value;
    notifyListeners();
  }

  /// Confirm (UI-SPEC behavior contract): empty name -> inline error and
  /// no publish; valid name -> publish and close immediately; refused
  /// publish -> error toast, dialog stays open for a retry. Exactly one
  /// publish per confirm -- re-entrant invocations (Enter key-repeat,
  /// Enter-then-tap) are ignored (WR-04).
  void submit(BuildContext context) {
    if (_submitHandled) {
      return;
    }
    final String name = nameController.text.trim();
    if (name.isEmpty) {
      _showNameError = true;
      notifyListeners();
      return;
    }
    final GcsCommand command = _buildCommand(name);
    if (!transport.publishCommand(command)) {
      showToast(
        context,
        kPublishFailedMessage,
        title: _errorTitle,
        type: ToastType.error,
      );
      return;
    }
    _submitHandled = true;
    // Close immediately -- no spinner, no waiting for the SpaceTree push.
    Navigator.of(context).pop();
  }

  /// Notifies both subtrees on keystrokes and clears a stale inline error
  /// once the name becomes valid.
  void _handleNameChanged() {
    if (_showNameError && nameController.text.trim().isNotEmpty) {
      _showNameError = false;
    }
    notifyListeners();
  }

  /// Builds the `GcsCommand` oneof arm for this mode (data-only, D-01).
  GcsCommand _buildCommand(String name) {
    switch (mode) {
      case SpaceRoomDialogMode.createFromHeader:
        if (_isSpaceType) {
          return GcsCommand(
            createSpace: CreateSpaceCommand(
              name: name,
              isPublic: _isPublic,
              autoJoinRooms: _autoJoinRooms,
            ),
          );
        }
        return GcsCommand(
          createRoom: CreateRoomCommand(name: name, parentSpaceId: ''),
        );
      case SpaceRoomDialogMode.createRoomInSpace:
        return GcsCommand(
          createRoom: CreateRoomCommand(name: name, parentSpaceId: parent!.id),
        );
      case SpaceRoomDialogMode.editSpace:
        return GcsCommand(
          updateSpace: UpdateSpaceCommand(
            spaceId: space!.id,
            name: name,
            isPublic: _isPublic,
            autoJoinRooms: _autoJoinRooms,
          ),
        );
    }
  }

  /// Releases the name controller; runs from the drawer's close callback,
  /// after both subtrees unmount and drop their listeners.
  @override
  void dispose() {
    nameController.removeListener(_handleNameChanged);
    nameController.dispose();
    super.dispose();
  }
}

/// The fields column of the dialog (the `children` subtree of
/// [ResponsiveDrawer.show]).
class _DialogFields extends StatelessWidget {
  /// Creates the fields column bound to [form].
  const _DialogFields({required this.form});

  /// Shared view-local form state.
  final _DialogForm form;

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: form,
      builder: (BuildContext context, Widget? child) {
        final ScaffoldDimens dimens = context.dimens;
        return Column(
          // ListView children get unbounded height; min keeps this column
          // intrinsically sized.
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            TextEntryFieldWidget(
              logic: TextFormFieldLogic(
                context,
                controller: form.nameController,
                hintText: form.nameHint,
                autofocus: true,
                maxLength: kMaxNameLength,
                // Enter submits (UI-SPEC field composition).
                onFieldSubmitted: (String _) => form.submit(context),
              ),
            ),
            if (form.showNameError)
              Padding(
                padding: EdgeInsets.only(top: dimens.space2),
                child: Text(
                  'Enter a name.',
                  style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                    color: context.palette.statusError,
                  ),
                ),
              ),
            if (form.showsTypeSelector) ...<Widget>[
              SizedBox(height: dimens.space4),
              _radioRow(
                context,
                label: 'Space',
                value: form.isSpaceType,
                onSelect: () => form.selectSpaceType(true),
              ),
              _radioRow(
                context,
                label: 'Standalone room',
                value: !form.isSpaceType,
                onSelect: () => form.selectSpaceType(false),
              ),
            ],
            if (form.showsSpaceConfig) ...<Widget>[
              SizedBox(height: dimens.space4),
              _radioRow(
                context,
                label: 'Public',
                value: form.isPublic,
                onSelect: () => form.selectVisibility(true),
              ),
              _radioRow(
                context,
                label: 'Private',
                value: !form.isPublic,
                onSelect: () => form.selectVisibility(false),
              ),
              SizedBox(height: dimens.space4),
              _autoJoinRow(context),
            ],
          ],
        );
      },
    );
  }

  /// One tappable radio + Body label row (the whole row toggles, not just
  /// the 48px indicator -- D-05 field composition). The radio's own tap
  /// and the row tap both route to [onSelect]: the bool radio atom is
  /// coordinated per-pair, so a row always writes its own value and never
  /// toggles.
  Widget _radioRow(
    BuildContext context, {
    required String label,
    required bool value,
    required VoidCallback onSelect,
  }) {
    final ScaffoldDimens dimens = context.dimens;
    return ScaffoldPressable(
      onPressed: onSelect,
      child: Row(
        children: <Widget>[
          ScaffoldSelectionIndicatorRadio(
            value: value,
            onChanged: (bool _) => onSelect(),
          ),
          SizedBox(width: dimens.space2),
          Expanded(
            child: Text(
              label,
              style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                color: context.palette.textPrimary,
              ),
            ),
          ),
        ],
      ),
    );
  }

  /// The autoJoinRooms row: toggle + Body label + Label description at
  /// `textSecondary` (UI-SPEC typography contract).
  Widget _autoJoinRow(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return Row(
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        ScaffoldSelectionIndicatorToggle(
          value: form.autoJoinRooms,
          onChanged: form.setAutoJoinRooms,
        ),
        SizedBox(width: dimens.space2),
        Expanded(
          child: Column(
            mainAxisSize: MainAxisSize.min,
            crossAxisAlignment: CrossAxisAlignment.start,
            children: <Widget>[
              Text(
                'Auto-join rooms',
                style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                  color: context.palette.textPrimary,
                ),
              ),
              SizedBox(height: dimens.space2),
              Text(
                "You'll automatically join every room in this space. Rooms "
                'stay visible either way.',
                style: Theme.of(context).textTheme.labelSmall?.copyWith(
                  color: context.palette.textSecondary,
                ),
              ),
            ],
          ),
        ),
      ],
    );
  }
}

/// The end-aligned action row of the dialog (the `footer` subtree of
/// [ResponsiveDrawer.show]): transparent Cancel then the accent confirm
/// pill (UI-SPEC footer contract).
class _DialogFooter extends StatelessWidget {
  /// Creates the footer row bound to [form].
  const _DialogFooter({required this.form});

  /// Shared view-local form state.
  final _DialogForm form;

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: form,
      builder: (BuildContext context, Widget? child) {
        final ScaffoldDimens dimens = context.dimens;
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
                      form.confirmLabel,
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
