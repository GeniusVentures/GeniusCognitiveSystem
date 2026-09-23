# Phase 3: Messaging - Research

**Researched:** 2026-09-23 (updated same day — D-08 crypto surface verified in-repo: vendored OpenSSL version, gcs_core linkability, EVP AES-256-GCM pattern, HKDF availability, threading constraints, nonce strategy, KDF parameters)
**Domain:** P2P group messaging over a CRDT (SuperGenius GlobalDB) + GossipSub, with a Dart FFI-push render layer; full-record AES-256-GCM encryption of message payloads (D-08)
**Confidence:** HIGH (architecture/storage surface verified in-repo; SuperGenius API signatures MEDIUM — prebuilt SDK not in local tree; D-08 crypto surface HIGH — vendored OpenSSL headers, docs, archive symbols, and build machinery all verified in-repo this session)

<user_constraints>
## User Constraints (from CONTEXT.md)

### Locked Decisions

- **D-01:** **Per-message keys + converged prefix scan.** Each message is one serialized `ChatMessageState` record under `gcs/messages/<room_topic>/<message-id>` (generalizes Phase 2's per-entity key layout and the existing `room_topic + "/" + message-id` precedent in `gcs_core_ffi.cpp` ~line 709). Read-back enumerates via GlobalDB's **`QueryKeyValues`** converged prefix scan (verified exposed in SuperGenius `globaldb.hpp` ~line 144) — surfaced through `GcsGlobalDb`/`CoreSession` as a new API — then deserializes, sorts by `(timestamp, id)` (id tiebreak makes ordering total), and ships as one batch. Rejected: a per-room index manifest of message ids (the Phase 2 entity-manifest pattern) — a manifest held under one key is a read-union-write LWW lost-update hazard at message send rates, and it is unnecessary given the prefix scan exists. Single-writer-per-key LWW is exactly correct for immutable messages: no concurrent writes to the same key, so no lost-update path.
- **D-02:** **`GcsGlobalDb::Put` gets a topics parameter.** Today `Put` hardcodes `kNoTopics` (`gcs_global_db.cpp` ~lines 264-266) — every write is local-only, nothing replicates. Message Puts must pass `{room_topic}` so the CRDT broadcast replicates the archive. Fix is at the wrapper (add a topics-aware overload; keep the existing signature delegating to it with `kNoTopics`), not a new storage class.
- **D-03:** **GossipSub fast path for live delivery; CRDT as the archive.** Send (C++ side): mint id (`NextMessageId()` idiom) → stamp sender + timestamp → push local `pending` echo (D-06) → **publish the serialized `ChatMessageState` on the room's pub/sub topic** (live path, reuses Phase 1's topic infra) **and `Put` it with `{room_topic}`** (archive replication per D-02) → push `complete`. Receive: a pub/sub message arrives → decode → **apply-once keyed by message id** (dedupe — the same message will also arrive via CRDT sync) → render + `PutLocal` into the archive with no re-broadcast (no echo loops; graphsync heals any pub/sub delivery that was missed). Rejected: CRDT write-through as the only delivery path — render latency ties to graphsync convergence rather than the live topic. *(User choice, 2026-09-22: "GossipSub fast-path seems better as then the CRDT is just the archive of that message.")*
- **D-04:** **Sender identity = wallet address, stamped by C++, unsigned MVP.** The sender field is the local node's `GeniusNode::GetAddress()` (the embedded child wallet from `gcs_init`), set in the FFI send path — Dart never supplies or trusts a sender. Not cryptographically signed in this phase (any syncing node could forge a sender string); signing is v1.1 — payload encryption is in-phase per D-08 (encryption ≠ signing: the sender field stays unsigned and display-only). `ChatMessageState` gains a `sender` field (append-only, next free tag); UI renders it truncated/short-form.
- **D-05:** **Append-only field additions to `ChatMessageState`:** `sender` (D-04) and tombstone fields (`deleted` + `deleted_at_ms`) present from creation — greenfield now per Phase 2 D-03; Phase 5 moderation flips flags rather than migrating. Ordering fields (timestamp, id) already exist.
- **D-06:** **Join-time replay.** On `join_topic` (and derived joins from Phase 2's evaluator), C++ runs the D-01 prefix scan under the existing session mutex and pushes one new **`MessageHistory`** batch event (new `GcsEvent` oneof arm, append-only) containing the full sorted room history; Dart replaces the room's message list (`replaceAll` semantics, mirroring `RailCubit.setRooms`). Live messages append after the batch — the apply-once dedupe set (D-03) absorbs any pub/sub message that lands between scan and subscribe. No incremental pagination, no scroll-windowing API for MVP (the client-side cap bounds rendering).
- **D-07:** **Pending echo + by-id upsert.** C++ pushes the sender's own message twice — once as `pending` (optimistic echo at send-accept) and once as `complete` with the same id; failures push `error` for that id. `MessageFlowCubit` upserts by message id (~10-line change to the current append-only mapping) so pending → complete/error replaces in place rather than duplicating. Error is a **terminal** tint; re-send is a **manual user action** (re-submit via the composer path with a new id) — no auto-retry loop. Transport-level failures (topic publish throws) additionally surface a `showToast`; send-path validation failures stay inline per the Phase 2 dialog pattern. The composer itself is unchanged — Enter and the button already converge on one submit path in `composer_cubit.dart`.
- **D-08:** **Full-record encrypted storage & transport, Matrix-shaped envelope, phased key distribution.** The serialized `ChatMessageState` is encrypted as a whole before BOTH the live `Publish` (D-03) and the archive `Put`/`PutLocal` — nonce-prefixed AES-256-GCM via the vendored OpenSSL (EVP; exact signatures + gcs_core linkability pinned below and in 03-01). Decryption happens only in the local `Messaging` layer (`ApplyMessage` funnel and `QueryHistory` scan) before parse/dedupe/sort — the storage layer stays opaque-bytes, `gcs_chat.proto` stays unchanged (bytes are bytes), and Dart is unchanged (C++ decrypts before push). One group session per room; interim Phase 3 key = HKDF(room_topic), because no member roster exists until Phase 4 membership — Phase 4 swaps in Matrix/Megolm-style member-key distribution (session keys encrypted to member identity keys) with **no record-format change**. All Phase 3 rooms are encrypted by default (Element's current default for private rooms). Encryption is encapsulated behind **injected crypto functions** (an encrypt/decrypt seam on the `Messaging` component — it never calls OpenSSL directly): when the room/messaging does not have encryption enabled, the injected functions are simply not called and payloads flow as plaintext — one component, two paths. The disabled path exists now so both encrypted and plaintext behavior are testable in this phase, and the v1.1 per-room opt-out flag needs no architectural change. Records that fail decryption (wrong/missing key) skip-and-log — the same posture as unparseable records; disk and wire carry ciphertext + nonce only. *(User choice, 2026-09-23: follow what Element Matrix does for E2E-encrypted topics/channels — full record, both paths, envelope now / key swap at Phase 4.)*

### Claude's Discretion
- Proto field names/numbering within the append-only discipline (`sender`, tombstone fields, `MessageHistory` event shape).
- Dedupe-set implementation in C++ (fixed-size LRU per room vs global keyed map) and its retention policy.
- Exact prefix string layout (`gcs/messages/<room_topic>/<id>` — slash-vs-colon details) as long as it is one stable prefix per room and consistent with the scan.
- Whether `QueryKeyValues` is exposed on `GcsGlobalDb` as a raw passthrough returning key/value byte pairs or as a decoded-message helper.
- Pending/complete/error state representation in the pushed `ChatMessageState` (reuse `MessageState` enum values vs new values — append-only either way).
- Dart-side history `replaceAll` wiring details and any client-side render cap constant.
- Exact HKDF parameters, nonce size, and the OpenSSL EVP call pattern for D-08 (pinned below in this research; recorded in the 03-01 signature record before implementation).
- The injection shape of the D-08 crypto seam (pair of `std::function` encrypt/decrypt vs a small interface) — as long as `Messaging` itself never calls OpenSSL directly and the seam is injectable per room/messaging instance.
- Whether decryption failure logs at `spdlog::warn` or debug level.

### Deferred Ideas (OUT OF SCOPE)
- Cryptographic message signing + sender verification — v1.1 (D-04 keeps the field unsigned; payload encryption moved into Phase 3 per D-08).
- Megolm-style member-key distribution + key rotation when members leave (ENCR-02) — Phase 4 membership; the D-08 envelope is designed so only the distribution swaps, not the record format.
- Per-room encryption opt-in/opt-out flag + room settings UI — v1.1+ (all Phase 3 rooms are encrypted by default per D-08).
- DM auto-encryption (ENCR-03) — DMs don't exist yet.
- Element-style device verification (emoji/key verification) — v1.1+.
- Reply threading / `reply_to_id` causal ordering (Lamport/HLC stamps, vector clocks) — bot phase; revisit only if bot turns need causal ordering. Simple `(timestamp, id)` sort is locked for Phase 3.
- Message deletion UI + moderator flows — Phase 5 Moderation; tombstone fields land now (D-05).
- Pagination / scroll-windowed history loading — post-MVP; batch `replaceAll` is locked for Phase 3.
- Auto-retry of failed sends — rejected for MVP (manual re-send only, D-07); revisit with a backoff policy if UAT shows transport flakiness.
- @mentions and GCS bot auto-answer — Phase 5+.
</user_constraints>

<phase_requirements>
## Phase Requirements

| ID | Description | Research Support |
|----|-------------|------------------|
| CORE-04 | User can send and receive text messages in real-time | D-02 (topics-aware Put), D-03 (GossipSub fast path + CRDT archive), D-06 (join-time replay), D-07 (pending echo + by-id upsert), D-08 (full-record AES-256-GCM envelope on both paths, verified crypto surface below) |

Success criteria trace (roadmap SC 1-4):
1. "Send and see it locally" → D-07 pending echo at send-accept (C++ pushes before any network round-trip).
2. "Second user receives without manual refresh" → D-03 live GossipSub publish on the room topic; receive path decrypts (D-08), decodes, and pushes to Dart.
3. "History identical across participants after sync" → D-01 converged `QueryKeyValues` prefix scan over per-message keys; D-02 topics-aware Put replicates the archive; D-08 wrong-key/failed-decrypt records skip-and-log so one bad record never hides the rest.
4. "Chronological order with sender identification" → D-01 `(timestamp, id)` total sort; D-04 sender = wallet address rendered short-form.
</phase_requirements>

## Project Constraints (from CLAUDE.md / submodule contracts)

No root `./CLAUDE.md` exists in this repo (`config.json` points to `./CLAUDE.md`; the file is absent). Binding constraints come from three sources and are treated as locked:

**C++ coding standards (user global instructions, apply to all `.cpp/.hpp` in `src/`):**
- C++17 ceiling — no C++20 features (no `boost::coroutines`, no concepts/ranges beyond C++17).
- No OS preprocessor guards in source (`#ifdef __APPLE__` etc.); platform code lives in `os/{platform}/Platform.hpp`.
- No magic numbers — `constexpr` `kCamelCase` constants (exceptions: 0, 1, -1 in trivial contexts).
- Allman/Ullman bracing, Doxygen headers on every function/public interface, all variables initialized.
- Diagnostics via `spdlog::debug/error/warn` only — never `fprintf`/`cout`/`cerr`/`printf`.
- Tests use wait-condition templates (`test/test_wait_condition.hpp`) — **never** `std::this_thread::sleep_for`. Target ≥80% coverage on new code.
- Only thirdparty libraries (no system-installed); a new library goes through the thirdparty build system. **D-08 needs no new thirdparty package — OpenSSL is already vendored** (verified below).
- Program to interfaces, service-locator over singletons, composition over inheritance.
- **No try-and-retry / no debug-printf development** — analyze, propose minimal change, verify by tests.

**Scaffold submodule (`src/app/scaffold/CLAUDE.md` — public contract, read-only):**
- `lib/` is read-only; never edit generated widget families (`scaffold_{card,state_view,search_bar}*`, `scaffold_{animated_display,formatted_value,image_placeholder,selection_indicator}_*`). They are committed Jinja2 output — edit `templates/components/*.jinja2` + regenerate.
- `dart analyze --fatal-infos` must be clean; `flutter test` must pass.
- App-side message-list/sender/pending/error rendering belongs in the app's own consumer space (`src/app/lib/`, `src/app/templates/`), NOT in scaffold `lib/`.

**Inherited phase contracts (already enforced in code):**
- Append-only proto evolution (`gcs_chat.proto` — only add fields/messages/enums; never retype/remove/reorder). Phase 1 D-24/D-26, Phase 2 D-01..D-04. **D-08 changes no proto field — the envelope is opaque bytes at the existing `bytes`/string boundaries.**
- C++ owns state; Dart Cubits are thin subscribers rendering pushed events (Phase 1 D-04/D-27).
- Tombstones at record creation, never physical key removal (Phase 2 D-03).
- Opaque C++-minted ids; data-only Dart commands (Phase 2 D-01, Phase 1 D-27).

## Summary

Phase 3 turns the existing Phase 1 "local echo" into a real P2P messaging path. Today the `send_text` FFI arm (`src/ffi/gcs_core_ffi.cpp` ~lines 667-720) mints an id, stamps role/state/text/timestamp, writes the message locally via `GcsGlobalDb::Put` (which hardcodes `kNoTopics`, so nothing replicates), and pushes the single `ChatMessageState` back to Dart. There is no pub/sub publish, no receive-side callback, and no history read-back — two peers in the same room currently cannot exchange messages at all.

Phase 3 closes that gap with three coordinated changes, all grounded in code verified in-repo:
1. **Storage widening** — `GcsGlobalDb::Put` gains a topics-aware overload (the 3-arg `GlobalDB::Put(key, value, topicSet)` already exists in the SuperGenius API — the local `gcs_global_db.cpp:266` already calls it with an empty set). A new `QueryKeyValues`-backed prefix scan is surfaced through `GcsGlobalDb`/`CoreSession` for history read-back.
2. **FFI send/receive rewrite** — the send path publishes the serialized `ChatMessageState` on the room topic (live GossipSub) AND `Put`s it with `{room_topic}` (archive), pushing `pending` then `complete`; the receive path (via SuperGenius `RegisterNewElementCallback` + pub/sub decode) applies-once by message id and `Put`s locally without re-broadcast.
3. **Proto + Dart** — `ChatMessageState` gains `sender` + tombstone fields; `GcsEvent` gains a `MessageHistory` batch arm; `MessageFlowCubit` gains by-id upsert (its generated base `ChatMessageFlowCubit` already has `replaceAll`); `SessionCubit` gains a `messageHistory` dispatch arm.

**D-08 adds a fourth layer over all of that:** the serialized record is encrypted whole before BOTH the `Publish` and the `Put`, and decrypted only inside the `Messaging` layer before parse/dedupe/sort. The crypto surface for this is fully verified in-repo this session (details in the next section): the vendored OpenSSL is **3.3.3** (headers, static libs, and archive symbols all confirmed at the resolved thirdparty build path), the `gcs_core` target can link it directly via the **`OpenSSL::Crypto` imported target** that the root `CMakeLists.txt:31` `find_package(OpenSSL REQUIRED)` already defines, `EVP_aes_256_gcm` + `EVP_PKEY_HKDF` + `RAND_bytes` are all exposed by the resolved headers and present as defined symbols in `libcrypto.a`, and the vendored OpenSSL docs pin the exact GCM ctrl ordering and HKDF derivation pattern. Envelope layout is pinned: **`value = nonce(12) || ciphertext || tag(16)`**, with a per-message random 96-bit nonce from `RAND_bytes`.

**Primary recommendation:** Implement in the C++ layer first (storage widening → crypto adapter + seam → send/receive path → join-time replay), then the append-only proto additions, then the thin Dart dispatch/upsert changes. The SuperGenius CRDT API names (`QueryKeyValues`, `RegisterNewElementCallback`, topics-parameter `Put`) are confirmed by the NEO-SWARM submodule's own `03-gcs-globaldb-integration` planning artifacts and the local `gcs_global_db.cpp`, but the **exact signatures** must be re-verified against the prebuilt SDK headers at plan time (the SDK is not in the local tree — see Environment Availability). The D-08 crypto surface has no such gap — everything below is verified against the actual repos.

## D-08 Crypto Surface — Verified Facts

> Every claim in this section was verified against the actual repos this session (2026-09-23). Paths are relative to the GCS repo root unless they start with the shared Network root `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/`.

### 1. Vendored OpenSSL version and resolved include dirs

| Item | Verified Value | Evidence |
|------|----------------|----------|
| Vendored source tree | `../thirdparty/openssl/` — full OpenSSL source checkout, `VERSION.dat` = MAJOR 3 / MINOR 3 / PATCH 3 / PRE_RELEASE_TAG `dev` (3.3.3-dev) | `thirdparty/openssl/VERSION.dat` |
| Resolved build (what compiles actually use) | **OpenSSL 3.3.3** — `OPENSSL_VERSION_STR "3.3.3"`, MAJOR 3 / MINOR 3 / PATCH 3 | `thirdparty/build/OSX/Debug/openssl/build/include/openssl/opensslv.h` |
| Resolved include dir (`-isystem`) | `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/thirdparty/build/OSX/Debug/openssl/build/include` | `build/OSX/Debug/compile_commands.json` — appears on the compile lines of `src/lib/gcs_storage/gcs_global_db.cpp`, `src/lib/gcs_core.cpp`, `src/lib/gcs_entity_store.cpp`, `src/ffi/gcs_core_ffi.cpp`, and all 6 GCS test files (34 compile commands carry it) |
| Built libraries | `libcrypto.a` (45.4 MB) + `libssl.a` (12.9 MB), static, plus cmake/ and pkgconfig/ export dirs | `thirdparty/build/OSX/Debug/openssl/build/lib/` |
| Build machinery | ExternalProject `openssl` from `THIRDPARTY_DIR/openssl`, installs to `${CMAKE_CURRENT_BINARY_DIR}/openssl/build`, byproducts `lib/libssl.a`, `lib/libcrypto.a`, `include/openssl/ssl.h`; `OPENSSL_ROOT_DIR` set at line 104 | `thirdparty/build/OSX/CMakeLists.txt:103-120` |
| Configure flags | `no-asm enable-threads no-shared` (darwin64-x86_64-cc and darwin64-arm64-cc) — static, thread-enabled; **no** feature-disabling flags (no `no-hkdf` etc.) | `thirdparty/build/OSX/Openssl-build/build.sh:39,47` |
| Cross-platform | Openssl-build dirs exist for OSX, iOS, Android (verified); `thirdparty/build/{Linux,Windows,iOS,OSX,Android,mobile}` all present | directory listing of `thirdparty/build/` |
| Perf note | `no-asm` means no AES-NI/ARMv8 assembly paths — GCM throughput is lower than an asm build. Irrelevant at chat sizes (records ≤ ~5 KB serialized; text capped at 4096). | `build.sh:39` |

Only `libcrypto` is needed for D-08 (EVP + HKDF + RAND all live in libcrypto; `libssl` is TLS protocol).

### 2. gcs_core linkability — the exact mechanism

**Answer: yes, `gcs_core` can use OpenSSL directly today, with zero build-subsystem changes.** Two mechanisms are already in place:

**Include path (compiles today, no edit needed):**
- Root `CMakeLists.txt:31` — `find_package(OpenSSL REQUIRED)`.
- Root `CMakeLists.txt:32` — `include_directories(${OPENSSL_INCLUDE_DIR})` — directory-scoped, so every target added after it in this dir and all subdirectories (including `src/` added at line 52) already compiles `#include <openssl/evp.h>` cleanly. This is why `-isystem .../openssl/build/include` appears on every GCS compile line in `compile_commands.json`.

**Link (already present transitively; make it explicit):**
- `find_package(OpenSSL)` in module mode (CMP0167 pinned OLD at root `CMakeLists.txt:3-5`) defines the imported targets **`OpenSSL::Crypto`** and `OpenSSL::SSL`, resolved to the vendored statics: CMakeCache `OPENSSL_CRYPTO_LIBRARY` = `.../openssl/build/lib/libcrypto.a` (line 437), `OPENSSL_SSL_LIBRARY` (line 452), `OPENSSL_ROOT_DIR` (line 449), and `FIND_PACKAGE_MESSAGE_DETAILS_OpenSSL:INTERNAL=[...libcrypto.a][...include][c][v3.3.3()]` (line 764) — find_package is already resolving to the **vendored 3.3.3**, not the system copy.
- The vendored `libcrypto.a` + `libssl.a` **already sit on the final link lines** of `gcs_ffi.dylib` and the six GCS test executables (verified in `build/OSX/Debug/build.ninja`), arriving transitively through SuperGenius exported targets (`$<LINK_ONLY:OpenSSL::Crypto>` in `SuperGenius/build/OSX/Debug/SuperGenius/lib/cmake/SuperGenius/supergeniusTargets.cmake`). `gcs_core` is a STATIC library (`src/CMakeLists.txt:4-7`), so its OpenSSL usage lands on exactly those consumer link lines — no new plumbing.
- **The one-line explicit change:** add `target_link_libraries(gcs_core PUBLIC OpenSSL::Crypto)` in `src/CMakeLists.txt`. This works because find_package runs at root line 31, before `add_subdirectory(src)` at line 52, so the imported target exists when `src/CMakeLists.txt` is processed. PUBLIC so test executables linking only `gcs_core` still resolve. This documents the direct dependency and keeps it correct if SuperGenius ever drops the transitive linkage; functionally the symbols already link.

**Hazard (verified, must avoid):** the CMakeCache also contains pkg-config entries `pkgcfg__OPENSSL_ssl/crypto` pointing at **homebrew 3.6.3** (`/opt/homebrew/Cellar/openssl@3/3.6.3/lib`, CMakeCache lines 552-555). Never consume OpenSSL via pkg-config or bare `-lssl -lcrypto` — that silently links the system library and violates the no-system-libraries rule. The `OpenSSL::Crypto` target (and `${OPENSSL_CRYPTO_LIBRARY}` cache var) is the only sanctioned handle; both resolve to the vendored static.

### 3. EVP AES-256-GCM — exact C++17 call pattern (verified in resolved headers + vendored docs)

All symbols verified in the resolved header `thirdparty/build/OSX/Debug/openssl/build/include/openssl/evp.h`:

| Symbol | evp.h line |
|--------|-----------|
| `EVP_aes_256_gcm(void)` | 1061 |
| `EVP_EncryptInit_ex(ctx, cipher, impl, key, iv)` | 760 |
| `EVP_EncryptUpdate(ctx, out, outl, in, inl)` | 768 |
| `EVP_EncryptFinal_ex(ctx, out, outl)` | 770 |
| `EVP_DecryptInit_ex` | 777 |
| `EVP_DecryptUpdate` | 785 |
| `EVP_DecryptFinal_ex` | 789 |
| `EVP_CIPHER_CTX_new` / `_reset` / `_free` | 882 / 883 / 884 |
| `EVP_CIPHER_CTX_ctrl(ctx, type, arg, ptr)` | 887 |
| `EVP_CTRL_GCM_SET_IVLEN` / `GET_TAG` / `SET_TAG` (aliases of `EVP_CTRL_AEAD_*`) | 382-384 |

Semantics verified from the vendored docs (`thirdparty/openssl/doc/man3/EVP_EncryptInit.pod`):
- **IV length:** default for GCM AES is **12 bytes (96 bits)** (pod:1366-1369). `EVP_CTRL_GCM_SET_IVLEN` is only needed when deviating — for a 12-byte nonce it can be omitted, but calling it explicitly is harmless and self-documenting.
- **Tag:** `GET_TAG` is valid only when encrypting and **after** `EVP_EncryptFinal` (pod:1372-1376). `SET_TAG` when decrypting sets the expected tag (1-16 bytes) and **must** be called before `EVP_DecryptFinal` (pod:1378-1387). Default/max tag length is 16.
- **Ciphertext length == plaintext length:** GCM is a stream-style mode ("can handle 1 byte at a time", pod:1351) — no padding, so `EncryptFinal` emits 0 bytes for GCM.
- All EVP calls return `1` on success, `0`/negative on failure — every call must be checked (tag mismatch surfaces as `EVP_DecryptFinal_ex` returning ≤ 0).

Working code pattern (also in Code Examples below): encrypt = `CTX_new` → `EncryptInit_ex(EVP_aes_256_gcm(), key, nonce)` → `EncryptUpdate` → `EncryptFinal_ex` (0 out bytes) → `ctrl(GET_TAG, 16)` → `CTX_free`; decrypt = `CTX_new` → `DecryptInit_ex` → `DecryptUpdate` → `ctrl(SET_TAG, 16, expected)` → `DecryptFinal_ex` (≤ 0 = wrong key / tampered / corrupted → skip-and-log per D-08).

### 4. HKDF — available in the vendored OpenSSL (3.3.3 ≥ 1.1.0)

Verified in the resolved headers:
- `EVP_PKEY_HKDF` = `NID_hkdf` — **evp.h:76**; `NID_hkdf = 1036` — `obj_mac.h:5463`.
- Legacy PKEY HKDF setters, all present and **not** marked deprecated in 3.3 — `kdf.h:105-117`: `EVP_PKEY_CTX_set_hkdf_md`, `EVP_PKEY_CTX_set1_hkdf_salt`, `EVP_PKEY_CTX_set1_hkdf_key`, `EVP_PKEY_CTX_add1_hkdf_info`, `EVP_PKEY_CTX_set_hkdf_mode`.
- Supporting: `EVP_PKEY_CTX_new_id` (evp.h:1785), `EVP_PKEY_CTX_free` (evp.h:1792), `EVP_PKEY_derive_init` (evp.h:1927), `EVP_PKEY_derive` (evp.h:1932), `EVP_sha256` (evp.h:922).
- Canonical derivation example is in the vendored docs: `thirdparty/openssl/doc/man3/EVP_PKEY_CTX_set_hkdf_md.pod:122-141`.
- The modern `EVP_KDF_fetch("HKDF")` API also exists (kdf.h:24-49) — not needed; the PKEY API is simpler (no `OSSL_PARAM` construction) and equally supported.

**Fallback not required:** no HMAC-SHA256 hand-rolled HKDF-expand is needed. The exact pattern: `EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, NULL)` → `EVP_PKEY_derive_init` → `set_hkdf_md(EVP_sha256())` → `set1_hkdf_salt(salt, len)` → `set1_hkdf_key(ikm, len)` → `add1_hkdf_info(info, len)` → `EVP_PKEY_derive(out, &outlen)` → `EVP_PKEY_CTX_free`. Default mode is EXTRACT_AND_EXPAND (kdf.h:90) — do not call `set_hkdf_mode`.

### 5. Threading constraint for the crypto seam

Threads that invoke the Messaging crypto paths (verified):
| Path | Thread | Evidence |
|------|--------|----------|
| `SendMessage` (encrypt) | FFI command thread (the Dart platform thread calling `gcs_publish` synchronously) | Phase 1 FFI contract; `gcs_publish` is a direct call |
| `ApplyMessage` via live GossipSub | GossipPubSub strand thread | `thirdparty/build/OSX/Debug/ipfs-pubsub/include/ipfs_pubsub/gossip_pubsub.hpp:305` (`std::shared_ptr<boost::asio::io_context::strand> m_strand`; Subscription wraps `strand_`, lines 81-88) |
| `ApplyMessage` via CRDT sync (`RegisterNewElementCallback`) | CRDT DagWorker threads (`std::async(std::launch::async, ...)`) | `../SuperGenius/src/crdt/impl/crdt_datastore.cpp:79-88` (worker creation), `:2071-2074` (`PutElementsCallback` → `crdt_cb_manager_.PutDataCallback`), reached through the CrdtSet observer lambda at `:63-73` |
| `QueryHistory` scan (decrypt) | FFI command thread (join_topic / derived joins under `g_mutex`) | `gcs_core_ffi.cpp` join arm + `RefreshDerivedJoins` |

OpenSSL's own threading rules (vendored `thirdparty/openssl/doc/man7/openssl-threads.pod`):
- "Objects are thread-safe as long as the API's being invoked don't modify the object" (pod:50).
- "it is generally not safe for one thread to mutate an object … while another thread is using it" (pod:63).
- "The same API's can usually be used simultaneously on different objects without interference. For example, two threads can calculate a signature using two different `EVP_PKEY_CTX` objects." (pod:67-70).

**Constraint (state this in code Doxygen on the crypto adapter):** `EVP_CIPHER_CTX` and `EVP_PKEY_CTX` are mutable state machines — a shared/member ctx is NOT safe across the three thread types above. The crypto adapter must:
1. Create a **fresh `EVP_CIPHER_CTX` per encrypt/decrypt call** (`EVP_CIPHER_CTX_new` at function entry, `EVP_CIPHER_CTX_free` at exit — RAII wrapper or careful early-return discipline), and a fresh `EVP_PKEY_CTX` per HKDF derivation.
2. Hold **no mutable shared state**. Recommended MVP shape: derive the room key per call (HKDF is two HMAC passes over ≤128 bytes — microseconds; at chat rates per-message derivation is free) so the adapter is stateless and thread-safe by construction. A key cache would need its own mutex; not worth it this phase.
3. Use `RAND_bytes` for nonces — thread-safe (internally-locked DRBG; rand.h:61, symbol verified in the archive). Do NOT use `std::random_device`.

### 6. Nonce strategy and envelope layout (PINNED)

- **Nonce:** 12 random bytes per message from `RAND_bytes` (`rand.h:61`; `_RAND_bytes` verified as a defined symbol in `libcrypto.a`). 12 bytes is the GCM default IV length (EVP_EncryptInit.pod:1366-1369) so no `SET_IVLEN` ctrl is required.
- **Layout (explicit and consistent everywhere):**
  ```
  stored/published value = nonce (12 bytes) || ciphertext (== plaintext length) || tag (16 bytes)
  ```
  Nonce first (D-08's "nonce-prefixed"), tag last (it is produced after `EncryptFinal`, so appending is the natural write order). Decrypt splits: first `kGcmNonceLength` bytes = nonce, last `kGcmTagLength` bytes = tag, middle = ciphertext; minimum valid envelope = 28 bytes + record. Constants: `constexpr size_t kGcmNonceLength = 12; constexpr size_t kGcmTagLength = 16;` (plus `kRoomKeyLengthBytes = 32`) — no magic numbers.
- **Collision math:** with a random 96-bit nonce under one key, collision probability ≈ n²/2⁹⁷ (birthday). The key is per-room (HKDF(room_topic)), so n counts messages in ONE room: n = 2²⁰ (a million messages in one room) → p ≈ 2⁻⁵⁷ ≈ 7×10⁻¹⁸; even n = 2³² → p ≈ 2⁻³³. NIST SP 800-38D bounds random-IV GCM at 2³² encryptions per key [CITED: NIST SP 800-38D, §8 / §5.2.1.1] — Phase 3 per-room message volumes are orders of magnitude below any concern. (External standard, cited — not repo-verifiable; the math itself is arithmetic on the verified 96-bit nonce.)

### 7. Key derivation parameters (INTERIM until Phase 4 — CONTEXT D-08)

```
room_key = HKDF-SHA256(
    ikm  = room_topic UTF-8 bytes,          // the room topic string, e.g. "gcs/chat/<id>"
    salt = fixed constant (non-secret),     // recommended: "gcs-messages-hkdf-v1" (constexpr)
    info = "gcs-messages-v1",               // context/domain separator (constexpr)
    L    = 32 bytes)                        // AES-256 key
```
- Mode: default EXTRACT_AND_EXPAND (kdf.h:90) — no `set_hkdf_mode` call.
- HKDF salt need not be secret; a fixed constant salt is standard when the ikm is already key-like. The **security-limiting input is the room topic string, which is public** (it is the GossipSub topic name and the CRDT key prefix) — see Security Domain for the honest scope statement.
- Phase 4 membership replaces ONLY this derivation (member-key distribution); the envelope format and the seam stay unchanged (D-08 lock).

### Crypto seam shape (discretion item — research recommendation)

Per CONTEXT discretion ("pair of `std::function` vs a small interface — as long as `Messaging` never calls OpenSSL directly"), recommended:
- Two `std::function` callables injected into `Messaging` construction, both optional (`std::optional` or empty = plaintext path):
  - `EncryptFn: outcome::result<std::string>(const std::string &roomTopic, const std::string &plaintext)` → envelope
  - `DecryptFn: outcome::result<std::string>(const std::string &roomTopic, const std::string &envelope)` → plaintext
- The OpenSSL-backed implementation lives in a **new `src/lib/gcs_crypto.{hpp,cpp}`** (namespace `gcs::crypto`: `DeriveRoomKey(roomTopic)`, `EncryptRecord(key, nonce, plain)`, `DecryptRecord(key, envelope)` or a single pair of entry functions matching the seam signatures). This is the ONLY file that includes `<openssl/*>`; it is linked into `gcs_core`; the FFI/session composition injects it into `Messaging`.
- Tests exercise both paths per D-08: with the real adapter injected (round-trip, wrong-key, tamper) and with no adapter (plaintext flows unchanged).
- Binary-safety note: envelope and record travel as `std::string` (may contain NUL bytes) — never `c_str()`/`strlen()` on ciphertext; `GcsGlobalDb::Put` already passes `std::string` by value into `GlobalDB::Buffer::put` (size-aware); the GossipSub `Publish` takes `std::vector<uint8_t>` — convert with explicit begin/end iterators.

## Architectural Responsibility Map

| Capability | Primary Tier | Secondary Tier | Rationale |
|------------|-------------|----------------|-----------|
| Message id minting (`NextMessageId`) | API/Backend (C++ FFI) | — | C++ owns authority (D-04); id is the CRDT key suffix and must be process-unique (CR-01). |
| Sender identity stamping | API/Backend (C++ FFI) | — | `GeniusNode::GetAddress()` only exists C++-side; Dart never supplies/trusts a sender (D-04). |
| Live message delivery (send + receive push) | API/Backend (C++ GossipSub) | — | GossipSub fast path (D-03); topic infra already wired via `AddBroadcastTopic`/`AddListenTopic`. |
| CRDT archive replication | Database/Storage (GlobalDB) | API/Backend (C++ wrapper) | `Put` with `{room_topic}` replicates via the room's CRDT broadcast topic (D-02). |
| History read-back (prefix scan) | Database/Storage (GlobalDB `QueryKeyValues`) | API/Backend (C++ sort + batch) | Converged prefix scan over `gcs/messages/<room_topic>/` (D-01); C++ decrypts, deserializes, sorts `(timestamp, id)`. |
| Payload encryption/decryption (D-08) | API/Backend (C++ `Messaging` layer via injected seam) | — | Decrypt-only-in-Messaging (D-08): `ApplyMessage` funnel + `QueryHistory` scan decrypt before parse/dedupe/sort; storage stays opaque-bytes; Dart never sees ciphertext. |
| Key derivation (HKDF-SHA256 per room) | API/Backend (C++ `gcs::crypto` adapter) | — | Interim key = HKDF(room_topic) until Phase 4 membership (D-08); adapter is stateless/thread-safe (verified constraint above). |
| Receive-side dedupe + local archive write | API/Backend (C++) | Database/Storage | Apply-once keyed by id absorbs the pub/sub↔CRDT double-delivery; `PutLocal` = empty-topic Put (no echo loops). |
| Message rendering (sender, pending/error/complete, chronological list) | Browser/Client (Flutter) | — | Thin subscriber; all state arrives pushed and already decrypted by C++ (D-08 — Dart unchanged). |
| Composer send path | Browser/Client (Flutter) | — | Already converges on one submit path (`composer_cubit.dart`); unchanged this phase. |
| Join-triggered history replay | API/Backend (C++) | Browser/Client (Flutter replaceAll) | Hooked on `join_topic` + `RefreshDerivedJoins` (D-06); Dart mirrors `RailCubit.setRooms` full-replacement. |

## Standard Stack

### Core
| Library | Version | Purpose | Why Standard |
|---------|---------|---------|--------------|
| SuperGenius GlobalDB | prebuilt (thirdparty release) | CRDT store: per-key LWW, `QueryKeyValues` prefix scan, `RegisterNewElementCallback`, topics-parameter `Put` | Already owned by `GcsGlobalDb`; the CRDT substrate for CORE-05 (Phase 1). No alternative. |
| libp2p GossipPubSub | prebuilt (via SuperGenius/GeniusSDK) | Live room-topic message delivery (D-03 fast path) | Already wired (`AddBroadcastTopic`/`AddListenTopic`); reuses Phase 1 topic infra. |
| **OpenSSL (vendored)** | **3.3.3** (verified: `opensslv.h` OPENSSL_VERSION_STR at `thirdparty/build/OSX/Debug/openssl/build/include`) | AES-256-GCM (EVP), HKDF-SHA256, RAND_bytes — the entire D-08 crypto surface | Already vendored + built by the thirdparty ExternalProject (`thirdparty/build/OSX/CMakeLists.txt:103-120`); already linked transitively into every GCS final artifact; find_package resolves to it (CMakeCache line 764). No new dependency. |
| protobuf (C++) | thirdparty (add_proto_library) | `gcs_chat.proto` wire contract, both halves | Append-only schema source (D-29); `protoc` is the both-sides generator. D-08 changes no field. |
| protobuf (Dart) | 4.2.0 (locked) | Generated `gcs_chat.pb.dart` decode/encode | Matches the C++ wire; `protoc_plugin` 22.5.0 dev-dep. |
| flutter_bloc | 9.1.1 (locked) | Cubit state holders (`MessageFlowCubit`, `SessionCubit`, `ComposerCubit`, `RailCubit`) | Established in Phases 1-2. |
| frontend_scaffold | path `scaffold` (submodule pin `8ada743`) | Message-list atoms, `showToast`, composer | Read-only public contract (scaffold CLAUDE.md); app composites live in `src/app/`. |

### Supporting
| Library | Version | Purpose | When to Use |
|---------|---------|---------|-------------|
| spdlog | thirdparty | C++ diagnostics (incl. D-08 decrypt-failure skip-and-log) | Mandatory for all new logging (no stdio). |
| GTest + `test_wait_condition.hpp` | thirdparty | C++ unit/integration tests (incl. crypto round-trip/wrong-key/plaintext-path) | Wait-condition templates only; no `sleep_for`. |
| ffi / fixnum (Dart) | 2.2.0 / 1.1.1 (locked) | FFI bindings; int64 proto fields | Already wired; unchanged. |
| Boost.Asio | prebuilt (via SuperGenius) | io_context/scheduler for GlobalDB | Already owned by `GcsGlobalDb`; unchanged. |

### Alternatives Considered
| Instead of | Could Use | Tradeoff |
|------------|-----------|----------|
| `QueryKeyValues` prefix scan (D-01) | Per-room id-manifest key (Phase 2 pattern) | Manifest is a read-union-write LWW lost-update hazard at send rates; rejected (locked). |
| Topics-aware `Put` (D-02) | CRDT write-through as the only delivery path | Render latency ties to graphsync convergence; rejected in favor of GossipSub fast path (locked). |
| `PutConvergentImmutable` (exists in GlobalDB ~line 104) | Regular `Put` with topics (locked D-01/D-02) | Immutable-put may skip LWW merge, but single-writer-per-key makes LWW already correct; regular Put is locked. |
| OpenSSL EVP AES-256-GCM (D-08) | Hand-rolled AES / another crypto lib (cryptopp is vendored too) | EVP is verified available, already linked, FIPS-adjacent standard surface; cryptopp would add a second dependency for the same result. Rejected — D-08 locks vendored OpenSSL. |
| Legacy PKEY HKDF API | Modern `EVP_KDF_fetch("HKDF")` + OSSL_PARAM | Both verified present; PKEY API is simpler in C++17 (no OSSL_PARAM builder). Recommended: PKEY API. |
| Per-message room-key HKDF derivation | Cached per-room key map | Cache = shared mutable state needing its own mutex across 3 thread types; HKDF of a ≤128-byte topic is microseconds. Recommended: stateless per-call derivation. |

**Installation:** No new external packages are installed this phase. The proto is extended in-repo; the SuperGenius SDK and all Dart deps are already vendored/pinned from Phases 1-2; OpenSSL is already vendored + built (D-08 uses it in place).

**Version verification:** Locked Dart versions confirmed from `src/app/pubspec.lock` (flutter_bloc 9.1.1, protobuf 4.2.0, ffi 2.2.0, fixnum 1.1.1). SuperGenius/GeniusSDK versions are thirdparty release tarballs (not versionable here — see Environment Availability). **OpenSSL verified 3.3.3 this session from the resolved build tree** (`opensslv.h` + CMakeCache find_package details + `VERSION.dat` 3.3.3-dev source). Toolchain probed on this machine: cmake 3.29.2, ninja 1.13.2, Flutter 3.41.9, Dart 3.11.5, Apple clang 17.

## Package Legitimacy Audit

> This phase installs **zero** new external packages. No npm/PyPI/crates/pub.dev additions; therefore no slopcheck gate applies. The only dependency surface changes are (a) appending fields/messages to the in-repo `gcs_chat.proto`, (b) calling already-vendored SuperGenius GlobalDB APIs, and (c) calling already-vendored OpenSSL (D-08) — no new thirdparty package, no new build-system entry (the openssl ExternalProject already exists at `thirdparty/build/OSX/CMakeLists.txt:107`).

| Package | Registry | Notes | Disposition |
|---------|----------|-------|-------------|
| SuperGenius GlobalDB / GeniusSDK | thirdparty (prebuilt release, already a dependency) | Already linked by `gcs_storage`; Phase 3 only calls more of its API. | Approved (existing dep) |
| **OpenSSL** | thirdparty (vendored source + ExternalProject build, already a dependency) | Already linked transitively into gcs_ffi.dylib + all test executables; D-08 calls its EVP/HKDF/RAND API. Version 3.3.3 verified in-repo. | Approved (existing dep) |
| gcs_chat.proto | in-repo | Extended (sender/tombstones/MessageHistory), not installed; D-08 adds no field. | N/A (source) |
| flutter_bloc / protobuf / ffi / fixnum / frontend_scaffold | pub.dev / path (already locked) | Already in `pubspec.lock`. | Approved (existing deps) |

**Packages removed due to slopcheck [SLOP] verdict:** none
**Packages flagged as suspicious [SUS]:** none

## Architecture Patterns

### System Architecture Diagram

```
Dart (Flutter)                      C++ FFI (gcs_core_ffi.cpp)              Storage / Network
───────────────                     ──────────────────────────────          ────────────────────────
ComposerCubit ── publishCommand ──► gcs_publish (command topic)
   (send_text)                         │ parse GcsCommand
                                       ▼
                              ┌── send_text arm ───────────────────────┐
                              │ mint id (NextMessageId)                │
                              │ stamp sender=GetAddress(), role, ts    │
                              │ push pending ChatMessageState ──► Dart │  (plaintext to Dart only)
                              │ serialize record                        │
                              │ ENCRYPT(room_topic, record) ────────── ┼─┐  D-08 seam (injected)
                              │ publish envelope on room topic ────────┼─┼──► GossipSub ──► peers
                              │ Put(key, envelope, {room_topic}) ──────┼─┼──► GlobalDB (CRDT archive)
                              │ push complete ChatMessageState ──► Dart │ │
                              └────────────────────────────────────────┘ │
                                                                        ▼
                                                              nonce(12)‖ciphertext‖tag(16)

                              ┌── receive path (ApplyMessage) ─────────┐
GossipSub (peer) ────────────►│ DECRYPT(room_topic, envelope) ──────────┼── gcs::crypto adapter
GlobalDB RegisterNewElement   │   fail → skip-and-log (spdlog) ────────┐│   (fresh EVP ctx per call;
  callback (CRDT sync) ──────►│ parse ChatMessageState ◄───────────────┘│    3 calling thread types)
                              │ apply-once keyed by message id          │
                              │   (dedupe absorbs pub/sub + CRDT dup)   │
                              │ render → push ChatMessageState ──► Dart │  (decrypted plaintext)
                              │ Put(key, envelope, {})  // local, no rebroadcast
                              └─────────────────────────────────────────┘

                              ┌── join_topic arm ──────────────────────┐
join_topic / derived join ───►│ QueryKeyValues("gcs/messages/<room>/") │──► GlobalDB prefix scan
                              │ for each record: DECRYPT → parse        │
                              │   (skip-and-log on decrypt/parse fail)  │
                              │ sort (timestamp, id)                    │
                              │ push MessageHistory batch ──────► Dart │
                              └────────────────────────────────────────┘

Dart receive port ─► SessionCubit._dispatchEvent:
   message       → MessageFlowCubit.upsert (by id, pending→complete)
   messageHistory→ MessageFlowCubit.replaceAll
   (SpaceTree/RoomList/readiness/error unchanged)
```

### Recommended Project Structure (delta, not full tree)

```
src/lib/gcs_crypto.{hpp,cpp}                   # NEW (D-08): OpenSSL adapter — the ONLY file including <openssl/*>.
                                                #   DeriveRoomKey / EncryptRecord / DecryptRecord; stateless,
                                                #   fresh EVP_CIPHER_CTX + EVP_PKEY_CTX per call (threading constraint)
src/lib/gcs_messaging.{hpp,cpp}                # NEW (03-04): Messaging component — dedupe set, apply-once funnel,
                                                #   QueryHistory scan, injected EncryptFn/DecryptFn seam (never calls
                                                #   OpenSSL directly; seam empty = plaintext path, D-08)
src/lib/gcs_storage/gcs_global_db.{hpp,cpp}    # + topics-aware Put overload; + QueryKeyValues prefix scan
src/lib/gcs_core.{hpp,cpp}                     # + pass-through widening (Put(topics), QueryMessages)
src/CMakeLists.txt                             # + target_link_libraries(gcs_core PUBLIC OpenSSL::Crypto)  (one line, D-08)
src/ffi/gcs_core_ffi.cpp                       # send_text rewrite (encrypt→publish+Put+pending/complete); join_topic replay;
                                                #   receive bridge (decrypt inside Messaging funnel)
src/proto/gcs_chat.proto                       # + sender/deleted/deleted_at_ms on ChatMessageState; + MessageHistory in GcsEvent
                                                #   (D-08 adds NO field — envelope is opaque bytes)
src/app/lib/cubits/message_flow_cubit.dart     # + upsert (by-id); keep append for compat
src/app/lib/cubits/session_cubit.dart          # + messageHistory dispatch arm
src/app/templates/ + src/app/lib/generated/    # regenerate if sender/pending/error chrome changes (never hand-edit)
test/                                           # + test_gcs_messaging.cpp (send/receive/history/dedupe + crypto both-paths)
src/app/test/                                   # + cubit upsert + messageHistory dispatch tests
```

### Pattern 1: Topics-aware Put overload (D-02)
**What:** Add an overload that accepts a topic set; the existing 2-arg `Put` delegates with `kNoTopics`. This is the fix at the wrapper, not a new storage class.
**When to use:** Every archive-replicating message write (send path — value is the D-08 envelope). Receive path uses the 2-arg form (empty topics) as the "PutLocal" primitive (value is the already-encrypted envelope; receivers store ciphertext).

### Pattern 2: Apply-once dedupe keyed by message id (D-03)
**What:** A bounded set of seen message ids. Both the pub/sub fast path and the CRDT sync callback funnel into one `OnMessageArrived(id, envelope)` that returns early if the id is seen, else records it, decrypts (D-08 — inside the funnel, before parse), pushes to Dart, and `Put`s locally. The same message arrives twice by design (once via GossipSub, once via CRDT graphsync), so dedupe is load-bearing, not optional.
**When to use:** Every receive path. Retention is Claude's discretion (fixed-size LRU per room vs global keyed map).

### Pattern 3: Join-time batch replay + replaceAll (D-06)
**What:** Under the existing `g_mutex`, run the prefix scan, decrypt each record (skip-and-log on failure — same posture as unparseable), build a full sorted `MessageHistory`, push it as one `GcsEvent`; Dart does a full-list replace (mirrors `RailCubit.setRooms` and the generated `ChatMessageFlowCubit.replaceAll`). Live messages append after via the dedupe set.
**When to use:** `join_topic` arm AND `RefreshDerivedJoins` (derived joins from Phase 2). Push ordering matters: history batch first, then any live appends.

### Pattern 4: Append-only proto evolution (D-05)
**What:** `ChatMessageState` gains `sender` (next free tag) + `deleted`/`deleted_at_ms`; `GcsEvent` oneof gains `message_history = 6`. Never retype/reorder/remove. The generated Dart `gcs_chat.pb.dart` is regenerated (gitignored `generated/`), never hand-edited. **D-08 touches no proto field** — the encryption wraps the serialized record wholesale.

### Pattern 5: Injected crypto seam (D-08)
**What:** `Messaging` holds two optional callables (EncryptFn/DecryptFn over `roomTopic` + payload bytes). Present → every publish/Put path encrypts first and every receive/scan path decrypts first. Absent → bytes flow as-is (plaintext path). The OpenSSL implementation is a separate `gcs::crypto` adapter injected at composition time; `Messaging` never includes `<openssl/*>`.
**When to use:** Both paths must be constructible in tests (D-08 lock): inject the real adapter for round-trip/wrong-key/tamper tests; inject nothing for plaintext-path tests. The v1.1 per-room opt-out flag composes the same seam.
**Constraint:** decrypt may fire concurrently on the GossipSub strand thread, CRDT DagWorker threads, and the FFI command thread — the adapter must be stateless or internally locked (verified OpenSSL threading rules; see D-08 Crypto Surface §5).

### Pattern 6: Envelope layout (D-08 — pinned)
**What:** `value = nonce(12) || ciphertext || tag(16)`; nonce from `RAND_bytes` per message; tag appended after `EncryptFinal`. Decrypt: split by fixed offsets (12 from front, 16 from back), verify via `SET_TAG` before `DecryptFinal`; failure (≤ 0 return) = wrong key/tamper → skip-and-log.
**When to use:** Every encrypted write (send path) and every encrypted read (apply funnel, history scan). One layout everywhere — never a second framing.

### Anti-Patterns to Avoid
- **Per-room id-manifest for message history:** read-union-write LWW lost-update under concurrency — rejected in D-01; use the prefix scan.
- **CRDT write-through as the only delivery path:** render latency ties to graphsync convergence — rejected in D-03.
- **Echo loops:** re-broadcasting a received message; receivers must `Put` with empty topics.
- **Dart-side sender/authority:** Dart supplying `sender`, `id`, or `timestamp` — C++ stamps all authority fields (D-04/D-27).
- **Hand-editing generated Dart:** `lib/generated/chat/*` and scaffold families are generated output — regenerate, never edit.
- **`sleep_for` in tests:** use `WaitForCondition` (`test/test_wait_condition.hpp`).
- **Calling OpenSSL from `Messaging` directly (D-08):** breaks the seam; the plaintext path and the v1.1 opt-out both depend on injection.
- **Shared/member `EVP_CIPHER_CTX` (D-08):** mutated concurrently by three thread types — OpenSSL docs forbid; fresh ctx per call.
- **Linking system OpenSSL via pkg-config / `-lcrypto`:** CMakeCache pkg-config entries point at homebrew 3.6.3 (lines 552-555); use the `OpenSSL::Crypto` target, which resolves to the vendored static.
- **`c_str()`/`strlen()` on ciphertext:** envelopes and envelopes-in-`std::string` are binary — size-explicit handling only.

## Don't Hand-Roll

| Problem | Don't Build | Use Instead | Why |
|---------|-------------|-------------|-----|
| Message history enumeration | A per-room id manifest key | GlobalDB `QueryKeyValues` prefix scan | Manifest is a LWW lost-update hazard; the prefix scan already exists in SuperGenius (verified in NEO-SWARM planning + CONTEXT.md). |
| Live message delivery | A custom multicast/relay | GossipSub room topic (`AddBroadcastTopic`/`AddListenTopic` + publish) | Already wired in Phase 1; libp2p handles fan-out, dedupe-at-transport, and missed-delivery heuristics. |
| Archive replication | Hand-rolled merge protocol | GlobalDB `Put` with `{room_topic}` | CRDT per-key LWW already correct for immutable single-writer-per-key messages. |
| Message id uniqueness | Timestamp-only ids | `NextMessageId()` (seed + random + counter) | Bare counters collide across restarts (CR-01, documented in-repo). |
| Echo/duplicate suppression | Custom delivery tracking | Apply-once dedupe set keyed by message id | The pub/sub + CRDT double-delivery is by design (D-03); dedupe is the sanctioned convergence guard. |
| Pending/complete/error lifecycle | Auto-retry, queues, offline buffers | C++-pushed pending→complete/error + terminal error tint; manual re-send | Locked D-07 — no retry loop in MVP. |
| **AEAD encryption (D-08)** | Any hand-written AES-GCM, XOR stream, or "simple" cipher | Vendored OpenSSL `EVP_aes_256_gcm` (verified 3.3.3) | Authenticated encryption has catastrophic failure modes (nonce reuse, tag mishandling); the EVP surface is verified present and already linked. |
| **Key derivation (D-08)** | Own HKDF/HMAC-concat scheme | Vendored OpenSSL `EVP_PKEY_HKDF` (evp.h:76, kdf.h:105-117) | Verified present incl. canonical example in vendored docs; hand-rolled KDFs are a classic vulnerability class. |
| **Random nonces (D-08)** | `std::random_device`, `rand()`, timestamp-based nonces | OpenSSL `RAND_bytes` (rand.h:61; symbol verified in libcrypto.a) | CSPRNG with internal locking (thread-safe across the three calling threads); `random_device` quality is platform-dependent. |

**Key insight:** Every hard problem in this phase (convergent enumeration, live fan-out, replication, dedupe, **authenticated encryption, key derivation, CSPRNG**) already has a primitive in the vendored SuperGenius/libp2p/OpenSSL stack. The phase is a wiring + contract-extension exercise, not a new-distributed-systems or new-cryptography build.

## Common Pitfalls

### Pitfall 1: SuperGenius API signature drift (QueryKeyValues / RegisterNewElementCallback)
**What goes wrong:** Planning against a guessed callback typedef (`CRDTNewElementCallback` vs `GlobalDBNewElementCallback`) or a guessed `QueryKeyValues` return shape that does not match the prebuilt SDK.
**Why it happens:** The SuperGenius/GeniusSDK headers are prebuilt release tarballs pulled at CMake configure — not present in the local tree or git. The NEO-SWARM submodule's own plan explicitly warns "VERIFY the exact callback signatures in crdt_datastore.hpp … do not guess."
**How to avoid:** Add a Wave-0 verification task that locates `globaldb.hpp`/`crdt_datastore.hpp` in the resolved thirdparty build dir and confirms: `Put(HierarchicalKey, Buffer, topicSet)`, `QueryKeyValues(prefix)` return type, and the exact `RegisterNewElementCallback` typedef + bool return. Adapt signatures exactly.
**Warning signs:** Compile errors naming the callback typedef; a callback that never fires because the lambda signature mismatches the stored `std::function`.

### Pitfall 2: Echo loop (re-broadcasting received messages)
**What goes wrong:** A receiver re-publishes a received message, which re-arrives, gets re-published … unbounded amplification.
**Why it happens:** The send path publishes; if the receive path also publishes, every message fans out forever.
**How to avoid:** Receive path does `Put(key, envelope, /*empty topics*/)` only (the existing 2-arg `Put` with `kNoTopics` is exactly this primitive). Only the originating send path publishes + Puts-with-topics.
**Warning signs:** Duplicate messages multiplying in the archive; topic traffic growing without new sends.

### Pitfall 3: History/live ordering race (missing the join scan window)
**What goes wrong:** A message arrives via pub/sub between the prefix scan and the subscribe/apply-once activation, so it is neither in the batch nor deduped, and either duplicates or drops.
**Why it happens:** D-06 pushes history as a batch and then live messages append; the window between "scan" and "dedupe armed" is the race.
**How to avoid:** Run the scan under the session mutex and activate the apply-once set (or run the scan and register the callback atomically under `g_mutex`) so any pub/sub message that lands mid-join is either in the batch or absorbed by dedupe (D-03/D-06 design intent). Push the `MessageHistory` batch before any subsequent live message for that room.
**Warning signs:** Missing/duplicated messages right after a join; flaky history-equality tests at the join boundary.

### Pitfall 4: Ordering instability without a total sort
**What goes wrong:** Sorting by timestamp alone leaves ties unordered, so two peers disagree on message order (violates SC 3/4).
**Why it happens:** Wall-clock timestamps collide (same-ms sends).
**How to avoid:** Sort by `(timestamp, id)` — the id tiebreak makes the order total (D-01). Do not sort by id alone (ids are process-unique but not temporally ordered across peers).
**Warning signs:** Same-millisecond messages rendering in different orders across peers.

### Pitfall 5: Sender trust boundary (unsigned field)
**What goes wrong:** Treating `sender` as authenticated — any syncing node can forge a sender string on the wire.
**Why it happens:** D-04 locks unsigned MVP; signing is v1.1. Encryption (D-08) does NOT authenticate the sender — encryption ≠ signing.
**How to avoid:** Render `sender` as display-only short-form (truncated wallet address). Do not gate any authorization on it this phase. Document the spoofing surface in the code Doxygen (security note).
**Warning signs:** Building access-control or moderation logic on the sender field (that's Phase 4/5 territory and needs signed identity).

### Pitfall 6: Generated-Dart drift / scaffold read-only violations
**What goes wrong:** Hand-editing `lib/generated/chat/*` or scaffold `lib/components/*`, causing `dart analyze --fatal-infos` failures or silent overwrite.
**Why it happens:** The generated flow cubit (`chat_message_flow_cubit.dart`) and scaffold families are Jinja2 output; the scaffold is a read-only public contract.
**How to avoid:** App-side changes (by-id upsert, sender/pending/error rendering) live in `src/app/lib/cubits/` + app-owned templates under `src/app/templates/`; regenerate and never hand-edit generated output.
**Warning signs:** Edits inside `generated/`; `dart analyze` fatal-infos on a modified generated file.

### Pitfall 7: Nonce reuse under the same room key (D-08)
**What goes wrong:** Reusing a nonce with the same AES-GCM key is catastrophic — it leaks the plaintext XOR of both messages and can leak the authentication key outright.
**Why it happens:** Counter-based nonces after restart, deterministic nonces from timestamps, or a shared nonce buffer.
**How to avoid:** Fresh 12 random bytes from `RAND_bytes` per message, inline in the envelope (pinned layout §6). Collision math is negligible at per-room volumes (≈ 2⁻⁵⁷ at a million messages per room; NIST bound 2³² per key). One key per room means the budget is per-room.
**Warning signs:** Any "nonce" derived from the message id/timestamp; any cached/reused nonce buffer; two envelopes sharing a prefix under the same key.

### Pitfall 8: Shared EVP context across the three receive/send threads (D-08)
**What goes wrong:** A member/`thread_local`-cached `EVP_CIPHER_CTX` mutated concurrently by the GossipSub strand, CRDT DagWorkers, and the FFI thread → corruption or crashes.
**Why it happens:** EVP looks like a "handle you'd reuse"; OpenSSL's rule is objects are safe only when not mutated concurrently (openssl-threads.pod:50-70).
**How to avoid:** Fresh `EVP_CIPHER_CTX_new()`/`EVP_PKEY_CTX_new_id` per call, freed at scope exit; stateless adapter; `RAND_bytes` is internally thread-safe and fine.
**Warning signs:** Any `EVP_CIPHER_CTX` stored as a class member or `static`; intermittent decrypt failures under load; TSAN reports in the crypto adapter.

### Pitfall 9: GCM tag ctrl ordering errors (D-08)
**What goes wrong:** `GET_TAG` before `EncryptFinal`, or `SET_TAG` after `DecryptFinal`, or expecting `DecryptFinal` to emit plaintext — decryption always fails or tags never verify.
**Why it happens:** The ctrl ordering is a protocol, not a suggestion (vendored EVP_EncryptInit.pod:1372-1387).
**How to avoid:** Encrypt: Init → Update → Final (0 bytes out for GCM) → `GET_TAG`. Decrypt: Init → Update → `SET_TAG` → `Final` (≤ 0 = tag mismatch). Check every return (`1` = success).
**Warning signs:** All decryptions failing with "wrong key" even with the right key; unit tests passing with no tag verification.

### Pitfall 10: Treating the topic-derived key as confidential (D-08 scope)
**What goes wrong:** Assuming Phase 3 encryption gives E2E confidentiality against room members or topic observers.
**Why it happens:** The interim key is HKDF of the room topic string, which is public on the wire (GossipSub topic name, CRDT key prefix).
**How to avoid:** State the scope in the crypto adapter Doxygen and the code: Phase 3 D-08 encrypts at-rest/transport payloads (opaque local RocksDB, opaque pub/sub payloads to non-topic observers); member-robust confidentiality arrives with Phase 4 key distribution. Anyone who knows the topic string can derive the key — by design, interim.
**Warning signs:** Marketing the phase as "E2E encrypted"; gating anything security-sensitive on Phase 3 encryption.

### Pitfall 11: Linking system OpenSSL via pkg-config (D-08 build hazard)
**What goes wrong:** `pkg-config --libs openssl` or bare `-lssl -lcrypto` picks up homebrew 3.6.3 (CMakeCache `pkgcfg__OPENSSL_*` lines 552-555) — a system library, violating the thirdparty-only rule and drifting from the vendored 3.3.3 the rest of the stack links.
**Why it happens:** The cache carries both resolutions; pkg-config entries are just as visible as the find_package ones.
**How to avoid:** Only `target_link_libraries(... OpenSSL::Crypto)` (or `${OPENSSL_CRYPTO_LIBRARY}`); both verified to resolve to the vendored static. Never add openssl via pkg-config or manual link flags.
**Warning signs:** Link lines showing `/opt/homebrew/...`; version macros at runtime reporting 3.6.x.

## Code Examples

Patterns anchored in verified in-repo code (file:line references checked this session). SuperGenius API snippets are marked `[ASSUMED signature]` where the prebuilt header is not local. **OpenSSL snippets are verified against the resolved vendored headers and the vendored OpenSSL man pages (paths cited).**

### Topics-aware Put overload (D-02 — fix site is `gcs_global_db.cpp:256-271`)
```cpp
// Existing 2-arg form becomes a delegate (verified: current body hardcodes kNoTopics).
outcomes::result<void> GcsGlobalDb::Put(const std::string &key, const std::string &value) {
  return Put(key, value, /*topics=*/{}); // empty set = local-only (the "PutLocal" primitive)
}

// New topics-aware overload (D-02): passes the room topic through to the
// 3-arg GlobalDB::Put already in use (verified call at gcs_global_db.cpp:266).
outcome::result<void> GcsGlobalDb::Put(const std::string &key, const std::string &value,
                                       const std::unordered_set<std::string> &topics) {
  if (!m_running.load()) {
    return outcome::failure(Error::GcsDbError);
  }
  crdt::HierarchicalKey keyTyped{key};
  crdt::GlobalDB::Buffer valueTyped;
  valueTyped.put(value); // size-aware — binary-safe for D-08 envelopes
  auto result = m_db->Put(keyTyped, valueTyped, topics);
  if (result.has_error()) {
    return outcome::failure(Error::GcsDbError);
  }
  return outcome::success();
}
```

### Send path rewrite (D-03/D-07/D-08 — current arm at `gcs_core_ffi.cpp:667-720`)
```cpp
// After the existing validation + id mint + authority stamping:
// 1. push pending echo (state = MESSAGE_STATE_PENDING) to Dart.        (plaintext — Dart never sees ciphertext)
// 2. envelope = encrypt(room_topic, message->SerializeAsString());      (D-08 seam; skipped when seam absent)
// 3. publish envelope on sendText.room_topic() (live path).
// 4. Put envelope under "gcs/messages/<room_topic>/<id>" with {room_topic} (archive, D-02).
// 5. push complete echo (state = MESSAGE_STATE_COMPLETE, same id) to Dart.
// Failure on encrypt/publish/Put pushes an error-state ChatMessageState for that id (D-07).
// Key generalizes the Phase 1 precedent at gcs_core_ffi.cpp:709:
//   room_topic + "/" + message->id()  →  "gcs/messages/" + room_topic + "/" + message->id()
```

### AES-256-GCM encrypt (D-08 — verified against resolved evp.h + vendored EVP_EncryptInit.pod:1358-1395)
```cpp
// Source: thirdparty/build/OSX/Debug/openssl/build/include/openssl/evp.h (lines 382-384, 760-770, 882-887, 1061)
//         thirdparty/openssl/doc/man3/EVP_EncryptInit.pod (GCM and OCB Modes, lines 1358-1395)
// Layout: envelope = nonce(12) || ciphertext || tag(16). Fresh ctx per call (threading constraint).
constexpr size_t kGcmNonceLength = 12; // GCM AES default IV length (EVP_EncryptInit.pod:1366-1369)
constexpr size_t kGcmTagLength   = 16; // max/default tag length (1..16)

outcome::result<std::string> EncryptRecord(const std::vector<unsigned char> &key,   // 32 bytes
                                           const unsigned char *nonce,              // 12 bytes (RAND_bytes)
                                           const std::string &plaintext)
{
  std::string envelope;
  envelope.reserve(kGcmNonceLength + plaintext.size() + kGcmTagLength);
  envelope.append(reinterpret_cast<const char *>(nonce), kGcmNonceLength);

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();                    // evp.h:882
  if (ctx == nullptr) { return outcome::failure(Error::GcsDbError); }
  int outLen = 0;
  std::string ciphertext(plaintext.size(), '\0');
  bool ok = EVP_EncryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), nonce) == 1   // evp.h:760,1061
         && EVP_EncryptUpdate(ctx, reinterpret_cast<unsigned char *>(&ciphertext[0]),
                              &outLen,
                              reinterpret_cast<const unsigned char *>(plaintext.data()),
                              static_cast<int>(plaintext.size())) == 1                     // evp.h:768
         && EVP_EncryptFinal_ex(ctx, nullptr, &outLen) == 1;                               // 0 out bytes for GCM
  unsigned char tag[kGcmTagLength] = { 0 };
  if (ok) { ok = EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_GET_TAG, kGcmTagLength, tag) == 1; } // evp.h:383,887
  EVP_CIPHER_CTX_free(ctx);                                      // evp.h:884 — always
  if (!ok) { return outcome::failure(Error::GcsDbError); }
  envelope.append(ciphertext);
  envelope.append(reinterpret_cast<const char *>(tag), kGcmTagLength);
  return envelope;
}
```

### AES-256-GCM decrypt with tag verify (D-08)
```cpp
// Source: same headers; SET_TAG must precede DecryptFinal (EVP_EncryptInit.pod:1378-1387).
// Return: failure on tag mismatch == wrong key / tampered / corrupted → caller skip-and-logs (D-08).
outcome::result<std::string> DecryptRecord(const std::vector<unsigned char> &key,   // 32 bytes
                                           const std::string &envelope)
{
  if (envelope.size() < kGcmNonceLength + kGcmTagLength) { return outcome::failure(Error::GcsDbError); }
  const unsigned char *nonce = reinterpret_cast<const unsigned char *>(envelope.data());
  const size_t cipherLen = envelope.size() - kGcmNonceLength - kGcmTagLength;
  const unsigned char *ciphertext = nonce + kGcmNonceLength;
  const unsigned char *tag = ciphertext + cipherLen;

  EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
  if (ctx == nullptr) { return outcome::failure(Error::GcsDbError); }
  int outLen = 0;
  std::string plaintext(cipherLen, '\0');
  bool ok = EVP_DecryptInit_ex(ctx, EVP_aes_256_gcm(), nullptr, key.data(), nonce) == 1  // evp.h:777
         && EVP_DecryptUpdate(ctx, reinterpret_cast<unsigned char *>(&plaintext[0]),
                              &outLen, ciphertext, static_cast<int>(cipherLen)) == 1
         && EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_GCM_SET_TAG, kGcmTagLength,
                                const_cast<unsigned char *>(tag)) == 1;                  // evp.h:384
  int finalLen = 0;
  if (ok) { ok = EVP_DecryptFinal_ex(ctx, nullptr, &finalLen) == 1; }  // <=0 → tag mismatch (wrong key/tamper)
  EVP_CIPHER_CTX_free(ctx);
  if (!ok) { return outcome::failure(Error::GcsDbError); }
  return plaintext;
}
```

### HKDF-SHA256 room key derivation (D-08 — verified against resolved kdf.h/evp.h + vendored man page)
```cpp
// Source: thirdparty/build/OSX/Debug/openssl/build/include/openssl/kdf.h:105-117 (setters, not deprecated in 3.3),
//         evp.h:76 (EVP_PKEY_HKDF), 922 (EVP_sha256), 1785/1792/1927/1932 (ctx + derive),
//         thirdparty/openssl/doc/man3/EVP_PKEY_CTX_set_hkdf_md.pod:122-141 (canonical example).
constexpr const char kMessagesHkdfSalt[] = "gcs-messages-hkdf-v1"; // fixed, non-secret
constexpr const char kMessagesHkdfInfo[] = "gcs-messages-v1";
constexpr size_t kRoomKeyLengthBytes = 32;                         // AES-256

outcome::result<std::vector<unsigned char>> DeriveRoomKey(const std::string &roomTopic)
{
  EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr);          // evp.h:76,1785
  if (pctx == nullptr) { return outcome::failure(Error::GcsDbError); }
  std::vector<unsigned char> key(kRoomKeyLengthBytes, 0);
  size_t outLen = key.size();
  bool ok = EVP_PKEY_derive_init(pctx) == 1                                    // evp.h:1927
         && EVP_PKEY_CTX_set_hkdf_md(pctx, EVP_sha256()) == 1                  // kdf.h:105
         && EVP_PKEY_CTX_set1_hkdf_salt(pctx, kMessagesHkdfSalt,
                                         static_cast<int>(sizeof(kMessagesHkdfSalt) - 1)) == 1
         && EVP_PKEY_CTX_set1_hkdf_key(pctx,
                                         reinterpret_cast<const unsigned char *>(roomTopic.data()),
                                         static_cast<int>(roomTopic.size())) == 1
         && EVP_PKEY_CTX_add1_hkdf_info(pctx, kMessagesHkdfInfo,
                                         static_cast<int>(sizeof(kMessagesHkdfInfo) - 1)) == 1
         && EVP_PKEY_derive(pctx, key.data(), &outLen) == 1                    // evp.h:1932
         && outLen == kRoomKeyLengthBytes;
  EVP_PKEY_CTX_free(pctx);                                                     // evp.h:1792 — always
  if (!ok) { return outcome::failure(Error::GcsDbError); }
  return key;
}
```

### Nonce generation (D-08)
```cpp
// Source: thirdparty/build/OSX/Debug/openssl/build/include/openssl/rand.h:61
//         (_RAND_bytes verified as a defined symbol in libcrypto.a). Thread-safe.
unsigned char nonce[kGcmNonceLength] = { 0 };
if (RAND_bytes(nonce, static_cast<int>(kGcmNonceLength)) != 1) { /* skip-and-log path */ }
```

### Receive bridge (D-03) — `[ASSUMED signature]`
```cpp
// SuperGenius receive-side push hook (verified name in NEO-SWARM 03-02-PLAN.md:120;
// exact typedef to re-verify against crdt_datastore.hpp). Fires on CRDT DagWorker
// threads (verified: crdt_datastore.cpp:79-88, 2071-2074) — lock g_mutex before
// touching session state; decrypt happens inside the Messaging funnel (D-08).
//   m_db->RegisterNewElementCallback(prefix, [](key, value) { ... });
// Inside the callback: decrypt(value) → if fail skip-and-log; parse ChatMessageState;
//   if !seen(id) { seen.insert(id); PostToDart(message); g_session->Put(key, value, {}); }  // envelope stored as-received
```

### Append-only proto additions (D-04/D-05/D-06)
```proto
// ChatMessageState (gcs_chat.proto:67) gains (next free field tags):
//   string sender = 7;       // wallet address, stamped by C++ (D-04)
//   bool   deleted = 8;      // tombstone fields, present from creation (D-05)
//   int64  deleted_at_ms = 9;
// GcsEvent oneof (gcs_chat.proto:92) gains:
//   MessageHistory message_history = 6;  // batch history replay (D-06)
//   message MessageHistory { string room_topic = 1; repeated ChatMessageState message = 2; }
// D-08 adds NO proto field — the encrypted envelope is opaque bytes at the existing boundaries.
```

## State of the Art

| Old Approach | Current Approach | When Changed | Impact |
|--------------|------------------|--------------|--------|
| Phase 1: send = local Put (kNoTopics) + local echo; no pub/sub, no receive | GossipSub fast path + topics-aware Put archive + apply-once receive (D-03) | Phase 3 (this phase) | Messages now reach peers live and converge via CRDT. |
| Phase 1: no history read-back | `QueryKeyValues` converged prefix scan + `MessageHistory` batch (D-01/D-06) | Phase 3 | Join-time replay of full room history. |
| Phase 2: per-entity manifest as the only enumeration | Message history uses prefix scan (manifest pattern rejected for messages) | Phase 3 | No LWW lost-update hazard at message rates. |
| Append-only message records (id/role/state/text/timestamp) | Add `sender` + tombstone fields (D-04/D-05) | Phase 3 | Sender identity + forward-compatible moderation (Phase 5). |
| Plaintext storage/transport of message payloads | Full-record AES-256-GCM envelope on BOTH paths, interim HKDF(room_topic) key, injected crypto seam (D-08) | Phase 3 (2026-09-23 user choice; Matrix-shaped envelope) | Disk + wire carry ciphertext only; Dart + proto unchanged; Phase 4 swaps key distribution with no record-format change. |

**Deprecated/outdated:**
- The Phase 1 per-record key `room_topic + "/" + message_id` (`gcs_core_ffi.cpp:709`) is generalized to `gcs/messages/<room_topic>/<id>` for a stable scan prefix (D-01).
- The Phase 1 "local echo only" send path is replaced, not extended — the store-write semantics change from local-only to replicating.
- Phase 2's manifest enumeration remains valid for entities (spaces/rooms) — it is NOT deprecated; only its application to messages is rejected.
- D-04's original "signing with encryption in v1.1" framing is superseded: encryption moved into Phase 3 (D-08); signing remains v1.1.

## Assumptions Log

| # | Claim | Section | Risk if Wrong |
|---|-------|---------|---------------|
| A1 | `QueryKeyValues(prefix)` exact signature (return type `outcome::result<QueryResult>` per NNS 03-02-PLAN.md:117) and `RegisterNewElementCallback` typedef match the prebuilt SuperGenius SDK | Standard Stack / Code Examples | Compile-time mismatch; callback never fires. Mitigated by Wave-0 header-verification task. |
| A2 | The 3-arg `GlobalDB::Put(key, value, topicSet)` is the same call the topics-aware overload will use (already proven locally at `gcs_global_db.cpp:266`, so HIGH, not assumed) | Standard Stack | n/a — verified in-repo. |
| A3 | `GeniusNode::GetAddress()` is the correct API for the local wallet address (D-04) | Summary / Security | Wrong sender string. Verify against `GeniusSDK.hpp` at plan time (header not local). |
| A4 | Prefix `gcs/messages/` matches `QueryKeyValues` prefix-matching semantics (trailing-slash vs not) | Standard Stack | Wrong prefix returns empty/over-broad scans. NNS 03-02-PLAN.md:171 flags the same verification need. |
| A5 | `RegisterNewElementCallback` fires on CRDT DagWorker threads (verified this session in `../SuperGenius/src/crdt/impl/crdt_datastore.cpp:79-88, 2071-2074` — upgraded from assumed to verified) | Architecture / D-08 Threading | Was mutex discipline; now confirmed. GossipSub strand verified at `gossip_pubsub.hpp:305`. |
| A6 | Adding `target_link_libraries(gcs_core PUBLIC OpenSSL::Crypto)` to `src/CMakeLists.txt` configures and links cleanly with no build-submodule change. Mechanism verified (find_package at root CMakeLists.txt:31 precedes add_subdirectory(src) at :52; imported target resolves to the vendored static per CMakeCache:437,764; libcrypto.a already on all GCS final link lines) — but the edit itself was not configure-tested this session. | D-08 Crypto Surface §2 / Project Structure | LOW — worst case a configure-time target-not-found, caught immediately; fallback `${OPENSSL_CRYPTO_LIBRARY}` raw path is already in the cache. |
| A7 | Non-OSX platform builds of the vendored OpenSSL (iOS/Android/Linux/Windows) expose the same GCM/HKDF/RAND symbols. Build scripts verified to exist for all platforms (`thirdparty/build/{OSX,iOS,Android}/Openssl-build`, Linux/Windows build dirs present); symbols verified only in the OSX archive; default OpenSSL configure (no feature-disable flags in the scripts) includes all three on every platform. | D-08 Crypto Surface §1 | LOW — a platform-specific configure flag issue would surface at that platform's build, same as any thirdparty dep. No cross-compiled archives available locally to check. |

**Assumptions A1/A3/A4 all reduce to one environment fact:** the SuperGenius/GeniusSDK headers are prebuilt tarballs not present locally, so exact signatures must be re-verified against the resolved thirdparty build dir at plan/implement time. All are flagged MEDIUM confidence; none are presented as verified fact. **A5 was upgraded to verified this session.** A6/A7 are LOW-risk verification gaps around an otherwise fully verified mechanism. Every other D-08 crypto claim in this document is verified in-repo (headers, docs, archive symbols, build.ninja, CMakeCache) or explicitly cited (NIST bound).

## Open Questions

1. **SuperGenius header resolution at build time** — RESOLVED by plan 03-01 Task 1 (`03-API-SIGNATURES.md`): the Wave-0 task locates the resolved headers via `build/OSX/Debug/compile_commands.json` and records the exact `QueryKeyValues` / `RegisterNewElementCallback` / `GeniusSDKGetAddress` signatures before any implementation.
   - What we know: `gcs_global_db.cpp` includes `crdt/globaldb/globaldb.hpp`, `GeniusSDK.hpp`, `ipfs_pubsub/gossip_pubsub.hpp`; these resolve from the thirdparty prebuilt (not in local tree/git).
   - What's unclear: the exact resolved path after CMake configure on the dev machine and the exact API signatures.
   - Recommendation: Wave-0 task greps the resolved headers for `QueryKeyValues`, `RegisterNewElementCallback`, `GetAddress`, `Put(` and records exact signatures before any implementation.

2. **Receive-side callback threading model** — RESOLVED (verified this session): GossipSub subscription callbacks fire on the pubsub strand (`gossip_pubsub.hpp:305`); `RegisterNewElementCallback` fires on CRDT DagWorker threads (`crdt_datastore.cpp:79-88` `std::async` workers → `PutElementsCallback` at `:2071-2074`). Neither is the FFI command thread.
   - What we know: three distinct thread types reach the Messaging funnels; the single-funnel + `g_mutex` discipline (03-05) covers them.
   - Recommendation: single funnel function `OnMessageArrived` that takes `g_mutex`; both callback paths call it. The D-08 crypto adapter must be stateless per the verified OpenSSL threading rules (D-08 Crypto Surface §5).

3. **Dedupe-set scope and retention (Claude's discretion)** — RESOLVED by plan 03-04 Task 2: a global `std::unordered_set<std::string> m_seenIds` bounded by `constexpr size_t kMaxSeenIds = 4096;` with clear-on-full.
   - What we know: apply-once keyed by message id; fixed-size LRU per room vs global keyed map, retention unspecified.
   - What's unclear: the eviction bound and whether it is per-room or global.
   - Recommendation: a global `std::unordered_set<std::string>` keyed by id with a per-room clear on join is the simplest correct MVP; planner may pick an LRU cap constant (`constexpr`).

4. **Sender truncation format** — RESOLVED by plan 03-03 Task 1: app-side `_truncateSender` helper with `kSenderShortPrefixLength = 8` / `kSenderShortSuffixLength = 8` (`0x` + first/last 8 hex + `…`).
   - What we know: UI renders sender short-form (D-04); scaffold has no built-in address formatter.
   - What's unclear: exact truncation (e.g. `0x1234…abcd` — first/last N chars).
   - Recommendation: app-side helper with a `constexpr` char budget; render the raw string as-is if under budget.

5. **Vendored OpenSSL version + resolved includes (D-08)** — RESOLVED (verified this session): 3.3.3 at `thirdparty/build/OSX/Debug/openssl/build` (headers `include/openssl/opensslv.h`, statics `lib/libcrypto.a` + `lib/libssl.a`); the `-isystem` include dir appears on every GCS compile line in `compile_commands.json`. Source checkout is 3.3.3-dev per `VERSION.dat`.

6. **gcs_core OpenSSL linkability (D-08)** — RESOLVED (verified): compile — already works via root `CMakeLists.txt:32` `include_directories(${OPENSSL_INCLUDE_DIR})`; link — one line `target_link_libraries(gcs_core PUBLIC OpenSSL::Crypto)` in `src/CMakeLists.txt` (imported target exists before the subdirectory is added, resolves to the vendored static, and libcrypto.a already reaches every GCS final link line transitively). Do NOT use pkg-config (points at homebrew 3.6.3). Residual assumption A6: edit not configure-tested this session.

7. **EVP AES-256-GCM exact pattern (D-08)** — RESOLVED (verified): symbols at evp.h:760-789, 882-887, 1061; ctrl constants at evp.h:382-384; semantics from the vendored EVP_EncryptInit.pod (default IV 12, GET_TAG after Final, SET_TAG before DecryptFinal, taglen ≤ 16, ciphertext length == plaintext length). Full code pattern recorded in Code Examples.

8. **HKDF availability + pattern (D-08)** — RESOLVED (verified): `EVP_PKEY_HKDF` at evp.h:76 (NID_hkdf 1036), setters at kdf.h:105-117 (not deprecated in 3.3), derive trio at evp.h:1785/1927/1932, canonical example in the vendored EVP_PKEY_CTX_set_hkdf_md.pod:122-141. No HMAC fallback needed.

9. **EVP threading constraint (D-08)** — RESOLVED (verified): fresh `EVP_CIPHER_CTX`/`EVP_PKEY_CTX` per call; no shared mutable ctx across the GossipSub strand / CRDT DagWorker / FFI command threads (openssl-threads.pod:50-70); `RAND_bytes` thread-safe. Recorded as a hard constraint in Pattern 5 / Pitfall 8.

10. **Nonce strategy + envelope layout (D-08)** — RESOLVED (pinned): `RAND_bytes` 12-byte per-message nonce; `value = nonce(12) || ciphertext || tag(16)`; collision math negligible per-room (NIST 2³²-per-key bound cited). Recorded in Pattern 6.

11. **HKDF parameters (D-08)** — RESOLVED (pinned, interim): `HKDF-SHA256(ikm=room_topic, salt="gcs-messages-hkdf-v1", info="gcs-messages-v1", L=32)`, EXTRACT_AND_EXPAND default mode. Swapped for member-key distribution at Phase 4 with no envelope change (D-08).

12. **Decrypt-failure log level (warn vs debug)** — OPEN (Claude's discretion, non-blocking). Recommendation: `spdlog::warn` for a first failure per room scan and `spdlog::debug` for per-record detail, to avoid log flooding on a large pre-D-08 archive; planner picks one and pins it in 03-01/03-04.

13. **Crypto seam injection shape (std::function pair vs small interface)** — OPEN (discretion), with research recommendation recorded (two `std::function`s, optional, `src/lib/gcs_crypto.{hpp,cpp}` as the only OpenSSL-including file). Either shape satisfies D-08 as long as `Messaging` never calls OpenSSL directly.

## Environment Availability

| Dependency | Required By | Available | Version | Fallback |
|------------|------------|-----------|---------|----------|
| SuperGenius GlobalDB + GeniusSDK headers | QueryKeyValues / RegisterNewElementCallback / GetAddress verification, C++ build | ✗ (not in local tree — prebuilt, pulled at configure) | thirdparty release | Verify signatures from NEO-SWARM planning artifacts now; add Wave-0 header-verification task at implement time |
| **Vendored OpenSSL (headers + statics)** | D-08 crypto (EVP GCM, HKDF, RAND) | ✓ (verified: resolved build tree exists, include dir on all GCS compile lines, symbols in libcrypto.a, already on all GCS link lines) | 3.3.3 (vendored, static, `no-asm`) | none needed — no system OpenSSL usage permitted |
| cmake / ninja | C++ build | ✓ | 3.29.2 / 1.13.2 | — |
| C++ compiler | C++ build | ✓ | Apple clang 17 (g++/clang++) | — |
| protoc | proto regeneration | ✗ (system) | — | thirdparty protoc via `add_proto_library` (already wired — Phase 1 D-07); no system protoc needed |
| Flutter / Dart | app build + Dart tests | ✓ | 3.41.9 / 3.11.5 | — |
| GTest | C++ tests | ✓ (thirdparty, `find_package`/manual path in `test/CMakeLists.txt`) | — | — |
| GeniusSDK node (runtime) | end-to-end send/receive tests | ✓ (boots embedded via `gcs_init`; or injected pubsub seam in unit tests) | — | injected port-0 GossipPubSub fixture (Tier 2 seam, already used) |

**Missing dependencies with no fallback:**
- SuperGenius/GeniusSDK headers (local). This does not block planning (the API names are confirmed in-repo), but the exact signatures are unverifiable until the thirdparty build resolves them. The planner must include a Wave-0 signature-verification task.
- Nothing else. The D-08 crypto surface has NO missing dependency — OpenSSL headers, statics, docs, and link machinery are all verified present locally.

**Missing dependencies with fallback:**
- System `protoc` — not needed; the thirdparty protobuf toolchain is already wired.

## Validation Architecture

### Test Framework
| Property | Value |
|----------|-------|
| Framework | GTest (C++, `test/` tree) + flutter_test (Dart, `src/app/test/`) |
| Config file | `test/CMakeLists.txt` (gcs_test macro + ctest registration); Dart has none (flutter_test default) |
| Quick run command | C++: `cd build/OSX/Debug && ctest -R test_gcs_messaging --output-on-failure` |
| Full suite command | C++: `cd build/OSX/Debug && ctest --output-on-failure`; Dart: `cd src/app && flutter test` |

### Phase Requirements → Test Map
| Req ID | Behavior | Test Type | Automated Command | File Exists? |
|--------|----------|-----------|-------------------|-------------|
| CORE-04 SC1 | send_text pushes pending then complete with same id; local archive write | unit (FFI + injected pubsub) | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 SC2 | peer session receives message without refresh (live GossipSub fast path) | manual (human-verify, two live app instances) | — (03-06) | ✅ 03-06 |
| CORE-04 SC3 | two sessions' history converges (QueryKeyValues prefix scan equal after sync) | manual (human-verify, two live app instances) | — (03-06) | ✅ 03-06 |
| CORE-04 SC4 | history batch sorted `(timestamp, id)`; sender field stamped | unit | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 (D-07) | MessageFlowCubit upsert replaces pending→complete by id, no duplicate | unit (Dart) | `cd src/app && flutter test test/cubits/shell_cubits_test.dart` | ❌ extend existing |
| CORE-04 (D-06) | SessionCubit dispatches messageHistory → replaceAll | unit (Dart) | `cd src/app && flutter test test/cubits/shell_cubits_test.dart` | ❌ extend existing |
| CORE-04 (D-02) | topics-aware Put passes `{room_topic}`; 2-arg Put delegates empty | unit (C++) | `ctest -R test_gcs_storage` | ❌ extend existing |
| CORE-04 (D-08 round-trip) | encrypt→decrypt returns original record; envelope ≠ plaintext; envelope size = record + 28 | unit (C++, real `gcs::crypto` adapter injected) | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 (D-08 wrong-key skip) | record decryptable only under its room key; wrong-key/tampered envelope fails tag verify → skip-and-log, scan continues | unit (C++) | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 (D-08 plaintext path) | seam absent → publish/Put/scan flow plaintext unchanged (both paths testable, D-08) | unit (C++) | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 (D-08 wire/storage opacity) | bytes handed to Publish and Put are the envelope, never the serialized record | unit (C++, injected recording pubsub/storage seam) | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |
| CORE-04 (D-08 tamper) | flipping any envelope byte (nonce/ciphertext/tag) fails decryption | unit (C++) | `ctest -R test_gcs_messaging` (new) | ❌ Wave 0 |

### Sampling Rate
- **Per task commit:** `cd build/OSX/Debug && ninja gcs_core gcs_ffi && ctest -R test_gcs_messaging --output-on-failure` (C++), `cd src/app && flutter test test/cubits/` (Dart)
- **Per wave merge:** full `ctest` + `flutter test`
- **Phase gate:** full C++ + Dart suites green, plus `dart analyze --fatal-infos` clean, before `/gsd:verify-work`

### Wave 0 Gaps
- [ ] `test/test_gcs_messaging.cpp` — CORE-04 send/receive/history/dedupe + D-08 both-paths coverage (new test target `test_gcs_messaging`, links `gcs_core;gcs_storage;neoswarm_common`)
- [ ] `test_gcs_storage.cpp` — extend for topics-aware Put + QueryKeyValues prefix scan
- [ ] `src/app/test/cubits/shell_cubits_test.dart` — extend for upsert + messageHistory dispatch
- [ ] SuperGenius header signature verification task (Environment Availability A1/A3/A4)
- [ ] Optional: focused `test_gcs_crypto.cpp` for the adapter (round-trip / wrong-key / tamper / known-length checks) if the planner prefers crypto coverage isolated from the messaging funnel; otherwise fold into `test_gcs_messaging.cpp` (test target already links gcs_core, which links OpenSSL::Crypto)

*(Existing infra: `test_wait_condition.hpp` wait-condition template; Tier-2 injected pubsub fixture in `test_gcs_core_smoke.cpp`/`test_gcs_entities.cpp`; Dart cubit tests in `src/app/test/cubits/shell_cubits_test.dart`. Crypto tests need no network, no fixtures — pure unit.)*

## Security Domain

### Applicable ASVS Categories (ASVS Level 1)

| ASVS Category | Applies | Standard Control |
|---------------|---------|-----------------|
| V2 Authentication | No (deferred) | Sender identity = wallet address is display-only; signing deferred to v1.1 (D-04) |
| V3 Session Management | No | P2P — no server-side sessions; local FFI handle lifecycle already guarded |
| V4 Access Control | No (Phase 4) | No membership/roles this phase; room topic is the only gate |
| V5 Input Validation | **Yes** | FFI validates: room_topic non-empty + ≤`kMaxTopicLength`(128), text ≤`kMaxMessageTextLength`(4096), UTF-8 code-point caps on names; protobuf parse from untrusted bytes never reaches partial state; D-08 envelopes size-checked before split (≥ 28 bytes) |
| V6 Cryptography | **Yes (D-08)** | AES-256-GCM via vendored OpenSSL 3.3.3 EVP (never hand-rolled); HKDF-SHA256 key derivation via `EVP_PKEY_HKDF`; nonces from `RAND_bytes` (CSPRNG); 96-bit random nonce per message under a per-room key; tag verification failure = skip-and-log |

### Known Threat Patterns for this stack

| Pattern | STRIDE | Standard Mitigation |
|---------|--------|---------------------|
| Forged sender on the wire (unsigned `sender` field) | Spoofing | Accepted MVP limitation (D-04); render sender as display-only; do NOT authorize on it; document in Doxygen. v1.1 signing. Note: D-08 encryption does NOT sign — any member can still forge a sender string inside an encrypted record. |
| Nonce reuse under one room key (GCM catastrophic failure) | Tampering / Information Disclosure | Fresh `RAND_bytes` 96-bit nonce per message, inline in the envelope; per-room keys bound the exposure; collision probability ≈ 2⁻⁵⁷ at 10⁶ messages/room (NIST SP 800-38D caps random-IV GCM at 2³² per key) [CITED]. |
| Interim key derived from a PUBLIC input (room topic) | Information Disclosure | **Honest scope statement (D-08 design):** the Phase 3 key is HKDF(room_topic) and the topic string is visible on GossipSub and in CRDT key prefixes — anyone who knows the topic can decrypt. Phase 3 D-08 makes local storage and pub/sub payloads opaque (at-rest + casual-observer protection), NOT member-robust E2E; Phase 4 membership swaps the distribution with no envelope change. Documented in code Doxygen. |
| Tampered/spliced envelope (bit-flip, cut-and-paste between rooms) | Tampering | GCM tag verification fails on ANY modification (nonce, ciphertext, tag) → skip-and-log. Post-parse consistency check (room_topic/id match the record's storage key) is a cheap hardening the apply funnel can do after decrypt; formal AAD binding deferred with v1.1 hardening. |
| Malformed/corrupt CRDT records (hostile or partial writes) | Tampering | Per-entry skip-and-log during scan (EntityStore `LoadFromStore` precedent + NNS QueryKeyValues fault-tolerance pattern) — now including decrypt failure as a skip reason (D-08); one bad record never hides the rest or aborts. |
| Unbounded message growth / oversized publishes | DoS | `kMaxMessageTextLength` 4096, `kMaxTopicLength` 128, client-side `kMaxFlowItems` cap (drop oldest). |
| Echo loops / message amplification | DoS | Apply-once dedupe by id + receive path `Put` with empty topics (no re-broadcast, D-03). |
| System-library substitution (homebrew OpenSSL via pkg-config) | Tampering / supply chain | Only `OpenSSL::Crypto` target (verified vendored 3.3.3); pkg-config entries in the cache point at homebrew 3.6.3 and are forbidden (Pitfall 11). |
| Crypto API misuse (shared ctx, tag-order errors, unchecked returns) | Tampering / DoS | Verified-pattern code examples + constraints recorded (fresh ctx per call; GET_TAG after Final / SET_TAG before Final; every return checked); covered by the Wave-0 unit tests. |

## Sources

### Primary (HIGH confidence — verified in-repo this session)

**D-08 crypto surface (all new this session):**
- `thirdparty/build/OSX/Debug/openssl/build/include/openssl/opensslv.h` — OPENSSL_VERSION_STR "3.3.3" (the headers the build actually resolves).
- `thirdparty/build/OSX/Debug/openssl/build/include/openssl/evp.h` — EVP_aes_256_gcm (1061), Encrypt/Decrypt Init/Update/Final_ex (760-789), CTX new/reset/free/ctrl (882-887), EVP_CTRL_GCM_* (382-384), EVP_PKEY_HKDF (76), EVP_sha256 (922), PKEY ctx/derive (1785, 1792, 1927, 1932).
- `thirdparty/build/OSX/Debug/openssl/build/include/openssl/kdf.h` — legacy PKEY HKDF setters (105-117), HKDF modes (89-91), modern EVP_KDF API (24-49).
- `thirdparty/build/OSX/Debug/openssl/build/include/openssl/rand.h` — RAND_bytes (61).
- `thirdparty/build/OSX/Debug/openssl/build/include/openssl/obj_mac.h` — NID_hkdf 1036 (5463).
- `thirdparty/build/OSX/Debug/openssl/build/lib/libcrypto.a` — `nm` confirms defined symbols `_EVP_aes_256_gcm`, `_EVP_EncryptInit_ex`, `_EVP_DecryptFinal_ex`, `_EVP_CIPHER_CTX_ctrl`, `_EVP_PKEY_derive`, `_RAND_bytes`.
- `thirdparty/openssl/doc/man3/EVP_EncryptInit.pod` — GCM and OCB Modes (1358-1395): default IV 12, GET_TAG after Final, SET_TAG before Final, taglen 1-16; stream behavior (1351).
- `thirdparty/openssl/doc/man3/EVP_PKEY_CTX_set_hkdf_md.pod` — canonical HKDF derivation example (122-141).
- `thirdparty/openssl/doc/man7/openssl-threads.pod` — object thread-safety rules (50, 63, 67-70).
- `thirdparty/openssl/VERSION.dat` — vendored source = 3.3.3-dev.
- `thirdparty/build/OSX/CMakeLists.txt:103-120` — openssl ExternalProject; `thirdparty/build/OSX/Openssl-build/build.sh:39,47` — configure flags.
- `build/OSX/Debug/compile_commands.json` — resolved `-isystem .../openssl/build/include` on all GCS compile lines (34 commands).
- `build/OSX/Debug/CMakeCache.txt` — OPENSSL_* (437-452), FIND_PACKAGE_MESSAGE_DETAILS v3.3.3 (764), pkgconfig-homebrew hazard (552-555).
- `build/OSX/Debug/build.ninja` — vendored libcrypto.a/libssl.a on the gcs_ffi.dylib + 6 test link lines.
- `SuperGenius/build/OSX/Debug/SuperGenius/lib/cmake/SuperGenius/supergeniusTargets.cmake` — `$<LINK_ONLY:OpenSSL::Crypto>` transitive linkage.
- Root `CMakeLists.txt:3-5, 31-32, 52` — CMP0167 OLD, find_package(OpenSSL REQUIRED), global include dir, add_subdirectory ordering.
- `cmake/CommonBuildParameters.cmake:69` — OPENSSL_ROOT_DIR pinned to the thirdparty build.
- `thirdparty/build/OSX/Debug/ipfs-pubsub/include/ipfs_pubsub/gossip_pubsub.hpp:81-88, 305` — GossipSub strand.
- `../SuperGenius/src/crdt/impl/crdt_datastore.cpp:63-88, 2071-2074` + `../SuperGenius/src/crdt/globaldb/globaldb.cpp:707-710` — RegisterNewElementCallback path and DagWorker threads.
- `src/CMakeLists.txt:4-42`, `src/ffi/CMakeLists.txt`, `src/lib/gcs_storage/CMakeLists.txt` — target graph (gcs_core STATIC; gcs_storage → sgns:: targets).

**Existing messaging/storage surface (verified in prior session, re-confirmed present):**
- `src/lib/gcs_storage/gcs_global_db.cpp` — `Put` hardcodes `kNoTopics` at lines 264-266 (D-02 fix site); 3-arg `GlobalDB::Put(key, value, topicSet)` call proven local; `AddListenTopic`/`AddBroadcastTopic` wrap GlobalDB (lines 234-254).
- `src/ffi/gcs_core_ffi.cpp` — `send_text` arm (667-720, Phase 1 local-echo only), `NextMessageId()` (449-458), per-record key `room_topic + "/" + id` (709), `join_topic` arm (621-666), `RefreshDerivedJoins` (374-421), globals + caps (44-100), `PostToDart` (294-312).
- `src/lib/gcs_core.{hpp,cpp}` — `CoreSession` thin pass-through of Put/Get/AddListenTopic/AddBroadcastTopic (needs the same widening).
- `src/lib/gcs_entity_store.hpp` — Phase 2 manifest pattern (why it is NOT reused for messages).
- `src/proto/gcs_chat.proto` — current `ChatMessageState` (67-74), `GcsEvent` oneof (92-100), append-only Phase 2 additions.
- `src/app/lib/cubits/message_flow_cubit.dart` — append-only (47-49); `composer_cubit.dart` — converged submit path (unchanged); `session_cubit.dart` — `_dispatchEvent` (364-387, needs messageHistory arm); `rail_cubit.dart` — `setRooms` full-replacement precedent.
- `src/app/lib/generated/chat/chat_message_flow_cubit.dart` — `replaceAll` + `cappedItems` already exist (generated).
- `src/app/pubspec.yaml` + `pubspec.lock` — flutter_bloc 9.1.1, protobuf 4.2.0, ffi 2.2.0, fixnum 1.1.1, frontend_scaffold (path).
- `test/CMakeLists.txt` + `test/test_wait_condition.hpp` — test targets + wait-condition template.
- `src/app/scaffold/CLAUDE.md` — read-only lib/ contract.
- `.planning/workstreams/app/phases/03-messaging/03-CONTEXT.md` — locked decisions D-01..D-08 (the authoritative user decisions, incl. amended D-04 and new D-08).

### Secondary (MEDIUM confidence — cross-referenced in-repo, source not local)
- `GNUS-NEO-SWARM/.planning/workstreams/neoswarm/phases/03-gcs-globaldb-integration/03-02-PLAN.md` — documents `QueryKeyValues` signature (line 117), `RegisterNewElementCallback` (line 120), const-ness of Get/QueryKeyValues (line 174), prefix-matching caveat (line 171), and the "verify exact callback typedefs — do not guess" warning (line 236).
- `GNUS-NEO-SWARM/.planning/workstreams/neoswarm/phases/03-gcs-globaldb-integration/03-CONTEXT.md` + `ARCHIVED.md` — SuperGenius `globaldb.hpp` API surface (Put/Get/Remove/QueryKeyValues/BeginTransaction/topics/callbacks).
- `.planning/notes/gcs-chat-architecture.md` — Message Flow section, room topic model.
- NIST SP 800-38D — random-IV GCM invocation bound (2³² per key) and 96-bit IV guidance [CITED; external standard, not repo-verifiable].

### Tertiary (LOW confidence — not locally verifiable, flagged in Assumptions Log)
- Exact SuperGenius signatures (`QueryKeyValues` return type, `RegisterNewElementCallback`/`CRDTNewElementCallback` typedefs, `GeniusNode::GetAddress`, prefix-match semantics) — prebuilt SDK headers absent from local tree; all marked `[ASSUMED]` (A1/A3/A4).
- Non-OSX OpenSSL archive symbol parity (A7).

## Metadata

**Confidence breakdown:**
- Standard stack: **HIGH** — no new packages; all deps verified in-repo (pubspec.lock, CMakeLists, the 3-arg Put); OpenSSL vendored version/headers/symbols/linkage verified this session.
- Architecture: **HIGH** — decisions map directly onto verified in-repo code (send arm, Put site, manifest pattern, generated replaceAll, dispatch switch); D-08 seam fits the existing component-injection pattern (EntityStore precedent) with no architectural change.
- D-08 crypto surface: **HIGH** — every API claim verified against the resolved vendored headers, the vendored man pages, the archive symbol table, the ninja link lines, and the CMake cache; only the one-line CMake edit (A6) and non-OSX archives (A7) carry residual verification gaps, both LOW risk.
- Pitfalls: **MEDIUM-HIGH** — eleven pitfalls grounded in code, vendored docs, and NNS artifacts; the SuperGenius signature-drift pitfall depends on a non-local header (flagged).
- SuperGenius API signatures: **MEDIUM** — confirmed by name in two independent in-repo sources, but exact typedefs/signatures unverifiable until the prebuilt SDK resolves.
- Nonce collision bound: **MEDIUM** — arithmetic on verified parameters, bounded per NIST SP 800-38D [CITED].

**Research date:** 2026-09-23 (D-08 crypto surface update)
**Valid until:** 2026-10-07 (stable stack; the only fast-moving element is the thirdparty SuperGenius release, which is pinned per CI; the vendored OpenSSL 3.3.3 is a frozen source checkout)
