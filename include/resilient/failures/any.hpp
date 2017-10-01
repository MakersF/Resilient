#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <resilient/common/foldinvoke.hpp>
#include <resilient/failures/base_failure.hpp>

namespace resilient {

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

    // Callable must return a Failable
    template<typename Callable, typename ...Args>
    decltype(auto) operator()(Callable&& callable, Args&&... args)
    {
        // TODO rewrite to be iterative.
        // refactor so that a failure condition defines a pre/post and return whether it failed or not
        return foldInvoke(
            d_failureConditions,
            std::forward<Callable>(callable),
            std::forward<Args>(args)...
        );
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