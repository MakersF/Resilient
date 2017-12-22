#pragma once

#include <utility>
#include <resilient/detail/invoke.hpp>

namespace resilient {

/**
 * @brief Run `task`s with a specific `policy`.
 *
 * @tparam Policy The `policy` to use when running the task.
 */
template<typename Policy>
class Job
{
public:

    /**
     * @brief Create an instance of `Job` with the specific policy.
     *
     * @param policy The policy to use.
     */
    explicit Job(Policy&& policy)
    : d_policy(std::forward<Policy>(policy))
    { }

    /**
     * @brief Execute a `task` with the given `policy`.
     *
     * @param callable The `task` to run.
     * @param args The arguments to pass to the `task`.
     * @return The value returned by invoking the `policy` with the `task`.
     */
    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &
    {
        return detail::invoke(d_policy, std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    /**
     * @brief Like `run() &`, but also moves the policy when invoking it with the task.
     */
    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &&
    {
        // Even if Job is rvalue here we can move the policy only if it was not an lvalue type,
        // otherwise the user might still have a reference to it.
        // To avoid the problem we use forward
        return detail::invoke(std::forward<Policy>(d_policy), std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

private:
    Policy d_policy;
};


/**
 * @brief Create a `Job` from a policy to run tasks with.
 * @related Job
 *
 * @param policy The policy to use.
 * @return The `Job` using the policy.
 */
template<typename Policy>
inline Job<Policy> with(Policy&& policy)
{
    return Job<Policy>(std::forward<Policy>(policy));
}

}