---
title: GNUS-NEO-SWARM/src/network/sg_client

---

# GNUS-NEO-SWARM/src/network/sg_client





## Files

| Name           |
| -------------- |
| **[GNUS-NEO-SWARM/src/network/sg_client/sg_channel_manager.cpp](/source-reference/Files/da/dd8/sg__channel__manager_8cpp/#file-sg_channel_manager.cpp)** <br/>gRPC channel lifecycle implementation — TLS, keepalive, reconnect  |
| **[GNUS-NEO-SWARM/src/network/sg_client/sg_channel_manager.hpp](/source-reference/Files/d6/da5/sg__channel__manager_8hpp/#file-sg_channel_manager.hpp)** <br/>Manages gRPC channel lifecycle — create, keepalive, reconnect, health check.  |
| **[GNUS-NEO-SWARM/src/network/sg_client/sg_job_submitter.cpp](/source-reference/Files/d5/d97/sg__job__submitter_8cpp/#file-sg_job_submitter.cpp)** <br/>Publishes signed Task messages to the SuperGenius grid channel via PubSub.  |
| **[GNUS-NEO-SWARM/src/network/sg_client/sg_job_submitter.hpp](/source-reference/Files/d4/d72/sg__job__submitter_8hpp/#file-sg_job_submitter.hpp)** <br/>Publishes signed Task messages to the SuperGenius grid channel via PubSub.  |
| **[GNUS-NEO-SWARM/src/network/sg_client/sg_message_authenticator.cpp](/source-reference/Files/d6/d66/sg__message__authenticator_8cpp/#file-sg_message_authenticator.cpp)** <br/>Signs and verifies messages via hardened NodeIdentity + MessageSigning.  |
| **[GNUS-NEO-SWARM/src/network/sg_client/sg_message_authenticator.hpp](/source-reference/Files/d6/d2b/sg__message__authenticator_8hpp/#file-sg_message_authenticator.hpp)** <br/>Signs and verifies messages using the node's secp256k1 identity.  |
| **[GNUS-NEO-SWARM/src/network/sg_client/sg_result_collector.cpp](/source-reference/Files/d6/d9e/sg__result__collector_8cpp/#file-sg_result_collector.cpp)** <br/>Timeout-bounded result collection from SuperGenius PubSub result channels.  |
| **[GNUS-NEO-SWARM/src/network/sg_client/sg_result_collector.hpp](/source-reference/Files/d4/d81/sg__result__collector_8hpp/#file-sg_result_collector.hpp)** <br/>Subscribes to per-job result channels and collects TaskResult messages.  |
| **[GNUS-NEO-SWARM/src/network/sg_client/super_genius_client.cpp](/source-reference/Files/d9/db5/super__genius__client_8cpp/#file-super_genius_client.cpp)** <br/>Bridges GNUS NEO SWARM to SuperGenius via PubSub gRPC dispatch.  |
| **[GNUS-NEO-SWARM/src/network/sg_client/super_genius_client.hpp](/source-reference/Files/db/d7a/super__genius__client_8hpp/#file-super_genius_client.hpp)** <br/>Client for SuperGenius blockchain compute network dispatch via PubSub gRPC.  |






-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
