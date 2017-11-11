#pragma once

#include <utility>
#include <type_traits>
#include <resilient/policy/noop.hpp>

namespace resilient {

template<typename Policy>
class Job
{
public:
    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &
    {
        return d_policy(std::forward<Callable>(callable), std::forward<Args>(args)...);
    }

    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &&
    {
        return std::forward<Policy>(d_policy)(std::forward<Callable>(callable), std::forward<Args>(args)...);
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