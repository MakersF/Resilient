#pragma once

#include <resilient/common/failable.hpp>

namespace resilient {

template<typename T>
class Returns
{
public:
    Returns(T&& faliureValue)
    : d_failureValue(std::forward<T>(faliureValue))
    { }

    // i'd prefer an interface, but needs to be a template on the return type at least
    // how to go around it?
    template<typename Callable, typename ...Args>
    auto operator()(Callable&& callable, Args&&... args)
    {
        // Return by auto (no decltype(auto)) so we don't risk passing around dangling references.
        // We move anyway.
        auto&& result = std::forward<Callable>(callable)(std::forward<Args>(args)...);

        if (result.isFailure() || result.value() == d_failureValue)
        {
            using Result = std::decay_t<decltype(result)>;
            return failure<typename Result::failure_type, typename Result::value_type>();
        }
        else
        {
            return std::move(result);
        }
    }

private:
    T d_failureValue;
};

}