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
#include <memory>
#include <string>
#include <thread>

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
  std::thread m_ioThread; ///< io->run() worker thread
  std::atomic<bool> m_running{false}; ///< Lifecycle flag
  Logger m_logger;                    ///< spdlog component logger
};

} // namespace sgns::neoswarm::storage

#endif // GCS_STORAGE_GCS_GLOBAL_DB_HPP
