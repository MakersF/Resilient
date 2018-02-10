#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <resilient/policy/circuitbreaker.hpp>
#include <test/policy/policy_common.t.hpp>

using namespace policy_test;
using namespace resilient;

namespace {

struct CircuiBreakerStrategyMock : ICircuitBreakerStrategy
{
    MOCK_METHOD0(allowCall, bool());
    MOCK_METHOD0(registerFailure, void());
    MOCK_METHOD0(registerSuccess, void());
};

} // namespace

TEST_F(MultiPolicies, CircuitBreakerOpenReturnsError)
{
    std::unique_ptr<CircuiBreakerStrategyMock> strategy{
        new testing::StrictMock<CircuiBreakerStrategyMock>()};

    EXPECT_CALL(d_callable, call()).Times(0);
    EXPECT_CALL(*strategy, allowCall()).WillOnce(testing::Return(false));

    CircuitBreaker cb(std::move(strategy));
    auto result = cb.execute(d_callable);
    EXPECT_TRUE(holds_failure(result));
    EXPECT_TRUE(holds_alternative<CircuitBreakerIsOpen>(get_failure(result)));
}