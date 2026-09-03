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
    : m_config(std::move(config)),
      m_db(std::make_unique<sgns::neoswarm::storage::GcsGlobalDb>(
          sgns::neoswarm::storage::GcsGlobalDb::Config{m_config.m_dbPath})) {}

CoreSession::~CoreSession() {
  if (IsRunning()) {
    Shutdown();
  }
}

outcome::result<void> CoreSession::Initialize() { return m_db->Initialize(); }

outcome::result<void> CoreSession::Initialize(
    std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> pubsub) {
  return m_db->Initialize(std::move(pubsub));
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

} // namespace gcs
