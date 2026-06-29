---
title: GNUS-NEO-SWARM/src/network/p2p_node.hpp
summary: libp2p swarm node (PTDS §4.2) 

---

# GNUS-NEO-SWARM/src/network/p2p_node.hpp



libp2p swarm node (PTDS §4.2)  [More...](#detailed-description)

## Namespaces

| Name           |
| -------------- |
| **[sgns](/source-reference/Namespaces/d2/d2b/namespacesgns/)**  |
| **[sgns::neoswarm](/source-reference/Namespaces/d6/d33/namespacesgns_1_1neoswarm/)**  |
| **[sgns::neoswarm::network](/source-reference/Namespaces/dc/d2a/namespacesgns_1_1neoswarm_1_1network/)**  |

## Classes

|                | Name           |
| -------------- | -------------- |
| class | **[sgns::neoswarm::network::P2PNode](/source-reference/Classes/d1/db5/classsgns_1_1neoswarm_1_1network_1_1_p2_p_node/)** <br/>Manages a libp2p host for swarm task broadcasting and CRDT sync.  |
| struct | **[sgns::neoswarm::network::P2PNode::Config](/source-reference/Classes/dc/d46/structsgns_1_1neoswarm_1_1network_1_1_p2_p_node_1_1_config/)**  |

## Detailed Description

libp2p swarm node (PTDS §4.2) 

**Date**: 2026-05-06 



## Source code

```cpp


#ifndef NEOSWARM_NETWORK_P2PNODE_HPP
#define NEOSWARM_NETWORK_P2PNODE_HPP

#include "common/error.hpp"
#include "common/types.hpp"
#include "security/node_identity.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace sgns::neoswarm::network
{
    class P2PNode
    {
        public:
        struct Config
        {
            std::string listen_addr_ = "/ip4/0.0.0.0/tcp/0";
            std::string bootstrap_peer_ = ""; 
            bool enable_mdns_ = true;         
            bool enable_kademlia_ = true;
            int max_peers_ = 50;
        };

        using TaskHandler = std::function<void( const Task& task, const std::string& from_peer )>;
        using CRDTHandler = std::function<void( const std::string& crdt_data )>;

        P2PNode( std::shared_ptr<security::NodeIdentity> identity, Config cfg );
        explicit P2PNode( std::shared_ptr<security::NodeIdentity> identity );
        ~P2PNode();

        outcome::result<void> Start();

        void Stop();

        bool IsRunning() const
        {
            return m_running;
        }

        std::string ListenAddress() const;

        std::string PeerId() const;

        void OnTask( TaskHandler handler )
        {
            m_taskHandler = std::move( handler );
        }

        void OnCRDT( CRDTHandler handler )
        {
            m_crdtHandler = std::move( handler );
        }

        outcome::result<void> BroadcastTask( const Task& task );

        outcome::result<void> BroadcastCRDT( const std::string& crdt_data );

        std::vector<std::string> ConnectedPeers() const;

        private:
        struct Impl;
        std::unique_ptr<Impl> m_impl;
        std::shared_ptr<security::NodeIdentity> m_identity;
        Config m_cfg;
        bool m_running = false;
        TaskHandler m_taskHandler;
        CRDTHandler m_crdtHandler;
    };

} // namespace sgns::neoswarm::network

#endif // NEOSWARM_NETWORK_P2PNODE_HPP
```


-------------------------------

Updated on 2026-06-28 at 23:28:42 -0700
