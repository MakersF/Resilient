#include "resilient/policies/pipeline.hpp"
#include "resilient/policies/circuitbreaker.hpp"
#include "resilient/policies/retry.hpp"
#include "resilient/policies/noop.hpp"
#include "resilient/common/failable.hpp"

#include <type_traits>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace resilient;

struct FailureType{};
using FailableType = Failable<FailureType, int>;
using FT = FailableTraits<FailableType>;
static const FailableType RESULT = make_failable<FailureType>(int);

class Callable
{
public:
    MOCK_METHOD0(call, FailableType());
};

class Policies : public testing::Test
{
protected:
    testing::StrictMock<Callable> d_callable;
};

TEST_F(Policies, Noop)
{
    EXPECT_CALL(d_callable, call())
        .WillOnce(testing::Invoke([&RESULT]() { return RESULT; } ));

    Noop noop;
    auto result = noop(d_callable);

    bool isSame = std::is_same<decltype(result), Result>::value;
    EXPECT_TRUE(isSame);
    EXPECT_TRUE(FT::isSuccess(result));
    EXPECT_EQ(FT::getValue(result), Result{0});
}

TEST_F(Policies, Retries)
{
    EXPECT_CALL(d_callable, call()).Times(1);

    Noop noop;
    noop(d_callable);
}