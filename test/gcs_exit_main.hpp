/**
 * @file       gcs_exit_main.hpp
 * @brief      Own-main for test binaries that boot the GeniusSDK node.
 * @details    The node's async init chain leaves detached sleeper threads
 *             behind (SuperGenius ScheduleBlockchainRetry/ScheduleMigrationRetry
 *             sleep 10s/5s in std::thread(...).detach(); ~GeniusNode joins
 *             only io_threads_/upnp_thread). When a test shuts the SDK down
 *             while that chain is in flight — the fast tests land exactly
 *             inside the retry window — the stragglers wake after main()
 *             returns and lock mutexes that static destruction has already
 *             torn down: exit-time "mutex lock failed: Invalid argument"
 *             aborts AFTER all tests passed (CI 35910956692 OSX Debug,
 *             test_gcs_global_db_sdk; same family as the aarch64 exit
 *             segfaults in 35904291986).
 *
 *             Wiring assertions are complete once RUN_ALL_TESTS() returns,
 *             so we skip static teardown entirely via std::_Exit — matching
 *             the upstream SuperGenius precedent 3cf347e05 ("leaking state
 *             at exit is cheaper than the crash"). C streams are flushed
 *             first: ctest pipes output (fully buffered), and _Exit skips
 *             the normal flush.
 *
 *             Binaries using this header must link GTest::GTest only —
 *             gtest_main's main() would collide (OWN_MAIN in gcs_test()).
 */
#ifndef GCS_EXIT_MAIN_HPP
#define GCS_EXIT_MAIN_HPP

#include <cstdio>

#include <gtest/gtest.h>

#include <cstdlib>

int main( int argc, char **argv )
{
    ::testing::InitGoogleTest( &argc, argv );
    const int result = RUN_ALL_TESTS();

    std::fflush( stdout );
    std::fflush( stderr );
    std::_Exit( result );
}

#endif // GCS_EXIT_MAIN_HPP
