#pragma once

#include <type_traits>
#include <variant>

namespace resilient {
namespace detail {

template<typename ...T>
using Variant = std::variant<T...>;

template<typename T>
struct is_variant : std::false_type {};
template<typename ...T>
struct is_variant<Variant<T...>> : std::true_type {};

using std::holds_alternative;
using std::get;
using std::visit;

}
}