---
title: sgns::neoswarm::network

---

# sgns::neoswarm::network





## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::network::P2PNode](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/)** <br/>Manages a libp2p host for swarm task broadcasting and CRDT sync.  |
| class | **[sgns::neoswarm::network::ResultAggregation](/source-reference/Classes/d8/d65/classsgns_1_1neoswarm_1_1network_1_1_result_aggregation/)** <br/>Collects [NodeOutput](/source-reference/Classes/d7/d96/structsgns_1_1neoswarm_1_1_node_output/) responses from swarm peers with a timeout.  |
| class | **[sgns::neoswarm::network::SGChannelManager](/source-reference/Classes/d7/dce/classsgns_1_1neoswarm_1_1network_1_1_s_g_channel_manager/)** <br/>Manages a persistent gRPC channel to a SuperGenius node.  |
| class | **[sgns::neoswarm::network::SGJobSubmitter](/source-reference/Classes/de/d51/classsgns_1_1neoswarm_1_1network_1_1_s_g_job_submitter/)** <br/>Signs and publishes [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) messages to the SuperGenius grid channel.  |
| class | **[sgns::neoswarm::network::SGMessageAuthenticator](/source-reference/Classes/d0/d6c/classsgns_1_1neoswarm_1_1network_1_1_s_g_message_authenticator/)** <br/>Wraps NodeIdentity and MessageSigning for SuperGenius dispatch.  |
| struct | **[sgns::neoswarm::network::SGResultCollectorConfig](/source-reference/Classes/d1/dd3/structsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector_config/)**  |
| class | **[sgns::neoswarm::network::SGResultCollector](/source-reference/Classes/de/d02/classsgns_1_1neoswarm_1_1network_1_1_s_g_result_collector/)** <br/>Collects inference results from SuperGenius PubSub result channels.  |
| class | **[sgns::neoswarm::network::SGClient](/source-reference/Classes/d4/d9a/classsgns_1_1neoswarm_1_1network_1_1_s_g_client/)** <br/>Client that bridges GNUS NEO SWARM to the SuperGenius blockchain compute network via PubSub-based gRPC dispatch.  |






-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700