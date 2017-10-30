#include <resilient/detector/execution_context.hpp>
#include <resilient/detector/returns.hpp>
#include <resilient/detector/any.hpp>
#include <resilient/detector/never.hpp>
#include <resilient/detector/always.hpp>
#include <resilient/common/failable.hpp>

#include <utility>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

using namespace resilient;

using FailureExample = char;

template<typename T>
struct CallResult: ICallResult<T>
{
    MOCK_METHOD0(consumeException, void());
    MOCK_CONST_METHOD0(isException, bool());
    MOCK_CONST_METHOD0(getException, const std::exception_ptr&());
    MOCK_CONST_METHOD0_T(getResult, const T&());
};

template<typename T>
using CallResultMock = testing::StrictMock<CallResult<T>>;

struct ReturnsDetector_F : testing::Test
{
    ReturnsDetector_F()
    : d_value(3)
    , d_returns(d_value)
    { }

    int d_value;
    Returns<int&> d_returns;
    CallResultMock<int> d_callresult;
};

TEST_F(ReturnsDetector_F, ReturnMatches)
{
    EXPECT_CALL(d_callresult, isException())
    .WillOnce(testing::Return(false));

    EXPECT_CALL(d_callresult, getResult())
    .WillOnce(testing::ReturnRef(d_value));

    auto state = d_returns.preRun();
    auto detected_failure = d_returns.postRun(std::move(state), d_callresult);
    EXPECT_TRUE(holds_failure(detected_failure));
}

TEST_F(ReturnsDetector_F, ReturnDoesntMatch)
{
    EXPECT_CALL(d_callresult, isException())
    .WillOnce(testing::Return(false));

    EXPECT_CALL(d_callresult, getResult())
    .WillOnce(testing::ReturnRefOfCopy(d_value + 1));

    auto state = d_returns.preRun();
    auto detected_failure = d_returns.postRun(std::move(state), d_callresult);
    EXPECT_FALSE(holds_failure(detected_failure));
}

TEST(AnyDetector, When_NoneMatches_Then_NoFailureIsDetected)
{
    CallResultMock<int> callresult;
    Returns<int> equal1(4);
    Returns<int> equal2(5);
    auto any = anyOf(equal1, equal2);

    EXPECT_CALL(callresult, isException()).WillRepeatedly(testing::Return(false));
    EXPECT_CALL(callresult, getResult()).WillRepeatedly(testing::ReturnRefOfCopy(3));

    auto state = any.preRun();
    auto detected_failure = any.postRun(std::move(state), callresult);
    EXPECT_FALSE(holds_failure(detected_failure));
}

TEST(AnyDetector, When_FirstMatches_Then_SameFailureAsTheFirst)
{
    CallResultMock<int> callresult;
    Always always;
    Returns<int> equal(5);
    auto any = anyOf(always, equal);

    EXPECT_CALL(callresult, isException())
    .WillRepeatedly(testing::Return(false));

    EXPECT_CALL(callresult, getResult())
    .WillRepeatedly(testing::ReturnRefOfCopy(3));

    auto state = any.preRun();
    auto detected_failure = any.postRun(std::move(state), callresult);
    EXPECT_TRUE(holds_alternative<AlwaysError>(detected_failure));
}

TEST(AnyDetector, When_SecondMatches_Then_SameFailureAsTheSecond)
{
    CallResultMock<int> callresult;
    Always always;
    Returns<int> equal(5);
    auto any = anyOf(equal, always);

    EXPECT_CALL(callresult, isException())
    .WillRepeatedly(testing::Return(false));

    EXPECT_CALL(callresult, getResult())
    .WillRepeatedly(testing::ReturnRefOfCopy(3));

    auto state = any.preRun();
    auto detected_failure = any.postRun(std::move(state), callresult);
    EXPECT_TRUE(holds_alternative<AlwaysError>(detected_failure));
}


TEST(AnyDetector, When_MultipleMatches_Then_SameFailureAsTheFirstMatching)
{
    CallResultMock<int> callresult;
    Never never;
    Always always;
    Returns<int> equal(3);
    auto any = anyOf(never, equal, always);

    EXPECT_CALL(callresult, isException())
    .WillRepeatedly(testing::Return(false));

    EXPECT_CALL(callresult, getResult())
    .WillRepeatedly(testing::ReturnRefOfCopy(3));

    auto state = any.preRun();
    auto detected_failure = any.postRun(std::move(state), callresult);
    EXPECT_TRUE(holds_alternative<ErrorReturn>(detected_failure));
}

TEST(NeverDetector, DoesNotMatch)
{
    CallResultMock<int> callresult;
    Never never;

    auto state = never.preRun();
    auto detected_failure = never.postRun(std::move(state), callresult);
    EXPECT_FALSE(holds_failure(detected_failure));
}