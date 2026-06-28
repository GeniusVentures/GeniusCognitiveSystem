---
title: sgns::neoswarm::network::P2PNode::Impl

---

# sgns::neoswarm::network::P2PNode::Impl





## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[GossipSubs](/source-reference/Classes/da/d36/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl_1_1_gossip_subs/)**  |

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[listen_addr_](/source-reference/Classes/d4/da2/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl/#variable-listen-addr-)**  |
| std::string | **[peer_m_id](/source-reference/Classes/d4/da2/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl/#variable-peer-m-id)**  |
| std::vector< std::string > | **[peers_](/source-reference/Classes/d4/da2/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl/#variable-peers-)**  |
| std::atomic< bool > | **[m_running](/source-reference/Classes/d4/da2/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl/#variable-m-running)**  |
| std::shared_ptr< libp2p::Host > | **[host_](/source-reference/Classes/d4/da2/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl/#variable-host-)**  |
| std::shared_ptr< libp2p::protocol::gossip::Gossip > | **[gossip_](/source-reference/Classes/d4/da2/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl/#variable-gossip-)**  |
| std::shared_ptr< libp2p::peer::IdentityManager > | **[id_mgr_](/source-reference/Classes/d4/da2/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl/#variable-id-mgr-)**  |
| std::unique_ptr< [GossipSubs](/source-reference/Classes/da/d36/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl_1_1_gossip_subs/) > | **[subs_](/source-reference/Classes/d4/da2/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl/#variable-subs-)**  |

## Public Attributes Documentation

### variable listen_addr_

```cpp
std::string listen_addr_;
```


### variable peer_m_id

```cpp
std::string peer_m_id;
```


### variable peers_

```cpp
std::vector< std::string > peers_;
```


### variable m_running

```cpp
std::atomic< bool > m_running { false };
```


### variable host_

```cpp
std::shared_ptr< libp2p::Host > host_;
```


### variable gossip_

```cpp
std::shared_ptr< libp2p::protocol::gossip::Gossip > gossip_;
```


### variable id_mgr_

```cpp
std::shared_ptr< libp2p::peer::IdentityManager > id_mgr_;
```


### variable subs_

```cpp
std::unique_ptr< GossipSubs > subs_;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700