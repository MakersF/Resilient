#include "resilient/policies/pipeline.hpp"
#include "resilient/policies/circuitbreaker.hpp"
#include "resilient/policies/retry.hpp"
#include "resilient/policies/noop.hpp"
#include "resilient/common/failable.hpp"

#include <type_traits>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace resilient;

namespace {

struct FailureType {};

using FailableType = Failable<FailureType, int>;
using FT = FailableTraits<FailableType>;


class Callable
{
public:
    MOCK_METHOD0(call, FailableType());

    FailableType operator()()
    {
        return call();
    }
};


class Policies : public testing::Test
{
protected:
    testing::StrictMock<Callable> d_callable;
};

}

TEST_F(Policies, Noop)
{
    EXPECT_CALL(d_callable, call())
    .WillOnce(testing::Return(make_failable<FailureType>(0)));

    Noop noop;
    auto result = noop(d_callable);

    EXPECT_TRUE(FT::isSuccess(result));
    EXPECT_EQ(FT::getValue(result), 0);
}

TEST_F(Policies, RetryPolicySuccessAfterFewTries)
{
    testing::Sequence s;
    EXPECT_CALL(d_callable, call())
        .WillOnce(testing::Return(FT::failure()))
        .WillOnce(testing::Return(FT::failure()))
        .WillOnce(testing::Return(FT::failure()))
        .WillOnce(testing::Return(make_failable<FailureType>(1)));

    RetryPolicy retry(5);

    auto result = retry(d_callable);
    EXPECT_TRUE(FT::isSuccess(result));
    EXPECT_EQ(FT::getValue(result), 1);
}

TEST_F(Policies, RetryPolicyNeverSucceeds)
{
    EXPECT_CALL(d_callable, call())
        .Times(3)
        .WillRepeatedly(testing::Return(FT::failure()));

    RetryPolicy retry(3);

    auto result = retry(d_callable);
    EXPECT_TRUE(FT::isFailure(result));
}