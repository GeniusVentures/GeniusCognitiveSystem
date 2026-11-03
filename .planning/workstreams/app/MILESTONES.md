# Milestones: App Workstream

## v1.0 GCS Chat (In Progress)

**Started:** 2026-08-15
**Status:** Planning

**Goal:** Build a working multi-party chat with spaces, rooms, membership, and GCS bot integration.

**Target features:**
- Space and room creation with configurable inheritance
- CRDT-based message sync
- Membership roles and moderation
- Invite system with capability tokens
- GCS bot participant with auto-answer policies

---

## v1.0.1 Coupling Remediation (Planned)

**Added:** 2026-11-03
**Status:** Planned
**Source:** [Whole-project coupling audit](../../notes/whole-project-coupling-audit.md) (develop `8526135`)

**Goal:** Demote the FFI adapter to translation. The domain rules it currently
owns move to `src/lib` owners, per the audit's target shape. Quick milestone:
scoped moves with tests green at each step, no behavior changes.

**Scope (from the audit):**
1. Room-session service in `src/lib` — owns the joined-topic registry (dedup,
   re-join, D-07 listen-then-broadcast ordering, derived/explicit partition,
   eviction), the composed D-02 predicate, the send/create/update permission
   gates, and the topic grammar + length policies currently in
   `gcs_core_ffi.cpp`
2. `Membership::Approve` reads entity state through `EntityStore` functions —
   removes the lib's only cross-domain raw-key access
3. Single-home the `"gcs/chat/"` prefix constant (currently 3 homes)
4. Dart capability/dormancy predicates — single home, or computed flags pushed
   from C++ (the `self_address` precedent)
5. Canonical-payload tripwire test; lib-only test target linking `gcs_core`
   without `gcs_ffi` (the audit's gate — unwritable today)

**Out of scope:** the D-02 creator-join product decision; the wire-level
dormancy divergence (separately deferred, STATE.md 2026-10-20); v1.1 encryption.

---

## Future Milestones

### v1.1 Private Spaces & Encryption
- App-layer symmetric key encryption
- Key rotation on membership change
- Encrypted DMs

### v1.2 Rich Content
- File attachments
- Message threading
- Reactions

### v2.0 Genius-Tube Integration
- Channels as branded spaces
- Stream/VOD rooms
- Subscriber/monetization layer
