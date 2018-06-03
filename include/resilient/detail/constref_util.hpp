#pragma once

#include <utility>

namespace resilient {
namespace detail {

namespace impl {

template<typename T, typename Q>
struct same_ref_as
{
    using type = std::remove_reference_t<Q>;
};
template<typename T, typename Q>
struct same_ref_as<T&, Q>
{
    using type = std::add_lvalue_reference_t<std::remove_reference_t<Q>>;
};
template<typename T, typename Q>
struct same_ref_as<T&&, Q> : same_ref_as<T, Q>
{
};

template<typename T, typename Q>
struct same_const_as
{
    using type = std::remove_const_t<Q>;
};
template<typename T, typename Q>
struct same_const_as<const T, Q>
{
    using type = std::add_const_t<Q>;
};

} // namespace impl

// Alias for the type T as if it was declared with the same ref-qualifier as Q
template<typename T, typename Q>
using same_ref_as_t = typename impl::same_ref_as<T, Q>::type;

// Alias for the type T as if it was declared with the same const-qualifier as Q
template<typename T, typename Q>
using same_const_as_t = typename impl::same_const_as<std::remove_reference_t<T>, Q>::type;
// Need to remove the references because references are never const and probably the user
// wants to use the constness of the referred to type

template<typename T, typename Q>
using same_const_ref_as_t = same_ref_as_t<T, same_const_as_t<T, Q>>;

} // namespace detail
} // namespace resilient