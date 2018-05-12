#pragma once

#include <utility>

#define HAS_VARIANT __cplusplus >= 201703L

#if FORCE_STD && FORCE_BOOST
static_assert(false, "Only one of FORCE_STD and FORCE_BOOST can be set.");
#endif

#if ((HAS_VARIANT || FORCE_STD) && !FORCE_BOOST)
#include <resilient/detail/variant_std.hpp>
#else
#include <resilient/detail/variant_boost.hpp>
#endif

namespace resilient {

/**
 * @ingroup Common
 * @brief A Variant implementation.
 *
 * The concept is equivalent with the std::variant from C++17.
 *
 * @see http://en.cppreference.com/w/cpp/utility/variant
 *
 * @tparam T... The types of the variant
 */
template<typename... T>
using Variant = detail::Variant<T...>;

/**
 * @ingroup Common
 * @brief Check whether a type is a Variant.
 * @related resilient::Variant
 *
 * @tparam T The type to check
 */
template<typename T>
using is_variant = detail::is_variant<T>;

/**
 * @ingroup Common
 * @brief SFINAE type which is valid if the type is a variant.
 * @related resilient::Variant
 *
 * @tparam T The type that should be a variant
 */
template<typename T>
using if_is_variant = std::enable_if_t<is_variant<std::decay_t<T>>::value, void*>;

/**
 * @ingroup Common
 * @brief SFINAE type which is valid if the type is not a variant.
 * @related resilient::Variant
 *
 * @tparam T The type that should be a variant
 */
template<typename T>
using if_is_not_variant = std::enable_if_t<not is_variant<std::decay_t<T>>::value, void*>;

/**
 * @ingroup Common
 * @brief Check whether the Variant contains a type
 * @related resilient::Variant
 * @see http://en.cppreference.com/w/cpp/utility/variant
 */
using detail::holds_alternative;

/**
 * @ingroup Common
 * @brief Get the type contained by the variant
 * @related resilient::Variant
 * @see http://en.cppreference.com/w/cpp/utility/variant
 */
using detail::get;

/**
 * @ingroup Common
 * @brief Calls a visitor with the type currently hold by the variant.
 * @related resilient::Variant
 * @see http://en.cppreference.com/w/cpp/utility/variant
 */
using detail::visit;
} // namespace resilient