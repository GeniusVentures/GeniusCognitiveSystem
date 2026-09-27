# Phase 4: Membership & Invites - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-09-27
**Phase:** 4-Membership & Invites
**Areas discussed:** Member records & role state, Invite token design, Room-key distribution (WR-02), Approval flow & permission matrix, Invite lifecycle (follow-up)

---

## Member Records & Role State

| Option | Description | Selected |
|--------|-------------|----------|
| Per-member LWW records | One record per member per scope (`gcs/members/<scopeId>/<wallet>`); derived room access via `autoJoin && !explicitLeave`; flat `MemberList` event; C++ write guard for Super Admin protection | ✓ |
| Op-based membership log | Architecture note's Op sketch — per-key LWW silently drops concurrent ops to a shared log key; needs a merge reducer with no in-repo precedent | |

**User's choice:** Per-member LWW records (recommended)
**Notes:** Research verified the LWW no-union constraint directly in `crdt_datastore.cpp`; Phase 2 D-04 had already rejected per-member ops as an event storm. `SpaceRecord` creator-field gap surfaced here.

## Invite Token Design

| Option | Description | Selected |
|--------|-------------|----------|
| Bearer key in URL | `gcs://invite/{space\|room}/<id>?key=<key>`; doubles as the room-key delivery vehicle (WR-02); role + `membersCanInvite` enforced in the CRDT record at mint/redeem | ✓ |
| HMAC claim token | Structured payload + HMAC tag — forge-any-role on a serverless network (every member holds the shared key); needs a new HMAC surface; leaves WR-02 unsolved | |

**User's choice:** Bearer key in URL (recommended)
**Notes:** Signed claims impossible until v1.1 keypairs; the bearer key's double duty as the D-06 delivery vehicle was the deciding interlock.

## Room-Key Distribution (WR-02)

| Option | Description | Selected |
|--------|-------------|----------|
| Per-member AES-GCM key-wrap | Random 32-byte room key as HKDF ikm (salt/info stable, envelope unchanged); wrapped copy per member record; per-member key rides the invite; no new algorithms | ✓ |
| Megolm-style identity keys | ed25519/x25519 keypairs now + session keys encrypted to them — net-new asymmetric surface serving only deferred v1.1 signing / v2 rotation | |

**User's choice:** Per-member key-wrap (recommended)
**Notes:** Phase 4 is initial distribution only (no rotation). gcs_crypto's documented statelessness gets a mutex-guarded key-store exception.

## Approval Flow & Permission Matrix

| Option | Description | Selected |
|--------|-------------|----------|
| Embedded approved_by | Destructive op written dormant (empty `approved_by`); Super Admin overwrites same key to activate; write-time local checks; SpaceRecord gains creator field | ✓ |
| Separate pending-approval record | Pending record + flip command — cross-key join divergence risk under LWW; only pays off with multi-approver quorum | |

**User's choice:** Embedded approved_by (recommended)
**Notes:** Phase 4 destructive scope fixed to admin demotion, member removal, space/room deletion (kick/ban/message-delete are Phase 5). Single-signer approval eliminates the LWW union-loss hazard.

## Invite Lifecycle (follow-up)

| Option | Description | Selected |
|--------|-------------|----------|
| Multi-use, no expiry | Key valid until member removal or v2 rotation; no consume records; revocation = member tombstone | ✓ |
| Single-use + consume record | Nonce consume record per redeem; redeem-race handling under eventual consistency | |

**User's choice:** Multi-use, no expiry (recommended)
**Notes:** Simplest MVP surface; revisit if invite-leak abuse shows up.

---

## Claude's Discretion

- Proto naming/numbering (append-only), member-key prefix layout, wrapped-key encoding.
- Space-invite wrapping strategy (all rooms at redeem vs lazy on first join).
- Invite UI entry points + member-list surface (UI phase designs).
- Pending-approval event surfacing; permission-matrix placement in C++; redeem-command shape.

## Deferred Ideas

- Key rotation on member leave (ENCR-02, v2); sender signing + identity keypairs (v1.1); single-use/expiring invites; kick/ban/message-deletion (Phase 5); member-list UI polish (UI phase); per-room audit history; DM invites; lobby/discovery (Phase 7).
