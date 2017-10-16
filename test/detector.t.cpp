#include <resilient/detector/execution_context.hpp>
#include <resilient/detector/returns.hpp>
#include <resilient/detector/any.hpp>
#include <resilient/detector/never.hpp>
#include <resilient/common/failable.hpp>

#include <utility>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace resilient;

using FailureExample = char;

struct ReturnsFailureSignalMock : FailureSignal<Returns<int>::failure_types>
{
    MOCK_METHOD1(signalFailure, void(const ErrorReturn&));
    void signalFailure(ErrorReturn&& er) { signalFailure(er); }
};

struct NeverFailureSignalMock : FailureSignal<Never::failure_types>
{
    // Derives from an empty interface
};

struct FailureDetector_F : testing::Test
{
    FailureDetector_F()
    : d_resultValue(3)
    , d_result(d_resultValue)
    { }

    int d_resultValue;
    OperationResult<int> d_result;
    testing::StrictMock<ReturnsFailureSignalMock> d_failureSignal;
};

TEST_F(FailureDetector_F, ReturnMatches)
{
    Returns<int> equals(3);
    EXPECT_CALL(d_failureSignal, signalFailure(testing::_)).Times(1);

    auto state = equals.preRun();
    equals.postRun(std::move(state), d_result, d_failureSignal);
}

TEST_F(FailureDetector_F, ReturnDoesntMatch)
{
    Returns<int> equals(4);
    EXPECT_CALL(d_failureSignal, signalFailure(testing::_)).Times(0);

    auto state = equals.preRun();
    equals.postRun(std::move(state), d_result, d_failureSignal);
}

TEST_F(FailureDetector_F, AnyNoMatches)
{
    Returns<int> equal1(4);
    Returns<int> equal2(5);
    auto any = anyOf(equal1, equal2);
    EXPECT_CALL(d_failureSignal, signalFailure(testing::_)).Times(0);

    auto state = any.preRun();
    any.postRun(std::move(state), d_result, d_failureSignal);
}

TEST_F(FailureDetector_F, AnyMatchesOne)
{
    Returns<int> equal1(3);
    Returns<int> equal2(5);
    auto any = anyOf(equal1, equal2);
    EXPECT_CALL(d_failureSignal, signalFailure(testing::_)).Times(1);

    auto state = any.preRun();
    any.postRun(std::move(state), d_result, d_failureSignal);
}

TEST_F(FailureDetector_F, AnyMatchesAll)
{
    Returns<int> equal1(3);
    Returns<int> equal2(3);
    auto any = anyOf(equal1, equal2);
    EXPECT_CALL(d_failureSignal, signalFailure(testing::_)).Times(2);

    auto state = any.preRun();
    any.postRun(std::move(state), d_result, d_failureSignal);
}

TEST(failures, NeverDoesNotMatch)
{
    int resultValue = 3;
    OperationResult<int> result(resultValue);
    NeverFailureSignalMock failureSignal;
    Never never;

    auto state = never.preRun();
    never.postRun(state, result, failureSignal);
}