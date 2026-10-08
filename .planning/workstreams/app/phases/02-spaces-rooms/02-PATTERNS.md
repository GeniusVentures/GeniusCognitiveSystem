# Phase 2: Spaces & Rooms - Pattern Map

**Mapped:** 2026-09-19
**Files analyzed:** 15 (5 new, 10 modified/extended)
**Analogs found:** 14 / 15 (1 partial — the dialog widget has no in-repo dialog ancestor)

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `src/proto/gcs_chat.proto` (extend) | config (wire contract) | request-response | itself — `GcsCommand`/`GcsEvent` oneofs, `ChatMessageState` | exact |
| `src/lib/gcs_entity_store.hpp` (new) | service/model | CRUD (KV records + manifest) + transform (derived join) | `src/lib/gcs_core.hpp` (CoreSession class shape) | role-match |
| `src/lib/gcs_entity_store.cpp` (new) | service | CRUD | `src/lib/gcs_core.cpp` + FFI per-record Put (`gcs_core_ffi.cpp:355`) | role-match |
| `src/ffi/gcs_core_ffi.cpp` (extend) | controller (FFI dispatch) | event-driven | itself — `kJoinTopic`/`kSendText` arms, `BuildRoomListEvent` | exact |
| `src/CMakeLists.txt` (extend, one line) | config (build) | n/a | itself — `gcs_core` target | exact |
| `src/app/lib/generated/proto/*.pb*.dart` (regen) | generated | n/a | existing committed `gcs_chat.pb.dart` + pinned regen command | exact (process) |
| `src/app/lib/cubits/rail_cubit.dart` (extend) | store (cubit) | event-driven (full replacement on push) | itself — `setRooms` | exact |
| `src/app/lib/cubits/session_cubit.dart` (extend) | controller (dispatcher) | event-driven | itself — `_dispatchEvent` | exact |
| `src/app/lib/shell/room_rail.dart` (extend) | component | event-driven render | itself — `_RoomRow`, section header, `ScaffoldStateView` empty state | exact |
| `src/app/lib/shell/space_room_dialog.dart` (new) | component | request-response (publish command → await pushed SpaceTree) | `room_rail.dart` composition + scaffold atoms; no in-repo dialog exists | partial |
| `test/test_gcs_entities.cpp` (new) | test | CRUD + restart persistence | `test/test_gcs_core_smoke.cpp` fixture + `test_gcs_ffi_sdk.cpp` two-cycle | exact (fixture) |
| `test/test_gcs_ffi_sdk.cpp` (extend) | test | event-driven (fake-port push capture) | itself — `RunSessionCycle`, `PushedEventLog` | exact |
| `test/CMakeLists.txt` (extend) | config (build) | n/a | itself — `gcs_test` macro | exact |
| `src/app/test/cubits/shell_cubits_test.dart` (extend) | test | event-driven | itself — `_RecordingTransport`, `handlePushedBytes` tests | exact |
| `src/app/test/shell/space_room_dialog_test.dart` (new) | test | request-response | `chat_shell_test.dart` harness + `shell_cubits_test.dart` `_RecordingTransport` | role-match |

Note: `src/app/test/chat_shell_test.dart` is also extended (rail tree render tests) — same classification and analog as `shell_cubits_test.dart` (itself, exact).

## Pattern Assignments

### `src/proto/gcs_chat.proto` (config/contract, request-response)

**Analog:** itself. The append-only discipline means every existing tag is frozen; new messages land below and oneof arms take the next free tag. Verified current tags: `GcsCommand` = `join_topic=1, send_text=2` (next 3-5); `GcsEvent` = `message=1, room_list=2, readiness=3, error=4` (next 5).

**Envelope + record precedent** (lines 48-54, 56-65, 82-90):
```protobuf
message GcsCommand {
  oneof payload {
    JoinTopicCommand join_topic = 1;
    SendTextCommand send_text = 2;
  }
}

// Authoritative message record. This proto type IS the C++ half (D-24 reinterpreted by D-26):
// C++ stamps id/timestamp/role/state; Dart renders.
message ChatMessageState {
  string id = 1;
  string room_topic = 2;
  ...
  int64 timestamp = 6;
}

message GcsEvent {
  oneof payload {
    ChatMessageState message = 1;
    RoomList room_list = 2;
    Readiness readiness = 3;
    ErrorNotice error = 4;
  }
}
```

New messages copy: the `ChatMessageState` comment style (authority-field annotation), the data-only command comment style (line 37: "D-27: data-only; C++ stamps authority"), and field numbering from 1 per message. Proposed shapes in `02-RESEARCH.md` Pattern 1 are grounded and sound — planner can adopt them verbatim. `RoomList` (lines 67-70) MUST NOT be touched.

---

### `src/lib/gcs_entity_store.hpp` (new) + `src/lib/gcs_entity_store.cpp` (new)

**Analog:** `src/lib/gcs_core.hpp` / `.cpp` for class conventions; the FFI's per-record Put for the persistence idiom.

**Class declaration conventions** (`src/lib/gcs_core.hpp:1-18` — file header Doxygen, `@file/@brief/@details/@copyright`; `:20-23` namespace + outcome alias; `:33-51` constructor-stores-config-only; `:58-61` deleted copy/move):
```cpp
namespace gcs {
// Error domain alias — gcs_core reuses the sgns::gcs error domain vendored in
// gcs_storage/common (bridged into GcsGlobalDb's namespace).
namespace outcome = libp2p::outcome;

class CoreSession {
public:
  struct Config {
    std::string m_dbPath; ///< RocksDB path for the GCS CRDT store
  };
  explicit CoreSession(Config config);
  CoreSession(const CoreSession &) = delete;
  CoreSession &operator=(const CoreSession &) = delete;
```

**Doxygen method headers** (`gcs_core.hpp:121-136` — every public method, `@brief/@param[in]/@return`):
```cpp
  /**
   * @brief Put a key/value pair into the CRDT store.
   *
   * @param[in] key   Hierarchical key path.
   * @param[in] value UTF-8 payload bytes.
   * @return outcome::success on success; propagated Error otherwise.
   */
  outcome::result<void> Put(const std::string &key, const std::string &value);
```

**Implementation file conventions** (`gcs_core.cpp:1-27` — .cpp Doxygen header, thin forward, all braces Allman; note the empty-dbPath guard comment style):
```cpp
/**
 * @file       gcs_core.cpp
 * @brief      Implementation of gcs::CoreSession — the C++-owned session object
 * exposed to the FFI layer (D-01/D-04).
 ...
 */
#include "gcs_core.hpp"

namespace gcs {

CoreSession::CoreSession(Config config)
    : m_config(std::move(config)) {
```

**Per-record CRDT Put idiom to generalize** (`src/ffi/gcs_core_ffi.cpp:350-356` — key = scope + "/" + authority id; value = serialized payload proto, not the push envelope):
```cpp
            // Phase 1 echo: store the authoritative record, then push it to the port
            // (the real pub/sub flow lands in Phase 3). Key by the C++-stamped
            // authority id (D-04) so per-room history survives — a room-topic-only
            // key made each new message overwrite the previous one; store the
            // payload message, not the push envelope.
            if ( !g_session->Put( sendText.room_topic() + "/" + message->id(),
                                  message->SerializeAsString() ).has_value() )
```
Entity keys follow directly: `gcs/entities/spaces/<id>` / `gcs/entities/rooms/<id>` with the record proto as value bytes, and `gcs/index/manifest` with the manifest proto.

**Manifest Get failure = empty catalog** (no excerpt needed — `GcsGlobalDb::Get` collapses missing-key into `Error::GcsDbError`, per 02-RESEARCH Pitfall 2; `LoadFromStore` treats any Get failure as absent + `spdlog::debug`).

**CMake wiring** (`src/CMakeLists.txt:4-6` — the one-line addition):
```cmake
add_library(gcs_core
        lib/gcs_core.cpp
)
```
becomes `add_library(gcs_core lib/gcs_core.cpp lib/gcs_entity_store.cpp)` — nothing else in the target changes (include dirs and link libs already cover `gcs_storage`, spdlog, fmt).

---

### `src/ffi/gcs_core_ffi.cpp` (extend: 3 command arms + SpaceTree push)

**Analog:** itself. Four patterns to copy exactly.

**1. Command-arm validation + dispatch + error push** (lines 290-322, the `kJoinTopic` arm — empty-check → `PostErrorNotice` + `GCS_ERROR_INVALID_ARGUMENT` → mutate under `g_mutex` → push updated event):
```cpp
        case gcs::chat::GcsCommand::kJoinTopic:
        {
            const std::string roomTopic = command.join_topic().room_topic();
            if ( roomTopic.empty() )
            {
                PostErrorNotice( "join_topic rejected: room_topic is empty" ); // D-29: raw error string on the push port
                return GCS_ERROR_INVALID_ARGUMENT;
            }
            // Listen first, then broadcast — the same ordering
            // GcsGlobalDb::Initialize uses (D-07). ...
            if ( !g_session->AddListenTopic( roomTopic ).has_value()
                 || !g_session->AddBroadcastTopic( roomTopic ).has_value() )
            {
                spdlog::error( "gcs_ffi: join_topic('{}') failed — topic not "
                               "added to the room list",
                               roomTopic );
                PostErrorNotice( "join_topic failed for room '" + roomTopic + "'" );
                return GCS_ERROR_GENERIC;
            }
            // Idempotent room list: a repeated join of an already-joined topic
            // (Dart-side retry, double-tap) must not append a duplicate room.
            if ( std::find( g_roomTopics.begin(), g_roomTopics.end(), roomTopic ) == g_roomTopics.end() )
            {
                g_roomTopics.push_back( roomTopic );
            }
            PostToDart( BuildRoomListEvent() );
            return GCS_OK;
        }
```
New arms (`kCreateSpace`, `kCreateRoom`, `kUpdateSpace`) copy: empty-name check first, parent-existence check (`IsValidParentSpace`) for create_room, store failure → `PostErrorNotice` + `GCS_ERROR_GENERIC`, success → push `BuildSpaceTreeEvent()` (+ `BuildRoomListEvent()` when the derived set changed). Research's Code Examples section has the concrete `kCreateRoom` shape — use it.

**2. Event builder** (lines 112-121 — `BuildSpaceTreeEvent()` mirrors this exactly):
```cpp
    gcs::chat::GcsEvent BuildRoomListEvent()
    {
        gcs::chat::GcsEvent event;
        gcs::chat::RoomList* roomList = event.mutable_room_list();
        for ( const std::string& topic : g_roomTopics )
        {
            roomList->add_room_topic( topic );
        }
        return event;
    }
```

**3. Id minting** (lines 149-158 — `NextEntityId(prefix)` generalizes: distinct `kSpaceIdPrefix`/`kRoomIdPrefix` constants beside `kMessageIdPrefix` at line 45, and a `g_entitySeq` atomic beside `g_messageSeq` at line 51):
```cpp
    std::string NextMessageId()
    {
        static const std::string seed = [] {
            const int64_t nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch() ).count();
            std::random_device randomDevice;
            return std::to_string( nowMs ) + "-" + std::to_string( randomDevice() );
        }();
        return kMessageIdPrefix + seed + "-" + std::to_string( g_messageSeq.fetch_add( 1 ) );
    }
```

**4. Subscribe-time push ordering** (lines 390-400 — insert `SpaceTree` BEFORE the existing `RoomList` push, per D-02 tree-before-membership-before-readiness):
```cpp
        g_dartPort = dartPort;

        // D-05/D-26 push-not-pull: RoomList first, then Readiness(ready=true). ...
        PostToDart( BuildRoomListEvent() );

        gcs::chat::GcsEvent readyEvent;
        readyEvent.mutable_readiness()->set_ready( true );
        PostToDart( readyEvent );
        return GCS_OK;
```

**Global + shutdown wiring** (lines 47-49 for the `std::unique_ptr<gcs::EntityStore> g_entities` beside `g_session`; lines 403-414 for `gcs_shutdown` resetting it alongside `g_roomTopics.clear()`; `gcs_init` constructs + `LoadFromStore()` after the smoke-topic block at 219-247 — no pushes at init, the port is never registered yet).

---

### `src/app/lib/cubits/rail_cubit.dart` (extend: RailState flat → tree)

**Analog:** itself — `setRooms` (lines 48-56) is the full-replacement pattern `setTree` copies verbatim, including the dangling-selection clear:
```dart
  void setRooms(List<String> rooms) {
    final List<String> next = List<String>.unmodifiable(rooms);
    final String? active = state.activeRoom;
    if (active == null || next.contains(active)) {
      emit(state.copyWith(rooms: next));
      return;
    }
    emit(state.copyWith(rooms: next, clearActiveRoom: true));
  }
```
Conventions to keep: doc-comment on every public member (`///`), `copyWith` with nullable-clear flag idiom (lines 25-34), `const` default `const <String>[]` (line 14), library doc header (lines 1-7). `setRooms` and `rooms` stay untouched (joined view per D-02); `setTree` adds `spaces` + `standaloneRooms` fields. `selectRoom` joined-only guard (lines 60-65) is the Pitfall-7 decision point — research recommends disabled rows, not a selectRoom change.

### `src/app/lib/cubits/session_cubit.dart` (extend: SpaceTree dispatch arm)

**Analog:** itself — `_dispatchEvent` (lines 353-372). The new arm slots in FIRST (tree renders before membership):
```dart
  void _dispatchEvent(GcsEvent event) {
    if (event.hasRoomList()) {
      _railCubit?.setRooms(event.roomList.roomTopic);
      return;
    }
    ...
```
becomes `if (event.hasSpaceTree()) { _railCubit?.setTree(...); return; }` above the existing arms. The dialog needs no transport change — `GcsCommandTransport.publishCommand` (lines 63-66, 304-325) already carries any oneof arm.

### `src/app/lib/shell/room_rail.dart` (extend: two sections, badge, "+" affordances)

**Analog:** itself. Three excerpts to reuse:

**Section header** (lines 67-84 — the "Spaces" section copies this verbatim; note `kHeaderLetterSpacing` constant, UI-SPEC typography):
```dart
                child: Text(
                  'Rooms',
                  style: Theme.of(context).textTheme.titleSmall?.copyWith(
                    color: palette.textSecondary,
                    letterSpacing: kHeaderLetterSpacing,
                  ),
                ),
```

**Row composition** (lines 124-163 — `_RoomRow`: `ScaffoldPressable` → `DecoratedBox` (selected tint `ScaffoldColors.btnFilterSelected`, `radiusMd`) → `Padding` → `ConstrainedBox(minHeight: minTouchTarget - space2*2)` → Row(icon + expanded ellipsized text)). Space rows add `ScaffoldBadge` + trailing affordances inside the same Row; per-research Pitfall 7, unjoined standalone rooms render via `ScaffoldPressable(disabled: true)`.

**Empty state** (lines 56-64):
```dart
          if (rail.rooms.isEmpty) {
            return const ScaffoldStateView(
              state: 'empty',
              emptyHeadline: 'No rooms yet',
              ...
```

**Doc conventions** (lines 1-30): library doc header, named `k`-prefixed static constants for every number (`kWidth = 280.0`), helper function with doc comment (`roomDisplayName`).

### `src/app/lib/shell/space_room_dialog.dart` (new)

**Analog:** partial — no dialog widget exists in `src/app/lib/shell/` (verified: only `gcs_shell.dart`, `room_rail.dart`). Composition copies `room_rail.dart` conventions (library doc, `ScaffoldPressable` for the confirm button, palette/dimens via `context.palette`/`context.dimens`); every UI primitive comes from verified scaffold atom APIs:

**`ResponsiveDrawer.show`** (`src/app/scaffold/lib/components/bottom_drawer/responsive_drawer.dart:8-14` — exact signature):
```dart
  static Future<T?> show<T>({
    required BuildContext context,
    required String title,
    required List<Widget> children,
    Widget? footer,
    VoidCallback? onClose,
  })
```

**`TextEntryFieldWidget`** (`.../text_entry_field_widget.dart:5-11` — takes a logic object that owns the controller):
```dart
class TextEntryFieldWidget extends StatelessWidget {
  final TextFormFieldLogic logic;
  const TextEntryFieldWidget({super.key, required this.logic});
```

**`ScaffoldSelectionIndicatorRadio`** (`.../scaffold_selection_indicator_radio.dart:21-36` — bool-based, NOT group-value; a public/private pair is two radios with coordinated booleans):
```dart
  const ScaffoldSelectionIndicatorRadio({
    super.key,
    required this.value,        // bool
    this.onChanged,             // ValueChanged<bool>?
    this.disabled = false,
  });
```

**`ScaffoldSelectionIndicatorToggle`** (`.../scaffold_selection_indicator_toggle.dart:23-31` — same bool shape, for autoJoinRooms).

**`showToast`** (`.../toast/toast_manager.dart:37-44`):
```dart
void showToast(
  BuildContext context,
  String message, {
  String? title,
  ToastType type = ToastType.success,
  Duration? duration,
  VoidCallback? onClose,
})
```

**`ScaffoldBadge`** (`.../scaffold_badge.dart:25-36` — `variant: BadgeVariant.text/icon/dot/count`, `badgeColor` override, composes into a Row/Stack, never positions itself).

**Publish-on-confirm pattern** — the dumb-form behavioral analog is `ComposerCubit.send` driving `_RecordingTransport` in tests: build the `GcsCommand()` oneof arm, call `transport.publishCommand(command)`, treat `false` as toast-worthy, close on `true`; no local optimism (rail updates only when `SpaceTree` arrives). Scaffold submodule contract: `lib/` is read-only, never edit generated families (`scaffold_selection_indicator_*` are generated).

---

### `test/test_gcs_entities.cpp` (new) + `test/test_gcs_ffi_sdk.cpp` (extend)

**Analog:** `test/test_gcs_core_smoke.cpp` for the fixture (EntityStore unit tests), `test_gcs_ffi_sdk.cpp` for two-cycle restart + push capture.

**Fixture skeleton to copy** (`test/test_gcs_core_smoke.cpp:78-104` — soralog SetUpTestSuite, per-test salted temp dir):
```cpp
        static void SetUpTestSuite()
        {
            auto loggerConfigurator = std::make_shared<libp2p::log::Configurator>();
            auto configFromYaml     = std::make_shared<soralog::ConfiguratorFromYAML>( loggerConfigurator,
                                                                                       std::string{ kLoggingYaml } );
            auto loggingSystem      = std::make_shared<soralog::LoggingSystem>( configFromYaml );
            const auto confResult   = loggingSystem->configure();
            ASSERT_FALSE( confResult.has_error ) << "Could not configure test logging system";
            libp2p::log::setLoggingSystem( loggingSystem );
        }

        void SetUp() override
        {
            const auto *info       = ::testing::UnitTest::GetInstance()->current_test_info();
            const auto  uniqueSalt = std::chrono::steady_clock::now().time_since_epoch().count();
            m_tempPath = ( std::filesystem::temp_directory_path()
                           / ( std::string{ "gcs_core_smoke_" } + info->name + "_" +
                               std::to_string( uniqueSalt ) ) )
                             .string();
            std::filesystem::create_directories( m_tempPath );
        }
```
(`kLoggingYaml` at lines 45-57 and `MakeStartedPubSub` at 113-131 copy too; `gcs::test::MakeGraphsyncContext` from `test_graphsync_network.hpp`.)

**Injected-pubsub session bring-up** (lines 142-153 — the EntityStore test body shape; research's restart skeleton composes two sequential blocks of this on one db_path):
```cpp
        auto pubsub = MakeStartedPubSub( m_tempPath + "/key" );
        ASSERT_NE( pubsub, nullptr );
        auto graphsync = gcs::test::MakeGraphsyncContext( pubsub );

        gcs::CoreSession::Config cfg{};
        cfg.m_dbPath = m_tempPath + "/db";
        gcs::CoreSession session( cfg );

        auto res = session.Initialize( pubsub, graphsync.network );
        ASSERT_TRUE( res.has_value() );
```

**Two-cycle restart pattern** (`test_gcs_ffi_sdk.cpp:260-297` `RunSessionCycle` — init → subscribe fake port → publish → collect pushed events → `gcs_shutdown(handle)`; and 362-393 the CR-01 salt assertion `kMinIdSaltSeparators` — mirror it for entity ids). `PushedEventLog`/`FakePostCObject`/`InstallFakeApiDlTable` (lines 113-184) are the push-capture seam for FFI-level create/update tests.

**Wait-condition template** (`test/test_wait_condition.hpp:38-56` — mandatory, never `sleep_for`):
```cpp
    inline bool WaitForCondition( const std::function<bool()> &predicate, std::chrono::milliseconds timeout )
```

**Registration** (`test/CMakeLists.txt:45-56` the `gcs_test` macro; `:72` the smoke precedent):
```cmake
gcs_test(test_gcs_core_smoke test_gcs_core_smoke.cpp "gcs_core;gcs_storage;neoswarm_common")
```
New: `gcs_test(test_gcs_entities test_gcs_entities.cpp "gcs_core;gcs_storage;neoswarm_common")` — same libs as the smoke test (EntityStore lives in gcs_core). Note `GTest::Main` ordering comment (lines 47-49).

### `src/app/test/shell/space_room_dialog_test.dart` (new) + cubit/shell test extensions

**Analog:** `chat_shell_test.dart` harness + `shell_cubits_test.dart` doubles.

**Recording transport** (`shell_cubits_test.dart:111-119` and `chat_shell_test.dart:44-52` — identical double in both files; the dialog test gets its own copy):
```dart
class _RecordingTransport implements GcsCommandTransport {
  final List<GcsCommand> commands = <GcsCommand>[];

  @override
  bool publishCommand(GcsCommand command) {
    commands.add(command);
    return true;
  }
}
```

**Pushed-bytes dispatch test pattern** (`shell_cubits_test.dart:340-346` — new SpaceTree arm test follows this shape):
```dart
      cubit.handlePushedBytes(
        (GcsEvent()
              ..roomList = (RoomList()
                ..roomTopic.addAll(<String>['gcs/chat/a', 'gcs/chat/b'])))
            .writeToBuffer(),
      );
      expect(rail.state.rooms, <String>['gcs/chat/a', 'gcs/chat/b']);
```

**Widget-test harness** (`chat_shell_test.dart:58-66, 68-105` — `_pumpUntil` bounded-frame wait (never sleeps), cubits constructed in the test body, `addTearDown` per cubit, `MaterialApp(theme: GcsTheme.light, ...)` wrapper, `find.byType`/`find.text` assertions).

### `src/app/lib/generated/proto/` (regen)

**Analog:** the committed `gcs_chat.pb.dart`/`.pbenum.dart`/`.pbjson.dart`. Regen is a manual pinned step (from `01-05-PLAN.md`, quoted in research Pitfall 1) — protoc_plugin MUST be 22.5.0:
```bash
cd src/app && /Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/thirdparty/build/OSX/Debug/protobuf/bin/protoc \
  -I ../proto --dart_out=lib/generated/proto \
  --plugin=protoc-gen-dart=$HOME/.pub-cache/bin/protoc-gen-dart \
  ../proto/gcs_chat.proto
```
C++ halves regenerate automatically via `add_proto_library` (`src/proto/CMakeLists.txt:8-12`). Dart regen task must precede all Dart work in wave order.

## Shared Patterns

### FFI command-arm dispatch (validate → mutate under g_mutex → push or PostErrorNotice)
**Source:** `src/ffi/gcs_core_ffi.cpp:288-374`
**Apply to:** all three new command arms. Every arm: switch case inside the existing `gcs_publish` lock, arg validation first, `PostErrorNotice` + `GCS_ERROR_INVALID_ARGUMENT` for bad input, `GCS_ERROR_GENERIC` for store failure, `PostToDart` of rebuilt events on success. Never throw across the ABI.

### Event build + push (full envelope, codec-tagged bytes)
**Source:** `BuildRoomListEvent` (`gcs_core_ffi.cpp:112-121`), `PostToDart` (`:85-103`), `SerializeGcsEvent` (`:62-73`)
**Apply to:** `BuildSpaceTreeEvent` and every push site. Serialize the payload proto (not the envelope) to KV; serialize the envelope to push.

### Seed+counter opaque ids (CR-01-safe)
**Source:** `NextMessageId` (`gcs_core_ffi.cpp:149-158`), salt assertions (`test_gcs_ffi_sdk.cpp:103, 385-393`)
**Apply to:** `NextEntityId` with `kSpaceIdPrefix`/`kRoomIdPrefix`; entity tests assert the salt-separator shape (`kMinIdSaltSeparators`).

### C++ class conventions (Doxygen, outcome, deferred registration, deleted copy/move)
**Source:** `src/lib/gcs_core.hpp` (whole file), `src/lib/gcs_core.cpp`
**Apply to:** `gcs_entity_store.{hpp,cpp}`. Allman braces, initialized members, `m_` member prefix, no magic numbers (`constexpr` kCamelCase), C++17 only, no OS ifdefs, spdlog-only diagnostics.

### Dart thin-subscriber conventions
**Source:** `rail_cubit.dart` (full-replacement + copyWith-clear idiom), `session_cubit.dart:353-372` (dispatch arms), `room_rail.dart` (k-constants, palette/dimens, ScaffoldStateView)
**Apply to:** every Dart change. Cubits never seed state locally; widgets read cubit state only.

### Test discipline (wait-condition, fixtures, doubles)
**Source:** `test/test_wait_condition.hpp`, `test_gcs_core_smoke.cpp` fixture, `test_gcs_ffi_sdk.cpp` two-cycle + fake API_DL port, `chat_shell_test.dart` `_pumpUntil`
**Apply to:** all four test surfaces. No `sleep_for`/sleeps anywhere; per-test salted temp dirs; cubits constructed in test bodies with `addTearDown`.

## No Analog Found

| File | Role | Data Flow | Reason |
|------|------|-----------|--------|
| `src/app/lib/shell/space_room_dialog.dart` (the dialog composition itself) | component | request-response | No dialog/modal widget exists anywhere in `src/app/lib/` (shell contains only `gcs_shell.dart`, `room_rail.dart`). All primitives are scaffold atoms (APIs verified above); behavioral analog is the dumb-form-publish via `GcsCommandTransport`. Planner should follow 02-RESEARCH Pattern 7 for composition and `room_rail.dart` for file conventions. |

Everything else has an in-repo analog — Phase 2 is composition, not invention.

## Metadata

**Analog search scope:** `src/proto`, `src/lib`, `src/lib/gcs_storage`, `src/ffi`, `src/app/lib/{cubits,shell,generated}`, `src/app/scaffold/lib/components`, `src/app/test`, `test/`, `src/CMakeLists.txt`, `test/CMakeLists.txt`, `src/proto/CMakeLists.txt`
**Files scanned:** 20 read in full or targeted part
**Pattern extraction date:** 2026-09-19
