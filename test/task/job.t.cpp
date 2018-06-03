#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iostream>
#include <resilient/detector/returns.hpp>
#include <resilient/policy/retry/factory/ctorretrystatefactory.hpp>
#include <resilient/policy/retry/retry.hpp>
#include <resilient/policy/retry/state/retrytimes.hpp>
#include <resilient/task/task.hpp>

using namespace resilient;

TEST(Job, SimpleRun)
{
    Retry<CtorRetryStateFactory<RetryTimes, unsigned int>> retry{{3}};

    auto result = retry.execute(task([start = 0]() mutable {
                                    std::cout << "Run: " << start << std::endl;
                                    return start++;
                                })
                                    .failsIf(Returns<int>(0)));

    EXPECT_TRUE(holds_value(result));
}