#ifndef RESILIENT_FAILURE_DETECTION_H
#define RESILIENT_FAILURE_DETECTION_H

#include <utility>
#include <tuple>
#include <type_traits>
#include <resilient/foldinvoke.hpp>

namespace resilient {

template<typename T>
struct CallResult
{
    T* result; // possibly use a void* to erase the type
    bool failure;
};

template<typename Callable>
struct WrapResultInCallResult
{
    Callable&& d_callableRef;

    template<typename ...Args>
    using result_of_this = typename std::result_of<Callable(Args...)>::type;

    template<typename ...Args>
    CallResult<result_of_this<Args...>>
    operator()(Args&&... args)
    {
        using ResultType = result_of_this<Args...>;
        CallResult<ResultType> cr{nullptr, false};
        // TODO what if it's void?
        cr.result = new ResultType(d_callableRef(std::forward<Args>(args)...));
        return cr;
    }
};

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
    decltype(auto) detectFailure(C&& callable, Args&&... args)
    {
        // TODO Execute all the failure detectors one into the other
        return foldInvoke(
            d_failureConditions,
            WrapResultInCallResult<C>{std::forward<C>(callable)},
            std::forward<Args>(args)...
        );
    }

private:
    std::tuple<FailureConditions...> d_failureConditions;

    explicit FailureDetector(std::tuple<FailureConditions...>&& failureConditions)
    : d_failureConditions(std::move(failureConditions))
    { }

    template<typename ...T>
    friend class FailureDetector;
};

}
#endif