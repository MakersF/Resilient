#pragma once

#include <utility>

#define HAS_VARIANT __cplusplus >= 201703L

#if FORCE_STD && FORCE_BOOST
static_assert(false, "Only one of FORCE_STD and FORCE_BOOST can be set.")
#endif

#if ((HAS_VARIANT || FORCE_STD) && !FORCE_BOOST)
#include <resilient/detail/variant_std.hpp>
#else
#include <resilient/detail/variant_boost.hpp>
#endif

namespace resilient {

template<typename ...T>
using Variant = detail::Variant<T...>;

template<typename T>
using is_variant = detail::is_variant<T>;

template<typename T>
using if_is_variant = std::enable_if_t<is_variant<std::decay_t<T>>::value, void*>;

using detail::holds_alternative;
using detail::get;
using detail::visit;

}