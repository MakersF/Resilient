#pragma once

#include <utility>
#include <tuple>
#include <type_traits>
#include <resilient/common/result.hpp>
#include <resilient/common/utilities.hpp>
#include <resilient/common/foldinvoke.hpp>
#include <resilient/failures/failure_detection.hpp>
#include <resilient/policies/pipeline.hpp>

namespace resilient {

template<typename Policy, typename FailureDetector>
class Job
{
public:
    // TODO
    // decide how to add failure detectors to the job


    // TODO enable only if Policy is the Noop One?
    template<typename NewPolicy>
    Job<NewPolicy, FailureDetector> with(NewPolicy&& policy) &
    {
        return Job<NewPolicy, FailureDetector>(
            std::forward<NewPolicy>(policy), d_failureDetector);
    }

    template<typename NewPolicy>
    Job<NewPolicy, FailureDetector> with(NewPolicy&& policy) &&
    {
        return Job<NewPolicy, FailureDetector>(
            std::forward<NewPolicy>(policy), std::move(d_failureDetector));
    }

    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &
    {
        return d_policy(
            [this](auto&&... innerArgs) mutable {
                return this->d_failureDetector.detectFailure(std::forward<decltype(innerArgs)>(innerArgs)...);
            },
            std::forward<Callable>(callable),
            std::forward<Args>(args)...);
    }

    template<typename Callable, typename ...Args>
    decltype(auto) run(Callable&& callable, Args&&... args) &&
    {
        return d_policy(
            [self = std::move(*this)](auto&&... innerArgs) mutable {
                return std::move(self.d_failureDetector).detectFailure(std::forward<decltype(innerArgs)>(innerArgs)...);
            },
            std::forward<Callable>(callable),
            std::forward<Args>(args)...);
    }

    explicit Job(Policy&& policy, FailureDetector&& failureDetector)
    : d_policy(std::forward<Policy>(policy))
    , d_failureDetector(std::forward<FailureDetector>(failureDetector))
    { }

private:

    Policy d_policy;
    FailureDetector d_failureDetector;
};

inline Job<NoopPolicy, FailureDetector<>> job()
{
    return Job<NoopPolicy, FailureDetector<>>(NoopPolicy(), noDetector());
}

}