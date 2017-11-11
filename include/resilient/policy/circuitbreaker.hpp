#pragma once

#include <type_traits>
#include <utility>

#include <resilient/common/failable.hpp>
#include <resilient/policy/policy_utils.hpp>

namespace resilient {

struct CircuitBreakerIsOpen {};

class CircuitBreaker
{
// TODO implement properly
private:
    template<typename Callable, typename ...Args>
    using return_type_t =
        add_failure_to_noref_t<forward_result_of_t<Callable, Args...>, CircuitBreakerIsOpen>;

public:
    CircuitBreaker(bool open = false) : d_open(open) { }

    template<typename Callable, typename ...Args>
    return_type_t<Callable, Args...> operator()(Callable&& callable, Args&&... args)
    {
        if(d_open)
        {
            return CircuitBreakerIsOpen();
        }
        return std::forward<Callable>(callable)(std::forward<Args>(args)...);
    }

private:
    bool d_open;
};

}