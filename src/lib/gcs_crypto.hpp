/**
 * @file       gcs_crypto.hpp
 * @brief      Stateless OpenSSL crypto adapter for Phase 3 messaging (D-08):
 *             per-room HKDF-SHA256 key derivation + AES-256-GCM
 *             encrypt/decrypt of full message records.
 * @details    This adapter is the ONLY file in src/ that includes OpenSSL
 *             headers (the `openssl/...` family). It is stateless except for
 *             ONE documented exception (D-06): a mutex-guarded per-room key
 *             store holds 32-byte room-key BYTES only, never EVP contexts
 *             (P7). Every call derives a fresh room key
 *             and constructs a fresh EVP_CIPHER_CTX / EVP_PKEY_CTX, because
 *             callers invoke it from three different thread types — the
 *             GossipSub strand thread (live receive), the CRDT DagWorker
 *             threads (heal receive), and the FFI command thread (send /
 *             history). OpenSSL context objects are mutable state machines and
 *             must never be shared across concurrent calls.
 *
 *             Envelope layout (one framing everywhere):
 *               stored/published value = nonce(12) || ciphertext || tag(16)
 *             where the ciphertext length equals the plaintext length (GCM has
 *             no padding). The nonce is 12 fresh RAND_bytes bytes per message.
 *
 *             Honest scope (D-08): the interim Phase 3 room key is
 *             HKDF-SHA256(ikm = room_topic), and the room topic string is
 *             PUBLIC (it is the GossipSub topic name and part of the CRDT key
 *             prefix). This delivers at-rest / casual-observer opacity, NOT
 *             member-robust end-to-end encryption — anyone who knows the topic
 *             can derive the key. Phase 4 membership swaps ONLY the key
 *             distribution; the envelope format does not change.
 * @copyright  (c) 2026 GNUS.AI
 */

#ifndef GCS_CRYPTO_HPP
#define GCS_CRYPTO_HPP

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include <libp2p/outcome/outcome.hpp>

namespace gcs::crypto {
namespace outcome = libp2p::outcome;

/// AES-GCM default IV length in bytes (12) — pinned in 03-API-SIGNATURES §5.6.
constexpr size_t kGcmNonceLength = 12;
/// AES-GCM default/maximum tag length in bytes (16) — pinned in 03-API-SIGNATURES §5.6.
constexpr size_t kGcmTagLength = 16;
/// AES-256 key length in bytes (32) — pinned in 03-API-SIGNATURES §5.6.
constexpr size_t kRoomKeyLengthBytes = 32;
/// Fixed, non-secret HKDF salt (domain separator) — pinned in 03-API-SIGNATURES §5.6.
constexpr const char kMessagesHkdfSalt[] = "gcs-messages-hkdf-v1";
/// Fixed HKDF info string (domain separator) — pinned in 03-API-SIGNATURES §5.6.
constexpr const char kMessagesHkdfInfo[] = "gcs-messages-v1";
/// Fixed HKDF salt (domain separator) for the invite-authenticator key (WR-04).
constexpr const char kInviteAuthHkdfSalt[] = "gcs-invite-auth-hkdf-v1";
/// Fixed HKDF info string (domain separator) for the invite-authenticator key.
constexpr const char kInviteAuthHkdfInfo[] = "gcs-invite-auth-v1";
/// Fixed, domain-separating prefix for the invite-authenticator marker digest.
constexpr const char kInviteAuthMarkerPrefix[] = "gcs-invite-auth-marker-v1";
/// Invite-authenticator marker length in bytes — the SHA-256 digest size; equal
/// to kRoomKeyLengthBytes so the marker seals with the existing 32-byte wrap.
constexpr size_t kInviteAuthMarkerLengthBytes = 32;

/**
 * @brief Mint a fresh random 32-byte room key (D-06).
 *
 * Draws kRoomKeyLengthBytes bytes from RAND_bytes with a checked return. This
 * is the per-room key minted at room creation and distributed to members
 * AES-256-GCM-wrapped under each member's invite key (never derived from the
 * public room topic).
 *
 * @return The 32-byte room key; an EMPTY vector when RAND_bytes fails (the
 *         documented empty-on-failure contract for this accessor).
 */
std::vector<unsigned char> MintRoomKey();

/**
 * @brief Store (or overwrite) a room's 32-byte key in the per-room key store
 *        (D-06 — the documented exception to this adapter's statelessness).
 *
 * The store holds key BYTES only, guarded by a std::mutex; no EVP context is
 * ever cached, so the fresh-context-per-call rule still holds for every
 * derive/encrypt/decrypt (P7).
 *
 * @param[in] roomTopic The room topic that addresses the key.
 * @param[in] roomKey   The 32-byte room key bytes.
 */
void StoreRoomKey(const std::string &roomTopic,
                  const std::vector<unsigned char> &roomKey);

/**
 * @brief Resolve a room topic to its stored 32-byte key (D-06).
 *
 * @param[in] roomTopic The room topic to look up.
 * @return The stored 32-byte key; std::nullopt when the topic has no stored key
 *         (pre-membership rooms), which makes DeriveRoomKey fall back to
 *         topic-derivation.
 */
std::optional<std::vector<unsigned char>> RoomKeyFor(const std::string &roomTopic);

/**
 * @brief Remove a room's key from the per-room key store (removal paths, D-06).
 *
 * Harmless when the topic is absent.
 *
 * @param[in] roomTopic The room topic to clear.
 */
void ClearRoomKey(const std::string &roomTopic);

/**
 * @brief Derive the per-room symmetric key (D-08 / D-06 / WR-02).
 *
 * room_key = HKDF-SHA256(ikm, salt = kMessagesHkdfSalt,
 *                        info = kMessagesHkdfInfo, L = kRoomKeyLengthBytes)
 * using the default EXTRACT_AND_EXPAND mode (no set_hkdf_mode call), where the
 * input keying material is the member-distributed 32-byte room key from the
 * per-room key store (RoomKeyFor) when present, else the room_topic UTF-8
 * string — the pre-membership fallback for rooms minted before the swap. The
 * salt and info are STABLE across the swap; only the ikm changes.
 *
 * A fresh EVP_PKEY_CTX is created and freed on every call; the derived key is
 * never cached, so the adapter stays thread-safe apart from the mutex-guarded
 * byte store.
 *
 * @param[in] roomTopic The room topic string (addressing the key store, and
 *                      the fallback HKDF input keying material).
 * @return outcome::success with the 32-byte room key; outcome::failure on any
 *         EVP/HKDF failure.
 */
outcome::result<std::vector<unsigned char>> DeriveRoomKey(const std::string &roomTopic);

/**
 * @brief Encrypt a full message record into a nonce-prefixed AES-256-GCM
 *        envelope (D-08).
 *
 * envelope = nonce(kGcmNonceLength, RAND_bytes) || ciphertext (== plaintext
 *            length) || tag(kGcmTagLength). The returned std::string is
 *            binary-safe (arbitrary bytes — never assume NUL termination).
 *
 * Stateless: a fresh EVP_CIPHER_CTX is created and freed on every call; the
 * nonce is 12 fresh RAND_bytes bytes per message.
 *
 * @param[in] roomTopic Room topic (drives the room key).
 * @param[in] plaintext Serialized message record bytes.
 * @return outcome::success with the binary envelope; outcome::failure on any
 *         EVP/RAND failure.
 */
outcome::result<std::string> EncryptPayload(const std::string &roomTopic,
                                            const std::string &plaintext);

/**
 * @brief Decrypt and authenticate an envelope back to the plaintext (D-08).
 *
 * Splits the envelope at fixed offsets (first kGcmNonceLength = nonce, last
 * kGcmTagLength = tag, middle = ciphertext), verifies the GCM tag BEFORE the
 * finalization step, and returns the plaintext (ciphertext length == plaintext
 * length for GCM). A wrong key, tampered byte, or truncated envelope makes the
 * tag verification / EVP_DecryptFinal_ex fail, which is reported as an
 * outcome::failure — the caller skip-and-logs (same posture as an unparseable
 * record).
 *
 * Stateless: a fresh EVP_CIPHER_CTX is created and freed on every call.
 *
 * @param[in] roomTopic Room topic (drives the room key).
 * @param[in] envelope  The nonce || ciphertext || tag envelope bytes.
 * @return outcome::success with the plaintext; outcome::failure on truncation,
 *         wrong key, tampering, or corruption.
 */
outcome::result<std::string> DecryptPayload(const std::string &roomTopic,
                                            const std::string &envelope);

/**
 * @brief Wrap a 32-byte room key under a member (invite) key, reusing the D-08
 *        AES-256-GCM envelope (D-06).
 *
 * envelope = nonce(kGcmNonceLength, RAND_bytes) || ciphertext (== room key
 *            length) || tag(kGcmTagLength). The returned std::string is
 *            binary-safe (arbitrary bytes — never assume NUL termination).
 *
 * Unlike EncryptPayload, the AES-256 key is the caller-supplied memberKey — NOT
 * HKDF(roomTopic); no DeriveRoomKey call is made. Both keys must be
 * kRoomKeyLengthBytes long; any other size is rejected up front with
 * outcome::failure (never passed to EVP).
 *
 * @param[in] memberKey The 32-byte member/invite key (the AES-256 key).
 * @param[in] roomKey   The 32-byte room key to wrap.
 * @return outcome::success with the binary envelope; outcome::failure on a
 *         wrong key size or any EVP/RAND failure.
 */
outcome::result<std::string> WrapRoomKey(const std::vector<unsigned char> &memberKey,
                                         const std::vector<unsigned char> &roomKey);

/**
 * @brief Unwrap (decrypt + authenticate) a wrapped room key with a member key
 *        (D-06).
 *
 * Splits the envelope at fixed offsets (first kGcmNonceLength = nonce, last
 * kGcmTagLength = tag, middle = the wrapped room key), verifies the GCM tag,
 * and returns the 32-byte room key. The ciphertext length must equal
 * kRoomKeyLengthBytes; the memberKey must be kRoomKeyLengthBytes long. A wrong
 * member key, tampered byte, or truncated envelope fails as outcome::failure.
 *
 * @param[in] memberKey The 32-byte member/invite key (the AES-256 key).
 * @param[in] envelope  The nonce || ciphertext || tag envelope bytes.
 * @return outcome::success with the 32-byte room key; outcome::failure on a
 *         wrong key size, truncation, wrong key, tampering, or corruption.
 */
outcome::result<std::vector<unsigned char>> UnwrapRoomKey(
    const std::vector<unsigned char> &memberKey, const std::string &envelope);

/**
 * @brief Compute the deterministic invite-authenticator marker (WR-04).
 *
 * The marker is a SHA-256 digest over the domain-separated pair
 * (`kInviteAuthMarkerPrefix`, scopeId, inviteId). Binding BOTH ids is
 * mandatory: an authenticator lifted from one invite must not verify another
 * (T-04-09-07).
 *
 * @param[in] scopeId  The invite's scope id.
 * @param[in] inviteId The invite id.
 * @return The kInviteAuthMarkerLengthBytes-byte marker; an EMPTY vector when
 *         the digest fails (documented empty-on-failure contract).
 */
std::vector<unsigned char> InviteAuthMarker(const std::string &scopeId,
                                            const std::string &inviteId);

/**
 * @brief Mint an invite authenticator bound to an invite (WR-04).
 *
 * Derives an auth key from @p inviteKey through HKDF-SHA256 with the
 * invite-domain salt/info (D-06/D-08 derivation discipline), then seals
 * InviteAuthMarker(scopeId, inviteId) with the module's existing 32-byte
 * AES-256-GCM wrap. The returned std::string is binary-safe.
 *
 * @param[in] inviteKey The 32-byte bearer/invite key from the URL.
 * @param[in] scopeId   The invite's scope id.
 * @param[in] inviteId  The invite id.
 * @return outcome::success with the binary authenticator envelope; Error on a
 *         wrong key size or any interior HKDF/wrap failure.
 */
outcome::result<std::string> MintInviteAuthenticator(
    const std::vector<unsigned char> &inviteKey, const std::string &scopeId,
    const std::string &inviteId);

/**
 * @brief Verify an invite authenticator against the bearer key (WR-04).
 *
 * Re-derives the auth key, unseals the authenticator, and constant-time
 * compares the result against InviteAuthMarker(scopeId, inviteId). Never
 * throws and never logs.
 *
 * @param[in] inviteKey     The bearer/invite key from the URL.
 * @param[in] scopeId       The invite's scope id.
 * @param[in] inviteId      The invite id.
 * @param[in] authenticator The stored authenticator envelope.
 * @return outcome::success(true) only on an exact match;
 *         outcome::success(false) for any mismatch or parse failure; Error
 *         only for an interior failure.
 */
outcome::result<bool> VerifyInviteAuthenticator(
    const std::vector<unsigned char> &inviteKey, const std::string &scopeId,
    const std::string &inviteId, const std::string &authenticator);

} // namespace gcs::crypto

#endif // GCS_CRYPTO_HPP
