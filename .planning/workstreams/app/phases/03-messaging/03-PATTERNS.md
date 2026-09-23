# Phase 3: Messaging - Pattern Map

**Mapped:** 2026-09-23 (updated same day — D-08 crypto seam added to the map; all other analogs unchanged)
**Files analyzed:** 16 (10 modify, 4 new, 2 conditional/regenerate)
**Analogs found:** 15 / 16

## File Classification

| New/Modified File | Role | Data Flow | Closest Analog | Match Quality |
|-------------------|------|-----------|----------------|---------------|
| `src/lib/gcs_storage/gcs_global_db.hpp` | service (storage wrapper) | CRUD | self — existing `Put`/`Get`/`Add*Topic` decls | exact (modify) |
| `src/lib/gcs_storage/gcs_global_db.cpp` | service (storage wrapper) | CRUD | self — `Put` impl (lines 256-271) | exact (modify) |
| `src/lib/gcs_core.hpp` | service (session facade) | CRUD pass-through | self — existing `Put`/`Get` decls | exact (modify) |
| `src/lib/gcs_core.cpp` | service (session facade) | CRUD pass-through | self — thin delegate impl | exact (modify) |
| `src/ffi/gcs_core_ffi.cpp` | controller (FFI command dispatcher) | event-driven + request-response | self — `send_text` arm (667-720), `join_topic` arm (621-666) | exact (modify) |
| `src/proto/gcs_chat.proto` | config (wire schema) | data-model | self — append-only Phase 2 section | exact (modify) |
| `src/lib/gcs_messaging.{hpp,cpp}` | service (messaging component) | event-driven + CRUD | `src/lib/gcs_entity_store.{hpp,cpp}` | role-match (new) |
| `src/lib/gcs_crypto.{hpp,cpp}` (D-08) | service (crypto adapter) | transform (encrypt/derive) | self — stateless free functions; the ONLY file including `<openssl/*>` | role-match (new) |
| `src/CMakeLists.txt` (D-08) | config (build) | — | self — existing `target_link_libraries` blocks (13-16, 25-29) | exact (modify — one line: `OpenSSL::Crypto`) |
| `src/app/lib/cubits/message_flow_cubit.dart` | store (cubit state holder) | event-driven | self — `append` + `buildChatFlowItemTextBubble` | exact (modify) |
| `src/app/lib/cubits/session_cubit.dart` | store (cubit dispatcher) | event-driven | self — `_dispatchEvent` | exact (modify) |
| `test/test_gcs_messaging.cpp` | test | event-driven | `test/test_gcs_entities.cpp` | exact (new) |
| `test/test_gcs_global_db.cpp` (target `test_gcs_storage`) | test | CRUD | self — lifecycle fixture | exact (modify) |
| `src/app/test/cubits/shell_cubits_test.dart` | test | event-driven | self — `_RecordingBindings` + cubit groups | exact (modify) |
| `src/app/templates/components/chat_message_bubble.dart.jinja2` + `chat_message_bubble_vars.json` | template/config | transform | self — existing bubble template + vars | exact (conditional) |
| `src/app/lib/generated/chat/chat_message_bubble.dart` | generated (never hand-edit) | transform | self — regenerate only | exact (conditional) |

---

## Pattern Assignments

### `src/lib/gcs_storage/gcs_global_db.{hpp,cpp}` (service, CRUD — D-02 topics-aware Put + D-01 prefix scan)

**Analog:** self. The D-02 fix site is `Put`; the new `QueryKeyValues`-backed scan follows the same wrapper shape as `Get`.

**Header declaration pattern** (`gcs_global_db.hpp` lines 180-216) — the exact shape to copy for the new overloads:
```cpp
outcome::result<void> AddBroadcastTopic(const std::string &topicName);
outcome::result<void> AddListenTopic(const std::string &topicName);
outcome::result<void> Put(const std::string &key, const std::string &value);
outcome::result<std::string> Get(const std::string &key);
```

**Core `Put` pattern — D-02 fix site** (`gcs_global_db.cpp` lines 256-271). The 2-arg form hardcodes `kNoTopics`; the new topics-aware overload copies this body but takes a `topics` parameter, and the existing signature delegates to it with an empty set:
```cpp
outcome::result<void> GcsGlobalDb::Put(const std::string &key,
                                       const std::string &value) {
  if (!m_running.load()) {
    return outcome::failure(Error::GcsDbError);
  }
  crdt::HierarchicalKey keyTyped{key};
  crdt::GlobalDB::Buffer valueTyped;
  valueTyped.put(value);
  // No publish topics — plain local store, not a broadcast write.
  const std::unordered_set<std::string> kNoTopics{};
  auto result = m_db->Put(keyTyped, valueTyped, kNoTopics);
  if (result.has_error()) {
    return outcome::failure(Error::GcsDbError);
  }
  return outcome::success();
}
```

**Get pattern** (`gcs_global_db.cpp` lines 273-283) — the shape the new `QueryKeyValues` prefix scan mirrors (guard → call `m_db` → error-map → value transform):
```cpp
outcome::result<std::string> GcsGlobalDb::Get(const std::string &key) {
  if (!m_running.load()) {
    return outcome::failure(Error::GcsDbError);
  }
  auto result = m_db->Get(crdt::HierarchicalKey{key});
  if (result.has_error()) {
    return outcome::failure(Error::GcsDbError);
  }
  const auto &buf = result.value();
  return std::string{buf.toString()};
}
```

**Error mapping** (`gcs_global_db.cpp` lines 47-59) — all SuperGenius `GlobalDB::Error` values collapse to `Error::GcsDbError`; reuse for the scan:
```cpp
Error MapGlobalDbError(crdt::GlobalDB::Error globalDbErr) noexcept {
  switch (globalDbErr) {
  case crdt::GlobalDB::Error::ROCKSDB_IO:
  // ... every arm falls through to:
  case crdt::GlobalDB::Error::GLOBALDB_NOT_STARTED:
    return Error::GcsDbError;
  }
  return Error::GcsDbError;
}
```

**STYLE NOTE:** `gcs_global_db.cpp` carries a style exemption (K&R braces, 2-space indent — file header lines 6-9). Do NOT reformat it piecemeal; the new overload must match the file's existing attached-brace style. New `.hpp` decls follow the repo Allman standard.

---

### `src/lib/gcs_core.{hpp,cpp}` (service, CRUD pass-through — widened Put/QueryMessages)

**Analog:** self. Thin delegation only; GcsGlobalDb owns state, logging, and error mapping.

**Header declaration pattern** (`gcs_core.hpp` lines 104-136) — add `Put(key, value, topics)` and a `QueryMessages(prefix)` decl right beside the existing pass-throughs:
```cpp
outcome::result<void> AddBroadcastTopic(const std::string &topicName);
outcome::result<void> AddListenTopic(const std::string &topicName);
outcome::result<void> Put(const std::string &key, const std::string &value);
outcome::result<std::string> Get(const std::string &key);
```

**Core pass-through pattern** (`gcs_core.cpp` lines 52-69) — each method is a one-line `m_db->...` forward; the new overloads copy this exactly:
```cpp
outcome::result<void>
CoreSession::AddBroadcastTopic(const std::string &topicName) {
  return m_db->AddBroadcastTopic(topicName);
}
outcome::result<void> CoreSession::Put(const std::string &key,
                                       const std::string &value) {
  return m_db->Put(key, value);
}
outcome::result<std::string> CoreSession::Get(const std::string &key) {
  return m_db->Get(key);
}
```

---

### `src/lib/gcs_messaging.{hpp,cpp}` (service, event-driven + CRUD — NEW, the testable messaging seam)

**Analog:** `src/lib/gcs_entity_store.{hpp,cpp}` — the only existing CoreSession-composed component. Copy its structure, not the manifest pattern (the manifest is the anti-pattern D-01 rejects; the prefix scan replaces it).

**Class shape / composition-over-inheritance** (`gcs_entity_store.hpp` lines 46-56, 168-173):
```cpp
class EntityStore {
public:
  explicit EntityStore(CoreSession &session);   // store the reference only, no I/O
  EntityStore(const EntityStore &) = delete;
  EntityStore &operator=(const EntityStore &) = delete;
  // ...
private:
  CoreSession &m_session; ///< Borrowed session (Put/Get pass-throughs)
  std::map<std::string, SpaceRecord> m_spaces;
  std::map<std::string, RoomRecord> m_rooms;
};
```
Constructor pattern (`gcs_entity_store.cpp` lines 105-108): store `m_session(session)`, nothing else — deferred registration, no fallible work.

**Key-prefix constants** (`gcs_entity_store.cpp` lines 27-38) — the model for the `gcs/messages/<room_topic>/<id>` prefix:
```cpp
constexpr const char *kSpacesKeyPrefix = "gcs/entities/spaces/";
constexpr const char *kRoomsKeyPrefix  = "gcs/entities/rooms/";
constexpr const char *kRoomTopicPrefix = "gcs/chat/";
```

**Skip-and-log scan loop — per-entry fault tolerance** (`gcs_entity_store.cpp` lines 142-184). The D-01 history scan copies this loop shape: one bad record never hides the rest; tombstone check applied per record:
```cpp
for (const std::string &spaceId : manifest.space_id()) {
  auto recordBytes = m_session.Get(std::string(kSpacesKeyPrefix) + spaceId);
  if (!recordBytes.has_value()) {
    spdlog::warn("gcs_entity_store: skipping space '{}' — record read failed", spaceId);
    continue;
  }
  gcs::chat::SpaceRecord record;
  if (!record.ParseFromString(recordBytes.value())) {
    spdlog::warn("gcs_entity_store: skipping space '{}' — record bytes unparseable", spaceId);
    continue;
  }
  m_spaces.emplace(record.id(), record);
}
```

**Id minting** (`gcs_entity_store.cpp` lines 110-119) — same seed + counter idiom the send path already uses (`gcs_core_ffi.cpp` lines 449-458 `NextMessageId`). Phase 3 MOVES id minting into `gcs::Messaging::NextMessageId` (03-04); the FFI's free function is deleted in 03-05 and `send_text` delegates to `Messaging::SendMessage`. C++ still owns id authority (D-04) — the component mints, the FFI does not.

---

**D-08 injected crypto seam (added 2026-09-23):** `Messaging` gains two OPTIONAL callables at construction (EncryptFn/DecryptFn over `roomTopic` + payload bytes; empty/absent = plaintext path). Copy the constructor-injection shape above; the seam members are `std::function` values, not a base class (composition over inheritance). `Messaging` never includes `<openssl/*>` — the OpenSSL-backed implementation lives in `src/lib/gcs_crypto.{hpp,cpp}` (`gcs::crypto`: `DeriveRoomKey` / `EncryptRecord` / `DecryptRecord`, stateless, fresh `EVP_CIPHER_CTX`/`EVP_PKEY_CTX` per call) and is injected at composition time in the FFI/session setup. Envelope layout pinned in 03-RESEARCH (Pattern 6): `nonce(12) || ciphertext || tag(16)`; decrypt failure = skip-and-log (same loop posture as the unparseable-record skip below). Verified crypto surface + threading constraint: 03-RESEARCH "D-08 Crypto Surface".

---

### `src/ffi/gcs_core_ffi.cpp` (controller, event-driven + request-response — send rewrite, join replay, receive bridge)

**Analog:** self. The `send_text` arm (667-720) and `join_topic` arm (621-666) are the exact sites being extended; the receive bridge reuses `PostToDart` and the `g_mutex` discipline.

**Globals / mutex discipline** (lines 84-100) — the receive callback and join replay run under `g_mutex`:
```cpp
std::mutex g_mutex;                            // guards g_session + g_entities + topic sets
std::unique_ptr<gcs::CoreSession> g_session;   // Phase 1: single global session
std::unique_ptr<gcs::EntityStore> g_entities;
std::vector<std::string> g_roomTopics;
std::atomic<int64_t> g_dartPort{ 0 };
std::atomic<uint64_t> g_messageSeq{ 0 };
```

**Caps / constants** (lines 52, 65, 71) — reuse `kMaxTopicLength` (128) and `kMaxMessageTextLength` (4096) in the rewritten send arm:
```cpp
constexpr const char* kMessageIdPrefix = "msg-";
constexpr size_t kMaxTopicLength = 128;
constexpr size_t kMaxMessageTextLength = 4096;
```

**Authority stamping — send path** (`send_text` arm lines 691-702) — copy into the pending/complete echo construction; Phase 3 adds `sender` = `GeniusNode::GetAddress()` and splits the single push into pending→publish→Put→complete:
```cpp
gcs::chat::GcsEvent event;
gcs::chat::ChatMessageState* message = event.mutable_message();
message->set_id(NextMessageId());
message->set_room_topic(sendText.room_topic());
message->set_role(gcs::chat::MESSAGE_ROLE_USER_SELF);
message->set_state(gcs::chat::MESSAGE_STATE_COMPLETE);
message->set_text(sendText.text());
message->set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
    std::chrono::system_clock::now().time_since_epoch()).count());
```

**Validation → PostErrorNotice → error return** (send arm lines 670-690, join arm 624-648) — the unchanged rejection idiom for every new guard:
```cpp
if (sendText.room_topic().empty()) {
  PostErrorNotice("send_text rejected: room_topic is empty");
  return GCS_ERROR_INVALID_ARGUMENT;
}
```

**Key layout precedent** (line 709) — the exact string being generalized to `gcs/messages/<room_topic>/<id>`:
```cpp
if (!g_session->Put(sendText.room_topic() + "/" + message->id(),
                    message->SerializeAsString()).has_value()) {
  spdlog::error("gcs_ffi: send_text store write failed for room '{}'", sendText.room_topic());
  PostErrorNotice("send_text store write failed for room '" + sendText.room_topic() + "'");
  return GCS_ERROR_GENERIC;
}
```

**Push plane** (`PostToDart` lines 294-312, `PostErrorNotice` 428-433) — unchanged; the `MessageHistory` batch and the pending/complete echoes all flow through `PostToDart`. `SerializeGcsEvent` (271-282) is the envelope serializer.

**Join-triggered replay hook** — `join_topic` arm lines 621-666 (listen-first-then-broadcast ordering, lines 634-649) plus `RefreshDerivedJoins` lines 374-421 (the derived-join path that must ALSO trigger the D-06 history scan). Both call the same new `PushMessageHistory(roomTopic)` helper under `g_mutex`.

---

### `src/proto/gcs_chat.proto` (config, data-model — append-only additions D-04/D-05/D-06)

**Analog:** self. Phase 2 additions (lines 102-169) show the append-only discipline and comment-marker convention.

**`ChatMessageState` to extend** (lines 67-74; next free tags are 7, 8, 9):
```proto
message ChatMessageState {
  string id = 1;
  string room_topic = 2;
  MessageRole role = 3;
  MessageState state = 4;
  string text = 5;
  int64 timestamp = 6;
  // + string sender = 7;        (D-04 — wallet address, C++-stamped)
  // + bool   deleted = 8;       (D-05 — tombstone fields present from creation)
  // + int64  deleted_at_ms = 9;
}
```

**Tombstone-field precedent** — `SpaceRecord` lines 115-116 (`deleted = 7`, `deleted_at_ms = 8` set by no command, present from creation) is the exact convention for the message tombstone fields.

**`MessageState` enum already carries pending/complete/error** (lines 34-41): `MESSAGE_STATE_PENDING = 1`, `MESSAGE_STATE_COMPLETE = 4`, `MESSAGE_STATE_ERROR = 5` — reuse these for D-07 (no new enum values required; Claude's discretion).

**`GcsEvent` oneof to extend** (lines 92-100; next free tag is 6):
```proto
message GcsEvent {
  oneof payload {
    ChatMessageState message = 1;
    RoomList room_list = 2;
    Readiness readiness = 3;
    ErrorNotice error = 4;
    SpaceTree space_tree = 5;
    // + MessageHistory message_history = 6;
  }
}
```
New `MessageHistory` message follows the `SpaceTree` shape (line 164-169): a flat `repeated ChatMessageState message` + `string room_topic`.

---

### `src/app/lib/cubits/message_flow_cubit.dart` (store, event-driven — D-07 by-id upsert)

**Analog:** self. Keep `append` for live messages; add an `upsert` that replaces by `instanceId`.

**Current append-only mapping** (lines 47-49) and item builder (lines 55-62):
```dart
void append(ChatFlowItem item) {
  emit(ChatMessageFlowCubit.cappedItems(<ChatFlowItem>[...state, item]));
}

ChatFlowItemTextBubble buildChatFlowItemTextBubble(ChatMessageState message) {
  return ChatFlowItemTextBubble(
    instanceId: message.id,
    role: kRoleVariants[message.role] ?? 'system',
    state: kStateVariants[message.state] ?? 'complete',
    text: message.text,
  );
}
```

**Upsert model (~10 lines)** — the by-id replace the planner writes must preserve cap semantics via the generated `cappedItems` (already the single cap-invariant home, `chat_message_flow_cubit.dart` lines 48-55):
```dart
void upsert(ChatFlowItem item) {
  final List<ChatFlowItem> next = <ChatFlowItem>[
    for (final ChatFlowItem existing in state)
      if (existing.instanceId != item.instanceId) existing,
    item,
  ];
  emit(ChatMessageFlowCubit.cappedItems(next));
}
```

**`replaceAll` already exists** in the generated base (`chat_message_flow_cubit.dart` lines 68-70) — the D-06 history batch uses it directly; `MessageFlowCubit` can expose a thin `replaceAll` delegating to `ChatMessageFlowCubit.cappedItems`:
```dart
void replaceAll(List<ChatFlowItem> items) {
  emit(ChatMessageFlowCubit.cappedItems(items));
}
```

**Role/state variant maps** (lines 21-37) already cover `MESSAGE_ROLE_USER_PEER`, `MESSAGE_STATE_PENDING`, and `MESSAGE_STATE_ERROR` — pending/error chrome needs no new mapping.

---

### `src/app/lib/cubits/session_cubit.dart` (store, event-driven — D-06 messageHistory dispatch arm)

**Analog:** self — `_dispatchEvent` gains one arm; `handlePushedBytes` and the FFI lifecycle are unchanged.

**Dispatch switch to extend** (lines 364-387) — insert the `hasMessageHistory()` arm after `hasMessage()`; the `RailCubit.setRooms` full-replacement precedent (`rail_cubit.dart` lines 118-126) is the semantic model for `MessageFlowCubit.replaceAll`:
```dart
void _dispatchEvent(GcsEvent event) {
  if (event.hasSpaceTree()) { _railCubit?.setTree(...); return; }
  if (event.hasRoomList())  { _railCubit?.setRooms(...); return; }
  if (event.hasReadiness()) { emit(state.copyWith(isReady: event.readiness.ready)); return; }
  if (event.hasMessage()) {
    final MessageFlowCubit? flow = _messageFlowCubit;
    if (flow != null) {
      flow.append(flow.buildChatFlowItemTextBubble(event.message));  // -> may become upsert per D-07
    }
    return;
  }
  if (event.hasError()) { emit(state.copyWith(error: event.error.message)); }
}
```

**FFI allocate/copy/call/free seam** (lines 246-293 `start`, lines 313-334 `publishCommand`) — unchanged; the transport path (`GcsCommandTransport` abstract, lines 64-67) and `ComposerCubit` (`composer_cubit.dart` lines 82-96) are untouched this phase.

---

### `test/test_gcs_messaging.cpp` (test, event-driven — NEW)

**Analog:** `test/test_gcs_entities.cpp` (fixture, injected pubsub, wait-condition) — the exact template. It links the SAME libs the research mandates (`gcs_core;gcs_storage;neoswarm_common`, `test/CMakeLists.txt` line 76), which is why the messaging logic must live at the CoreSession/messaging-component layer, NOT only inside `gcs_core_ffi.cpp`.

**Fixture + injected pubsub + soralog setup** (`test_gcs_entities.cpp` lines 88-168) — copy verbatim; the `MakeStartedPubSub` helper (lines 131-149) is the Tier-2 seam:
```cpp
std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> MakeStartedPubSub(const std::string &keyDir) {
  sgns::crdt::KeyPairFileStorage keyStore(keyDir);
  auto keyPairResult = keyStore.GetKeyPair();
  // ...
  auto pubsub = std::make_shared<sgns::ipfs_pubsub::GossipPubSub>(keyPairResult.value());
  auto startFuture = pubsub->Start(0, {}, kListenIp, {}); // port 0 = ephemeral
  // ...
}
```

**Wait-condition usage** (lines 185-187; template in `test/test_wait_condition.hpp` lines 38-56) — never `sleep_for`:
```cpp
EXPECT_TRUE(gcs::test::WaitForCondition(
    [&dbPath]() { return std::filesystem::exists(dbPath); }, gcs::test::kWaitTimeout));
```

**Two-session convergence pattern** (lines 457-510 `EntitiesSurviveSessionRestartOnSharedDb`) — the SC3 convergence test shape: session A writes, shuts down, session B reads the same db_path.

**Register the target** in `test/CMakeLists.txt` following line 76:
```cmake
gcs_test(test_gcs_messaging test_gcs_messaging.cpp "gcs_core;gcs_storage;neoswarm_common")
```

---

### `test/test_gcs_global_db.cpp` (test, CRUD — extend for topics-aware Put + prefix scan)

**Analog:** self. The `LifecycleWithInjectedPubSub` test (lines 169-192) is the fixture; add `Put`-with-topics and `QueryKeyValues` round-trip tests using the same `MakeStartedPubSub` + `MakeGraphsyncContext` pair (lines 171-173).

---

### `src/app/test/cubits/shell_cubits_test.dart` (test, event-driven — extend for upsert + messageHistory)

**Analog:** self. `_RecordingBindings` (lines 33-109) and `_RecordingTransport` (lines 112-120) are the fakes; the `MessageFlowCubit` group (lines 157-216) and `handlePushedBytes decodes and dispatches` (lines 334-374) are the exact sites. New tests:
- upsert replaces pending→complete by `instanceId` with no duplicate (extend the `MessageFlowCubit` group).
- `handlePushedBytes` with a `messageHistory` event calls `replaceAll` (extend the dispatch test; build the `GcsEvent` via `..messageHistory = (MessageHistory()..roomTopic = ... ..message.add(...))`).
- `pumpEventQueue` for async propagation (already used, line 232) — no sleeps.

---

### `src/app/templates/components/chat_message_bubble.dart.jinja2` + `chat_message_bubble_vars.json` (template, transform — conditional sender chrome)

**Analog:** self. The app owns this consumer-space template set (generates `src/app/lib/generated/chat/chat_message_bubble.dart`).

**Current vars** (`chat_message_bubble_vars.json`) already declare the pending/error states:
```json
{
  "widget_class_name": "ChatMessageBubble",
  "file_stem": "chat_message_bubble",
  "roles": ["user_self", "user_peer", "assistant", "system"],
  "states": ["pending", "streaming", "thinking", "complete", "error"],
  "payload": "text"
}
```
The only NEW chrome is the `sender` short-form (D-04). Add a `sender` payload field to vars + jinja2 and regenerate — never hand-edit `src/app/lib/generated/chat/*`. Pending/error rendering already exists, so this is conditional on the UI decision to show sender inline. The scaffold submodule `lib/` is read-only (`src/app/scaffold/CLAUDE.md`); app-side rendering lives only under `src/app/`.

---

## Shared Patterns

### Error domain (`outcome::result` + `Error` enum)
**Source:** `src/lib/gcs_storage/common/error.hpp` (lines 26-34), `gcs_global_db.cpp` lines 47-59
**Apply to:** every new C++ service method (GcsGlobalDb, CoreSession, gcs_messaging)
```cpp
enum class Error : uint8_t {
  GcsDbError = 22,       ///< GCS GlobalDB operation failed
  SdkNotInitialized = 23, ///< GeniusSDKGetNode() returned nullptr
};
```
Every new operation: guard `if (!m_running.load()) return outcome::failure(Error::GcsDbError);`, map SuperGenius errors via `MapGlobalDbError`, return `outcome::success()`. Do NOT add new Error codes this phase (the store has no NotFound — `gcs_entity_store.cpp` line 231 documents why GcsDbError doubles as generic rejection).

### Logging (spdlog only)
**Source:** `gcs_core_ffi.cpp` lines 237, 259 (`spdlog::error/info` free functions) and `gcs_global_db.cpp` line 73 (`sgns::gcs::CreateLogger` component logger)
**Apply to:** all C++ code. Never `fprintf`/`cout`/`cerr`/`printf`. New messaging component uses a component logger like `CreateLogger("GcsMessaging")`; FFI-side additions use the existing `spdlog::error/info` free calls.

### Mutex discipline (thread safety)
**Source:** `gcs_core_ffi.cpp` lines 84-100 (`g_mutex` guards `g_session`, `g_entities`, topic sets) + `PostToDart` (294-312) caller contract ("Callers must hold g_mutex when the event is built from guarded state")
**Apply to:** the receive callback (SuperGenius `RegisterNewElementCallback` fires on the io thread per A5) and the join-replay scan. Both funnel into a single `OnMessageArrived`-style function that takes `g_mutex`; push ordering matters (history batch before live appends, D-06).

### Test infrastructure (wait-condition + injected pubsub)
**Source:** `test/test_wait_condition.hpp` (lines 38-56), `test/test_gcs_entities.cpp` lines 88-168
**Apply to:** `test_gcs_messaging.cpp` and the extended `test_gcs_storage`. No `std::this_thread::sleep_for` anywhere.

### Append-only proto discipline
**Source:** `src/proto/gcs_chat.proto` — Phase 2 additions (lines 102-169) append after a `// === Phase 2 ... ===` marker; never retype/remove/reorder fields.
**Apply to:** the Phase 3 additions (add `sender`/`deleted`/`deleted_at_ms` to `ChatMessageState`; add `MessageHistory` to `GcsEvent`). New marker `// === Phase 3: Messaging (append-only additions) ===`. Regenerate `src/app/lib/generated/proto/gcs_chat.pb.dart` — never hand-edit.

### Dart immutable-state + copyWith + thin-subscriber
**Source:** `rail_cubit.dart` lines 59-105 (immutable state, `copyWith`, unmodifiable lists), `session_cubit.dart` lines 71-103 (`SessionState.copyWith` with nullable-field `clearError` idiom)
**Apply to:** any state shape change from D-07/D-06. Cubits are thin subscribers; C++ owns all authority (D-04).

### Dart FFI allocate/copy/call/free
**Source:** `session_cubit.dart` lines 246-293 (`start`), 313-334 (`publishCommand`)
**Apply to:** unchanged this phase; no new FFI functions are added (the four-function ABI in `src/ffi/gcs_core.h` stays fixed — Phase 3 is payload/schema change, not ABI change).

---

## No Analog Found

Files/patterns with no close in-repo match — planner uses RESEARCH.md `[ASSUMED signature]` excerpts and must include the Wave-0 header-verification task (A1/A3/A4/A5):

| File / Pattern | Role | Data Flow | Reason |
|----------------|------|-----------|--------|
| `QueryKeyValues` prefix-scan wrapper on `GcsGlobalDb` | service | CRUD scan | No existing wrapper enumerates; `Get`/`Put` are single-key only. `EntityStore::LoadFromStore` (manifest + N+1 reads) is the pattern being superseded — use `Get`'s wrapper shape but verify the exact `QueryKeyValues(prefix)` return type against the resolved thirdparty header. |
| Receive-side dedupe set (apply-once keyed by id) | service | event-driven | No existing dedupe/bounded-set logic. `gcs_messaging` implements a global `std::unordered_set<std::string>` keyed by id (or per-room LRU — Claude's discretion) as the single funnel for both the pub/sub decode path and `RegisterNewElementCallback`. |
| `RegisterNewElementCallback` receive hook | service | event-driven | No existing callback registration in `GcsGlobalDb`. Exact typedef and return shape must be verified against `crdt_datastore.hpp` before wiring (NEO-SWARM 03-02-PLAN.md:120 warns "do not guess"). |
| `GeniusNode::GetAddress()` sender stamp (D-04) | utility | request-response | No existing sender/identity stamp. Verify `GetAddress()` exists on the SDK header; if absent, ask the user before substituting. |

---

## Metadata

**Analog search scope:** `src/lib/`, `src/lib/gcs_storage/`, `src/ffi/`, `src/proto/`, `src/app/lib/cubits/`, `src/app/lib/generated/`, `src/app/lib/shell/`, `src/app/templates/`, `test/`, `src/app/test/cubits/`
**Files scanned:** 20 (read in full: gcs_global_db.{hpp,cpp}, gcs_core.{hpp,cpp}, gcs_core_ffi.cpp, gcs_chat.proto, gcs_entity_store.{hpp,cpp}, message_flow_cubit.dart, session_cubit.dart, rail_cubit.dart, composer_cubit.dart, chat_message_flow_cubit.dart, gcs_shell.dart, test_gcs_global_db.cpp, test_gcs_entities.cpp, test_wait_condition.hpp, test/CMakeLists.txt, shell_cubits_test.dart, error.hpp, scaffold CLAUDE.md, bubble/flow template vars)
**Pattern extraction date:** 2026-09-23

**Planning notes carried into this map:**
- The research test map links `test_gcs_messaging` against `gcs_core;gcs_storage;neoswarm_common` (same as `test_gcs_entities`), NOT `gcs_ffi` — the send/receive/history/dedupe logic must be reachable at the `CoreSession`/`gcs_messaging` layer so it is testable without the FFI dylib. The FFI (`gcs_core_ffi.cpp`) is the thin command dispatcher over it.
- The SuperGenius API signatures (`QueryKeyValues`, `RegisterNewElementCallback`, `GetAddress`) are prebuilt-SDK headers absent from the local tree — all MEDIUM confidence (RESEARCH Assumptions A1/A3/A4/A5). Planner must schedule a Wave-0 signature-verification task before any C++ implementation.
