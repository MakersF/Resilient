#pragma once

#include <cassert>

#include <utility>
#include <tuple>
#include <resilient/common/utilities.hpp>
#include <boost/variant.hpp>

namespace resilient {

namespace detail {

// Visitor used to determine if the current type in the variant is T
template<typename T>
class IsType
    : public boost::static_visitor<bool>
{
public:

    bool operator()(const T&) const
    {
        return true;
    }

    template<typename Other>
    bool operator()(const Other&) const
    {
        return false;
    }
};

template<typename Q>
using if_is_default_constructible = std::enable_if_t<std::is_default_constructible<Q>::value>;

}

template<typename Variant>
struct variant_traits; // Specialize it for your variant type

template<typename Other>
using variant_traits_t = variant_traits<std::decay_t<Other>>;

template<typename Q>
using is_variant = is_complete_type<variant_traits_t<Q>>;
// A type is a variant if a specialization of variant_traits exists for it.
// If the specialization is defined than the type is complete, otherwise is
// not complete (since by default it's not defined but only declared).

template<typename Q>
using if_is_variant = std::enable_if_t<is_variant<Q>::value, void*>;

template<typename Q>
using if_is_not_variant = std::enable_if_t<not is_variant<Q>::value, void*>;

template<typename ...Types>
class Variant
{
// wrap boost variant, providing a more c++17 similar interface.
// TODO use std::variant if available
public:
    template<typename U = argpack_element_t<0, Types...>,
             typename = detail::if_is_default_constructible<U>>
    Variant() {}

    template<typename Self, if_is_variant<Self> = nullptr>
    Variant(Self&& other)
    : d_data(variant_traits_t<Self>::get_data(other)) {}

    template<typename Other, if_is_not_variant<Other> = nullptr>
    Variant(Other&& other)
    : d_data(std::forward<Other>(other)) {}

    template<typename Self, if_is_variant<Self> = nullptr>
    Variant& operator=(Self&& other)
    {
        d_data = move_if_rvalue<Self>(variant_traits_t<Self>::get_data(other));
        return *this;
    }

    template<typename Other, if_is_not_variant<Other> = nullptr>
    Variant& operator=(Other&& other)
    {
        d_data = std::forward<Other>(other);
        return *this;
    }

private:
    boost::variant<Types...> d_data;

    template<typename ...OtherTypes>
    friend class Variant;

    friend class variant_traits<Variant>;
};

// Trait used to extract the underlying boost::variant data, so that we can use
// boost functions on the variant.
// Whether a fully defined specialization for this trait exists is also used to
// determine if a type is a variant or not
template<typename ...Types>
struct variant_traits<Variant<Types...>>
{
    template<typename V>
    static same_const_ref_as_t<V, boost::variant<Types...>> get_data(V&& variant)
    {
        return move_if_rvalue<V>(variant.d_data);
    }
};

template<typename T, typename Variant, if_is_variant<Variant> = nullptr>
bool holds_alternative(Variant&& variant) // TODO make it fail to compile if the variant can not hold T
{
    return boost::apply_visitor(
        detail::IsType<T>(),
        variant_traits_t<Variant>::get_data(variant));
}

template<typename T, typename Variant, if_is_variant<Variant> = nullptr>
auto get(Variant&& variant) -> same_const_ref_as_t<Variant, T>
{
    assert(holds_alternative<T>(variant));
    return move_if_rvalue<Variant>(
            boost::strict_get<T>(
                variant_traits_t<Variant>::get_data(
                    std::forward<Variant>(variant))));
}

template<typename Variant>
struct variant_cat;

template<typename ...Types>
struct variant_cat<Variant<Types...>>
{
    template<typename ...NewTypes>
    using type = Variant<Types..., NewTypes...>;
};

template<typename Variant, typename ...NewTypes>
using variant_cat_t = typename variant_cat<Variant>::template type<NewTypes...>;

template<typename Variant>
struct is_any_variant : std::false_type {};

template<typename ...Types>
struct is_any_variant<Variant<Types...>> : std::true_type {};
}