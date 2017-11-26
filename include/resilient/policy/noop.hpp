#pragma once

#include <utility>
#include <resilient/common/invoke.hpp>

namespace resilient {

class Noop
{
public:
    template<typename Callable, typename ...Args>
    decltype(auto) operator()(Callable&& callable, Args&&... args)
    {
        return detail::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...);
    }
};

}