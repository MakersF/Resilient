#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <resilient/detail/utilities.hpp>
#include <resilient/detail/foldinvoke.hpp>

namespace resilient {

// Define a sequence of polices, which will be executed in order
template<typename ...Policies>
class Pipeline
{
public:
    template<typename Policy>
    Pipeline<Policies... , Policy> then(Policy&& policy) &
    {
        return Pipeline<Policies... , Policy>(
            tuple_append(d_policies, std::forward<Policy>(policy)));
    }

    template<typename Policy>
    Pipeline<Policies... , Policy> then(Policy&& policy) &&
    {
        return Pipeline<Policies... , Policy>(
            tuple_append(std::move(d_policies), std::forward<Policy>(policy)));
    }

    template<typename Callable, typename ...Args>
    decltype(auto) operator()(Callable&& callable, Args&&... args) &
    {
        return detail::foldInvoke(d_policies, std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    template<typename Callable, typename ...Args>
    decltype(auto) operator()(Callable&& callable, Args&&... args) &&
    {
        return detail::foldInvoke(std::move(d_policies), std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    explicit Pipeline(std::tuple<Policies...>&& policies)
    : d_policies(std::move(policies))
    { }

private:
    std::tuple<Policies...> d_policies;
};


template<typename Policy>
Pipeline<Policy> pipelineOf(Policy&& policy)
{
    return Pipeline<Policy>(std::tuple<Policy>(std::forward<Policy>(policy)));
}

} // resilient