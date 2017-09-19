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
        using FT = FailableTraits<ResultType>;
        for(int i = 0; i < d_retries; ++i)
        {
            auto&& result = job(FWD(args)...);
            if(FT::isSuccess(result))
            {
                return std::move(result);
            }
        }
        return FT::failure();
    }

private:
    int d_retries;
};

}