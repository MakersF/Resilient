#include <resilient/task/task.hpp>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <resilient/detector/basedetector.hpp>

#include <string>

using namespace resilient;

namespace {

using ResultType = std::string;
struct FailureMock {};

struct DetectorMock : FailureDetectorTag<FailureMock> // Test with empty and with multiple
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

struct Task_F : ::testing::Test
{
    testing::StrictMock<Callable> d_callable;
    testing::StrictMock<DetectorMock> d_detector;
};

}

TEST_F(Task_F, TaskReturnsResultWithSuccess)
{
    EXPECT_CALL(d_callable, call())
    .WillOnce(testing::Return("A test"));

    EXPECT_CALL(d_detector, postRun(testing::_, testing::_))
    .WillOnce(testing::Return(NoFailure()));

    auto tsk = task(d_callable).failsIf(d_detector);
    auto result = std::move(tsk)();

    EXPECT_TRUE(result.isValue());
    EXPECT_EQ(result.value(), "A test");
}

TEST_F(Task_F, TaskReturnsFailureIfDetected)
{
    EXPECT_CALL(d_callable, call())
    .WillOnce(testing::Return("A test"));

    EXPECT_CALL(d_detector, postRun(testing::_, testing::_))
    .WillOnce(testing::Invoke([](NoState, ICallResult<ResultType>& result){
        result.consumeException();
        return FailureMock();
    }));

    auto tsk = task(d_callable).failsIf(d_detector);
    auto result = std::move(tsk)();

    EXPECT_TRUE(result.isFailure());
}

TEST_F(Task_F, TaskReturnsFailureIfThrows)
{
    EXPECT_CALL(d_callable, call())
    .WillOnce(testing::Throw(std::runtime_error("An error")));

    EXPECT_CALL(d_detector, postRun(testing::_, testing::_))
    .WillOnce(testing::Return(NoFailure()));

    auto tsk = task(d_callable).failsIf(d_detector);
    auto result = std::move(tsk)();

    EXPECT_TRUE(result.isFailure());
}