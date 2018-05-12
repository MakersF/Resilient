#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <resilient/policy/ratelimiterstrategy/blockingfixedconcurrentexecutionsstrategy.hpp>

#include <chrono>

using namespace resilient;
using namespace std::chrono_literals;

TEST(BlockingFixedConcurrentExecutionsStrategy,
     When_PermitsAreAvailableAndOneIsRequested_Then_APermitIsIssued)
{
    BlockingFixedConcurrentExecutionsStrategy bmces{1, 1ms};
    auto permit = bmces.acquire();
    EXPECT_TRUE(holds_alternative<MaxConcurrentPermit>(permit));
    bmces.release(std::move(get<MaxConcurrentPermit>(permit)));
}

TEST(BlockingFixedConcurrentExecutionsStrategy,
     When_PermitsAreNotAvailableAndOneIsRequested_Then_ErrorIsReturnedAfterTimeout)
{
    auto timeout = 1ms;
    BlockingFixedConcurrentExecutionsStrategy bmces{0, timeout};
    auto before = std::chrono::steady_clock::now();
    auto permit = bmces.acquire();
    auto after = std::chrono::steady_clock::now();
    EXPECT_TRUE(holds_alternative<PermitAcquireTimeout>(permit));
    EXPECT_GE(after - before, timeout);
}

TEST(BlockingFixedConcurrentExecutionsStrategy,
     When_ReturningAPreviouslyIssuedPermit_Then_AnotherRequestCanGetIt)
{
    BlockingFixedConcurrentExecutionsStrategy bmces{1, 1ms};
    auto permit = bmces.acquire();
    auto expected_failure = bmces.acquire();
    EXPECT_TRUE(holds_alternative<PermitAcquireTimeout>(expected_failure));
    bmces.release(std::move(get<MaxConcurrentPermit>(permit)));

    auto expected_success = bmces.acquire();
    EXPECT_TRUE(holds_alternative<MaxConcurrentPermit>(expected_success));
    bmces.release(std::move(get<MaxConcurrentPermit>(expected_success)));
}