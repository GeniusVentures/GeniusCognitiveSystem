/// RailCubit -- left-rail state for the GCS chat shell (D-11/D-21).
///
/// Thin state holder (D-04): the room list is REPLACED by the pushed
/// RoomList event decoded in [SessionCubit] -- never seeded locally. No
/// hardcoded default room list exists here (D-21: the smoke topics arrive
/// pushed from C++ after the port is registered).
library;

import 'package:flutter_bloc/flutter_bloc.dart';

import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';

/// A catalog room node in the rail tree (D-01: opaque id, name is display
/// metadata). Topic is derived, never stored.
class RailRoom {
  /// Creates a [RailRoom].
  const RailRoom({required this.id, required this.name});

  /// Opaque C++-minted room id (identity; never changes).
  final String id;

  /// Human display name (mutable metadata on the record).
  final String name;

  /// Derived pub/sub topic for this room (D-01 namespacing).
  String get topic => 'gcs/chat/$id';
}

/// A catalog space node in the rail tree, owning its grouped rooms.
class RailSpace {
  /// Creates a [RailSpace].
  const RailSpace({
    required this.id,
    required this.name,
    required this.isPublic,
    required this.autoJoinRooms,
    this.rooms = const <RailRoom>[],
  });

  /// Opaque C++-minted space id (identity; never changes).
  final String id;

  /// Human display name (mutable metadata on the record).
  final String name;

  /// Whether the space is public (display metadata only in Phase 2).
  final bool isPublic;

  /// D-04 derived-join source of truth (edit surface toggles this).
  final bool autoJoinRooms;

  /// Rooms grouped under this space by `parent_space_id` (pushed order).
  final List<RailRoom> rooms;
}

/// Immutable rail state: the pushed joined-topic list plus the active room
/// (session-membership lifecycle) AND the pushed catalog tree (persistent
/// lifecycle, D-02 -- the two never overwrite each other).
class RailState {
  /// Creates a [RailState].
  const RailState({
    this.rooms = const <String>[],
    this.activeRoom,
    this.spaces = const <RailSpace>[],
    this.standaloneRooms = const <RailRoom>[],
    this.treeReceived = false,
  });

  /// Room topics the session is currently in, as pushed by the most recent
  /// RoomList event (order preserved; owned unmodifiable).
  final List<String> rooms;

  /// Currently selected room topic; null when nothing is selected.
  final String? activeRoom;

  /// Catalog spaces with their grouped rooms, as pushed by the most recent
  /// SpaceTree event (full replacement; owned unmodifiable).
  final List<RailSpace> spaces;

  /// Rooms with an empty parent space id (standalone, D-01 unified model).
  final List<RailRoom> standaloneRooms;

  /// Whether any SpaceTree has arrived yet -- separates "loading" from
  /// "empty catalog" during startup.
  final bool treeReceived;

  /// Returns a copy of this state with the given fields replaced. Passing
  /// [clearActiveRoom] clears the selection (nullable-field copyWith idiom).
  RailState copyWith({
    List<String>? rooms,
    String? activeRoom,
    bool clearActiveRoom = false,
    List<RailSpace>? spaces,
    List<RailRoom>? standaloneRooms,
    bool? treeReceived,
  }) {
    return RailState(
      rooms: rooms ?? this.rooms,
      activeRoom: clearActiveRoom ? null : (activeRoom ?? this.activeRoom),
      spaces: spaces ?? this.spaces,
      standaloneRooms: standaloneRooms ?? this.standaloneRooms,
      treeReceived: treeReceived ?? this.treeReceived,
    );
  }
}

/// Cubit holding the room rail state (D-11 per-screen cubit).
///
/// [setRooms] is called only from the pushed RoomList dispatch in
/// [SessionCubit]; [selectRoom] is called only from rail row taps.
class RailCubit extends Cubit<RailState> {
  /// Creates a [RailCubit] with an empty rail (rooms arrive pushed).
  RailCubit() : super(const RailState());

  /// REPLACES the room list with the pushed set (source of truth = the
  /// RoomList event, D-21/D-26). A selection no longer present in the pushed
  /// list is cleared so it can never dangle.
  void setRooms(List<String> rooms) {
    final List<String> next = List<String>.unmodifiable(rooms);
    final String? active = state.activeRoom;
    if (active == null || next.contains(active)) {
      emit(state.copyWith(rooms: next));
      return;
    }
    emit(state.copyWith(rooms: next, clearActiveRoom: true));
  }

  /// Sets the active room; ignores topics outside the pushed list and
  /// redundant re-selections of the active room.
  void selectRoom(String roomTopic) {
    if (!state.rooms.contains(roomTopic) || state.activeRoom == roomTopic) {
      return;
    }
    emit(state.copyWith(activeRoom: roomTopic));
  }

  /// REPLACES the catalog tree with the pushed records (source of truth =
  /// the SpaceTree event, D-02). Maps each [SpaceRecord] to a [RailSpace]
  /// and each [RoomRecord] to a [RailRoom]; rooms with a non-empty
  /// [RoomRecord.parentSpaceId] are grouped under the matching space in
  /// pushed order, rooms with an empty parent become standalone (D-01).
  /// Never touches the joined view (`rooms`/`activeRoom`) -- RoomList
  /// pushes keep driving those via [setRooms].
  void setTree(List<SpaceRecord> spaces, List<RoomRecord> rooms) {
    final Map<String, List<RailRoom>> grouped = <String, List<RailRoom>>{};
    final List<RailSpace> mappedSpaces = <RailSpace>[];
    for (final SpaceRecord record in spaces) {
      grouped[record.id] = <RailRoom>[];
      mappedSpaces.add(
        RailSpace(
          id: record.id,
          name: record.name,
          isPublic: record.isPublic,
          autoJoinRooms: record.autoJoinRooms,
        ),
      );
    }
    final List<RailRoom> standalone = <RailRoom>[];
    for (final RoomRecord record in rooms) {
      final RailRoom room = RailRoom(id: record.id, name: record.name);
      final List<RailRoom>? siblings = grouped[record.parentSpaceId];
      if (record.parentSpaceId.isEmpty || siblings == null) {
        standalone.add(room);
        continue;
      }
      siblings.add(room);
    }
    final List<RailSpace> frozenSpaces = <RailSpace>[
      for (final RailSpace space in mappedSpaces)
        _withRooms(space, grouped[space.id]!),
    ];
    emit(
      state.copyWith(
        spaces: List<RailSpace>.unmodifiable(frozenSpaces),
        standaloneRooms: List<RailRoom>.unmodifiable(standalone),
        treeReceived: true,
      ),
    );
  }

  /// Returns a copy of [space] carrying [rooms] (immutable mapping helper).
  RailSpace _withRooms(RailSpace space, List<RailRoom> rooms) {
    return RailSpace(
      id: space.id,
      name: space.name,
      isPublic: space.isPublic,
      autoJoinRooms: space.autoJoinRooms,
      rooms: List<RailRoom>.unmodifiable(rooms),
    );
  }
}
