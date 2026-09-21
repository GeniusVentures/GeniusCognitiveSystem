/// RoomRail -- left rail of the GCS chat shell (D-21).
///
/// Renders the spaces/rooms tree held by [RailCubit]: a persistent toolbar
/// ("Spaces" label + the create affordance), expandable space nodes (private
/// badge, per-node "+" and edit affordances, nested room rows), a standalone
/// Rooms section hidden while empty, and the loading skeleton until the
/// first pushed SpaceTree (D-02/D-05). Joined room rows are selectable;
/// catalog-but-not-joined rows render dimmed and non-tappable (the
/// autoJoinRooms dim/un-dim observable, D-04). Everything is REPLACED
/// wholesale by pushed state -- the rail never synthesizes entities (C++ is
/// the sole source of truth). Space-node expansion is view-local, never in
/// [RailState]. Shell-side one-off widget (plain Dart per D-11) composed
/// from scaffold atoms (D-10/D-14).
library;

import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:frontend_scaffold/components/scaffold_badge.dart';
import 'package:frontend_scaffold/components/scaffold_motion.dart';
import 'package:frontend_scaffold/components/scaffold_pressable.dart';
import 'package:frontend_scaffold/components/scaffold_state_view.dart';
import 'package:frontend_scaffold/components/scaffold_surface.dart';
import 'package:frontend_scaffold/theme/scaffold_colors.dart';
import 'package:frontend_scaffold/theme/scaffold_dimens.dart';
import 'package:frontend_scaffold/theme/scaffold_palette.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import '../cubits/rail_cubit.dart';
import '../cubits/session_cubit.dart';
import 'space_room_dialog.dart';

/// Short display name of a pushed room topic (`gcs/chat/smoke-test` ->
/// `smoke-test`); topics without a slash render unchanged.
///
/// Used by the composer's active-room hint (gcs_shell.dart); the rail tree
/// renders [RailRoom.name] directly (never the topic-derived name).
String roomDisplayName(String roomTopic) {
  final int lastSlash = roomTopic.lastIndexOf('/');
  if (lastSlash < 0 || lastSlash == roomTopic.length - 1) {
    return roomTopic;
  }
  return roomTopic.substring(lastSlash + 1);
}

/// The left room rail of the GCS chat shell (D-21).
///
/// The toolbar persists in every rail state (loading, empty, populated) --
/// the creation affordance is never a dead end. Selection highlights the
/// active room (UI-SPEC color contract: accent at 10% alpha via
/// `ScaffoldColors.btnFilterSelected`); tapping a joined row drives
/// [RailCubit.selectRoom]. Unjoined rows are disabled pressables (T-02-11).
class RoomRail extends StatefulWidget {
  /// Creates a [RoomRail].
  const RoomRail({super.key});

  /// Rail width in logical pixels (UI-SPEC rail contract: 280 expanded).
  static const double kWidth = 280.0;

  /// Section-header letter spacing (UI-SPEC typography contract, label row).
  static const double kHeaderLetterSpacing = 0.5;

  @override
  State<RoomRail> createState() => _RoomRailState();
}

class _RoomRailState extends State<RoomRail> {
  /// View-local expansion map (space id -> collapsed). Never enters
  /// RailState (RailState mirrors pushed C++ truth only, D-04 Phase 1).
  /// Absent key = expanded (new spaces default open); keyed by space id so
  /// expansion survives `setTree` full replacements.
  final Map<String, bool> _collapsedSpaceIds = <String, bool>{};

  /// Whether [spaceId]'s node is expanded (default true).
  bool _isExpanded(String spaceId) => _collapsedSpaceIds[spaceId] ?? true;

  /// Toggles [spaceId]'s node expansion (whole-row tap).
  void _toggleExpanded(String spaceId) {
    setState(() {
      _collapsedSpaceIds[spaceId] = !_isExpanded(spaceId);
    });
  }

  @override
  Widget build(BuildContext context) {
    final ScaffoldPalette palette = context.palette;
    final ScaffoldDimens dimens = context.dimens;
    return ColoredBox(
      color: palette.grayPrimary,
      child: BlocBuilder<RailCubit, RailState>(
        builder: (BuildContext context, RailState rail) {
          return Column(
            children: <Widget>[
              // Toolbar (UI-SPEC rail contract): persists in every rail
              // state -- the create affordance is never a dead end.
              Padding(
                padding: EdgeInsets.all(dimens.space8),
                child: Row(
                  children: <Widget>[
                    Expanded(
                      child: Align(
                        alignment: AlignmentDirectional.centerStart,
                        child: Text(
                          'Spaces',
                          style: Theme.of(context).textTheme.titleSmall
                              ?.copyWith(
                                color: palette.textSecondary,
                                letterSpacing: RoomRail.kHeaderLetterSpacing,
                              ),
                        ),
                      ),
                    ),
                    const _HeaderCreateButton(),
                  ],
                ),
              ),
              Expanded(
                child: !_railHasTree(rail)
                    ? const ScaffoldStateView(state: 'loading')
                    : _railIsEmpty(rail)
                    ? ScaffoldStateView(
                        state: 'empty',
                        emptyHeadline: 'No spaces yet',
                        emptyBody:
                            'Spaces organize your rooms. Create your first space to get started.',
                        emptyAction: _CreateSpaceAction(
                          onTap: () => _openCreateFromHeader(context),
                        ),
                      )
                    : _buildTree(context, rail, dimens),
              ),
            ],
          );
        },
      ),
    );
  }

  /// Whether any SpaceTree has been pushed (loading-vs-empty flag, D-02).
  bool _railHasTree(RailState rail) => rail.treeReceived;

  /// Whether the pushed catalog is entirely empty (no spaces, no rooms).
  bool _railIsEmpty(RailState rail) =>
      rail.spaces.isEmpty && rail.standaloneRooms.isEmpty;

  /// The populated tree: space nodes, then the standalone Rooms section
  /// (hidden entirely while empty -- no empty-section noise).
  Widget _buildTree(BuildContext context, RailState rail, ScaffoldDimens dimens) {
    // Prune collapse state for spaces absent from the pushed tree (IN-02):
    // stale ids would otherwise accumulate for the whole session lifetime
    // (ids are unique per creation, so they never come back). Safe to mutate
    // mid-build without setState -- a pruned key cannot affect this frame
    // because its space is not rendering.
    _collapsedSpaceIds.removeWhere(
      (String id, bool collapsed) =>
          !rail.spaces.any((RailSpace space) => space.id == id),
    );
    final ScaffoldPalette palette = context.palette;
    return ListView(
      children: <Widget>[
        for (final RailSpace space in rail.spaces)
          _SpaceNode(
            space: space,
            expanded: _isExpanded(space.id),
            onToggle: () => _toggleExpanded(space.id),
            joinedTopics: rail.rooms,
            activeRoom: rail.activeRoom,
          ),
        if (rail.standaloneRooms.isNotEmpty) ...<Widget>[
          SizedBox(height: dimens.space12),
          Padding(
            padding: EdgeInsets.fromLTRB(
              dimens.space8,
              0,
              dimens.space8,
              dimens.space4,
            ),
            child: Align(
              alignment: AlignmentDirectional.centerStart,
              child: Text(
                'Rooms',
                style: Theme.of(context).textTheme.titleSmall?.copyWith(
                  color: palette.textSecondary,
                  letterSpacing: RoomRail.kHeaderLetterSpacing,
                ),
              ),
            ),
          ),
          for (final RailRoom room in rail.standaloneRooms)
            _TreeRoomRow(
              room: room,
              joined: rail.rooms.contains(room.topic),
              selected: rail.activeRoom == room.topic,
              leadingInset: dimens.space6,
            ),
        ],
      ],
    );
  }

  /// Opens the header-launch create dialog (D-05: "Space"/"Standalone room"
  /// type selector inside).
  void _openCreateFromHeader(BuildContext context) {
    showSpaceRoomDialog(
      context: context,
      transport: context.read<SessionCubit>(),
      mode: SpaceRoomDialogMode.createFromHeader,
    );
  }
}

/// The toolbar's 40px circular accent "+" (UI-SPEC: accent reserved list,
/// item 6 -- one instance per rail), lifted to a 48px hit area by
/// ScaffoldPressable's internal touch target.
class _HeaderCreateButton extends StatelessWidget {
  /// Creates the header create affordance.
  const _HeaderCreateButton();

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    final double size = dimens.minTouchTarget - dimens.space4;
    return ScaffoldPressable(
      semanticLabel: 'Create space or room',
      onPressed: () => showSpaceRoomDialog(
        context: context,
        transport: context.read<SessionCubit>(),
        mode: SpaceRoomDialogMode.createFromHeader,
      ),
      child: ScaffoldSurface(
        shape: BoxShape.circle,
        color: context.palette.lightGreenPrimary,
        child: SizedBox(
          width: size,
          height: size,
          child: const Center(
            child: Icon(Icons.add, color: ScaffoldColors.btnText),
          ),
        ),
      ),
    );
  }
}

/// The empty-state accent action pill ("Create space"; UI-SPEC: accent
/// reserved list, item 8) -- opens the header-launch create dialog.
class _CreateSpaceAction extends StatelessWidget {
  /// Creates the action; [onTap] opens the createFromHeader dialog.
  const _CreateSpaceAction({required this.onTap});

  /// Opens the create dialog.
  final VoidCallback onTap;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return ScaffoldPressable(
      onPressed: onTap,
      child: ScaffoldSurface(
        color: context.palette.lightGreenPrimary,
        borderRadius: BorderRadius.circular(dimens.borderRadiusButton),
        padding: EdgeInsets.symmetric(horizontal: dimens.space8),
        child: SizedBox(
          height: dimens.minTouchTarget - dimens.space4,
          child: Center(
            child: Text(
              'Create space',
              style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                color: ScaffoldColors.btnText,
                fontWeight: FontWeight.w500,
              ),
            ),
          ),
        ),
      ),
    );
  }
}

/// One expandable space node: the whole-row-tap header (chevron, name,
/// private badge, edit and "+" affordances) plus the nested room rows
/// revealed inside an [AnimatedSize] (ScaffoldDisclosure motion vocabulary
/// composed directly -- the atom's String title slot cannot carry the badge
/// and trailing actions).
class _SpaceNode extends StatelessWidget {
  /// Creates a [_SpaceNode] for [space].
  const _SpaceNode({
    required this.space,
    required this.expanded,
    required this.onToggle,
    required this.joinedTopics,
    required this.activeRoom,
  });

  /// The catalog space rendered by this node.
  final RailSpace space;

  /// Whether the node's nested rooms are revealed.
  final bool expanded;

  /// Toggles expansion (whole-row tap; spaces are not selectable in
  /// Phase 2).
  final VoidCallback onToggle;

  /// Joined room topics from the pushed RoomList (join-state source).
  final List<String> joinedTopics;

  /// Currently selected room topic, if any.
  final String? activeRoom;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    final bool reducedMotion = ScaffoldMotion.of(context).reducedMotion;
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        ScaffoldPressable(
          semanticLabel: space.name,
          onPressed: onToggle,
          child: Padding(
            padding: EdgeInsets.symmetric(
              horizontal: dimens.space6,
              vertical: dimens.space2,
            ),
            child: ConstrainedBox(
              // UI-SPEC rail contract: 40px row content + space2*2 vertical
              // padding clears the 48px minimum touch target.
              constraints: BoxConstraints(
                minHeight: dimens.minTouchTarget - dimens.space2 * 2,
              ),
              child: Row(
                children: <Widget>[
                  AnimatedRotation(
                    // Chevron spin, zero-duration under reduced motion.
                    turns: expanded ? 0.25 : 0.0,
                    duration: reducedMotion
                        ? Duration.zero
                        : ScaffoldMotionDurations.short,
                    curve: ScaffoldMotionCurves.standard,
                    child: Icon(
                      Icons.chevron_right,
                      color: context.palette.textSecondary,
                    ),
                  ),
                  SizedBox(width: dimens.space3),
                  Flexible(
                    child: Text(
                      space.name,
                      overflow: TextOverflow.ellipsis,
                      style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                        color: context.palette.textPrimary,
                      ),
                    ),
                  ),
                  if (!space.isPublic) ...<Widget>[
                    SizedBox(width: dimens.space2),
                    ScaffoldBadge(
                      variant: BadgeVariant.text,
                      text: 'Private',
                      badgeColor: context.palette.textSecondary,
                    ),
                  ],
                  const Spacer(),
                  _NodeIconAffordance(
                    icon: Icons.edit_outlined,
                    semanticLabel: 'Edit space ${space.name}',
                    onTap: () => showSpaceRoomDialog(
                      context: context,
                      transport: context.read<SessionCubit>(),
                      mode: SpaceRoomDialogMode.editSpace,
                      space: space,
                    ),
                  ),
                  SizedBox(width: dimens.space2),
                  _NodeIconAffordance(
                    icon: Icons.add,
                    semanticLabel: 'New room in ${space.name}',
                    onTap: () => showSpaceRoomDialog(
                      context: context,
                      transport: context.read<SessionCubit>(),
                      mode: SpaceRoomDialogMode.createRoomInSpace,
                      parent: space,
                    ),
                  ),
                ],
              ),
            ),
          ),
        ),
        AnimatedSize(
          // Body reveal, zero-duration under reduced motion. Padding keeps a
          // stable child across the transition (ScaffoldDisclosure idiom).
          duration: reducedMotion
              ? Duration.zero
              : ScaffoldMotionDurations.medium,
          curve: ScaffoldMotionCurves.standard,
          child: Padding(
            padding: EdgeInsets.zero,
            child: expanded
                ? _buildNestedRows(context)
                : const SizedBox.shrink(),
          ),
        ),
      ],
    );
  }

  /// The nested room rows, or the inline empty row for an expanded space
  /// with no rooms.
  Widget _buildNestedRows(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    if (space.rooms.isEmpty) {
      return _NoRoomsYetRow(leadingInset: dimens.space6 * 2);
    }
    return Column(
      mainAxisSize: MainAxisSize.min,
      crossAxisAlignment: CrossAxisAlignment.start,
      children: <Widget>[
        for (final RailRoom room in space.rooms)
          _TreeRoomRow(
            room: room,
            joined: joinedTopics.contains(room.topic),
            selected: activeRoom == room.topic,
            // One space6 nesting indent beyond the space row's own space6
            // (UI-SPEC spacing contract).
            leadingInset: dimens.space6 * 2,
          ),
      ],
    );
  }
}

/// One trailing icon affordance of a space node (edit / "+"): a 40px
/// circular area at `textSecondary` (accent stays reserved for the header
/// "+"), lifted to 48px by ScaffoldPressable's internal touch target. The
/// innermost gesture wins the arena, so these taps never collapse the node.
class _NodeIconAffordance extends StatelessWidget {
  /// Creates the affordance; [onTap] opens the matching dialog mode.
  const _NodeIconAffordance({
    required this.icon,
    required this.semanticLabel,
    required this.onTap,
  });

  /// Icon glyph (Icons.edit_outlined / Icons.add).
  final IconData icon;

  /// Accessible name (icon-only pressable, WCAG 4.1.2).
  final String semanticLabel;

  /// Opens the dialog for this affordance's mode.
  final VoidCallback onTap;

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
          child: Icon(icon, color: context.palette.textSecondary),
        ),
      ),
    );
  }
}

/// One catalog room row (nested under a space, or standalone). Joined rows
/// follow the Phase 1 row contract (selected = accent tint + textPrimary,
/// else textSecondary, tap selects). Catalog-but-not-joined rows are
/// disabled pressables: ScaffoldDisabledOverlay at 0.40 plus
/// `Semantics(enabled: false)` (T-02-11) -- the dim/un-dim transition when
/// the joined set changes IS the autoJoinRooms observable (D-04).
class _TreeRoomRow extends StatelessWidget {
  /// Creates a [_TreeRoomRow] for [room].
  const _TreeRoomRow({
    required this.room,
    required this.joined,
    required this.selected,
    required this.leadingInset,
  });

  /// The catalog room rendered by this row.
  final RailRoom room;

  /// Whether the session is in this room (pushed RoomList membership).
  final bool joined;

  /// Whether this row is the active room.
  final bool selected;

  /// Leading padding (nesting indent for space children; base space6 for
  /// standalone rows).
  final double leadingInset;

  @override
  Widget build(BuildContext context) {
    final ScaffoldPalette palette = context.palette;
    final ScaffoldDimens dimens = context.dimens;
    final Color foreground = selected
        ? palette.textPrimary
        : palette.textSecondary;
    return ScaffoldPressable(
      disabled: !joined,
      onPressed: () => context.read<RailCubit>().selectRoom(room.topic),
      child: DecoratedBox(
        // Selected room = lightGreenPrimary at 10% alpha (UI-SPEC color
        // contract; ScaffoldColors.btnFilterSelected computes the tint).
        decoration: BoxDecoration(
          color: selected ? ScaffoldColors.btnFilterSelected : null,
          borderRadius: BorderRadius.circular(dimens.radiusMd),
        ),
        child: Padding(
          padding: EdgeInsetsDirectional.only(
            start: leadingInset,
            end: dimens.space6,
            top: dimens.space2,
            bottom: dimens.space2,
          ),
          child: ConstrainedBox(
            // UI-SPEC rail contract: 40px row content + space2*2 vertical
            // padding clears the 48px minimum touch target.
            constraints: BoxConstraints(
              minHeight: dimens.minTouchTarget - dimens.space2 * 2,
            ),
            child: Row(
              children: <Widget>[
                Icon(Icons.chat_bubble_outline, color: foreground),
                SizedBox(width: dimens.space3),
                Expanded(
                  child: Text(
                    room.name,
                    overflow: TextOverflow.ellipsis,
                    style: Theme.of(context).textTheme.bodyMedium?.copyWith(
                      color: foreground,
                    ),
                  ),
                ),
              ],
            ),
          ),
        ),
      ),
    );
  }
}

/// The inline empty row under an expanded space with no rooms (Body,
/// textSecondary, indented one nesting level, non-interactive).
class _NoRoomsYetRow extends StatelessWidget {
  /// Creates the row at [leadingInset] (one nesting level).
  const _NoRoomsYetRow({required this.leadingInset});

  /// Leading padding (one nesting level, matching the room rows).
  final double leadingInset;

  @override
  Widget build(BuildContext context) {
    final ScaffoldDimens dimens = context.dimens;
    return Padding(
      padding: EdgeInsetsDirectional.only(
        start: leadingInset,
        end: dimens.space6,
        top: dimens.space2,
        bottom: dimens.space2,
      ),
      child: ConstrainedBox(
        constraints: BoxConstraints(
          minHeight: dimens.minTouchTarget - dimens.space2 * 2,
        ),
        child: Align(
          alignment: AlignmentDirectional.centerStart,
          child: Text(
            'No rooms yet',
            overflow: TextOverflow.ellipsis,
            style: Theme.of(context).textTheme.bodyMedium?.copyWith(
              color: context.palette.textSecondary,
            ),
          ),
        ),
      ),
    );
  }
}
