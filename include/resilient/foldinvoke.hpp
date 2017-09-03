#ifndef RESILIENT_FOLDINVOKE_H
#define RESILIENT_FOLDINVOKE_H

#include <utility>
#include <tuple>
#include <type_traits>

namespace resilient {

template<typename Tuple, std::size_t index, std::size_t length>
struct FoldInvokeImpl
{
    template<typename Callable>
    static decltype(auto) call(Tuple& tuple, Callable&& callable)
    {
        return std::get<index>(tuple)([&tuple, &callable]() mutable
        {
            return FoldInvokeImpl<Tuple, index + 1, length>::call(
                tuple,
                std::forward<Callable>(callable)
            );
        });
    }
};

template<typename Tuple, std::size_t length>
struct FoldInvokeImpl<Tuple, length, length>
{
    template<typename Callable>
    static decltype(auto) call(Tuple&, Callable&& callable)
    {
        return std::forward<Callable>(callable)();
    }
};

template<typename Tuple, typename Callable>
decltype(auto) foldInvoke(Tuple& tuple, Callable&& callable)
{
    return FoldInvokeImpl<Tuple, 0, std::tuple_size<Tuple>::value>::call(
        tuple, std::forward<Callable>(callable));
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