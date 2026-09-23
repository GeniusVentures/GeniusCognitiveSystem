/**
 * @file       gcs_messaging.hpp
 * @brief      Messaging component (Phase 3): send/receive/history over the
 *             CoreSession CRDT + raw GossipSub surface, with the D-08 injected
 *             crypto seam.
 * @details    SendMessage publishes the serialized ChatMessageState on the room
 *             topic (live full-value GossipSub, D-03) and archives it under
 *             gcs/messages/<room_topic>/<id> with {room_topic} (CRDT archive,
 *             D-02). When encryption is enabled the SAME
 *             nonce||ciphertext||tag envelope goes to BOTH the live publish and
 *             the archive Put, while the pushed events stay plaintext (D-08).
 *             Both receive routes — OnLiveMessage (raw GossipSub) and
 *             OnMessageArrived (CRDT heal) — funnel into ApplyMessage, which
 *             decrypts first (when enabled), parses, dedupes by message id
 *             (every message arrives twice), pushes a role-flipped event, and
 *             archives the received envelope without re-broadcast. QueryHistory
 *             decrypts each record before parse/sort and returns the room's
 *             messages sorted by (timestamp, id).
 *
 *             Messaging NEVER includes OpenSSL directly: encryption is behind
 *             the injected CryptoSeam (encrypt/decrypt callables + enabled
 *             flag). When the seam is disabled the callables are simply not
 *             called and payloads flow as plaintext — one component, two paths.
 * @copyright  (c) 2026 GNUS.AI
 */

#ifndef GCS_MESSAGING_HPP
#define GCS_MESSAGING_HPP

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>

#include "gcs_core.hpp"          // CoreSession + gcs::outcome alias
#include "proto/gcs_chat.pb.h"   // chat::GcsEvent / ChatMessageState / MessageHistory

namespace gcs {

/**
 * @brief C++-owned messaging component (send/receive/history) over the session.
 *
 * The constructor stores its dependencies only (deferred registration idiom —
 * no I/O). The sender address is injected by the FFI layer (the local wallet
 * address, D-04) and is display-only in this phase. The EventSink receives
 * plaintext GcsEvent messages only (Dart never sees ciphertext).
 */
class Messaging {
public:
  /// Push sink — receives plaintext GcsEvent envelopes.
  using EventSink = std::function<void(const chat::GcsEvent &)>;
  /// Encrypt callable of the D-08 seam (room topic + plaintext -> envelope).
  using EncryptFn = std::function<outcome::result<std::string>(
      const std::string &roomTopic, const std::string &plaintext)>;
  /// Decrypt callable of the D-08 seam (room topic + envelope -> plaintext).
  using DecryptFn = std::function<outcome::result<std::string>(
      const std::string &roomTopic, const std::string &envelope)>;

  /**
   * @brief D-08 injected crypto seam — injectable per Messaging instance.
   *
   * Production wires gcs::crypto::EncryptPayload / DecryptPayload; tests wire
   * the real adapter or counting stubs. When enabled is false (or either
   * callable is empty) the plaintext path runs and the callables are never
   * called.
   */
  struct CryptoSeam {
    EncryptFn encrypt; ///< Encrypt callable (empty = plaintext when disabled)
    DecryptFn decrypt; ///< Decrypt callable (empty = plaintext when disabled)
    bool      enabled; ///< Master switch for the D-08 path

    /// Default-constructs a disabled, empty seam (plaintext path).
    CryptoSeam() : encrypt(), decrypt(), enabled( false ) {}
  };

  /// CRDT archive key prefix (one serialized/enveloped record per message key).
  static constexpr const char *kMessagesKeyPrefix = "gcs/messages/";

  /**
   * @brief Construct the component, storing its dependencies only.
   *
   * Performs no I/O and no fallible work (deferred registration idiom).
   *
   * @param[in] session       The owning session; must outlive this component.
   * @param[in] senderAddress The local wallet address (D-04 authority stamp).
   * @param[in] sink          Plaintext event sink (Dart push plane).
   * @param[in] crypto        The injected D-08 seam (defaults to disabled).
   */
  explicit Messaging(CoreSession &session, std::string senderAddress,
                     EventSink sink, CryptoSeam crypto = {});

  /**
   * @brief Stop the archive worker and drain its queue.
   *
   * Signals the worker to stop, drains any queued archive writes, and joins the
   * thread. The borrowed session reference must still be alive when this
   * destructor runs (the worker may call Put on it while draining).
   */
  ~Messaging();

  /**
   * @brief Whether the encryption path is active for this instance.
   *
   * @return true when the seam is enabled AND both callables are non-empty.
   */
  bool EncryptionEnabled() const;

  /**
   * @brief Send a message: pending echo -> live publish + archive Put ->
   *        complete echo (D-03/D-07/D-08).
   *
   * Pushes a pending event, then (when encryption is enabled) encrypts the
   * serialized record ONCE and publishes the SAME envelope on both the live
   * room topic and the archive Put. On any encrypt/publish/archive failure it
   * pushes a terminal error-state event for the id and returns the error.
   *
   * @param[in] roomTopic Room topic to publish on and archive under.
   * @param[in] text      Message text (plaintext).
   * @return outcome::success on a fully published + archived message; otherwise
   *         the propagated error (after pushing the error-state event).
   */
  outcome::result<void> SendMessage(const std::string &roomTopic,
                                    const std::string &text);

  /**
   * @brief Handle a raw GossipSub live delivery (D-03 live path).
   *
   * @param[in] topic      The topic the message arrived on.
   * @param[in] valueBytes The full value bytes (envelope when encrypted).
   */
  void OnLiveMessage(const std::string &topic, const std::string &valueBytes);

  /**
   * @brief Handle a CRDT heal delivery (D-03 archive/heal path).
   *
   * Derives the room topic from the (possibly raw) archive key and funnels the
   * same bytes through ApplyMessage; the id-keyed dedupe makes the second
   * delivery of a message a no-op.
   *
   * @param[in] key        The archive key (logical or raw datastore form).
   * @param[in] valueBytes The full value bytes (envelope when encrypted).
   */
  void OnMessageArrived(const std::string &key, const std::string &valueBytes);

  /**
   * @brief Read and decrypt the room's converged history (D-01/D-06/D-08).
   *
   * Enumerates the room prefix, decrypts each record first (when enabled),
   * skips unparseable/undecryptable records with a warning, flips each
   * message's role to self/peer, sorts by (timestamp, id), and returns the
   * batch.
   *
   * @param[in] roomTopic Room topic to scan.
   * @return outcome::success with the sorted MessageHistory; propagated Error
   *         on a failed scan.
   */
  outcome::result<chat::MessageHistory> QueryHistory(const std::string &roomTopic);

private:
  /**
   * @brief Mint the next process-unique message id (D-04 authority stamp).
   *
   * Prefix "msg-" + per-process seed (wall-clock ms + random_device token) +
   * in-process counter — the same CR-01-safe idiom as the FFI NextMessageId.
   */
  std::string NextMessageId();

  /**
   * @brief Build the archive key for a message: "gcs/messages/" + topic + "/" + id.
   */
  static std::string BuildKey(const std::string &roomTopic, const std::string &id);

  /**
   * @brief Extract the room topic from a message archive key.
   *
   * Handles both the logical key (gcs/messages/<topic>/<id>, possibly with a
   * leading '/') and the raw datastore key (/<ns>/k/gcs/messages/<topic>/<id>/v).
   * Room topics may themselves contain '/', so only the last segment is dropped.
   *
   * @param[in] key The archive key.
   * @return The room topic; empty if the key carries no gcs/messages/ prefix.
   */
  static std::string RoomTopicFromKey(const std::string &key);

  /**
   * @brief The single receive funnel: decrypt (when enabled) -> parse -> dedupe
   *        -> push (role-flipped) -> archive the envelope as-received.
   *
   * Load-bearing: every message arrives twice (live + CRDT heal); the id-keyed
   * dedupe makes the second delivery a no-op. Decrypt/parse failures are
   * skip-and-log (same posture).
   *
   * @param[in] roomTopic  Room topic (derived by the caller).
   * @param[in] valueBytes Received value bytes (envelope when encrypted).
   */
  void ApplyMessage(const std::string &roomTopic, const std::string &valueBytes);

  /**
   * @brief Push a plaintext message event with the self/peer role flip applied.
   */
  void PushMessage(const chat::ChatMessageState &msg);

  /**
   * @brief Role for a message: USER_SELF iff its sender is the local sender.
   */
  static chat::MessageRole RoleFor(const chat::ChatMessageState &msg,
                                   const std::string &sender);

  /**
   * @brief Enqueue a received message's archive write for the worker thread.
   *
   * @param[in] key   Archive key (gcs/messages/<room>/<id>).
   * @param[in] value The envelope bytes as received (ciphertext when enabled).
   */
  void EnqueueArchive(const std::string &key, const std::string &value);

  /**
   * @brief Archive worker loop: drain the queue, writing each record via the
   *        session's local Put off the GossipSub/CRDT callback threads.
   *
   * Preserves per-message ordering (single worker) and exits once stopped with
   * an empty queue.
   */
  void ArchiveWorker();

  CoreSession &m_session; ///< Borrowed session (Put/Publish/QueryKeyValues pass-throughs)
  std::string  m_sender;  ///< Injected local wallet address (D-04)
  EventSink    m_sink;    ///< Plaintext event sink
  CryptoSeam   m_crypto;  ///< D-08 injected seam (never called when disabled)
  bool         m_encryptionEnabled; ///< crypto.enabled && both callables set
  std::mutex   m_seenMutex; ///< Guards the m_seenIds dedupe set
  std::unordered_set<std::string>
      m_seenIds; ///< Apply-once dedupe set (bounded by kMaxSeenIds)
  std::mutex m_archiveMutex; ///< Guards the archive queue + stop flag
  std::condition_variable m_archiveCond; ///< Wakes the archive worker
  std::queue<std::pair<std::string, std::string>>
      m_archiveQueue; ///< Pending archive writes
  std::thread m_archiveThread; ///< Dedicated archive worker
  bool m_archiveStopped = false; ///< Archive worker stop flag (guarded by m_archiveMutex)
};

} // namespace gcs

#endif // GCS_MESSAGING_HPP
