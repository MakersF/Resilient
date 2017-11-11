#pragma once

#include <utility>
#include <tuple>

namespace resilient {

template<typename Tuple, typename T>
decltype(auto) tuple_append(Tuple&& tuple, T&& item)
{
    return std::tuple_cat(std::forward<Tuple>(tuple),
                          std::tuple<T>(std::forward<T>(item)));
}

// Prepend or append a new type or a tuple to a tuple
template<typename A, typename B>
struct tuple_extend;

template<typename ...T, typename B>
struct tuple_extend<std::tuple<T...>, B>
{
    using type = std::tuple<T..., B>;
};

template<typename A, typename ...T>
struct tuple_extend<A, std::tuple<T...>>
{
    using type = std::tuple<A, T...>;
};

template<typename ...T, typename ...Q>
struct tuple_extend<std::tuple<T...>, std::tuple<Q...>>
{
    using type = std::tuple<T..., Q...>;
};

template<typename A, typename B>
using tuple_extend_t = typename tuple_extend<A, B>::type;

template<typename ...Tuples>
using tuple_flatten_t = decltype(std::tuple_cat(std::declval<Tuples>()...));

template<typename As, typename T>
constexpr decltype(auto)
move_if_rvalue(T&& value)
{
    static_assert(
        not(std::is_rvalue_reference<T>::value and
            std::is_lvalue_reference<As>::value),
        "Forwarding an rvalue reference as a lvalue reference is not allowed.");
    // Allowing forwarding a rval as lval might cause dangling references
     return static_cast<
         std::conditional_t<
             std::is_lvalue_reference<As>::value,
                 std::remove_reference_t<T> &,
                 std::remove_reference_t<T> &&>>(value);
}

// Get the n-th type from the template argument pack
template<std::size_t N, typename ...Args>
using argpack_element_t = std::tuple_element_t<N, std::tuple<Args...>>;

template<typename T, typename Q>
struct same_ref_as {using type = std::add_rvalue_reference_t<std::remove_reference_t<Q>>;};
template<typename T, typename Q>
struct same_ref_as<T&, Q> {using type = std::add_lvalue_reference_t<std::remove_reference_t<Q>>;};
template<typename T, typename Q>
struct same_ref_as<T&&, Q> : same_ref_as<T, Q> {};

// Alias for the type T as if it was declared with the same ref-qualifier as Q
template<typename T, typename Q>
using same_ref_as_t = typename same_ref_as<T, Q>::type;

template<typename T, typename Q>
struct same_const_as {using type = std::remove_const_t<Q>;};
template<typename T, typename Q>
struct same_const_as<const T, Q> {using type = std::add_const_t<Q>;};

// Alias for the type T as if it was declared with the same const-qualifier as Q
template<typename T, typename Q>
using same_const_as_t = typename same_const_as<std::remove_reference_t<T>, Q>::type;
// Need to remove the references because references are never const and probably the user
// wants to use the constness of the referred to type

template<typename T, typename Q>
using same_const_ref_as_t = same_ref_as_t<T, same_const_as_t<T, Q>>;

namespace detail {

template<class First, std::size_t>
using first_t = First;

}

template<class T>
struct is_complete_type: std::false_type {};

template<class T>
struct is_complete_type<detail::first_t<T, sizeof(T)>> : std::true_type {};

}