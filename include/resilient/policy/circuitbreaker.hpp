#pragma once

#include <memory>
#include <utility>

#include <resilient/detail/invoke.hpp>
#include <resilient/detail/utilities.hpp>
#include <resilient/detail/variant_utils.hpp>
#include <resilient/policy/policy_utils.hpp>
#include <resilient/task/failable_utils.hpp>

namespace resilient {

/**
 * @brief Indicate a failure because the `CircuitBreaker` was open.
 * @related resilient::CircuitBreaker
 */
struct CircuitBreakerIsOpen
{
};

/**
 * @brief Interface to specify the algorithm the `CircuitBreaker` should use.
 * @related resilient::CircuitBreaker
 */
class ICircuitBreakerStrategy
{
public:
    /**
     * @brief Check whether the `CircuitBreaker` tripped and it should stop executing tasks.
     *
     * @return true Do not execute the task.
     * @return false Execute the task.
     */
    virtual bool isOpen() = 0;

    /**
     * @brief Notify the algorithm that an execution resulted in failure.
     */
    virtual void registerFailure() = 0;

    /**
     * @brief Notify the algorithm that an execution resulted in success.
     */
    virtual void registerSuccess() = 0;

    virtual ~ICircuitBreakerStrategy() {}
};

/**
 * @ingroup Policy
 * @brief Limit the impact of a failing dependecy.
 *
 * A missbehaving dependency might take a long time to respond.
 * When many operations depend on it the slowdown can be propagated to the rest of the system.
 *
 * The `CircuitBreaker` uses an algorithm (implemented in the strategy) to determine when
 * to stop contacting the dependency and just assume a failure.
 *
 * This keeps limit the performance impact a dependency can cause to the whole system.
 *
 */
class CircuitBreaker
{
private:
    template<typename Callable, typename... Args>
    using return_type_t =
        add_failure_to_noref_t<forward_result_of_t<Callable, Args...>, CircuitBreakerIsOpen>;

public:
    /**
     * @brief Construct a `CircuitBreaker` with the given strategy.
     */
    CircuitBreaker(std::unique_ptr<ICircuitBreakerStrategy>&& strategy)
    : d_strategy(std::move(strategy))
    {
    }

    /**
     * @brief Execute the task if the `CircuitBreaker` is not open.
     *
     * @param callable The task to execute
     * @param args... The arguments to the task
     * @return The result of invoking the task, or CircuitBreakerIsOpen if the task was not executed.
     */
    template<typename Callable, typename... Args>
    return_type_t<Callable, Args...> execute(Callable&& callable, Args&&... args)
    {
        if (d_strategy->isOpen()) {
            return from_failure<return_type_t<Callable, Args...>>(CircuitBreakerIsOpen());
        }

        // Invoke the task and keep the result
        decltype(auto) result{
            detail::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...)};

        if (holds_failure(result)) {
            d_strategy->registerFailure();
        }
        else
        {
            d_strategy->registerSuccess();
        }

        // Create a Failable from the returned failable (which is narrower than the one returned
        // by this method).
        // Need to use forward on result because it might be a lvalue and using move would be wrong.
        return from_narrower_failable<return_type_t<Callable, Args...>>(
            std::forward<decltype(result)>(result));
    }

private:
    std::unique_ptr<ICircuitBreakerStrategy> d_strategy;
};

} // namespace resilient