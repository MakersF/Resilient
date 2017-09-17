#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <resilient/common/result.hpp>
#include <resilient/common/utilities.hpp>
#include <resilient/common/foldinvoke.hpp>
#include <iostream>

namespace resilient {

template<typename Policy>
struct PolicyTraits
{
private:
    using FunReturningResult = ResultTraits<int>::type (*) ();
    using PolicyResult = std::result_of_t<Policy(FunReturningResult)>;

public:
    static constexpr bool is_valid_policy =
        ResultTraits<PolicyResult>::is_result_type;
};

template<typename ...Policies>
class Pipeline
{
public:
    template<typename Policy>
    Pipeline<Policies... , Policy> then(Policy&& policy) &
    {
        static_assert(PolicyTraits<Policy>::is_valid_policy, "Not a valid policy");
        return Pipeline<Policies... , Policy>(
            tuple_append(d_policies, std::forward<Policy>(policy)));
    }

    template<typename Policy>
    Pipeline<Policies... , Policy> then(Policy&& policy) &&
    {
        static_assert(PolicyTraits<Policy>::is_valid_policy, "Not a valid policy");
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
    static_assert(PolicyTraits<Policy>::is_valid_policy, "Not a valid policy");
    return Pipeline<Policy>(std::tuple<Policy>(std::forward<Policy>(policy)));
}


///////////////////////////////////////////////////////

struct NoopPolicy
{
    template<typename Job, typename ...Args>
    decltype(auto) operator()(Job&& job, Args&&... args)
    {
        return std::forward<Job>(job)(FWD(args)...);
    }
};

struct RetryPolicy
{
    int times;

    template<typename Job, typename ...Args>
    auto operator()(Job&& job, Args&&... args) -> std::result_of_t<Job(Args&&...)>
    {
        for(int i = 0; i < times; i++)
        {
            std::cout << "Retry " << i << " " << std::flush;
            auto&& result = job(FWD(args)...);
            if(not isFailure(result))
            {
                return std::move(result);
            }
        }
        return Failure{};
    }
};

struct Monitor
{
    template<typename Job, typename ...Args>
    auto operator()(Job&& job, Args&&... args) -> std::result_of_t<Job(Args&&...)>
    {
        std::cout << "Before " << std::flush;
        auto ret = std::forward<Job>(job)(FWD(args)...);
        std::cout << "After " << std::flush;
        return std::move(ret);
    }
};

struct CircuitBreak
{
    bool open = false;

    template<typename Job, typename ...Args>
    auto operator()(Job&& job, Args&&... args) -> std::result_of_t<Job(Args&&...)>
    {
        if(open)
        {
            std::cout << "Open " << std::flush;
            return Failure{};
        }
        std::cout << "Closed " << std::flush;
        return std::forward<Job>(job)(FWD(args)...);
    }
};


} // resilient