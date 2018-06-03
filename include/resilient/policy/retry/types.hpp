#pragma once

#include <chrono>

namespace resilient {
namespace retry {

/**
 * @brief Tag type which contains a Failure type.
 * @related resilient::Retry
 *
 * The failure is the one that can be returned by the task which is being retired
 * and to which the retry_state is associated.
 *
 * @tparam Failure The type of the failure.
 */
template<typename Failure>
struct retriedtask_failure
{
    using type = Failure;
};

/**
 * @brief Specify how long to wait before the next retry.
 * @related resilient::Retry
 */
struct retry_after
{
    std::chrono::microseconds value;
};

} // namespace retry
} // namespace resilient