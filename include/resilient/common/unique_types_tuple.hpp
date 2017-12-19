#pragma once

#include <tuple>
#include <resilient/common/utilities.hpp>

namespace resilient {
namespace detail {

// Check if T appears in Tail
template<typename T, typename ...Tail>
struct is_in_list;

template<typename T>
struct is_in_list<T> : std::false_type {};

template<typename T, typename Head, typename ...Tail>
struct is_in_list<T, Head, Tail...>
{
    static constexpr bool value = is_in_list<T, Tail...>::value;
};

template<typename T, typename ...Tail>
struct is_in_list<T, T, Tail...> : std::true_type {};

template<typename T>
struct unique_types_tuple;

template<>
struct unique_types_tuple<std::tuple<>>
{
    using type = std::tuple<>;
};

template<typename Head, typename ...Tail>
struct unique_types_tuple<std::tuple<Head, Tail...>>
{
    using unique_tail = typename unique_types_tuple<std::tuple<Tail...>>::type;
    using type = std::conditional_t<not is_in_list<Head, Tail...>::value, typename tuple_extend<Head, unique_tail>::type, unique_tail>;
};

// Given a tuple A, define a tuple B where every type that appears in tuple A appears in tuple B only once
template<typename T>
using unique_types_tuple_t = typename unique_types_tuple<T>::type;

}
}