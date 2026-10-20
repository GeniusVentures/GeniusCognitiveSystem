/**
 * @file       gcs_membership.hpp
 * @brief      Membership component (Phase 4): the C++-owned member roster,
 *             five-tier role/permission enforcement, capability-token invite
 *             mint/redeem, and the D-07 dormant-until-approved model.
 * @details    Membership mirrors gcs_entity_store (per-key LWW records,
 *             tombstone-skip reads, full-state rewrites) and gcs_messaging
 *             (EventSink push, deferred-registration constructor, injected
 *             seam) to implement D-01 (one serialized MemberRecord per member
 *             per scope under gcs/members/<scopeId>/<wallet>, enumerated by
 *             the Phase 3 QueryKeyValues prefix scan — no manifest), D-03
 *             (Super Admin write guard), D-05 (bearer-key invite in a
 *             gcs://invite/{space|room}/<inviteId>?key=<IK> URL) and D-07
 *             (dormant-until-approved destructive ops). It also carries D-06's
 *             per-member AES-GCM key-wrap through the injected KeyWrapSeam,
 *             never including OpenSSL headers itself.
 *
 *             Invite model (write-at-redeem): MintInvite writes NO member
 *             record — the recipient wallet is unknown at mint. It mints an
 *             opaque invite id + a random bearer key IK, wraps the target room
 *             key(s) under IK, and persists ONE InviteRecord under
 *             gcs/invites/<scopeId>/<inviteId>. RedeemInvite looks that record
 *             up by inviteId, unwraps the room key(s) with IK, stores them in
 *             the per-room key store, then writes the redeemer's wallet-keyed
 *             member record. Multi-use/no-expiry (D-05) holds because the
 *             invite record is never consumed — N redeemers each write their
 *             own member record.
 * @copyright  (c) 2026 GNUS.AI
 */

#ifndef GCS_MEMBERSHIP_HPP
#define GCS_MEMBERSHIP_HPP

#include <cstddef>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "gcs_core.hpp"        // CoreSession + gcs::outcome alias
#include "proto/gcs_chat.pb.h" // MemberRecord / MemberList / Role / InviteRecord

namespace gcs {

// The entity catalog is borrowed by reference; the full definition is needed
// only by the implementation (forward declaration keeps this header light).
class EntityStore;

/**
 * @brief C++-owned membership component (roster, roles, invites, approval).
 *
 * The constructor stores its dependencies only (deferred registration idiom —
 * no I/O). The sender address is injected by the FFI layer (the local wallet
 * address, D-03 identity). The EventSink receives plaintext GcsEvent messages
 * only (MemberList / SpaceTree).
 */
class Membership {
public:
  /// Push sink — receives plaintext GcsEvent envelopes.
  using EventSink = std::function<void(const chat::GcsEvent &)>;
  /// Key-wrap callable of the D-06 seam (member key + room key -> envelope).
  using WrapFn = std::function<outcome::result<std::string>(
      const std::vector<unsigned char> &memberKey,
      const std::vector<unsigned char> &roomKey)>;
  /// Key-unwrap callable of the D-06 seam (member key + envelope -> room key).
  using UnwrapFn = std::function<outcome::result<std::vector<unsigned char>>(
      const std::vector<unsigned char> &memberKey, const std::string &envelope)>;
  /// Invite-authenticator mint callable (invite key + scope id + invite id ->
  /// authenticator envelope).
  using MintAuthFn = std::function<outcome::result<std::string>(
      const std::vector<unsigned char> &inviteKey, const std::string &scopeId,
      const std::string &inviteId)>;
  /// Invite-authenticator verify callable (invite key + scope id + invite id +
  /// authenticator -> authenticated?).
  using VerifyAuthFn = std::function<outcome::result<bool>(
      const std::vector<unsigned char> &inviteKey, const std::string &scopeId,
      const std::string &inviteId, const std::string &authenticator)>;
  /// Issuer-signing callable: (canonical payload bytes, issuer address) ->
  /// the RAW secp256k1 signature bytes (a binary-safe string of exactly
  /// kSignatureBytes bytes). Hex conversion happens at the proto boundary,
  /// never inside the seam.
  using SignFn = std::function<outcome::result<std::string>(
      const std::string &payload, const std::string &issuer)>;
  /// Issuer-verification callable: (issuer address, canonical payload bytes,
  /// RAW signature bytes) -> verifies?. NOTE the seam order — issuer, payload,
  /// signature. The adapter into `sgns::GeniusSigner::VerifySignature` MUST
  /// reorder to (address, signature, data): the payload never occupies the
  /// signature slot.
  using VerifyIssuerFn = std::function<outcome::result<bool>(
      const std::string &issuer, const std::string &payload,
      const std::string &signature)>;
  /// Member-scan callable (key prefix -> raw key/value pairs) used by
  /// AdminCount. Empty means CoreSession::QueryKeyValues (production); the
  /// narrow seam exists so the fail-closed scan branch of the approval gate is
  /// exercisable without a broken store, and must not replace any other
  /// session call.
  using ScanFn = std::function<outcome::result<
      std::vector<std::pair<std::string, std::string>>>(const std::string &keyPrefix)>;

  /**
   * @brief D-06 injected key-wrap seam — injectable per Membership instance.
   *
   * Production wires gcs::crypto::WrapRoomKey / UnwrapRoomKey; tests wire the
   * real adapter. When enabled is false (or either callable is empty) the
   * wrap/unwrap callables are never called and invite records carry no wrapped
   * room keys (the same never-call-when-disabled posture as Messaging's
   * CryptoSeam). The invite authenticator is minted/verified only when the seam
   * is enabled AND both mintAuth/verifyAuth are set. The issuer sign/verify
   * pair is likewise only used when the seam is enabled AND both are set;
   * invite minting and redeeming REQUIRE it (fail closed — an unsigned invite
   * record is never accepted).
   */
  struct KeyWrapSeam {
    WrapFn wrap;             ///< Wrap callable (empty = no wrapped keys)
    UnwrapFn unwrap;         ///< Unwrap callable (empty = no wrapped keys)
    MintAuthFn mintAuth;     ///< Authenticator mint callable (empty = auth off)
    VerifyAuthFn verifyAuth; ///< Authenticator verify callable (empty = auth off)
    SignFn sign;             ///< Issuer sign callable (empty = issuer auth off)
    VerifyIssuerFn verifyIssuer; ///< Issuer verify callable (empty = issuer auth off)
    bool   enabled;          ///< Master switch for the D-06 path

    /// Default-constructs a disabled, empty seam (no wrapped keys, no auth).
    KeyWrapSeam()
        : wrap(), unwrap(), mintAuth(), verifyAuth(), sign(), verifyIssuer(),
          enabled( false )
    {
    }
  };

  /// Member-record key prefix — one serialized MemberRecord per member per
  /// scope under gcs/members/<scopeId>/<wallet> (D-01, no manifest).
  static constexpr const char *kMembersKeyPrefix = "gcs/members/";

  /// Invite-record key prefix — one serialized InviteRecord per mint under
  /// gcs/invites/<scopeId>/<inviteId> (D-05 write-at-redeem).
  static constexpr const char *kInvitesKeyPrefix = "gcs/invites/";

  /// CRDT receive-callback regex pattern (FULL match per CRDTCallbackManager's
  /// std::regex_match) — matches the member key form the callback receives,
  /// "/gcs/members/<scopeId>/<wallet>".
  static constexpr const char *kMembersKeyCallbackPattern = "/gcs/members/.*";

  /// Well-known membership replication topic (WR-06). A recipient by
  /// definition has NOT joined the invited scope's room topic, so member and
  /// invite records replicate on this one channel that every node registers at
  /// init (production registration in 04-11, exercised in 04-12). The scope id
  /// stays in the record KEY, never in the topic.
  static constexpr const char *kMembershipTopic = "gcs/membership/replication";

  /// Raw secp256k1 signature length in bytes (64) — the ONE home for this
  /// length. The FFI adapter (04-16) and the multinode fixture (04-18) MUST
  /// reference this constant instead of a literal 64.
  static constexpr std::size_t kSignatureBytes = 64;

  /**
   * @brief Construct the component, storing its dependencies only.
   *
   * Performs no I/O and no fallible work (deferred registration idiom).
   *
   * @param[in] session       The owning session; must outlive this component.
   * @param[in] entities      The entity catalog (scope lookup); must outlive
   *                          this component.
   * @param[in] senderAddress The local wallet address (D-03 authority stamp).
   * @param[in] sink          Plaintext event sink (Dart push plane).
   * @param[in] keyWrap       The injected D-06 seam (defaults to disabled).
   * @param[in] memberScan    AdminCount's member enumeration seam (defaults to
   *                          CoreSession::QueryKeyValues). Tests inject a
   *                          failing scan to exercise the fail-closed branch.
   */
  explicit Membership(CoreSession &session, EntityStore &entities,
                      std::string senderAddress, EventSink sink,
                      KeyWrapSeam keyWrap = {}, ScanFn memberScan = {});

  Membership(const Membership &) = delete;
  Membership &operator=(const Membership &) = delete;

  /**
   * @brief Enumerate a scope's live roster (D-01/D-04).
   *
   * Runs the QueryKeyValues prefix scan over gcs/members/<scopeId>/, parses
   * each value into a MemberRecord, derives the wallet from the KEY (a
   * body/key mismatch is skipped), verifies the record (signature, creator
   * chain, authority transition matrix, predecessor gate) and skip-and-logs
   * unparseable or unverifiable bytes, then skips tombstoned (deleted())
   * records. Verification runs BEFORE the tombstone skip so a verified
   * tombstone advances the predecessor cache. The returned MemberList's
   * self_address is stamped from m_senderAddress (the pushed local wallet) so
   * Dart derives the "You" marker from authority instead of guessing.
   *
   * An unverifiable record is not a member: it is excluded from the roster and
   * from every authority decision.
   *
   * @param[in] scopeId Space id or standalone room id.
   * @return outcome::success with the flat full-replacement roster; propagated
   *         Error on a failed scan.
   */
  outcome::result<chat::MemberList> MembersFor(const std::string &scopeId) const;

  /**
   * @brief Resolve the local sender's role in a scope.
   *
   * A missing or tombstoned local member record resolves to ROLE_GUEST (the
   * safest default — D-08 read-only).
   *
   * @param[in] scopeId Space id or standalone room id.
   * @return The local member's role, or ROLE_GUEST when absent.
   */
  chat::Role LocalRoleFor(const std::string &scopeId) const;

  /**
   * @brief Whether the local sender has explicitly left a space scope (D-02).
   *
   * @param[in] spaceId Space id whose space-scope member record is consulted.
   * @return true when the local member record carries explicit_leave.
   */
  bool HasExplicitLeave(const std::string &spaceId) const;

  /**
   * @brief Mint a capability-token invite link (D-05/D-06/MEMB-01).
   *
   * Validates the scope is a live space or standalone room, enforces CanInvite
   * (MEMB-05), requires the issuer-signature seam (fail closed), mints an
   * opaque invite id + a random bearer key IK, wraps the target room key(s)
   * under IK, stamps the issuing wallet (created_by) and its signature over
   * the canonical record, persists ONE InviteRecord under
   * gcs/invites/<scopeId>/<inviteId>, and writes NO member record (the
   * recipient wallet is unknown until redeem).
   *
   * @param[in] scopeId Space id or standalone room id.
   * @return outcome::success with the
   *         gcs://invite/{space|room}/<scopeId>/<inviteId>?key=<IK> URL; Error
   *         when the scope is invalid, the sender lacks invite permission, the
   *         issuer seam is unavailable, key minting/signing fails, or the
   *         invite-record Put fails.
   */
  outcome::result<std::string> MintInvite(const std::string &scopeId);

  /**
   * @brief Redeem an invite link: unwrap keys, deliver key material (D-05).
   *
   * Re-parses the exact
   * gcs://invite/{space|room}/<scopeId>/<inviteId>?key=<key> shape, looks the
   * record up under InviteKey(scopeId, inviteId), cross-checks the record's
   * scope and kind, verifies the bearer authenticator and the issuer signature
   * (rejecting an unsigned or foreign record and an issuer without invite
   * authority when the scope resolves locally), enforces the D-05 GUEST role
   * cap on every redeem, then unwraps each scope-validated wrapped room key
   * (undecryptable envelopes of a kept entry are skipped and logged — never
   * aborting the whole redeem) and stores them in the per-room key store.
   *
   * The redeemer's own wallet-keyed member record is then merged, not
   * overwritten: an existing LIVE member keeps its stored role, approved_by,
   * pending_action, pending_role, deleted and explicit_leave — only the
   * carried key material is refreshed, so a redeem can never grant a role or
   * cancel a dormant destructive op (a pending removal is activated only by
   * Approve). The preserved record keeps its authority signature, prior role,
   * and signed timestamp verbatim (an authority-preserving write); a record
   * that does not yet exist is author-signed as ROLE_GUEST. A TOMBSTONED
   * record is rejected immediately after the authenticator gate and before the
   * unwrap loop — neither a member record nor a room key is written
   * (CR-01/CR-02). The invite record is never consumed (multi-use).
   *
   * @param[in] inviteUrl The full invite link to redeem.
   * @return outcome::success once the member record is written and the roster
   *         pushed; Error on a malformed URL, an unknown invite, an
   *         unauthenticated bearer key, a tombstoned wallet, or a failed write.
   */
  outcome::result<void> RedeemInvite(const std::string &inviteUrl);

  /**
   * @brief Change a member's role with full-state LWW semantics (D-03/D-08).
   *
   * Rejects any demotion targeting the scope creator (MEMB-03) and any grant
   * the author's role cannot make. An admin demotion with 2+ admins writes
   * dormant (empty approved_by + pending_action ROLE_CHANGE + pending_role);
   * otherwise the record is rewritten with the new role. A member with a
   * pending removal is rejected outright: a pending REMOVE is terminal until
   * Approve, so a role change cannot silently cancel it (WR-05). Pushes the
   * roster.
   *
   * @param[in] scopeId Space id or standalone room id.
   * @param[in] target  Target member wallet address.
   * @param[in] role    Desired role.
   * @return outcome::success on a written change; Error when the guard rejects
   *         or the target record is absent/tombstoned.
   */
  outcome::result<void> ChangeRole(const std::string &scopeId,
                                   const std::string &target, chat::Role role);

  /**
   * @brief Remove a member (D-07): dormant when 2+ admins, else tombstone.
   *
   * Dormancy uses the same predicate as verification: 2+ admins AND (the
   * target or the author is admin-tier). A Moderator's Member removal is
   * therefore immediate rather than an unverifiable wire-only state. A
   * dormant REMOVE clears pending_role; an immediate tombstone clears
   * pending_action, pending_role, and approved_by, so neither path leaves
   * stale approval or role state (WR-05).
   *
   * @param[in] scopeId Space id or standalone room id.
   * @param[in] target  Target member wallet address.
   * @return outcome::success on a written removal; Error when the guard
   *         rejects or the target record is absent/tombstoned.
   */
  outcome::result<void> RemoveMember(const std::string &scopeId,
                                     const std::string &target);

  /**
   * @brief The caller explicitly leaves a scope (D-02 — never a tombstone).
   *
   * Writes explicit_leave = true on the caller's own record and stamps a
   * SUBJECT signature over the canonical leave payload (another wallet cannot
   * flip the flag on a copied record). The record is written by an
   * authority-preserving write, so the existing authority signature, prior
   * role, and signed timestamp survive even on a cold node. The Super Admin
   * cannot leave (guard rejects).
   *
   * @param[in] scopeId Space id or standalone room id.
   * @return outcome::success on a written leave; Error when the caller is the
   *         Super Admin or has no live record in the scope.
   */
  outcome::result<void> Leave(const std::string &scopeId);

  /**
   * @brief Delete a scope (D-07): dormant when 2+ admins, else tombstone.
   *
   * Rewrites the ENTITY record — dormant (empty approved_by +
   * pending_action DELETE, not tombstoned) when 2+ admins exist, otherwise
   * tombstoned. Pushes the SpaceTree.
   *
   * @param[in] scopeId Space id or standalone room id.
   * @return outcome::success on a written deletion; Error when the guard
   *         rejects, the scope is unknown, or the entity write fails.
   */
  outcome::result<void> DeleteScope(const std::string &scopeId);

  /**
   * @brief Activate a dormant destructive op (D-07, Super Admin only).
   *
   * Reads the target record (member for REMOVE/ROLE_CHANGE, the entity for
   * DELETE), requires an empty approved_by and a matching pending_action,
   * overwrites approved_by with the author's address (single-signer overwrite
   * on the SAME key — never a cross-key join), then applies the op.
   *
   * A pending REMOVE is re-validated into a verifiable tombstone: the approved
   * record clears pending_action and pending_role before the creator-signed
   * write, because a deleted record that still carries a pending action is
   * rejected by verification (WR-05). A pending ROLE_CHANGE is re-validated
   * before mutation: its pending_role must be exactly Admin, Moderator,
   * Member, or Guest (never Super Admin or Unspecified) and its target may
   * never be the creator — the approval is irreversible (WR-03).
   *
   * @param[in] scopeId Space id or standalone room id.
   * @param[in] target  Member address (remove/role change) or the scope id
   *                    itself (scope deletion).
   * @return outcome::success on an applied approval; Error when the guard
   *         rejects or no matching dormant op exists.
   */
  outcome::result<void> Approve(const std::string &scopeId,
                                const std::string &target);

  /**
   * @brief Handle a CRDT heal delivery of a member record (D-01/D-04).
   *
   * Derives the scope id from the key, parses the value, derives the wallet
   * from the key, and verifies the record. An unparseable or unverifiable
   * record is skip-and-logged and pushes NOTHING (an unverifiable record is
   * not a member). A verified arrival pushes the affected scope's MemberList so
   * remote joins/role changes converge live (never a restart). A tombstoned
   * arrival still pushes the tombstone-filtered roster so a remote removal
   * converges instead of being silently dropped (WR-05).
   *
   * @param[in] key   The member key (logical or raw datastore form).
   * @param[in] value The serialized MemberRecord bytes.
   */
  void OnMemberArrived(const std::string &key, const std::string &value);

  /**
   * @brief Mint the creator's own Super Admin record on scope creation (D-03).
   *
   * For a standalone room (!isSpace) it also mints the room's 32-byte key,
   * stores it in the per-room key store, and carries it wrapped under a
   * freshly minted creator member key on the record.
   *
   * The creator root is author-signed by the local wallet and write-once: a
   * second call returns success without rewriting an already-verified root,
   * and fails closed rather than overwrite an occupying record it cannot
   * verify (D-03).
   *
   * @param[in] scopeId   Newly created space or standalone room id.
   * @param[in] roomTopic The room topic (gcs/chat/<id>) for key storage.
   * @param[in] isSpace   true for a space scope, false for a standalone room.
   * @return outcome::success once the record is written and the roster pushed;
   *         Error when key minting or the record write fails.
   */
  outcome::result<void> OnScopeCreated(const std::string &scopeId,
                                       const std::string &roomTopic,
                                       bool isSpace);

private:
  /**
   * @brief Resolved catalog view of a scope (space or standalone room, D-01).
   */
  struct ScopeInfo {
    bool        found = false;            ///< The id is a live catalog record
    bool        isSpace = false;          ///< Space (true) vs room (false)
    bool        isStandaloneRoom = false; ///< Room with an empty parent
    std::string creator;                  ///< D-03 Super Admin (empty on legacy)
    bool        membersCanInvite = false; ///< MEMB-05 invite gate
  };

  /**
   * @brief Build the member key: "gcs/members/" + scopeId + "/" + address.
   */
  static std::string MemberKey(const std::string &scopeId,
                               const std::string &address);

  /**
   * @brief Build the invite key: "gcs/invites/" + scopeId + "/" + inviteId.
   */
  static std::string InviteKey(const std::string &scopeId,
                               const std::string &inviteId);

  /**
   * @brief The room chat topic for a room id: "gcs/chat/" + roomId.
   */
  static std::string RoomTopicFor(const std::string &roomId);

  /**
   * @brief Extract the scope id from a member key (logical or raw form).
   *
   * @param[in] key The member key as delivered to the CRDT callback.
   * @return The scope id; empty when the key carries no member prefix.
   */
  static std::string ScopeIdFromKey(const std::string &key);

  /**
   * @brief Mint the next process-unique invite id (CR-01-safe idiom).
   *
   * Prefix "invite-" + per-process seed (wall-clock ms + random_device token)
   * + in-process counter — the same idiom as EntityStore's NextEntityId.
   */
  static std::string NextInviteId();

  /// @brief Whether the D-06 key-wrap seam is enabled AND both callables set.
  bool KeyWrapEnabled() const;

  /// @brief Whether the invite-authenticator seam is enabled AND both callables
  ///        set (WR-04 — every invite is bearer-gated when this holds).
  bool InviteAuthEnabled() const;

  /// @brief Whether the issuer-signature seam is enabled AND both callables set.
  ///
  /// Invite minting and redeeming REQUIRE this (fail closed): without it no
  /// record can be signed at mint and no record can be attributed at redeem,
  /// so both operations are rejected rather than silently accepting an
  /// unsigned record.
  bool IssuerAuthEnabled() const;

  /// @brief Whether the local sender may mint an invite (MEMB-05/D-08).
  bool CanInvite(const std::string &scopeId) const;

  /// @brief Whether @p address may mint an invite in @p scopeId (MEMB-05/D-08).
  ///
  /// Resolves the ADDRESS's stored role exactly as LocalRoleFor resolves the
  /// local sender's (tombstone/absent -> ROLE_GUEST), then applies the
  /// unchanged creator/admin/member-with-gate rule. Used at redeem to check a
  /// third-party issuer's authority.
  ///
  /// @param[in] scopeId Scope id.
  /// @param[in] address Candidate issuer wallet address.
  /// @return true only when the address may mint an invite in the scope.
  bool CanInviteFor(const std::string &scopeId,
                    const std::string &address) const;

  /// @brief Whether the local sender may apply the requested role change.
  ///
  /// Grants only a concrete role (Admin, Moderator, Member, or Guest — never
  /// ROLE_UNSPECIFIED and never a new ROLE_SUPER_ADMIN, the D-03 root created
  /// with its scope), and the author must strictly outrank the target's
  /// CURRENT stored role, mirroring CanRemoveMember: a Moderator cannot alter
  /// an Admin or another Moderator, and an Admin cannot alter a non-creator
  /// Super Admin. The scope creator is the documented exception — as the D-03
  /// authority root it may amend any other member, and the D-07/WR-03
  /// dormancy rule keeps a peer Super Admin demotion dormant until approval.
  /// The exception is resolved from the catalog by identity
  /// (IsSuperAdmin(scopeId, m_sender)), never from the record's stored role,
  /// so a record that merely claims SUPER_ADMIN receives no creator authority
  /// (WR-02).
  bool CanChangeRole(const std::string &scopeId, const std::string &target,
                     chat::Role role) const;

  /// @brief Whether the local sender may remove the target member.
  ///
  /// Requires the author to hold a destructive tier AND to strictly outrank
  /// the target's current role (WR-02/D-08); the creator is never removable.
  bool CanRemoveMember(const std::string &scopeId,
                       const std::string &target) const;

  /// @brief Whether the local sender may delete the scope (Super Admin/Admin).
  bool CanDelete(const std::string &scopeId) const;

  /// @brief Whether the local sender may activate a dormant op (Super Admin).
  bool CanApprove(const std::string &scopeId) const;

  /**
   * @brief Whether @p address is the scope's creator (D-03 Super Admin).
   *
   * @param[in] scopeId Scope id.
   * @param[in] address Candidate wallet address.
   * @return true only when the resolved creator is non-empty and equal.
   */
  bool IsSuperAdmin(const std::string &scopeId,
                    const std::string &address) const;

  /**
   * @brief Count live members with SUPER_ADMIN or ADMIN role (D-07 "2+ admins").
   *
   * Fails closed: the count gates the D-07 dormancy decision, so a failed member
   * scan must abort the destructive op rather than read as zero admins (which
   * would take the immediate-destructive branch). Only VERIFIED live records
   * are counted — a forged admin record cannot change the dormancy decision.
   * Unparseable, body/key-mismatched, unverifiable, or tombstoned records are
   * skipped.
   *
   * @param[in] scopeId Scope id.
   * @return outcome::success with the admin-tier count; Error::GcsDbError when
   *         the member scan fails.
   */
  outcome::result<unsigned int> AdminCount(const std::string &scopeId) const;

  /**
   * @brief Resolve a scope id against the entity catalog (Spaces(), Rooms()).
   */
  ScopeInfo ResolveScope(const std::string &scopeId) const;

  /**
   * @brief Read a member record, returning it only when it verifies.
   *
   * Depth-0 wrapper over TryReadMemberVerified. An unverifiable record is not
   * a member, so every authorization decision that resolves through this read
   * (LocalRoleFor, HasExplicitLeave, CanInviteFor, CanChangeRole,
   * CanRemoveMember, and the command bodies) trusts only authenticated
   * authority.
   *
   * @param[in]  scopeId Scope id.
   * @param[in]  address Member wallet address.
   * @param[out] out     Verified record when found.
   * @return true only when an authentic, authorized record exists (a verified
   *         tombstoned record still returns true — callers skip it).
   */
  bool TryReadMember(const std::string &scopeId, const std::string &address,
                     chat::MemberRecord &out) const;

  /**
   * @brief Maximum signer-chain recursion depth for member authority checks.
   *
   * Bounds the third-party signer recursion so a cycle of mutually signed
   * records terminates (T-04-19-06).
   */
  static constexpr std::size_t kMaxMemberChainDepth = 4;

  /**
   * @brief A node-local snapshot of the last verified authority write.
   *
   * The cache is an in-memory, non-replicated read gate — never a merge rule.
   * It detects a replayed or mis-declared predecessor on a node that has
   * already verified the newer state. A cold start has no history and falls
   * back to the signed prior_role (documented residual T-04-19-09).
   */
  struct VerifiedMemberState
  {
    std::string signature;  ///< record_signature of the verified write
    int64_t     updatedAtMs = 0; ///< signed updated_at_ms of the verified write
    chat::Role  role = chat::ROLE_UNSPECIFIED; ///< verified role after the write
  };

  /**
   * @brief Verify the record's authority signature names its claimed signer.
   *
   * Requires IssuerAuthEnabled(), non-empty created_by and record_signature,
   * hex-decodes the signature (tolerating a leading "0x"), and accepts it only
   * when the injected seam verifies it for the CLAIMED issuer. Signature length
   * is enforced at the crypto seam (the FFI adapter and the multinode MakeSeam
   * check kSignatureBytes), never in core — the seam is the crypto boundary.
   *
   * @param[in] scopeId Scope the record belongs to (payload binding).
   * @param[in] address Wallet the record is stored under (payload binding).
   * @param[in] record  The member record whose signature is checked.
   * @return true only when a valid authority signature exists.
   */
  bool VerifyMemberSignature(const std::string &scopeId, const std::string &address,
                          const chat::MemberRecord &record) const;

  /**
   * @brief Verify the subject-signed explicit_leave attestation (D-02).
   *
   * explicit_leave == true requires a valid signature by @p address over the
   * canonical leave payload; explicit_leave == false requires an empty
   * leave_signature. A copied record cannot be flipped by another wallet.
   *
   * @param[in] scopeId Scope the record belongs to.
   * @param[in] address The subject wallet (the signature's required signer).
   * @param[in] record  The member record.
   * @return true only when the leave attestation is coherent and valid.
   */
  bool VerifyLeaveSignature(const std::string &scopeId, const std::string &address,
                            const chat::MemberRecord &record) const;

  /**
   * @brief Verify a member record against the full authority transition matrix.
   *
   * Enforces the common clauses (body/key binding, signature, leave
   * attestation, tombstone/pending coherence), the node-local predecessor gate,
   * the self-signed cap (GUEST for a non-creator, SUPER_ADMIN only for the
   * creator), the unconditional creator-target rejection, the creator-rooted
   * signer chain, the grant matrix, and strict outrank against the signed
   * prior_role. Every admin-tier destruction or dormant request requires the
   * resolved creator (D-03/D-07).
   *
   * @param[in] scopeId Scope the record belongs to.
   * @param[in] address Wallet the record is stored under.
   * @param[in] record  The record to verify.
   * @param[in] depth   Current signer-chain depth.
   * @return true only when the record is authentic and authorized.
   */
  bool VerifyMemberRecord(const std::string &scopeId, const std::string &address,
                          const chat::MemberRecord &record,
                          std::size_t depth) const;

  /**
   * @brief Resolve a wallet's verified role through the member chain.
   *
   * Reads the raw record, rejects a missing or tombstoned record, verifies it
   * at @p depth, and returns its role. Fails closed when the chain is too deep.
   *
   * @param[in] scopeId Scope id.
   * @param[in] address Wallet whose authority is wanted.
   * @param[in] depth   Current signer-chain depth.
   * @return The verified role; Error when the chain cannot be trusted.
   */
  outcome::result<chat::Role> VerifiedRoleFor(const std::string &scopeId,
                                              const std::string &address,
                                              std::size_t depth) const;

  /**
   * @brief Read and parse a member record without verifying it.
   *
   * @param[in]  scopeId Scope id.
   * @param[in]  address Member wallet address.
   * @param[out] out     Parsed record when found.
   * @return true when a parseable record exists (tombstoned records still
   *         return true — callers decide whether to skip them).
   */
  bool TryReadRawMember(const std::string &scopeId, const std::string &address,
                        chat::MemberRecord &out) const;

  /**
   * @brief Read a member record and return it only when it verifies.
   *
   * @param[in]  scopeId Scope id.
   * @param[in]  address Member wallet address.
   * @param[out] out     Verified record when found.
   * @param[in]  depth   Current signer-chain depth.
   * @return true only when an authentic, authorized record exists.
   */
  bool TryReadMemberVerified(const std::string &scopeId,
                             const std::string &address, chat::MemberRecord &out,
                             std::size_t depth) const;

  /**
   * @brief Full-state Put of an AUTHOR-signed member record.
   *
   * The acting wallet is the signer. Preserves created_at_ms from the stored
   * record, derives prior_role from the verified pre-write record (the ONE
   * place that sets it), stamps updated_at_ms monotonically
   * (max(NowMs(), verified predecessor + 1)) so consecutive authority writes
   * are strictly ordered, stamps created_by = the local sender, signs the
   * canonical member payload, and Puts. Fails closed without the issuer seam —
   * no unsigned authority record is ever persisted.
   *
   * @param[in] record Full desired member state.
   * @return outcome::success on a persisted record; propagated Error otherwise.
   */
  outcome::result<void> WriteMemberAsAuthor(chat::MemberRecord record);

  /**
   * @brief Full-state Put of a record whose authority already exists.
   *
   * Used only for non-authority updates (explicit_leave plus its subject
   * signature, or wrapped room-key refresh). Requires non-empty created_by and
   * record_signature; re-verifies the record and warns on failure but does NOT
   * fail the write (a cold node cannot resolve the creator, and failing would
   * block a legitimate leave/redeem). Never re-signs and never mutates any
   * authority field or the signed updated_at_ms, so the authority signature
   * stays valid on nodes that can resolve the scope; read paths remain the
   * authority gate.
   *
   * @param[in] record Full desired member state.
   * @return outcome::success on a persisted record; propagated Error otherwise.
   */
  outcome::result<void> WriteMemberPreservingAuthority(chat::MemberRecord record);

  /**
   * @brief Build and push the scope's MemberList via the sink (D-04).
   */
  outcome::result<void> PushMemberList(const std::string &scopeId);

  /**
   * @brief Build and push a flat SpaceTree over the catalog via the sink.
   */
  outcome::result<void> PushSpaceTree();

  CoreSession &m_session;    ///< Borrowed session (Put/Get/QueryKeyValues)
  EntityStore &m_entities;   ///< Borrowed entity catalog (scope/creator reads)
  std::string  m_sender;     ///< Injected local wallet address (D-03)
  EventSink    m_sink;       ///< Plaintext event sink
  KeyWrapSeam  m_keyWrap;    ///< D-06 injected seam (never called when disabled)
  ScanFn       m_memberScan; ///< AdminCount member-scan seam (empty = session)
  /// Node-local verified-predecessor cache (in-memory, non-replicated).
  mutable std::mutex m_verifiedStateMutex;
  /// Keyed by scopeId + "/" + address; see VerifiedMemberState.
  mutable std::map<std::string, VerifiedMemberState> m_verifiedState;
};

} // namespace gcs

#endif // GCS_MEMBERSHIP_HPP
