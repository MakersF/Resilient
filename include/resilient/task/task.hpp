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
    using type = Variant<T...>;
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
        using DetectorFailure = typename std::decay_t<FailureDetector>::failure;
        using Result = std::result_of_t<Callable(Args...)>;
        using _Failable = Failable<DetectorFailure, Result>;

        OperationResult<Result> operationResult;
        auto state = d_failureDetector.preRun();

        // TODO possibly make here a lambda which is called by the two try brances if needed

        try
        {
            _Failable result{std::forward<Callable>(d_callable)(std::forward<Args>(args)...)};
            operationResult = result.value();
            decltype(auto) failure{d_failureDetector.postRun(std::move(state), operationResult)};
            if(holds_failure(failure))
            {
                result = std::move(failure);
            }
            return std::move(result);
        }
        catch (...)
        {
            operationResult = std::current_exception();
            // TODO how to check if the exception was consumed?
            // Option: put in OperationResult
            decltype(auto) failure{d_failureDetector.postRun(std::move(state), operationResult)};
            if(holds_failure(failure))
            {
                return _Failable{std::move(failure)};
            }
            if(not operationResult.isExceptionConsumed())
            {
                std::rethrow_exception(operationResult.getException());
            }
            else
            {
                // The exception was consumed but no failure was reported? Likely a bug.
                // We have no result and no failure and no exception.
                throw 1; // TODO proper type
            }
        }
    }

    Task(Callable&& callable, FailureDetector&& condition)
    : d_callable(std::forward<Callable>(callable))
    , d_failureDetector(std::forward<FailureDetector>(condition))
    { }

private:
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