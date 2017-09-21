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

}