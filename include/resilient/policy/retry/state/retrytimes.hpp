#pragma once

#include <chrono>
#include <cstdint>
#include <limits>

#include <resilient/common/variant.hpp>
#include <resilient/policy/retry/types.hpp>

namespace resilient {

namespace detail {

struct RetryAlways
{
};
struct RetryNever
{
};

} // namespace detail

/**
 * @brief Use to instantiate a `RetryTimes` that always retries.
 * @related resilient::RetryTimes
 */
static constexpr detail::RetryAlways retry_always{};
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
    RetryTimes(detail::RetryAlways) : d_retriesLeft(std::numeric_limits<std::uint64_t>::max()) {}
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
    std::uint64_t d_retriesLeft;
};

} // namespace resilient