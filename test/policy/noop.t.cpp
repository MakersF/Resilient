#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <resilient/policy/noop.hpp>
#include <test/policy/policy_common.t.hpp>

using namespace policy_test;
using namespace resilient;

TEST_F(SinglePolicies, Noop)
{
    EXPECT_CALL(d_callable, call()).WillOnce(testing::Return(SingleFailureFailable(0)));

    Noop noop;
    auto result = noop.execute(d_callable);

    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 0);
}