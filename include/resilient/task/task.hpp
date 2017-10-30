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

class UnknownTaskResult : std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

namespace detail {

template<typename T>
class OperationResult : public ICallResult<T>
{
private:
    // ConstRefType is a dependent type so it needs to be qualified with typename
    using typename ICallResult<T>::ConstRefType;
    using ConstPtrT = const std::decay_t<T>*;
    using Base = Variant<std::exception_ptr, ConstPtrT>;

    Base d_data;
    bool d_isExceptionConsumed = false;

public:
    OperationResult() = default;
    OperationResult(const std::exception_ptr& ptr) : d_data(ptr) {}
    OperationResult(ConstRefType ref) : d_data(&ref) {}

    bool isExceptionConsumed() { return d_isExceptionConsumed; }
    void consumeException() override
    {
        if(holds_alternative<std::exception_ptr>(d_data))
        {
            d_isExceptionConsumed = true;
        }
        else
        {
            throw std::runtime_error("Consumed exception while the result is not an exception.");
        }
    }

    bool isException() const override
    {
        return holds_alternative<std::exception_ptr>(d_data);
    }

    const std::exception_ptr& getException() const override
    {
        return get<std::exception_ptr>(d_data);
    }

    ConstRefType getResult() const override
    {
        return *get<ConstPtrT>(d_data);
    }
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

        using DetectorFailure = typename std::decay_t<FailureDetector>::failure;
        using Result = std::result_of_t<Callable(Args...)>;
        using _Failable = Failable<DetectorFailure, Result>;

        decltype(auto) state{d_failureDetector.preRun()};

        // There is a bit of duplication in the two try branches. Can that be factored out?
        try
        {
            _Failable result{std::forward<Callable>(d_callable)(std::forward<Args>(args)...)};
            detail::OperationResult<Result> operationResult{result.value()};
            decltype(auto) failure{d_failureDetector.postRun(std::move(state), operationResult)};
            if(holds_failure(failure))
            {
                result = std::move(failure);
            }
            return std::move(result);
        }
        catch (...)
        {
            auto current_exception = std::current_exception();
            detail::OperationResult<Result> operationResult{current_exception};
            decltype(auto) failure{d_failureDetector.postRun(std::move(state), operationResult)};
            if(not operationResult.isExceptionConsumed())
            {
                std::rethrow_exception(current_exception);
            }
            else if(holds_failure(failure))
            {
                return _Failable{std::move(failure)};
            }
            else
            {
                // The exception was consumed but no failure was reported? Likely a bug.
                // We have no result and no failure and no exception.
                throw UnknownTaskResult(
                    "Task throed exception: no failure was detected but the exception was consumed.");
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