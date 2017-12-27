#pragma once

#include <resilient/detail/invoke.hpp>
#include <utility>

namespace resilient {

class Noop
{
public:
    template<typename Callable, typename... Args>
    decltype(auto) execute(Callable&& callable, Args&&... args)
    {
        return detail::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
    }
};

} // namespace resilient