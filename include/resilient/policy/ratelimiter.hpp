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
    using permit_ptr = void*;

    /**
     * @brief Acquire a single permit to execute a request.
     *
     * `acquire()` is called before invoking the function to wait for a permit.
     * This call may block.
     *
     * @returns A pointer to any object. The pointer is only used to call `release()`.
     */
    virtual permit_ptr acquire() = 0;

    /**
     * @brief Release a permit that was previously acquired.
     *
     * `release()` is called after the invoked function returned.
     *
     * @param permit_ptr The permit returned by `acquire()`.
     */
    virtual void release(permit_ptr) = 0;
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
private:
    // RAII scope guard to acquire and release tokens.
    struct AcquireReleaseGuard
    {
        AcquireReleaseGuard(IRateLimiterStrategy& strategy)
        : d_rateLimiterStrategy(strategy), d_permit_ptr(d_rateLimiterStrategy.acquire())
        {
        }

        ~AcquireReleaseGuard() { d_rateLimiterStrategy.release(d_permit_ptr); }

        IRateLimiterStrategy& d_rateLimiterStrategy;
        IRateLimiterStrategy::permit_ptr d_permit_ptr;
    };

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
        AcquireReleaseGuard guard(*d_strategy);
        return detail::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

private:
    std::unique_ptr<IRateLimiterStrategy> d_strategy;
};

} // namespace resilient