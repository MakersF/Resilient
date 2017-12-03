#pragma once

#include <cassert>
#include <utility>
#include <tuple>
#include <type_traits>
#include <exception>

#include <resilient/common/invoke.hpp>
#include <resilient/failable/failable.hpp>
#include <resilient/common/utilities.hpp>
#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>


namespace resilient {

struct NoFailureDetector : FailureDetectorTag<> { };

class UnknownTaskResult : std::runtime_error
{
public:
    using std::runtime_error::runtime_error;
};

class BadImplementationError : std::runtime_error // Should we use std::terminate instead?
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
        assert(isException());
        return get<std::exception_ptr>(d_data);
    }

    ConstRefType getResult() const override
    {
        assert(not isException());
        return *get<ConstPtrT>(d_data);
    }
};

template<typename T, typename Q>
using is_decayed_same = std::is_same<std::decay_t<T>, Q>;

template<typename T, typename Q>
using if_is_decayed_same = std::enable_if_t<is_decayed_same<T, Q>::value, void*>;

template<typename T, typename Q>
using if_is_not_decayed_same = std::enable_if_t<not is_decayed_same<T, Q>::value, void*>;

// Create a visitor which forwards the call to the visitor it wraps except for a specific type.
// Usefull when you want to perform operations on a variant while knowing one of the possible types
// it contains is not the current value.
template<typename IgnoreType, typename Wrapped>
struct IgnoreTypeVisitor
{
    using result_type = typename std::decay_t<Wrapped>::result_type;
    Wrapped d_wrapped;

    template<typename T, if_is_decayed_same<T, IgnoreType> = nullptr>
    result_type operator()(T&&)
    {
        throw BadImplementationError("The ignored type should never be contained by the variant");
    }

    template<typename T, if_is_not_decayed_same<T, IgnoreType> = nullptr>
    result_type operator()(T&& value)
    {
        return detail::invoke(std::forward<Wrapped>(d_wrapped), std::forward<T>(value));
    }
};

template<typename IgnoreType, typename Wrapped>
IgnoreTypeVisitor<IgnoreType, Wrapped> make_ignoretype(Wrapped&& wrapped)
{
    return IgnoreTypeVisitor<IgnoreType, Wrapped>{std::forward<Wrapped>(wrapped)};
}

// Assign the current value of a variant to another variant
template<typename Destination>
struct AssignVisitor
{
     // this is required because boost (possible implementation of variant) requires it from visitors.
    using result_type = void;
    Destination& d_destinationVariant;

    template<typename T>
    void operator()(T&& value)
    {
        d_destinationVariant = std::forward<T>(value);
    }
};

// Construct an object with the current value of a variant
template<typename ConstructedType>
struct ConstructVisitor
{
     // this is required because boost (possible implementation of variant) requires it from visitors.
    using result_type = ConstructedType;

    template<typename T>
    ConstructedType operator()(T&& value)
    {
        return ConstructedType{std::forward<T>(value)};
    }
};

// Define a Variant<Failures...> from a std::tuple<Failures...>
template<typename ...Failures>
struct failure_variant_type;

template<typename ...Failures>
struct failure_variant_type<std::tuple<Failures...>>
{
    using type = Variant<Failures...>;
};

template<typename FailureDetector, typename Callable, typename ...Args>
auto runTaskImpl(FailureDetector&& failureDetector, Callable&& callable, Args&&... args)
{
    using DetectorFailureTypes = typename std::decay_t<FailureDetector>::failure_types;
    using DetectorFailure = typename failure_variant_type<DetectorFailureTypes>::type;
    using Result = detail::invoke_result_t<Callable, Args...>;
    using _Failable = Failable<DetectorFailure, Result>;

    decltype(auto) state{failureDetector.preRun()};

    // There is a bit of duplication in the two try branches. Can that be factored out?
    try
    {
        _Failable result{detail::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...)};
        detail::OperationResult<Result> operationResult{get_value(result)};
        // TODO this might throw! Big problem if it does: it's going to be considered as if result threw, and state is moved twice
        decltype(auto) failure{failureDetector.postRun(std::move(state), operationResult)};
        if(holds_failure(failure))
        {
            // Move the failure from the failure into the result.
            // In this process we ignore the NoFailure type as it is not a valid state for the result.
            visit(detail::make_ignoretype<NoFailure>(detail::AssignVisitor<decltype(result)>{result}),
                  std::forward<decltype(failure)>(failure));
        }
        return std::move(result);
    }
    catch (const BadImplementationError&)
    {
        throw; // Reraise
    }
    catch (...)
    {
        auto current_exception = std::current_exception();
        detail::OperationResult<Result> operationResult{current_exception};
        decltype(auto) failure{failureDetector.postRun(std::move(state), operationResult)};
        if(not operationResult.isExceptionConsumed())
        {
            std::rethrow_exception(current_exception);
        }
        else if(holds_failure(failure))
        {
            // Construct a DetectorFailure from the failure and initialize a _Failable with it.
            // The constructed DetectorFailure is different from the returned failure because it does not
            // contain the NoFailure type.
            return _Failable{visit(detail::make_ignoretype<NoFailure>(detail::ConstructVisitor<DetectorFailure>{}),
                                   std::forward<decltype(failure)>(failure))};
        }
        else
        {
            // The exception was consumed but no failure was reported? Likely a bug.
            // We have no result and no failure and no exception.
            throw UnknownTaskResult(
                "Task throwed exception: no failure was detected but the exception was consumed.");
        }
    }
}

}

// Wrap a callable and attach a Detector to it, so that when invoked
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
    auto operator()(Args&&... args) &
    {
        static_assert(not std::is_same<FailureDetector, NoFailureDetector>::value,
            "The Task does not have a failure condition.");

        return detail::runTaskImpl(d_failureDetector, d_callable, std::forward<Args>(args)...);
    }

    template<typename ...Args>
    auto operator()(Args&&... args) &&
    {
        static_assert(not std::is_same<FailureDetector, NoFailureDetector>::value,
            "The Task does not have a failure condition.");

        // This is invoked when Task is an rvalue.
        // We can move it's members, but ony if they are not lvalues, as the user might have a reference
        // to them otherwise.
        // If they were passed as reference moving them is incorect: that's why we use forward.
        return detail::runTaskImpl(std::forward<FailureDetector>(d_failureDetector),
                                   std::forward<Callable>(d_callable),
                                   std::forward<Args>(args)...);
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