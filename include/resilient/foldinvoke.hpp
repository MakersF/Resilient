#ifndef RESILIENT_FOLDINVOKE_H
#define RESILIENT_FOLDINVOKE_H

#include <utility>
#include <tuple>
#include <type_traits>

namespace resilient {

// TODO move index into the call, merge the two FoldInvokeImpl into one
template<typename Tuple, std::size_t index, std::size_t length>
struct FoldInvokeImpl
{
    template<typename Callable, typename ...Args>
    static decltype(auto) call(Tuple& tuple, Callable&& callable, Args&&... args)
    {
        return std::get<index>(tuple)([&tuple, &callable](Args&&... innerArgs) mutable
        {
            FoldInvokeImpl<Tuple, index + 1, length>::call(
                tuple, std::forward<Callable>(callable), std::forward<Args>(innerArgs)...
            );
        }, args...);
    }
};

template<typename Tuple, std::size_t length>
struct FoldInvokeImpl<Tuple, length, length>
{
    static_assert(std::tuple_size<Tuple>::value == length, "The length parameter should be the length of the tuple.");

    template<typename Callable, typename ...Args>
    static decltype(auto) call(Tuple&, Callable&& callable, Args&&... args)
    {
        return std::forward<Callable>(callable)(std::forward<Args>(args)...);
    }
};

template<typename Tuple, typename Callable, typename ...Args>
decltype(auto) foldInvoke(Tuple& tuple, Callable&& callable, Args&&... args)
{
    return FoldInvokeImpl<Tuple, 0, std::tuple_size<Tuple>::value>::call(
        tuple, std::forward<Callable>(callable), std::forward<Args>(args)...
    );
};

template<typename Tuple, std::size_t... I, typename ...Args>
void invokeAllImpl(Tuple&& tuple, std::index_sequence<I...>, Args&&... args)
{
    (void) std::initializer_list<int>{
        ((void) (std::get<I>(tuple)(std::forward<Args>(args)...)), 0)...
    };
}

template<typename Tuple, typename ...Args>
void invokeAll(Tuple&& tuple, Args&&... args)
{
    invokeAllImpl(
        std::forward<Tuple>(tuple),
        std::make_index_sequence<std::tuple_size<std::remove_reference_t<Tuple>>::value>(),
        std::forward<Args>(args)...
    );
}

}

#endif