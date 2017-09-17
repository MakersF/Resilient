#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <resilient/common/failable.hpp>
#include <resilient/common/result.hpp>
#include <resilient/common/utilities.hpp>
#include <resilient/common/foldinvoke.hpp>
#include <resilient/failures/failure_detection.hpp>
#include <resilient/policies/pipeline.hpp>

namespace resilient {

template<typename Callable, typename FailureCondition>
class Task
{
public:
    // TODO enable_if FailureCondition == DefaultCondition
    template<typename NewFailureCondition>
    Task<Callable, NewFailureCondition> failsIf(NewFailureCondition&& condition) &&
    {
        return Task<Callable, NewFailureCondition>(
            std::move(d_callable),
            std::forward<NewFailureCondition>(condition));
    }

    template<typename ...Args>
    decltype(auto) operator()(Args&&... args) &&
    {
        // TODO Task job is to create a Failable from the function.
        // it means it should get the result of the call (or catch the exception)
        // and wrap it in a failable.
        // With the failable it should call the detector to see if it matches a condition.
        return make_failable<Failure>(std::forward<Callable>(d_callable)(std::forward<Args>(args)...));
    }

    Task(Callable&& callable, FailureCondition&& condition)
    : d_callable(std::forward<Callable>(callable))
    , d_failureDetector(std::forward<FailureCondition>(condition))
    { }

private:
    Callable d_callable;
    FailureCondition d_failureDetector;
};

// TODO define a default failure. Possibly something that does nothing
template<typename Callable>
Task<Callable, int> task(Callable&& callable)
{
    return Task<Callable, int>(std::forward<Callable>(callable), 12345);
}

}