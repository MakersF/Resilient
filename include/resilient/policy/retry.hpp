#pragma once

#include <utility>

#include <resilient/detail/invoke.hpp>
#include <resilient/policy/policy_utils.hpp>

namespace resilient {

struct NoMoreRetriesAvailable
{
};

// Concept: RetryStateFactory
//
// startExecution() -> RetryState
// endExecution(RetryState)
//
// Concept: RetryState
//
// shouldExecute() -> bool
// failedWith(Error) -> void

template<typename RetryState>
class CopyRetryStateFactory
{
public:
    using state_type = RetryState;

    CopyRetryStateFactory(RetryState state) : d_retryState(std::forward<RetryState>(state)) {}

    RetryState getRetryState() { return d_retryState; }

    void returnRetryState(RetryState) {}

private:
    RetryState d_retryState;
};

template<typename RetryStateFactory>
class Retry
{
private:
    template<typename Callable, typename... Args>
    using return_type_t =
        add_failure_to_noref_t<noforward_result_of_t<Callable, Args...>, NoMoreRetriesAvailable>;

    using state_type = typename RetryStateFactory::state_type;

    // RAII scope guard to end executions
    struct ExecutionGuard
    {
        ~ExecutionGuard()
        {
            d_retryStateFactor.returnRetryState(std::forward<state_type>(d_state));
        }

        RetryStateFactory& d_retryStateFactor;
        state_type d_state;
    };

public:
    Retry(RetryStateFactory retryStateFactory)
    : d_retryStateFactory(std::forward<RetryStateFactory>(retryStateFactory))
    {
    }

    template<typename Callable, typename... Args>
    return_type_t<Callable, Args...> execute(Callable&& callable, Args&&... args)
    {
        ExecutionGuard guard{d_retryStateFactory, d_retryStateFactory.getRetryState()};

        while (guard.d_state.shouldExecute()) {
            // This can be called several times, so we can't move callable and args... into it.
            decltype(auto) result{detail::invoke(callable, args...)};
            if (holds_value(result)) {
                return get_value(std::forward<decltype(result)>(result));
            }
            else
            {
                guard.d_state.failedWith(get_failure(std::forward<decltype(result)>(result)));
            }
        }
        return NoMoreRetriesAvailable();
    }

private:
    RetryStateFactory d_retryStateFactory;
};

} // namespace resilient