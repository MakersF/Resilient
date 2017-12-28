#pragma once

#include <memory>
#include <utility>

#include <resilient/detail/invoke.hpp>

namespace resilient {

/**
 * @brief Interface to specify the algorithm the `Ratelimiter` should use.
 * @related resilient::Ratelimiter
 */
class IRateLimiterStrategy
{
public:
    /**
     * @brief Acquire a single permit to execute a request. This call may block.
     */
    virtual void acquire() = 0;
    virtual ~IRateLimiterStrategy() {}
};

/**
 * @ingroup Policy
 * @brief Execute a `Task` limiting the times it can be executed following a strategy.
 *
 * A rate limiter distributes executions over time to guarantee that they stay below a specific rate.
 * `Ratelimiter` uses a strategy, which is an implementation of a rate limiting algorithm, to execute
 * the provided `Task`.
 */
class Ratelimiter
{
public:
    /**
     * @brief Construct a `Ratelimiter` with the provided strategy.
     */
    Ratelimiter(std::unique_ptr<IRateLimiterStrategy>&& strategy) : d_strategy(std::move(strategy))
    {
    }

    /**
     * @brief Execute the task, using the strategy to ensure that the rate limit is satisfied.
     *
     * @param callable The task to execute
     * @param args... The arguments to the task
     * @return The result of invoking the task with the arguments
     */
    template<typename Callable, typename... Args>
    decltype(auto) execute(Callable&& callable, Args&&... args)
    {
        d_strategy->acquire();
        return detail::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...));
    }

private:
    std::unique_ptr<IRateLimiterStrategy> d_strategy;
};

} // namespace resilient