#pragma once

#include <resilient/common/variant.hpp>
#include <resilient/policy/retry/types.hpp>

namespace resilient {
namespace retry {

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
public:
    using stopretries_type = WillNeverHappenToStopRetries;

    Variant<retry_after, stopretries_type> shouldRetry()
    {
        return retry_after{std::chrono::microseconds(0)};
    }

    template<typename T>
    void failedWith(T)
    {
    }
};

} // namespace retry
} // namespace resilient
