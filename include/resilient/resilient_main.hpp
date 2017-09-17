#ifndef RESILIENT_RESILIENT_MAIN_H
#define RESILIENT_RESILIENT_MAIN_H

#if 0
#include <resilient/failures/failure_detection.hpp>
#include <type_traits>
#include <utility>
#include <tuple>

namespace resilient {

template<typename ...Exceptions>
struct Throws {};


template<typename FailureDetector>
class RetryPolicyImpl : private FailureDetector
{
public:
    RetryPolicyImpl() = default;

    explicit RetryPolicyImpl(FailureDetector&& failureDetector)
    : FailureDetector(std::forward<FailureDetector>(failureDetector))
    { }

private:
    template<typename FailureCondition>
    using after_adding_failure_t = RetryPolicyImpl<typename FailureDetector::template after_adding_t<FailureCondition>>;

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

using RetryPolicy = RetryPolicyImpl<resilient::FailureDetector<>>;

class ResilientJob
{
    // detect if it's a failure
    // what to do on failure
public:
    static RetryPolicy withRetryPolicy() // retrypolicy, circuitbreaker, fallback ...
    {
        return RetryPolicy();
    }
};

}
#endif
#endif