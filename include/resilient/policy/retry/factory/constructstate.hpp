#pragma once

#include <tuple>
#include <utility>

#include <resilient/policy/retry/types.hpp>

namespace resilient {
namespace retry {

namespace detail {

template<typename F, typename Tuple, std::size_t... I>
F construct_from_tuple(const Tuple& t, std::index_sequence<I...>)
{
    return F{std::get<I>(t)...};
}

} // namespace detail

/**
 * @brief A factory which construct a RetryState each time a new one is requested.
 * @related resilient::Retry
 *
 * @note
 * Implements the `RetryStateFactory` concept.
 *
 * @tparam RetryState The type of the RetryState constructed and returned.
 * @tparam Args... The arguments to construct the retry state.
 */
template<typename RetryState, typename... Args>
class ConstructState
{
public:
    ConstructState(Args... args) : d_stateArguments(std::forward<Args>(args)...) {}

    template<typename Failure>
    RetryState getRetryState(retriedtask_failure<Failure>)
    {
        return detail::construct_from_tuple<RetryState>(
            d_stateArguments, std::make_index_sequence<sizeof...(Args)>{});
    }

    void returnRetryState(RetryState) {}

private:
    std::tuple<Args...> d_stateArguments;
};

/**
 * @brief Create an instance of ConstructState with the given state and arguments.
 *
 * @tparam RetryState The state the factory will construct
 * @param args The arguments to use to construct
 * @return the factory
 */
template<typename RetryState, typename... Args>
ConstructState<RetryState, Args...> constructstate(Args&&... args)
{
    return ConstructState<RetryState, Args...>(std::forward<Args>(args)...);
}

} // namespace retry
} // namespace resilient