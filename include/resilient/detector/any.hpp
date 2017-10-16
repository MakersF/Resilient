#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <initializer_list>


#include <resilient/common/foldinvoke.hpp>
#include <resilient/detector/basedetector.hpp>

namespace resilient {

namespace detail {

template<typename Conds, size_t ...I>
auto callPreRun(Conds& conditions, std::index_sequence<I...> index)
{
    // Use initializer list to guarantee the order
    return std::tuple<decltype(std::get<I>(conditions).preRun())...>{std::get<I>(conditions).preRun()...};
}

template<typename Conds, typename ...States, typename OperationResult, typename FailureSignal, size_t ...I>
void callPostRun(Conds& conditions, std::tuple<States...>&& state, const OperationResult& result, FailureSignal& signal, std::index_sequence<I...> index)
{

    // Initializer list of comma expression with the first casted to void to prevent operator comma to be called
    (void) std::initializer_list<int> {
        ((void) std::get<I>(conditions).postRun(
            std::move(std::get<I>(state)),
            result,
            signal
        ), 0)...};
}

}

template<typename ...FailureConditions>
class Any : public FailureDetectorTag<typename std::decay_t<FailureConditions>::failure_types...>
{
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

    template<typename ...States, typename OperationResult, typename FailureSignal>
    void postRun(std::tuple<States...>&& state, const OperationResult& result, FailureSignal& signal)
    {
        detail::callPostRun(d_failureConditions, std::move(state), result, signal, std::make_index_sequence<sizeof...(FailureConditions)>());
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