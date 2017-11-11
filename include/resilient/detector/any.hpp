#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <initializer_list>

#include <resilient/detector/basedetector.hpp>

namespace resilient {

namespace detail {

template<typename Conds, size_t ...I>
auto callPreRun(Conds& conditions, std::index_sequence<I...>)
{
    // Use initializer list to guarantee the order
    return std::tuple<decltype(std::get<I>(conditions).preRun())...>{std::get<I>(conditions).preRun()...};
}

// Return an int so it's easy to use in the initializer_list
template<typename Failure, typename FailureCondition, typename State, typename T>
int singlePostRun(Failure& mainFailure, FailureCondition& condition, State&& state, ICallResult<T>& result)
{
    // The failure detection should happen in the same order as the failureconditions.
    // This means that we need to keep the failure of the first detector which triggers it.
    // So, if the Failure is already set we don't set it again otherwise we would override with
    // a later failure.
    decltype(auto) currentFailure{condition.postRun(std::forward<State>(state), result)};
    if(not holds_failure(mainFailure))
    {
        mainFailure = std::move(currentFailure);
    }

    return 0;
}

template<typename Failure, typename ...FailureConditions, typename ...States, typename T, size_t ...I>
Failure callPostRun(std::tuple<FailureConditions...>& conditions,
                 std::tuple<States...>&& state,
                 ICallResult<T>& result,
                 std::index_sequence<I...>)
{
    Failure mainFailure{NoFailure()};
    // Initializer list of comma expression
    (void) std::initializer_list<int>{
        singlePostRun(mainFailure, std::get<I>(conditions), std::move(std::get<I>(state)), result)...
    };

    return std::move(mainFailure);
}

}

// Check a set of detectors for any of them to fail
template<typename ...FailureConditions>
// TODO remove duplicates from the list
class Any : public FailureDetectorByTupleTag<tuple_flatten_t<typename std::decay_t<FailureConditions>::failure_types...>>
{
private:
    // failure is a dependent type, so it needs to be qualified
    using Base = FailureDetectorByTupleTag<tuple_flatten_t<typename std::decay_t<FailureConditions>::failure_types...>>;
    using typename Base::failure;

public:
    template<typename FailureCondition>
    using after_adding_t = Any<FailureConditions..., FailureCondition>;

    template<typename FailureCondition>
    after_adding_t<FailureCondition>
    addCondition(FailureCondition&& failureCondition) &&
    {
        return after_adding_t<FailureCondition>(
            std::tuple_cat(
                std::move(d_failureConditions),
                std::make_tuple(std::forward<FailureCondition>(failureCondition))
            )
        );
    }

    auto preRun()
    {
        return detail::callPreRun(d_failureConditions, std::make_index_sequence<sizeof...(FailureConditions)>());
    }

    template<typename ...States, typename T>
    failure postRun(std::tuple<States...>&& state, ICallResult<T>& result)
    {
        return detail::callPostRun<failure>(d_failureConditions,
                                            std::move(state),
                                            result,
                                            std::make_index_sequence<sizeof...(FailureConditions)>());
    }

    explicit Any(std::tuple<FailureConditions...>&& failureConditions)
    : d_failureConditions(std::move(failureConditions))
    { }

private:
    std::tuple<FailureConditions...> d_failureConditions;
};

template<typename ...FailureConditions>
Any<FailureConditions...> anyOf(FailureConditions&&... conditions)
{
    return Any<FailureConditions...>(
        std::tuple<FailureConditions...>(
            std::forward<FailureConditions>(conditions)...));
}

}