#pragma once

#include <utility>
#include <type_traits>
#include <resilient/policy/noop.hpp>

namespace resilient {

template<typename Policy>
class Job
{
public:
    template<typename NewPolicy,
             typename = std::enable_if_t<
                std::is_same<
                    std::decay_t<NewPolicy>,
                    Noop>::value>>
    Job<NewPolicy> with(NewPolicy&& policy) &
    {
        return Job<NewPolicy>(std::forward<NewPolicy>(policy));
    }

    template<typename NewPolicy
             typename = std::enable_if_t<
             std::is_same<
                 std::decay_t<NewPolicy>,
                 Noop>::value>>
    Job<NewPolicy> with(NewPolicy&& policy) &&
    {
        return Job<NewPolicy>(std::forward<NewPolicy>(policy));
    }

    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &
    {
        return d_policy(std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &&
    {
        return d_policy(
            [self = std::move(*this)](auto&&... innerArgs) mutable {
                return std::move(self.d_failureDetector).detectFailure(std::forward<decltype(innerArgs)>(innerArgs)...);
            },
            std::forward<Callable>(callable),
            std::forward<Args>(args)...);
    }

    explicit Job(Policy&& policy)
    : d_policy(std::forward<Policy>(policy))
    { }

private:
    Policy d_policy;
};


inline Job<NoopPolicy> job()
{
    return Job<NoopPolicy>(NoopPolicy());
}

}