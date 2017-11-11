#pragma once

#include <type_traits>
#include <utility>

#include <resilient/common/failable.hpp>
#include <resilient/policy/policy_utils.hpp>

namespace resilient {

struct NoMoreRetriesAvailable {};

class RetryPolicy
{
// TODO implement properly
private:
    template<typename Callable, typename ...Args>
    using return_type_t =
        add_failure_to_noref_t<noforward_result_of_t<Callable, Args...>, NoMoreRetriesAvailable>;

public:
    RetryPolicy(int numRetries) : d_retries(numRetries) { }

    template<typename Callable, typename ...Args>
    return_type_t<Callable, Args...> operator()(Callable&& callable, Args&&... args)
    {
        for(int i = 0; i <= d_retries; ++i)
        {
            decltype(auto) result{callable(args...)};
            if(holds_value(result))
            {
                return std::forward<decltype(result)>(result);
            }
        }
        return NoMoreRetriesAvailable();
    }

private:
    int d_retries;
};

}