---
phase: 02
slug: spaces-rooms
status: verified
threats_open: 0
asvs_level: 1
created: 2026-09-21
---

# Phase 02 — Security

> Per-phase security contract: threat register, accepted risks, and audit trail.

---

## Trust Boundaries

| Boundary | Description | Data Crossing |
|----------|-------------|---------------|
| CRDT store (RocksDB) → EntityStore | Persisted record bytes are re-read at startup; tampered DB bytes cross here | Serialized SpaceRecord/RoomRecord/manifest bytes (local, single-user at Phase 2) |
| Dart → `gcs_publish` C ABI | Untrusted command bytes cross the C ABI here | Serialized `GcsCommand` bytes + length |
| `gcs_core_ffi` → Dart push port | Error strings cross back as raw text (D-29) | Raw UTF-8 error strings (no PII in Phase 2) |
| C++ push port → `SessionCubit.handlePushedBytes` | Pushed event bytes are decoded here | Serialized `GcsEvent` bytes (trusted C++ output, still guarded) |
| Dialog form → GcsCommand publish | User-entered name/visibility/config leave the Dart UI here | Free-text name (≤64 chars client-side), is_public, autoJoinRooms |
| C++ ErrorNotice → showToast | Pushed raw error strings surface in a toast | Raw error text, display-only |
| Rail rows → `RailCubit.selectRoom` | Row taps drive selection; unjoined rows must not be selectable | Room topic id on tap |

---

## Threat Register

| Threat ID | Category | Component | Disposition | Mitigation | Status |
|-----------|----------|-----------|-------------|------------|--------|
| T-02-01 | Tampering / DoS | `EntityStore::LoadFromStore` record/manifest parse | mitigate | ParseFromString failure on any record ⇒ spdlog::warn + skip that id, never abort startup; manifest Get failure ⇒ empty catalog (`src/lib/gcs_entity_store.cpp:121-186`; learnings: unparseable manifest bytes = empty catalog, union write heals) | closed |
| T-02-02 | Tampering | `EntityStore::NextEntityId` | mitigate | seed+counter id minting (wall-clock ms + random_device + atomic seq, CR-01); entity tests assert the `<wallclock-ms>-<random-token>-<seq>` shape so a bare-counter revert fails (`src/lib/gcs_entity_store.cpp:110-119`) | closed |
| T-02-03 | DoS | Unbounded manifest id list | accept | AR-1 below | closed |
| T-02-04 | Tampering | `gcs_publish` GcsCommand parse | mitigate | ParseFromArray failure → PostErrorNotice + GCS_ERROR_INVALID_ARGUMENT before any state change; new arms inherit the existing pattern | closed |
| T-02-05 | Tampering | create_room / update_space arg validation | mitigate | Empty name, unknown parent_space_id, empty space_id all rejected with PostErrorNotice + GCS_ERROR_INVALID_ARGUMENT before any Put (`src/ffi/gcs_core_ffi.cpp:472-513`; VERIFICATION criterion evidence) | closed |
| T-02-06 | Tampering | Payload length size_t→int narrowing | mitigate | `payloadLength > std::numeric_limits<int>::max()` → GCS_ERROR_INVALID_ARGUMENT (inherited IN-01 guard, unchanged) | closed |
| T-02-07 | Information Disclosure | Error strings crossing FFI | accept | AR-2 below | closed |
| T-02-08 | Tampering | `handlePushedBytes` GcsEvent decode | mitigate | Existing try/catch emits a raw error string, never crashes (T-01-11-01 lineage); SpaceTree arm reached only after successful decode; test still green | closed |
| T-02-09 | Tampering | Dialog name input | mitigate | Client `maxLength: 64` + empty-name inline validation; C++ re-validates empty name / unknown parent as defense in depth (`src/app/lib/shell/space_room_dialog.dart`) | closed |
| T-02-10 | Information Disclosure / Access Control | `is_public` toggle | accept | AR-3 below | closed |
| T-02-11 | Tampering | Unjoined room row selection | mitigate | `ScaffoldPressable(disabled: true)` (in-atom ScaffoldDisabledOverlay 0.40) + `Semantics(enabled: false)` on catalog-but-not-joined rows, layered on the untouched `selectRoom` joined-only guard (`src/app/lib/shell/room_rail.dart`) | closed |
| T-02-SC | Tampering | Package installs | accept | AR-4 below (register entry repeated in all five plans; deduplicated here) | closed |

*Status: open · closed*
*Disposition: mitigate (implementation required) · accept (documented risk) · transfer (third-party)*

---

## Accepted Risks Log

| Risk ID | Threat Ref | Rationale | Accepted By | Date |
|---------|------------|-----------|-------------|------|
| AR-1 | T-02-03 | Local single-user store at Phase 2 scale (tens of entities); N+1 manifest reads are the documented accepted v1 tradeoff | Plan 02-01 (plan-time) | 2026-09-19 |
| AR-2 | T-02-07 | D-29 locked decision: raw error strings on the push port by design (no PII in Phase 2) | Plan 02-02 (plan-time) | 2026-09-19 |
| AR-3 | T-02-10 | `is_public` is display metadata ONLY in Phase 2 — no access enforcement until Phase 4/encryption; UI must not imply privacy enforcement | Plan 02-04 (plan-time) | 2026-09-19 |
| AR-4 | T-02-SC | Zero new packages this phase (02-RESEARCH Package Legitimacy Audit) — supply-chain gate not applicable | Plans 02-01..02-05 (plan-time) | 2026-09-19 |

*Accepted risks do not resurface in future audit runs.*

---

## Security Audit Trail

| Audit Date | Threats Total | Closed | Open | Run By |
|------------|---------------|--------|------|--------|
| 2026-09-21 | 12 | 12 | 0 | Claude (gsd-secure-phase, State B) |

Audit method: plan-time register short-circuit — all five PLANs authored parseable `<threat_model>` blocks (`register_authored_at_plan_time: true`); every mitigate-disposition threat was confirmed implemented in its plan SUMMARY Threat Flags section and independently re-verified by `02-VERIFICATION.md` (validate-first FFI arms, id-shape/tombstone-skip/decode-guard test evidence, dialog and rail widget tests, all re-run green 2026-09-19). threats_open: 0 → auditor spawn skipped per workflow short-circuit.

---

## Sign-Off

- [x] All threats have a disposition (mitigate / accept / transfer)
- [x] Accepted risks documented in Accepted Risks Log
- [x] `threats_open: 0` confirmed
- [x] `status: verified` set in frontmatter

**Approval:** verified 2026-09-21
