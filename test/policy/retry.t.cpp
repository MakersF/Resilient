#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <chrono>
#include <type_traits>

#include <resilient/policy/retry/factory/referencestate.hpp>
#include <resilient/policy/retry/retry.hpp>
#include <test/policy/policy_common.t.hpp>

using namespace policy_test;
using namespace resilient;
using namespace std::chrono_literals;

struct NoMoreRetriesAvailable
{
};

struct RetryStateMock
{
    using stopretries_type = NoMoreRetriesAvailable;

    MOCK_METHOD0(shouldRetry, Variant<retry::retry_after, NoMoreRetriesAvailable>());
    MOCK_METHOD1(failedWith, void(Failure));
};

using RetryFactory = retry::ReferenceState<::testing::StrictMock<RetryStateMock>>;

// TODO test that the state is returned correctly to endExecution()

TEST_F(SinglePolicies, When_CallFailsAndStrategyAllowsRetry_Then_CallIsMadeAgain)
{
    EXPECT_CALL(d_callable, call())
        .WillOnce(testing::Return(SingleFailureFailable{Failure()}))
        .WillOnce(testing::Return(SingleFailureFailable(1)));

    ::testing::StrictMock<RetryStateMock> stateMock;
    EXPECT_CALL(stateMock, failedWith(testing::A<Failure>())).Times(1);
    EXPECT_CALL(stateMock, shouldRetry()).WillOnce(testing::Return(retry::retry_after{1us}));

    retry::Retry<RetryFactory> retry{RetryFactory(stateMock)};

    auto result = retry.execute(d_callable);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 1);
}

TEST_F(SinglePolicies, When_StrategyDoesNotAllowRetry_Then_AnErrorIsReturned)
{
    EXPECT_CALL(d_callable, call()).WillOnce(testing::Return(SingleFailureFailable{Failure()}));

    ::testing::StrictMock<RetryStateMock> stateMock;
    EXPECT_CALL(stateMock, failedWith(testing::A<Failure>())).Times(1);
    EXPECT_CALL(stateMock, shouldRetry()).WillOnce(testing::Return(NoMoreRetriesAvailable()));

    retry::Retry<RetryFactory> retry{RetryFactory(stateMock)};

    auto result = retry.execute(d_callable);
    EXPECT_TRUE(holds_failure(result));
    static_assert(std::is_same<NoMoreRetriesAvailable&, decltype(get_failure(result))>::value,
                  "Expected type");
}