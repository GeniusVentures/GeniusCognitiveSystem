/// RailCubit -- left-rail state for the GCS chat shell (D-11/D-21).
///
/// Thin state holder (D-04): the room list is REPLACED by the pushed
/// RoomList event decoded in [SessionCubit] -- never seeded locally. No
/// hardcoded default room list exists here (D-21: the smoke topics arrive
/// pushed from C++ after the port is registered).
library;

import 'package:flutter_bloc/flutter_bloc.dart';

/// Immutable rail state: the pushed room-topic list plus the active room.
class RailState {
  /// Creates a [RailState].
  const RailState({this.rooms = const <String>[], this.activeRoom});

  /// Room topics the session is currently in, as pushed by the most recent
  /// RoomList event (order preserved; owned unmodifiable).
  final List<String> rooms;

  /// Currently selected room topic; null when nothing is selected.
  final String? activeRoom;

  /// Returns a copy of this state with the given fields replaced. Passing
  /// [clearActiveRoom] clears the selection (nullable-field copyWith idiom).
  RailState copyWith({
    List<String>? rooms,
    String? activeRoom,
    bool clearActiveRoom = false,
  }) {
    return RailState(
      rooms: rooms ?? this.rooms,
      activeRoom: clearActiveRoom ? null : (activeRoom ?? this.activeRoom),
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
}
