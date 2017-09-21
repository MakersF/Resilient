#include <resilient/task/task.hpp>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <string>

using namespace resilient;

namespace {

struct None {};

class Callable
{
public:
    MOCK_METHOD0(call, std::string());

    std::string operator()()
    {
        return call();
    }
};

}

TEST(task, TaskReturnsFailableWithSuccess)
{
    testing::StrictMock<Callable> callable;

    EXPECT_CALL(callable, call())
    .WillOnce(testing::Return("A test"));

    auto tsk = task(callable).failsIf(None());
    auto result = std::move(tsk)();

    EXPECT_TRUE(result.isValue());
    EXPECT_EQ(result.value(), "A test");
}

TEST(task, TaskReturnsFailure)
{
    testing::StrictMock<Callable> callable;

    EXPECT_CALL(callable, call())
    .WillOnce(testing::Throw(std::runtime_error("An error")));

    auto tsk = task(callable).failsIf(None());
    auto result = std::move(tsk)();

    EXPECT_TRUE(result.isFailure());
}