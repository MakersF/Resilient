#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <resilient/common/variant.hpp>
#include <resilient/task/failable.hpp>

namespace policy_test {

struct Failure
{
};

struct OtherFailure
{
};

using SingleFailureFailable = resilient::Failable<Failure, int>;
using MultipleFailureFailable = resilient::Failable<resilient::Variant<Failure, OtherFailure>, int>;

template<typename Failable>
class Callable
{
public:
    MOCK_METHOD0_T(call, Failable());

    Failable operator()() { return call(); }
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

} // namespace policy_test
