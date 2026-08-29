/**
 * @file       gcs_core.hpp
 * @brief      GCS core session — the C++-owned session object exposed to the
 * FFI layer (D-01/D-04). Owns a sgns::neoswarm::storage::GcsGlobalDb instance
 * and provides lifecycle + CRDT pass-through accessors.
 * @details    Phase 1 scope is intentionally minimal: init, shutdown,
 * isRunning, plus pass-throughs for the four CRDT ops on GcsGlobalDb. Message
 * logic, rooms, and spaces land in Phases 2-3.
 * @copyright  (c) 2026 GNUS.AI
 */

#ifndef GCS_CORE_HPP
#define GCS_CORE_HPP

#include <memory>
#include <string>

#include "gcs_storage/gcs_global_db.hpp" // via gcs_storage PUBLIC include dir (src/lib/)

namespace gcs {
// Error domain alias — gcs_core reuses the sgns::gcs error domain vendored in
// gcs_storage/common (bridged into GcsGlobalDb's namespace).
namespace outcome = libp2p::outcome;

/**
 * @brief Core session owning the GCS storage component.
 *
 * Lifecycle mirrors GcsGlobalDb (D-13): the constructor stores config only
 * (deferred registration — no I/O), Initialize() performs the fallible work,
 * Shutdown() is idempotent. Copy and move are deleted so the FFI layer
 * cannot double-free the underlying handle (T-01-02-03 mitigation).
 */
class CoreSession {
public:
  /**
   * @brief Session configuration.
   *
   * Aggregate-initializable; designed for designated initializers.
   */
  struct Config {
    std::string m_dbPath; ///< RocksDB path for the GCS CRDT store
  };

  /**
   * @brief Construct the session, storing the config only.
   *
   * Performs no I/O and no fallible work (deferred registration idiom).
   *
   * @param[in] config Session configuration.
   */
  explicit CoreSession(Config config);

  /**
   * @brief Destructor — calls Shutdown() if running; never throws.
   */
  ~CoreSession();

  CoreSession(const CoreSession &) = delete;
  CoreSession &operator=(const CoreSession &) = delete;
  CoreSession(CoreSession &&) = delete;
  CoreSession &operator=(CoreSession &&) = delete;

  /**
   * @brief Production init — delegates to GcsGlobalDb::Initialize()
   *        (acquires the shared pubsub and the node's graphsync Network via
   *        GeniusSDKGetNode()).
   *
   * @return outcome::success on a fully wired session; otherwise the
   *         propagated Error code (Error::SdkNotInitialized,
   *         Error::GcsDbError).
   */
  outcome::result<void> Initialize();

  /**
   * @brief Injected pubsub + graphsync Network init — test seam (Tier 2
   *        fixture pattern).
   *
   * @param[in] pubsub A started GossipPubSub whose lifetime outlives this
   * session.
   * @param[in] graphsyncNetwork A graphsync Network on pubsub's host whose
   * lifetime outlives this session; borrowed, never constructed or stopped.
   * @return outcome::success or the propagated Error code.
   */
  outcome::result<void> Initialize(
      std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> pubsub,
      std::shared_ptr<sgns::ipfs_lite::ipfs::graphsync::Network>
          graphsyncNetwork);

  /**
   * @brief Idempotent shutdown — delegates to GcsGlobalDb::Shutdown().
   *
   * Returns void (matches GcsGlobalDb::Shutdown()); safe to call when not
   * initialized.
   */
  void Shutdown() noexcept;

  /**
   * @brief Whether the underlying storage component is running.
   *
   * @return true if the session has a running GlobalDB + io thread.
   */
  bool IsRunning() const noexcept;

  /**
   * @brief Register a broadcast topic on the CRDT store.
   *
   * @param[in] topicName Topic identifier to broadcast on.
   * @return outcome::success on success; propagated Error otherwise.
   */
  outcome::result<void> AddBroadcastTopic(const std::string &topicName);

  /**
   * @brief Register a listen topic on the CRDT store.
   *
   * @param[in] topicName Topic identifier to subscribe to.
   * @return outcome::success on success; propagated Error otherwise.
   */
  outcome::result<void> AddListenTopic(const std::string &topicName);

  /**
   * @brief Put a key/value pair into the CRDT store.
   *
   * @param[in] key   Hierarchical key path.
   * @param[in] value UTF-8 payload bytes.
   * @return outcome::success on success; propagated Error otherwise.
   */
  outcome::result<void> Put(const std::string &key, const std::string &value);

  /**
   * @brief Get a value from the CRDT store.
   *
   * @param[in] key Hierarchical key path.
   * @return outcome::success with the value on success; propagated Error
   * otherwise.
   */
  outcome::result<std::string> Get(const std::string &key);

private:
  Config m_config; ///< Session configuration
  std::unique_ptr<sgns::neoswarm::storage::GcsGlobalDb>
      m_db; ///< Owned storage component
};
} // namespace gcs

#endif // GCS_CORE_HPP
