---
gsd_state_version: 1.0
milestone: v1.0.1
milestone_name: Coupling Remediation
status: planning
last_updated: "2026-11-03T19:36:19.094Z"
last_activity: 2026-11-03
progress:
  total_phases: 7
  completed_phases: 0
  total_plans: 0
  completed_plans: 0
  percent: 0
---

# Project State

## Project Reference

See: .planning/workstreams/app/PROJECT.md (updated 2026-09-22)

**Core value:** Users can create a space, invite others, and have a group conversation where an AI participant (GCS) responds to questions — all synchronized via CRDT without central servers.
**Current focus:** Phase 04 — membership-invites

## Current Position

Phase: Not started (4.1 — room-session service; inserted v1.0.1 phases 4.1–4.4 precede v1.0's remaining phases 5–7)
Plan: —
Status: Planning
Last activity: 2026-11-03 — Milestone v1.0.1 roadmap created (4 phases inserted)

## Performance Metrics

**Velocity:**

- Total plans completed: 11
- Average duration: N/A
- Total execution time: 0 hours

**By Phase:**

| Phase | Plans | Total | Avg/Plan |
|-------|-------|-------|----------|
| 02 | 5 | - | - |
| 03 | 6 | - | - |

**Recent Trend:**

- Last 5 plans: N/A
- Trend: N/A

*Updated after each plan completion*
| Phase 02 P01 | 9min | 3 tasks | 6 files |
| Phase 02 P02 | 5min | 2 tasks | 2 files |
| Phase 02 P03 | 3min | 3 tasks | 7 files |
| Phase 02 P04 | 2min | 2 tasks | 3 files |
| Phase 02 P05 | 3min | 2 tasks | 2 files |
| Phase 03 P01 | 8min | 2 tasks | 2 files |
| Phase 03 P02 | 12min | 2 tasks | 5 files |
| Phase 03 P03 | 10min | 2 tasks | 7 files |
| Phase 03 P04 | 13min | 3 tasks | 8 files |
| Phase 03 P05 | 14min | 2 tasks | 3 files |
| Phase 04 P08 | 11min | 3 tasks | 3 files |
| Phase 04 P09 | 22min | 4 tasks | 7 files |
| Phase 04 P10 | 2min | 3 tasks | 6 files |
| Phase 04 P11 | 6min | 3 tasks | 2 files |
| Phase 04 P12 | 6min | 3 tasks | 1 files |
| Phase 04 P13 | 10min | 2 tasks | 3 files |
| Phase 04 P14 | 15min | 3 tasks | 6 files |
| Phase 04 P15 | 16min | 3 tasks | 7 files |
| Phase 04 P16 | 25min | 3 tasks | 6 files |
| Phase 04 P17 | 5min | 2 tasks | 3 files |
| Phase 04 P18 | 5min | 2 tasks | 1 files |
| Phase 04 P19 | 20min | 3 tasks | 3 files |
| Phase 04 P20 | 20min | 3 tasks | 2 files |
| Phase 04 P21 | 12min | 3 tasks | 4 files |
| Phase 04 P22 | 25min | 3 tasks | 3 files |
| Phase 04 P23 | 14min | 2 tasks | 3 files |

## Accumulated Context

### Decisions

Decisions are logged in PROJECT.md Key Decisions table.
Recent decisions affecting current work:

- [Roadmap]: Single unified room model with `autoAnswer` policy
- [Roadmap]: App-layer encryption (not libp2p PSK) — deferred to v1.1
- [Roadmap]: Spaces as containers with `autoJoinRooms` config
- [Roadmap]: Multiple Admins, no single Owner
- [Roadmap]: Super Admin = creator, cannot be demoted
- [Roadmap]: Destructive actions need Super Admin approval when 2+ admins
- [Phase 02]: Unparseable manifest bytes = empty catalog (union write heals); Get failure = absent key (Pitfall 2)
- [Phase 02]: EntityStore rejections reuse Error::GcsDbError — no NotFound code added; FFI arms do INVALID_ARGUMENT validation first
- [Phase 02]: UpdateSpace preserves created_at_ms + tombstone state from stored record; full-state rewrite otherwise
- [Phase 02]: Tombstoned records stay in-memory (hidden by readers) so IsValidParentSpace keeps returning false
- [Phase 02]: FFI entity arms validate first (empty name / unknown parent / empty space_id -> INVALID_ARGUMENT before any Put); store failures surface as GENERIC with the entity name in the raw error string
- [Phase 02]: create_space pushes SpaceTree only (new space has no rooms); create_room/update_space run RefreshDerivedJoins before pushing so SpaceTree + RoomList stay consistent in one publish
- [Phase 02]: Derived-join projection — failed topic registration keeps the topic out of g_roomTopics; toggle-false removes from projection only while pubsub stays sticky (no Remove*Topic API, Pitfall 4)
- [Phase ?]: Phase 02 P03: proto comments backtick-wrap <id> tokens — raw angle brackets flow into generated Dart doc comments and trip unintended_html_in_doc_comment under --fatal-infos; fix at the proto source, never in generated files
- [Phase ?]: Phase 02 P03: setTree sends unmatched-parent rooms to standaloneRooms (defensive; orphans unreachable via FFI validation) — catalog data never silently dropped; hasSpaceTree dispatched first (D-02 tree before membership)
- [Phase ?]: Phase 02 P04: dialog form state lives in a private _DialogForm ChangeNotifier shared by the children/footer subtrees (ResponsiveDrawer.show mounts them separately; one State cannot rebuild both); disposal rides the drawer's onClose
- [Phase ?]: Phase 02 P04: dialog confirm disables only on raw-empty name; whitespace-only stays enabled so the inline 'Enter a name.' validation is reachable
- [Phase ?]: Phase 02 P04: dialog title is static per open (atom title is a String); createFromHeader opens as 'New space' on the default type — hint and confirm label flip live with the selector
- [Phase 02 P05]: Rail expansion state is a view-local collapsed-ids map (absent = expanded) on RoomRail — new spaces default open, keyed by id so expansion survives setTree full replacements; never enters RailState
- [Phase 02 P05]: One _TreeRoomRow serves nested + standalone rows (leadingInset parameter); standalone rows are permanently dimmed emergently (never in the pushed RoomList in Phase 2), and unjoined gating layers ScaffoldPressable(disabled) over the selectRoom joined-only guard (T-02-11)
- [Phase 03]: Buffer::toString() returns std::string_view (binary-safe data+size); use std::string{buf.toString()} for D-08 envelope bytes — never C-string APIs
- [Phase 03]: OpenSSL linkage: OpenSSL::Crypto imported target (vendored 3.3.3) is the only sanctioned handle; pkgcfg_lib__OPENSSL_crypto/ssl + _OPENSSL_LDFLAGS point at homebrew 3.6.3 (forbidden)
- [Phase 03]: HKDF PKEY setters are legacy-3.0-guarded (OPENSSL_NO_DEPRECATED_3_0) but present in the default vendored build; 03-02 must retain m_pubsub before the std::move into GlobalDB::New
- [Phase 03]: QueryKeyValues returns raw datastore keys (/crdt/k/<key>/v); wrapper is a blind std::string passthrough — 03-04 parses the message id from the raw key
- [Phase 03]: PutLocal (SuperGenius PutKeyLocal) is overwrite-only — requires the key's priority record to already exist; fresh-key PutLocal is rejected, so 03-04 receive-path archive write must account for this
- [Phase 03]: GcsGlobalDb retains the shared GossipPubSub (m_pubsub) before std::move into GlobalDB::New — one handle backs CRDT broadcast + raw live path (D-03)
- [Phase 03]: Receive-path archive write uses the 2-arg Put (empty topics = local-only, no rebroadcast) instead of PutLocal — SuperGenius PutLocal/PutKeyLocal is overwrite-only and rejects fresh keys (03-02 verified contract)
- [Phase 03]: RoomTopicFromKey strips raw datastore framing (/crdt/k/.../v) and the leading '/' HierarchicalKey adds to callback keys; QueryHistory ignores the raw QueryKeyValues key (the value carries room/id)
- [Phase 03]: CryptoSeam built by member assignment, not brace-init — CryptoSeam{...} is not a C++17 aggregate (user-provided default ctor from 03-04), so the plan's brace-init form does not compile
- [Phase 03]: Deleted NextMessageId()/kMessageIdPrefix/g_messageSeq from the FFI — id minting now lives entirely in gcs_messaging; the FFI is a thin validate-and-delegate dispatcher
- [Phase 03]: P06 two-node deadlock root cause FIXED — three faces of one class: (1) ApplyMessage's synchronous archive Put blocked the GossipSub strand (fixed bd0ea41: async archive worker in gcs::Messaging); (2) heal-callback pattern was regex_match-inert — kMessagesKeyCallbackPattern "/gcs/messages/.*" now full-matches (7d1a105); (3) activating the heal path exposed a g_mutex ABBA — gcs_publish held g_mutex across SendMessage's WaitForJob-put while the DagWorker heal lambda needed g_mutex (fixed a0f8499: lock contract is "g_mutex guards FFI globals + PostToDart; Messaging is never called under g_mutex")
- [Phase 03]: Deferred hardening — GcsGlobalDb graphsync scheduler runs on private m_io; SuperGenius mandates the pubsub host's asio context (cross-thread WriteQueue Debug-assertion risk). c5f8104 tried, e51f08e reverted (no deadlock effect); follow-up candidate
- [Phase 04]: EntityStore owns a catalog mutex; accessors lock internally and return by value, mutators never hold it across a session Put/Get — Closes CR-01 without duplicating the locking discipline at every FFI call site; preserves the FFI g_mutex lock order
- [Phase 04]: Invite authenticator is an AES-256-GCM envelope of a SHA-256 marker over the prefix, scope id, and invite id, keyed by HKDF(inviteKey) and hex-encoded for the proto3 string field
- [Phase 04]: RedeemInvite verifies the bearer authenticator unconditionally when the seam is enabled, before any member write
- [Phase 04]: InviteRecord.is_space is stamped at mint and cross-checked against the URL kind at redeem
- [Phase 04]: Member and invite records replicate on Membership::kMembershipTopic; the scope id stays in the record key
- [Phase 04]: PrivilegeRank is the single ordering authority for removal, demotion dormancy, and AdminCount
- [Phase 04]: Pending invite links are keyed by InviteLink.scope_id in a per-scope map; the unkeyed accessor was removed
- [Phase 04]: Invite pending links clear on drawer close, never on read, so the push emission still delivers the link
- [Phase 04]: Membership resets beside Messaging before session->Shutdown; it borrows the session and catalog but owns no worker thread to drain
- [Phase 04]: gcs_init joins Membership::kMembershipTopic listen-then-broadcast before installing g_session; join failure shuts the local session down and fails init
- [Phase 04]: kMembershipTopic is infrastructure and never enters g_roomTopics, so no phantom room reaches pushed lists
- [Phase 04]: Production KeyWrapSeam assigns mintAuth/verifyAuth beside wrap/unwrap so InviteAuthEnabled() is true
- [Phase 04]: BuildInviteLinkEvent stamps the command scope_id onto the pushed InviteLink; Dart never parses the URL
- [Phase 04]: Negative multinode convergence assertions compare against a baseline captured AFTER the positive sub-case; the invariant is record count/bytes unchanged, never 'only the creator'
- [Phase 04]: Wire-level forged invites are written through session->Put on kMembershipTopic; no test-only malformed-invite helper exists
- [Phase 04]: Remote-removal convergence is asserted on the pushed member_list sink after draining B's events, pinning OnMemberArrived rather than the local CRDT scan
- [Phase 04]: RedeemInvite delivers key material only: an existing live member keeps its stored role and dormant state, and a fresh record is always ROLE_GUEST
- [Phase 04]: A tombstoned wallet redeem is rejected after the authenticator gate and before the unwrap loop so no member record or room key is written
- [Phase 04]: ApplySpaceState/ApplyRoomState are full-state entity writes: the session is authoritative, so no immutable preservation or timestamp restamp; the catalog mirrors the stored record and tombstoned entries stay in the map
- [Phase 04]: Approve's entity branch learns only the record kind from the catalog, then evaluates the dormancy predicate on the freshly re-read authoritative bytes
- [Phase 04]: AdminCount fails closed (Error::GcsDbError) because it gates the D-07 immediate-destructive branch; ChangeRole, RemoveMember, and DeleteScope propagate it
- [Phase 04]: Invite mint and redeem fail closed without the issuer seam (KeyWrapSeam sign + verifyIssuer) — an unsigned InviteRecord on a permissionless CRDT topic is forgeable by any writer (CR-08)
- [Phase 04]: The issuer signature covers domain + invite id + the record with issuer_signature cleared, and verification binds the CLAIMED created_by — a signature that does not name its signer can be replayed under a different issuer
- [Phase 04]: The D-05 GUEST role cap applies on every redeem; issuer authority is checked only when the scope resolves locally — an authorized membersCanInvite member could otherwise self-sign an escalation, and a cold node cannot resolve remote roles (T-04-15-04)
- [Phase 04]: Invite URLs carry kind/scope/invite and redeem does an exact InviteKey(scope, invite) lookup instead of a global scan — the URL scope must match the record scope or a colliding invite id could be redeemed across scopes
- [Phase 04]: The FFI builds one GeniusAccount per session from the node's active mnemonic; an absent account leaves the issuer seam unset so mint/redeem fail closed
- [Phase 04]: g_inviteAccount is a shared_ptr captured per seam lambda and reset after membership eviction, so no live callable dangles
- [Phase 04]: verifyIssuer strips 0x, requires exactly kSignatureBytes, and reorders the seam to VerifySignature(address, signature, data)
- [Phase 04]: The FFI fixture pins a deterministic BIP39 mnemonic so tests exercise the production issuer seam with no test-only override
- [Phase 04]: CanChangeRole never grants ROLE_SUPER_ADMIN: it is the D-03 root created with its scope, never a grantable role — Closes CR-03; prevents a second authority root for every author including the creator
- [Phase 04]: CanChangeRole rejects ROLE_UNSPECIFIED — It sorts below Guest and would corrupt every PrivilegeRank comparison (CR-03)
- [Phase 04]: CanChangeRole requires a live target and strict outrank of the target's CURRENT stored role, mirroring CanRemoveMember — Closes CR-04; a Moderator cannot act on an Admin/peer Moderator and an Admin cannot act on a non-creator Super Admin
- [Phase 04]: The creator exception sits after the target read and before the rank comparison — Preserves the documented WR-03 peer Super Admin demotion dormancy workflow (T-04-17-05)
- [Phase 04]: The multinode fixture owns real GeniusSigner identities declared before the nodes, so the issuer seam's reference capture cannot dangle and ShutdownNode evicts the membership first
- [Phase 04]: The multinode verifyIssuer strips 0x, guards Membership::kSignatureBytes, and reorders the seam to VerifySignature(address, signature, data), proving the production argument order on the wire
- [Phase 04]: The forged-issuer test mints a valid authenticator from the existing kWellFormedInviteKey (32 zero bytes) so only issuer binding can reject the record
- [Phase 04]: The member authority signature covers authority fields only; record_signature, leave_signature, wrapped_room_key, and explicit_leave are excluded because they are self-referential, subject-owned, or bearer material — never authority
- [Phase 04]: prior_role is derived in exactly one place (WriteMemberAsAuthor) from the verified pre-write record, and updated_at_ms is stamped monotonically as max(NowMs(), verified + 1) before signing so consecutive authority writes stay strictly ordered
- [Phase 04]: Authority-preserving writes (leave, redeem key refresh) never re-sign or re-stamp; they keep the authority bytes verbatim and only warn when a cold node cannot verify, so read paths remain the authority gate (T-04-19-08 accepted)
- [Phase 04]: MembersFor and AdminCount derive the wallet from the key and verify BEFORE the tombstone skip, so a verified tombstone advances the node-local predecessor cache and cannot be bypassed by a pre-tombstone replay
- [Phase 04]: OnMemberArrived pushes nothing when verification fails; an unverifiable record stays in the CRDT but never enters a roster or an authority decision
- [Phase 04]: The verified-predecessor cache is in-memory and non-replicated; a cold node falls back to the signed prior_role, and the cold-start rollback residual is documented (T-04-19-09)
- [Phase 04]: Approve's REMOVE branch still writes deleted + pending REMOVE; the verifier correctly treats that as incoherent, and 04-21 T3 clears the pending fields (known transient, not a regression)
- [Phase 04]: One shared test-side member/leave payload builder and hex signature helper; SeedMember and arrival records sign as their claimed creator
- [Phase 04]: Member seeds/restores replicate on kMembershipTopic; a 2-arg re-write of identical bytes yields the same CRDT node CID and is deduplicated, so it cannot restore a value
- [Phase 04]: A forged creator root grants nobody authority: every creator-rooted chain fails closed, including the attacker's own role (pinned as GUEST)
- [Phase 04]: MemberRecord gained created_by (12), record_signature (13), leave_signature (14), and prior_role (15); the canonical authority payload covers authority fields only, KEEPS created_at_ms, updated_at_ms, and prior_role, and clears record_signature, leave_signature, wrapped_room_key, and explicit_leave (T-04-19-02/09/10)
- [Phase 04]: Every member write is either author-signed (WriteMemberAsAuthor) or authority-preserving (WriteMemberPreservingAuthority), and every read path verifies before use (T-04-19-01/03)
- [Phase 04]: Authority preservation never mutates authority fields, never re-stamps updated_at_ms, and never re-signs; it warns when local verification is impossible (a cold node cannot resolve the creator) but still applies the non-authority update, because failing would block a legitimate leave/redeem or force a Guest downgrade — reads remain the authority gate (T-04-19-08 accepted amplification-only risk)
- [Phase 04]: A self-signed record is Guest-only unless it is the resolved creator's Super Admin root; a third-party record must chain to a verified signer, pass the unconditional creator-target rejection, and satisfy strict outrank against the signed prior_role plus the grant matrix (T-04-19-01/02/04/05)
- [Phase 04]: explicit_leave == true requires a subject-signed leave_signature and false requires it empty; wrapped_room_key remains mutable by any verified author and is an availability-only risk (T-04-19-03, D-02)
- [Phase 04]: Verification fails closed on a cold node without the scope entity and on an absent issuer seam; recorded residual: per-key LWW (D-01) has no anti-rollback ordering, so a cold node may accept an older validly signed state until it verifies a newer one, while the in-memory predecessor cache rejects a replay on any node with history (T-04-19-09)
- [Phase 04]: CanChangeRole's creator exception is IsSuperAdmin(scopeId, m_sender) — the resolved catalog identity, never the record's stored role — closing WR-02/T-04-21-01
- [Phase 04]: Approve re-validates pending_role (Admin/Moderator/Member/Guest only) and rejects a creator target; its REMOVE branch clears pending_action and pending_role so the creator-signed tombstone passes verification and converges (WR-03/WR-05, T-04-21-02/06)
- [Phase 04]: ChangeRole rejects while PENDING_ACTION_REMOVE is set; RemoveMember clears pending_role on dormant writes and clears pending_action, pending_role, and approved_by on immediate writes (WR-05, T-04-21-03/04/05)
- [Phase 04]: WriteMemberAsAuthor stamps updated_at_ms = max(NowMs(), verifiedExisting.updated_at_ms() + 1) (or NowMs() with no verified predecessor) before signing, so two authority writes in the same millisecond are strictly ordered and the predecessor gate accepts the second (04-21 T1)
- [Phase 04]: Pending transitions require an admin-tier participant: pending REMOVE is accepted only with priorRank >= ADMIN || signerRank >= ADMIN, pending ROLE_CHANGE only with priorRank >= ADMIN; RemoveMember applies the same predicate, so a Moderator's Member removal is immediate rather than a wire-only dormant state with no cancel path (T-04-21-07)
- [Phase 04]: A cold node verifies nothing until the entity catalog resolves the scope: the fixture mirrors the manifest and space/room records A-to-B after OnScopeCreated, so membership is excluded before metadata and re-verified after; production's LoadFromStore-before-Membership ordering is the contract this models, and a late OnMemberArrived heals the roster (T-04-22-03/07)
- [Phase 04]: Wire-injected authority is excluded, not rolled back: a validly signed hostile record remains the winning raw CRDT value while both rosters omit the target and every command fails; restoring the saved creator-signed record recovers the target. A signer-bound Membership must warm its predecessor cache before the hostile publish, because a never-synced instance trusts the signed prior_role (the documented cold-start residual), while a live node's verified history rejects the lie (T-04-22-01/02/04/06)
- [Phase 04]: The scope binding lives at the two authority-preserving call sites (RedeemInvite, Leave), not inside TryReadRawMember — a helper-level false is indistinguishable from absent and would make redeem take the fresh-record branch and succeed while reversing OnScopeCreated's write-once root (CR-01)
- [Phase 04]: TryReadRawMember stays behavior-identical; its Doxygen now states the non-binding raw contract and names how each caller is bound, so a future raw caller cannot repeat the defect silently

### Pending Todos

None yet.

### Blockers/Concerns

- Phase 1 depends on GlobalDB CRDT integration from GNUS-NEO-SWARM Phase 3 — verify availability before planning.

### Quick Tasks Completed

| # | Description | Date | Commit | Directory |
|---|-------------|------|--------|-----------|
| 260922-gev | Audit frontend/scaffold separation of concerns vs genius-ai-boss pattern | 2026-09-22 | 846c7c1 | [260922-gev-audit-frontend-scaffold-separation-of-co](../quick/260922-gev-audit-frontend-scaffold-separation-of-co/) |

## Deferred Items

Items acknowledged and carried forward from previous milestone close:

| Category | Item | Status | Deferred At |
|----------|------|--------|-------------|
| Membership authority | Wire-level admin-authored immediate tombstone accepted by `VerifyMemberRecord` while the command path makes the same removal dormant when 2+ admins | Deferred by user decision 2026-10-20 — requires its own design decision; do not fold into another plan (see 04-23-PLAN.md:102) | 2026-10-20 |
| Room-key persistence | CR-07 — room keys live in the process-local store; no restart/cross-process survival | Deferred by user decision — no later phase assigned (ROADMAP.md Phase 4 deferral note) | 2026-10-19 |

## Session Continuity

Last session: 2026-10-21T17:46:17.836Z
Stopped at: Gap plan 04-23 executed: RedeemInvite/Leave reject cross-scope raw records before the authority-preserving write, TryReadRawMember documents its non-binding contract, byte-level regression green (full suite 13/13). Next: re-run /gsd:verify-work for phase 04.
Resume file: None
