#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <resilient/policy/retry.hpp>
#include <resilient/detector/returns.hpp>
#include <resilient/task/task.hpp>
#include <iostream>

using namespace resilient;

TEST(aaa, aaa)
{
    RetryPolicy(5)
        .execute(
            task([start = 0] () mutable { std::cout << "Run" << std::endl; return start++; })
                .failsIf(Returns<int>(0)));
}