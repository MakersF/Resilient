#pragma once

#include <utility>

#include <resilient/policy/retry/types.hpp>

namespace resilient {
namespace retry {

/**
 * @brief Factory which returns references to the same state.
 * @related resilient::Retry
 *
 * @note
 * Implements the `RetryStateFactory` concept.
 *
 * @tparam RetryState The state to copy
 */
template<typename RetryState>
class ReferenceState
{
public:
    /**
     * @brief Construct a new ReferenceState.
     *
     * @param state The state that will be referenced
     */
    ReferenceState(RetryState& state) : d_retryState(state) {}

    template<typename Failure>
    RetryState& getRetryState(retriedtask_failure<Failure>)
    {
        return d_retryState;
    }

    void returnRetryState(RetryState&) {}

private:
    RetryState& d_retryState;
};

} // namespace retry
} // namespace resilient