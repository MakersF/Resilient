#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <resilient/policy/retry.hpp>
#include <test/policy/policy_common.t.hpp>

using namespace policy_test;
using namespace resilient;

struct RetryStateMock
{
    MOCK_METHOD0(shouldExecute, bool());
    MOCK_METHOD1(failedWith, void(Failure));
};

using RetryFactory = CopyRetryStateFactory<::testing::StrictMock<RetryStateMock>&>;

// TODO test that the state is returned correctly to endExecution()

TEST_F(SinglePolicies, When_CallFailsAndStrategyAllowsRetry_Then_CallIsMadeAgain)
{
    EXPECT_CALL(d_callable, call())
        .WillOnce(testing::Return(SingleFailureFailable{Failure()}))
        .WillOnce(testing::Return(SingleFailureFailable(1)));

    ::testing::StrictMock<RetryStateMock> stateMock;
    EXPECT_CALL(stateMock, shouldExecute()).Times(2).WillRepeatedly(testing::Return(true));
    EXPECT_CALL(stateMock, failedWith(testing::A<Failure>())).Times(1);

    Retry<RetryFactory> retry{RetryFactory(stateMock)};

    auto result = retry.execute(d_callable);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 1);
}

TEST_F(SinglePolicies, When_StrategyDoesNotAllowRetry_Then_AnErrorIsReturned)
{
    EXPECT_CALL(d_callable, call()).Times(0);

    ::testing::StrictMock<RetryStateMock> stateMock;
    EXPECT_CALL(stateMock, shouldExecute()).WillOnce(testing::Return(false));

    Retry<RetryFactory> retry{RetryFactory(stateMock)};

    auto result = retry.execute(d_callable);
    EXPECT_TRUE(holds_failure(result));
    EXPECT_TRUE(holds_alternative<NoMoreRetriesAvailable>(get_failure(result)));
}