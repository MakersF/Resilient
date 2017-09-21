#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <resilient/common/failable.hpp>
#include <resilient/common/utilities.hpp>
#include <resilient/common/foldinvoke.hpp>

namespace resilient {

template<typename Policy>
struct PolicyTraits
{
private:
    // Test whether passing a function which returns a Failable
    // the policy returns another failable.
    using Failure = char;
    using ReturnType = int;
    using FunReturningFailable = Failable<Failure, ReturnType> (*) ();
    using PolicyResult = std::result_of_t<Policy(FunReturningFailable)>;

public:
    static constexpr bool is_policy = PolicyResult::is_failable;
};

template<typename ...Policies>
class Pipeline
{
public:
    template<typename Policy>
    Pipeline<Policies... , Policy> then(Policy&& policy) &
    {
        static_assert(PolicyTraits<Policy>::is_policy, "Not a valid policy");
        return Pipeline<Policies... , Policy>(
            tuple_append(d_policies, std::forward<Policy>(policy)));
    }

    template<typename Policy>
    Pipeline<Policies... , Policy> then(Policy&& policy) &&
    {
        static_assert(PolicyTraits<Policy>::is_policy, "Not a valid policy");
        return Pipeline<Policies... , Policy>(
            tuple_append(std::move(d_policies), std::forward<Policy>(policy)));
    }

    template<typename Callable, typename ...Args>
    decltype(auto) operator()(Callable&& callable, Args&&... args) &
    {
        return foldInvoke(d_policies, std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    template<typename Callable, typename ...Args>
    decltype(auto) operator()(Callable&& callable, Args&&... args) &&
    {
        return foldInvoke(std::move(d_policies), std::forward<Callable>(callable), std::forward<Args>(args)...);
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
    static_assert(PolicyTraits<Policy>::is_policy, "Not a valid policy");
    return Pipeline<Policy>(std::tuple<Policy>(std::forward<Policy>(policy)));
}

} // resilient