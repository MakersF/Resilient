#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <exception>

#include <boost/optional.hpp>

#include <resilient/common/failable.hpp>
#include <resilient/common/utilities.hpp>
#include <resilient/common/foldinvoke.hpp>
#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/execution_context.hpp>


namespace resilient {

struct NoFailureDetector : FailureDetectorTag<> { };

struct ExceptionFailure
{
    ExceptionFailure(std::exception_ptr ptr) : d_exception_ptr(ptr) { }
    std::exception_ptr d_exception_ptr;
};

namespace detail {

template<typename T>
struct FailureType;

// Extract the types from the tuple
template<typename ...T>
struct FailureType<std::tuple<T...>>
{
    using type = Failure<T...>;
};

}

template<typename Callable, typename FailureDetector>
class Task
{
public:
    template<typename NewFailureDetector>
    Task<Callable, NewFailureDetector> failsIf(NewFailureDetector&& condition) &&
    {
        static_assert(std::is_same<FailureDetector, NoFailureDetector>::value,
            "The Task already has a failure condition.");

        return Task<Callable, NewFailureDetector>(
            std::forward<Callable>(d_callable),
            std::forward<NewFailureDetector>(condition));
    }

    template<typename ...Args>
    auto operator()(Args&&... args) &&
    {
        static_assert(!std::is_same<FailureDetector, NoFailureDetector>::value,
            "The Task does not have a failure condition.");

        // TODO Task create a Failable from the function.
        // it means it should get the result of the call (or catch the exception)
        // and wrap it in a failable.
        // With the failable it should call the detector to see if it matches a condition.
        using Failure = typename detail::FailureType<tuple_extend_t<ExceptionFailure, FailureTypes>>::type;
        using Result = std::result_of_t<Callable(Args...)>;
        using ThisFailable = Failable<Failure, Result>;
        boost::optional<ThisFailable> buffer;
        OperationResult<Result> operationResult;

        auto state = d_failureDetector.preRun();

        try
        {
            buffer = std::forward<Callable>(d_callable)(std::forward<Args>(args)...);
            operationResult = buffer->value();
        }
        catch (...)
        {
            auto exception = std::current_exception();
            buffer = Failure(ExceptionFailure(exception));
            operationResult = exception;
        }

        d_failureDetector.postRun(
            std::move(state),
            operationResult,
            // TODO "implement the interface to set the failures". Use templates to make it simple
            // Possible alternative is to return variant of failures in the detectors
        );

        return std::move(*buffer);
    }

    Task(Callable&& callable, FailureDetector&& condition)
    : d_callable(std::forward<Callable>(callable))
    , d_failureDetector(std::forward<FailureDetector>(condition))
    { }

private:

    using FailureTypes = typename std::decay_t<FailureDetector>::failure_types;

    Callable d_callable;
    FailureDetector d_failureDetector;
};

template<typename Callable, typename FailureDetector = NoFailureDetector>
Task<Callable, FailureDetector> task(Callable&& callable, FailureDetector&& detector = FailureDetector())
{
    return Task<Callable, FailureDetector>(
        std::forward<Callable>(callable), std::forward<FailureDetector>(detector));
}

}