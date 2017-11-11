#include "resilient/policy/pipeline.hpp"
#include "resilient/policy/circuitbreaker.hpp"
#include "resilient/policy/retry.hpp"
#include "resilient/policy/noop.hpp"
#include "resilient/common/failable.hpp"

#include <type_traits>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace resilient;

namespace {

struct Failure {};
struct OtherFailure {};

using SingleFailureFailable = Failable<Failure, int>;
using MultipleFailureFailable = Failable<Variant<Failure, OtherFailure>, int>;

template<typename Failable>
class Callable
{
public:
    MOCK_METHOD0_T(call, Failable());

    Failable operator()()
    {
        return call();
    }
};


class SinglePolicies : public testing::Test
{
protected:
    testing::StrictMock<Callable<SingleFailureFailable>> d_callable;
};

class MultiPolicies : public testing::Test
{
protected:
    testing::StrictMock<Callable<MultipleFailureFailable>> d_callable;
};

}

TEST_F(SinglePolicies, Noop)
{
    EXPECT_CALL(d_callable, call())
    .WillOnce(testing::Return(SingleFailureFailable(0)));

    Noop noop;
    auto result = noop(d_callable);

    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 0);
}

TEST_F(SinglePolicies, RetryPolicySuccessAfterFewTries)
{
    testing::Sequence s;
    EXPECT_CALL(d_callable, call())
        .WillOnce(testing::Return(SingleFailureFailable{Failure()}))
        .WillOnce(testing::Return(SingleFailureFailable(1)));

    RetryPolicy retry(1);

    auto result = retry(d_callable);
    EXPECT_TRUE(holds_value(result));
    EXPECT_EQ(get_value(result), 1);
}

TEST_F(SinglePolicies, RetryPolicyNeverSucceeds)
{
    EXPECT_CALL(d_callable, call())
        .Times(2)
        .WillRepeatedly(testing::Return(SingleFailureFailable{Failure()}));

    RetryPolicy retry(1);

    auto result = retry(d_callable);
    EXPECT_TRUE(holds_failure(result));
    EXPECT_TRUE(holds_alternative<NoMoreRetriesAvailable>(get_failure(result)));
}

TEST_F(MultiPolicies, CircuitBreakerOpenReturnsError)
{
    EXPECT_CALL(d_callable, call()).Times(0);

    CircuitBreaker cb(true);

    auto result = cb(d_callable);
    EXPECT_TRUE(holds_failure(result));
    EXPECT_TRUE(holds_alternative<CircuitBreakerIsOpen>(get_failure(result)));
}