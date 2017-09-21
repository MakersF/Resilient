#include <resilient/failures/returns.hpp>
#include <resilient/failures/any.hpp>
#include <resilient/failures/never.hpp>
#include <resilient/common/failable.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace resilient;

using FailureExample = char;

TEST(failures, ReturnMatches)
{
    Returns<int> equal3(3);
    auto f = []() { return make_failable<FailureExample>(3);};
    auto value = equal3(f);
    EXPECT_TRUE(value.isFailure());
}

TEST(failures, ReturnNotMatches)
{
    Returns<int> equal3(3);
    auto f = []() { return make_failable<FailureExample>(2);};
    auto value = equal3(f);
    EXPECT_TRUE(value.isValue());
}

TEST(failures, AnyNoMatches)
{
    Returns<int> equal3(3);
    Returns<int> equal4(4);
    auto f = []() { return make_failable<FailureExample>(2);};
    auto value = anyOf(equal3, equal4)(f);
    EXPECT_TRUE(value.isValue());
}

TEST(failures, AnyMatchesOne)
{
    Returns<int> equal3(3);
    Returns<int> equal4(4);
    auto f = []() { return make_failable<FailureExample>(3);};
    auto value = anyOf(equal3, equal4)(f);
    EXPECT_TRUE(value.isFailure());
}

TEST(failures, AnyMatchesAll)
{
    Returns<int> equal4_first(4);
    Returns<int> equal4_second(4);
    auto f = []() { return make_failable<FailureExample>(4);};
    auto value = anyOf(equal4_first, equal4_second)(f);
    EXPECT_TRUE(value.isFailure());
}

TEST(failures, NeverDoesNotMatch)
{
    Never never;
    auto f = []() { return make_failable<FailureExample>(2);};
    auto value = never(f);
    EXPECT_TRUE(value.isValue());
}