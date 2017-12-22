#pragma once

#include <utility>

#include <resilient/detail/invoke.hpp>
#include <resilient/policy/policy_utils.hpp>
#include <resilient/detail/variant_utils.hpp>
#include <resilient/task/failable_utils.hpp>
#include <resilient/detail/utilities.hpp>

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
            return from_failure<return_type_t<Callable, Args...>>(CircuitBreakerIsOpen());
        }

        return from_narrower_failable<return_type_t<Callable, Args...>>(detail::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...));
    }

private:
    bool d_open;
};

}