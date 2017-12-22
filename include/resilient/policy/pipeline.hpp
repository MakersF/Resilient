#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <resilient/detail/utilities.hpp>
#include <resilient/detail/foldinvoke.hpp>
#include <resilient/detail/invoke.hpp>

namespace resilient {

namespace detail {

// Wrap the policy offering a callable interface which forwards to execute.
// The callable interface is needed to use foldInvoke
template<typename Policy>
struct PolicyAsCallable
{
    template<typename ...Args>
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

}

// Define a sequence of polices, which will be executed in order
template<typename ...Policies>
class Pipeline
{
public:
    template<typename Policy>
    Pipeline<Policies... , Policy> then(Policy&& policy) &
    {
        return Pipeline<Policies... , Policy>(
            tuple_append(d_policies, detail::make_callable(std::forward<Policy>(policy))));
    }

    template<typename Policy>
    Pipeline<Policies... , Policy> then(Policy&& policy) &&
    {
        return Pipeline<Policies... , Policy>(
            tuple_append(std::move(d_policies), detail::make_callable(std::forward<Policy>(policy))));
    }

    template<typename Callable, typename ...Args>
    decltype(auto) execute(Callable&& callable, Args&&... args)
    {
        return detail::foldInvoke(d_policies, std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

private:

    explicit Pipeline(std::tuple<detail::PolicyAsCallable<Policies>...>&& callablePolicies)
    : d_policies(std::move(callablePolicies))
    { }

    std::tuple<detail::PolicyAsCallable<Policies>...> d_policies;

    template<typename ...T>
    friend class Pipeline;

    template<typename ...Policy>
    friend Pipeline<Policy...> pipelineOf(Policy&&... policy);
};


template<typename ...Policy>
Pipeline<Policy...> pipelineOf(Policy&&... policy)
{
    return Pipeline<Policy...>(std::make_tuple(detail::make_callable(std::forward<Policy>(policy))...));
}

} // resilient