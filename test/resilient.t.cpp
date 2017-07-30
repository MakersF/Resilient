#include <resilient.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <type_traits>

using namespace resilient;

TEST(test, test1)
{
    auto a = Returns<int>(11)(Returns<int>(22), [](){ return CallResult<int>{new int(12), false}; });
    EXPECT_FALSE(a.failure);

    FailureDetector<> fd;
    auto fd1 = std::move(fd).addFailureCondition(Returns<int>(11));
    static_assert(std::is_same<decltype(fd1), FailureDetector<Returns<int>>>::value, "Wrong Type");
    auto fd2 = std::move(fd1).addFailureCondition(Returns<int>(22));
    static_assert(std::is_same<decltype(fd2), FailureDetector<Returns<int>, Returns<int>>>::value, "Wrong Type");

#if 0
    auto rt = ResilientJob::withRetryPolicy();
    static_assert(std::is_same<decltype(rt), RetryPolicy<FailureDetector<>>>::value, "Wrong Type");

    auto rt1 = std::move(rt).failsIf(Returns<int>(11));
    static_assert(std::is_same<decltype(rt1), RetryPolicy<FailureDetector<Returns<int>>>>::value, "Wrong Type");

    auto rt2 = std::move(rt1).failsIf(Returns<int>(22));
    static_assert(std::is_same<decltype(rt2), RetryPolicy<FailureDetector<Returns<int>, Returns<int>>>>::value, "Wrong Type");

    rt2.run([](int val) { return val;}, 10);
#endif
}