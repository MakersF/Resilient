#pragma once

#include <utility>

namespace resilient {

class Noop
{
public:
    template<typename Job, typename ...Args>
    decltype(auto) operator()(Job&& job, Args&&... args)
    {
        return std::forward<Job>(job)(std::forward<Args>(args)...);
    }
};

}