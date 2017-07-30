#ifndef RESILIENT_RESILIENT_MAIN_H
#define RESILIENT_RESILIENT_MAIN_H

#include <resilient/failure_detection.hpp>
#include <type_traits>
#include <utility>
#include <tuple>

namespace resilient {

template<typename ...Exceptions>
struct Throws {};

template<typename T>
struct CallResult
{
    T* result; // possibly use a void* to erase the type
    bool failure;
};

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
    template<typename C, typename ...Args>
    auto operator()(C&& callable, Args&&... args) -> decltype(callable(std::forward<Args>(args)...))
    {
        // TODO we image this failure detector is based on return type.
        // The system in reality needs to be expandable
        auto cr = callable(std::forward<Args>(args)...);
        cr.failure = cr.failure || (*cr.result == d_failureValue);
        return std::move(cr);
        // TODO callable can be a FailureDetector (wrapped in a lambda).
        // Specialize on the return type CallResult to OR the failure condition
    }
};

template<typename FailureDetector = resilient::FailureDetector<>>
class RetryPolicy : private FailureDetector
{
public:
    RetryPolicy() = default;

    explicit RetryPolicy(FailureDetector&& failureDetector)
    : FailureDetector(std::forward<FailureDetector>(failureDetector))
    { }

private:
    template<typename FailureCondition>
    using after_adding_failure_t = RetryPolicy<typename FailureDetector::template after_adding_t<FailureCondition>>;

public:
    template<typename FailureCondition>
    after_adding_failure_t<FailureCondition>
    failsIf(FailureCondition&& failureCondition) &&
    {
        return after_adding_failure_t<FailureCondition>(
            std::move(*this).addFailureCondition(std::forward<FailureCondition>(failureCondition))
        );
    }

    template<typename C, typename ...Args>
    auto run(C&& callable, Args&&... args)
        -> decltype(detectFailure(std::forward<C>(callable), std::forward<Args>(args)...))
    {
        return detectFailure(std::forward<C>(callable), std::forward<Args>(args)...);
    }
private:
};

class ResilientJob
{
    // detect if it's a failure
    // what to do on failure
public:
    static RetryPolicy<> withRetryPolicy() // retrypolicy, circuitbreaker, fallback ...
    {
        return RetryPolicy<>();
    }
};

}
#endif