---
title: Whole-project coupling audit — where GCS domain rules actually live
date: 2026-11-03
context: Paused phase-04 human UAT at the user's request, mirroring ../apps/genius-ai-boss/.planning/notes/whole-project-coupling-audit.md. Evidence from three parallel full-source sweeps (src/lib, src/ffi/gcs_core_ffi.cpp, src/app/lib) plus direct reads of src/proto, src/CMakeLists.txt, and src/lib/gcs_storage; load-bearing claims spot-checked first-hand. Every row carries file:line; nothing inferred.
---

# Whole-Project Coupling Audit (2026-11-03)

The contract, in the project's own words: `src/lib/` is "a **core library** that can
be embedded or extended" — genius-tube consumes it as a submodule/linked lib and
"extends but does not modify core primitives" (`.planning/notes/gcs-chat-architecture.md:125-141`).
CLAUDE.md hard invariants: one owning module per domain rule (10), never duplicate
logic (11), no domain logic in adapters or presentation (12); read another module's
storage through its functions, never its keys.

Verdict: **the leak is inverted relative to genius-ai-boss.** There, plugins coupled
through raw storage keys, bidirectionally. Here, storage discipline is nearly clean —
one breach, read-only, one call site. The coupling runs **upward**: the FFI adapter
has become a fifth domain module in disguise. `gcs_core_ffi.cpp` is 1990 lines —
larger than crypto + entity-store + messaging *combined* (556 + 459 + 402) — because
the one rule set with no lib owner (joined-room state) gravitated to the only layer
that could hold it. A thinner second leak runs into Dart: capability and dormancy
rules are re-derived there, in two independent homes, under self-declared carve-outs.

## Against the developer's own tests

1. **Core library embeddable/extendable by genius-tube** — fails for every rule
   that lives above `src/lib`: the joined-room registry (join dedup, re-join
   semantics, listen-then-broadcast ordering, derived/explicit partition —
   `gcs_core_ffi.cpp:841-878, 1359-1412`), the D-02 composed predicate
   (`:801-830`), the role gates on send/create/update arms (`:1459-1480,
   1586-1594, 1668-1675`), and the name/topic/message-length policies
   (`:59-82`). A consumer linking `gcs_core` alone gets a Membership module
   that cannot answer "which rooms is this session in?"
2. **C++ stamps authority, Dart renders** (D-26/D-27) — holds on the data plane:
   commands are data-only (`gcs_chat.proto:43, 162-163, 312-313`), no local
   optimism anywhere in the app (`space_room_dialog.dart:8-14, 303-305`,
   `join_dialog.dart:8-10, 148-150`), `self_address` is pushed so Dart never
   guesses identity (`gcs_chat.proto:270-275`). Leaks on the *decision* plane:
   see Dart table below.
3. **One owner per rule, no duplication** — fails: D-02 in 3 homes, `"gcs/chat/"`
   prefix in 3 homes, dormancy in 4 homes (2 C++, 2 Dart), name-length policy in
   2 homes with the owning module absent entirely, canonical payloads mirrored
   in test helpers with no tripwire.

## Library layer (src/lib) verdicts

| # | Check | Verdict | Evidence |
|---|-------|---------|----------|
| 1 | Cross-module storage access | **One breach** | `Membership::Approve` reads `gcs/entities/{spaces,rooms}/<id>` raw via `m_session.Get` — `gcs_membership.cpp:2176-2178` — using locally redefined entity prefixes (`:43-45`; canonical at `gcs_entity_store.cpp:30-32`), then parses `SpaceRecord`/`RoomRecord` itself (`:2186-2225`). Contrast: `DeleteScope` routes the same domain's writes through `m_entities.ApplySpaceState/ApplyRoomState` (`:2025, 2050`) — the sanctioned path, one arm away. |
| 2 | Module × key-prefix discipline | **Clean elsewhere** | Matrix from every raw session call: EntityStore touches only `gcs/entities/*` + `gcs/index/manifest` (`gcs_entity_store.cpp:74-378`); Messaging only `gcs/messages/*` (`gcs_messaging.cpp:140, 232, 311-312`); crypto zero storage (`gcs_crypto.cpp` memory-only). `GcsGlobalDb` itself knows no `gcs/` prefix at all. |
| 3 | Ownership/lifetime/composition | **Clean** | No inheritance, no `friend` in src/lib; all modules standalone classes composed via borrowed refs, copy/move deleted (`gcs_core.hpp:62-65`, `gcs_membership.hpp:176-177`); all construction happens once at the FFI boundary. Seams are injected `std::function` structs, not class hierarchies (`gcs_membership.hpp:115-130`). |
| 4 | D-02 derived-join rule | **Split 3 ways** | `EntityStore::DerivedJoinedTopics` owns the autoJoinRooms half (`gcs_entity_store.cpp:419-450`); `Membership::HasExplicitLeave` owns the explicit-leave half (`gcs_membership.cpp:1113-1121`); the **composition exists only in the adapter** (`gcs_core_ffi.cpp:801-830`). The full rule is assembled nowhere in the lib. |
| 5 | D-07 dormancy + role matrix | **Single owner, internal mirror** | All in Membership: `AdminCount` (`:866-919`), `PrivilegeRank` single definition (`:96-113`), write guards (`:1128-1258`). The dormancy predicate is written twice inside the module — write path `:1892-1897` vs verifier `:727-767` — self-acknowledged as a deliberate mirror (`:1883-1886`); the wire-level divergence this mirror already produced is the deferred decision recorded in `STATE.md` (2026-10-20). |
| 6 | Canonical payloads vs test helpers | **Mirror without tripwire** | Prod builders are anon-namespace (`MemberIssuerPayload` `:307-318`, `InviteIssuerPayload` `:271-278`); `test/test_gcs_member_helpers.hpp:25, 43-54, 66-69` and `test/test_gcs_invite_helpers.hpp:22, 35-42` duplicate them verbatim, self-described as "the ONE test-side implementation — invariant 11" — but no test compares prod vs helper output, so drift is undetectable. |
| 7 | Room-key store scope | **Process-global** | The per-room key store is a hidden function-local static shared process-wide (`gcs_crypto.cpp:134-153`) — not instance-scoped like everything else; and Membership reaches it by direct static calls (`:1289, 1322, 1329, 1751`…) while wrap/unwrap/sign/verify flow through the injected `KeyWrapSeam` — two access paths for one dependency. |
| 8 | Constant duplication | **`"gcs/chat/"` × 3** | `gcs_entity_store.cpp:40` (canonical), `gcs_membership.cpp:47`, `gcs_core_ffi.cpp:76`. |

## FFI adapter (src/ffi/gcs_core_ffi.cpp) verdicts

| # | Check | Verdict | Evidence |
|---|-------|---------|----------|
| 1 | Raw storage bypass | **Clean** | Zero `QueryKeyValues`/`Put`/`Get` calls in 1990 lines; CoreSession touched only through `Initialize`/`AddListenTopic`/`AddBroadcastTopic`/`Subscribe`/`Shutdown`. The bypass is API-surface, not storage (row 2). |
| 2 | Joined-room registry | **A domain module in exile** | Join dedup `:1376-1388`; re-join = refresh-only `:1398-1409`; D-07 listen-then-broadcast ordering `:841-842, 982-983, 1015-1016, 1359-1367`; replay-then-subscribe "Pitfall 3" `:1410-1412`; derived/explicit/smoke partition + eviction (WR-03) `:849-878`; "send requires joined" `:1443-1448` — all in adapter globals because CoreSession offers only raw `AddListenTopic/AddBroadcastTopic` (`gcs_core.hpp:114-122`) and no lib module owns joined-room state. |
| 3 | Permission gates | **Split by arm** | Membership commands (invite/redeem/change_role/remove/leave/delete/approve) delegate with zero adapter re-checks — exemplar (`:1711-1897`, guards at `gcs_membership.cpp:462-506` via `CanInvite`…`CanApprove`). But send_text/create_room/update_space apply the adapter's own `RoleAtLeast` rank predicate (`:773-776`), `ScopeHasRoster` (`:752-756`), and a hand-rolled scope-resolution loop (`:1463-1471`) — the D-08 matrix re-expressed outside its owner. |
| 4 | Boundary policies | **Duplicated, owner absent** | Room-topic grammar `IsValidRoomTopicShape` + `kMaxTopicLength=128` (`:71-76, 184-190`); `kMaxEntityNameLength=64` (`:59-64`); `kMaxMessageTextLength=4096` (`:77-82`); topic↔id string derivation (`:811, 1460-1461, 1617, 1630`). None of these live in EntityStore/Messaging — the adapter is the *only* C++ home, and Dart re-encodes two of them (Dart table #4). |
| 5 | Room-key minting policy | **Adapter decides** | Nested room: adapter mints + stores the key at creation; standalone: minted inside `Membership::OnScopeCreated` — the branch and the `"gcs/chat/" + roomId` construction are adapter-side (`:1604-1635`). |
| 6 | Issuer sign/verify seams | **Seam bodies hold rules** | The FFI lambdas implement issuer==local check, 0x-hex stripping, `kSignatureBytes` length gate (`:1226-1274`) — a policy implementation, not wiring; mirrors rules documented at `gcs_membership.hpp/82, 155`. |
| 7 | Smoke topics | **Product policy in the ABI** | `kSmokeTopicA/B` hardcoded (`:57-58`), pre-joined at init so "the pushed RoomList is non-empty" (`:56, 976-978`), init fails if neither joins (`:997-1002`), and the behavior is documented into the public C header (`gcs_core.h:74-75, 98`). |
| 8 | Globals in a dylib | **18 file-scope mutables** | `g_session`…`g_tearingDown` (`:103-145`), receive lambdas capture them directly (`:702-724, 1071-1137`). Mitigations are real: every mutation site enumerated, teardown order explicit with reasons (`:381-411`), in-flight drain (`:336-337, 1975`). Self-documented "Phase 1: single global session" (`:104`). |
| 9 | Error mapping | **Lossy** | All membership failures → one code + invented strings ("permission denied" for a store failure, `:1784-1788, 1810-1813, 1862-1865`); the real cause is classified nowhere. |

**Underline:** the adapter's *membership* arms are the conforming code — pure
translate-validate-delegate. Everything around them is a rule without a lib home.
The adapter is not misbehaving; it is **holding a missing module.**

## Dart app (src/app/lib) verdicts

| # | Check | Verdict | Evidence |
|---|-------|---------|----------|
| 1 | Capability decisions in presentation | **Leak, self-carved-out** | `viewerCanManageMembers`/`viewerIsSuperAdmin`/`viewerCanDeleteScope` (`members_cubit.dart:153-173`), `_viewerCanInvite` = the MEMB-05 matrix (`members_dialog.dart:223-228`), row affordance gates incl. D-03 Super-Admin immunity (`:326-338, 512-514`), `room_rail.dart:425-434, 743-755`. Each carries a comment deferring enforcement to C++ (T-04-07-EoP) — true for security, but the *rule* now has 2+ homes and can drift from the matrix it mirrors. |
| 2 | Dormancy rule | **Computed in Dart, twice** | `approvedBy.isEmpty && pendingAction != UNSPECIFIED` composed in `members_cubit.dart:93-95` — under a comment claiming it "lives entirely on the pushed record, never computed locally" — and re-derived independently in `room_rail.dart:426-428, 746-749`. Total homes for D-07 dormancy: 4 (2 C++, 2 Dart). |
| 3 | Wire-format knowledge | **Regex mirror** | `kInviteLinkPattern` encodes the full `gcs://invite/{space|room}/<id>?key=` grammar client-side (`join_dialog.dart:56-58, 131, 156`) — duplicating the C++ redeem parser's contract. Out-direction is clean: Dart keys links by pushed `scopeId`, never parses (`session_cubit.dart:531-547`). |
| 4 | Topic scheme in the app model | **Hardcoded** | `'gcs/chat/$id'` derived in `rail_cubit.dart:43`, parsed back in `room_rail.dart:41-47` — the third home for the prefix (lib table #8). `kMaxNameLength=64` re-encoded in `space_room_dialog.dart:59, 387`. |
| 5 | Join-topic protocol | **Cubit-side ordering rules** | Idempotent publish on room change, clear-before-publish, null-selection-no-publish (`session_cubit.dart:435-449`); CR-01 active-room message gating (`:499-517`). The *decision* of which room is joined is C++-authority; the *protocol* around publishing it lives in the cubit. |
| 6 | Scaffold boundary | **Clean — cleanest layer audited** | Zero GCS/brand terms in `scaffold/lib` code; all 18 app imports hit barrel-exported public paths; chat-specific generated families live app-side under `src/app/templates/` (`src/app/CMakeLists.txt:32-107`) — drift direction correct. |
| 7 | Generated code | **Clean** | One proto source (`src/proto/gcs_chat.proto`); committed Dart stubs are regeneration-commits only; gitignored chat families generated by CMake; no hand edits found. |
| 8 | Pure display (conforming) | **Clean** | Roster rendering reads pushed state only (`members_dialog.dart:260-294`); rail dims purely on pushed `RoomList` (`room_rail.dart:219, 597, 757`) — `autoJoinRooms` never interpreted client-side; every command publish fills a data-only proto arm. `roleLabel()` enum→string is legitimate display mapping (`members_cubit.dart:18-36`). |

## Target shape

The missing module is a **room-session service in `src/lib`** (composition of
EntityStore + Membership over CoreSession): owns the joined-topic registry and its
rules (dedup, re-join, D-07 ordering, derived/explicit partition, eviction), the
composed D-02 predicate, the send/create/update permission gates (via Membership's
existing `LocalRoleFor`), the topic grammar + length policies, and the joined-check
send_text consults. The FFI file shrinks to translate-validate-delegate — the shape
its membership arms already model. Smaller moves, independent of that one:

- `Membership::Approve` reads entity state through `EntityStore`'s functions
  (the `DeleteScope` path shows how) — kills the lib's only key breach.
- Push computed viewer-capability flags in `MemberList`/`SpaceTree` (the
  `self_address` precedent) OR consolidate Dart predicates to one home — either
  ends the two-homes-in-Dart drift.
- A tripwire test comparing prod canonical payloads vs the test-helper mirrors.
- `"gcs/chat/"` prefix: one exported constant, referenced by the other two homes.

**Open forks (deliberately undecided):** where the room-session service lives
(new module vs extending Membership vs CoreSession facade); Dart capability
predicates (push-from-C++ vs single-home-in-Dart with the T-04-07-EoP carve-out
made explicit); canonical payloads (expose prod builders in the header vs keep the
mirror + tripwire); the D-07 write/verifier dormancy divergence (already a
recorded deferred decision — 2026-10-20 — and the reason the internal mirror is
risky); smoke-topic policy (config vs lib default vs removal).

**The gate that makes it real** (per "gates, not promises"): a `test/` executable
that links **`gcs_core` only — no `gcs_ffi`** and drives a room lifecycle:
create space with autoJoinRooms, create nested room, explicit leave, re-derive —
asserting the joined set. Today it cannot be written; the day it passes, the
adapter is demoted to translation. Alongside it: the canonical-payload tripwire,
and a CI grep forbidding entity/member key literals outside their owning TUs.
The contract is enforced when absence is tested, not promised.
