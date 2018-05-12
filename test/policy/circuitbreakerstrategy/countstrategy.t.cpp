#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <resilient/policy/circuitbreakerstrategy/countstrategy.hpp>

#include <chrono>

using namespace resilient;
using namespace std::chrono_literals;
using ::testing::Return;

struct ClockMockState;
using StrictClockMockState = ::testing::StrictMock<ClockMockState>;

struct ClockMock
{
    typedef uint64_t rep;
    typedef std::ratio<1l, 1000000000l> period;
    typedef std::chrono::duration<rep, period> duration;
    typedef std::chrono::time_point<ClockMock> time_point;

    ClockMock(StrictClockMockState* state) : d_state(state) {}

    time_point now();

    StrictClockMockState* d_state;
};

using time_point = ClockMock::time_point;
using duration = ClockMock::duration;

struct ClockMockState
{
    MOCK_CONST_METHOD0(now, time_point());
};

time_point ClockMock::now() { return d_state->now(); }

StrictClockMockState* setStrategyCtorExpectations(StrictClockMockState* state)
{
    EXPECT_CALL(*state, now()).WillOnce(Return(time_point(0s)));
    return state;
}

struct CountStrategy_F : ::testing::Test
{
    CountStrategy_F()
    : d_clockState()
    , d_failureThreshold(5)
    , d_failureInterval(60)
    , d_tripDuration(30)
    , d_recoverSuccesses(2)
    , d_strategy(d_failureThreshold,
                 d_failureInterval,
                 d_tripDuration,
                 d_recoverSuccesses,
                 ClockMock(setStrategyCtorExpectations(&d_clockState)))
    , d_currentTime(0)
    {
        EXPECT_CALL(d_clockState, now())
            .WillRepeatedly(::testing::Invoke(this, &CountStrategy_F::testTime));
    }

    StrictClockMockState d_clockState;
    unsigned long d_failureThreshold;
    std::chrono::seconds d_failureInterval;
    std::chrono::seconds d_tripDuration;
    unsigned long d_recoverSuccesses;
    CountStrategy<ClockMock> d_strategy;

    // Used to trace time in the tests
    std::chrono::milliseconds d_currentTime;

    time_point testTime() { return time_point(d_currentTime); }

    void advanceTime(std::chrono::milliseconds delta = std::chrono::milliseconds(1))
    {
        d_currentTime += delta;
    }

    void triggerShortcircuit()
    {
        for (unsigned int i = 0; i < d_failureThreshold; i++) {
            advanceTime();
            d_strategy.registerFailure();
        }
    }
};

TEST_F(CountStrategy_F, Given_IsOpen_When_RequestsSucceed_Then_CallsAreAllowed)
{
    for (unsigned int i = 0; i < d_failureThreshold * 2; i++) {
        EXPECT_TRUE(d_strategy.allowCall());
        d_strategy.registerSuccess();
    }
}

TEST_F(CountStrategy_F, Given_IsOpen_When_LessThanThresholdFailsInInterval_Then_CallsAreAllowed)
{
    for (unsigned int i = 0; i < d_failureThreshold; i++) {
        EXPECT_TRUE(d_strategy.allowCall());
        advanceTime();

        d_strategy.registerFailure();
    }
}

TEST_F(CountStrategy_F, Given_IsOpen_When_RequestsFail_Then_CallsAreShortcircuited)
{
    for (unsigned int i = 0; i < d_failureThreshold; i++) {
        EXPECT_TRUE(d_strategy.allowCall());

        advanceTime();

        d_strategy.registerFailure();
    }

    EXPECT_CALL(d_clockState, now()).WillOnce(Return(testTime()));
    EXPECT_FALSE(d_strategy.allowCall());
}

TEST_F(CountStrategy_F, Given_IsShortcircuited_When_RequestArriveInInterval_Then_NotAllow)
{
    triggerShortcircuit();
    for (unsigned int i = 0; i < 100; i++) {
        advanceTime();
        EXPECT_FALSE(d_strategy.allowCall());
    }
}

TEST_F(CountStrategy_F, Given_IsShortcircuited_When_EnoughTimeIsPassed_Then_Allow)
{
    triggerShortcircuit();
    advanceTime(d_tripDuration);
    EXPECT_TRUE(d_strategy.allowCall());
}