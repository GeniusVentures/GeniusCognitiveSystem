/**
 * @file       test_wait_condition.hpp
 * @brief      Wait-condition template (condition_variable polling, never a raw
 *             thread sleep).
 * @details    Copied verbatim from the moved test_gcs_global_db.cpp (originally
 *             GNUS-NEO-SWARM/test/storage/test_gcs_global_db.cpp) per the project
 *             testing rule (raw thread sleeps are forbidden in tests).
 * @date       2026-08-15
 */

#ifndef GCS_TEST_WAIT_CONDITION_HPP
#define GCS_TEST_WAIT_CONDITION_HPP

#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>

namespace gcs::test
{
    /// Upper bound for a single wait-condition call (mirrors the fixture's WAIT_TIMEOUT).
    constexpr std::chrono::milliseconds kWaitTimeout{ 25000 };
    /// Re-check interval for pure polling predicates inside WaitForCondition.
    constexpr std::chrono::milliseconds kPollInterval{ 10 };

    /**
     * @brief Wait-condition template (NEO-SWARM): poll `predicate` via
     *        condition_variable::wait_for until it returns true or `timeout` elapses.
     *
     * condition_variable only wakes on notify, so for pure polling predicates
     * (e.g. "file exists on disk") we use the sanctioned polling-with-cv idiom:
     * cv.wait_for(lock, kPollInterval, pred) inside a deadline loop.
     *
     * @param[in] predicate Nullary callable returning bool.
     * @param[in] timeout   Maximum time to wait.
     * @return true if the predicate became true before the deadline; false otherwise.
     */
    inline bool WaitForCondition( const std::function<bool()> &predicate, std::chrono::milliseconds timeout )
    {
        std::mutex              mtx;
        std::condition_variable cv;
        std::unique_lock<std::mutex> lock( mtx );
        const auto deadline = std::chrono::steady_clock::now() + timeout;
        while ( std::chrono::steady_clock::now() < deadline )
        {
            if ( predicate() )
            {
                return true;
            }
            const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(
                deadline - std::chrono::steady_clock::now() );
            const auto slice     = std::min( kPollInterval, remaining );
            cv.wait_for( lock, slice );
        }
        return predicate();
    }
} // namespace gcs::test

#endif // GCS_TEST_WAIT_CONDITION_HPP
