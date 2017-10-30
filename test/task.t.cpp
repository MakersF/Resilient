#include <resilient/task/task.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <resilient/detector/basedetector.hpp>

#include <string>

using namespace resilient;

namespace {

using ResultType = std::string;
struct FailureMock {};

struct Detector : FailureDetectorTag<FailureMock> // TODO Test with empty and with multiple failures
{
    NoState preRun()
    {
        return NoState();
    }

    MOCK_METHOD2(postRun, failure(NoState, ICallResult<ResultType>&));
};

class Callable
{
public:
    MOCK_METHOD0(call, ResultType());

    ResultType operator()() { return call(); }
};

using DetectorMock = testing::StrictMock<Detector>;
using CallableMock = testing::StrictMock<Callable>;
using TaskType = Task<CallableMock&, DetectorMock&>;

struct TaskReturnsResult_F : ::testing::Test
{
    TaskReturnsResult_F()
    : d_value("A test")
    , d_task(d_callable, d_detector)
    {}

    std::string d_value;
    CallableMock d_callable;
    DetectorMock d_detector;
    TaskType d_task;

    virtual void SetUp() override
    {
        EXPECT_CALL(d_callable, call())
        .WillOnce(testing::Return(d_value));
    }
};

struct TaskThrowsException_F : ::testing::Test
{
    TaskThrowsException_F()
    : d_exception("An exception")
    , d_task(d_callable, d_detector)
    {}

    std::runtime_error d_exception;
    CallableMock d_callable;
    DetectorMock d_detector;
    TaskType d_task;

    virtual void SetUp() override
    {
        EXPECT_CALL(d_callable, call())
        .WillOnce(testing::Throw(d_exception));
    }
};

}

TEST_F(TaskReturnsResult_F, When_NoFailureDetected_Then_ValueIsResult)
{
    EXPECT_CALL(d_detector, postRun(testing::_, testing::_))
    .WillOnce(testing::Return(NoFailure()));

    auto result = std::move(d_task)();

    EXPECT_TRUE(result.isValue());
    EXPECT_EQ(result.value(), d_value);
}

TEST_F(TaskReturnsResult_F, When_FailureIsDetected_Then_ReturnIsFailure)
{
    EXPECT_CALL(d_detector, postRun(testing::_, testing::_))
    .WillOnce(testing::Invoke([](NoState, ICallResult<ResultType>&){
        return FailureMock();
    }));

    auto result = std::move(d_task)();

    EXPECT_TRUE(result.isFailure());
}

TEST_F(TaskThrowsException_F, When_NoFailureDetected_Then_ThrowsSameException)
{
    EXPECT_CALL(d_detector, postRun(testing::_, testing::_))
    .WillOnce(testing::Return(NoFailure()));

    EXPECT_THROW(std::move(d_task)(), decltype(d_exception));
}

TEST_F(TaskThrowsException_F, When_FailureIsDetectedAndExceptionConsumed_Then_ReturnIsFailure)
{
    EXPECT_CALL(d_detector, postRun(testing::_, testing::_))
    .WillOnce(testing::Invoke([](NoState, ICallResult<ResultType>& result){
        result.consumeException();
        return FailureMock();
    }));

    auto result = std::move(d_task)();

    EXPECT_TRUE(result.isFailure());
}

TEST_F(TaskThrowsException_F, When_NoFailureDetectedAndExceptionConsumed_Then_ThrowUnknownTaskResult)
{
    EXPECT_CALL(d_detector, postRun(testing::_, testing::_))
    .WillOnce(testing::Invoke([](NoState, ICallResult<ResultType>& result){
        result.consumeException();
        return NoFailure();
    }));

    EXPECT_THROW(std::move(d_task)(), UnknownTaskResult);
}