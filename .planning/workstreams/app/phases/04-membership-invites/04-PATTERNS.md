# Phase 4: Membership & Invites - Pattern Map

**Mapped:** 2026-09-27
**Files analyzed:** 20 (7 new, 13 modified)
**Analogs found:** 20 / 20 (all have a same-role or cross-component analog; one borrows across components)

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `src/lib/gcs_membership.hpp` | component/service | CRUD (per-key LWW) + event-driven push | `src/lib/gcs_entity_store.hpp` | exact |
| `src/lib/gcs_membership.cpp` | component/service | CRUD + event-driven | `src/lib/gcs_entity_store.cpp` + `gcs_messaging.cpp` `QueryHistory` | exact |
| `src/proto/gcs_chat.proto` | config/schema | append-only schema | `src/proto/gcs_chat.proto` (self) | exact |
| `src/lib/gcs_entity_store.{hpp,cpp}` | model/store | CRUD | self (creator stamp) | exact |
| `src/lib/gcs_crypto.{hpp,cpp}` | utility/crypto | transform | self (`EncryptPayload`/`DecryptPayload`) + `gcs_messaging` mutex | role-match |
| `src/lib/gcs_messaging.{hpp,cpp}` | service | event-driven | self (`ApplyMessage`/`CryptoSeam`) | exact |
| `src/ffi/gcs_core_ffi.cpp` | controller/FFI | request-response | self (dispatch switch + event builders) | exact |
| `src/app/lib/cubits/members_cubit.dart` | cubit/store | transform (full replacement) | `src/app/lib/cubits/rail_cubit.dart` `setTree` | exact |
| `src/app/lib/cubits/session_cubit.dart` | cubit/controller | event-driven | self (`_dispatchEvent`) | exact |
| `src/app/lib/shell/members_dialog.dart` | shell widget | request-response | `src/app/lib/shell/space_room_dialog.dart` | exact |
| `src/app/lib/shell/join_dialog.dart` | shell widget | request-response | `src/app/lib/shell/space_room_dialog.dart` (single-field mode) | exact |
| `src/app/lib/shell/room_rail.dart` | shell widget | request-response | self (`_NodeIconAffordance`/`_TreeRoomRow`) | exact |
| `src/app/lib/shell/gcs_shell.dart` | shell wiring | — | self (`MultiBlocProvider`) | exact |
| `test/test_gcs_membership.cpp` | test | CRUD | `test/test_gcs_entities.cpp` | exact |
| `test/test_gcs_membership_multinode.cpp` | test | event-driven | `test/test_gcs_messaging_multinode.cpp` | exact |
| `test/test_gcs_crypto.cpp` | test | transform | self (pure unit) | exact |
| `test/test_gcs_ffi.cpp` | test | request-response | self (ABI fixture) | exact |

Codegen (regenerated, not hand-written; planner must trigger the app codegen step after the proto change):
`src/app/lib/generated/proto/gcs_chat.pb.dart`, `.pbenum.dart`, `.pbjson.dart`.

---

## Pattern Assignments

### `src/lib/gcs_membership.hpp` / `.cpp` (component, CRUD + event-driven push)

**Analog:** `src/lib/gcs_entity_store.{hpp,cpp}` (same role: C++-owned record store over `CoreSession` Put/Get, per-key LWW, tombstone-skip) with the `QueryKeyValues` prefix-scan from `gcs_messaging.cpp`.

**Imports pattern** (`gcs_entity_store.hpp:19-25`, `gcs_messaging.hpp:30-41`):
```cpp
#include <map>
#include <string>
#include <vector>

#include "gcs_core.hpp"                 // CoreSession Put/Get/QueryKeyValues pass-throughs
#include "gcs_storage/common/error.hpp" // sgns::gcs::Error (no NotFound code)
#include "proto/gcs_chat.pb.h"          // MemberRecord / MemberList / Role
```

**Constructor + deferred registration** (`gcs_entity_store.hpp:55`, `gcs_messaging.hpp:99-100` + `gcs_messaging.cpp:60-71`): store dependencies only, no I/O. Membership must inject the local wallet address and an `EventSink`, exactly as `Messaging` does:
```cpp
// gcs_messaging.hpp:56-58
using EventSink = std::function<void(const chat::GcsEvent &)>;

// gcs_messaging.cpp:60-71 — constructor stores deps, computes derived flags
Messaging::Messaging(CoreSession &session, std::string senderAddress,
                     EventSink sink, CryptoSeam crypto)
    : m_session(session), m_sender(std::move(senderAddress)), m_sink(std::move(sink)), ...
```

**ID minting** (`gcs_entity_store.cpp:110-119` — CR-01-safe salted id; copy for member/invite ids, prefix `member-`/`invite-`):
```cpp
std::string EntityStore::NextEntityId(const char *prefix)
{
    static const std::string seed = [] {
        const int64_t      nowMs        = NowMs();
        std::random_device randomDevice;
        return std::to_string(nowMs) + "-" + std::to_string(randomDevice());
    }();
    static std::atomic<uint64_t> sequence{ 0 };
    return std::string(prefix) + seed + "-" + std::to_string(sequence.fetch_add(1));
}
```

**Core record write pattern** (`gcs_entity_store.cpp:188-221` — full-state write, C++ stamps every authority field):
```cpp
outcome::result<SpaceRecord> EntityStore::CreateSpace(...)
{
    SpaceRecord record;
    const std::string id = NextEntityId(kSpaceIdPrefix);
    record.set_id(id);
    record.set_name(name);
    record.set_created_at_ms(nowMs);
    record.set_updated_at_ms(nowMs);
    record.set_deleted(false);
    record.set_deleted_at_ms(0);
    auto putResult = m_session.Put(std::string(kSpacesKeyPrefix) + id, record.SerializeAsString());
    if (!putResult.has_value()) { return putResult.error(); }
    m_spaces.emplace(id, record);
    return record;
}
```
D-03 role-change rewrites the member record as full state, preserving immutables, exactly `UpdateSpace` (`gcs_entity_store.cpp:265-294`): copy the stored record's `created_at_ms`/tombstone fields, re-stamp `updated_at_ms`, Put, then update the in-memory map.

**Member enumeration — prefix scan** (`gcs_messaging.cpp:309-384` `QueryHistory`, the exact D-01 pattern; member key prefix `gcs/members/<scopeId>/<wallet>`):
```cpp
const std::string prefix = std::string(kMembersKeyPrefix) + scopeId + "/";
auto scanResult = m_session.QueryKeyValues(prefix);
if (!scanResult.has_value()) { return scanResult.error(); }
chat::MemberList list;
list.set_scope_id(scopeId);
for (const auto &entry : scanResult.value())
{
    chat::MemberRecord record;
    if (!record.ParseFromString(entry.second)) { spdlog::warn("...skip..."); continue; }
    if (record.deleted()) { continue; }   // tombstone-skip (never key removal)
    *list.add_member() = record;
}
```

**Push pattern** (`gcs_messaging.cpp:386-393`): wrap the built message/roster in a `GcsEvent` and invoke `m_sink` — the same shape the FFI wires to `PostToDart`.

**Error handling** (`gcs_entity_store.cpp:226-234`): reject invalid input with `spdlog::warn(...)` + `return outcome::failure(sgns::gcs::Error::GcsDbError);` — the Error domain has no NotFound code; `GcsDbError` doubles as the generic rejection.

---

### `src/proto/gcs_chat.proto` (append-only schema extension)

**Analog:** self. Follow the existing append-only oneof discipline exactly.

**Command oneof extension** (`gcs_chat.proto:55-63` — add arms after `update_space = 5`; next free `= 6` onward):
```proto
message GcsCommand {
  oneof payload {
    JoinTopicCommand join_topic = 1;
    SendTextCommand send_text = 2;
    CreateSpaceCommand create_space = 3;
    CreateRoomCommand create_room = 4;
    UpdateSpaceCommand update_space = 5;
    // Phase 4 additions: invite/redeem/change-role/remove/leave/delete/approve
  }
}
```

**Event oneof extension** (`gcs_chat.proto:95-104` — `space_tree = 5`, `message_history = 6`; next free `= 7`):
```proto
message GcsEvent {
  oneof payload {
    ChatMessageState message = 1;
    RoomList room_list = 2;
    Readiness readiness = 3;
    ErrorNotice error = 4;
    SpaceTree space_tree = 5;
    MessageHistory message_history = 6;
    // Phase 4: MemberList member_list = 7; InviteLink invite_link = 8;
  }
}
```

**Record + enum precedents:** `MessageRole` enum (`gcs_chat.proto:25-31`) for the new five-tier `Role`; `SpaceRecord`/`RoomRecord` (`gcs_chat.proto:112-134`) for the `creator`/`members_can_invite` fields (P1/P2/P3: `SpaceRecord.creator = 9`, `SpaceRecord.members_can_invite = 10`, `RoomRecord.creator = 8`, `RoomRecord.members_can_invite = 9`); `SpaceTree` flat-repeated-record push (`gcs_chat.proto:168-173`) for the flat `MemberList`; data-only command shapes (`gcs_chat.proto:147-166`) for the new command messages.

---

### `src/lib/gcs_entity_store.{hpp,cpp}` (creator stamping)

**Analog:** self. Add `creator`/`members_can_invite` stamping where `CreateSpace`/`CreateRoom` already stamp authority fields (`gcs_entity_store.cpp:188-263`):
```cpp
record.set_id(id);
record.set_name(name);
record.set_created_at_ms(nowMs);
// NEW: record.set_creator(<injected wallet address>);
```
Pitfall P1: the `EntityStore` constructor currently receives only `CoreSession&` (`gcs_entity_store.hpp:55`). Per the RESEARCH recommendation, either add the local wallet address to the constructor (mirroring `Messaging`'s `senderAddress` injection, `gcs_messaging.hpp:99-100`) or let the membership component own creator stamping — a planner decision. Stamp creator for standalone rooms (`parent_space_id` empty) in `CreateRoom` (P2).

---

### `src/lib/gcs_crypto.{hpp,cpp}` (per-room key store + Wrap/Unwrap)

**Analog:** self for the EVP surface; `gcs_messaging.hpp` mutex map for the new shared state.

**Wrap/Unwrap** compose from the existing GCM envelope (`gcs_crypto.cpp:68-177`): copy `EncryptPayload`/`DecryptPayload` but pass the caller-supplied 32-byte invite key directly to `EVP_EncryptInit_ex`/`EVP_DecryptInit_ex` instead of `DeriveRoomKey(roomTopic)`; the plaintext is the 32-byte room key. Nonce stays 12 fresh `RAND_bytes` bytes, tag 16.

**WR-02 site** (`gcs_crypto.cpp:46-49`): swap only the HKDF ikm:
```cpp
&& ( EVP_PKEY_CTX_set1_hkdf_key(
       pctx,
       reinterpret_cast<const unsigned char *>( roomKey.data() ),  // was roomTopic.data()
       static_cast<int>( roomKey.size() ) )                        // was roomTopic.size()
   == 1 )
```
`kMessagesHkdfSalt` / `kMessagesHkdfInfo` (`gcs_crypto.hpp:50-52`) stay STABLE.

**Key store (the documented statelessness exception)** — store only 32-byte bytes under `std::mutex`; never cache EVP contexts (P7). Borrow the mutex-guarded map idiom from `gcs_messaging.hpp:241-243`:
```cpp
std::mutex m_seenMutex;                     // guards the map
std::unordered_map<std::string, std::vector<unsigned char>> m_roomKeys; // topic -> 32-byte key
```
Add a `RoomKeyFor(roomTopic)` accessor that the injected `CryptoSeam` resolves through; `Messaging` never includes OpenSSL directly (`gcs_messaging.hpp:20-23`).

---

### `src/lib/gcs_messaging.{hpp,cpp}` (decrypt path resolves key from store)

**Analog:** self. `CryptoSeam` (`gcs_messaging.hpp:72-79`) is the seam to extend; `ApplyMessage`'s decrypt-first skip-and-log posture (`gcs_messaging.cpp:258-307`) and `QueryHistory`'s decrypt path (`gcs_messaging.cpp:309-384`) are unchanged in shape — only the key source swaps. Keep the wrong-key `spdlog::warn("...skipping undecryptable...")` + `continue`/`return` posture verbatim.

---

### `src/ffi/gcs_core_ffi.cpp` (dispatch arms + MemberList event + members heal callback)

**Analog:** self.

**Command dispatch + validation arm shape** (`gcs_core_ffi.cpp:1137-1165` `kCreateSpace` / `kUpdateSpace`):
```cpp
case gcs::chat::GcsCommand::kChangeRole:
{
    const auto &cmd = command.change_role();
    if (cmd.scope_id().empty() || cmd.target().empty())
    {
        PostErrorNotice("change_role rejected: scope_id and target must be non-empty");
        return GCS_ERROR_INVALID_ARGUMENT;
    }
    // D-03/D-08: write-time guard (never CRDT semantics)
    if (!g_membership->CanChangeRole(cmd.scope_id(), cmd.target(), cmd.role()))
    {
        PostErrorNotice("change_role rejected: permission denied");
        return GCS_ERROR_INVALID_ARGUMENT;
    }
    // ... full-state rewrite via g_membership ...
}
```

**Event builder** (`gcs_core_ffi.cpp:541-554` `BuildSpaceTreeEvent` — the D-04 `MemberList` shape):
```cpp
gcs::chat::GcsEvent BuildMemberListEvent(const std::string &scopeId)
{
    gcs::chat::GcsEvent event;
    auto members = g_membership->MembersFor(scopeId);
    if (members.has_value()) { *event.mutable_member_list() = std::move(members.value()); }
    return event;
}
```

**Members heal callback** (`gcs_core_ffi.cpp:902-933` — the `/gcs/messages/.*` callback; register `/gcs/members/.*` with the SAME teardown-intake-gate + copy-pointer-outside-lock + `g_inFlight` discipline):
```cpp
if (!g_session->RegisterNewElementCallback(
       gcs::Membership::kMembersKeyCallbackPattern,
       [](const std::string &key, const std::string &value)
       {
           if (g_receivingDisabled.load()) { return; }   // teardown intake gate (CR-02)
           gcs::Membership* membership = nullptr;
           {
               std::lock_guard<std::mutex> lock(g_mutex);
               membership = g_membership.get();
               if (membership != nullptr) { g_inFlight += 1; }
           }
           if (membership == nullptr) { return; }
           membership->OnMemberArrived(key, value);
           { std::lock_guard<std::mutex> lock(g_mutex); g_inFlight -= 1; }
           g_idleCond.notify_all();
       })
    .has_value())
{ spdlog::error("gcs_ffi: member receive-callback registration failed"); }
```

**Wallet-address identity** (`gcs_core_ffi.cpp:885`): `GeniusSDKGetAddress().address` — reuse this exact stamp for the local member identity and for `SpaceRecord.creator`.

**Supporting helpers:** `PostErrorNotice` (`gcs_core_ffi.cpp:587-592`), `PostToDart` (`494-512`), `BuildRoomListEvent` (`521-530`), `SerializeGcsEvent` (`471-482`).

---

### `src/app/lib/cubits/members_cubit.dart` (cubit, full-replacement roster)

**Analog:** `src/app/lib/cubits/rail_cubit.dart` (`setTree` full-replacement + `copyWith` state idiom).

**State + full-replacement pattern** (`rail_cubit.dart:89-104`, `148-183`):
```dart
class MembersState {
  const MembersState({this.byScope = const <String, List<MemberInfo>>{}});
  final Map<String, List<MemberInfo>> byScope;   // scopeId -> full-replacement roster
  MembersState copyWith({Map<String, List<MemberInfo>>? byScope}) =>
      MembersState(byScope: byScope ?? this.byScope);
}

class MembersCubit extends Cubit<MembersState> {
  MembersCubit() : super(const MembersState());

  /// REPLACES the roster for [scopeId] (source of truth = the pushed MemberList event, D-04).
  void setMembers(String scopeId, List<MemberRecord> members) {
    final Map<String, List<MemberInfo>> next = Map<String, List<MemberInfo>>.of(state.byScope);
    next[scopeId] = List<MemberInfo>.unmodifiable([
      for (final MemberRecord m in members) MemberInfo.fromRecord(m),
    ]);
    emit(state.copyWith(byScope: next));
  }
}
```
Dart stays a dumb read of the pushed `MemberList`; pending state derives from each entry's empty `approved_by`, never locally computed (UI-SPEC State Contracts).

---

### `src/app/lib/cubits/session_cubit.dart` (dispatch arms)

**Analog:** self `_dispatchEvent` (`session_cubit.dart:426-467`) — add two arms in the same `if (event.hasX())` chain before the `error` arm:
```dart
if (event.hasMemberList()) {
  _membersCubit?.setMembers(event.memberList.scopeId, event.memberList.member);
  return;
}
if (event.hasInviteLink()) {
  _pendingInviteLink = event.inviteLink.url;   // route to open invite view or queue
  return;
}
```
Constructor mirrors `SessionCubit`'s existing `RailCubit?`/`MessageFlowCubit?` optional-injection params (`session_cubit.dart:123-138`); `GcsCommandTransport.publishCommand` (`346-367`) is reused unchanged for the new command arms.

---

### `src/app/lib/shell/members_dialog.dart` / `join_dialog.dart` (shell dialogs)

**Analog:** `src/app/lib/shell/space_room_dialog.dart` (the `ResponsiveDrawer.show` + `_DialogForm` ChangeNotifier + `_DialogFields`/`_DialogFooter` subtrees) and `room_rail.dart` `_NodeIconAffordance`.

**Entry point + drawer contract** (`space_room_dialog.dart:79-114`):
```dart
Future<void> showSpaceRoomDialog({ required BuildContext context,
    required GcsCommandTransport transport, required SpaceRoomDialogMode mode, ... }) {
  final _DialogForm form = _DialogForm(mode: mode, ..., transport: transport);
  return ResponsiveDrawer.show<void>(
    context: context,
    title: form.dialogTitle,
    children: <Widget>[_DialogFields(form: form)],
    footer: _DialogFooter(form: form),
    onClose: form.dispose,
  );
}
```
`ResponsiveDrawer.show` signature (`responsive_drawer.dart:8-14`): `{required BuildContext context, required String title, required List<Widget> children, Widget? footer, VoidCallback? onClose}`.

**View-local form state** (`space_room_dialog.dart:120-357`): a private `_DialogForm extends ChangeNotifier` holding `TextEditingController` + config, with a `_submitHandled` one-shot confirm guard (`space_room_dialog.dart:171`, `283-306`). `join_dialog.dart` copies the single-field `createRoomInSpace` mode: `TextEntryFieldWidget` + `TextFormFieldLogic` (`space_room_dialog.dart:380-390`) with autofocus + `onFieldSubmitted` + inline validation, `submit()` publishing via `transport.publishCommand(...)` and closing immediately on success, `showToast` on refused publish (`space_room_dialog.dart:283-306`).

**Toast (compact pill, no title)** (`toast_manager.dart:37-53`):
```dart
showToast(context, 'Invite link copied', type: ToastType.success);  // no title => pill
```

**Role badge atom** (`scaffold_badge.dart:24-51`, `176-185`): `ScaffoldBadge(variant: BadgeVariant.text, text: 'Member', badgeColor: palette.textSecondary)` — five-tier text badges per the UI-SPEC color contract (Super Admin = `lightGreenPrimary`; others `textSecondary`; pending = `statusWarningText`).

**Circular icon affordance + footer pill** (`gcs_shell.dart:264-297` `_SendButton`): 40px circular `ScaffoldSurface(shape: BoxShape.circle)` inside `ScaffoldPressable(semanticLabel: ...)`; confirm pill = `ScaffoldSurface(borderRadius: BorderRadius.circular(dimens.borderRadiusButton))` (see `space_room_dialog.dart:550-566`).

---

### `src/app/lib/shell/room_rail.dart` (affordance additions)

**Analog:** self `_NodeIconAffordance` (`room_rail.dart:438-471`) for the new "Members"/"Delete" trailing icons and the toolbar "Join" affordance; `_HeaderCreateButton` (`room_rail.dart:214-242`) for the toolbar affordance shape. Copy the `_NodeIconAffordance` idiom verbatim for `Icons.group_outlined` (members), `Icons.person_add_outlined` (join), `Icons.delete_outline` (delete, `statusError` tint), wiring each `onTap` to open the matching dialog.

---

### `src/app/lib/shell/gcs_shell.dart` (MembersCubit wiring)

**Analog:** self `_GCSChatState.initState` + `MultiBlocProvider` (`gcs_shell.dart:89-109`, `134-140`):
```dart
late final MembersCubit _membersCubit;
_membersCubit = widget.membersCubit ?? MembersCubit();
_sessionCubit = widget.sessionCubit ?? SessionCubit.openDefault(
  dbPath: widget.dbPath, railCubit: _railCubit,
  messageFlowCubit: _messageFlowCubit, membersCubit: _membersCubit);
```
Add `BlocProvider<MembersCubit>.value(value: _membersCubit)` to the `MultiBlocProvider` list, and follow the same own-vs-injected disposal discipline (`gcs_shell.dart:111-130`).

---

### `test/test_gcs_membership.cpp` (unit test)

**Analog:** `test/test_gcs_entities.cpp` — the injected-pubsub fixture + wait-condition template.

**Fixture + injected seam** (`test_gcs_entities.cpp:88-168`, `176-214`):
```cpp
class GcsMembershipTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() { /* soralog config — copy verbatim from test_gcs_entities.cpp:96-105 */ }
  void SetUp() override { /* per-test temp dir — copy from test_gcs_entities.cpp:107-116 */ }
  void TearDown() override { std::error_code ec; std::filesystem::remove_all(m_tempPath, ec); }
  std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> MakeStartedPubSub(const std::string &keyDir); // :131-149
  std::string m_tempPath;
};
// Per test: pubsub + MakeGraphsyncContext + CoreSession::Initialize(pubsub, graphsync.network)
```
**Wait-condition** (`test_wait_condition.hpp:38-56`; usage `test_gcs_entities.cpp:186-187`):
```cpp
EXPECT_TRUE(gcs::test::WaitForCondition([&dbPath]() { return std::filesystem::exists(dbPath); },
                                        gcs::test::kWaitTimeout));
```
Never `std::this_thread::sleep_for`. Register in `test/CMakeLists.txt` via the `gcs_test(<name> <srcs> "gcs_core;gcs_storage;neoswarm_common")` macro (`test/CMakeLists.txt:48-93`, see the `test_gcs_entities` line at `:113`).

---

### `test/test_gcs_crypto.cpp` (extension)

**Analog:** self — pure unit tests, no pubsub/DB, no sleeps (`test_gcs_crypto.cpp:16-22`, `53-70`). Add `WrapRoomKey`/`UnwrapRoomKey` round-trip, wrong-key rejection, tamper, and key-store thread-safety cases, asserting lengths via `kGcmNonceLength`/`kGcmTagLength`/`kRoomKeyLengthBytes` (no magic literals).

---

### `test/test_gcs_ffi.cpp` (extension)

**Analog:** self — `OWN_MAIN` fixture + `gcs_exit_main.hpp` (`test_gcs_ffi.cpp:26-35`, `72-131`). Extend with invite/redeem/role/leave/delete/approve arm tests: `gcs_init` with a temp db, serialize the new `GcsCommand` arms, assert `GCS_ERROR_INVALID_ARGUMENT` on the validation rejections and `GCS_OK` on valid mints, and that pushed `MemberList`/`InviteLink` reach a registered port.

---

### `test/test_gcs_membership_multinode.cpp` (integration, Wave-0 gap)

**Analog:** `test/test_gcs_messaging_multinode.cpp` — the two-node mesh fixture:
```cpp
struct MultinodeNode {                    // test_gcs_messaging_multinode.cpp:110-120
  std::string basePath; std::string dbPath;
  std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> pubsub;
  TestGraphsyncContext graphsync;
  std::unique_ptr<gcs::CoreSession> session;
  std::mutex sinkMutex; std::vector<chat::GcsEvent> events;
  std::unique_ptr<gcs::Messaging> messaging;
};
// StartTransport (:133-159) + StartSession (:168-176) + cross-node WaitForCondition converge (:414-437)
```
Copy the `StartTransport`/`StartSession` helpers; assert remote redeem converges the peer's pushed `MemberList` via `WaitForCondition` (P6).

---

## Shared Patterns

### Error domain & rejection
**Source:** `src/lib/gcs_entity_store.cpp:226-234`, `src/ffi/gcs_core_ffi.cpp:587-592`
**Apply to:** `gcs_membership`, all new FFI arms, `gcs_crypto` key store
- C++: `outcome::failure(sgns::gcs::Error::GcsDbError)` — the Error domain has no NotFound code; `GcsDbError` is the generic rejection. Skip-and-log hostile records with `spdlog::warn(...)`, never abort.
- FFI: reject with `PostErrorNotice("...rejected: ...")` + `return GCS_ERROR_INVALID_ARGUMENT;`.

### CR-01-safe id minting
**Source:** `src/lib/gcs_entity_store.cpp:110-119`, `src/lib/gcs_messaging.cpp:126-136`
**Apply to:** member ids, invite/mint ids
- `prefix + "<wallclock-ms>-<random-token>-<seq>"` — a bare counter revisits prior-session key space under LWW.

### Tombstone-skip reads (never key removal)
**Source:** `src/lib/gcs_entity_store.cpp:296-320`, `src/lib/gcs_messaging.cpp:321-348`
**Apply to:** `MembersFor` enumeration, member read gating for dormant `approved_by` records
- Keep tombstoned records in memory; readers `continue` past `deleted()` entries. D-07 dormant records are read-gated the same way (skip records with empty `approved_by` when enforcement applies), never deleted.

### Deferred-registration constructors
**Source:** `src/lib/gcs_entity_store.hpp:55`, `src/lib/gcs_messaging.hpp:99-100`
**Apply to:** `gcs::Membership`
- Constructor stores references/values only, no I/O, no fallible work. Mutators may read-union-write (manifest idiom) or write-after-validate.

### spdlog diagnostics
**Source:** global directive; `src/lib/gcs_messaging.cpp:115,147,252,267`
**Apply to:** every new/changed C++ file
- `spdlog::debug/info/warn/error` only; never `fprintf`/`cout`/`printf`.

### C++ owns state; Dart is a thin subscriber
**Source:** `src/app/lib/cubits/session_cubit.dart:426-467`, `rail_cubit.dart:148-183`
**Apply to:** `members_cubit.dart`, `members_dialog.dart`, `join_dialog.dart`
- Dart never mints ids/keys, never enforces permissions, never synthesizes rosters — it renders pushed `GcsEvent` data and publishes data-only `GcsCommand` envelopes. View-local dialog state stays in the `_DialogForm` ChangeNotifier, never in Cubits.

### Test discipline
**Source:** `test/test_wait_condition.hpp:38-56`, `test/test_gcs_entities.cpp:176-214`
**Apply to:** all new tests
- GTest via `gcs_test()` macro (`test/CMakeLists.txt:48-93`); wait-condition templates only (no `sleep_for`); injected pubsub/graphsync seam for non-node tests; `gcs_exit_main.hpp` + `OWN_MAIN` for SDK-booting FFI tests.

### C++17 / bracing / Doxygen / constants
**Source:** global CLAUDE.md directive; observed in every file above
**Apply to:** all new/changed C++ files
- C++17 ceiling; Allman/Ullman bracing with braces on every `if`/`while`/`for`/`switch`; Doxygen `@brief`/`@param`/`@return` on every function; `constexpr` `kCamelCase` named constants (no magic numbers except `0`/`1`/`-1` in trivial contexts); `unsigned int` for non-negative ordinals; no OS `#ifdef` guards in source (platform code lives in `os/{OSX,Linux,Windows,iOS,Android}/Platform.hpp`).

---

## No Analog Found

No file is without a precedent — every new artifact extends an in-repo component. The single genuinely new pattern is **shared mutable state inside `gcs_crypto`** (the per-room key store), which contradicts that file's documented statelessness (`gcs_crypto.hpp:7-14`). It has no direct in-crypto analog; borrow the mutex-guarded-map idiom from `gcs_messaging.hpp:241-243` and keep the exception documented in the header the way D-06 requires (store only 32-byte key bytes under `std::mutex`; never cache EVP contexts — P7).

The dedicated `InviteLink` event arm has no prior pushed-data arm that carries transient response data (not an error): model it on `ErrorNotice`'s dedicated single-field message (`gcs_chat.proto:89-92`, `gcs_core_ffi.cpp:587-592`), but route it to the invite view instead of the error surface.

## Metadata

**Analog search scope:** `src/lib/`, `src/ffi/`, `src/proto/`, `src/app/lib/`, `src/app/scaffold/lib/components/`, `test/`
**Files scanned:** 20 source/analog files read; 5 additional scaffold atoms surveyed by signature
**Pattern extraction date:** 2026-09-27
