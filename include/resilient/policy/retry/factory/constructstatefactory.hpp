#pragma once

#include <tuple>
#include <utility>

#include <resilient/policy/retry/types.hpp>

namespace resilient {

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
class ConstructRetryStateFactory
{
public:
    ConstructRetryStateFactory(Args... args) : d_stateArguments(std::forward<Args>(args)...) {}

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
 * @brief A factory which construct a RetryState from a template each time a new one is requested.
 * @related resilient::Retry
 *
 * @note
 * Implements the `RetryStateFactory` concept.
 *
 * @tparam RetryStateTemplate The template parametrized on the Failure returned by the task.
 * @tparam Args... The arguments to construct the retry state
 */
template<template<typename> class RetryStateTemplate, typename... Args>
class ConstructTemplatedRetryStateFactory
{
public:
    ConstructTemplatedRetryStateFactory(Args... args)
    : d_stateArguments(std::forward<Args>(args)...)
    {
    }

    template<typename Failure>
    RetryStateTemplate<Failure> getRetryState(retriedtask_failure<Failure>)
    {
        return detail::construct_from_tuple<RetryStateTemplate<Failure>>(
            d_stateArguments, std::make_index_sequence<sizeof...(Args)>{});
    }

    template<typename Failure>
    void returnRetryState(RetryStateTemplate<Failure>)
    {
    }

private:
    std::tuple<Args...> d_stateArguments;
};

} // namespace resilient