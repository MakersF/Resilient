#pragma once

#include <tuple>

namespace resilient {
namespace detail {

// Append an element to a tuple
template<typename Tuple, typename T>
decltype(auto) tuple_append(Tuple&& tuple, T&& item)
{
    return std::tuple_cat(std::forward<Tuple>(tuple), std::tuple<T>(std::forward<T>(item)));
}

// Flatten a list of tuples into a single tuple
template<typename... Tuples>
using tuple_flatten_t = decltype(std::tuple_cat(std::declval<Tuples>()...));

namespace impl {

template<typename A, typename B>
struct tuple_extend;

template<typename... T, typename B>
struct tuple_extend<std::tuple<T...>, B>
{
    using type = std::tuple<T..., B>;
};

template<typename A, typename... T>
struct tuple_extend<A, std::tuple<T...>>
{
    using type = std::tuple<A, T...>;
};

template<typename... T, typename... Q>
struct tuple_extend<std::tuple<T...>, std::tuple<Q...>>
{
    using type = std::tuple<T..., Q...>;
};

// Check if T appears in Tail
template<typename T, typename... Tail>
struct is_in_list;

template<typename T>
struct is_in_list<T> : std::false_type
{
};

template<typename T, typename Head, typename... Tail>
struct is_in_list<T, Head, Tail...>
{
    static constexpr bool value = is_in_list<T, Tail...>::value;
};

template<typename T, typename... Tail>
struct is_in_list<T, T, Tail...> : std::true_type
{
};

template<typename T>
struct unique_types_tuple;

template<>
struct unique_types_tuple<std::tuple<>>
{
    using type = std::tuple<>;
};

template<typename Head, typename... Tail>
struct unique_types_tuple<std::tuple<Head, Tail...>>
{
    using unique_tail = typename unique_types_tuple<std::tuple<Tail...>>::type;
    using type = std::conditional_t<not is_in_list<Head, Tail...>::value,
                                    typename tuple_extend<Head, unique_tail>::type,
                                    unique_tail>;
};

} // namespace impl

// Given a tuple A, define a tuple B where every type that appears in tuple A appears in tuple B
// only once
template<typename T>
using unique_types_tuple_t = typename impl::unique_types_tuple<T>::type;

} // namespace detail
} // namespace resilient
