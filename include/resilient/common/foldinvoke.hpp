#pragma once

#include <utility>
#include <tuple>
#include <type_traits>

#include <resilient/common/utilities.hpp>

namespace resilient {

namespace detail {

template<typename Tuple, std::size_t index, std::size_t length>
struct FoldInvokeImpl
{
    template<typename Callable, typename ...Args>
    static decltype(auto) call(Tuple&& tuple, Callable&& callable, Args&&... args)
    {
        // Invoke the index-th function with a lambda which will all the index+1-th function
        return move_if_rvalue<Tuple>(std::get<index>(tuple))(
            [tuple = std::forward<Tuple>(tuple), callable = std::forward<Callable>(callable)]
            (auto&&... innerArgs) mutable
            {
                return FoldInvokeImpl<Tuple, index + 1, length>::call(
                    std::forward<Tuple>(tuple),
                    std::forward<Callable>(callable),
                    std::forward<decltype(innerArgs)>(innerArgs)...
                );
            },
            std::forward<Args>(args)...
        );
    }
};

template<typename Tuple, std::size_t length>
struct FoldInvokeImpl<Tuple, length, length>
{
    template<typename Callable, typename ...Args>
    static decltype(auto) call(Tuple&&, Callable&& callable, Args&&... args)
    {
        return std::forward<Callable>(callable)(std::forward<Args>(args)...);
    }
};

} //namespace detail

// Tuple must be a tuple of functions.
// All the functions must take a function as first argument
// and the arguments to call the function with as following arguments.
// foldInvoke calls each function, from left to right, passing a function which
// will recursively invoke all the remaining functions.
// Callable is the function passed to the last function in the tuple.
//
// tl;dr recursively call one function inside eachother
// given [f, g, h] this calls f(K), where K is a function which calls g(Y), where
// Y is a function which calls h(callable, args)
template<typename Tuple, typename Callable, typename ...Args>
decltype(auto) foldInvoke(Tuple&& tuple, Callable&& callable, Args&&... args)
{
    return detail::FoldInvokeImpl<Tuple, 0, std::tuple_size<std::decay_t<Tuple>>::value>::call(
        std::forward<Tuple>(tuple),
        std::forward<Callable>(callable),
        std::forward<Args>(args)...);
};

}
