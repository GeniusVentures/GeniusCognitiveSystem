/**
 * @file       gcs_core.cpp
 * @brief      Implementation of gcs::CoreSession — the C++-owned session object
 * exposed to the FFI layer (D-01/D-04).
 * @details    CoreSession owns a sgns::neoswarm::storage::GcsGlobalDb instance
 * and delegates lifecycle + CRDT operations to it. Thin pass-through only;
 * GcsGlobalDb owns all state and performs all logging/error mapping.
 * @copyright  (c) 2026 GNUS.AI
 */

#include "gcs_core.hpp"

namespace gcs {

CoreSession::CoreSession(Config config)
    : m_config(std::move(config)) {
  // An explicitly empty path keeps GcsGlobalDb's documented default
  // (kDefaultDbPath): overriding the default member initializer with "" here
  // would hand RocksDB an empty path and fail gcs_init on configs that omit
  // db_path (proto3 optional field).
  sgns::neoswarm::storage::GcsGlobalDb::Config dbConfig{};
  if (!m_config.m_dbPath.empty()) {
    dbConfig.m_dbPath = m_config.m_dbPath;
  }
  m_db = std::make_unique<sgns::neoswarm::storage::GcsGlobalDb>(
      std::move(dbConfig));
}

CoreSession::~CoreSession() {
  if (IsRunning()) {
    Shutdown();
  }
}

outcome::result<void> CoreSession::Initialize() { return m_db->Initialize(); }

outcome::result<void> CoreSession::Initialize(
    std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> pubsub,
    std::shared_ptr<sgns::ipfs_lite::ipfs::graphsync::Network>
        graphsyncNetwork) {
  return m_db->Initialize(std::move(pubsub), std::move(graphsyncNetwork));
}

void CoreSession::Shutdown() noexcept {
  if (IsRunning()) {
    m_db->Shutdown();
  }
}

bool CoreSession::IsRunning() const noexcept { return m_db->IsRunning(); }

outcome::result<void>
CoreSession::AddBroadcastTopic(const std::string &topicName) {
  return m_db->AddBroadcastTopic(topicName);
}

outcome::result<void>
CoreSession::AddListenTopic(const std::string &topicName) {
  return m_db->AddListenTopic(topicName);
}

outcome::result<void> CoreSession::Put(const std::string &key,
                                       const std::string &value) {
  return m_db->Put(key, value);
}

outcome::result<std::string> CoreSession::Get(const std::string &key) {
  return m_db->Get(key);
}

outcome::result<void>
CoreSession::Put(const std::string &key, const std::string &value,
                 const std::unordered_set<std::string> &topics) {
  return m_db->Put(key, value, topics);
}

outcome::result<void> CoreSession::PutLocal(const std::string &key,
                                            const std::string &value,
                                            const std::string &id) {
  return m_db->PutLocal(key, value, id);
}

outcome::result<std::vector<std::pair<std::string, std::string>>>
CoreSession::QueryKeyValues(const std::string &keyPrefix) {
  return m_db->QueryKeyValues(keyPrefix);
}

outcome::result<void> CoreSession::RegisterNewElementCallback(
    const std::string &pattern,
    std::function<void(const std::string &key, const std::string &value)>
        callback) {
  return m_db->RegisterNewElementCallback(pattern, std::move(callback));
}

outcome::result<void> CoreSession::Publish(const std::string &topic,
                                           const std::string &data) {
  return m_db->Publish(topic, data);
}

outcome::result<void> CoreSession::Subscribe(
    const std::string &topic,
    std::function<void(const std::string &topic, const std::string &data)>
        callback) {
  return m_db->Subscribe(topic, std::move(callback));
}

} // namespace gcs
