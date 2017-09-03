#ifndef RESILIENT_TUPLE_UTILS_H
#define RESILIENT_TUPLE_UTILS_H

#include <utility>
#include <tuple>

namespace resilient {

template<typename ...Args, typename NewItem>
std::tuple<Args..., NewItem> cat(std::tuple<Args...>&& oldTuple, NewItem&& newItem)
{
    return std::tuple_cat(
        std::forward<decltype(oldTuple)>(oldTuple),
        std::forward_as_tuple(newItem));
}

}

#endif