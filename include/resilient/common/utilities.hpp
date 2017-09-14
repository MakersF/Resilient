#pragma once

#include <utility>
#include <tuple>

#define FWD(arg) ::std::forward<decltype(arg)>(args)

namespace resilient {

template<typename Tuple, typename T>
decltype(auto) tuple_append(Tuple&& tuple, T&& item)
{
    return std::tuple_cat(std::forward<Tuple>(tuple),
                          std::tuple<T>(std::forward<T>(item)));
}

}