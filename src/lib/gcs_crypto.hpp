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

} // namespace gcs::crypto

#endif // GCS_CRYPTO_HPP
