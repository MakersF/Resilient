#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>

#include <resilient/policy/ratelimiter.hpp>

namespace resilient {

/// @brief Error returned when acquiring a permit for the Ratelimiter takes too long.
struct PermitAcquireTimeout
{
};

/// @brief A permit for a concurrent execution.
struct MaxConcurrentPermit
{
    // We don't require any state here
};

class BlockingFixedConcurrentExecutionsStrategy
: IRateLimiterStrategy<MaxConcurrentPermit, PermitAcquireTimeout>
{
public:
    BlockingFixedConcurrentExecutionsStrategy(unsigned long maxConcurrentExecutions,
                                              std::chrono::microseconds maxWaitTime)
    : d_remainingTokens(maxConcurrentExecutions), d_maxWaitTime(maxWaitTime)
    {
    }

    virtual Variant<MaxConcurrentPermit, PermitAcquireTimeout> acquire() override
    {
        std::unique_lock<std::mutex> lock(d_mutex);
        if (d_condition.wait_for(lock, d_maxWaitTime, [this]() { return d_remainingTokens > 0; })) {
            --d_remainingTokens;
            return MaxConcurrentPermit{};
        }
        return PermitAcquireTimeout{};
    }

    virtual void release(MaxConcurrentPermit) override
    {
        std::unique_lock<std::mutex> lock(d_mutex);
        d_remainingTokens++;
        d_condition.notify_one();
    }

private:
    std::mutex d_mutex;
    std::condition_variable d_condition;
    unsigned long d_remainingTokens;
    std::chrono::microseconds d_maxWaitTime;
};

} // namespace resilient