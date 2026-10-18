/// MembersDialog -- the Phase 4 membership surface (D-04/D-07/D-08).
///
/// One module owns every membership surface: the per-scope member list, the
/// invite view, the role-change selector, and the destructive confirmations
/// (remove / leave / delete). Every surface is a dumb read of pushed state --
/// the roster from [MembersCubit]'s `MemberList` push, the invite link from
/// [SessionCubit]'s `InviteLink` push -- publishing data-only `GcsCommand`
/// envelopes through the injected [GcsCommandTransport]. No permission
/// enforcement, no id/key minting, no local optimism (D-08): a refused
/// publish toasts and stays open, and the roster/link only ever changes when
/// C++ pushes the next event.
///
/// [ResponsiveDrawer.show] takes a static title per open (the atom builds its
/// header once), so each mode is its own open of the same drawer shell rather
/// than one route retitled in place: the roster, the invite view, the
/// role-change selector, and each confirmation are separate opens.
///
/// View-local state (the role selection and the one-shot submit guards) lives
/// in private [_DialogForm] notifiers -- never in Cubits (Phase 2 D-05
/// discipline).
library;

import 'dart:async' show unawaited;

import 'package:flutter/material.dart';
import 'package:flutter/services.dart' show Clipboard, ClipboardData;
import 'package:flutter_bloc/flutter_bloc.dart';

import 'package:frontend_scaffold/components/bottom_drawer/responsive_drawer.dart';
import 'package:frontend_scaffold/components/scaffold_badge.dart';
import 'package:frontend_scaffold/components/scaffold_pressable.dart';
import 'package:frontend_scaffold/components/scaffold_selection_indicator_radio.dart';
import 'package:frontend_scaffold/components/scaffold_state_view.dart';
import 'package:frontend_scaffold/components/scaffold_surface.dart';
import 'package:frontend_scaffold/components/toast/toast_manager.dart';
import 'package:frontend_scaffold/theme/scaffold_colors.dart';
import 'package:frontend_scaffold/theme/scaffold_dimens.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import '../cubits/members_cubit.dart';
import '../cubits/message_flow_cubit.dart' show truncateSender;
import '../cubits/session_cubit.dart';
import '../generated/proto/gcs_chat.pb.dart';
import 'space_room_dialog.dart'
    show kConfirmButtonHeight, kPublishFailedMessage;

// ---------------------------------------------------------------------------
// Copy (verbatim 04-UI-SPEC)
// ---------------------------------------------------------------------------

/// Member-list footer CTA (UI-SPEC: accent reserved list, item 9).
const String kInviteMembersLabel = 'Invite members';

/// Invite-view title prefix; the full title is `Invite to <name>`.
const String kInviteViewTitlePrefix = 'Invite to ';

/// Invite-view helper line under the link.
const String kInviteViewHelper = 'Anyone with this link joins as a Guest.';

/// Invite-view copy action (UI-SPEC: accent reserved list, item 10).
const String kCopyLinkLabel = 'Copy link';

/// Invite-view close action.
const String kInviteViewDoneLabel = 'Done';

/// Copy-success compact pill (no title => pill density, not a card).
const String kInviteLinkCopiedMessage = 'Invite link copied';

/// Member-list empty state.
const String kNoMembersHeadline = 'No members yet';
const String kNoMembersBody = 'Invite someone to this space to get started.';

/// Role-change dialog title and confirm.
const String kChangeRoleTitle = 'Change role';
const String kSaveRoleLabel = 'Save role';

/// Remove confirmation (title is fixed; the target lives in the body).
const String kRemoveMemberTitle = 'Remove member';
const String kRemoveMemberConfirmLabel = 'Remove';

/// Error-toast titles (UI-SPEC copy table; each pairs the failed command
/// with the shared transport message as the body).
const String kErrorCouldNotCreateInvite = "Couldn't create invite";
const String kErrorCouldNotChangeRole = "Couldn't change role";
const String kErrorCouldNotRemoveMember = "Couldn't remove member";
const String kErrorCouldNotApprove = "Couldn't approve";
const String kErrorCouldNotLeave = "Couldn't leave";

/// Error-toast title for a refused delete publish. The UI-SPEC copy table
/// lists no delete title; this follows the established `Couldn't <verb>`
/// pattern for the one destructive command it omits.
const String kErrorCouldNotDelete = "Couldn't delete";

/// Delete-confirmation title suffix/body/confirm (UI-SPEC copy table).
const String kDeleteScopeBody =
    "This deletes it for everyone. This can't be undone.";
const String kDeleteConfirmLabel = 'Delete';

/// Roles grantable through the role-change selector (D-08). Super Admin is
/// creator-only -- never grantable through the UI, never listed (D-03).
const List<Role> kGrantableRoles = <Role>[
  Role.ROLE_ADMIN,
  Role.ROLE_MODERATOR,
  Role.ROLE_MEMBER,
  Role.ROLE_GUEST,
];

/// The pinned destructive-pill foreground.
///
/// [ScaffoldBadge] resolves its on-status color from fill luminance: fills
/// above its 0.40 threshold take dark text, everything else white. Both
/// `statusError` variants (light `#D13438`, dark `#FF4D4D`) sit below that
/// threshold, so the destructive confirm's foreground is the light one --
/// which also clears WCAG AA on both fills (white on `#D13438` is ~4.98:1).
const Color kDestructiveForeground = ScaffoldColors.white;

// ---------------------------------------------------------------------------
// Entry points
// ---------------------------------------------------------------------------

/// The Phase 4 membership surfaces (D-04/D-07/D-08).
class MembersDialog {
  MembersDialog._();

  /// Opens the member-list drawer for the scope [scopeId] (display name
  /// [scopeName]).
  ///
  /// The roster is a dumb read of [MembersCubit]'s pushed `MemberList` for
  /// that scope: loading until the first push, the UI-SPEC empty state when
  /// the pushed roster is empty, otherwise one row per [MemberInfo].
  ///
  /// [membersCanInvite] is the pushed scope flag (MEMB-05): a Member viewer
  /// sees the "Invite members" CTA only when it is true. Super Admin and
  /// Admin viewers always see it; the flag is ignored for them.
  static Future<void> show({
    required BuildContext context,
    required GcsCommandTransport transport,
    required String scopeId,
    required String scopeName,
    bool membersCanInvite = false,
  }) {
    return ResponsiveDrawer.show<void>(
      context: context,
      title: 'Members of $scopeName',
      children: <Widget>[
        _RosterBody(
          scopeId: scopeId,
          scopeName: scopeName,
          transport: transport,
        ),
      ],
      footer: _RosterFooter(
        scopeId: scopeId,
        scopeName: scopeName,
        membersCanInvite: membersCanInvite,
        transport: transport,
      ),
    );
  }

  /// Opens the destructive delete confirmation for the scope [scopeId].
  ///
  /// Shared by the rail's Delete affordance (space nodes and standalone
  /// rooms) so the copy, the confirmation composition, and the command live
  /// in exactly one place.
  static Future<void> showDeleteConfirm({
    required BuildContext context,
    required GcsCommandTransport transport,
    required String scopeId,
    required String scopeName,
  }) {
    return _showConfirm(
      context: context,
      transport: transport,
      title: 'Delete $scopeName?',
      body: kDeleteScopeBody,
      confirmLabel: kDeleteConfirmLabel,
      errorTitle: kErrorCouldNotDelete,
      command: GcsCommand()
        ..deleteScope = (DeleteScopeCommand()..scopeId = scopeId),
    );
  }
}

/// Opens the member-list drawer (the plan 04-07 entry point; delegates to
/// [MembersDialog.show]).
Future<void> showMembersDialog({
  required BuildContext context,
  required GcsCommandTransport transport,
  required String scopeId,
  required String scopeName,
  bool membersCanInvite = false,
}) {
  return MembersDialog.show(
    context: context,
    transport: transport,
    scopeId: scopeId,
    scopeName: scopeName,
    membersCanInvite: membersCanInvite,
  );
}

// ---------------------------------------------------------------------------
// Roster
// ---------------------------------------------------------------------------

/// Whether a viewer with [viewerRole] may offer the invite CTA (MEMB-05):
/// Super Admin/Admin always; Member only when the pushed [membersCanInvite]
/// flag says so.
bool _viewerCanInvite(String? viewerRole, bool membersCanInvite) {
  if (viewerCanManageMembers(viewerRole)) {
    return true;
  }
  return viewerRole == kRoleLabelMember && membersCanInvite;
}

/// Maps a five-tier display label back to its proto [Role] (the inverse of
/// [kRoleLabels]); unknown labels degrade to Member.
Role _roleFromLabel(String label) {
  for (final MapEntry<Role, String> entry in kRoleLabels.entries) {
    if (entry.value == label) {
      return entry.key;
    }
  }
  return Role.ROLE_MEMBER;
}

/// The member-list body: the pushed roster for [scopeId], or the loading /
/// empty sub-state (UI-SPEC roster sub-states).
class _RosterBody extends StatelessWidget {
  const _RosterBody({
    required this.scopeId,
    required this.scopeName,
    required this.transport,
  });

  /// Scope whose roster is rendered (space id or standalone room id).
  final String scopeId;

  /// Display name of the scope (confirmation copy).
  final String scopeName;

  /// Publish seam for the row affordances.
  final GcsCommandTransport transport;

  @override
  Widget build(BuildContext context) {
    return BlocBuilder<MembersCubit, MembersState>(
      builder: (BuildContext context, MembersState state) {
        final List<MemberInfo>? roster = state.byScope[scopeId];
        if (roster == null) {
          // No MemberList pushed for this scope yet (UI-SPEC: skeleton).
          return const ScaffoldStateView(state: 'loading');
        }
        if (roster.isEmpty) {
          // Edge: the creator is always a member, so an empty push is a
          // degenerate roster rather than a normal state.
          return const ScaffoldStateView(
            state: 'empty',
            emptyHeadline: kNoMembersHeadline,
            emptyBody: kNoMembersBody,
          );
        }
        final String? viewerRole = viewerOf(roster)?.role;
        return Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            for (final MemberInfo member in roster)
              _MemberRow(
                member: member,
                viewerRole: viewerRole,
                scopeId: scopeId,
                scopeName: scopeName,
                transport: transport,
              ),
          ],
        );
      },
    );
  }
}

/// One roster row: truncated address, "You" marker, pending badge, role
/// badge, and the gated trailing affordances (UI-SPEC roster anatomy).
class _MemberRow extends StatelessWidget {
  const _MemberRow({
    required this.member,
    required this.viewerRole,
    required this.scopeId,
    required this.scopeName,
    required this.transport,
  });

  /// The pushed member this row mirrors.
  final MemberInfo member;

  /// The viewer's pushed role label, or null when unknown.
  final String? viewerRole;

  /// Scope id carried by every published command.
  final String scopeId;

  /// Scope display name (confirmation copy).
  final String scopeName;

  /// Publish seam for the row affordances.
  final GcsCommandTransport transport;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    final bool viewerCanManage = viewerCanManageMembers(viewerRole);
    final bool targetIsSuperAdmin = member.role == kRoleLabelSuperAdmin;
    // UI-SPEC presence table: a Super Admin target never offers
    // change-role/remove (D-03), a pending target offers neither (the op is
    // already in flight), and a row never offers remove on itself.
    final bool showChangeRole =
        viewerCanManage && !targetIsSuperAdmin && !member.pending;
    final bool showRemove =
        viewerCanManage &&
        !targetIsSuperAdmin &&
        !member.self &&
        !member.pending;
    final bool showApprove = viewerIsSuperAdmin(viewerRole) && member.pending;

    return Padding(
      padding: EdgeInsets.symmetric(
        horizontal: dimens.space6,
        vertical: dimens.space2,
      ),
      child: ConstrainedBox(
        // UI-SPEC rail row contract: 40px content + space2*2 vertical padding
        // clears the 48px minimum touch target.
        constraints: BoxConstraints(
          minHeight: dimens.minTouchTarget - dimens.space2 * 2,
        ),
        child: Row(
          children: <Widget>[
            Expanded(
              child: Text(
                truncateSender(member.address),
                overflow: TextOverflow.ellipsis,
                style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                  color: context.palette.textPrimary,
                ),
              ),
            ),
            if (member.self) ...<Widget>[
              SizedBox(width: dimens.space3),
              Text(
                'You',
                style: Theme.of(context).textTheme.labelSmall?.copyWith(
                  color: context.palette.textSecondary,
                ),
              ),
            ],
            if (member.pending) ...<Widget>[
              SizedBox(width: dimens.space2),
              ScaffoldBadge(
                variant: BadgeVariant.text,
                text: member.pendingLabel,
                badgeColor: context.palette.statusWarningText,
              ),
            ],
            SizedBox(width: dimens.space2),
            ScaffoldBadge(
              variant: BadgeVariant.text,
              text: member.role,
              // UI-SPEC role-badge color contract: accent only for the
              // singular Super Admin tier; every other tier is text-only.
              badgeColor: targetIsSuperAdmin
                  ? context.palette.lightGreenPrimary
                  : context.palette.textSecondary,
            ),
            if (showChangeRole) ...<Widget>[
              SizedBox(width: dimens.space2),
              _RowIconAffordance(
                icon: Icons.manage_accounts_outlined,
                semanticLabel: 'Change role for ${member.address}',
                onTap: () => _openRoleChange(
                  context: context,
                  transport: transport,
                  scopeId: scopeId,
                  address: member.address,
                  currentRole: member.role,
                ),
              ),
            ],
            if (showRemove) ...<Widget>[
              SizedBox(width: dimens.space2),
              _RowIconAffordance(
                // UI-SPEC names `Icons.person_remove_outline`, which does not
                // exist in the pinned Flutter; `person_remove_outlined` is the
                // same glyph family's outlined variant.
                icon: Icons.person_remove_outlined,
                semanticLabel: 'Remove ${member.address}',
                color: context.palette.statusError,
                onTap: () => _openRemoveConfirm(
                  context: context,
                  transport: transport,
                  scopeId: scopeId,
                  scopeName: scopeName,
                  address: member.address,
                ),
              ),
            ],
            if (showApprove) ...<Widget>[
              SizedBox(width: dimens.space2),
              _RowIconAffordance(
                icon: Icons.check_circle_outline,
                semanticLabel: member.pendingLabel == kPendingRemovalLabel
                    ? 'Approve removal of ${member.address}'
                    : 'Approve role change for ${member.address}',
                onTap: () => approvePendingOp(
                  context: context,
                  transport: transport,
                  scopeId: scopeId,
                  target: member.address,
                ),
              ),
            ],
          ],
        ),
      ),
    );
  }
}

/// A 40px circular icon-only affordance at `textSecondary` (or a caller
/// tint), lifted to a 48px hit area by [ScaffoldPressable]'s internal touch
/// target. Every instance carries a [semanticLabel] (WCAG 4.1.2).
class _RowIconAffordance extends StatelessWidget {
  const _RowIconAffordance({
    required this.icon,
    required this.semanticLabel,
    required this.onTap,
    this.color,
  });

  /// Icon glyph.
  final IconData icon;

  /// Accessible name (icon-only pressable).
  final String semanticLabel;

  /// Opens the affordance's dialog or publishes its command.
  final VoidCallback onTap;

  /// Optional glyph tint; defaults to `textSecondary`.
  final Color? color;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    final double size = dimens.minTouchTarget - dimens.space4;
    return ScaffoldPressable(
      semanticLabel: semanticLabel,
      onPressed: onTap,
      child: SizedBox(
        width: size,
        height: size,
        child: Center(
          child: Icon(icon, color: color ?? context.palette.textSecondary),
        ),
      ),
    );
  }
}

/// The member-list footer: start-aligned `Leave <name>` (non-Super-Admin
/// viewers), end-aligned Cancel and the accent "Invite members" CTA.
class _RosterFooter extends StatelessWidget {
  const _RosterFooter({
    required this.scopeId,
    required this.scopeName,
    required this.membersCanInvite,
    required this.transport,
  });

  /// Scope whose roster drives the viewer-role gate.
  final String scopeId;

  /// Scope display name (leave-confirmation copy).
  final String scopeName;

  /// Pushed scope flag gating a Member viewer's invite CTA (MEMB-05).
  final bool membersCanInvite;

  /// Publish seam.
  final GcsCommandTransport transport;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return BlocBuilder<MembersCubit, MembersState>(
      builder: (BuildContext context, MembersState state) {
        final String? viewerRole = viewerOf(state.byScope[scopeId])?.role;
        final bool canLeave =
            viewerRole != null && viewerRole != kRoleLabelSuperAdmin;
        final bool canInvite = _viewerCanInvite(viewerRole, membersCanInvite);
        return Row(
          children: <Widget>[
            if (canLeave)
              ScaffoldPressable(
                onPressed: () => _openLeaveConfirm(
                  context: context,
                  transport: transport,
                  scopeId: scopeId,
                  scopeName: scopeName,
                ),
                child: Text(
                  'Leave $scopeName',
                  style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                    color: context.palette.textSecondary,
                  ),
                ),
              ),
            const Spacer(),
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
            if (canInvite) ...<Widget>[
              SizedBox(width: dimens.space4),
              _PillButton(
                label: kInviteMembersLabel,
                fill: context.palette.lightGreenPrimary,
                onPressed: () => _startInvite(
                  context: context,
                  transport: transport,
                  scopeId: scopeId,
                  scopeName: scopeName,
                ),
              ),
            ],
          ],
        );
      },
    );
  }
}

// ---------------------------------------------------------------------------
// Invite view
// ---------------------------------------------------------------------------

/// Publishes the mint command, then opens the invite view (no local
/// optimism: the view renders whatever link C++ pushes back).
void _startInvite({
  required BuildContext context,
  required GcsCommandTransport transport,
  required String scopeId,
  required String scopeName,
}) {
  final bool accepted = transport.publishCommand(
    GcsCommand()..invite = (InviteCommand()..scopeId = scopeId),
  );
  if (!accepted) {
    showToast(
      context,
      kPublishFailedMessage,
      title: kErrorCouldNotCreateInvite,
      type: ToastType.error,
    );
    return;
  }
  unawaited(
    _showInviteView(
      context: context,
      scopeId: scopeId,
      scopeName: scopeName,
      session: _sessionOf(context),
    ),
  );
}

/// Opens the invite view for [scopeName] (title `Invite to <name>`).
///
/// [scopeId] is the scope the invite was minted for: the view reads only that
/// scope's pushed link, so a link minted elsewhere never renders here (WR-07).
/// [session] is the session holding the pushed `InviteLink`; null (no provider
/// in scope) leaves the view in its loading sub-state -- the view never
/// invents a link.
///
/// The pending link is dropped when the drawer closes (not when it is read):
/// the push's own emission must still reach the view, and no stale capability
/// may outlive the view (T-04-10-02).
Future<void> _showInviteView({
  required BuildContext context,
  required String scopeId,
  required String scopeName,
  required SessionCubit? session,
}) {
  return ResponsiveDrawer.show<void>(
    context: context,
    title: '$kInviteViewTitlePrefix$scopeName',
    children: <Widget>[_InviteLinkView(session: session, scopeId: scopeId)],
    footer: const _InviteFooter(),
    onClose: () => session?.clearPendingInviteLink(scopeId),
  );
}

/// Resolves the [SessionCubit] from [context], or null when no provider is in
/// scope (the dialog module is usable without a session in widget tests).
SessionCubit? _sessionOf(BuildContext context) {
  try {
    return context.read<SessionCubit>();
  } on ProviderNotFoundException {
    return null;
  }
}

/// The invite view body: the pushed link once it lands, the skeleton until
/// then (UI-SPEC: no local optimism).
class _InviteLinkView extends StatelessWidget {
  const _InviteLinkView({required this.session, required this.scopeId});

  /// Session holding the pushed link; null renders the loading sub-state.
  final SessionCubit? session;

  /// Scope this view was opened for; the only link it may render (WR-07).
  final String scopeId;

  @override
  Widget build(BuildContext context) {
    final SessionCubit? session = this.session;
    if (session == null) {
      return const ScaffoldStateView(state: 'loading');
    }
    return StreamBuilder<SessionState>(
      // The invite-link dispatch emits a fresh SessionState so this rebuilds
      // when C++ pushes the minted link.
      stream: session.stream,
      initialData: session.state,
      builder: (BuildContext context, AsyncSnapshot<SessionState> snapshot) {
        // Scope-keyed read: a link minted for another scope is unreachable
        // here, so the view stays in its loading sub-state instead of
        // rendering the wrong capability token.
        final String? link = session.pendingInviteLinkFor(scopeId);
        if (link == null || link.isEmpty) {
          return const ScaffoldStateView(state: 'loading');
        }
        return _InviteLinkContent(link: link);
      },
    );
  }
}

/// The pushed-link content: a read-only monospace surface plus the helper
/// line and the accent "Copy link" action.
class _InviteLinkContent extends StatelessWidget {
  const _InviteLinkContent({required this.link});

  /// The pushed `gcs://invite/...` capability URL.
  final String link;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        ScaffoldSurface(
          padding: EdgeInsets.all(dimens.space4),
          child: SelectionArea(
            child: Text(
              link,
              maxLines: 2,
              overflow: TextOverflow.ellipsis,
              // The design system registers no monospace face; the generic
              // platform alias degrades to the default face where absent.
              style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                fontFamily: 'monospace',
                color: context.palette.textPrimary,
              ),
            ),
          ),
        ),
        SizedBox(height: dimens.space4),
        Text(
          kInviteViewHelper,
          style: Theme.of(context).textTheme.labelSmall?.copyWith(
            color: context.palette.textSecondary,
          ),
        ),
        SizedBox(height: dimens.space4),
        Align(
          alignment: AlignmentDirectional.centerEnd,
          child: _PillButton(
            label: kCopyLinkLabel,
            fill: context.palette.lightGreenPrimary,
            onPressed: () => _copyLink(context),
          ),
        ),
      ],
    );
  }

  /// Copies the link and confirms with the compact success pill (no title).
  void _copyLink(BuildContext context) {
    Clipboard.setData(ClipboardData(text: link));
    showToast(context, kInviteLinkCopiedMessage, type: ToastType.success);
  }
}

/// The invite view footer: the transparent "Done" close action.
class _InviteFooter extends StatelessWidget {
  const _InviteFooter();

  @override
  Widget build(BuildContext context) {
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
              kInviteViewDoneLabel,
              style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                color: context.palette.textSecondary,
              ),
            ),
          ),
        ),
      ],
    );
  }
}

// ---------------------------------------------------------------------------
// Role change
// ---------------------------------------------------------------------------

/// Opens the role-change selector for [address] (title "Change role").
void _openRoleChange({
  required BuildContext context,
  required GcsCommandTransport transport,
  required String scopeId,
  required String address,
  required String currentRole,
}) {
  final _RoleForm form = _RoleForm(
    scopeId: scopeId,
    target: address,
    initialRole: _roleFromLabel(currentRole),
  );
  unawaited(
    ResponsiveDrawer.show<void>(
      context: context,
      title: kChangeRoleTitle,
      children: <Widget>[_RoleFields(form: form)],
      footer: _RoleFooter(form: form, transport: transport),
      onClose: form.dispose,
    ),
  );
}

/// The four-way role selector fields (four grantable tiers, Super Admin never
/// listed).
class _RoleFields extends StatelessWidget {
  const _RoleFields({required this.form});

  /// Shared view-local role-selection state.
  final _RoleForm form;

  @override
  Widget build(BuildContext context) {
    return ListenableBuilder(
      listenable: form,
      builder: (BuildContext context, Widget? child) {
        return Column(
          mainAxisSize: MainAxisSize.min,
          crossAxisAlignment: CrossAxisAlignment.start,
          children: <Widget>[
            for (final Role role in kGrantableRoles)
              _RoleRow(
                label: roleLabel(role),
                selected: form.selectedRole == role,
                onSelect: () => form.selectRole(role),
              ),
          ],
        );
      },
    );
  }
}

/// One tappable radio + Body label row (the whole row toggles, not just the
/// indicator -- the Phase 2 field composition).
class _RoleRow extends StatelessWidget {
  const _RoleRow({
    required this.label,
    required this.selected,
    required this.onSelect,
  });

  /// Five-tier display label for this tier.
  final String label;

  /// Whether this tier is the current selection.
  final bool selected;

  /// Selects this tier.
  final VoidCallback onSelect;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return ScaffoldPressable(
      onPressed: onSelect,
      child: Row(
        children: <Widget>[
          ScaffoldSelectionIndicatorRadio(
            value: selected,
            // The bool atom reports the toggled value; the four-way selector
            // ignores it and always writes its own tier (idempotent when the
            // row is already selected).
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
}

/// The role-change footer: Cancel then the accent "Save role" pill.
class _RoleFooter extends StatelessWidget {
  const _RoleFooter({required this.form, required this.transport});

  /// Shared view-local role-selection state.
  final _RoleForm form;

  /// Publish seam for the save action.
  final GcsCommandTransport transport;

  @override
  Widget build(BuildContext context) {
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
        _PillButton(
          label: kSaveRoleLabel,
          fill: context.palette.lightGreenPrimary,
          onPressed: () => form.submit(context, transport),
        ),
      ],
    );
  }
}

// ---------------------------------------------------------------------------
// Destructive confirmations
// ---------------------------------------------------------------------------

/// Opens the remove-member confirmation for [address] in [scopeName].
void _openRemoveConfirm({
  required BuildContext context,
  required GcsCommandTransport transport,
  required String scopeId,
  required String scopeName,
  required String address,
}) {
  unawaited(
    _showConfirm(
      context: context,
      transport: transport,
      title: kRemoveMemberTitle,
      body:
          'Remove ${truncateSender(address)} from $scopeName? '
          "They'll lose access.",
      confirmLabel: kRemoveMemberConfirmLabel,
      errorTitle: kErrorCouldNotRemoveMember,
      command: GcsCommand()
        ..removeMember = (RemoveMemberCommand()
          ..scopeId = scopeId
          ..target = address),
    ),
  );
}

/// Opens the leave confirmation for [scopeName] (writes `explicit_leave`,
/// never removes the record -- D-02).
void _openLeaveConfirm({
  required BuildContext context,
  required GcsCommandTransport transport,
  required String scopeId,
  required String scopeName,
}) {
  unawaited(
    _showConfirm(
      context: context,
      transport: transport,
      title: 'Leave $scopeName?',
      body: "You'll lose access to this space and its rooms.",
      confirmLabel: 'Leave',
      errorTitle: kErrorCouldNotLeave,
      command: GcsCommand()..leave = (LeaveCommand()..scopeId = scopeId),
    ),
  );
}

/// Publishes the Super Admin's approve for a pending destructive op on
/// [target] in [scopeId] (immediate, no confirmation -- D-07 single-signer).
///
/// One home for the approve publish: the member row's approve affordance and
/// the rail's approve-for-deletion affordance both call it, so the command
/// shape and the refusal toast can never drift apart.
void approvePendingOp({
  required BuildContext context,
  required GcsCommandTransport transport,
  required String scopeId,
  required String target,
}) {
  final bool accepted = transport.publishCommand(
    GcsCommand()
      ..approve = (ApproveCommand()
        ..scopeId = scopeId
        ..target = target),
  );
  if (!accepted) {
    showToast(
      context,
      kPublishFailedMessage,
      title: kErrorCouldNotApprove,
      type: ToastType.error,
    );
  }
}

/// Opens one destructive confirmation (shared title/body/confirm/fill
/// composition for Remove / Leave / Delete).
Future<void> _showConfirm({
  required BuildContext context,
  required GcsCommandTransport transport,
  required String title,
  required String body,
  required String confirmLabel,
  required String errorTitle,
  required GcsCommand command,
}) {
  final _ConfirmForm form = _ConfirmForm(
    command: command,
    errorTitle: errorTitle,
  );
  return ResponsiveDrawer.show<void>(
    context: context,
    title: title,
    children: <Widget>[_ConfirmBody(body: body)],
    footer: _ConfirmFooter(
      form: form,
      transport: transport,
      confirmLabel: confirmLabel,
    ),
    onClose: form.dispose,
  );
}

/// The confirmation body line (Body, textPrimary).
class _ConfirmBody extends StatelessWidget {
  const _ConfirmBody({required this.body});

  /// Confirmation body copy (UI-SPEC verbatim).
  final String body;

  @override
  Widget build(BuildContext context) {
    return Text(
      body,
      style: Theme.of(
        context,
      ).textTheme.bodyMedium?.copyWith(color: context.palette.textPrimary),
    );
  }
}

/// The confirmation footer: Cancel then the destructive pill (filled
/// `statusError`).
class _ConfirmFooter extends StatelessWidget {
  const _ConfirmFooter({
    required this.form,
    required this.transport,
    required this.confirmLabel,
  });

  /// Shared one-shot submit state.
  final _ConfirmForm form;

  /// Publish seam.
  final GcsCommandTransport transport;

  /// Destructive confirm label ("Remove" / "Leave" / "Delete").
  final String confirmLabel;

  @override
  Widget build(BuildContext context) {
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
        _PillButton(
          label: confirmLabel,
          fill: context.palette.statusError,
          foreground: kDestructiveForeground,
          onPressed: () => form.submit(context, transport),
        ),
      ],
    );
  }
}

// ---------------------------------------------------------------------------
// Shared pieces
// ---------------------------------------------------------------------------

/// A 40px filled pill button (accent or destructive), lifted to a 48px hit
/// area by [ScaffoldPressable] (the Phase 2 confirm-pill composition).
class _PillButton extends StatelessWidget {
  const _PillButton({
    required this.label,
    required this.fill,
    required this.onPressed,
    this.foreground,
  });

  /// Button label.
  final String label;

  /// Pill fill color.
  final Color fill;

  /// Confirm action.
  final VoidCallback onPressed;

  /// Foreground (defaults to the accent's dark on-status color).
  final Color? foreground;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return ScaffoldPressable(
      onPressed: onPressed,
      child: ScaffoldSurface(
        color: fill,
        borderRadius: BorderRadius.circular(dimens.borderRadiusButton),
        padding: EdgeInsets.symmetric(horizontal: dimens.space8),
        child: SizedBox(
          height: kConfirmButtonHeight,
          child: Center(
            child: Text(
              label,
              style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                color: foreground ?? ScaffoldColors.btnText,
                fontWeight: FontWeight.w500,
              ),
            ),
          ),
        ),
      ),
    );
  }
}

/// View-local dialog form state (UI-SPEC state contracts; never in Cubits).
///
/// Owns the one-shot submit guard (Phase 2 D-05 discipline): a held Enter key
/// or a fast Enter-then-tap must publish at most one command.
abstract class _DialogForm extends ChangeNotifier {
  bool _submitHandled = false;

  /// Error-toast title used when the publish is refused.
  String get errorTitle;

  /// Builds and publishes this form's data-only command; returns whether the
  /// transport accepted it.
  bool publishCommand(GcsCommandTransport transport);

  /// Submits once: accepted publishes close the dialog, refused publishes
  /// toast and leave it open for a retry. Re-entrant calls are ignored.
  void submit(BuildContext context, GcsCommandTransport transport) {
    if (_submitHandled) {
      return;
    }
    if (!publishCommand(transport)) {
      showToast(
        context,
        kPublishFailedMessage,
        title: errorTitle,
        type: ToastType.error,
      );
      return;
    }
    _submitHandled = true;
    Navigator.of(context).pop();
  }
}

/// The role-change form: holds the selected tier and publishes the
/// change-role command.
class _RoleForm extends _DialogForm {
  _RoleForm({
    required this.scopeId,
    required this.target,
    required Role initialRole,
  }) : _selectedRole = initialRole;

  /// Scope the role change applies to.
  final String scopeId;

  /// Member address whose role changes.
  final String target;

  Role _selectedRole;

  /// The currently selected grantable tier.
  Role get selectedRole => _selectedRole;

  @override
  String get errorTitle => kErrorCouldNotChangeRole;

  /// Writes the selected tier (idempotent).
  void selectRole(Role role) {
    if (_selectedRole == role) {
      return;
    }
    _selectedRole = role;
    notifyListeners();
  }

  @override
  bool publishCommand(GcsCommandTransport transport) {
    return transport.publishCommand(
      GcsCommand()
        ..changeRole = (ChangeRoleCommand()
          ..scopeId = scopeId
          ..target = target
          ..role = _selectedRole),
    );
  }
}

/// A destructive confirmation form: publishes one prebuilt command.
class _ConfirmForm extends _DialogForm {
  _ConfirmForm({required GcsCommand command, required this.errorTitle})
    : _command = command;

  final GcsCommand _command;

  @override
  final String errorTitle;

  @override
  bool publishCommand(GcsCommandTransport transport) =>
      transport.publishCommand(_command);
}
