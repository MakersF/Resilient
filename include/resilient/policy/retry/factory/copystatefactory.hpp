#pragma once

#include <utility>

#include <resilient/policy/retry/types.hpp>

namespace resilient {

/**
 * @brief Factory which returns copies of the same state
 * @related resilient::Retry
 *
 * @note
 * Implements the `RetryStateFactory` concept.
 *
 * @tparam RetryState The state to copy
 */
template<typename RetryState>
class CopyRetryStateFactory
{
public:
    /**
     * @brief Construct a new CopyRetryStateFactory.
     *
     * @param state The state that will be copied
     */
    CopyRetryStateFactory(RetryState state) : d_retryState(std::forward<RetryState>(state)) {}

    template<typename Failure>
    RetryState getRetryState(retriedtask_failure<Failure>)
    {
        return d_retryState;
    }

    void returnRetryState(RetryState) {}

private:
    RetryState d_retryState;
};

} // namespace resilient