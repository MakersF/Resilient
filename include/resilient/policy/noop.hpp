#pragma once

#include <utility>

namespace resilient {

class Noop
{
public:
    template<typename Callable, typename ...Args>
    decltype(auto) operator()(Callable&& callable, Args&&... args)
    {
        return std::forward<Callable>(callable)(std::forward<Args>(args)...);
    }
};

}