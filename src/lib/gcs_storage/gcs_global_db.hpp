/**
 * @file       gcs_global_db.hpp
 * @brief      NEO-SWARM-owned sgns::crdt::GlobalDB component (Phase 3,
 * D-01..D-04). Owns the GCS CRDT lifecycle: pulls the shared GossipPubSub and
 * the BORROWED graphsync Network from the in-process GeniusSDK (D-15/D-16a),
 * constructs io_context/scheduler/request-id generator locally (D-17,
 * amended 2026-08-27: a libp2p host has ONE protocol-handler slot per
 * protocol, so a second graphsync Network on the node's host would silently
 * replace the node's registration — the Network is borrowed via
 * GeniusNode::GetGraphsyncNetwork(), never constructed here), wires the
 * gcs-reputation topic (D-07), and exposes an init-style lifecycle (D-13).
 * @date       2026-08-10
 */

#ifndef GCS_STORAGE_GCS_GLOBAL_DB_HPP
#define GCS_STORAGE_GCS_GLOBAL_DB_HPP

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/asio/io_context.hpp>

#include "common/error.hpp"
#include "common/logging.hpp"

// ---------------------------------------------------------------------------
// Forward declarations — heavy SuperGenius/libp2p headers stay in the .cpp.
// ---------------------------------------------------------------------------
namespace sgns::crdt {
class GlobalDB;
}

namespace sgns::ipfs_pubsub {
class GossipPubSub;
}

namespace sgns::ipfs_lite::ipfs::graphsync {
class Network;
class RequestIdGenerator;
} // namespace sgns::ipfs_lite::ipfs::graphsync

namespace libp2p::basic {
class Scheduler;
}

namespace sgns::neoswarm::storage {
// GCS-owned common domain (src/lib/gcs_storage/common/) — aliased here so the
// moved class keeps its unqualified Error / Logger / outcome references.
namespace outcome = libp2p::outcome;
using sgns::gcs::CreateLogger;
using sgns::gcs::Error;
using sgns::gcs::Logger;

/**
 * @brief GCS GlobalDB component — single owner of the NEO-SWARM CRDT store.
 *
 * Lifecycle:
 *  - Constructor stores config only (no fallible work, no I/O) — D-13.
 *  - Initialize() runs the 7-step init chain (acquire pubsub + graphsync
 * Network via GeniusSDKGetNode, build local io/scheduler/generator, GlobalDB::New
 * with nullptr datastore, Start, wire gcs-reputation listen+broadcast topic,
 * spawn io thread).
 *  - Shutdown() is idempotent and joins the io thread.
 *
 * Error mapping (D-14):
 *  - GeniusSDKGetNode() == nullptr                -> Error::SdkNotInitialized
 *  - GlobalDB::New / AddBroadcastTopic failures    -> Error::GcsDbError
 *  - Double Initialize()                           -> Error::GcsDbError
 * (programmer error)
 */
class GcsGlobalDb {
public:
  /**
   * @brief Component configuration.
   *
   * Aggregate-initializable; designed for designated initializers.
   */
  struct Config {
    /// Default database directory, relative to the process working directory
    /// (D-02).
    static constexpr const char *kDefaultDbPath = "./gcs.db";
    /// Dedicated CRDT topic for reputation convergence across the swarm (D-07).
    static constexpr const char *kReputationTopic = "gcs-reputation";

    std::string m_dbPath =
        kDefaultDbPath; ///< RocksDB path for the GCS CRDT store
  };

  /**
   * @brief Construct the component, storing the config only.
   *
   * Performs no I/O, no network activity, and no fallible work — D-13.
   *
   * @param[in] cfg Configuration; defaults to ./gcs.db.
   */
  explicit GcsGlobalDb(Config cfg) noexcept;

  /**
   * @brief Destructor — calls Shutdown() if running; never throws.
   */
  ~GcsGlobalDb();

  GcsGlobalDb(const GcsGlobalDb &) = delete;
  GcsGlobalDb &operator=(const GcsGlobalDb &) = delete;
  GcsGlobalDb(GcsGlobalDb &&) = delete;
  GcsGlobalDb &operator=(GcsGlobalDb &&) = delete;

  /**
   * @brief Production init — acquires the shared pubsub AND the node's
   *        graphsync Network via GeniusSDKGetNode() and delegates to the
   *        injected overload.
   *
   * @return outcome::success on a fully wired, started GlobalDB; otherwise:
   *         Error::SdkNotInitialized (GeniusSDK init chain has not run — D-20
   * ordering — or the node's pubsub/graphsync Network is not initialized),
   *         Error::GcsDbError       (GlobalDB::New / AddBroadcastTopic /
   * double-init).
   */
  outcome::result<void> Initialize();

  /**
   * @brief Injected pubsub + graphsync Network init — test seam (Tier 2
   *        fixture pattern).
   *
   * Runs the full init chain (steps 3-7) against the supplied pubsub and
   * Network without consulting GeniusSDK. Production callers should prefer the
   * no-arg overload; this overload exists so unit tests can stand up a real
   * GossipPubSub on port 0 without bringing the entire GeniusNode online (per
   * RESEARCH Q-01). The Network is stored as-injected — never constructed —
   * because a libp2p host keeps one protocol-handler slot per protocol and a
   * second Network would silently replace the first registration.
   *
   * @param[in] pubsub A started GossipPubSub whose lifetime outlives this
   * component.
   * @param[in] graphsyncNetwork A graphsync Network on pubsub's host (or the
   * node's, in production) whose lifetime outlives this component; this
   * component borrows it and never stops it.
   * @return outcome::success or a specific Error code (see Initialize()).
   */
  outcome::result<void> Initialize(
      std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> pubsub,
      std::shared_ptr<sgns::ipfs_lite::ipfs::graphsync::Network>
          graphsyncNetwork);

  /**
   * @brief Idempotent shutdown — quiesces the GlobalDB, stops the io_context,
   *        and joins the io thread. Safe to call when not initialized.
   */
  void Shutdown() noexcept;

  /**
   * @brief Whether the component has a running GlobalDB + io thread.
   */
  bool IsRunning() const noexcept;

  /**
   * @brief The graphsync Network backing this component's GlobalDB.
   *
   * Test/inspection accessor: pointer-equality against the injector's (or the
   * node's) instance proves the Network is borrowed, never re-constructed —
   * a libp2p host has one protocol-handler slot per protocol, so a locally
   * constructed Network would silently replace the existing registration.
   *
   * @return The borrowed Network; nullptr before Initialize() succeeds.
   */
  std::shared_ptr<sgns::ipfs_lite::ipfs::graphsync::Network>
  GraphsyncNetwork() const noexcept;

  /**
   * @brief Register a broadcast topic on the underlying GlobalDB.
   *
   * Wraps sgns::crdt::GlobalDB::AddBroadcastTopic.
   *
   * @param[in] topicName Topic identifier to broadcast on.
   * @return outcome::success on success; Error::GcsDbError if not running or
   * the underlying call fails.
   */
  outcome::result<void> AddBroadcastTopic(const std::string &topicName);

  /**
   * @brief Register a listen topic on the underlying GlobalDB.
   *
   * Wraps sgns::crdt::GlobalDB::AddListenTopic.
   *
   * @param[in] topicName Topic identifier to subscribe to.
   * @return outcome::success on success; Error::GcsDbError if not running or
   * the underlying call fails.
   */
  outcome::result<void> AddListenTopic(const std::string &topicName);

  /**
   * @brief Put a key/value pair into the CRDT store.
   *
   * Wraps sgns::crdt::GlobalDB::Put; the std::string key/value are converted to
   * HierarchicalKey and Buffer at the call site.
   *
   * @param[in] key   Hierarchical key path.
   * @param[in] value UTF-8 payload bytes.
   * @return outcome::success on success; Error::GcsDbError if not running or
   * the underlying call fails.
   */
  outcome::result<void> Put(const std::string &key, const std::string &value);

  /**
   * @brief Get a value from the CRDT store.
   *
   * Wraps sgns::crdt::GlobalDB::Get; converts the returned Buffer to
   * std::string.
   *
   * @param[in] key Hierarchical key path.
   * @return outcome::success with the value on success; Error::GcsDbError if
   * not running or the underlying call fails.
   */
  outcome::result<std::string> Get(const std::string &key);

  /**
   * @brief Put a key/value pair into the CRDT store, replicating on the given
   *        broadcast topics (D-02 topics-aware write).
   *
   * Wraps sgns::crdt::GlobalDB::Put with the 3-arg (topics) overload; the
   * std::string key/value are converted to HierarchicalKey and Buffer at the
   * call site.
   *
   * @param[in] key    Hierarchical key path.
   * @param[in] value  UTF-8 payload bytes.
   * @param[in] topics Broadcast topics the write replicates on.
   * @return outcome::success on success; Error::GcsDbError if not running or
   * the underlying call fails.
   */
  outcome::result<void> Put(const std::string &key, const std::string &value,
                            const std::unordered_set<std::string> &topics);

  /**
   * @brief Put a key/value pair into local storage, bypassing DAG broadcast
   *        (D-03 receive-path no-rebroadcast write).
   *
   * Wraps sgns::crdt::GlobalDB::PutLocal.
   *
   * @param[in] key   Hierarchical key path.
   * @param[in] value UTF-8 payload bytes.
   * @param[in] id    Provenance/tie-break identifier for the local write.
   * @return outcome::success on success; Error::GcsDbError if not running or
   * the underlying call fails.
   */
  outcome::result<void> PutLocal(const std::string &key,
                                 const std::string &value,
                                 const std::string &id);

  /**
   * @brief Enumerate key/value pairs under a key prefix (D-01 converged prefix
   *        scan).
   *
   * Wraps sgns::crdt::GlobalDB::QueryKeyValues; converts each returned
   * Buffer key/value to a size-explicit std::string (binary-safe for the D-08
   * envelope's embedded NUL bytes).
   *
   * @param[in] keyPrefix Prefix to scan.
   * @return outcome::success with the matching (key, value) pairs on success;
   * Error::GcsDbError if not running or the underlying call fails.
   */
  outcome::result<std::vector<std::pair<std::string, std::string>>>
  QueryKeyValues(const std::string &keyPrefix);

  /**
   * @brief Register a callback fired when a new element matching the pattern
   *        converges into the CRDT store (D-03 heal path).
   *
   * Wraps sgns::crdt::GlobalDB::RegisterNewElementCallback; adapts the
   * SuperGenius (key, Buffer, cid) callback down to (key, value std::string).
   *
   * @param[in] pattern  Regex matched against the key string.
   * @param[in] callback (key, value) callback invoked on the CRDT worker thread.
   * @return outcome::success on success; Error::GcsDbError if not running or
   * registration is rejected.
   */
  outcome::result<void> RegisterNewElementCallback(
      const std::string &pattern,
      std::function<void(const std::string &key, const std::string &value)>
          callback);

  /**
   * @brief Publish a full-value payload on a raw GossipSub topic (D-03 live
   *        fast path).
   *
   * Wraps sgns::ipfs_pubsub::GossipPubSub::Publish — NOT the CRDT CID
   * broadcast.
   *
   * @param[in] topic Topic to publish on.
   * @param[in] data  Payload bytes.
   * @return outcome::success on success; Error::SdkNotInitialized if the
   * pubsub was not retained, Error::GcsDbError if not running or the
   * underlying call fails.
   */
  outcome::result<void> Publish(const std::string &topic,
                                const std::string &data);

  /**
   * @brief Subscribe a raw handler on a GossipSub topic (D-03 live fast path).
   *
   * Wraps sgns::ipfs_pubsub::GossipPubSub::Subscribe; the adapter drops the
   * empty end-of-stream optional and forwards (topic, data) as std::string.
   * Blocks on the returned future so the subscription is active before
   * returning.
   *
   * @param[in] topic    Topic to subscribe to.
   * @param[in] callback (topic, data) callback invoked on the GossipSub strand.
   * @return outcome::success on success; Error::SdkNotInitialized if the
   * pubsub was not retained, Error::GcsDbError if not running or the future
   * resolves to a null subscription.
   */
  outcome::result<void> Subscribe(
      const std::string &topic,
      std::function<void(const std::string &topic, const std::string &data)>
          callback);

private:
  Config m_cfg; ///< Component configuration
  std::shared_ptr<boost::asio::io_context>
      m_io; ///< Dedicated io_context (D-17)
  std::shared_ptr<libp2p::basic::Scheduler>
      m_scheduler; ///< libp2p scheduler over m_io
  std::shared_ptr<sgns::ipfs_lite::ipfs::graphsync::Network>
      m_graphsyncNetwork; ///< graphsync network
  std::shared_ptr<sgns::ipfs_lite::ipfs::graphsync::RequestIdGenerator>
      m_generator; ///< request-id generator
  std::shared_ptr<sgns::crdt::GlobalDB>
      m_db;               ///< Owned GlobalDB (nullptr until Initialize)
  std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub>
      m_pubsub; ///< Shared GossipPubSub — retained for the raw live path (D-03)
  std::thread m_ioThread; ///< io->run() worker thread
  std::atomic<bool> m_running{false}; ///< Lifecycle flag
  Logger m_logger;                    ///< spdlog component logger
};

} // namespace sgns::neoswarm::storage

#endif // GCS_STORAGE_GCS_GLOBAL_DB_HPP
