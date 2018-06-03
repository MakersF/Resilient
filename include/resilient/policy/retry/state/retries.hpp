#pragma once

#include <chrono>

#include <resilient/common/variant.hpp>
#include <resilient/policy/retry/types.hpp>

namespace resilient {

/**
 * @brief Type returned when the state does not allow to retry anymore.
 * @related resilient::Retries
 */
struct NoMoreRetriesLeft
{
};

/**
 * @brief State to allow to retry a fixed number of times.
 * @related resilient::Retry
 *
 * Failures returned by the executed function are ignored.
 *
 * @note
 * Implements the `RetryState` concept.
 *
 */
class Retries
{
public:
    using stopretries_type = NoMoreRetriesLeft;

    /**
     * @brief Construct a new Retry Times object which retries a fixed number of times
     *
     * @param numberOfRetries The number of retries
     */
    Retries(unsigned int numberOfRetries) : d_retriesLeft(numberOfRetries) {}

    Variant<retry_after, stopretries_type> shouldRetry()
    {
        if (d_retriesLeft != 0) {
            d_retriesLeft--;
            return retry_after{std::chrono::microseconds(0)};
        }
        else
        {
            return NoMoreRetriesLeft();
        }
    }

    template<typename T>
    void failedWith(T)
    {
    }

private:
    unsigned int d_retriesLeft;
};

} // namespace resilient