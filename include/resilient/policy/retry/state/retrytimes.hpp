#pragma once

#include <chrono>

#include <resilient/common/variant.hpp>
#include <resilient/policy/retry/types.hpp>

namespace resilient {

namespace detail {

struct RetryNever
{
};

} // namespace detail

/**
 * @brief Use to instantiate a `RetryTimes` that never retries.
 * @related resilient::RetryTimes
 */
static constexpr detail::RetryNever retry_never{};

/**
 * @brief Type returned when the state does not allow to retry anymore.
 * @related resilient::RetryTimes
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
class RetryTimes
{
public:
    using stopretries_type = NoMoreRetriesLeft;

    /**
     * @brief Construct a new Retry Times object which retries a fixed number of times
     *
     * @param numberOfRetries The number of retries
     */
    RetryTimes(unsigned int numberOfRetries) : d_retriesLeft(numberOfRetries) {}
    RetryTimes(detail::RetryNever) : d_retriesLeft(0) {}

    Variant<retry_after, NoMoreRetriesLeft> shouldRetry()
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

/**
 * @brief This type will never be returned because `resilient::AlwaysRetry` always retries.
 *
 */
struct WillNeverHappenToStopRetries
{
};

/**
 * @brief A State which always allows to retry.
 * @related resilient::Retry
 *
 * @note
 * Implements the `RetryState` concept.
 *
 */
class AlwaysRetry
{
    using stopretries_type = WillNeverHappenToStopRetries;

    Variant<retry_after, WillNeverHappenToStopRetries> shouldRetry()
    {
        return retry_after{std::chrono::microseconds(0)};
    }

    template<typename T>
    void failedWith(T)
    {
    }
};

} // namespace resilient