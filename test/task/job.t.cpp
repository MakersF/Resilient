#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <resilient/detector/returns.hpp>
#include <resilient/policy/retry.hpp>
#include <resilient/task/task.hpp>

using namespace resilient;

namespace {

struct MaxRetries
{
    int d_maxRetries;

    bool shouldExecute()
    {
        // shouldExecute gets called at the start as well, so we add 1 to offset that
        return (d_maxRetries-- + 1) != 0;
    }

    template<typename T>
    void failedWith(T)
    {
    }
};

} // namespace

TEST(aaa, aaa)
{
    Retry<CopyRetryStateFactory<MaxRetries>> retry{{{3}}};

    auto result = retry.execute(task([start = 0]() mutable {
                                    std::cout << "Run: " << start << std::endl;
                                    return start++;
                                })
                                    .failsIf(Returns<int>(0)));

    EXPECT_TRUE(holds_value(result));
}