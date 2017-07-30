#ifndef RESILIENT_FAILURE_DETECTION_H
#define RESILIENT_FAILURE_DETECTION_H

#include <utility>
#include <tuple>
#include <type_traits>

namespace resilient {

template<typename ...FailureConditions>
class FailureDetector
{
protected:
    template<typename FailureCondition>
    struct after_adding
    {
        using type = FailureDetector<FailureConditions..., FailureCondition>;
    };

    template<typename FailureCondition>
    using after_adding_t = typename after_adding<FailureCondition>::type;

public:
    FailureDetector() = default;

    template<typename FailureCondition>
    after_adding_t<FailureCondition>
    addFailureCondition(FailureCondition&& failureCondition) &&
    {
        return after_adding_t<FailureCondition>(
            std::tuple_cat(
                std::move(d_failureConditions),
                std::make_tuple(std::forward<FailureCondition>(failureCondition))
            )
        );
    }

    template<typename C, typename ...Args>
    auto detectFailure(C&& callable, Args&&... args) -> decltype(callable(std::forward<Args>(args)...))
    {
        // TODO Execute all the failure detectors one into the other
        return callable(std::forward<Args>(args)...);
    }

private:
    std::tuple<FailureConditions...> d_failureConditions;

    explicit FailureDetector(std::tuple<FailureConditions...>&& failureConditions)
    : d_failureConditions(std::move(failureConditions))
    { }

    template<typename ...T>
    friend class FailureDetector;
};

// Specialize for empty FailureDetector to simply call the callable TODO might not be needed
template<>
template<typename C, typename ...Args>
inline auto FailureDetector<>::detectFailure(C&& callable, Args&&... args)
-> typename std::result_of<C(Args...)>::type
{
    return std::forward<C>(callable)(std::forward<Args>(args)...);
}

}
#endif