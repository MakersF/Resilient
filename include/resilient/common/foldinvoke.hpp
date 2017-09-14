#ifndef RESILIENT_FOLDINVOKE_H
#define RESILIENT_FOLDINVOKE_H

#include <utility>
#include <tuple>
#include <type_traits>

namespace resilient {

namespace detail {

template<typename Tuple, std::size_t index, std::size_t length>
struct FoldInvokeImpl
{
    template<typename Callable, typename ...Args>
    static decltype(auto) call(Tuple&& tuple, Callable&& callable, Args&&... args)
    {
        return std::get<index>(std::forward<Tuple>(tuple))(
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

template<typename Tuple, typename Callable, typename ...Args>
decltype(auto) foldInvoke(Tuple&& tuple, Callable&& callable, Args&&... args)
{
    return detail::FoldInvokeImpl<Tuple, 0, std::tuple_size<std::decay_t<Tuple>>::value>::call(
        std::forward<Tuple>(tuple),
        std::forward<Callable>(callable),
        std::forward<Args>(args)...);
};

}

#endif