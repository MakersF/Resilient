#pragma once

#include <resilient/detail/foldinvoke.hpp>
#include <resilient/detail/invoke.hpp>
#include <resilient/detail/utilities.hpp>
#include <tuple>
#include <type_traits>
#include <utility>

namespace resilient {

namespace detail {

// Wrap the policy offering a callable interface which forwards to execute.
// The callable interface is needed to use foldInvoke
template<typename Policy>
struct PolicyAsCallable
{
    template<typename... Args>
    decltype(auto) operator()(Args&&... args)
    {
        return d_policy.execute(std::forward<Args>(args)...);
    }

    Policy d_policy;
};

template<typename Policy>
PolicyAsCallable<Policy> make_callable(Policy&& policy)
{
    return PolicyAsCallable<Policy>{std::forward<Policy>(policy)};
}

} // namespace detail

/**
 * @ingroup Policy
 * @brief Define a sequence of polices, which will be executed in order
 *
 * A pipeline of polices runs the task recursively inside a policy.
 * For example, given a pipeline of a `RetryPolicy` and a `RateLimiter`, when the pipeline is
 * executed the `RetryPolicy` is going to retry execution of the `RateLimiter` with the provided `Task`.
 *
 *
 * @tparam Policies... The types of the policies
 */
template<typename... Policies>
class Pipeline
{
public:
    /**
     * @brief Create a new Pipeline with a new policy.
     *
     * @param policy The policy to add.
     * @return The new Pipeline.
     */
    template<typename Policy>
    Pipeline<Policies..., Policy> then(Policy&& policy) &
    {
        return Pipeline<Policies..., Policy>(
            tuple_append(d_policies, detail::make_callable(std::forward<Policy>(policy))));
    }

    /**
     * @see `Pipeline::then`
     */
    template<typename Policy>
    Pipeline<Policies..., Policy> then(Policy&& policy) &&
    {
        return Pipeline<Policies..., Policy>(tuple_append(
            std::move(d_policies), detail::make_callable(std::forward<Policy>(policy))));
    }

    /**
     * @brief Execute the `Task` with the arguments.
     *
     * @return The result of executing the task on all the policies
     */
    template<typename Callable, typename... Args>
    decltype(auto) execute(Callable&& callable, Args&&... args)
    {
        return detail::foldInvoke(
            d_policies, std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

private:
    explicit Pipeline(std::tuple<detail::PolicyAsCallable<Policies>...>&& callablePolicies)
    : d_policies(std::move(callablePolicies))
    {
    }

    std::tuple<detail::PolicyAsCallable<Policies>...> d_policies;

    template<typename... T>
    friend class Pipeline;

    template<typename... Policy>
    friend Pipeline<Policy...> pipelineOf(Policy&&... policy);
};

/**
 * @brief Create a pipeline of policies.
 * @related resilient::Pipeline
 *
 * @param policies... The policies to use in creating the pipeline.
 * @return The pipeline.
 */
template<typename... Policies>
Pipeline<Policies...> pipelineOf(Policies&&... policies)
{
    return Pipeline<Policies...>(
        std::make_tuple(detail::make_callable(std::forward<Policies>(policies))...));
}

} // namespace resilient