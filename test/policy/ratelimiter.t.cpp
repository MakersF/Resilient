#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <resilient/policy/ratelimiter.hpp>
#include <test/policy/policy_common.t.hpp>

using namespace policy_test;
using namespace resilient;

namespace {

struct RateLimiterStrategyMock : IRateLimiterStrategy
{
    MOCK_METHOD0(acquire, IRateLimiterStrategy::permit_ptr());
    MOCK_METHOD1(release, void(IRateLimiterStrategy::permit_ptr));
};

} // namespace

TEST_F(SinglePolicies, AcquireReleaseAreInvoked)
{
    std::unique_ptr<RateLimiterStrategyMock> strategy{
        new testing::StrictMock<RateLimiterStrategyMock>()};

    int permit;
    EXPECT_CALL(d_callable, call()).WillOnce(testing::Return(SingleFailureFailable(Failure())));
    EXPECT_CALL(*strategy, acquire()).WillOnce(testing::Return(&permit));
    EXPECT_CALL(*strategy, release(testing::Eq<IRateLimiterStrategy::permit_ptr>(&permit)))
        .Times(1);

    Ratelimiter rl(std::move(strategy));
    auto result = rl.execute(d_callable);
    EXPECT_TRUE(holds_failure(result));
}