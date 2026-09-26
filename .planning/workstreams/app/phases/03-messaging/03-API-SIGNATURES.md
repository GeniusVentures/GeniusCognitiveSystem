# Phase 3 — API Signatures (03-01)

**Recorded:** 2026-09-23 (plan 03-01 Task 1)
**Purpose:** Contract file read first by 03-02 (storage widening), 03-04 (Messaging + crypto), 03-05 (FFI wiring). Every signature below was verified against the **resolved** thirdparty/build-tree headers on this machine — never the system/homebrew copies, never guessed.

**Confidence:** HIGH — all symbols verified by reading the actual resolved headers and build metadata this session.

---

## Resolved include directories

Located by reading the compile command for `src/lib/gcs_storage/gcs_global_db.cpp` in
`build/OSX/Debug/compile_commands.json`. These are the exact `-isystem`/`-I` dirs the
compiler uses:

| Package | Resolved include dir (`-isystem`, verbatim) |
|---------|---------------------------------------------|
| SuperGenius (CRDT/GlobalDB) | `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/SuperGenius/build/OSX/Debug/SuperGenius/include` |
| GeniusSDK | `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/GeniusSDK/build/OSX/Debug/GeniusSDK/include` |
| ipfs-pubsub (GossipPubSub) | `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/thirdparty/build/OSX/Debug/ipfs-pubsub/include` |
| libp2p (gossip protocol) | `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/thirdparty/build/OSX/Debug/libp2p/include` |
| OpenSSL (vendored, D-08) | `/Users/Shared/SSDevelopment/Development/GeniusVentures/GeniusNetwork/thirdparty/build/OSX/Debug/openssl/build/include` |

These resolve from the SuperGenius/GeniusSDK prebuilt releases and the thirdparty
ExternalProject build tree — the sibling source trees (`../SuperGenius/src/...`,
`../thirdparty/ipfs-pubsub/src/...`) are NOT the compile-time source of truth.

---

## 1. SuperGenius GlobalDB CRDT surface

Source of truth (resolved): `<SuperGenius>/include/crdt/globaldb/globaldb.hpp`.
All signatures verified verbatim; line numbers are the resolved header's line numbers.

### Put (topics-aware) — globaldb.hpp:99

```cpp
outcome::result<CID> Put( const HierarchicalKey                 &key,
                          const Buffer                          &value,
                          const std::unordered_set<std::string> &topics );
```

The 3-arg form already used by the local wrapper at `gcs_global_db.cpp:266`
(currently called with an empty topic set).

Related, same header (for completeness; not required by this phase):
- `outcome::result<CID> PutConvergentImmutable(...)` — globaldb.hpp:104
- `outcome::result<CID> Put(const std::vector<DataPair> &, const std::unordered_set<std::string> &)` — globaldb.hpp:114
- `outcome::result<CID> Remove(const HierarchicalKey &, const std::unordered_set<std::string> &)` — globaldb.hpp:138

### PutLocal — globaldb.hpp:125

```cpp
outcome::result<void> PutLocal( const HierarchicalKey &key, const Buffer &value, const std::string &id );
```

Local-only write (bypasses DAG broadcast) — the receive-path "no re-broadcast" primitive
(Pitfall 2). Third arg `id` is the provenance/tie-break identifier for the local write.

### QueryKeyValues (prefix scan) — globaldb.hpp:144

```cpp
outcome::result<QueryResult> QueryKeyValues( std::string_view keyPrefix );
```

Plus an overload for wildcard/negated middle-part queries (globaldb.hpp:153) — not needed
this phase. The return type resolves as:

```cpp
// globaldb.hpp:36-37
using Buffer      = base::Buffer;
using QueryResult = CrdtDatastore::QueryResult;
// crdt_datastore.hpp:56
using QueryResult = RocksDB::QueryResult;
// storage/rocksdb/rocksdb.hpp:33
using QueryResult = std::map<Buffer, Buffer>;   // Buffer = base::Buffer
```

So `QueryKeyValues` returns `std::map<base::Buffer, base::Buffer>` (key → value, both
opaque byte buffers).

### Binary-safety of Buffer access (D-08 envelope check)

**Verified:** `base::Buffer::toString()` returns `std::string_view` (NOT `std::string`),
constructed size-explicit — it is binary-safe for the D-08 envelope's embedded NUL bytes.

```cpp
// base/buffer.hpp:245 — declared as
std::string_view toString() const;
// base/buffer.cpp:37-40 — implementation
std::string_view Buffer::toString() const
{
    return { reinterpret_cast<const char *>( data_.data() ), data_.size() };
}
```

`data_.size()` is used (not `strlen`), so embedded NUL bytes survive. The existing
`GcsGlobalDb::Get` wrapper already uses the binary-safe idiom:

```cpp
// gcs_global_db.cpp:282
return std::string{ buf.toString() };   // std::string(std::string_view) is data+size
```

03-02's `QueryKeyValues` wrapper must use the same size-explicit idiom
(`std::string{buf.toString()}` or `std::string(buf.data(), buf.size())`), never
`buf.toString().data()` followed by `strlen`/C-string APIs. The D-08 envelope
(`nonce || ciphertext || tag`) is arbitrary bytes and will contain NUL bytes.

### RegisterNewElementCallback — globaldb.hpp:212

```cpp
bool RegisterNewElementCallback( const std::string &pattern, GlobalDBNewElementCallback callback );
```

The callback typedef resolves as:

```cpp
// globaldb.hpp:74
using GlobalDBNewElementCallback = CrdtDatastore::CRDTNewElementCallback;
// crdt_datastore.hpp:62
using CRDTNewElementCallback = CRDTCallbackManager::NewDataCallback;
// crdt/crdt_callback_manager.hpp:27-28
using NewDataPair     = std::pair<std::string, base::Buffer>;
using NewDataCallback = std::function<void( NewDataPair new_data, std::string cid )>;
```

So the effective type is:

```cpp
std::function<void( std::pair<std::string, base::Buffer> new_data, std::string cid )>
```

First arg `new_data` = `(key, value)` pair; second arg `cid` = content identifier of the
write. `pattern` is a **regex** matched against the key string (see
`crdt_callback_manager.hpp:33-35` `std::regex`).

### AddListenTopic / AddBroadcastTopic — globaldb.hpp:168,170

```cpp
outcome::result<void> AddBroadcastTopic( const std::string &topicName );  // :168
void                  AddListenTopic( std::string topicName );            // :170  (void — no failure path)
```

---

## 2. GeniusSDK sender identity (D-04)

Source of truth (resolved): `<GeniusSDK>/include/GeniusSDK.h`.

```cpp
// GeniusSDK.h:383
GNUS_VISIBILITY_DEFAULT GeniusAddress GeniusSDKGetAddress();
```

```cpp
// GeniusSDK.h:57
#define GENIUS_SDK_ADDRESS_SIZE ( 2 + 128 )   // 2 for "0x" prefix + 128 hex characters

// GeniusSDK.h:62-64
typedef struct
{
    char address[GENIUS_SDK_ADDRESS_SIZE + 1];   // == char address[131]; "0x" + 128 hex + NUL
} GeniusAddress;
```

Confirmed: `GeniusAddress { char address[131]; }` (the `+1` is the NUL terminator). The
sender field for D-04 is `GeniusSDKGetAddress().address` (a `"0x"`-prefixed 128-hex
wallet string). In production the local node's address is obtained via
`GeniusSDKGetNode()->GetAddress()` (see §4); `GeniusSDKGetAddress()` is the SDK-level
C ABI form. Both return the same `GeniusAddress` layout.

---

## 3. Raw GossipSub live fast path (D-03)

Source of truth (resolved): `<ipfs-pubsub>/include/ipfs_pubsub/gossip_pubsub.hpp`
(class `sgns::ipfs_pubsub::GossipPubSub`).

```cpp
// gossip_pubsub.hpp:59
typedef libp2p::protocol::gossip::Gossip::SubscriptionCallback MessageCallback;

// gossip_pubsub.hpp:153-154
std::shared_future<std::shared_ptr<GossipPubSub::Subscription>> Subscribe( const std::string &topic,
                                                                           MessageCallback onMessageCallback );

// gossip_pubsub.hpp:165
libp2p::outcome::result<void> Publish( const std::string &topic, const std::vector<uint8_t> &message );
```

This is the **raw full-value** publish/subscribe surface (NOT the CRDT CID broadcast that
`GlobalDB::Put-with-topics` performs). Also present (not needed this phase): `PublishBuffered`
(:173) and `PublishBatch` (:179).

### The callback typedef chain

```cpp
// gossip_pubsub.hpp:59 — MessageCallback
using MessageCallback = libp2p::protocol::gossip::Gossip::SubscriptionCallback;

// libp2p/protocol/gossip/gossip.hpp:151-152
using SubscriptionData     = boost::optional<const Message &>;   // empty optional = end-of-stream
using SubscriptionCallback = std::function<void(SubscriptionData)>;

// gossip.hpp:131-135
struct Message {
  const ByteArray &from;
  const TopicId   &topic;
  const ByteArray &data;
};

// gossip.hpp:39, 96  +  libp2p/common/byteutil.hpp:14
using ByteArray = std::vector<uint8_t>;
using TopicId   = std::string;
```

So the effective callback type is:

```cpp
std::function<void( boost::optional<const Gossip::Message &> )>
// Message { const std::vector<uint8_t> &from; const std::string &topic;
//           const std::vector<uint8_t> &data; }
```

`Publish` takes the payload as `std::vector<uint8_t>` — binary-safe (the D-08 envelope is
arbitrary bytes). 03-05 converts the `std::string` envelope with explicit begin/end
iterators, never `c_str()`.

---

## 4. Shared `GossipPubSub` handle (03-02 must retain a copy)

Verified in `src/lib/gcs_storage/gcs_global_db.cpp`:

- Production `GcsGlobalDb::Initialize()` acquires the pubsub via
  `GeniusSDKGetNode()->GetPubSub()` (line 91) and passes
  `std::move(pubsub)` into the private `Initialize(...)` (line 113).
- The private `Initialize` then **`std::move`s it into `crdt::GlobalDB::New`** (line 154):

  ```cpp
  auto dbResult = crdt::GlobalDB::New(
      m_io, m_cfg.m_dbPath, std::move(pubsub), ... );
  ```

- `GcsGlobalDb` currently holds **no** `m_pubsub` member — after the move, the class no
  longer has a usable `GossipPubSub` handle.

**Action for 03-02:** add a `std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> m_pubsub;`
member and assign `m_pubsub = pubsub;` **before** the `std::move(pubsub)` into
`GlobalDB::New`. The same handle backs BOTH the CRDT broadcast (inside GlobalDB) and the
raw live `Publish`/`Subscribe` path — they share one `GossipPubSub` instance, which is the
designed topology (D-03).

---

## 5. D-08 crypto surface — vendored OpenSSL 3.3.3 (verified)

### 5.1 Version

`<openssl>/opensslv.h` (resolved): `OPENSSL_VERSION_STR "3.3.3"` (line 77);
`OPENSSL_VERSION_MAJOR 3` / `MINOR 3` / `PATCH 3` (lines 30-32).

### 5.2 EVP AES-256-GCM symbols (evp.h, resolved line numbers)

| Symbol | evp.h line |
|--------|-----------|
| `EVP_PKEY_HKDF` (= `NID_hkdf`) | 76 |
| `EVP_CTRL_GCM_SET_IVLEN` / `GET_TAG` / `SET_TAG` (aliases of `EVP_CTRL_AEAD_*`) | 382 / 383 / 384 |
| `EVP_EncryptInit_ex` | 760 |
| `EVP_EncryptUpdate` | 768 |
| `EVP_EncryptFinal_ex` | 770 |
| `EVP_DecryptInit_ex` | 777 |
| `EVP_DecryptUpdate` | 785 |
| `EVP_DecryptFinal_ex` | 789 |
| `EVP_CIPHER_CTX_new` / `_free` | 882 / 884 |
| `EVP_CIPHER_CTX_ctrl` | 887 |
| `EVP_sha256` | 922 |
| `EVP_aes_256_gcm` | 1061 |
| `EVP_PKEY_CTX_new_id` / `_free` | 1785 / 1792 |
| `EVP_PKEY_derive_init` | 1927 |
| `EVP_PKEY_derive` | 1932 |

Verified call patterns (pinned for 03-04; every call returns `1` on success and MUST be
checked):

- **encrypt** = `EVP_CIPHER_CTX_new` → `EVP_EncryptInit_ex(EVP_aes_256_gcm(), nullptr, key, nonce)`
  → `EVP_EncryptUpdate` → `EVP_EncryptFinal_ex` (emits 0 out bytes for GCM) →
  `EVP_CIPHER_CTX_ctrl(GET_TAG, 16, tag)` → `EVP_CIPHER_CTX_free`.
- **decrypt** = `EVP_CIPHER_CTX_new` → `EVP_DecryptInit_ex` → `EVP_DecryptUpdate` →
  `EVP_CIPHER_CTX_ctrl(SET_TAG, 16, expected)` **before** `EVP_DecryptFinal_ex` →
  `EVP_DecryptFinal_ex` (return ≤ 0 = wrong key / tampered / corrupted) → `EVP_CIPHER_CTX_free`.

### 5.3 HKDF symbols (kdf.h / evp.h)

`<openssl>/kdf.h` (resolved) exposes the legacy PKEY HKDF setters at lines 105-116:

```cpp
int EVP_PKEY_CTX_set_hkdf_md(EVP_PKEY_CTX *ctx, const EVP_MD *md);        // :105
int EVP_PKEY_CTX_set1_hkdf_salt(EVP_PKEY_CTX *ctx, const unsigned char *salt, int saltlen);  // :107
int EVP_PKEY_CTX_set1_hkdf_key(EVP_PKEY_CTX *ctx, const unsigned char *key, int keylen);      // :110
int EVP_PKEY_CTX_add1_hkdf_info(EVP_PKEY_CTX *ctx, const unsigned char *info, int infolen);   // :113
int EVP_PKEY_CTX_set_hkdf_mode(EVP_PKEY_CTX *ctx, int mode);               // :116
```

**Precision note (drift from RESEARCH wording):** these setters sit inside the
`#ifndef OPENSSL_NO_DEPRECATED_3_0` guard (kdf.h:15) — they are OpenSSL's **legacy 3.0
PKEY-HKDF APIs**, present and fully functional in the default vendored build (which
defines no feature-disable flags), but technically in the deprecated-3.0 legacy set rather
than "not deprecated." They are available and the recommended path (simpler than the modern
`EVP_KDF_fetch("HKDF")` + `OSSL_PARAM` API). Derivation pattern:

`EVP_PKEY_CTX_new_id(EVP_PKEY_HKDF, nullptr)` → `EVP_PKEY_derive_init` →
`set_hkdf_md(EVP_sha256())` → `set1_hkdf_salt(...)` → `set1_hkdf_key(ikm, len)` →
`add1_hkdf_info(...)` → `EVP_PKEY_derive(out, &outlen)` → `EVP_PKEY_CTX_free`.
Default mode is EXTRACT_AND_EXPAND — do **not** call `set_hkdf_mode`.

### 5.4 RAND

`<openssl>/rand.h:61` — `int RAND_bytes(unsigned char *buf, int num);` (thread-safe,
internally-locked DRBG). Use for per-message nonces; never `std::random_device`.

### 5.5 Linkage — the ONLY sanctioned handle (D-08)

- Root `CMakeLists.txt:31` — `find_package(OpenSSL REQUIRED)`; `:32` —
  `include_directories(${OPENSSL_INCLUDE_DIR})`; `:52` — `add_subdirectory(.../src src)`
  (so `src/CMakeLists.txt` runs after find_package).
- `build/OSX/Debug/CMakeCache.txt`:
  - `OPENSSL_CRYPTO_LIBRARY:FILEPATH=.../thirdparty/build/OSX/Debug/openssl/build/lib/libcrypto.a` (line 437)
  - `OPENSSL_ROOT_DIR:PATH=.../openssl/build` (line 449)
  - `OPENSSL_SSL_LIBRARY:FILEPATH=.../openssl/build/lib/libssl.a` (line 452)
  - `FIND_PACKAGE_MESSAGE_DETAILS_OpenSSL:INTERNAL=[...libcrypto.a][...include][c ][v3.3.3()]` (line 764)

  → `find_package` resolves to the **vendored 3.3.3 static**, and defines the imported
  target **`OpenSSL::Crypto`** (CMP0167 pinned OLD at root `CMakeLists.txt:3-4`).

- **The one-line change 03-04 makes** in `src/CMakeLists.txt` (currently no OpenSSL entry
  on the `gcs_core` target, which links `spdlog::spdlog`, `fmt::fmt`, `gcs_storage`,
  `gcs_proto`):

  ```cmake
  target_link_libraries(gcs_core PUBLIC OpenSSL::Crypto)
  ```

  Fallback if the imported target is unexpectedly not visible:
  `${OPENSSL_CRYPTO_LIBRARY}` (the same vendored static, already in the cache).

**HAZARD (FORBIDDEN):** the CMakeCache also carries pkg-config-derived entries pointing at
the **homebrew system OpenSSL 3.6.3**:

- `pkgcfg_lib__OPENSSL_crypto:FILEPATH=/opt/homebrew/Cellar/openssl@3/3.6.3/lib/libcrypto.dylib` (line 552)
- `pkgcfg_lib__OPENSSL_ssl:FILEPATH=/opt/homebrew/Cellar/openssl@3/3.6.3/lib/libssl.dylib` (line 555)
- `_OPENSSL_LDFLAGS:INTERNAL=-L/opt/homebrew/Cellar/openssl@3/3.6.3/lib;-lssl;-lcrypto` (line 899)
- `_OPENSSL_CFLAGS:INTERNAL=-I/opt/homebrew/Cellar/openssl@3/3.6.3/include` (line 893)

Never consume OpenSSL via pkg-config or bare `-lssl`/`-lcrypto` — those resolve to the
homebrew 3.6.3 **system** library, violating the no-system-libraries rule and drifting from
the vendored 3.3.3 the rest of the stack links. The `OpenSSL::Crypto` imported target is
the ONLY sanctioned handle (threat T-03-16).

### 5.6 Envelope layout, KDF parameters, threading constraint (PINNED)

- **Envelope layout** (one framing everywhere):
  ```
  stored/published value = nonce(12) || ciphertext (== plaintext length) || tag(16)
  ```
  Constants (no magic numbers):
  ```cpp
  constexpr size_t kGcmNonceLength     = 12;  // GCM default IV length
  constexpr size_t kGcmTagLength       = 16;  // default/max tag length
  constexpr size_t kRoomKeyLengthBytes = 32;  // AES-256 key
  ```
  Decrypt splits by fixed offsets: first `kGcmNonceLength` bytes = nonce, last
  `kGcmTagLength` bytes = tag, middle = ciphertext; minimum valid envelope = 28 bytes.

- **KDF parameters** (interim Phase 3 key; swapped for member-key distribution in Phase 4):
  ```
  room_key = HKDF-SHA256(
      ikm  = room_topic UTF-8 bytes,
      salt = "gcs-messages-hkdf-v1",     // fixed, non-secret
      info = "gcs-messages-v1",          // domain separator
      L    = 32 bytes )                  // AES-256
  ```
  Mode: default EXTRACT_AND_EXPAND — no `set_hkdf_mode` call.

- **Threading constraint (fresh ctx per call):** `EVP_CIPHER_CTX` and `EVP_PKEY_CTX` are
  mutable state machines and are unsafe under concurrent mutation. The crypto adapter must
  create a **fresh** `EVP_CIPHER_CTX`/`EVP_PKEY_CTX` per call and hold **no mutable shared
  state** (no member/`static`/`thread_local` ctx, no key cache without its own mutex).
  `ApplyMessage` decryption fires on BOTH the GossipSub strand thread AND the CRDT DagWorker
  threads, while `SendMessage`/`QueryHistory` encrypt/decrypt on the FFI command thread.
  `RAND_bytes` is internally thread-safe and may be called from any of them.

---

## 6. Receive-side threading note (03-05 mutex discipline)

Verified this session:

- **CRDT receive callback** (`RegisterNewElementCallback` → `RegisterNewDataCallback` →
  `CRDTCallbackManager::PutDataCallback`): fires on the **CRDT DagWorker threads**.
  Evidence (`../SuperGenius/src/crdt/impl/crdt_datastore.cpp`): workers created via
  `std::async(std::launch::async, ...)` at line 89; `PutDataCallback(key, value, cid)`
  invoked at line 2073 (through `PutElementsCallback`).

- **Raw GossipSub callback** (`Subscribe`'s `onMessageCallback`): fires on the
  **GossipPubSub strand thread**. Evidence (`<ipfs-pubsub>/gossip_pubsub.hpp`):
  `std::shared_ptr<boost::asio::io_context::strand> m_strand` (line 305); `Subscription`
  wraps `strand_` (lines 80-88).

Neither is the FFI command thread. **03-05 must take `g_mutex` in BOTH receive lambdas**
(live pub/sub decode + CRDT `RegisterNewElementCallback` bridge) before touching session
state or pushing to Dart.

---

## 7. Drift log (resolved reality vs. plan/RESEARCH baseline)

| # | Item | Baseline (plan/RESEARCH) | Resolved actual | Impact |
|---|------|--------------------------|-----------------|--------|
| 1 | `base::Buffer::toString()` | returns `std::string` built from data+size | returns `std::string_view` (buffer.hpp:245; impl buffer.cpp:37-40 uses `data_.data(), data_.size()`) | Binary-safe either way; use `std::string{buf.toString()}` (existing `Get` idiom, gcs_global_db.cpp:282) for a size-explicit `std::string`. |
| 2 | pkg-config OpenSSL cache var names | `pkgcfg__OPENSSL_ssl/crypto` (RESEARCH lines 552-555) | `pkgcfg_lib__OPENSSL_crypto` (552) / `pkgcfg_lib__OPENSSL_ssl` (555) + `_OPENSSL_LDFLAGS` (899) / `_OPENSSL_CFLAGS` (893) → homebrew 3.6.3 | Hazard is real (homebrew 3.6.3 visible via pkg-config path); recorded exact var names. Use `OpenSSL::Crypto` only. |
| 3 | HKDF setter deprecation status | "present and not marked deprecated in 3.3" | Present but inside `#ifndef OPENSSL_NO_DEPRECATED_3_0` (kdf.h:15) — legacy 3.0 APIs, available in the default vendored build | No code impact; APIs are available and recommended. Noted for precision. |

No CRDT/GossipSub/GeniusSDK signature drifted from the verified baseline.

---

## 8. Canonical signature strings (whitespace-normalized grep anchors)

The resolved headers render C++ with Allman-style spacing (`Publish( const std::string ... )`).
The following are the whitespace-normalized canonical forms — these are the exact strings
downstream plans (and the 03-01 verify gate) grep for. Semantics are identical; only spacing
differs from the header rendering.

```cpp
outcome::result<CID> Put(const HierarchicalKey &key, const Buffer &value,
                         const std::unordered_set<std::string> &topics);
outcome::result<void> PutLocal(const HierarchicalKey &key, const Buffer &value, const std::string &id);
outcome::result<QueryResult> QueryKeyValues(std::string_view keyPrefix);
bool RegisterNewElementCallback(const std::string &pattern, GlobalDBNewElementCallback callback);
void AddListenTopic(std::string topicName);
outcome::result<void> AddBroadcastTopic(const std::string &topicName);
GeniusAddress GeniusSDKGetAddress();
libp2p::outcome::result<void> Publish(const std::string &topic, const std::vector<uint8_t> &message);
std::shared_future<std::shared_ptr<GossipPubSub::Subscription>> Subscribe(const std::string &topic, MessageCallback onMessageCallback);
```
