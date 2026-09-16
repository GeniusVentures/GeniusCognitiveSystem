/// RoomRail -- left rail of the GCS chat shell (D-21).
///
/// Renders the room-topic list held by [RailCubit] as selectable rows. The
/// list is REPLACED wholesale by the pushed RoomList event (D-21/D-26: the
/// rail never hardcodes a room set -- the smoke topics arrive pushed from
/// C++ after `gcs_subscribe` registers the port). Empty state renders a
/// scaffold status panel, never a crash. Shell-side one-off widget (plain
/// Dart per D-11) composed from scaffold atoms (D-10/D-14).
library;

import 'package:flutter/material.dart';
import 'package:flutter_bloc/flutter_bloc.dart';
import 'package:frontend_scaffold/components/scaffold_pressable.dart';
import 'package:frontend_scaffold/components/scaffold_state_view.dart';
import 'package:frontend_scaffold/theme/scaffold_colors.dart';
import 'package:frontend_scaffold/theme/scaffold_dimens.dart';
import 'package:frontend_scaffold/theme/scaffold_palette.dart';
import 'package:frontend_scaffold/theme/scaffold_theme.dart';

import '../cubits/rail_cubit.dart';

/// Short display name of a pushed room topic (`gcs/chat/smoke-test` ->
/// `smoke-test`); topics without a slash render unchanged.
String roomDisplayName(String roomTopic) {
  final int lastSlash = roomTopic.lastIndexOf('/');
  if (lastSlash < 0 || lastSlash == roomTopic.length - 1) {
    return roomTopic;
  }
  return roomTopic.substring(lastSlash + 1);
}

/// The left room rail of the GCS chat shell (D-21).
///
/// Selection highlights the active room (UI-SPEC color contract: accent at
/// 10% alpha via `ScaffoldColors.btnFilterSelected`); tapping a row drives
/// [RailCubit.selectRoom], which changes the active-room projection consumed
/// by the composer hint and message flow.
class RoomRail extends StatelessWidget {
  /// Creates a [RoomRail].
  const RoomRail({super.key});

  /// Rail width in logical pixels (UI-SPEC rail contract: 280 expanded).
  static const double kWidth = 280.0;

  /// Section-header letter spacing (UI-SPEC typography contract, label row).
  static const double kHeaderLetterSpacing = 0.5;

  @override
  Widget build(BuildContext context) {
    final ScaffoldPalette palette = context.palette;
    final ScaffoldDimens dimens = context.dimens;
    return ColoredBox(
      color: palette.grayPrimary,
      child: BlocBuilder<RailCubit, RailState>(
        builder: (BuildContext context, RailState rail) {
          if (rail.rooms.isEmpty) {
            return const ScaffoldStateView(
              state: 'empty',
              emptyHeadline: 'No rooms yet',
              emptyBody:
                  "This space doesn't have any rooms. Rooms will appear here "
                  "once they're created.",
            );
          }
          return Column(
            children: <Widget>[
              Padding(
                padding: EdgeInsets.fromLTRB(
                  dimens.space8,
                  dimens.space8,
                  dimens.space8,
                  dimens.space4,
                ),
                child: Align(
                  alignment: AlignmentDirectional.centerStart,
                  child: Text(
                    'Rooms',
                    style: Theme.of(context).textTheme.titleSmall?.copyWith(
                      color: palette.textSecondary,
                      letterSpacing: kHeaderLetterSpacing,
                    ),
                  ),
                ),
              ),
              Expanded(
                child: ListView.builder(
                  itemCount: rail.rooms.length,
                  itemBuilder: (BuildContext context, int index) {
                    final String roomTopic = rail.rooms[index];
                    return _RoomRow(
                      roomTopic: roomTopic,
                      selected: roomTopic == rail.activeRoom,
                    );
                  },
                ),
              ),
            ],
          );
        },
      ),
    );
  }
}

/// One selectable room row of the rail.
class _RoomRow extends StatelessWidget {
  /// Creates a [_RoomRow] for [roomTopic].
  const _RoomRow({required this.roomTopic, required this.selected});

  /// Full pushed room topic (`gcs/chat/<name>`); the label shows the short
  /// name via [roomDisplayName].
  final String roomTopic;

  /// Whether this row is the active room.
  final bool selected;

  @override
  Widget build(BuildContext context) {
    final ScaffoldPalette palette = context.palette;
    final ScaffoldDimens dimens = context.dimens;
    final Color foreground = selected
        ? palette.textPrimary
        : palette.textSecondary;
    return ScaffoldPressable(
      onPressed: () => context.read<RailCubit>().selectRoom(roomTopic),
      child: DecoratedBox(
        // Selected room = lightGreenPrimary at 10% alpha (UI-SPEC color
        // contract; ScaffoldColors.btnFilterSelected computes the tint).
        decoration: BoxDecoration(
          color: selected ? ScaffoldColors.btnFilterSelected : null,
          borderRadius: BorderRadius.circular(dimens.radiusMd),
        ),
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
                Icon(Icons.chat_bubble_outline, color: foreground),
                SizedBox(width: dimens.space3),
                Expanded(
                  child: Text(
                    roomDisplayName(roomTopic),
                    overflow: TextOverflow.ellipsis,
                    style: Theme.of(
                      context,
                    ).textTheme.bodyMedium?.copyWith(color: foreground),
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
