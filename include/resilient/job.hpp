#pragma once

#include <utility>
#include <resilient/common/invoke.hpp>

namespace resilient {

template<typename Policy>
class Job
{
public:
    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &
    {
        return detail::invoke(d_policy, std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &&
    {
        return detail::invoke(std::forward<Policy>(d_policy), std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    explicit Job(Policy&& policy)
    : d_policy(std::forward<Policy>(policy))
    { }

private:
    Policy d_policy;
};


template<typename Policy>
inline Job<Policy> with(Policy&& policy)
{
    return Job<Policy>(std::forward<Policy>(policy));
}

}