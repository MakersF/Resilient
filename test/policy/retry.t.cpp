#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <resilient/policy/retry.hpp>
#include <test/policy/policy_common.t.hpp>

using namespace policy_test;
using namespace resilient;

TEST_F(SinglePolicies, RetryPolicySuccessAfterFewTries)
{
    testing::Sequence s;
    EXPECT_CALL(d_callable, call())
        .WillOnce(testing::Return(SingleFailureFailable{Failure()}))
        .WillOnce(testing::Return(SingleFailureFailable(1)));

    RetryPolicy retry(1);

    auto result = retry.execute(d_callable);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 1);
}

TEST_F(SinglePolicies, RetryPolicyNeverSucceeds)
{
    EXPECT_CALL(d_callable, call())
        .Times(2)
        .WillRepeatedly(testing::Return(SingleFailureFailable{Failure()}));

    RetryPolicy retry(1);

    auto result = retry.execute(d_callable);
    EXPECT_TRUE(holds_failure(result));
    EXPECT_TRUE(holds_alternative<NoMoreRetriesAvailable>(get_failure(result)));
}