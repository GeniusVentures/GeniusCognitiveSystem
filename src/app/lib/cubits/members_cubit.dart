/// MembersCubit -- per-scope membership roster state for the GCS chat shell
/// (D-04/D-11).
///
/// Thin state holder (D-04): the roster IS the pushed `MemberList` event --
/// members arrive as decoded `MemberRecord`s dispatched by [SessionCubit] and
/// are mapped here onto [MemberInfo]s. Every scope's roster is REPLACED in
/// full by each push, never appended and never synthesized locally: the C++
/// side owns membership authority (D-08), Dart only mirrors the pushed
/// snapshot.
library;

import 'package:flutter_bloc/flutter_bloc.dart';

import 'package:flutter_app/generated/proto/gcs_chat.pb.dart';

/// Five-tier role display names (UI-SPEC: the exact badge text). One home per
/// label -- nothing else in the app inlines these strings.
const String kRoleLabelSuperAdmin = 'Super Admin';
const String kRoleLabelAdmin = 'Admin';
const String kRoleLabelModerator = 'Moderator';
const String kRoleLabelMember = 'Member';
const String kRoleLabelGuest = 'Guest';

/// Role (proto) -> D-08 five-tier display name. Unspecified/unknown roles
/// degrade to [kRoleLabelMember]: C++ always stamps a granted role
/// (MEMB-02), so the fallback is unreachable for pushed records.
const Map<Role, String> kRoleLabels = <Role, String>{
  Role.ROLE_SUPER_ADMIN: kRoleLabelSuperAdmin,
  Role.ROLE_ADMIN: kRoleLabelAdmin,
  Role.ROLE_MODERATOR: kRoleLabelModerator,
  Role.ROLE_MEMBER: kRoleLabelMember,
  Role.ROLE_GUEST: kRoleLabelGuest,
};

/// Returns the five-tier display name for [role] (D-08).
String roleLabel(Role role) => kRoleLabels[role] ?? kRoleLabelMember;

/// D-07 pending-member badge text for a dormant removal (UI-SPEC copy).
const String kPendingRemovalLabel = 'Pending removal';

/// D-07 pending-member badge prefix for a dormant role change; the badge text
/// is this prefix followed by the target role's display name (UI-SPEC copy,
/// e.g. `Pending: Member`).
const String kPendingRoleChangeLabelPrefix = 'Pending: ';

/// D-07 pending badge text for a dormant scope deletion (UI-SPEC copy). A
/// scope-level op is carried on the scope's own record, so its badge text has
/// no member-scope counterpart.
const String kPendingDeletionLabel = 'Pending deletion';

/// D-07 badge text for a dormant member-scope op: the target role of a
/// dormant role change makes the badge informative -- the record's own
/// `role` still holds the pre-change tier until the approval lands.
String _pendingLabel(MemberRecord record) {
  switch (record.pendingAction) {
    case PendingAction.PENDING_ACTION_REMOVE:
      return kPendingRemovalLabel;
    case PendingAction.PENDING_ACTION_ROLE_CHANGE:
      return '$kPendingRoleChangeLabelPrefix${roleLabel(record.pendingRole)}';
    default:
      // Scope-level actions (PENDING_ACTION_DELETE) are carried on the scope's
      // own record, never on a member record -- no member badge text exists.
      return '';
  }
}

/// A single member row derived from one pushed [MemberRecord] (D-01/D-04/D-07).
class MemberInfo {
  /// Creates a [MemberInfo].
  const MemberInfo({
    required this.address,
    required this.role,
    required this.explicitLeave,
    required this.pending,
    required this.pendingLabel,
    required this.self,
  });

  /// Maps a pushed [MemberRecord] onto its display row.
  ///
  /// [selfAddress] is the pushed `MemberList.selfAddress` -- the local wallet
  /// C++ stamped alongside the roster. It is never resolved from the session
  /// and never defaulted to `''`, so the "You" marker always comes from
  /// authority rather than a guess.
  ///
  /// `pending` is [MemberRecord.approvedBy] emptiness combined with a
  /// non-unspecified [MemberRecord.pendingAction] (D-07): the dormancy marker
  /// lives entirely on the pushed record, never computed locally.
  factory MemberInfo.fromRecord(
    MemberRecord record, {
    required String selfAddress,
  }) {
    final bool pending =
        record.approvedBy.isEmpty &&
        record.pendingAction != PendingAction.PENDING_ACTION_UNSPECIFIED;
    return MemberInfo(
      address: record.address,
      role: roleLabel(record.role),
      explicitLeave: record.explicitLeave,
      pending: pending,
      pendingLabel: pending ? _pendingLabel(record) : '',
      self: record.address == selfAddress,
    );
  }

  /// Wallet identity of the member (D-03; stable, never re-minted).
  final String address;

  /// Five-tier display name derived from the C++-stamped proto role (D-08).
  final String role;

  /// D-02 derived-join precedence term: the member explicitly left this scope.
  final bool explicitLeave;

  /// Whether a D-07 destructive op is dormant on this member (awaiting the
  /// Super Admin's approval).
  final bool pending;

  /// D-07 badge text for the dormant op ([kPendingRemovalLabel] or
  /// `Pending: <role>`); empty when the member has no pending op.
  final String pendingLabel;

  /// Whether this row is the local user, per the pushed `selfAddress` (D-04).
  final bool self;
}

/// The local user's own row in a pushed [roster], or null when the roster is
/// absent or carries no `self` row.
///
/// One home for the "who am I in this scope" read: the members dialog and the
/// rail's viewer-role gates both call it, and a null result means the role is
/// UNKNOWN (no roster pushed yet) -- gated affordances stay hidden rather than
/// guessing a tier (D-08: presentation gating reads the pushed role only).
MemberInfo? viewerOf(List<MemberInfo>? roster) {
  if (roster == null) {
    return null;
  }
  for (final MemberInfo member in roster) {
    if (member.self) {
      return member;
    }
  }
  return null;
}

/// Whether a viewer whose pushed role label is [viewerRole] may manage the
/// scope's membership (Super Admin or Admin; UI-SPEC roster anatomy).
/// Presentation only (T-04-07-EoP) -- the C++ write guards are the
/// enforcement, this decides which affordances the rail/roster render.
///
/// One home for the role->capability rule: the members dialog and the rail's
/// Delete affordance both gate on it.
bool viewerCanManageMembers(String? viewerRole) =>
    viewerRole == kRoleLabelSuperAdmin || viewerRole == kRoleLabelAdmin;

/// Whether a viewer whose pushed role label is [viewerRole] is the scope's
/// Super Admin -- the single approver of dormant destructive ops (D-07).
/// Presentation only (T-04-07-EoP). A null [viewerRole] (roster not pushed)
/// is never the approver.
bool viewerIsSuperAdmin(String? viewerRole) =>
    viewerRole == kRoleLabelSuperAdmin;

/// Immutable membership state: the per-scope rosters, each the full
/// replacement pushed by the most recent `MemberList` for that scope (D-04).
class MembersState {
  /// Creates a [MembersState].
  const MembersState({this.byScope = const <String, List<MemberInfo>>{}});

  /// Rosters keyed by scope id (space id or standalone room id, D-01). Each
  /// value is the complete pushed roster for that scope, owned unmodifiable.
  final Map<String, List<MemberInfo>> byScope;

  /// Returns a copy of this state with the given fields replaced.
  MembersState copyWith({Map<String, List<MemberInfo>>? byScope}) {
    return MembersState(byScope: byScope ?? this.byScope);
  }
}

/// Cubit holding the per-scope membership rosters (D-04/D-11).
///
/// [setMembers] is called only from the pushed `MemberList` dispatch in
/// [SessionCubit]; nothing else mutates the rosters.
class MembersCubit extends Cubit<MembersState> {
  /// Creates a [MembersCubit] with no rosters (they arrive pushed).
  MembersCubit() : super(const MembersState());

  /// REPLACES the roster of [scopeId] with the pushed [members] (source of
  /// truth = the `MemberList` event, D-04): a push is always the complete
  /// roster, so the prior entry is discarded, never merged. [selfAddress] is
  /// the pushed `MemberList.selfAddress` used to derive the "You" marker.
  /// No locally synthesized rosters and no id/key minting.
  void setMembers(
    String scopeId,
    List<MemberRecord> members,
    String selfAddress,
  ) {
    final List<MemberInfo> roster = List<MemberInfo>.unmodifiable(<MemberInfo>[
      for (final MemberRecord record in members)
        MemberInfo.fromRecord(record, selfAddress: selfAddress),
    ]);
    final Map<String, List<MemberInfo>> next =
        Map<String, List<MemberInfo>>.unmodifiable(<String, List<MemberInfo>>{
          ...state.byScope,
          scopeId: roster,
        });
    emit(state.copyWith(byScope: next));
  }
}
