# Phase 3: Messaging - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-09-23 (update session on the 2026-09-22 context)
**Phase:** 03-messaging
**Areas discussed:** encrypted chat-history storage (D-04 amendment + new D-08), crypto encapsulation seam

---

## Session Context

Original decisions D-01..D-07 were locked 2026-09-22 (logged in 03-CONTEXT.md).
This session re-opened the context on one topic: the user stated storing chat history
encrypted is necessary, backed by REQUIREMENTS.md ENCR-01 (app-layer symmetric key
encryption — classified v2, moved into Phase 3 by this session), while reaffirming the
D-03 pub/sub fast path. Six committed plans exist and will be replanned for D-08.

---

## Encryption Approach (shared room key derivation)

| Option | Description | Selected |
|--------|-------------|----------|
| Topic-derived key | HKDF over room topic name → AES-256-GCM; zero UX, no distribution; topic-name knowers can decrypt | |
| Follow Element/Matrix E2E model | Matrix/Megolm-shaped: full-record ciphertext, per-room group session, member-key distribution | ✓ (direction) |
| User passphrase | Creator-set passphrase entered on join; real shared secret, new UX + wrong-key handling | |
| Keep v2 deferral | No encryption this phase; ENCR-01 stays v2 per REQUIREMENTS.md | |

**User's choice:** "This should follow what Element Matrix and others do with end-to-end encryption if enabled for the topic/channel"
**Notes:** Structural constraint surfaced: Matrix session-key distribution requires a member roster; Phase 3 has none (membership & invites is Phase 4). Resolved in the phasing question below.

---

## Encryption Coverage

| Option | Description | Selected |
|--------|-------------|----------|
| Full record | Whole serialized ChatMessageState encrypted; disk/wire reveal nothing (stronger than Matrix, which leaves sender/timestamps visible to the server); decrypt-before-dedupe/sort | ✓ |
| Text field only | Only message text encrypted; metadata stays plaintext at rest | |

**User's choice:** Full record (Recommended)

---

## Live Path Treatment

| Option | Description | Selected |
|--------|-------------|----------|
| Encrypt both | One encrypt call before Publish and Put; ciphertext on every wire and disk | ✓ |
| Archive only | Live GossipSub keeps plaintext protobuf (libp2p transport already encrypts in transit); archive carries ciphertext | |

**User's choice:** Encrypt both (Recommended)

---

## Key Distribution Phasing

| Option | Description | Selected |
|--------|-------------|----------|
| Envelope now, key swap later | Durable E2E envelope in Phase 3 (full-record AES-256-GCM, per-room group session); interim key = HKDF(room_topic); Phase 4 swaps in Megolm-style member-key distribution with no record-format change; all rooms encrypted by default (Element's default) | ✓ |
| Full distribution in Phase 3 | Pull member discovery + 1:1 session-key distribution forward (effectively Phase 4 membership merged in) | |
| Room passphrase now | KDF(passphrase) shared secret without roster; not the Matrix model; adds join UX | |

**User's choice:** Envelope now, key swap later (Recommended)

---

## Crypto Encapsulation (injection seam)

| Option | Description | Selected |
|--------|-------------|----------|
| Injected crypto seam | Messaging takes injected encrypt/decrypt functions; they are simply not called when encryption is disabled — plaintext flows; both paths testable in this phase | ✓ |
| Inline OpenSSL calls | Messaging calls EVP directly behind an enabled flag; no injection seam | |

**User's choice:** "keep the messaging encapsulated, so that if the room/messaging doesn't have encryption enabled, it just doesn't call the injected crypto functions. That way we can test both paths in this phase"
**Notes:** Default remains encryption-enabled for all Phase 3 rooms; the disabled path is exercised by tests now and becomes the v1.1 per-room opt-out with no architectural change. Messaging never calls OpenSSL directly.

---

## Claude's Discretion

- Exact HKDF parameters, nonce size, OpenSSL EVP call pattern (pinned in 03-01 signature record).
- Injection shape of the crypto seam (pair of `std::function` encrypt/decrypt vs a small interface), as long as Messaging never calls OpenSSL directly and the seam is injectable per room/messaging instance.
- Decryption-failure log level (warn vs debug).
- Crypto helper placement inside the Messaging component.

## Deferred Ideas

- Cryptographic signing + sender verification — v1.1 (D-04 unsigned stays).
- Megolm-style member-key distribution + key rotation on leave (ENCR-02) — Phase 4.
- Per-room encryption opt-in/opt-out flag + room settings UI — v1.1+.
- DM auto-encryption (ENCR-03) — DMs don't exist yet.
- Element-style device verification — v1.1+.

---

*Phase: 03-messaging*
*Discussion log generated: 2026-09-23*
