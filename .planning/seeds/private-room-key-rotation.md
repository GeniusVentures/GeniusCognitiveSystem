---
title: Private room key rotation
date: 2026-08-15
trigger_condition: MVP chat sync proven working, private rooms requested
context: gs-explore --ws app
---

# Private room key rotation

After MVP proves basic CRDT sync, add app-layer encryption for private rooms.

## Scope

- Per-room symmetric key generation and distribution
- Key rotation when participants join/leave
- Encrypted payload format (nonce + ciphertext + tag)
- Key storage on device (secure enclave / keychain)

## Out of scope (future)

- Forward secrecy (requires async key agreement)
- Post-compromise security
