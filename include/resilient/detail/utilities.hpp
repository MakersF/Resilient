#pragma once

#include <tuple>
#include <utility>

namespace resilient {
namespace detail {

// Equivalent of forward but we can use a different type to decide how to forward.
// This is useful when you want to forward a variable based on the type of a different variable.
template<typename As, typename T>
constexpr decltype(auto) move_if_not_lvalue(T&& value)
{
    static_assert(not(std::is_rvalue_reference<T>::value and std::is_lvalue_reference<As>::value),
                  "Forwarding an rvalue reference as a lvalue reference is not allowed.");
    // Allowing forwarding a rval as lval might cause dangling references
    return static_cast<std::conditional_t<std::is_lvalue_reference<As>::value,
                                          std::remove_reference_t<T>&,
                                          std::remove_reference_t<T>&&>>(value);
}

} // namespace detail
} // namespace resilient