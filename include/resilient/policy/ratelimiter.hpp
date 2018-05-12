#pragma once

#include <memory>
#include <type_traits>
#include <utility>

#include <resilient/common/variant.hpp>
#include <resilient/detail/invoke.hpp>
#include <resilient/detail/variant_utils.hpp>
#include <resilient/policy/policy_utils.hpp>
#include <resilient/task/failable_utils.hpp>

namespace resilient {

/**
 * @brief Interface to specify the algorithm the `Ratelimiter` should use.
 * @related resilient::Ratelimiter
 *
 * @tparam Permit The type of permits the strategy returns.
 * @tparam Error The kind of error returned when acquire fails.
 */
template<typename Permit, typename Error>
class IRateLimiterStrategy
{
public:
    /**
     * @brief The type of the permit returned by acquire.
     */
    using permit_type = Permit;

    /**
     * @brief The type of the error returned by acquire.
     */
    using error_type = Error;

    /**
     * @brief Acquire a single permit to execute a request.
     *
     * `acquire()` is called before invoking the function to wait for a permit.
     * This call may block.
     *
     * @returns Either return a permit or an error. If a permit is returned `release()`
     *          is called with the permit.
     */
    virtual Variant<permit_type, error_type> acquire() = 0;

    /**
     * @brief Release a permit that was previously acquired.
     *
     * `release()` is called after the invoked function returned or throwed.
     *
     * @param permit_type The permit returned by `acquire()`.
     */
    virtual void release(permit_type) = 0;

    virtual ~IRateLimiterStrategy() {}
};

/**
 * @ingroup Policy
 * @brief Execute a `Task` limiting the times it can be executed following a strategy.
 *
 * A rate limiter distributes executions over time to guarantee that they stay below a specific rate.
 * `Ratelimiter` uses a strategy, which is an implementation of a rate limiting algorithm, to execute
 * the provided `Task`.
 *
 * @tparam Strategy: the strategy to use. Must derive from IRateLimiterStrategy.
 */
template<typename Strategy>
class Ratelimiter
{
private:
    using strategy_permit = typename Strategy::permit_type;
    using strategy_error = typename Strategy::error_type;

    static_assert(
        std::is_convertible<Strategy*,
                            IRateLimiterStrategy<strategy_permit, strategy_error>*>::value,
        "The strategy must derive from IRateLimiterStrategy.");

    // RAII scope guard to release permits.
    struct ReleaseGuard
    {
        ReleaseGuard(Strategy& strategy, strategy_permit permit)
        : d_rateLimiterStrategy(strategy), d_permit(std::move(permit))
        {
        }

        ~ReleaseGuard() { d_rateLimiterStrategy.release(std::move(d_permit)); }

        Strategy& d_rateLimiterStrategy;
        strategy_permit d_permit;
    };

    template<typename Callable, typename... Args>
    using return_type_t =
        add_failure_to_noref_t<forward_result_of_t<Callable, Args...>, strategy_error>;

public:
    /**
     * @brief Construct a `Ratelimiter` with the provided strategy.
     *
     * @param strategy A pointer to the strategy to use.
     */
    Ratelimiter(std::unique_ptr<Strategy> strategy) : d_strategy(std::move(strategy)) {}

    /**
     * @brief Execute the task, using the strategy to ensure that the rate limit is satisfied.
     *
     * @param callable The task to execute
     * @param args... The arguments to the task
     * @return The result of invoking the task with the arguments
     */
    template<typename Callable, typename... Args>
    return_type_t<Callable, Args...> execute(Callable&& callable, Args&&... args)
    {
        using result_type = return_type_t<Callable, Args...>;
        decltype(auto) maybePermit{d_strategy->acquire()};
        return visit(detail::overload<result_type>(
                         [this, &callable, &args...](strategy_permit permit) {
                             ReleaseGuard guard(*d_strategy, std::forward<strategy_permit>(permit));
                             return from_narrower_failable<return_type_t<Callable, Args...>>(
                                 detail::invoke(std::forward<Callable>(callable),
                                                std::forward<Args>(args)...));
                         },
                         [](strategy_error error) {
                             return from_failure<return_type_t<Callable, Args...>>(
                                 std::forward<strategy_error>(error));
                         }),
                     std::forward<decltype(maybePermit)>(maybePermit));
    }

private:
    std::unique_ptr<Strategy> d_strategy;
};

} // namespace resilient