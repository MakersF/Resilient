#pragma once

/**
 * @defgroup Task
 *
 * @brief A task is an operation which may fail.
 *
 * # Description
 *
 * Operations, especially the ones which have to interact with an external system (a web service,
 * perform IPC, etc...), might fail. A `task` acknowledges this and make it explicit in its return
 * type: either a result value or a failure.
 *
 * # The Concept
 *
 * A `task` is any callable object which returns a `Failable` when invoked.
 *
 * Any callable object can be transformed into a `task` by instantiating a `resilient::Task` and
 * adding some detectors for the failure conditions.
 */

#include <cassert>
#include <exception>
#include <tuple>
#include <type_traits>
#include <utility>

#include <resilient/detail/invoke.hpp>
#include <resilient/detail/utilities.hpp>
#include <resilient/detail/variant_utils.hpp>
#include <resilient/detector/basedetector.hpp>
#include <resilient/detector/callresult.hpp>
#include <resilient/task/failable.hpp>

namespace resilient {

struct NoFailureDetector : FailureDetectorTag<>
{
};

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
        if (holds_alternative<std::exception_ptr>(d_data)) {
            d_isExceptionConsumed = true;
        }
        else
        {
            throw std::runtime_error("Consumed exception while the result is not an exception.");
        }
    }

    bool isException() const override { return holds_alternative<std::exception_ptr>(d_data); }

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

// Define a Variant<Failures...> from a std::tuple<Failures...>
template<typename... Failures>
struct failure_variant_type;

template<typename... Failures>
struct failure_variant_type<std::tuple<Failures...>>
{
    using type = Variant<Failures...>;
};

template<typename FailureDetector, typename Callable, typename... Args>
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
        _Failable result{
            detail::invoke(std::forward<Callable>(callable), std::forward<Args>(args)...)};
        detail::OperationResult<Result> operationResult{get_value(result)};
        // TODO this might throw! Big problem if it does: it's going to be considered as if result
        // threw, and state is moved twice
        decltype(auto) failure{failureDetector.postRun(std::move(state), operationResult)};
        if (holds_failure(failure)) {
            // Move the failure from the failure into the result.
            // In this process we ignore the NoFailure type as it is not a valid state for the
            // result.
            visit(
                detail::make_ignoretype<NoFailure>(detail::AssignVisitor<decltype(result)>{result}),
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
        if (not operationResult.isExceptionConsumed()) {
            std::rethrow_exception(current_exception);
        }
        else if (holds_failure(failure))
        {
            // Construct a DetectorFailure from the failure and initialize a _Failable with it.
            // The constructed DetectorFailure is different from the returned failure because it
            // does not contain the NoFailure type.
            return _Failable{visit(
                detail::make_ignoretype<NoFailure>(detail::ConstructVisitor<DetectorFailure>{}),
                std::forward<decltype(failure)>(failure))};
        }
        else
        {
            // The exception was consumed but no failure was reported? Likely a bug.
            // We have no result, no failure and no exception.
            throw UnknownTaskResult(
                "Task throwed exception: no failure was detected but the exception was consumed.");
        }
    }
}

} // namespace detail

// Wrap a callable and attach a Detector to it, so that when invoked
/**
 * @ingroup Task
 * @brief Create a `task` from a callable, using a `Detector` to detect failures.
 *
 * Wrap a callable object, making it return a `Failable` when invoked.
 *
 * `Task` uses a detector to detect whether the invocation of the callable failed or not.
 *
 * @tparam Callable The callable object which will be used when invoking the task.
 * @tparam FailureDetector The detector that will be used to detect failures when invoking the
 * callable.
 */
template<typename Callable, typename FailureDetector>
class Task
{
    public:
    /**
     * @brief Instantiate a new `Task` with the given callable and failure detector
     *
     * @note The preferred way to create a `Task` is to use the `task()` function.
     *
     * @param callable The callable to use.
     * @param detector The detector to use.
     */
    Task(Callable&& callable, FailureDetector&& detector)
    : d_callable(std::forward<Callable>(callable))
    , d_failureDetector(std::forward<FailureDetector>(detector))
    {
    }

    /**
     * @brief Create a new task using the current callable and a new failure condition
     *
     * @pre It only compiles if the current `Task` has no failure condition assigned.
     *
     * @param condition The new failure condition.
     * @return The new `Task`
     */
    template<typename NewFailureDetector>
    Task<Callable, NewFailureDetector> failsIf(NewFailureDetector&& condition) &&
    {
        static_assert(std::is_same<FailureDetector, NoFailureDetector>::value,
                      "The Task already has a failure condition.");

        return Task<Callable, NewFailureDetector>(std::forward<Callable>(d_callable),
                                                  std::forward<NewFailureDetector>(condition));
    }

    /**
     * @brief Invoke the callable checking for failures.
     *
     * @param args The arguments to be used when invoking the callable.
     */
    template<typename... Args>
    auto operator()(Args&&... args) &
    {
        static_assert(not std::is_same<FailureDetector, NoFailureDetector>::value,
                      "The Task does not have a failure condition.");

        return detail::runTaskImpl(d_failureDetector, d_callable, std::forward<Args>(args)...);
    }

    /**
     * @brief Same as `operator()&`, but also forwards the callable when invoking it.
     */
    template<typename... Args>
    auto operator()(Args&&... args) &&
    {
        static_assert(not std::is_same<FailureDetector, NoFailureDetector>::value,
                      "The Task does not have a failure condition.");

        // This is invoked when Task is an rvalue.
        // We can move it's members, but ony if they are not lvalues, as the user might have a
        // reference to them otherwise. If they were passed as reference moving them is incorect:
        // that's why we use forward.
        return detail::runTaskImpl(std::forward<FailureDetector>(d_failureDetector),
                                   std::forward<Callable>(d_callable),
                                   std::forward<Args>(args)...);
    }

    private:
    Callable d_callable;
    FailureDetector d_failureDetector;
};

/**
 * @brief Wrap a callable object in a `Task`.
 * @related Task
 *
 * @param callable The callable to wrap.
 * @param detector The detector to use.
 * @return The `Task` wrapping the callable and using the provided detector.
 *         If no detector is provided a task with no detector is returned.
 *         In such a case you need to invoke `failsIf()` on the task to provide a detector,
 *         otherwise the `Task` will not compile.
 */
template<typename Callable, typename FailureDetector = NoFailureDetector>
Task<Callable, FailureDetector> task(Callable&& callable,
                                     FailureDetector&& detector = FailureDetector())
{
    return Task<Callable, FailureDetector>(std::forward<Callable>(callable),
                                           std::forward<FailureDetector>(detector));
}

} // namespace resilient