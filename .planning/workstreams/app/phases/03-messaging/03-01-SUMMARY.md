---
phase: 03-messaging
plan: 01
subsystem: messaging
tags: [protobuf, crdt, globaldb, gossipsub, openssl, aes-256-gcm, hkdf, supergenius, geniussdk, c++17]

# Dependency graph
requires:
  - phase: 02-spaces-rooms
    provides: gcs_chat.proto wire contract (SpaceRecord/RoomRecord/SpaceTree, GcsEvent oneof arm 5), gcs_global_db.cpp topics-aware Put site
  - phase: 01-foundation
    provides: gcs_global_db.cpp Put/Get wrapper, gcs_chat.proto ChatMessageState/GcsEvent, GeniusSDK pubsub acquisition
provides:
  - "03-API-SIGNATURES.md: resolved SuperGenius/GeniusSDK/raw-GossipSub signatures + D-08 vendored OpenSSL 3.3.3 crypto surface (EVP AES-256-GCM + HKDF + RAND, OpenSSL::Crypto linkage, envelope, KDF params, threading constraint)"
  - "gcs_chat.proto append-only additions: ChatMessageState sender=7/deleted=8/deleted_at_ms=9 + GcsEvent message_history=6 + MessageHistory message"
affects: [03-02 (storage widening), 03-03 (Dart pb regen), 03-04 (Messaging + gcs_crypto), 03-05 (FFI wiring)]

# Tech tracking
tech-stack:
  added: [] # zero new packages (D-08 reuses vendored OpenSSL 3.3.3)
  patterns:
    - "Signature-pin contract file read first by all downstream implementation plans (no guessed signatures)"
    - "Append-only proto evolution (add fields/messages/oneof arms; never retype/reorder/renumber)"
    - "Binary-safe Buffer/string handling: Buffer::toString() is std::string_view (data+size); std::string{buf.toString()} for opaque envelope bytes"
    - "OpenSSL::Crypto imported target (vendored 3.3.3) is the only sanctioned linkage handle; pkg-config/homebrew forbidden"
    - "Fresh EVP_CIPHER_CTX/EVP_PKEY_CTX per call; no shared mutable crypto state across 3 calling thread types"

key-files:
  created:
    - .planning/workstreams/app/phases/03-messaging/03-API-SIGNATURES.md
  modified:
    - src/proto/gcs_chat.proto

key-decisions:
  - "Recorded Buffer::toString() as std::string_view (data+size, binary-safe) — 03-02 must use std::string{buf.toString()}, never C-string APIs on the D-08 envelope"
  - "Pinned the OpenSSL::Crypto imported target (resolves to vendored 3.3.3 libcrypto.a) as the only sanctioned link handle; pkgcfg_lib__OPENSSL_crypto/ssl + _OPENSSL_LDFLAGS point at homebrew 3.6.3 (forbidden)"
  - "Noted the HKDF PKEY setters sit under the OPENSSL_NO_DEPRECATED_3_0 guard (legacy 3.0 APIs, present in the default vendored build)"
  - "03-02 must retain m_pubsub before the std::move into crdt::GlobalDB::New — the shared GossipPubSub handle backs both CRDT broadcast and the raw live path"

patterns-established:
  - "Whitespace-normalized canonical signature strings recorded alongside resolved-header Allman-style rendering (grep anchors for downstream verify gates)"

requirements-completed: [CORE-04]  # Phase 3 requirement; delivered across plans 03-01..03-06

# Metrics
duration: 8min
completed: 2026-09-23
---

# Phase 3 Plan 1: API Signature Verification + Proto Contract Summary

**Pinned the resolved SuperGenius/GeniusSDK/raw-GossipSub signatures and the vendored OpenSSL 3.3.3 crypto surface (EVP AES-256-GCM + HKDF + RAND, OpenSSL::Crypto-only linkage), and extended gcs_chat.proto with sender + tombstone fields and a MessageHistory batch event**

## Performance

- **Duration:** 8 min
- **Started:** 2026-09-23T21:30:00Z
- **Completed:** 2026-09-23T21:38:14Z
- **Tasks:** 2
- **Files modified:** 2

## Accomplishments
- 03-API-SIGNATURES.md written with all eight SuperGenius/GeniusSDK/GossipSub signatures verbatim, resolved against the actual build-tree headers (never guessed): Put/PutLocal/QueryKeyValues/RegisterNewElementCallback/AddListenTopic/AddBroadcastTopic, GeniusSDKGetAddress, and raw GossipSub Publish/Subscribe with the full callback typedef chain
- D-08 crypto surface pinned from the resolved vendored OpenSSL 3.3.3 headers: EVP AES-256-GCM encrypt/decrypt call ordering, HKDF PKEY setters, RAND_bytes, the one-line `target_link_libraries(gcs_core PUBLIC OpenSSL::Crypto)` linkage change, the `nonce(12)||ciphertext||tag(16)` envelope, HKDF(room_topic) params, and the fresh-ctx-per-call threading constraint
- gcs_chat.proto extended append-only: ChatMessageState gains sender=7/deleted=8/deleted_at_ms=9 (D-04/D-05), GcsEvent oneof gains message_history=6, and a new MessageHistory batch message (D-06); all existing tags verified untouched by grep

## Task Commits

Each task was committed atomically:

1. **Task 1: Confirm SuperGenius/GeniusSDK signatures AND the D-08 crypto surface** - `100d492` (chore)
2. **Task 2: Append-only proto additions — sender, tombstone fields, MessageHistory** - `67804a8` (feat)

**Plan metadata:** final docs commit follows (SUMMARY.md + STATE.md + ROADMAP.md + REQUIREMENTS.md)

## Files Created/Modified
- `.planning/workstreams/app/phases/03-messaging/03-API-SIGNATURES.md` - Resolved signature + D-08 crypto contract read first by 03-02/03-04/03-05
- `src/proto/gcs_chat.proto` - Append-only: sender/tombstone fields on ChatMessageState, message_history arm on GcsEvent, MessageHistory message

## Decisions Made
- Confirmed every CRDT/GossipSub/GeniusSDK signature against the resolved thirdparty build headers (located via compile_commands.json) — zero semantic drift; only whitespace differences vs. the plan's canonical baseline
- Recorded three precision corrections in the signature record's drift log: Buffer::toString() returns std::string_view (not std::string), the pkg-config homebrew hazard uses `pkgcfg_lib__OPENSSL_crypto/ssl` + `_OPENSSL_LDFLAGS` variable names, and the HKDF setters are legacy-3.0-guarded (still present/usable)
- Confirmed the shared GossipPubSub handle is std::move'd into GlobalDB::New (gcs_global_db.cpp:154) — 03-02 must retain its own copy first

## Deviations from Plan

None - plan executed exactly as written. The resolved-header findings (Buffer::toString() return type, pkg-config variable names, HKDF legacy guard) are the plan's intended "record the ACTUAL resolved signature" output, documented in the signature record's Drift log (§7), not unplanned work.

## Issues Encountered

None. All symbols resolved on first read of the resolved headers; the only adjustment was adding whitespace-normalized canonical signature strings (§8 of the record) so the plan's grep-based verify anchors match the resolved headers' Allman-style rendering.

## User Setup Required

None - no external service configuration required.

## Next Phase Readiness
- 03-02 can proceed: QueryKeyValues/PutLocal/RegisterNewElementCallback signatures, the m_pubsub retention requirement, and the binary-safe accessor are all pinned in 03-API-SIGNATURES.md
- 03-04 can proceed: the D-08 crypto surface (EVP/HKDF/RAND patterns, OpenSSL::Crypto linkage, envelope layout, KDF params, threading constraint) is pinned for implementation
- No blockers

## Self-Check: PASSED

- FOUND: .planning/workstreams/app/phases/03-messaging/03-API-SIGNATURES.md
- FOUND: .planning/workstreams/app/phases/03-messaging/03-01-SUMMARY.md
- FOUND: src/proto/gcs_chat.proto
- FOUND: 100d492 (Task 1 commit)
- FOUND: 67804a8 (Task 2 commit)

---
*Phase: 03-messaging*
*Completed: 2026-09-23*
