#pragma once

#include <type_traits>
#include <utility>

#include <resilient/common/failable.hpp>

namespace resilient {

class CircuitBreaker
{
// TODO implement properly
public:
    CircuitBreaker(bool open = false) : d_open(open) { }

    template<typename Job, typename ...Args>
    auto operator()(Job&& job, Args&&... args)
    {
        using ResultType = std::result_of_t<Job(Args&&...)>;
        using FT = FailableTraits<ResultType>;
        if(d_open)
        {
            return FT::failure();
        }
        return std::forward<Job>(job)(FWD(args)...);
    }

private:
    bool d_open;
};

}