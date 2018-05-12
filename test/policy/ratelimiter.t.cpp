#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utility>

#include <resilient/policy/ratelimiter.hpp>
#include <test/policy/policy_common.t.hpp>

using namespace policy_test;
using namespace resilient;

namespace {

struct RateLimiterStrategyError
{
};

using IRateLimiterStrategyMock = IRateLimiterStrategy<int, RateLimiterStrategyError>;
struct RateLimiterStrategyMock : IRateLimiterStrategyMock
{
    using acquire_return_type = Variant<int, RateLimiterStrategyError>;

    MOCK_METHOD0(acquire, acquire_return_type());
    MOCK_METHOD1(release, void(int));
};

} // namespace

TEST_F(SinglePolicies, AcquireReleaseAreInvoked)
{
    std::unique_ptr<RateLimiterStrategyMock> strategy{
        new testing::StrictMock<RateLimiterStrategyMock>()};

    int token = 123;
    EXPECT_CALL(d_callable, call()).WillOnce(testing::Return(SingleFailureFailable(Failure())));
    EXPECT_CALL(*strategy, acquire())
        .WillOnce(testing::Return(RateLimiterStrategyMock::acquire_return_type{token}));
    EXPECT_CALL(*strategy, release(testing::Eq(token))).Times(1);

    Ratelimiter<RateLimiterStrategyMock> rl(std::move(strategy));
    auto result = rl.execute(d_callable);
    EXPECT_TRUE(holds_failure(result));
}

TEST_F(MultiPolicies, When_StrategyFailsToAcquire_Then_NoCallToCallableAndToRelease)
{
    std::unique_ptr<RateLimiterStrategyMock> strategy{
        new testing::StrictMock<RateLimiterStrategyMock>()};

    EXPECT_CALL(d_callable, call()).Times(0);
    EXPECT_CALL(*strategy, acquire())
        .WillOnce(testing::Return(
            RateLimiterStrategyMock::acquire_return_type{RateLimiterStrategyError()}));
    EXPECT_CALL(*strategy, release(testing::An<int>())).Times(0);

    Ratelimiter<RateLimiterStrategyMock> rl(std::move(strategy));
    auto result = rl.execute(d_callable);
    EXPECT_TRUE(holds_failure(result));
}