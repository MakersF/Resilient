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
        auto trait = traits(result);
        static_assert(trait.is_failable, "The callable must return a Failable.");

        if (trait.isFailure(result) || trait.getValue(result) == d_failureValue)
        {
            return trait.failure();
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