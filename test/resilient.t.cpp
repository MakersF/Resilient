#include <resilient.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <type_traits>

#include "resilient/common/result.hpp"

using namespace resilient;

TEST(test, test1)
{
#if 0
    auto a = Returns<int>(11)(Returns<int>(22), [](){ return CallResult<int>{new int(12), false}; });
    EXPECT_FALSE(a.failure);

    FailureDetector<> fd;
    auto fd1 = std::move(fd).addFailureCondition(Returns<int>(11));
    static_assert(std::is_same<decltype(fd1), FailureDetector<Returns<int>>>::value, "Wrong Type");
    auto fd2 = std::move(fd1).addFailureCondition(Returns<int>(22));
    static_assert(std::is_same<decltype(fd2), FailureDetector<Returns<int>, Returns<int>>>::value, "Wrong Type");
#endif
    struct T
    {
        int a=11;
        int& operator()() {return a;}
    };

    auto ret = Returns<int>(11);
    auto detector = failsIf(ret)
                    .orIf(Returns<int>(22));
    //auto result = detector.detectFailure([](){ return 12; });
    auto result = detector.detectFailure(T());
    static_assert(std::is_same<boost::variant<Failure, int&>, decltype(result)>::value, "not same");
    EXPECT_TRUE(isFailure(result));
}

TEST(test, test2)
{
#if 0
    auto rt = ResilientJob::withRetryPolicy();
    static_assert(std::is_same<decltype(rt), RetryPolicyImpl<FailureDetector<>>>::value, "Wrong Type");

    auto rt1 = std::move(rt).failsIf(Returns<int>(11));
    static_assert(std::is_same<decltype(rt1), RetryPolicyImpl<FailureDetector<Returns<int>>>>::value, "Wrong Type");

    auto rt2 = std::move(rt1).failsIf(Returns<int>(22));
    static_assert(std::is_same<decltype(rt2), RetryPolicyImpl<FailureDetector<Returns<int>, Returns<int>>>>::value, "Wrong Type");

    rt2.run([](int val) { return val;}, 10);
#endif
}

