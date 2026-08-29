/**
 * @file       test_graphsync_network.hpp
 * @brief      Test-owned graphsync Network construction (Tier 2 fixture
 *             pattern — no GeniusNode required).
 * @details    A test-constructed GossipPubSub owns its libp2p host exclusively
 *             (no protocol-handler contention), so building a graphsync
 *             Network on that host is safe. The Network's AsioSchedulerBackend
 *             must keep its io_context + scheduler alive exactly as long as
 *             the Network itself — TestGraphsyncContext bundles all three so
 *             the fixture holds one lifetime.
 * @date       2026-08-27
 */

#ifndef GCS_TEST_GRAPHSYNC_NETWORK_HPP
#define GCS_TEST_GRAPHSYNC_NETWORK_HPP

#include <chrono>
#include <memory>
#include <cstdint>

#include <boost/asio/io_context.hpp>

#include <libp2p/basic/scheduler.hpp>
#include <libp2p/basic/scheduler/asio_scheduler_backend.hpp>
#include <libp2p/basic/scheduler/scheduler_impl.hpp>

#include "ipfs_lite/ipfs/graphsync/impl/network/network.hpp"

#include "ipfs_pubsub/gossip_pubsub.hpp"

namespace gcs::test
{
    /// Scheduler tick for the test network's AsioSchedulerBackend (mirrors
    /// gcs_global_db.cpp kSchedulerTickMs).
    constexpr uint32_t kGraphsyncSchedulerTickMs{ 100 };

    /**
     * @brief Lifetime bundle: io_context + scheduler + the Network built on
     *        them. Members destroy in reverse declaration order, so the
     *        Network dies before the scheduler/io_context it references.
     */
    struct TestGraphsyncContext
    {
        std::shared_ptr<boost::asio::io_context> io;
        std::shared_ptr<libp2p::basic::Scheduler> scheduler;
        std::shared_ptr<sgns::ipfs_lite::ipfs::graphsync::Network> network;
    };

    /**
     * @brief Construct a graphsync Network on a test-owned pubsub's host.
     *
     * @param[in] pubsub A started GossipPubSub whose host backs the Network.
     * @return The lifetime bundle; pass .network to the two-arg
     *         GcsGlobalDb::Initialize seam and keep the bundle alive until
     *         after Shutdown().
     */
    inline TestGraphsyncContext MakeGraphsyncContext(
        const std::shared_ptr<sgns::ipfs_pubsub::GossipPubSub> &pubsub )
    {
        TestGraphsyncContext ctx;
        ctx.io = std::make_shared<boost::asio::io_context>();
        ctx.scheduler = std::make_shared<libp2p::basic::SchedulerImpl>(
            std::make_shared<libp2p::basic::AsioSchedulerBackend>( ctx.io ),
            libp2p::basic::Scheduler::Config{
                std::chrono::milliseconds{ kGraphsyncSchedulerTickMs } } );
        ctx.network = std::make_shared<sgns::ipfs_lite::ipfs::graphsync::Network>(
            pubsub->GetHost(), ctx.scheduler );
        return ctx;
    }
} // namespace gcs::test

#endif // GCS_TEST_GRAPHSYNC_NETWORK_HPP
