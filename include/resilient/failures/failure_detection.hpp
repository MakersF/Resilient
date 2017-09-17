#ifndef RESILIENT_FAILURE_DETECTION_H
#define RESILIENT_FAILURE_DETECTION_H

#include <utility>
#include <tuple>
#include <type_traits>
#include <resilient/common/foldinvoke.hpp>
#include <resilient/common/result.hpp>

namespace resilient {

template<typename ...FailureConditions>
class FailureDetector
{
public:
    template<typename FailureCondition>
    struct after_adding
    {
        using type = FailureDetector<FailureConditions..., FailureCondition>;
    };

    template<typename FailureCondition>
    using after_adding_t = FailureDetector<FailureConditions..., FailureCondition>; //typename after_adding<FailureCondition>::type;

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


    explicit FailureDetector(std::tuple<FailureConditions...>&& failureConditions)
    : d_failureConditions(std::move(failureConditions))
    { }

private:

    std::tuple<FailureConditions...> d_failureConditions;
};

template<typename FailureCondition>
FailureDetector<FailureCondition> failsIf(FailureCondition&& condition)
{
    return FailureDetector<FailureCondition>(
        std::tuple<FailureCondition>(std::forward<FailureCondition>(condition)));
}

inline FailureDetector<> noDetector()
{
    return FailureDetector<>(std::tuple<>());
}


//////////////////////////////

template<typename T>
struct Returns
{
    T d_failureValue;

    Returns(T&& faliureValue)
    : d_failureValue(std::forward<T>(faliureValue))
    { }

    // Types of failure detector: throws, return, ...


    // i'd prefer an interface, but needs to be a template on the return type at least
    // how to go around it?
    // NOTE callable always return a CallResult
    template<typename Callable, typename ...Args>
    auto
    operator()(Callable&& callable, Args&&... args)
    {
        // Always return a FailableResult, so we can get it by value with auto
        using ResultType = std::result_of_t<Callable(Args...)>;
        ResultType result = std::forward<Callable>(callable)(std::forward<Args>(args)...);

        if (isFailure(result) || getResult(result) == d_failureValue)
        {
            return ResultType(Failure());
        }
        else
        {
            return std::move(result);
        }
    }
};


}
#endif