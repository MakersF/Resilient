#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <resilient/policy/noop.hpp>
#include <resilient/policy/pipeline.hpp>
#include <resilient/task/failable.hpp>
#include <test/policy/policy_common.t.hpp>

using namespace policy_test;
using namespace resilient;

TEST_F(SinglePolicies, Pipeline)
{
    EXPECT_CALL(d_callable, call()).WillOnce(testing::Return(SingleFailureFailable(0)));

    Noop noop;
    auto result =
        pipelineOf(noop).execute([callable_p = &d_callable]() { return (*callable_p)(); });

    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 0);
}