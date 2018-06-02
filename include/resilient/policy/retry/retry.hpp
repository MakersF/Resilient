#pragma once

#include <thread>
#include <type_traits>
#include <utility>

#include <resilient/detail/invoke.hpp>
#include <resilient/policy/policy_utils.hpp>
#include <resilient/policy/retry/types.hpp>

namespace resilient {

/**
 * @ingroup Policy
 * @brief Reduce the impact of tasks failing due to transient failures.
 *
 * A task might fail because of a temporary condition.
 * In such a situation the failure can be managed by retrying the operation again.
 *
 * The `Retry` calls the tasks which is executed with it until it succeeds or it's
 * determined that it shouldn't be executed anymore.
 * Optionally the `Retry` can wait between calls. This can be useful if the task is consuming
 * resources and there is no reason to try several executions as close as possible.
 *
 * For each execution of a task an associated `RetryState` is used.
 * Any class which abides to the `RetryState` concept can be used.
 * `Retry` is instantiate with a factory class which is called to get an instance of `RetryState`
 * for each task.
 * Any class which abides to the `RetryStateFactory` concept can be used.
 *
 * @par `RetryState` concept
 * A `RetryState` keeps track of the retries and failures of a task in a `Retry` and decides
 * whether it should be re-executed or not.
 * The following must be valid for an instance named `state` of type `T` which implements
 * the `RetryState` concept.
 *
 * @par
 * - Checking if `Retry` should retry.
 * The statement `resilient::Variant<resilient::retry_after, T::stopretries_type> a = state.shouldRetry();` must
 * be valid.
 * When `Retry` should execute again `holds_alternative<resilient::retry_after>(a)` must be true.
 * The current thread will wait for the duration of the value contained in `retry_after` if it's strictly positive.
 * When `Retry` should stop executing the task `holds_alternative<T::stopretries_type>(a)` must be true.
 * `Retry` will return a Failable with the failure set to the value of type `T::stop_retries`.
 *
 * @par
 * - Propagating failure informations.
 * Given an instance `failure` of type `Q`, the statement `state.failedWith(std::move(q));` must be valid.
 * `Q` is the `Failure` hold by the `Failable` the associated task returns.
 *
 *
 * @par `RetryStateFactory` concept
 * A `RetryStateFactory` is used to create a `RetryState` to associate to the execution of a task with the
 * `Retry` policy.
 * The following must be true for an instance named `factory` of type `T` which implements the
 * `RetryStateFactory` concept.
 *
 * @par
 * - Getting a `RetryState` instance
 * The statement `decltype(auto) state{factory.getRetryState(retriedtask_failure<Q>{})};` must be valid.
 * `Q` is the `Failure` hold by the `Failable` the associated task returns.
 * `state` must be an instance of a type which implements the `RetryState` concept.
 *
 * @par
 * - Returning a `RetryState` instance
 * The statement `factory.returnRetryState(std::forward<decltype(state)>(state));` must be valid.
 * The method is always called once for each call made to `getRetryState` which did not throw an exception,
 * independently from the result of the execution of the task.
 * This call is done after all the retries for the given task have been executed.
 *
 *
 * @tparam RetryStateFactory The factory used to create the RetryState used while
 */
template<typename RetryStateFactory>
class Retry
{
private:
    template<typename Failure>
    using retry_state =
        decltype(std::declval<RetryStateFactory>().getRetryState(retriedtask_failure<Failure>{}));

    // We are swallowing the failures returned by the callable.
    // In order to give a way to propagate them, we provide the failure type
    // as parameter to the stopretries_type, so that an user implementation might
    // store the failures and propagate them.
    template<typename Failure>
    using stopretries_type =
        typename std::remove_reference_t<retry_state<Failure>>::stopretries_type;

    template<typename Callable, typename... Args>
    using failure_type_of_returned_failable =
        typename std::remove_reference_t<noforward_result_of_t<Callable, Args...>>::failure_type;

    // We return in 2 cases:
    //   - if the task returned a value
    //   - if we can't retry anymore
    // This means that the failure type returned by the task is never returned.
    // Because of this, instead of adding the new failure type we return when we
    // don't retry anymore, we replace the task's failure types.
    template<typename Callable, typename... Args>
    using return_type_t = replace_failure_in_noref_failable_t<
        noforward_result_of_t<Callable, Args...>,
        stopretries_type<failure_type_of_returned_failable<Callable, Args...>>>;

    // RAII scope guard to end executions
    template<typename RetryState>
    struct ExecutionGuard
    {
        ~ExecutionGuard()
        {
            d_retryStateFactor.returnRetryState(std::forward<RetryState>(d_state));
        }

        RetryStateFactory& d_retryStateFactor;
        RetryState d_state;
    };

public:
    /**
     * @brief Construct a new Retry object
     *
     * @param retryStateFactory The factory to be used to generate states for the retries.
     */
    Retry(RetryStateFactory retryStateFactory)
    : d_retryStateFactory(std::forward<RetryStateFactory>(retryStateFactory))
    {
    }

    /**
     * @brief Execute the task, retrying if it returns a failure.
     *
     * Execute the task until either it returns a success or the
     * retry state returns a `stopretries_type` when checking if we should retry.
     *
     * @param callable The task to execute
     * @param args... The arguments to the task
     * @return A Failable which has as value the value returned by
     *         task and as failure the `stopretries_type` of the retry state.
     */
    template<typename Callable, typename... Args>
    return_type_t<Callable, Args...> execute(Callable&& callable, Args&&... args)
    {
        using failure_type = failure_type_of_returned_failable<Callable, Args...>;
        using retry_state = Retry::retry_state<failure_type>;
        using stopretries_type = Retry::stopretries_type<failure_type>;

        ExecutionGuard<retry_state> guard{
            d_retryStateFactory,
            d_retryStateFactory.getRetryState(retriedtask_failure<failure_type>{})};

        // First execution always happens.
        // This can be called several times, so we can't move callable and args... into it.
        decltype(auto) result{detail::invoke(callable, args...)};
        if (holds_value(result)) {
            return get_value(std::forward<decltype(result)>(result));
        }
        else
        {
            guard.d_state.failedWith(get_failure(std::forward<decltype(result)>(result)));
        }

        decltype(auto) shouldRetry = guard.d_state.shouldRetry();

        // While the retry result specifies a time to wait for we keep retrying
        while (holds_alternative<retry_after>(shouldRetry)) {
            std::chrono::microseconds sleepTime = get<retry_after>(shouldRetry).value;
            if (sleepTime != std::chrono::microseconds::zero()) {
                std::this_thread::sleep_for(sleepTime);
            }
            // Same as before, we can not move here
            decltype(auto) result{detail::invoke(callable, args...)};
            if (holds_value(result)) {
                return get_value(std::forward<decltype(result)>(result));
            }
            else
            {
                guard.d_state.failedWith(get_failure(std::forward<decltype(result)>(result)));
                shouldRetry = guard.d_state.shouldRetry();
            }
        }

        // We got a stopretries_type in the variant, so we return it
        return get<stopretries_type>(std::forward<decltype(shouldRetry)>(shouldRetry));
    }

private:
    RetryStateFactory d_retryStateFactory;
};

} // namespace resilient