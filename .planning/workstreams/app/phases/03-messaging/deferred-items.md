# Phase 03 Deferred Items (out-of-scope discoveries)

## WR-02 — room encryption key is derived solely from the public room topic (deep re-review 2026-09-24)

- **Observed:** `DeriveRoomKey` uses `ikm = roomTopic` (`src/lib/gcs_crypto.cpp:46-49`)
  — a string that is simultaneously the gossipsub topic name and the CRDT key prefix, so
  any peer (or passive topic observer) can derive the identical AES-256-GCM key and
  decrypt all traffic and at-rest envelopes. "Encrypted" rooms provide obfuscation only,
  not membership confidentiality.
- **Scope ruling:** the finding's own fix routes it to Phase 4; deferred by owner in fix
  pass 2 (no code change this phase). The interim user-facing-copy half is vacuous today:
  no UI copy claims encryption, so there is no padlock to soften.
- **Action:** carry into Phase 4 (membership & invites) planning. Swap the HKDF input for
  per-room key material distributed via the membership layer — the envelope format
  already supports this without change. Keep `kMessagesHkdfSalt`/`kMessagesHkdfInfo`
  stable until then so Phase 4 can key-rotate without breaking the archive framing.
