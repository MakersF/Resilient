#pragma once

#include <type_traits>
#include <utility>

#include <resilient/common/failable.hpp>

namespace resilient {

class RetryPolicy
{
// TODO implement properly
public:
    RetryPolicy(int numRetries) : d_retries(numRetries) { }

    template<typename Job, typename ...Args>
    auto operator()(Job&& job, Args&&... args)
    {
        using ResultType = std::result_of_t<Job(Args&&...)>;
        for(int i = 0; i < d_retries; ++i)
        {
            auto&& result = job(std::forward<Args>(args)...);
            if(result.isValue())
            {
                return std::move(result);
            }
        }
        return failure_for<ResultType>();
    }

private:
    int d_retries;
};

}