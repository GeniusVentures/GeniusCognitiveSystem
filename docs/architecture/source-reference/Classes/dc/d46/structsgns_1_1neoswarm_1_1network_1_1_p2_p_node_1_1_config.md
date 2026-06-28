---
title: sgns::neoswarm::network::P2PNode::Config

---

# sgns::neoswarm::network::P2PNode::Config






`#include <p2p_node.hpp>`

## Public Attributes

|                | Name           |
| -------------- | -------------- |
| std::string | **[listen_addr_](/source-reference/Classes/dc/d46/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_config/#variable-listen-addr-)**  |
| std::string | **[bootstrap_peer_](/source-reference/Classes/dc/d46/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_config/#variable-bootstrap-peer-)** <br/>optional bootstrap peer multiaddr  |
| bool | **[enable_mdns_](/source-reference/Classes/dc/d46/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_config/#variable-enable-mdns-)** <br/>local peer discovery  |
| bool | **[enable_kademlia_](/source-reference/Classes/dc/d46/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_config/#variable-enable-kademlia-)**  |
| int | **[max_peers_](/source-reference/Classes/dc/d46/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_config/#variable-max-peers-)**  |

## Public Attributes Documentation

### variable listen_addr_

```cpp
std::string listen_addr_ = "/ip4/0.0.0.0/tcp/0";
```


### variable bootstrap_peer_

```cpp
std::string bootstrap_peer_ = "";
```

optional bootstrap peer multiaddr 

### variable enable_mdns_

```cpp
bool enable_mdns_ = true;
```

local peer discovery 

### variable enable_kademlia_

```cpp
bool enable_kademlia_ = true;
```


### variable max_peers_

```cpp
int max_peers_ = 50;
```


-------------------------------

Updated on 2026-06-28 at 13:58:22 -0700