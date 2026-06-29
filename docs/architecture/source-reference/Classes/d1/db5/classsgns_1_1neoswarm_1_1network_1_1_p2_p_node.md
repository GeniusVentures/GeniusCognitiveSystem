---
title: sgns::neoswarm::network::P2PNode
summary: Manages a libp2p host for swarm task broadcasting and CRDT sync. 

---

# sgns::neoswarm::network::P2PNode



Manages a libp2p host for swarm task broadcasting and CRDT sync.  [More...](#detailed-description)


`#include <p2p_node.hpp>`

## Public Classes

|                | Name           |
| -------------- | -------------- |
| struct | **[Config](/source-reference/Classes/dc/d46/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_config/)**  |
| struct | **[Impl](/source-reference/Classes/d4/da2/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_impl/)**  |

## Public Types

|                | Name           |
| -------------- | -------------- |
| using std::function< void(const [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) &task, const std::string &from_peer)> | **[TaskHandler](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#using-taskhandler)**  |
| using std::function< void(const std::string &crdt_data)> | **[CRDTHandler](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#using-crdthandler)**  |

## Public Functions

|                | Name           |
| -------------- | -------------- |
| | **[P2PNode](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-p2pnode)**(std::shared_ptr< [security::NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) > identity, [Config](/source-reference/Classes/dc/d46/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_config/) cfg) |
| | **[P2PNode](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-p2pnode)**(std::shared_ptr< [security::NodeIdentity](/source-reference/Classes/d6/d99/classsgns_1_1neoswarm_1_1security_1_1_node_identity/) > identity) |
| | **[~P2PNode](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-~p2pnode)**() |
| outcome::result< void > | **[Start](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-start)**()<br/>Start the libp2p host and begin listening.  |
| void | **[Stop](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-stop)**()<br/>Stop the host and disconnect all peers.  |
| bool | **[IsRunning](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-isrunning)**() const |
| std::string | **[ListenAddress](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-listenaddress)**() const |
| std::string | **[PeerId](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-peerid)**() const |
| void | **[OnTask](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-ontask)**([TaskHandler](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#using-taskhandler) handler)<br/>Register a handler for incoming task broadcasts.  |
| void | **[OnCRDT](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-oncrdt)**([CRDTHandler](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#using-crdthandler) handler)<br/>Register a handler for incoming CRDT sync messages.  |
| outcome::result< void > | **[BroadcastTask](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-broadcasttask)**(const [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) & task)<br/>Broadcast a task to all connected peers via GossipSub.  |
| outcome::result< void > | **[BroadcastCRDT](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-broadcastcrdt)**(const std::string & crdt_data)<br/>Broadcast a CRDT state update to all peers.  |
| std::vector< std::string > | **[ConnectedPeers](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-connectedpeers)**() const<br/>Get the list of currently connected peer IDs.  |

## Detailed Description

```cpp
class sgns::neoswarm::network::P2PNode;
```

Manages a libp2p host for swarm task broadcasting and CRDT sync. 

Uses Noise protocol for encryption and Yamux for stream multiplexing. Falls back to a local stub when libp2p is not compiled in. 

## Public Types Documentation

### using TaskHandler

```cpp
using sgns::neoswarm::network::P2PNode::TaskHandler = std::function<void( const Task& task, const std::string& from_peer )>;
```


### using CRDTHandler

```cpp
using sgns::neoswarm::network::P2PNode::CRDTHandler = std::function<void( const std::string& crdt_data )>;
```


## Public Functions Documentation

### function P2PNode

```cpp
P2PNode(
    std::shared_ptr< security::NodeIdentity > identity,
    Config cfg
)
```


### function P2PNode

```cpp
explicit P2PNode(
    std::shared_ptr< security::NodeIdentity > identity
)
```


### function ~P2PNode

```cpp
~P2PNode()
```


### function Start

```cpp
outcome::result< void > Start()
```

Start the libp2p host and begin listening. 

**Return**: outcome::success or NetworkError. 

### function Stop

```cpp
void Stop()
```

Stop the host and disconnect all peers. 

### function IsRunning

```cpp
inline bool IsRunning() const
```


**Return**: True if the node is currently running. 

### function ListenAddress

```cpp
std::string ListenAddress() const
```


**Return**: Our listen multiaddress (available after [Start()](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/#function-start)). 

### function PeerId

```cpp
std::string PeerId() const
```


**Return**: Our peer ID string. 

### function OnTask

```cpp
inline void OnTask(
    TaskHandler handler
)
```

Register a handler for incoming task broadcasts. 

**Parameters**: 

  * **handler** Callback invoked when a task is received from a peer. 


### function OnCRDT

```cpp
inline void OnCRDT(
    CRDTHandler handler
)
```

Register a handler for incoming CRDT sync messages. 

**Parameters**: 

  * **handler** Callback invoked when a CRDT update is received. 


### function BroadcastTask

```cpp
outcome::result< void > BroadcastTask(
    const Task & task
)
```

Broadcast a task to all connected peers via GossipSub. 

**Parameters**: 

  * **task** [Task](/source-reference/Classes/db/d71/structsgns_1_1neoswarm_1_1_task/) to broadcast. 


**Return**: outcome::success or NetworkError. 

### function BroadcastCRDT

```cpp
outcome::result< void > BroadcastCRDT(
    const std::string & crdt_data
)
```

Broadcast a CRDT state update to all peers. 

**Parameters**: 

  * **crdt_data** Serialised CRDT state. 


**Return**: outcome::success or NetworkError. 

### function ConnectedPeers

```cpp
std::vector< std::string > ConnectedPeers() const
```

Get the list of currently connected peer IDs. 

**Return**: Vector of peer ID strings. 

-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700